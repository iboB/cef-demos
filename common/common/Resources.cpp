// CEF-Demos
// Copyright (c) 2019 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "Resources.hpp"

namespace cefdemos
{

void Resources::SetupResourceManager(CefRefPtr<CefResourceManager> resource_manager, std::string uri, std::string dir)
{
    if (!CefCurrentlyOn(TID_IO)) {
        // Execute on the browser IO thread.
        CefPostTask(TID_IO, base::Bind(SetupResourceManager, resource_manager, uri, dir));
        return;
    }

    resource_manager->AddDirectoryProvider(uri, dir, 1, dir);
}

}
