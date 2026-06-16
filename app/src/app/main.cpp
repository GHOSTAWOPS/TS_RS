#include "ui/mainwindow/MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QTimer>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    tsrs::ui::MainWindow window;
    window.show();

    if (QCoreApplication::arguments().contains(QStringLiteral("--smoke"))) {
        QTimer::singleShot(0, &app, &QCoreApplication::quit);
    }

    return QApplication::exec();
}
