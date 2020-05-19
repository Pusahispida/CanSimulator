/*!
* \file
* \brief mainwindow.cpp foo
*/

#include "canitemdelegate.h"
#include "canmessage.h"
#include "cansignal.h"
#include "cansimulatorcore.h"
#include "datatablemodel.h"
#include "logger.h"
#include "loggerwindow.h"
#include "mainwindow.h"
#include "stdstring_to_qstring.h"
#include "ui_mainwindow.h"
#include "simulatortab.h"
#include <Qt>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QDateTime>
#include <QFileDialog>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTableView>
#include <functional>
#include <unordered_map>
#include <memory>
#include <string>

/*!
 * \brief MainWindow::MainWindow
 * Create and initialize the main window
 * \param parent: Parent widget
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
    , m_loggerWindow(std::unique_ptr<LoggerWindow>(new LoggerWindow(Q_NULLPTR)))
    , m_startTime(QDateTime::currentMSecsSinceEpoch())
{
    m_ui->setupUi(this);
    fixSize();

    QSettings *settings = new QSettings(confFile(), QSettings::NativeFormat);
    m_settings = std::unique_ptr<QSettings>(settings);

    for (CANSimParams &params : readTabHistory()) {
        SimulatorTab *tab = createTab(params);
        tab->readTableFromCAN();
        tab->getTableModel()->setFilterStates(false, QVector<QString>());
        tab->rescaleCols();
    }
    setWindowTitle("CAN Simulator");
    initMenu();
    initTabs();
    updateMenu();

    connect(m_loggerWindow.get(), &LoggerWindow::closed,
            this,                 &MainWindow::loggerWindowClosed);
}

/*!
 * \brief MainWindow::~MainWindow
 * Free used memory
 */
MainWindow::~MainWindow()
{
    m_settings->beginWriteArray("tabs");
    for (int i = 0; i < m_simParams.size(); ++i) {
        const CANSimParams &params = m_simParams.at(i);
        m_settings->setArrayIndex(i);

        const QString &cfg = std::get<0>(params);
        const QString &dbc = std::get<1>(params);
        const QString &interface = std::get<2>(params);

        m_settings->setValue("cfg", cfg);
        m_settings->setValue("dbc", dbc);
        m_settings->setValue("interface", interface);
    }
    m_settings->endArray();
    delete m_ui;
}

/*!
 * brief MainWindow::initMenu
 * Connect menu actions to respective slots
 */
void MainWindow::initMenu()
{
    connect(m_ui->actionNativeMode, &QAction::toggled,
            this, &MainWindow::actionNativeModeToggled);
    connect(m_ui->actionSendTime, &QAction::toggled,
            this, &MainWindow::actionSendTimeToggled);
    connect(m_ui->actionSendUTCTime, &QAction::triggered,
            this, &MainWindow::actionSendUTCTimeTriggered);
    connect(m_ui->actionSendLocalTime, &QAction::triggered,
            this, &MainWindow::actionSendLocalTimeTriggered);
    connect(m_ui->actionNewTab, &QAction::triggered,
            this, &MainWindow::actionNewTabTriggered);
    connect(m_ui->actionLoggerWindow, &QAction::toggled,
            this, &MainWindow::actionLoggerWindowToggled);
    connect(m_ui->actionDisableAllSignals, &QAction::triggered,
            this, &MainWindow::actionDisableAllSignalsTriggered);
    connect(m_ui->actionEnableAllSignals, &QAction::triggered,
            this, &MainWindow::actionEnableAllSignalsTriggered);
}

/*!
 * \brief MainWindow::updateMenu
 * Update menu according to different menu events
 */
void MainWindow::updateMenu()
{
    SimulatorTab *tab = getCurrentTab();
    if (tab == Q_NULLPTR) {
        m_ui->menuChooseTime->setEnabled(false);
        m_ui->actionNativeMode->setEnabled(false);
        m_ui->actionSendTime->setEnabled(false);
        return;
    }
    m_ui->actionNativeMode->setEnabled(true);
    m_ui->actionSendTime->setEnabled(true);

    signallessCheckAction(m_ui->actionNativeMode, tab->getNativeMode());
    signallessCheckAction(m_ui->actionSendTime, tab->getSendTime());

    m_ui->menuChooseTime->setEnabled(tab->getSendTime());
    signallessCheckAction(m_ui->actionSendUTCTime, tab->getTimeZone() == Qt::UTC);
    signallessCheckAction(m_ui->actionSendLocalTime, tab->getTimeZone() == Qt::LocalTime);
}

/*!
 * \brief MainWindow::initTabs
 * Connect tab widget signals to respective slots
 */
void MainWindow::initTabs()
{
    connect(m_ui->tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::tabChanged);
    connect(m_ui->tabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::tabCloseRequested);
}

/*!
 * \brief MainWindow::closeEvent
 * Close logger window when main window closes
 * \param event: Description of the close event
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    m_loggerWindow.reset();
    event->accept();
}

/*!
 * \brief MainWindow::signallessCheckAction
 * Check or uncheck a menu action without triggering a signal
 * \param action: Pointer to the menu action
 * \param checked: If it should be checked or not
 */
void MainWindow::signallessCheckAction(QAction *action, bool checked)
{
    bool oldBlocked = action->blockSignals(true);
    action->setChecked(checked);
    action->blockSignals(oldBlocked);
}

/*!
 * \brief MainWindow::tabCloseRequested
 * Slot for the close tab event
 * \param index: Which tab was requested to be closed
 */
void MainWindow::tabCloseRequested(int index)
{
    QTabWidget *tabs = m_ui->tabWidget;
    SimulatorTab *tab = static_cast<SimulatorTab *>(tabs->widget(index));
    tabs->removeTab(index);
    delete tab;

    m_simParams.erase(m_simParams.begin() + index);
}

/*!
 * \brief MainWindow::loggerWindowClosed
 * Slot for the logger window closing (uncheck logger window from menu)
 */
void MainWindow::loggerWindowClosed()
{
    signallessCheckAction(m_ui->actionLoggerWindow, false);
}

/*!
 * \brief MainWindow::actionNativeModeToggled
 * Switch native mode off or on in CAN simulator
 * \param checked: Whether we should start to use native mode
 */
void MainWindow::actionNativeModeToggled(bool checked)
{
    SimulatorTab *tab = getCurrentTab();
    Q_ASSERT(tab != Q_NULLPTR);

    tab->setNativeMode(checked);
    tab->getCANSimulatorCore()->setUseNativeUnits(checked);
    updateMenu();
}

/*!
 * \brief MainWindow::actionSendTimeToggled
 * Start or stop sending time to SUT
 * \param checked: Whether we should send time
 */
void MainWindow::actionSendTimeToggled(bool checked)
{
    SimulatorTab *tab = getCurrentTab();
    Q_ASSERT(tab != Q_NULLPTR);

    tab->setSendTime(checked);
    tab->getCANSimulatorCore()->setSendTime(checked);
    updateMenu();
}

/*!
 * \brief MainWindow::actionSendUTCTimeTriggered
 * Switch to use UTC time in CAN simulator core
 */
void MainWindow::actionSendUTCTimeTriggered()
{
    SimulatorTab *tab = getCurrentTab();
    Q_ASSERT(tab != Q_NULLPTR);

    tab->setTimeZone(Qt::UTC);
    tab->getCANSimulatorCore()->setUseUTCTime(true);
    updateMenu();
}

/*!
 * \brief MainWindow::actionSendLocalTimeTriggered
 * Switch to use local time in CAN simulator core
 */
void MainWindow::actionSendLocalTimeTriggered()
{
    SimulatorTab *tab = getCurrentTab();
    Q_ASSERT(tab != Q_NULLPTR);

    tab->setTimeZone(Qt::LocalTime);
    tab->getCANSimulatorCore()->setUseUTCTime(false);
    updateMenu();
}

/*!
 * \brief MainWindow::actionNewTabTriggered
 * Slot for creating a new tab
 */
void MainWindow::actionNewTabTriggered()
{
    QString cfg, dbc, interface;
    auto wasCancelPressed = [](const QString &rv) -> bool {
        return rv.isNull() || rv.isEmpty();
    };

    cfg = QFileDialog::getOpenFileName(this, "Choose CFG file", cfg, "*.cfg");
    if (wasCancelPressed(cfg)) {
        return;
    }
    dbc = QFileDialog::getOpenFileName(this, "Choose DBC file", dbc, "*.dbc");
    if (wasCancelPressed(dbc)) {
        return;
    }
    interface = QInputDialog::getText(this,
                                      "Choose CAN interface",
                                      "Please choose CAN interface",
                                      QLineEdit::Normal,
                                      interface);

    if (wasCancelPressed(interface)) {
        return;
    }

    CANSimParams params(cfg, dbc, interface);
    try {
        SimulatorTab *tab = createTab(params, true);
        tab->readTableFromCAN();
        tab->rescaleCols();

    // Hide the exception since createCANSimulatorCore() already shows a
    // message box if something goes wrong
    } catch (CANSimulatorCoreException &) {}
}

/*!
 * \brief MainWindow::actionLoggerWindowToggled
 * Slot for showing/hiding the logger window
 * \param checked: Whether the action is now checked or not
 */
void MainWindow::actionLoggerWindowToggled(bool checked)
{
    m_loggerWindow->setVisible(checked);
}

/*!
 * \brief MainWindow::actionDisableAllSignalsTriggered
 * Disable all signals for current tab
 */
void MainWindow::actionDisableAllSignalsTriggered()
{
    SimulatorTab *tab = getCurrentTab();
    Q_ASSERT(tab != Q_NULLPTR);
    CANSimulatorCore *sim = tab->getCANSimulatorCore();
    Q_ASSERT(sim != Q_NULLPTR);

    if (sim->initializeMessageFilterList(NULL, true)) {
        DataTableModel *model = tab->getTableModel();
        Q_ASSERT(model != Q_NULLPTR);
        model->setFilterStates(true, QVector<QString>());
    }
}

/*!
 * \brief MainWindow::actionEnableAllSignalsTriggered
 * Enable all signals for current tab
 */
void MainWindow::actionEnableAllSignalsTriggered()
{
    SimulatorTab *tab = getCurrentTab();
    Q_ASSERT(tab != Q_NULLPTR);
    CANSimulatorCore *sim = tab->getCANSimulatorCore();
    Q_ASSERT(sim != Q_NULLPTR);

    if (sim->initializeMessageFilterList(NULL, false)) {
        DataTableModel *model = tab->getTableModel();
        Q_ASSERT(model != Q_NULLPTR);
        model->setFilterStates(false, QVector<QString>());
    }
}

/*!
 * \brief MainWindow::tabChanged
 * Slot for tab change, update menu to reflect current tab
 * \param: Unused (new tab index)
 */
void MainWindow::tabChanged(int)
{
    updateMenu();
}

/*!
 * \brief MainWindow::getCurrentTab
 * Get pointer to the currently visible tab
 * \return Current tab
 */
SimulatorTab *MainWindow::getCurrentTab()
{
    return static_cast<SimulatorTab *>(m_ui->tabWidget->currentWidget());
}

/*!
 * \brief MainWindow::createCANSimulatorCore
 * Create a new CAN simulator core
 * \param params: The cfg file, dbc file and interface for new simulator
 * \return The created CAN simulator core
 */
std::unique_ptr<CANSimulatorCore> MainWindow::createCANSimulatorCore(const CANSimParams &params)
{
    std::unique_ptr<CANSimulatorCore> simulator;
    const QString &cfg = std::get<0>(params);
    const QString &dbc = std::get<1>(params);
    const QString &interface = std::get<2>(params);

    // If CANSimulatorCore constructor throws an exception, let main() handle
    // that instead of us
    try {
        auto tmp = new CANSimulatorCore(cfg.toStdString(),
                                        dbc.toStdString(),
                                        std::string(),
                                        interface.toStdString(),
                                        false,
                                        false);

        simulator = std::unique_ptr<CANSimulatorCore>(tmp);

    } catch (CANSimulatorCoreException &) {
        QMessageBox::critical(this,
                              "Failed to initialize CAN simulator",
                              "Could not initialize the CAN simulator core, please check that your DBC, CFG and interface are correctly configured");
        throw;
    }
    simulator->startCANReaderThread();
    simulator->startCANSenderThread();
    return simulator;
}

/*!
 * \brief MainWindow::readTabHistory
 * Read tab history from m_settings
 * \return List of tab parameters from the last time
 */
QVector<MainWindow::CANSimParams> MainWindow::readTabHistory()
{
    int size = m_settings->beginReadArray("tabs");
    QVector<CANSimParams> result(size);

    for (int i = 0; i < size; ++i) {
        m_settings->setArrayIndex(i);
        QString cfg = m_settings->value("cfg").toString();
        QString dbc = m_settings->value("dbc").toString();
        QString interface = m_settings->value("interface").toString();
        result[i] = CANSimParams(cfg, dbc, interface);
    }
    m_settings->endArray();
    return result;
}

/*!
 * \brief MainWindow::createTab
 * Create a new CAN simulator tab
 * \param params: Parameters (cfg file, dbc file and interface) for the sim
 * \param switchToNew: Whether to switch to the newly created tab
 * \return Pointer to the created tab
 */
SimulatorTab *MainWindow::createTab(const MainWindow::CANSimParams &params,
                                    bool switchToNew)
{
    const QString &interface = std::get<2>(params);
    std::unique_ptr<CANSimulatorCore> simulator = createCANSimulatorCore(params);
    simulator->initializeMessageFilterList(NULL, false);

    SimulatorTab *tab = new SimulatorTab();
    int index = m_ui->tabWidget->addTab(tab, interface);
    m_simParams.push_back(params);

    if (switchToNew) {
        m_ui->tabWidget->setCurrentIndex(index);
    }
    std::unique_ptr<QTableView> tableView(new QTableView(tab));
    std::unique_ptr<DataTableModel> tableModel(new DataTableModel());
    tableView->setModel(tableModel.get());

    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(tableView.get());

    tableView->setItemDelegate(new CANItemDelegate(this));
    tableView->horizontalHeader()->hide();
    tableView->verticalHeader()->hide();
    tableView->show();
    connect(tableModel.get(), &DataTableModel::dataChangedByUser,
            tab,              &SimulatorTab::dataChangedByUser);

    connect(tableModel.get(), &DataTableModel::filterToggledByUser,
            tab,              &SimulatorTab::filterToggledByUser);

    connect(tab,                  &SimulatorTab::logIncomingData,
            m_loggerWindow.get(), &LoggerWindow::logIncomingData);

    tab->setCANSimulatorCore(simulator);
    tab->setTableView(tableView);
    tab->setTableModel(tableModel);
    tab->setInterfaceName(std::get<2>(params));
    tab->setStartTime(m_startTime);

    return tab;
}

/*!
 * \brief MainWindow::fixSize
 * Disallow modifying window size, there seems to be no static way to do it in
 * UI Designer
 */
void MainWindow::fixSize()
{
    const int w = width(), h = height();
    setMaximumWidth(w);
    setMinimumWidth(w);
    setMaximumHeight(h);
    setMinimumHeight(h);
}

/*!
 * \brief MainWindow::resizeEvent
 * Rescale table when window is resized
 * \param event: Information about the event
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    for (TabIterator it(m_ui->tabWidget); !it.isIterated(); ++it) {
        it->rescaleCols();
    }
}

/*!
 * \brief MainWindow::TabIterator::TabIterator
 * Construct a tab iterator, starting from index 0
 * \param tabs: The tab widget to be iterated over
 */
MainWindow::TabIterator::TabIterator(const QTabWidget *tabs)
    : m_tabs(tabs)
    , m_index(0)
{}

/*!
 * \brief MainWindow::TabIterator::operator->
 * Return a pointer to the current tab
 * \return Current tab
 */
SimulatorTab *MainWindow::TabIterator::operator->()
{
    return static_cast<SimulatorTab *>(m_tabs->widget(m_index));
}

/*!
 * \brief MainWindow::TabIterator::operator++
 * Prefix increment; note that this is not error checked, remember to always
 * check isIterated()
 * \return Incremented iterator
 */
MainWindow::TabIterator &MainWindow::TabIterator::operator++()
{
    ++m_index;
    return *this;
}

/*!
 * \brief MainWindow::TabIterator::isIterated
 * Check if we've gone through the tabs already
 * \return Whether we're ready or not
 */
bool MainWindow::TabIterator::isIterated() const
{
    return m_index >= m_tabs->count();
}
