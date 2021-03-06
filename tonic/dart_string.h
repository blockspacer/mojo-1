// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TONIC_DART_STRING_H_
#define TONIC_DART_STRING_H_

#include "base/logging.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/wtf/text/WTFString.h"

namespace tonic {

Dart_Handle CreateDartString(StringImpl* string_impl);
String ExternalizeDartString(Dart_Handle handle);

}  // namespace tonic

#endif  // TONIC_DART_STRING_H_
