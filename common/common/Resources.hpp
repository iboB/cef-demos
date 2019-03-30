// CEF-Demos
// Copyright (c) 2019 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include <include/cef_client.h>
#include <include/wrapper/cef_resource_manager.h>


namespace cefdemos
{

class Resources
{
public:
    // setup a resource manager to associate uris below `uri` to point to files below the directory `dir`
    static void setupResourceManagerDirectoryProvider(CefRefPtr<CefResourceManager> resource_manager, std::string uri, std::string dir);
};

}
