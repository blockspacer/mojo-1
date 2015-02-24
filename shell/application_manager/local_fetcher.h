// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_APPLICATION_MANAGER_LOCAL_FETCHER_H_
#define SHELL_APPLICATION_MANAGER_LOCAL_FETCHER_H_

#include "base/files/file_path.h"
#include "mojo/services/network/public/interfaces/url_loader.mojom.h"
#include "shell/application_manager/fetcher.h"
#include "url/gurl.h"

namespace mojo {

// Implements Fetcher for file:// URLs.
class LocalFetcher : public Fetcher {
 public:
  LocalFetcher(const GURL& url, const FetchCallback& loader_callback);

 private:
  static base::FilePath UrlToFile(const GURL& url);

  URLResponsePtr AsURLResponse(base::TaskRunner* task_runner,
                               uint32_t skip) override;

  void AsPath(
      base::TaskRunner* task_runner,
      base::Callback<void(const base::FilePath&, bool)> callback) override;

  std::string MimeType() override;

  bool HasMojoMagic() override;

  bool PeekFirstLine(std::string* line) override;

  GURL url_;
  base::FilePath path_;

  DISALLOW_COPY_AND_ASSIGN(LocalFetcher);
};

#endif  // SHELL_APPLICATION_MANAGER_LOCAL_FETCHER_H_

}  // namespace mojo
