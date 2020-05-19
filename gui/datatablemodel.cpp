/*!
* \file
* \brief datatablemodel.cpp foo
*/

#include "datatablemodel.h"
#include "logger.h"
#include "stdstring_to_qstring.h"
#include "tableitem.h"
#include "value.h"
#include <functional>
#include <QAbstractTableModel>
#include <QColor>
#include <QTimer>

const int highlightTime = 1000;

/*!
 * \brief DataTableModel::rowCount
 * How many rows in the table
 * \param parent (unused): Under which to count rows
 * \return Number of items
 */
int DataTableModel::rowCount(const QModelIndex &) const
{
    return m_items.length();
}

/*!
 * \brief DataTableModel::columnCount
 * How many columns in the table
 * \param parent (unused): Next to which to count columns
 * \return Number of columns
 */
int DataTableModel::columnCount(const QModelIndex &) const
{
    return COLUMN_COUNT;
}

/*!
 * \brief DataTableModel::data
 * Get table item according to its column, for rendering use by Qt
 * \param index: Where to fetch data from
 * \param role: Which data are we looking for
 * \return The requested data
 */
QVariant DataTableModel::data(const QModelIndex &index, int role) const
{
    const TableItem &item = m_items.at(index.row());

    if (role == Qt::BackgroundColorRole) {
        if (item.isMessageFiltered()) {
            return QColor(Qt::lightGray);
        } else if (index.column() == DATA_COLUMN && item.getHighlighted()) {
            return QColor(Qt::red);
        } else {
            return QVariant();
        }
    } else if (role == Qt::DisplayRole) {
        switch(index.column()) {
        case 0:
            return item.getName();
            break;
        case 1:
            return item.uiRepresentation();
            break;
        case 2:
            return item.getUnit();
            break;
        case 3:
            return QVariant();
            break;
        default:
            LOG(LOG_ERR, "Requested for a column not in table!\n");
            return QVariant();
        }
    } else if (role == Qt::CheckStateRole) {
        if (index.column() == FILTER_COLUMN) {
            if (item.isMessageFiltered()) {
                return Qt::Checked;
            } else {
                return Qt::Unchecked;
            }
        } else {
            return QVariant();
        }
    } else {
        return QVariant();
    }
}

/*!
 * \brief DataTableModel::getData
 * Get the actual data contained in the table
 * \param index: Where to fetch data from
 * \return Wanted data at index
 */
const TableItem &DataTableModel::getData(const QModelIndex &index) const
{
    static const TableItem empty;
    if (index.column() != DATA_COLUMN) {
        LOG(LOG_ERR, "Not a data column at DataTableModel::getData()!\n");
        return empty;
    }
    return m_items.at(index.row());
}

/*!
 * \brief DataTableModel::getDataType
 * A bit simplified way to get data type for a given index
 * \param index: Where to fetch data type from
 * \return Type of data at index
 */
Value::Type DataTableModel::getDataType(const QModelIndex &index) const
{
    return getData(index).getValue().type();
}

/*!
 * \brief DataTableModel::flags
 * Get flags (data column editable, name and unit columns not) for Qt usage
 * \param index: Where to fetch flags from
 * \return Flags for the cell
 */
Qt::ItemFlags DataTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (index.column() == DATA_COLUMN) {
        flags |= Qt::ItemIsEditable | Qt::ItemIsSelectable;
    } else if (index.column() == FILTER_COLUMN) {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
        flags ^= Qt::ItemIsSelectable;
    } else {
        flags &= ~(Qt::ItemIsSelectable);
    }
    return flags;
}

/*!
 * \brief DataTableModel::setData
 * Modify table contents after user has edited it, do not call when new data
 * arrives from CAN!
 * \param index: Which cell to modify
 * \param value: New value for cell
 * \param role: What role was edited (only EditRole supported)
 * \return True on success, False on failure
 */
bool DataTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() == DATA_COLUMN) {
        if (role == Qt::EditRole) {
            Value newValue;
            volatile int row = index.row();
            TableItem &item = m_items[row];

            switch(item.getValue().type()) {
            case Value::Integer:

                // See if there's a corresponding CAN value for the alias given
                if (item.isEnum()) {
                    const TableItem::AliasMap &vds = item.getValueDescriptions();
                    const std::string key = value.toString().toStdString();

                    for (auto it = vds.cbegin(); it != vds.cend(); ++it) {
                        if (it->second == key) {
                            newValue = Value((int)it->first);
                            break;
                        }
                    }
                } else {
                    newValue = Value(value.toInt());
                }
                break;

            case Value::Double:
                newValue = Value(value.toDouble());
                break;

            default:
                LOG(LOG_ERR, "Unknown type %s\n", item.getValue().type());
                return false;
            }
            item.setValue(newValue);
            emit dataChangedByUser(item.getName(), newValue);
            emit dataChanged(index, index);
            return true;

        } else {
            LOG(LOG_ERR, "Tried to modify table values not in data column!\n");
            return false;
        }
    } else if (index.column() == FILTER_COLUMN) {
        volatile int row = index.row();
        TableItem &item = m_items[row];
        emit filterToggledByUser(item.getName(), !item.isMessageFiltered());
        return true;
    // Nothing other than the data and filter column should be editable, so resort to
    // default behavior and return false.
    } else {
        LOG(LOG_ERR, "DataTableModel::setData called outside of data and filter columns!");
        return false;
    }
}

/*!
 * \brief DataTableModel::setDataFromCAN
 * Modify table data according to CAN state change
 * \param item: Which item was modified
 * \param value: New value
 * \return True if data was changed, False otherwise
 */
bool DataTableModel::setDataFromCAN(TableItem &item, const Value &value)
{
    if (item.setValueIfModified(value)) {
        const QModelIndex &index = item.getIndex();
        emit dataChanged(index, index);
        return true;
    } else {
        return false;
    }
}

/*!
 * \brief DataTableModel::addItem
 * Add a new item to table
 * \param item: Item to be added
 */
void DataTableModel::addItem(TableItem item) {
    item.setIndex(QAbstractTableModel::index(rowCount(), DATA_COLUMN));
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_items.push_back(item);
    endInsertRows();
}

/*!
 * \brief DataTableModel::getValueDescriptions
 * Get value aliases for CAN variable at index
 * \param index: Where to fetch value descriptions from
 * \return List of value descriptions
 */
const TableItem::AliasMap &DataTableModel::getValueDescriptions(const QModelIndex &index) const
{
    return m_items.at(index.row()).getValueDescriptions();
}

/*!
 * \brief DataTableModel::isEnum
 * Check if the item at index is an enum
 * \param index: Which item to check
 * \return True if it is enum, False if not
 */
bool DataTableModel::isEnum(const QModelIndex &index) const
{
    return m_items.at(index.row()).isEnum();
}

/*!
 * \brief DataTableModel::hilightItem
 * Highlights an item and sets a timer for dehilighting
 * \param item: Which item to hilight
 */
void DataTableModel::hilightItem(TableItem &item)
{
    const QModelIndex &index = item.getIndex();
    QVector<int> roles(1);

    roles[0] = Qt::BackgroundColorRole;
    item.setHighlighted(true);
    emit dataChanged(index, index, roles);

    QTimer::singleShot(highlightTime,
                       [this, &item] () {
                           dehilightItem(item);
                       });
}

/*!
 * \brief DataTableModel::dehilightItem
 * Slot for dehilighting an item after timer timeout
 * \param item: Which item to dehilight
 */
void DataTableModel::dehilightItem(TableItem &item)
{
    const QModelIndex &index = item.getIndex();
    QVector<int> roles(1);

    roles[0] = Qt::BackgroundColorRole;
    item.setHighlighted(false);
    emit dataChanged(index, index, roles);
}

/*!
 * \brief DataTableModel::setFilterState
 * Enable or disable all signals of a message
 * \param filterState: New state of filtering
 * \param keys: signal names to set filter, if empty then filter setting will be applied to all signals
 * \return True if successful, false otherwise.
 */
void DataTableModel::setFilterStates(bool filterState, const QVector<QString> &keys)
{
    int i;
    for (i = 0; i < m_items.length(); i++) {
        TableItem &item = m_items[i];
        if (keys.isEmpty() || (keys.contains(item.getName()) && item.isMessageFiltered() != filterState)) {
            const QModelIndex &index = item.getIndex();
            item.setMessageFilterState(filterState);
            emit dataChanged(index.sibling(index.row(), 0), index.sibling(index.row(), DATA_COLUMN), QVector<int>(Qt::BackgroundColorRole));
        }
    }
}
