/*!
* \file
* \brief stringtools.cpp foo
*/

#include "stringtools.h"
#include <sstream>

/*!
 * \brief trim
 * Trim beginning and end of string using delimiter and return the result
 * \param str: Reference to string
 * \param delim: Delimiter string
 * \return Reference to the original string
 */
std::string& trim(std::string& str, const std::string& delim = " ")
{
    std::string::size_type pos = str.find_last_not_of(delim);
    if (pos == std::string::npos) {
        str.clear();
    } else {
        str.erase(pos + 1);
        str.erase(0, str.find_first_not_of(delim));
    }
    return str;
}

/*!
 * \brief split
 * Splits string at delimiter and assigns result in last parameter
 * \param str: Input string
 * \param delim: Delimiter character
 * \param items: Output vector for result
 * \return Reference to the output vector passed as parameter
 */
std::vector<std::string>& split(const std::string &str, char delim, std::vector<std::string> &items)
{
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delim)) {
        items.push_back(item);
    }
    return items;
}

/*!
 * \brief split
 * Splits string at delimiter and returns the result
 * \param str: Input string
 * \param delim: Delimiter character
 * \return A vector containing the resulting strings
 */
std::vector<std::string> split(const std::string &str, char delim)
{
    std::vector<std::string> elems;
    split(str, delim, elems);
    return elems;
}
