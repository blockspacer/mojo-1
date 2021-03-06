// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TONIC_DART_API_SCOPE_H_
#define TONIC_DART_API_SCOPE_H_

#include "base/macros.h"
#include "dart/runtime/include/dart_api.h"

namespace tonic {

class DartApiScope {
 public:
  DartApiScope() { Dart_EnterScope(); }
  ~DartApiScope() { Dart_ExitScope(); }

 private:
  DISALLOW_COPY_AND_ASSIGN(DartApiScope);
};

}  // namespace tonic

#endif  // TONIC_DART_API_SCOPE_H_
