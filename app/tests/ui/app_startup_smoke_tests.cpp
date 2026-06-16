#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include <iostream>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    const QString appPath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("tsrs_app.exe"));
    if (!QFileInfo::exists(appPath)) {
        std::cerr << "tsrs_app.exe not found: " << appPath.toStdString() << '\n';
        return 1;
    }

    QProcess process;
    process.setProgram(appPath);
    process.setArguments({QStringLiteral("--smoke")});
    process.setWorkingDirectory(QCoreApplication::applicationDirPath());
    process.start();
    if (!process.waitForStarted(10000)) {
        std::cerr << "failed to start tsrs_app.exe\n";
        return 1;
    }
    if (!process.waitForFinished(30000)) {
        std::cerr << "tsrs_app.exe did not exit in time\n";
        process.kill();
        process.waitForFinished();
        return 1;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        std::cerr << "tsrs_app.exe returned " << process.exitCode() << '\n';
        return 1;
    }
    return 0;
}
