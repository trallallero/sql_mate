#include "mainwindow.h"

#include <signal.h>

#include <QApplication>
#include <QLocale>
#include <QMessageBox>

void manageSegFailure(int signalCode)
{
    QMessageBox::critical(nullptr, QT_TR_NOOP("ERRORE fatale"), QT_TR_NOOP("Errore fatale, il programma verrà chiuso!"));
    signal(signalCode, SIG_DFL);
    QApplication::exit(3);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    signal(SIGSEGV, manageSegFailure);

    MainWindow w;
    try
    {
        w.show();
        return a.exec();
    }
    catch(std::exception& e)
    {
        QMessageBox::critical(nullptr, QT_TR_NOOP("ERRORE fatale"), e.what());
    }
    catch(...)
    {
        QMessageBox::critical(nullptr, QT_TR_NOOP("ERRORE fatale"), QT_TR_NOOP("ERRORE fatale"), QT_TR_NOOP("Errore fatale, il programma verrà chiuso!"));
    }
    return 1;
}
