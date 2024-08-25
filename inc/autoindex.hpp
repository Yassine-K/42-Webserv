#pragma once

#include <string>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <sys/stat.h>
#include <sstream>
#include <filesystem>
#include <fstream>


typedef std::vector<std::string> list;

class  AutoIndex
{
    public :
        AutoIndex();

        static std::string     getDirectoriesList(list&, list&, list&);
        static std::string     serve_autoindex(const std::string&, const std::string&);
        static std::string     getFileSize(const std::string& filePath);
        static std::string     readFile(std::string filename);

};