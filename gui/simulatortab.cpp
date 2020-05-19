/*!
* \file
* \brief simulatortab.cpp foo
*/

#include "cansimulatorcore.h"
#include "loggertablemodel.h"
#include "mainwindow.h"
#include "simulatortab.h"
#include <map>
#include <QDateTime>
#include <QHeaderView>

/*!
 * \brief SimulatorTab::SimulatorTab
 * Construct a simulator tab with default values
 */
SimulatorTab::SimulatorTab(QWidget *parent)
    : QWidget(parent)
    , m_canSimulatorCore(Q_NULLPTR)
    , m_readSignals(new QTimer(this))
    , m_sendTime(false)
    , m_timeZone(Qt::LocalTime)
    , m_nativeMode(false)
    , m_startTime(0)
{
    connect(m_readSignals.get(), &QTimer::timeout,
            this,                &SimulatorTab::pollCAN);

    m_readSignals->start(20);
}

/*!
 * \brief SimulatorTab::rescaleCols
 * Set all table columns to equal width
 */
void SimulatorTab::rescaleCols()
{
    /*
     * Resize unit column to be as narrow as possible and others to fill the
     * available room
     */
    QHeaderView *hHeader = m_tableView->horizontalHeader();
    hHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    hHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableView->resizeColumnToContents(2);
}

/*!
 * \brief SimulatorTab::setCANSimulatorCore
 * Setter for CAN simulator core
 * \param canSimulatorCore: CAN simulator core to use in this tab
 */
void SimulatorTab::setCANSimulatorCore(std::unique_ptr<CANSimulatorCore> &canSimulatorCore)
{
    m_canSimulatorCore = std::move(canSimulatorCore);
}

/*!
 * \brief SimulatorTab::setTableView
 * Setter for table view element
 * \param tableView: new table view element
 */
void SimulatorTab::setTableView(std::unique_ptr<QTableView> &tableView)
{
    m_tableView = std::move(tableView);
}

/*!
 * \brief SimulatorTab::setTableModel
 * Setter for table model
 * \param tableModel: new table model
 */
void SimulatorTab::setTableModel(std::unique_ptr<DataTableModel> &tableModel)
{
    m_tableModel = std::move(tableModel);
}

/*!
 * \brief SimulatorTab::setSendTime
 * Select whether to send time over CAN
 * \param sendTime: to send time or not
 */
void SimulatorTab::setSendTime(bool sendTime)
{
    m_sendTime = sendTime;
}

/*!
 * \brief SimulatorTab::setTimeZone
 * Setter for time zone
 * \param timeZone: preferred time zone
 */
void SimulatorTab::setTimeZone(Qt::TimeSpec timeZone)
{
    m_timeZone = timeZone;
}

/*!
 * \brief SimulatorTab::setNativeMode
 * Setter for native mode
 * \param nativeMode: whether to use native mode or not
 */
void SimulatorTab::setNativeMode(bool nativeMode)
{
    m_nativeMode = nativeMode;
}

/*!
 * \brief SimulatorTab::setInterfaceName
 * Setter for CAN interface name
 * \param interfaceName: Name of the interface
 */
void SimulatorTab::setInterfaceName(const QString &interfaceName)
{
    m_interfaceName = interfaceName;
}

/*!
 * \brief SimulatorTab::setStartTime
 * Setter for system start time
 * \param startTime: Start time of the system
 */
void SimulatorTab::setStartTime(qint64 startTime)
{
    m_startTime = startTime;
}

/*!
 * \brief SimulatorTab::getCANSimulatorCore
 * Getter for the CAN simulator core in use in this tab
 * \return The used simulator core
 */
CANSimulatorCore *SimulatorTab::getCANSimulatorCore()
{
    return m_canSimulatorCore.get();
}

/*!
 * \brief SimulatorTab::getTableView
 * Getter for the table view element
 * \return The used table view
 */
QTableView *SimulatorTab::getTableView()
{
    return m_tableView.get();
}

/*!
 * \brief SimulatorTab::getTableModel
 * Getter for the table model
 * \return The used table model
 */
DataTableModel *SimulatorTab::getTableModel()
{
    return m_tableModel.get();
}

/*!
 * \brief SimulatorTab::getSendTime
 * Check if we're sending time in the CAN simulator core
 * \return True if sending time, False if not
 */
bool SimulatorTab::getSendTime() const
{
    return m_sendTime;
}

/*!
 * \brief SimulatorTab::getTimeZone
 * Getter for time zone
 * \return Time zone in use
 */
Qt::TimeSpec SimulatorTab::getTimeZone() const
{
    return m_timeZone;
}

/*!
 * \brief SimulatorTab::getNativeMode
 * Getter for native mode
 * \return True if Native mode is in use, False if not
 */
bool SimulatorTab::getNativeMode() const
{
    return m_nativeMode;
}

/*!
 * \brief SimulatorTab::getInterfaceName
 * Getter for CAN interface name
 * \return CAN interface name used by the core
 */
const QString &SimulatorTab::getInterfaceName() const
{
    return m_interfaceName;
}

/*!
 * \brief SimulatorTab::getStartTime
 * Getter for system start time
 * \return Start time of the system
 */
qint64 SimulatorTab::getStartTime() const
{
    return m_startTime;
}

/*!
 * \brief SimulatorTab::dataChangedByUser
 * User changed the CAN state, forward change to CAN library
 * \param key: The variable name that was changed
 * \param value: New value
 */
void SimulatorTab::dataChangedByUser(const QString &key, const Value &value)
{
    m_canSimulatorCore->setValue(key.toStdString(), value);
    LOG(LOG_DBG, "Signal %s changed to %s\n",
                 key.toLatin1().data(),
                 value.toString());
}

/*!
 * \brief SimulatorTab::filterToggledByUser
 * User toggled filter, forward change to CAN library and update table if successful
 * \param key: The variable name that was changed
 * \param value: New value for filter
 */
void SimulatorTab::filterToggledByUser(const QString &key, const bool &value)
{
    uint32_t id = m_canSimulatorCore->getMessage(key.toStdString())->getId();
    if (m_canSimulatorCore->setMessageFilterState(id, value)) {
        const std::set<std::string> &signalNames = m_canSimulatorCore->getVariables();
        QVector<QString> keys;
        for (auto i = signalNames.cbegin(); i != signalNames.cend(); ++i) {
            const CANMessage *msg = m_canSimulatorCore->getMessage(*i);
            if (msg->getId() == id) {
                keys.append(qsFromSs(*i));
                m_tableModel->setFilterStates(value, keys);
                LOG(LOG_DBG, "Filter toggled %s changed to %i\n",
                             i->c_str(),
                             value);
            }
        }
    }
}

/*!
 * \brief SimulatorTab::readTableFromCAN
 * Get variable list from CAN and create items accordingly to the table
 */
void SimulatorTab::readTableFromCAN()
{
    const std::set<std::string> &signalNames = m_canSimulatorCore->getVariables();
    for(auto i = signalNames.cbegin(); i != signalNames.cend(); ++i) {
        const CANSignal *signal = m_canSimulatorCore->getSignal(*i);
        m_tableModel->addItem(TableItem(signal));
    }
}

/*!
 * \brief SimulatorTab::modifyTableFromMessage
 * Modify table according to received message
 * \param message: Received CAN message
 */
void SimulatorTab::modifyTableFromMessage(std::shared_ptr<CANMessage> msg)
{
    const std::map<std::string, CANSignal> &sigs = msg->getSignals();
    QVector<TableItem> &items = m_tableModel->getItems();

    for (auto it = items.begin(); it != items.end(); ++it) {
        const auto sig_it = sigs.find(it->getSignal()->getName());

        if (sig_it != sigs.end()) {
            TableItem &item = *it;
            const std::string &sigName = sig_it->first;
            const CANSignal &sig = sig_it->second;
            const Value &v = sig.getValue();

            bool wasChanged = m_tableModel->setDataFromCAN(item, v);
            if (wasChanged) {
                m_tableModel->hilightItem(item);
                logChange(msg->getName(), sigName, v);
            }
        }
    }
}

/*!
 * \brief SimulatorTab::pollCAN
 * Check CAN socket and read incoming messages, if any
 */
void SimulatorTab::pollCAN()
{
    if (m_canSimulatorCore != nullptr) {
        Queue<std::shared_ptr<CANMessage>> *messageQueue = m_canSimulatorCore->getMessageQueue();
        while (!messageQueue->empty()) {
            modifyTableFromMessage(messageQueue->pop());
        }
    }
}

/*!
 * \brief SimulatorTab::logChange
 * Notify logger window about a received signal (ie. a variable changes value)
 * \param msgName: Message name
 * \param sigName: Signal name
 * \param value: The new value
 */
void SimulatorTab::logChange(const std::string &msgName,
                             const std::string &sigName,
                             const Value &value)
{
    double dtime = (double)(QDateTime::currentMSecsSinceEpoch() - m_startTime) * .001f;
    Value time(dtime);

    LoggerTableModel::Event event = {
        LoggerTableItem(m_interfaceName),
        LoggerTableItem(qsFromSs(msgName)),
        LoggerTableItem(qsFromSs(sigName)),
        LoggerTableItem(value),
        LoggerTableItem(time)
    };
    emit logIncomingData(event);
}
