/*!
* \file
* \brief loggertableitem.h foo
*/

#ifndef LOGGERTABLEITEM_H_
#define LOGGERTABLEITEM_H_

#include "value.h"
#include <QString>

class LoggerTableItem {
public:
    enum Type {StringItem, ValueItem, DoubleItem};

    LoggerTableItem();
    explicit LoggerTableItem(const QString &str);
    explicit LoggerTableItem(const Value &v);
    explicit LoggerTableItem(double d);
    LoggerTableItem(const LoggerTableItem &other);

    QString toString() const;

    LoggerTableItem &operator=(const LoggerTableItem &other);
    bool operator==(const LoggerTableItem &other) const;
    bool operator<(const LoggerTableItem &other) const;
    bool operator>(const LoggerTableItem &other) const;

private:
    bool isSameType(const LoggerTableItem &other) const;
    static bool valueLt(const Value &a, const Value &b);

    Type m_type;
    QString m_str;
    Value m_value;
    double m_double;
};

#endif
