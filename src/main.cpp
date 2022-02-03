#include "inc/mywidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyWidget window;
    window.show();
    return a.exec();
}
