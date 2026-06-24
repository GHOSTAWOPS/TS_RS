#pragma once

#include <QMainWindow>
#include <QString>

#include <memory>

class QDockWidget;
class QTabWidget;
class QTextEdit;
class QTreeView;

namespace tsrs::application {
class ImportedModelStore;
}

namespace tsrs::presentation {
class OccViewerWidget;
}

namespace tsrs::ui {

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    [[nodiscard]] bool importStepFile(const QString& stepPath);
    [[nodiscard]] bool importStepFileForSmoke(const QString& stepPath);
    [[nodiscard]] QString currentStepSessionId() const;
    [[nodiscard]] std::size_t importedModelCount() const;
    [[nodiscard]] bool resolveFirstSelectionForSmoke(const QString& kind);
    [[nodiscard]] QString currentSelectionStableId() const;
    [[nodiscard]] QString currentSelectionKind() const;

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
    std::unique_ptr<application::ImportedModelStore> importedModelStore_;
    QString currentStepSessionId_;
    QString currentSelectionStableId_;
    QString currentSelectionKind_;
};

} // namespace tsrs::ui
