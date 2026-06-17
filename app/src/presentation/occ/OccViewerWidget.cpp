#include "presentation/occ/OccViewerWidget.h"

#include "presentation/style/StepModelDisplayStyle.h"

#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Quantity_Color.hxx>
#include <Standard_Failure.hxx>

#include <QLabel>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace tsrs::presentation {

OccViewerWidget::OccViewerWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setObjectName(QStringLiteral("central_occ_viewer"));
    setMinimumSize(640, 420);
    setStyleSheet(QStringLiteral("background-color: #8f9497;"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    statusLabel_ = new QLabel(QStringLiteral("OCCT Viewer"), this);
    statusLabel_->setObjectName(QStringLiteral("central_occ_viewer_placeholder"));
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setStyleSheet(QStringLiteral("color: white; font-size: 18px;"));
    layout->addWidget(statusLabel_);
}

OccViewerWidget::~OccViewerWidget()
{
    resetViewer();
}

bool OccViewerWidget::displayStepModel(const StepDisplayModel& model)
{
    if (model.shape.IsNull()) {
        displayedStepShapeCount_ = 0;
        if (statusLabel_ != nullptr) {
            statusLabel_->setText(QStringLiteral("STEP 导入失败"));
        }
        return false;
    }

    const StepModelDisplayStyle style = defaultStepModelDisplayStyle();
    if (!ensureViewer() || !initializeWindow()) {
        displayedStepShapeCount_ = 0;
        return false;
    }

    context_->RemoveAll(Standard_False);
    Handle(AIS_Shape) aisShape = new AIS_Shape(model.shape);
    aisShape->SetColor(Quantity_Color(style.red, style.green, style.blue, Quantity_TOC_RGB));
    context_->Display(aisShape, Standard_False);
    context_->SetDisplayMode(aisShape, AIS_Shaded, Standard_False);
    context_->UpdateCurrentViewer();
    view_->FitAll(0.01, Standard_True);
    view_->ZFitAll();
    redraw();

    displayedStepShapeCount_ = 1;
    if (statusLabel_ != nullptr) {
        statusLabel_->hide();
        statusLabel_->setText(
            QStringLiteral("STEP 已导入：%1 faces / %2 edges / %3 vertices，显示：%4")
                .arg(model.faceCount)
                .arg(model.edgeCount)
                .arg(model.vertexCount)
                .arg(QString::fromStdString(style.semanticName)));
    }
    return true;
}

int OccViewerWidget::displayedStepShapeCount() const
{
    return displayedStepShapeCount_;
}

QString OccViewerWidget::displayStatusText() const
{
    return statusLabel_ == nullptr ? QString{} : statusLabel_->text();
}

QPaintEngine* OccViewerWidget::paintEngine() const
{
    return nullptr;
}

void OccViewerWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    redraw();
}

void OccViewerWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (!window_.IsNull()) {
        windowReady_ = false;
        (void)initializeWindow();
    }
    redraw();
}

bool OccViewerWidget::ensureViewer()
{
    if (!view_.IsNull() && !context_.IsNull()) {
        return true;
    }

    try {
        displayConnection_ = new Aspect_DisplayConnection();
        graphicDriver_ = new OpenGl_GraphicDriver(displayConnection_, Standard_False);
        viewer_ = new V3d_Viewer(graphicDriver_);
        viewer_->SetDefaultLights();
        viewer_->SetLightOn();
        context_ = new AIS_InteractiveContext(viewer_);
        view_ = viewer_->CreateView();
        view_->SetBackgroundColor(Quantity_NOC_WHITE);
        view_->SetImmediateUpdate(Standard_False);
        window_ = new Aspect_NeutralWindow();
        return true;
    } catch (...) {
        resetViewer();
        return false;
    }
}

bool OccViewerWidget::initializeWindow()
{
    if (windowReady_) {
        return true;
    }
    if (view_.IsNull() || window_.IsNull()) {
        return false;
    }

    try {
        const double pixelRatio = devicePixelRatioF();
        const int widthPixels = std::max(1, static_cast<int>(std::lround(width() * pixelRatio)));
        const int heightPixels = std::max(1, static_cast<int>(std::lround(height() * pixelRatio)));
        const Aspect_Drawable nativeHandle =
            reinterpret_cast<Aspect_Drawable>(static_cast<quintptr>(winId()));

        window_->SetNativeHandle(nativeHandle);
        window_->SetSize(widthPixels, heightPixels);
        window_->Map();
        view_->SetWindow(window_);
        view_->MustBeResized();
        windowReady_ = true;
        return true;
    } catch (...) {
        windowReady_ = false;
        return false;
    }
}

void OccViewerWidget::redraw()
{
    if (view_.IsNull() || !windowReady_) {
        return;
    }
    try {
        view_->InvalidateImmediate();
        view_->Redraw();
    } catch (...) {
        resetViewer();
    }
}

void OccViewerWidget::resetViewer()
{
    windowReady_ = false;
    displayedStepShapeCount_ = 0;
    context_.Nullify();
    view_.Nullify();
    viewer_.Nullify();
    graphicDriver_.Nullify();
    displayConnection_.Nullify();
    window_.Nullify();
}

} // namespace tsrs::presentation
