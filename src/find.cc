#include <iostream>
#include <string>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <exception>

#include "pattern.hh"
#include "words.hh"
#include "find.hh"
#include "utils.hh"

bool is_directory (std::string& path)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) != 0)
        return false;
    if (S_ISDIR(sb.st_mode))
        return true;
    return false;
}

size_t iterate_dir (std::string& path, Pattern& pattern, Words& words, size_t& line_count, std::vector<int>& words_count)
{
    DIR *dir = opendir(path.c_str());
    if (dir == NULL)
    {
        return 0;
    }
    else
    {
        struct dirent *ent;
        while ((ent = readdir(dir)))
        {
            // Do not go through ".." or "." directories
            std::string tmp = ent->d_name;
            if (tmp == "." or tmp == "..")
                continue;
            // Update path
            std::string new_path;
            if (path[path.size() - 1] == '/')
                new_path = path + ent->d_name;
            else
                new_path = path + "/" + ent->d_name;
            // Matches a pattern
            try
            {
                if (is_match_pattern(new_path, pattern) == true and is_directory(new_path) == false)
                {
                    //std::cout << new_path << '\n';
                    std::string content = read_file(new_path);
                    if (words.get_size() != 0)
                        count_words_in_file(new_path, content, words, words_count);
                    size_t c = count_newlines(content);
                    line_count += c;
                }
            }
            catch (std::exception& e)
            {
                std::cerr << "Error reading file: " << e.what() << '\n';
            }
            // Recurse (since its a directory)
            DIR *tstdir = opendir(new_path.c_str());
            if (tstdir != NULL)
            {
                iterate_dir(new_path, pattern, words, line_count, words_count);
                closedir(tstdir);
            }
        }
    }
    closedir(dir);
    return line_count;
}

bool is_match_pattern (std::string& path, Pattern& pattern)
{
    size_t len = pattern.get_patterns().size();
    for (size_t i = 0; i < len; i++)
    {
        if (fnmatch(pattern[i].c_str(), path.c_str(), 0) == 0)
        {
            return true;
        }
    }
    return false;
}
