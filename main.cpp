#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif
#include "mainwindow.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("SimpleTextEditor"));

    MainWindow w;
    w.show();

    return app.exec();
}
