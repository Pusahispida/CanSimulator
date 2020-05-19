/*!
* \file
* \brief loggertableitem.cpp foo
* any other legal text to be defined later
*/

#include "logger.h"
#include "loggertableitem.h"
#include "value.h"
#include "stdstring_to_qstring.h"
#include <QString>

/*!
 * \brief LoggerTableItem::LoggerTableItem
 * Construct an empty logger table item
 */
LoggerTableItem::LoggerTableItem()
    : m_type(StringItem)
    , m_str(QString())
    , m_double(0.0)
{
}

/*!
 * \brief LoggerTableItem::LoggerTableItem
 * Construct a logger table string item
 * \param str: String to represent
 */
LoggerTableItem::LoggerTableItem(const QString &str)
    : m_type(StringItem)
    , m_str(str)
    , m_double(0.0)
{
}

/*!
 * \brief LoggerTableItem::LoggerTableItem
 * Construct a logger table value item
 * \param v: Value to represent
 */
LoggerTableItem::LoggerTableItem(const Value &v)
    : m_type(ValueItem)
    , m_value(v)
    , m_double(0.0)
{
}

/*!
 * \brief LoggerTableItem::LoggerTableItem
 * Construct a logger table double item
 * \param d: Double to represent
 */
LoggerTableItem::LoggerTableItem(double d)
    : m_type(DoubleItem)
    , m_double(d)
{
}

/*!
 * \brief LoggerTableItem::LoggerTableItem
 * Copy construct a logger table item
 * \param other: Item to copy from
 */
LoggerTableItem::LoggerTableItem(const LoggerTableItem &other)
    : m_type(other.m_type)
    , m_str(other.m_str)
    , m_value(other.m_value)
    , m_double(other.m_double)
{
}

/*!
 * \brief LoggerTableItem::toString
 * String representation of this item
 * \return The string representation
 */
QString LoggerTableItem::toString() const
{
    switch(m_type) {
    case StringItem:
        return QString(m_str);
    case ValueItem:
        return qsFromSs(m_value.toString());
    case DoubleItem:
        return QString("%1").arg(m_double, 0, 'f', 3);
    default:
        LOG(LOG_ERR, "LoggerTableItem type wrong\n");
        return QString("Error!");
    }
}

/*!
 * \brief LoggerTableItem::operator=
 * Assignment operator
 * \param other: Other item to copy for
 * \return Reference to self
 */
LoggerTableItem &LoggerTableItem::operator=(const LoggerTableItem &other)
{
    m_type = other.m_type;
    m_str = other.m_str;
    m_value = other.m_value;
    m_double = other.m_double;
    return *this;
}

/*!
 * \brief LoggerTableItem::operator==
 * Equality comparison
 * \param other: Other item to compare with
 * \return True if items' types AND values are equal, False otherwise
 */
bool LoggerTableItem::operator==(const LoggerTableItem &other) const
{
    if (!isSameType(other)) {
        return false;
    }
    switch(m_type) {
    case StringItem:
        return m_str == other.m_str;
    case ValueItem:
        return m_value == other.m_value;
    case DoubleItem:
        return m_double == other.m_double;
    default:
        LOG(LOG_ERR, "LoggerTableItem type wrong\n");
        return false;
    }
}

/*!
 * \brief LoggerTableItem::operator<
 * Less-than comparison
 * \param other: Other item to compare with
 * \return True if items' types are equal and our value is smaller than other's, False otherwise
 */
bool LoggerTableItem::operator<(const LoggerTableItem &other) const
{
    if (!isSameType(other)) {
        return false;
    }
    switch(m_type) {
    case StringItem:
        return m_str < other.m_str;
    case ValueItem:
        return valueLt(m_value, other.m_value);
    case DoubleItem:
        return m_double < other.m_double;
    default:
        LOG(LOG_ERR, "LoggerTableItem type wrong\n");
        return false;
    }
}

/*!
 * \brief LoggerTableItem::operator>
 * Greater-than comparison
 * \param other: Other item to compare with
 * \return True if items' types are equal and our value is greater than other's, False otherwise
 */
bool LoggerTableItem::operator>(const LoggerTableItem &other) const
{
    if (!isSameType(other)) {
        return false;
    }
    return !(*this == other || *this < other);
}

/*!
 * \brief LoggerTableItem::isSameType
 * Check if type is same as other's
 * \param other: Other item to compare with
 * \return True if items' types are equal, False if not
 */
bool LoggerTableItem::isSameType(const LoggerTableItem &other) const
{
    return m_type == other.m_type;
}

/*!
 * \brief LoggerTableItem::valueLt
 * Comparer for Value objects
 * \param a: First value
 * \param b: Second value
 * \return True if a's value is less than b's, False otherwise
 */
bool LoggerTableItem::valueLt(const Value &a, const Value &b)
{
    if (a.type() == Value::Type::Integer && b.type() == Value::Type::Integer) {
        return a.toInt() < b.toInt();
    } else {
        return a.toDouble() < b.toDouble();
    }
}
