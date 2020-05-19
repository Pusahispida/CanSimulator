/*!
* \file
* \brief simulatortab.h foo
*/

#ifndef SIMULATORTAB_H_
#define SIMULATORTAB_H_

#include "cansimulatorcore.h"
#include "datatablemodel.h"
#include "loggertablemodel.h"
#include "mainwindow.h"
#include <Qt>
#include <QString>
#include <QTableView>
#include <QTimer>
#include <QVector>
#include <QWidget>

class SimulatorTab : public QWidget
{
    Q_OBJECT

public:
    explicit SimulatorTab(QWidget *parent = Q_NULLPTR);

    void zeroInitVars();
    void setCANSimulatorCore(std::unique_ptr<CANSimulatorCore> &canSimulatorCore);
    void setTableView(std::unique_ptr<QTableView> &tableView);
    void setTableModel(std::unique_ptr<DataTableModel> &tableModel);

    void setSendTime(bool sendTime);
    void setTimeZone(Qt::TimeSpec timeZone);
    void setNativeMode(bool nativeMode);
    void setInterfaceName(const QString &interfaceName);
    void setStartTime(qint64 startTime);

    CANSimulatorCore *getCANSimulatorCore();
    QTableView *getTableView();
    DataTableModel *getTableModel();

    bool getSendTime() const;
    Qt::TimeSpec getTimeZone() const;
    bool getNativeMode() const;
    const QString &getInterfaceName() const;
    qint64 getStartTime() const;

    void readTableFromCAN();
    void rescaleCols();

public slots:
    void dataChangedByUser(const QString &key, const Value &value);
    void filterToggledByUser(const QString &key, const bool &value);

signals:
    void logIncomingData(const LoggerTableModel::Event &event);

private:
    void modifyTableFromMessage(std::shared_ptr<CANMessage> msg);
    void pollCAN();
    void logChange(const std::string &msgName,
                   const std::string &sigName,
                   const Value &v);

    std::unique_ptr<CANSimulatorCore> m_canSimulatorCore;
    std::unique_ptr<QTableView> m_tableView;
    std::unique_ptr<DataTableModel> m_tableModel;
    std::unique_ptr<QTimer> m_readSignals;

    bool m_sendTime;
    Qt::TimeSpec m_timeZone;
    bool m_nativeMode;
    QString m_interfaceName;
    qint64 m_startTime;
};

#endif
