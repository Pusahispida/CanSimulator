/*!
* \file
* \brief loggerwindow.cpp foo
*/

#include "loggerwindow.h"
#include "loggertablemodel.h"
#include <QCloseEvent>
#include <QDialog>
#include <QHeaderView>
#include <QResizeEvent>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

/*!
 * \brief LoggerWindow::LoggerWindow
 * Constructor
 * \param parent: Parent widget
 * \param size: Window size
 */
LoggerWindow::LoggerWindow(QWidget *parent, const QSize &size)
    : QDialog(parent)
    , m_layout(std::unique_ptr<QVBoxLayout>(new QVBoxLayout(this)))
    , m_tableView(std::unique_ptr<QTableView>(new QTableView(this)))
    , m_tableModel(std::unique_ptr<LoggerTableModel>(new LoggerTableModel(this)))
{
    m_layout->addWidget(m_tableView.get());
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout.get());

    m_tableView->setModel(m_tableModel.get());
    m_tableView->setSortingEnabled(true);

    QHeaderView *vHeader = m_tableView->verticalHeader();
    vHeader->setVisible(false);

    setWindowTitle(tr("Message log"));
    resize(size);
}

/*!
 * \brief LoggerWindow::logIncomingData
 * Add a new event to table
 * \param event: The event to be logged
 */
void LoggerWindow::logIncomingData(const LoggerTableModel::Event &event)
{
    m_tableModel->logIncomingData(event);
}

/*!
 * \brief LoggerWindow::closeEvent
 * Logger window was closed, notify main window of it
 * \param event: The close event
 */
void LoggerWindow::closeEvent(QCloseEvent *event)
{
    emit closed();
    event->accept();
}

/*!
 * \brief LoggerWindow::resizeEvent
 * Logger window was resized, resize table
 * \param event: The resize event
 */
void LoggerWindow::resizeEvent(QResizeEvent *event)
{
    QHeaderView *hHeader = m_tableView->horizontalHeader();
    for (int i = 0; i < hHeader->count(); ++i) {
        hHeader->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    event->accept();
}
