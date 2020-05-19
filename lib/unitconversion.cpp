/*!
* \file
* \brief unitconversion.cpp foo
*/

#include "unitconversion.h"
#include <vector>

// Supported conversion types, matches with ConvertTo enum
const std::vector<std::string> conversionTypes (
    { "none",       // no conversion is needed
    "mi",           // miles
    "km",           // kilometers
    "km/h",         // kilometers / hour
    "mi/h",         // miles / hour
    "m/s",          // meters / second
    "F",            // fahrenheit degrees
    "K" } );        // Kelvin degrees

/*!
 * \brief unitToConversionType
 * Check unit conversion type
 *
 * \param conversion: CAN getUnit value
 *
 * \return ConvertTo enum value
 */
ConvertTo unitToConversionType(std::string conversion)
{
    for (auto it = conversionTypes.begin(); it != conversionTypes.end(); ++it) {
        if (!conversion.compare(*it)) {
            // return iteration according to the enum list
            return (ConvertTo)std::distance(conversionTypes.begin(), it);
        }
    }
    return ConvertTo::NONE;
}

/*!
 * \brief convertFromMeters
 * Convert meters into desired distance
 *
 * \param value: CAN value in meters
 * \param conversion: ConvertTo enum value
 *
 * \return true if conversion happened, otherwise false
 */
bool convertFromMeters(long double &value, ConvertTo conversion)
{
    switch (conversion) {
        case ConvertTo::MI:        // To miles
            value *= 0.000621371;
            return true;
        case ConvertTo::KM:        // To kilometers
            value *= 0.001;
            return true;
        default:
            break;
    }
    return false;
}

/*!
 * \brief convertFromMetersPerHours
 * Convert meters/hour into desired speed
 *
 * \param value: CAN value in m/h
 * \param conversion: ConvertTo enum value
 *
 * \return true if conversion happened, otherwise false
 */
bool convertFromMetersPerHours(long double &value, ConvertTo conversion)
{
    switch (conversion) {
        case ConvertTo::KMH:       // To kilometers/hour
            value *= 0.001;
            return true;
        case ConvertTo::MPH:       // To miles/hour
            value *= 0.000621371;
            return true;
        case ConvertTo::MS:        // To meters/second
            value *= 0.000277778;
            return true;
        default:
            break;
    }
    return false;
}

/*!
 * \brief convertFromCelcius
 * Convert celcius to another degree
 *
 * \param value: CAN value in celcius
 * \param conversion: ConvertTo enum value
 *
 * \return true if conversion happened, otherwise false
 */
bool convertFromCelcius(long double &value, ConvertTo conversion)
{
    switch (conversion) {
        case ConvertTo::F:     // To Fahrenheit
            value = ((value * 1.8) + 32);
            return true;
        case ConvertTo::K:     // To Kelvin
            value += 273.15;
            return true;
        default:
            break;
    }
    return false;
}

/*!
 * \brief unitConversion
 * Converts value from source unit to signals unit type
 * Supported source units: m, m/h, degC
 *
 * \param value: Value desired to be converted into new unit
 * \param conversion: Enum ConvertTo value, to make the right conversion
 *
 * \return true if conversion happened, otherwise false
 */
bool unitConversion(long double &value, ConvertTo conversion)
{
    if (conversion == ConvertTo::NONE) {
        return false;;
    }

    switch (conversion) {
        case ConvertTo::MI:
        case ConvertTo::KM:
            return convertFromMeters(value, conversion);
            break;
        case ConvertTo::KMH:
        case ConvertTo::MPH:
        case ConvertTo::MS:
            return convertFromMetersPerHours(value, conversion);
            break;
        case ConvertTo::F:
        case ConvertTo::K:
            return convertFromCelcius(value, conversion);
            break;
        default:
            break;
    }
    return false;
}
