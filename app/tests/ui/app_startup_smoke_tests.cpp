#include "application/commands/StepImportCommandService.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>

#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPControl_Writer.hxx>
#include <TopoDS_Shape.hxx>

#include <iostream>

namespace {

QString writeStepFixture(QTemporaryDir* directory)
{
    const QString path = QDir(directory->path()).filePath(QStringLiteral("box.step"));
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(1.0, 1.0, 1.0).Shape();
    STEPControl_Writer writer;
    writer.Transfer(box, STEPControl_AsIs);
    writer.Write(path.toLocal8Bit().constData());
    return path;
}

int runAppSmoke(const QString& appPath, const QStringList& arguments)
{
    QProcess process;
    process.setProgram(appPath);
    process.setArguments(arguments);
    process.setWorkingDirectory(QFileInfo(appPath).absolutePath());
    process.start();
    if (!process.waitForStarted(10000)) {
        std::cerr << "failed to start tsrs_app.exe\n";
        return 1;
    }
    if (!process.waitForFinished(30000)) {
        std::cerr << "tsrs_app.exe did not exit in time\n";
        std::cerr << process.readAllStandardOutput().toStdString() << '\n';
        std::cerr << process.readAllStandardError().toStdString() << '\n';
        process.kill();
        process.waitForFinished();
        return 1;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        std::cerr << "tsrs_app.exe returned " << process.exitCode() << '\n';
        std::cerr << process.readAllStandardOutput().toStdString() << '\n';
        std::cerr << process.readAllStandardError().toStdString() << '\n';
        return 1;
    }
    return 0;
}

} // namespace

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    const QString appPath =
        argc > 1 ? QString::fromLocal8Bit(argv[1])
                 : QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("tsrs_app.exe"));
    if (!QFileInfo::exists(appPath)) {
        std::cerr << "tsrs_app.exe not found: " << appPath.toStdString() << '\n';
        return 1;
    }

    if (const int code = runAppSmoke(appPath, {QStringLiteral("--smoke")})) {
        return code;
    }

    QTemporaryDir stepFixtureDir;
    if (!stepFixtureDir.isValid()) {
        std::cerr << "failed to create step fixture temp directory\n";
        return 1;
    }
    const QString stepPath = writeStepFixture(&stepFixtureDir);
    const tsrs::application::StepImportCommandService importService;
    const tsrs::application::StepImportCommandResult imported =
        importService.importStep({stepPath.toStdString()});
    if (!imported.ok) {
        std::cerr << "generated STEP fixture is not importable before app smoke: "
                  << imported.diagnosticCode << ' ' << imported.diagnostic << '\n';
        return 1;
    }
    if (const int code = runAppSmoke(
            appPath,
            {QStringLiteral("--smoke"), QStringLiteral("--step"), stepPath})) {
        return code;
    }

    return 0;
}
