#include "ui/mainwindow/MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QTimer>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    tsrs::ui::MainWindow window;
    window.show();

    const QStringList arguments = QCoreApplication::arguments();
    const bool smokeMode = arguments.contains(QStringLiteral("--smoke"));
    const int stepArgIndex = arguments.indexOf(QStringLiteral("--step"));
    if (stepArgIndex >= 0) {
        if (stepArgIndex + 1 >= arguments.size()
            || !(smokeMode ? window.importStepFileForSmoke(arguments.at(stepArgIndex + 1))
                           : window.importStepFile(arguments.at(stepArgIndex + 1)))) {
            return 2;
        }
    }

    if (smokeMode) {
        QTimer::singleShot(0, &app, &QCoreApplication::quit);
    }

    return QApplication::exec();
}
