/*!
* \file
* \brief stringtools.h foo
*/

#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H

#include <string>
#include <vector>

std::string& trim(std::string& str, const std::string& delim);

std::vector<std::string>& split(const std::string &str, char delim, std::vector<std::string> &items);

std::vector<std::string> split(const std::string &str, char delim);

#endif // STRINGTOOLS_H
