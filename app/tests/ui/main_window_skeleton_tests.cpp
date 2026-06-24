#include "ui/mainwindow/MainWindow.h"
#include "presentation/occ/OccViewerWidget.h"

#include <QApplication>
#include <QDockWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeView>

#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPControl_Writer.hxx>
#include <TopoDS_Shape.hxx>

#include <filesystem>
#include <iostream>
#include <string>

namespace {

int fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

int expect(bool condition, const std::string& message)
{
    return condition ? 0 : fail(message);
}

std::filesystem::path writeBoxStepFixture()
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "tsrs_main_window_session_box.step";
    std::filesystem::remove(path);

    const TopoDS_Shape box = BRepPrimAPI_MakeBox(1.0, 1.0, 1.0).Shape();
    STEPControl_Writer writer;
    writer.Transfer(box, STEPControl_AsIs);
    writer.Write(path.string().c_str());
    return path;
}

} // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    tsrs::ui::MainWindow window;
    window.show();
    app.processEvents();

    if (const int code =
            expect(window.windowTitle() == QStringLiteral("TS_RS 钢筋设计"),
                   "MainWindow must expose the TS_RS product title")) {
        return code;
    }

    const auto* tabs = window.findChild<QTabWidget*>(QStringLiteral("main_tabs"));
    if (const int code = expect(tabs != nullptr, "MainWindow must expose main_tabs")) {
        return code;
    }
    if (const int code = expect(tabs->count() == 5, "MainWindow must expose five top tabs")) {
        return code;
    }

    const QStringList expectedTabs{
        QStringLiteral("开始"),
        QStringLiteral("显示"),
        QStringLiteral("钢筋"),
        QStringLiteral("查询"),
        QStringLiteral("工程图"),
    };
    for (int index = 0; index < expectedTabs.size(); ++index) {
        if (const int code =
                expect(tabs->tabText(index) == expectedTabs[index],
                       "MainWindow top tab order does not match UI spec")) {
            return code;
        }
    }

    if (const int code =
            expect(window.findChild<tsrs::presentation::OccViewerWidget*>(
                       QStringLiteral("central_occ_viewer")) != nullptr,
                   "MainWindow must expose the central OCCT viewer widget")) {
        return code;
    }

    if (const int code =
            expect(window.findChild<QDockWidget*>(QStringLiteral("dock_project_tree")) != nullptr,
                   "MainWindow must expose left project tree dock")) {
        return code;
    }
    if (const int code =
            expect(window.findChild<QTreeView*>(QStringLiteral("project_tree_view")) != nullptr,
                   "MainWindow must expose project tree view")) {
        return code;
    }
    if (const int code =
            expect(window.findChild<QDockWidget*>(QStringLiteral("dock_command_panel")) != nullptr,
                   "MainWindow must expose right command panel dock")) {
        return code;
    }
    if (const int code =
            expect(window.findChild<QDockWidget*>(QStringLiteral("dock_bottom_messages")) != nullptr,
                   "MainWindow must expose bottom messages dock")) {
        return code;
    }
    if (const int code =
            expect(window.findChild<QTextEdit*>(QStringLiteral("message_log")) != nullptr,
                   "MainWindow must expose message log")) {
        return code;
    }

    auto* projectDock = window.findChild<QDockWidget*>(QStringLiteral("dock_project_tree"));
    auto* commandDock = window.findChild<QDockWidget*>(QStringLiteral("dock_command_panel"));
    auto* bottomDock = window.findChild<QDockWidget*>(QStringLiteral("dock_bottom_messages"));
    if (const int code = expect(projectDock != nullptr && window.dockWidgetArea(projectDock) == Qt::LeftDockWidgetArea,
                                "Project tree dock must be placed in the left dock area")) {
        return code;
    }
    if (const int code = expect(commandDock != nullptr && window.dockWidgetArea(commandDock) == Qt::RightDockWidgetArea,
                                "Command panel dock must be placed in the right dock area")) {
        return code;
    }
    if (const int code = expect(bottomDock != nullptr && window.dockWidgetArea(bottomDock) == Qt::BottomDockWidgetArea,
                                "Message dock must be placed in the bottom dock area")) {
        return code;
    }

    if (const int code = expect(window.isVisible(), "MainWindow must be shown for smoke coverage")) {
        return code;
    }

    const std::filesystem::path fixture = writeBoxStepFixture();
    if (const int code =
            expect(window.importStepFileForSmoke(QString::fromStdString(fixture.string())),
                   "MainWindow smoke import must succeed through command service")) {
        return code;
    }
    if (const int code =
            expect(window.importedModelCount() == 1,
                   "MainWindow must store imported STEP as a session")) {
        return code;
    }
    if (const int code =
            expect(!window.currentStepSessionId().isEmpty(),
                   "MainWindow must expose current STEP session id after import")) {
        return code;
    }
    if (const int code =
            expect(window.resolveFirstSelectionForSmoke(QStringLiteral("edge")),
                   "MainWindow must resolve first edge selection through SelectionCommandService")) {
        return code;
    }
    if (const int code =
            expect(!window.currentSelectionStableId().isEmpty(),
                   "MainWindow must expose selected stable binding id")) {
        return code;
    }
    if (const int code =
            expect(window.currentSelectionKind() == QStringLiteral("edge"),
                   "MainWindow must expose selected binding kind")) {
        return code;
    }

    return 0;
}
