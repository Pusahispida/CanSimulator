/*!
* \file
* \brief value.cpp foo
*/

#include "value.h"
#include <cmath>
#include <sstream>

/*!
 * \brief Value::Value
 * Constructor
 */
Value::Value()
    : m_dataType(Value::Integer)
{
    m_data.i = 0;
}

/*!
 * \brief Value::Value
 * Constructor
 * \param i: Initial integer value
 */
Value::Value(int i)
    : m_dataType(Value::Integer)
{
    m_data.i = i;
}

/*!
 * \brief Value::Value
 * Constructor
 * \param d: Initial double value
 */
Value::Value(double d)
    : m_dataType(Value::Double)
{
    m_data.d = d;
}

/*!
 * \brief Value::Value
 * Constructor
 * \param u: Initial unsigned long value
 */
Value::Value(unsigned long u)
    : m_dataType(Value::Unsigned)
{
    m_data.u = u;
}

/*!
 * \brief Value::Value
 * Copy constructor
 * \param value: Value to copy
 */
Value::Value(const Value& value)
    : m_data(value.m_data)
{
    this->m_dataType = value.m_dataType;
}

/*!
 * \brief Value::toInt
 * Cast value to integer
 * \return Value as integer
 */
int Value::toInt() const
{
    if (m_dataType == Value::Integer) {
        return m_data.i;
    } else if (m_dataType == Value::Unsigned) {
        return m_data.u;
    } else {
        return llround(m_data.d);
    }
}

/*!
 * \brief Value::toDouble
 * Cast value to double
 * \return Value as double
 */
double Value::toDouble() const
{
    if (m_dataType == Value::Double) {
        return m_data.d;
    } else if (m_dataType == Value::Unsigned) {
        return (double)m_data.u;
    } else {
        return (double)m_data.i;
    }
}

/*!
 * \brief Value::toUnsigned
 * Cast value to unsigned long
 * \return Value as unsigned long
 */
unsigned long Value::toUnsigned() const
{
    if (m_dataType == Value::Unsigned) {
        return m_data.u;
    } else if (m_dataType == Value::Integer) {
        return m_data.i;
    } else {
        return llround(m_data.d);
    }
}

/*!
 * \brief Value::toString
 * Value as a string
 * \return Value as a string
 */
std::string Value::toString() const
{
    std::stringstream sstream;
    if (m_dataType == Value::Double) {
        sstream << m_data.d;
    } else if (m_dataType == Value::Unsigned) {
        sstream << m_data.u;
    } else {
        sstream << m_data.i;
    }
    return sstream.str();
}

/*!
 * \brief Value::type
 * Type (Integer, Double, Unsigned)
 * \return Type of the value
 */
Value::Type Value::type() const
{
    return m_dataType;
}

/*!
 * \brief Value::operator==
 * Comparison operator
 * \return Boolean whether the values are equal
 */
bool Value::operator==(const Value &value) const
{
    if (this->m_dataType != value.m_dataType) {
        return false;
    } else {
        if (this->m_dataType == Value::Integer) {
            return this->m_data.i == value.m_data.i;
        } else if (this->m_dataType == Value::Unsigned) {
            return this->m_data.u == value.m_data.u;
        } else {
            return this->m_data.d == value.m_data.d;
        }
    }
}

/*!
 * \brief Value::operator!=
 * Non-equal comparison operator
 * \return Boolean whether the values are different
 */
bool Value::operator!=(const Value &value) const
{
    return !(*this == value);
}

/*!
 * \brief Value::operator=
 * Assignment operator
 * \param value: Value to assign
 */
Value & Value::operator=(const Value &value)
{
    this->m_dataType = value.m_dataType;
    this->m_data = value.m_data;
    return *this;
}
