/*!
* \file
* \brief cansignal.cpp foo
*/

#include "cansignal.h"
#include "cansimulatorcore.h"
#include "logger.h"
#include <cmath>
#include <map>
#include <sstream>

/*!
 * \brief CANSignal::CANSignal
 * Constructor
 */
CANSignal::CANSignal(const Signal &signal) :
    m_modified(false)
{
    name = signal.getName();
    order = signal.getByteOrder();
    startBit = signal.getStartbit();
    length = signal.getLength();
    sign = signal.getSign();
    minimum = signal.getMinimum();
    maximum = signal.getMaximum();
    factor = signal.getFactor();
    offset = signal.getOffset();
    unit = signal.getUnit();
    m_conversion = unitToConversionType(signal.getUnit());
    multiplexor = signal.getMultiplexor();
    multiplexNum = signal.getMultiplexedNumber();
    to = signal.getTo();
    description = signal.getDescription();
    std::map<unsigned int, std::string> descs = signal.getValueDescriptions();
    for (auto const &desc : descs) {
        setValueDescription(desc.first, desc.second);
    }
    m_valueType = "int";
    m_isValueSet = false;
    std::map<std::string, Attribute> attributes = signal.getAttributes();
    for (const auto &attribute : attributes) {
        attributeList.insert({attribute.first, Attribute(attribute.second)});
    }
}

/*!
 * \brief CANSignal::isModified
 * Return whether signal content has been modified since last send
 * \return True if signal content has been modified, false otherwise.
 */
bool CANSignal::isModified() const
{
    return m_modified;
}

/*!
 * \brief CANSignal::setModified
 * Set whether signal content has been modified since last send
 * \param modified: Boolean whether signal content has been modified since last send
 */
void CANSignal::setModified(bool modified)
{
    m_modified = modified;
}

/*!
 * \brief CANSignal::getValue
 * Get current value of the signal
 * \return Value of the signal
 */
const Value &CANSignal::getValue() const
{
    return m_value;
}

/*!
 * \brief CANSignal::getDefaultValue
 * Get default value of the signal
 * \return Default value of the signal
 */
const Value &CANSignal::getDefaultValue() const
{
    return m_defaultValue;
}

/*!
 * \brief CANSignal::getRawValue
 * Get current value of the signal as raw value ready for CAN frame
 * \return Raw value the signal ready for CAN frame
 */
uint64_t CANSignal::getRawValue() const
{
    uint64_t rawValue = 0;
    if (sign == Sign::SIGNED) {
        if (length == 8) {
            rawValue = (uint8_t)llround((m_value.toDouble() - offset) / factor);
        } else if (length == 16) {
            rawValue = (uint16_t)llround((m_value.toDouble() - offset) / factor);
        } else if (length == 32) {
            rawValue = (uint32_t)llround((m_value.toDouble() - offset) / factor);
        } else {
            rawValue = llround((m_value.toDouble() - offset) / factor);
        }
    } else {
        rawValue = llround((m_value.toDouble() - offset) / factor);
    }
    return rawValue;
}

/*!
 * \brief CANSignal::isValueSet
 * Check whether value has been set using setValue or setValueFromRaw functions
 * after initialization or resetValue
 * \return True if value has been set, false otherwise.
 */
bool CANSignal::isValueSet() const
{
    return m_isValueSet;
}

/*!
 * \brief CANSignal::setDefaultValue
 * Set default value of the signal from a string
 * \param valueString: Value as a string
 * \return True if successful, false otherwise.
 */
bool CANSignal::setDefaultValue(const std::string &valueString)
{
    Value val;
    if (parseValue(valueString, val)) {
        m_defaultValue = val;
        return true;
    }
    return false;
}

/*!
 * \brief CANSignal::setValue
 * Set value of the signal from a string
 * \param valueString: Value as a string
 * \return True if successful, false otherwise.
 */
bool CANSignal::setValue(const std::string &valueString)
{
    Value val;
    if (parseValue(valueString, val)) {
        m_isValueSet = true;
        m_modified = true;
        m_value = val;
        return true;
    } else {
        LOG(LOG_WARN, "warning=4 Invalid value: %s=%s\n", name.c_str(), valueString.c_str());
        return false;
    }
}

/*!
 * \brief CANSignal::setValue
 * Set value of the signal
 * \param value: new value
 * \return True if successful, false otherwise.
 */
bool CANSignal::setValue(Value &value)
{
    if (testValue(value)) {
        m_isValueSet = true;
        m_modified = true;
        m_value = value;
        return true;
    } else {
        LOG(LOG_WARN, "warning=4 Invalid value: %s=%s\n", name.c_str(), value.toString().c_str());
        return false;
    }
}

/*!
 * \brief CANSignal::setValueFromRaw
 * Set value of the signal from raw CAN value
 * \param rawValue: new value
 * \return True if successful, false otherwise.
 */
bool CANSignal::setValueFromRaw(uint64_t rawValue)
{
    Value val;
    if (m_value.type() == Value::Double) {
        double value;
        if (sign == Sign::SIGNED) {
            if (length == 8) {
                value = ((int8_t)rawValue * factor) + offset;
            } else if (length == 16) {
                value = ((int16_t)rawValue * factor) + offset;
            } else if (length == 32) {
                value = ((int32_t)rawValue * factor) + offset;
            } else {
                return false;
            }
        } else {
            value = (rawValue * factor) + offset;
        }
        val = Value(value);
    } else if (m_value.type() == Value::Unsigned) {
        unsigned long value = llround((rawValue * factor) + offset);
        val = Value(value);
    } else {
        // No unit conversion for integers?
        int value;
        if (sign == Sign::SIGNED) {
            if (length == 8) {
                value = lround(((int8_t)rawValue * factor) + offset);
            } else if (length == 16) {
                value = lround(((int16_t)rawValue * factor) + offset);
            } else if (length == 32) {
                value = lround(((int32_t)rawValue * factor) + offset);
            } else {
                return false;
            }
        } else {
            value = lround((rawValue * factor) + offset);
        }
        val = Value(value);
    }
    if (testValue(val)) {
        m_isValueSet = true;
        m_modified = true;
        m_value = val;
        return true;
    } else {
        LOG(LOG_WARN, "warning=4 Invalid value: %s=%s\n", name.c_str(), val.toString().c_str());
        return false;
    }
}

/*!
 * \brief CANSignal::setVariableName
 * Set name of the signal variable
 * \param name: name of the signal variable
 */
void CANSignal::setVariableName(const std::string &name)
{
    m_variableName = name;
}

/*!
 * \brief CANSignal::getVariableName
 * Get name of the signal variable
 * \return Name of the signal variable
 */
const std::string &CANSignal::getVariableName() const
{
    return m_variableName;
}

/*!
 * \brief CANSignal::testValue
 * Test if value is valid
 * \param value: new value
 * \return True if successful, false otherwise.
 */
bool CANSignal::testValue(Value &value) const
{
    if (value.toDouble() < minimum || value.toDouble() > maximum) {
        if (std::abs(value.toDouble() - minimum) <= std::abs(std::min(value.toDouble(), minimum)) * std::numeric_limits<double>::epsilon() ||
            std::abs(value.toDouble() - maximum) <= std::abs(std::min(value.toDouble(), maximum)) * std::numeric_limits<double>::epsilon()) {
            return true;
        }
        return false;
    } else {
        return true;
    }
}

/*!
 * \brief CANSignal::parseValue
 * Parse a Value from a string and convert it, if nativeUnits are not used
 * \param valueString: Value as a string
 * \param value: Reference to the Value to be filled
 * \return True if successful, false otherwise.
 */
bool CANSignal::parseValue(const std::string &valueString, Value &value)
{
    long double parsedValue;
    try {
        parsedValue = std::stold(valueString);
    }
    catch (const std::invalid_argument &) {
        return false;
    }
    catch (const std::out_of_range &) {
        return false;
    }
    if (!CANSimulatorCore::getUseNativeUnits()) {
        unitConversion(parsedValue, m_conversion);
    }
    if (!m_valueType.compare("double")) {
        value = Value(static_cast<double>(parsedValue));
    } else if (!m_valueType.compare("unsigned")) {
        value = Value(static_cast<unsigned long>(llround(parsedValue)));
    } else {
        value = Value(static_cast<int>(lround(parsedValue)));
    }
    return testValue(value);
}

/*!
 * \brief CANSignal::setValueType
 * Set type of the signal value
 * \param type: Type of the signal value
 */
void CANSignal::setValueType(const std::string &type)
{
    m_valueType = type;
}

/*!
 * \brief CANSignal::resetValue
 * Reset signal to default value.
 * \param setValue: Define value as set after reset
 * \return True if value changed, false otherwise.
 */
bool CANSignal::resetValue(bool setValue)
{
    m_isValueSet = setValue;
    if (m_value != m_defaultValue) {
        m_value = m_defaultValue;
        m_modified = false;
        return true;
    }
    return false;
}

/*!
 * \brief CANSignal::getConversionUnit
 * Get the ConvertTo value from signal
 * \return ConvertTo enum value
 */
const ConvertTo &CANSignal::getConversionUnit() const
{
    return m_conversion;
}

/*!
 * \brief CANSignal::setConversionUnit
 * Set signal ConvertTo value
 * \param conv: new conversion type defined as ConvertTo
 */
void CANSignal::setConversionUnit(const ConvertTo &conv)
{
    m_conversion = conv;
}

/*!
 * \brief CANSignal::toString
 * Print information about the signal
 * \param details: Add also signal attributes and value descriptions
 * \return Compact overview of the signal as a string
 */
std::string CANSignal::toString(bool details) const
{
    std::stringstream sstream;
    if (!description.empty()) {
        sstream << description << "\n";
    }
    sstream << "name: " << name << ", type: " << m_valueType;

    if (!m_valueType.compare("double")) {
        sstream << ", value: " << m_value.toDouble() << ", range [" << minimum << ", " << maximum << "] " << unit << "\n";
    } else if (!m_valueType.compare("unsigned")) {
        sstream << ", value: " << m_value.toUnsigned() << ", range [" << llround(minimum) << ", " << llround(maximum) << "] " << unit << "\n";
    } else {
        sstream << ", value: " << m_value.toInt() << ", range [" << lround(minimum) << ", " << lround(maximum) << "] " << unit << "\n";
    }
    // Print value descriptions
    if (details) {
        for (std::map<unsigned int, std::string>::const_iterator value = valueDescriptions.begin(); value != valueDescriptions.end(); ++value) {
            sstream << "\t" << value->first << ": " << value->second << "\n";
        }
        sstream << "Signal attributes:" << "\n";
        // Attributes
        for (std::map<std::string, Attribute>::const_iterator value = attributeList.begin(); value != attributeList.end(); ++value) {
            sstream << "\t" << value->first << ": " << value->second.getValue() << "\n";
        }
    }
    return sstream.str();
}
