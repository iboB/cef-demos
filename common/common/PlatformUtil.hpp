// CEF-Demos
// Copyright (c) 2019 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include <string>

namespace cefdemos
{

class PlatformUtil
{
public:
    // get the full path to the current executable
    static std::string getCurrentExecutablePath();

    // starts looking from the current directory upwards until it discovers a valid subdirectory described by assetDir
    // for example
    // getAssetPath("/home/someuser/projects/xxx/build/bin", "assets"); will return /home/someuser/projects/xxx/assests if this directory exists
    std::string getAssetPath(std::string baseDir, const std::string& assetDir);

};

}
