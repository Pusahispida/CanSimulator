/*!
* \file
* \brief loggertablemodel.h foo
*/

#ifndef LOGGERTABLEMODEL_H_
#define LOGGERTABLEMODEL_H_

#include "loggertableitem.h"
#include <memory>
#include <Qt>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QMutex>
#include <QString>
#include <QVariant>
#include <QVector>

const int loggerTableModelNumCols = 5;
const std::array<const char *, loggerTableModelNumCols> loggerTableColNames = {
    "Interface",
    "Message",
    "Signal",
    "Value",
    "Time"
};

class LoggerTableModel : public QAbstractTableModel
{
public:
    typedef std::array<LoggerTableItem, loggerTableModelNumCols> Event;

    explicit LoggerTableModel(QObject *parent = Q_NULLPTR);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void sort(int column, Qt::SortOrder order);

    void logIncomingData(const Event &event);

private:
    class EventCompare;

    QVector<Event> m_events;
    QMutex m_mutex;
    bool m_sorted;
};

class LoggerTableModel::EventCompare
{
public:
    EventCompare(int column, Qt::SortOrder order);
    bool operator()(const Event &a, const Event &b) const;

private:
    int m_column;
    Qt::SortOrder m_order;
};

#endif
