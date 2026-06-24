#include "ui/mainwindow/MainWindow.h"

#include "application/commands/SelectionCommandService.h"
#include "application/commands/StepImportCommandService.h"
#include "application/import/ImportedModelStore.h"
#include "presentation/occ/OccViewerWidget.h"

#include <QDockWidget>
#include <QFileInfo>
#include <QLabel>
#include <QDebug>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

namespace tsrs::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , importedModelStore_(std::make_unique<application::ImportedModelStore>())
{
    buildUi();
}

MainWindow::~MainWindow() = default;

QString MainWindow::currentStepSessionId() const
{
    return currentStepSessionId_;
}

std::size_t MainWindow::importedModelCount() const
{
    return importedModelStore_ == nullptr ? 0 : importedModelStore_->size();
}

QString MainWindow::currentSelectionStableId() const
{
    return currentSelectionStableId_;
}

QString MainWindow::currentSelectionKind() const
{
    return currentSelectionKind_;
}

bool MainWindow::resolveFirstSelectionForSmoke(const QString& kind)
{
    if (importedModelStore_ == nullptr || currentStepSessionId_.isEmpty()) {
        return false;
    }

    const application::StepSession* session =
        importedModelStore_->findSession(currentStepSessionId_.toStdString());
    if (session == nullptr) {
        return false;
    }

    const std::vector<tsrs::step::TopologyBinding>* bindings = nullptr;
    if (kind == QStringLiteral("edge")) {
        bindings = &session->topologyBindings.edges();
    } else if (kind == QStringLiteral("face")) {
        bindings = &session->topologyBindings.faces();
    } else if (kind == QStringLiteral("vertex")) {
        bindings = &session->topologyBindings.vertices();
    } else {
        return false;
    }

    if (bindings == nullptr || bindings->empty()) {
        return false;
    }

    const tsrs::step::TopologyBinding& binding = bindings->front();
    const tsrs::step::TopologyBindingReference reference =
        tsrs::step::makeBindingReference("smokeSelection", binding);
    const application::SelectionCommandService service(&session->topologyBindings);
    const application::SelectionCommandResult result =
        service.resolveSelection({"smokeSelection", reference});

    if (!result.ok) {
        return false;
    }
    currentSelectionStableId_ = QString::fromStdString(result.bindingStableId);
    currentSelectionKind_ = QString::fromStdString(result.kind);
    return true;
}

void MainWindow::buildUi()
{
    setWindowTitle(QStringLiteral("TS_RS 钢筋设计"));
    resize(1280, 800);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    tabs_ = createMainTabs();
    viewer_ = new presentation::OccViewerWidget(central);

    layout->addWidget(tabs_);
    layout->addWidget(viewer_, 1);
    setCentralWidget(central);

    addDockWidget(Qt::LeftDockWidgetArea, createProjectTreeDock());
    addDockWidget(Qt::RightDockWidgetArea, createCommandPanelDock());
    addDockWidget(Qt::BottomDockWidgetArea, createBottomMessagesDock());
}

bool MainWindow::importStepFile(const QString& stepPath)
{
    return importStepFileInternal(stepPath, true);
}

bool MainWindow::importStepFileForSmoke(const QString& stepPath)
{
    return importStepFileInternal(stepPath, false);
}

bool MainWindow::importStepFileInternal(const QString& stepPath, bool displayInViewer)
{
    const tsrs::application::StepImportCommandService service(importedModelStore_.get());
    const tsrs::application::StepImportCommandResult imported =
        service.importStep({stepPath.toStdString()});
    if (!imported.ok) {
        qWarning().noquote() << QStringLiteral("STEP 导入失败：%1 [%2]")
                                    .arg(QString::fromStdString(imported.diagnostic))
                                    .arg(QString::fromStdString(imported.diagnosticCode));
        if (messageLog_ != nullptr) {
            messageLog_->append(QStringLiteral("STEP 导入失败：%1")
                                    .arg(QString::fromStdString(imported.diagnostic)));
        }
        return false;
    }
    currentStepSessionId_ = QString::fromStdString(imported.sessionId);

    if (displayInViewer && (viewer_ == nullptr || !viewer_->displayStepModel(imported.displayModel))) {
        qWarning().noquote() << QStringLiteral("STEP 显示失败：%1")
                                    .arg(QFileInfo(stepPath).fileName());
        if (messageLog_ != nullptr) {
            messageLog_->append(QStringLiteral("STEP 显示失败：%1")
                                    .arg(QFileInfo(stepPath).fileName()));
        }
        return false;
    }

    if (messageLog_ != nullptr) {
        messageLog_->append(QStringLiteral("STEP 已导入：%1，faces=%2，edges=%3，vertices=%4")
                                .arg(QFileInfo(stepPath).fileName())
                                .arg(imported.displayModel.faceCount)
                                .arg(imported.displayModel.edgeCount)
                                .arg(imported.displayModel.vertexCount));
        messageLog_->append(QStringLiteral("导入会话：%1").arg(currentStepSessionId_));
        if (!displayInViewer) {
            messageLog_->append(QStringLiteral("Smoke 模式：已跳过 Viewer 显示。"));
        }
    }
    return true;
}

QTabWidget* MainWindow::createMainTabs()
{
    auto* tabs = new QTabWidget(this);
    tabs->setObjectName(QStringLiteral("main_tabs"));

    const QStringList tabNames{
        QStringLiteral("开始"),
        QStringLiteral("显示"),
        QStringLiteral("钢筋"),
        QStringLiteral("查询"),
        QStringLiteral("工程图"),
    };

    for (const QString& tabName : tabNames) {
        auto* page = new QWidget(tabs);
        auto* layout = new QVBoxLayout(page);
        layout->setContentsMargins(8, 6, 8, 6);
        auto* label = new QLabel(tabName, page);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        layout->addWidget(label);
        tabs->addTab(page, tabName);
    }

    return tabs;
}

QDockWidget* MainWindow::createProjectTreeDock()
{
    auto* dock = new QDockWidget(QStringLiteral("工程树"), this);
    dock->setObjectName(QStringLiteral("dock_project_tree"));

    projectTree_ = new QTreeView(dock);
    projectTree_->setObjectName(QStringLiteral("project_tree_view"));

    auto* model = new QStandardItemModel(projectTree_);
    model->setHorizontalHeaderLabels({QStringLiteral("项目")});

    auto* root = new QStandardItem(QStringLiteral("TS_RS 工程"));
    root->appendRow(new QStandardItem(QStringLiteral("源 STEP 模型")));
    root->appendRow(new QStandardItem(QStringLiteral("结构实体")));
    root->appendRow(new QStandardItem(QStringLiteral("钢筋组")));
    root->appendRow(new QStandardItem(QStringLiteral("图纸")));
    root->appendRow(new QStandardItem(QStringLiteral("Detail 输出")));
    model->appendRow(root);

    projectTree_->setModel(model);
    projectTree_->expandAll();
    dock->setWidget(projectTree_);
    return dock;
}

QDockWidget* MainWindow::createCommandPanelDock()
{
    auto* dock = new QDockWidget(QStringLiteral("当前钢筋生成器"), this);
    dock->setObjectName(QStringLiteral("dock_command_panel"));

    auto* panel = new QWidget(dock);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(10, 10, 10, 10);

    const QStringList summaryRows{
        QStringLiteral("当前命令：空闲"),
        QStringLiteral("当前对象：无"),
        QStringLiteral("选择状态：无"),
        QStringLiteral("状态提示：等待命令"),
    };

    for (const QString& row : summaryRows) {
        auto* label = new QLabel(row, panel);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        layout->addWidget(label);
    }
    layout->addStretch(1);

    dock->setWidget(panel);
    return dock;
}

QDockWidget* MainWindow::createBottomMessagesDock()
{
    auto* dock = new QDockWidget(QStringLiteral("消息"), this);
    dock->setObjectName(QStringLiteral("dock_bottom_messages"));

    messageLog_ = new QTextEdit(dock);
    messageLog_->setObjectName(QStringLiteral("message_log"));
    messageLog_->setReadOnly(true);
    messageLog_->setPlainText(QStringLiteral("TS_RS 已启动。"));
    dock->setWidget(messageLog_);
    return dock;
}

} // namespace tsrs::ui
