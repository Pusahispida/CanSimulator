/*!
* \file
* \brief stdstring_to_qstring.cpp foo
*/

#include "stdstring_to_qstring.h"

/*!
 * \brief qsFromSs
 * QString from std::string
 * \param str: std::string to be translated
 * \return str as QString
 */
QString qsFromSs(const std::string &str)
{
    return QString::fromLocal8Bit(str.c_str());
}
