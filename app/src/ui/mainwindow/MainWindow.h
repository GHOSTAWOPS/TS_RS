#pragma once

#include <QMainWindow>
#include <QString>

class QDockWidget;
class QTabWidget;
class QTextEdit;
class QTreeView;

namespace tsrs::presentation {
class OccViewerWidget;
}

namespace tsrs::ui {

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
    [[nodiscard]] bool importStepFile(const QString& stepPath);
    [[nodiscard]] bool importStepFileForSmoke(const QString& stepPath);

private:
    [[nodiscard]] bool importStepFileInternal(const QString& stepPath, bool displayInViewer);
    void buildUi();
    QTabWidget* createMainTabs();
    QDockWidget* createProjectTreeDock();
    QDockWidget* createCommandPanelDock();
    QDockWidget* createBottomMessagesDock();

    presentation::OccViewerWidget* viewer_{nullptr};
    QTabWidget* tabs_{nullptr};
    QTreeView* projectTree_{nullptr};
    QTextEdit* messageLog_{nullptr};
};

} // namespace tsrs::ui
