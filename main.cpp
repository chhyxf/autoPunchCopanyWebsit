#include <QApplication>
#include "mainwindow.h"
#include <QTextEdit>
//#include <QTextCodec>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

//    QTextCodec *codec = QTextCodec::codecForName("utf8");
//    w.hide();
    return a.exec();
}


