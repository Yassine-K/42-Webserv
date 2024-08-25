#include "../inc/autoindex.hpp"
#include <cstring>
#include <sstream>
#include <string>

AutoIndex::AutoIndex()
{}

std::string AutoIndex::getDirectoriesList(list& directories_names, list& directories_dates, list& directories_sizes)
{
	std::string list = "<li>";

	for(size_t i = 0; i < directories_names.size(); i++)
	{
		list += "<a href=\"" + directories_names[i] + "\">" + directories_names[i] + "</a>" +
                "<span style=\"display: block; text-align: center;\">" + directories_dates[i] + "</span>" +
                "<span style=\"display: block; text-align: right;\">" + directories_sizes[i] + "</span>" +
       "</li>";
	}
	return (list);
}

std::string getModificationDate(const std::string& filePath)
{
    struct stat fileInfo;

    if (stat(filePath.c_str(), &fileInfo) != 0)
        return "Error in getting file info";
    // Extract the last modification time
    time_t modifiedTime = fileInfo.st_mtime;
    char dateStr[100];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", localtime(&modifiedTime));
    return dateStr;
}

std::string     AutoIndex::getFileSize(const std::string& filePath)
{
    struct stat fileInfo;
    std::stringstream s;
 
    if (stat(filePath.c_str(), &fileInfo) != 0)
        return "Error in getting file info";

    s << fileInfo.st_size;
    return s.str();
}

std::string        AutoIndex::readFile(std::string filename)
{
    std::string content;
    std::fstream    file;
    std::string     line;
	std::string 	token;
	
    content = "";
    file.open(filename.c_str(), std::ios::in);
    if (file.is_open())
    {
        while (getline(file, line))
			content += line + "\n";
	}
	file.close();
    return (content);
}

std::string	 AutoIndex::serve_autoindex(const std::string& uri, const std::string& full_path)
{
	DIR             *directory;
    struct dirent   *entry;
    list            directories_names;
    list            directories_dates;
    list            directories_sizes;
    std::string     content_page;

    struct stat fileInfo;

    std::cout << "response path : " << full_path << std::endl;

    stat(full_path.c_str(), &fileInfo);

    if (S_ISREG(fileInfo.st_mode))
    {
        std::cout << "IS FILE" << std::endl;
        content_page = readFile(full_path);
    }
    else 
    {
        // Open the directory
        directory = opendir(full_path.c_str());

        // Read each entry in the directory
        while ((entry = readdir(directory)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            directories_names.push_back(entry->d_name);
            directories_dates.push_back(getModificationDate(full_path + entry->d_name));
            directories_sizes.push_back(getFileSize(full_path + entry->d_name));
        }

        // Close the directory

        closedir(directory);
        content_page = 
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "    <meta charset=\"UTF-8\">\n"
            "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
            "    <title>Index of " + uri + "</title>\n"
            "</head>\n"
            "<body>\n"
            "    <h1>Index of " + uri + "</h1>\n"
            "    <hr>\n"
            "    <ul style=\"margin-right:50px; margin-left:50px;\">\n"
            "        <!-- Directory listing will be generated here dynamically -->\n"
            "        <!-- Each list item should represent a file or subdirectory -->\n"
            "        <!-- You can use server-side scripting or tools to automate this process -->\n"
                        + getDirectoriesList(directories_names, directories_dates, directories_sizes) +
            "        <!-- Add more entries as needed -->\n"
            "    </ul>\n"
            "    <hr>\n"
            "</body>\n"
            "</html>";
    }
	return (content_page);
}