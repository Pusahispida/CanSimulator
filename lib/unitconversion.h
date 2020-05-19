/*!
* \file
* \brief unitconversion.h foo
*/

#ifndef UNITCONVERSION_H
#define UNITCONVERSION_H

#include <string>

// Conversion types to avoid repetitive string comparison in conversion
enum class ConvertTo {
    NONE = 0,
    MI,
    KM,
    KMH,
    MPH,
    MS,
    F,
    K
};

ConvertTo unitToConversionType(std::string conversion);
bool unitConversion(long double &value, ConvertTo conversion);

#endif // UNITCONVERSION_H
