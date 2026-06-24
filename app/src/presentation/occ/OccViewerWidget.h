#pragma once

#include "presentation/occ/StepDisplayModel.h"

#include <QWidget>

#include <AIS_InteractiveContext.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#include <string>

class QLabel;
class QPaintEngine;
class QPaintEvent;
class QResizeEvent;
class Aspect_DisplayConnection;

namespace tsrs::presentation {

enum class ViewerSelectionMode {
    None = 0,
    Vertex,
    Edge,
    Face,
    Shape,
};

struct ViewerSelectionColors {
    std::string selectedSemantic;
    std::string highlightSemantic;
};

[[nodiscard]] ViewerSelectionColors defaultViewerSelectionColors();
[[nodiscard]] QString viewerSelectionModeStatusText(ViewerSelectionMode mode);

class OccViewerWidget final : public QWidget {
    Q_OBJECT

public:
    explicit OccViewerWidget(QWidget* parent = nullptr);
    ~OccViewerWidget() override;
    [[nodiscard]] bool displayStepModel(const StepDisplayModel& model);
    [[nodiscard]] int displayedStepShapeCount() const;
    [[nodiscard]] QString displayStatusText() const;
    void setSelectionMode(ViewerSelectionMode mode);
    [[nodiscard]] ViewerSelectionMode selectionMode() const;
    [[nodiscard]] QString selectionModeStatusText() const;

protected:
    QPaintEngine* paintEngine() const override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    [[nodiscard]] bool ensureViewer();
    [[nodiscard]] bool initializeWindow();
    void redraw();
    void resetViewer();

    QLabel* statusLabel_{nullptr};
    int displayedStepShapeCount_{0};
    ViewerSelectionMode selectionMode_{ViewerSelectionMode::None};
    Handle(Aspect_DisplayConnection) displayConnection_;
    Handle(OpenGl_GraphicDriver) graphicDriver_;
    Handle(V3d_Viewer) viewer_;
    Handle(AIS_InteractiveContext) context_;
    Handle(V3d_View) view_;
    Handle(Aspect_NeutralWindow) window_;
    bool windowReady_{false};
};

} // namespace tsrs::presentation
