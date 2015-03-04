// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_OUT_OF_PROCESS_DYNAMIC_SERVICE_RUNNER_H_
#define SHELL_OUT_OF_PROCESS_DYNAMIC_SERVICE_RUNNER_H_

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "mojo/public/cpp/bindings/error_handler.h"
#include "shell/app_child_process.mojom.h"
#include "shell/app_child_process_host.h"
#include "shell/application_manager/application_manager.h"
#include "shell/dynamic_service_runner.h"

namespace mojo {
namespace shell {

class Context;

// An implementation of |DynamicServiceRunner| that loads/runs the given app
// (from the file system) in a separate process (of its own).
class OutOfProcessDynamicServiceRunner : public NativeRunner {
 public:
  explicit OutOfProcessDynamicServiceRunner(Context* context);
  ~OutOfProcessDynamicServiceRunner() override;

  // |DynamicServiceRunner| method:
  void Start(const base::FilePath& app_path,
             NativeRunner::CleanupBehavior cleanup_behavior,
             InterfaceRequest<Application> application_request,
             const base::Closure& app_completed_callback) override;

 private:
  // |AppChildController::StartApp| callback:
  void AppCompleted(int32_t result);

  Context* const context_;

  base::FilePath app_path_;
  base::Closure app_completed_callback_;

  scoped_ptr<AppChildProcessHost> app_child_process_host_;

  DISALLOW_COPY_AND_ASSIGN(OutOfProcessDynamicServiceRunner);
};

class OutOfProcessDynamicServiceRunnerFactory : public NativeRunnerFactory {
 public:
  explicit OutOfProcessDynamicServiceRunnerFactory(Context* context)
      : context_(context) {}
  ~OutOfProcessDynamicServiceRunnerFactory() override {}

  scoped_ptr<NativeRunner> Create(const Options& options) override;

 private:
  Context* const context_;

  DISALLOW_COPY_AND_ASSIGN(OutOfProcessDynamicServiceRunnerFactory);
};

}  // namespace shell
}  // namespace mojo

#endif  // SHELL_OUT_OF_PROCESS_DYNAMIC_SERVICE_RUNNER_H_
