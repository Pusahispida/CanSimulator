/*!
* \file
* \brief cansignal.h foo
*/

#ifndef CANSIGNAL_H
#define CANSIGNAL_H

#include "unitconversion.h"
#include "value.h"
#include <can-dbcparser/header/signal.hpp>
#include <cstdint>
#include <string>

class CANMessage;
class Configuration;

class CANSignal : public Signal
{
    friend class CANMessage;
    friend class Configuration;

public:
    explicit CANSignal(const Signal &signal);
    const ConvertTo &getConversionUnit() const;
    const Value &getDefaultValue() const;
    uint64_t getRawValue() const;
    const Value &getValue() const;
    const std::string &getVariableName() const;
    bool isModified() const;
    bool isValueSet() const;
    std::string toString(bool details = false) const;

protected:
    bool resetValue(bool setValue = false);
    void setConversionUnit(const ConvertTo &conv);
    bool setDefaultValue(const std::string &valueString);
    void setModified(bool modified);
    bool setValue(const std::string &valueString);
    bool setValue(Value &value);
    bool setValueFromRaw(uint64_t rawValue);
    void setValueType(const std::string &type);
    void setVariableName(const std::string &name);

private:
    bool m_isValueSet;
    bool m_modified;
    ConvertTo m_conversion;
    std::string m_valueType;
    std::string m_variableName;
    Value m_value;
    Value m_defaultValue;
    bool parseValue(const std::string &valueString, Value &value);
    bool testValue(Value &value) const;
};

#endif // CANSIGNAL_H
