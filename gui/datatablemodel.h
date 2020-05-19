/*!
* \file
* \brief datatablemodel.h foo
*/

#ifndef DATATABLEMODEL_H
#define DATATABLEMODEL_H

#include "tableitem.h"
#include "value.h"
#include <QAbstractTableModel>
#include <unordered_map>

#define DATA_COLUMN 1
#define FILTER_COLUMN 3
#define COLUMN_COUNT 4

class DataTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool setDataFromCAN(TableItem &item, const Value &v);

    const TableItem &getData(const QModelIndex &index) const;
    Value::Type getDataType(const QModelIndex &index) const;
    void addItem(TableItem item);

    const TableItem::AliasMap &getValueDescriptions(const QModelIndex &index) const;
    bool isEnum(const QModelIndex &index) const;
    QVector<TableItem> &getItems() { return m_items; }

    void dehilightItem(TableItem &item);
    void hilightItem(TableItem &item);

    void setFilterStates(bool filterState, const QVector<QString> &keys);

signals:
    void dataChangedByUser(const QString &signalName, const Value &data);
    void filterToggledByUser(const QString &signalName, const bool &data);

private:
    QVector<TableItem> m_items;
};

#endif // DATATABLEMODEL_H
