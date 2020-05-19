/*!
* \file
* \brief loggerwindow.h foo
*/

#ifndef LOGGERWINDOW_H_
#define LOGGERWINDOW_H_

#include "loggertablemodel.h"
#include <memory>
#include <QDialog>
#include <QResizeEvent>
#include <QTableView>
#include <QVBoxLayout>

class LoggerWindow : public QDialog
{
    Q_OBJECT

public:
    LoggerWindow(QWidget *parent = Q_NULLPTR, const QSize &size = QSize(640, 480));

signals:
    void closed();

public slots:
    void logIncomingData(const LoggerTableModel::Event &event);

private:
    std::unique_ptr<QVBoxLayout> m_layout;
    std::unique_ptr<QTableView> m_tableView;
    std::unique_ptr<LoggerTableModel> m_tableModel;

private slots:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
};

#endif
