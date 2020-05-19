/*!
* \file
* \brief main.cpp foo
*/

#include "cansimulatorcore.h"
#include "mainwindow.h"
#include <QApplication>
#include <QtGlobal>

/*!
 * \brief main
 * Start up the GUI from command line
 * \param argc: number of command line parameters
 * \param argv: command line parameters
 */
int main(int argc, char **argv)
{
    try {
        QApplication a(argc, argv);
        MainWindow win;
        win.show();
        return a.exec();
    // Error message will be shown by win instead of us, because apparently
    // only QWidget-derived classes can show messageboxes
    } catch (CANSimulatorCoreException &) {
        return 2;
    }
}
