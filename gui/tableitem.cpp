/*!
* \file
* \brief tableitem.cpp foo
*/

#include "stdstring_to_qstring.h"
#include "tableitem.h"
#include "value.h"
#include "cansignal.h"
#include <QString>

/*!
 * \brief TableItem::TableItem
 * Constructor
 * \param signal: The CAN signal this item represents
 */
TableItem::TableItem(const CANSignal *signal)
    : m_signal(signal)
    , m_filtered(false)
    , m_highlighted(false)
{
}

/*!
 * \brief TableItem::setValueIfModified
 * Change value if new one differs from old one
 * \param value: New value
 * \return True if new value differed from old, False if not
 */
bool TableItem::setValueIfModified(const Value &value)
{
    if (m_value != value) {
        m_value = value;
        return true;
    } else {
        return false;
    }
}

/*!
 * \brief TableItem::uiRepresentation
 * How to represent the table item's value as a string
 * \return Alias for the value for enums, value cast as string otherwise
 */
QString TableItem::uiRepresentation() const
{
    const Value &value = getValue();
    if (isEnum()) {
        const AliasMap &valueDescriptions = getValueDescriptions();
        const auto it = valueDescriptions.find(value.toInt());

        if (it != valueDescriptions.end()) {
            return qsFromSs(it->second);
        }
    }
    return qsFromSs(value.toString());
}
