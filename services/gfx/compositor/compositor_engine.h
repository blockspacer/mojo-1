// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_GFX_COMPOSITOR_COMPOSITOR_H_
#define SERVICES_GFX_COMPOSITOR_COMPOSITOR_H_

#include <unordered_map>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "mojo/services/gfx/composition/interfaces/compositor.mojom.h"
#include "services/gfx/compositor/backend/scheduler.h"
#include "services/gfx/compositor/renderer_state.h"
#include "services/gfx/compositor/scene_state.h"

namespace compositor {

// Core of the compositor.
// All SceneState and RendererState objects are owned by the engine.
class CompositorEngine {
 public:
  explicit CompositorEngine();
  ~CompositorEngine();

  // COMPOSITOR REQUESTS

  // Registers a scene.
  mojo::gfx::composition::SceneTokenPtr CreateScene(
      mojo::InterfaceRequest<mojo::gfx::composition::Scene> scene_request,
      const mojo::String& label);

  // Creates a scene graph renderer.
  void CreateRenderer(
      mojo::ContextProviderPtr context_provider,
      mojo::InterfaceRequest<mojo::gfx::composition::Renderer> renderer_request,
      const mojo::String& label);

  // SCENE REQUESTS

  // Sets the scene listener.
  void SetListener(SceneState* scene_state,
                   mojo::gfx::composition::SceneListenerPtr listener);

  // Updates a scene.
  // Destroys |scene_state| if an error occurs.
  void Update(SceneState* scene_state,
              mojo::gfx::composition::SceneUpdatePtr update);

  // Publishes a scene.
  // Destroys |scene_state| if an error occurs.
  void Publish(SceneState* scene_state,
               mojo::gfx::composition::SceneMetadataPtr metadata);

  // Schedules a frame callback.
  void ScheduleFrame(SceneState* scene_state,
                     const SceneFrameCallback& callback);

  // RENDERER REQUESTS

  // Sets the root scene.
  // Destroys |renderer_state| if an error occurs.
  void SetRootScene(RendererState* renderer_state,
                    mojo::gfx::composition::SceneTokenPtr scene_token,
                    uint32 scene_version,
                    mojo::RectPtr viewport);

  // Performs a hit test.
  void HitTest(
      RendererState* renderer_state,
      mojo::PointPtr point,
      const mojo::gfx::composition::HitTester::HitTestCallback& callback);

 private:
  void OnSceneConnectionError(SceneState* scene_state);
  void DestroyScene(SceneState* scene_state);

  void OnRendererConnectionError(RendererState* renderer_state);
  void DestroyRenderer(RendererState* renderer_state);

  void InvalidateScene(SceneState* scene_state);
  SceneDef::Disposition PresentScene(SceneState* scene_state,
                                     int64_t presentation_time);

  void PresentRenderer(RendererState* renderer_state,
                       int64_t presentation_time);
  void SnapshotRenderer(RendererState* renderer_state,
                        const mojo::gfx::composition::FrameInfo& frame_info);
  void SnapshotRendererInner(
      RendererState* renderer_state,
      const mojo::gfx::composition::FrameInfo& frame_info,
      std::ostream* block_log);
  void ScheduleFrameForRenderer(RendererState* renderer_state,
                                Scheduler::SchedulingMode scheduling_mode);

  void OnOutputError(const base::WeakPtr<RendererState>& renderer_state_weak);
  void OnOutputUpdateRequest(
      const base::WeakPtr<RendererState>& renderer_state_weak,
      const mojo::gfx::composition::FrameInfo& frame_info);
  void OnOutputSnapshotRequest(
      const base::WeakPtr<RendererState>& renderer_state_weak,
      const mojo::gfx::composition::FrameInfo& frame_info);
  void OnPresentScene(const base::WeakPtr<SceneState>& scene_state_weak,
                      int64_t presentation_time);

  SceneDef* ResolveSceneReference(
      mojo::gfx::composition::SceneToken* scene_token);
  void SendResourceUnavailable(SceneState* scene_state, uint32_t resource_id);

  SceneState* FindScene(uint32_t scene_token);

  bool IsSceneStateRegisteredDebug(SceneState* scene_state) {
    return scene_state && FindScene(scene_state->scene_token()->value);
  }
  bool IsRendererStateRegisteredDebug(RendererState* renderer_state) {
    return renderer_state &&
           std::any_of(renderers_.begin(), renderers_.end(),
                       [renderer_state](RendererState* other) {
                         return renderer_state == other;
                       });
  }

  uint32_t next_scene_token_value_ = 1u;
  uint32_t next_renderer_id_ = 1u;
  std::unordered_map<uint32_t, SceneState*> scenes_by_token_;
  std::vector<RendererState*> renderers_;

  base::WeakPtrFactory<CompositorEngine> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(CompositorEngine);
};

}  // namespace compositor

#endif  // SERVICES_GFX_COMPOSITOR_COMPOSITOR_H_
