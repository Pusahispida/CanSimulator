/*!
* \file
* \brief mainwindow.h foo
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "canmessage.h"
#include "cansimulatorcore.h"
#include "datatablemodel.h"
#include "loggerwindow.h"
#include "simulatortab.h"
#include <memory>
#include <tuple>
#include <QMainWindow>
#include <QVector>
#include <QWidget>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class SimulatorTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class SimulatorTab;
    typedef std::tuple<QString, QString, QString> CANSimParams;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void resizeEvent(QResizeEvent *event);
    void tabCloseRequested(int index);
    void loggerWindowClosed();

    void actionNativeModeToggled(bool checked);
    void actionSendTimeToggled(bool checked);
    void actionSendUTCTimeTriggered();
    void actionSendLocalTimeTriggered();
    void actionNewTabTriggered();
    void actionLoggerWindowToggled(bool checked);
    void actionDisableAllSignalsTriggered();
    void actionEnableAllSignalsTriggered();

    void tabChanged(int);

private:
    // Is there a better way do consts in class scope?
    static const char *mwTitle()        { return "CAN simulator"; }
    static const char *confFile()       { return "./gui.conf"; }

    class TabIterator;

    Ui::MainWindow *m_ui;
    std::unique_ptr<QSettings> m_settings;
    QVector<CANSimParams> m_simParams;
    std::unique_ptr<LoggerWindow> m_loggerWindow;
    qint64 m_startTime;

    /*
     * GUI-related functions
     */
    void fixSize();
    void initMenu();
    void updateMenu();
    void initTabs();
    void closeEvent(QCloseEvent *event);
    static void signallessCheckAction(QAction *action, bool checked);

    QVector<CANSimParams> readTabHistory();
    SimulatorTab *createTab(const CANSimParams &params,
                            bool switchToNew = false);

    SimulatorTab *getCurrentTab();
    std::unique_ptr<CANSimulatorCore> createCANSimulatorCore(const CANSimParams &params);
};

class MainWindow::TabIterator
{
public:
    explicit TabIterator(const QTabWidget *tabs);
    SimulatorTab *operator->();
    TabIterator &operator++();
    bool isIterated() const;

private:
    const QTabWidget *m_tabs;
    int m_index;
};

#endif // MAINWINDOW_H
