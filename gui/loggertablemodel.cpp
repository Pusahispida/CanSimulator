/*!
* \file
* \brief loggertablemodel.cpp foo
* any other legal text to be defined later
*/

#include "loggertablemodel.h"
#include <algorithm>
#include <functional>
#include <Qt>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QMutex>
#include <QMutexLocker>
#include <QVariant>

/*!
 * \brief LoggerTableModel::LoggerTableModel
 * Construct a logger table model
 * \param parent: Parent object
 */
LoggerTableModel::LoggerTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_sorted(true)
{
}

/*!
 * \brief LoggerTableModel::rowCount
 * Get row count
 * \param parent: Parent index
 * \return Row count
 */
int LoggerTableModel::rowCount(const QModelIndex &parent) const
{
    int parentRow = parent.isValid() ? parent.row() : 0;
    return m_events.size() - parentRow;
}

/*!
 * \brief LoggerTableModel::columnCount
 * Get column count
 * \param: Unused
 * \return Column count
 */
int LoggerTableModel::columnCount(const QModelIndex &) const
{
    return loggerTableModelNumCols;
}

/*!
 * \brief LoggerTableModel::data
 * Get cell data
 * \param index: Which cell to access
 * \param role: Which role data we're looking for
 * \return Requested data
 */
QVariant LoggerTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const Event &event = m_events.at(index.row());
        const LoggerTableItem &item = event.at(index.column());
        return QVariant(item.toString());
    } else {
        return QVariant();
    }
}

/*!
 * \brief LoggerTableModel::headerData
 * Get horizontal header data (no vertical headers used)
 * \param section: Which column's header is looked for
 * \param orientation: Horizontal or vertical
 * \param role: Which role data we're looking for
 * \return Requested data
 */
QVariant LoggerTableModel::headerData(int section,
                                      Qt::Orientation orientation,
                                      int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return QVariant(loggerTableColNames.at(section));
    } else {
        return QVariant();
    }
}
/*!
 * \brief LoggerTableModel::flags
 * Get flags (nothing selectable) for Qt usage
 * \param index: Which table cell
 * \return Flags for the cell
 */
Qt::ItemFlags LoggerTableModel::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index) &= ~(Qt::ItemIsSelectable |
                                                  Qt::ItemIsEditable);
}

/*!
 * \brief LoggerTableModel::sort
 * Sort table by column
 * \param column: By which column's values to sort
 * \param order: Whether to sort in ascending or descending order
 */
void LoggerTableModel::sort(int column, Qt::SortOrder order)
{
    QModelIndex beginIndex = QAbstractTableModel::index(0, 0);
    QModelIndex endIndex = QAbstractTableModel::index(rowCount(), columnCount());
    EventCompare cmp(column, order);

    QMutexLocker locker(&m_mutex);
    std::stable_sort(m_events.begin(), m_events.end(), cmp);
    emit dataChanged(beginIndex, endIndex);
    m_sorted = true;
}

/*!
 * \brief LoggerTableModel::logIncomingData
 * Append new data change to table
 * \param event: The data change event
 */
void LoggerTableModel::logIncomingData(const Event &event)
{
    QModelIndex itemIndexBegin = QAbstractTableModel::index(m_events.size(), 0);
    QModelIndex itemIndexEnd = QAbstractTableModel::index(m_events.size(), columnCount());

    QMutexLocker locker(&m_mutex);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_events.push_back(event);
    m_sorted = false;
    endInsertRows();

    emit dataChanged(itemIndexBegin, itemIndexEnd);
}

/*!
 * \brief LoggerTableModel::EventCompare::EventCompare
 * Construct an event comparer
 * \param column: Which column from events to compare
 * \param order: Ascending or descending
 */
LoggerTableModel::EventCompare::EventCompare(int column, Qt::SortOrder order)
    : m_column(column)
    , m_order(order)
{
}

/*!
 * \brief LoggerTableModel::EventCompare::operator()
 * Compare two events
 * \param a: First event
 * \param b: Second event
 * \return True if a should go before b (compares superior), False otherwise
 */
bool LoggerTableModel::EventCompare::operator()(const Event &a, const Event &b) const
{
    const LoggerTableItem &aData = a.at(m_column);
    const LoggerTableItem &bData = b.at(m_column);

    if (m_order == Qt::AscendingOrder) {
        return aData < bData;
    } else {
        return aData > bData;
    }
}
