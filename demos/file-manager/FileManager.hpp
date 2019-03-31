// CEF-Demos
// Copyright (c) 2019 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
// GUI-agnostic file manager code
#pragma once

#include <vector>
#include <string>

namespace fileman
{

class FileManager
{
public:
    FileManager();
    ~FileManager();

    struct DirectoryContents
    {
        std::vector<std::string> files;
        std::vector<std::string> dirs;
    };

    // path will be interpreted as relative to inner path
    DirectoryContents getDirectoryContents(const char* path);

private:
    std::string m_path;
};

}