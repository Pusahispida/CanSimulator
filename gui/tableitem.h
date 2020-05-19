/*!
* \file
* \brief tableitem.h foo
*/

#ifndef TABLEITEM_H_
#define TABLEITEM_H_

#include "cansignal.h"
#include "stdstring_to_qstring.h"
#include "value.h"
#include <QModelIndex>
#include <QString>
#include <map>

class TableItem
{
public:
    typedef std::map<unsigned int, std::string> AliasMap;

    explicit TableItem(const CANSignal *signal = Q_NULLPTR);

    QString getName() const { return qsFromSs(m_signal->getVariableName()); }
    QString getUnit() const { return qsFromSs(m_signal->getUnit()); }

    const CANSignal *getSignal(void) const { return m_signal; }
    const QModelIndex &getIndex() const { return m_index; }
    bool getHighlighted(void) const { return m_highlighted; }
    const Value &getValue() const { return m_value; }
    const AliasMap &getValueDescriptions() const { return m_signal->getValueDescriptions(); }
    bool isMessageFiltered() const { return m_filtered; }

    void setSignal(const CANSignal *signal) { m_signal = signal; }
    void setIndex(const QModelIndex &index) { m_index = index; }
    void setHighlighted(bool highlighted) { m_highlighted = highlighted; }
    void setValue(const Value &value) { m_value = value; }
    bool setValueIfModified(const Value &value);
    void setMessageFilterState(bool filter) { m_filtered = filter; }

    bool isEnum() const { return !getValueDescriptions().empty(); }
    QString uiRepresentation() const;

private:
    const CANSignal *m_signal;
    QModelIndex m_index;
    bool m_filtered;
    bool m_highlighted;
    Value m_value;
};

#endif // TABLEITEM_H_
