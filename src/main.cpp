#include <QtWidgets/QApplication>
#include "MainWindow.h"
#include <fstream>

static void dbg(const char* s)
{
    std::ofstream f("/tmp/bookget_debug.log", std::ios::app);
    if (f) f << s << std::endl;
}

int main(int argc, char* argv[])
{
    dbg("main: start");
    QApplication app(argc, argv);
    dbg("main: after QApplication");
    MainWindow w;
    dbg("main: after MainWindow construction");
    w.show();
    dbg("main: after show");
    int r = QApplication::exec();
    dbg("main: after exec");
    return r;
}
