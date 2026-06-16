#include "presentation/occ/OccViewerWidget.h"

#include <QLabel>
#include <QVBoxLayout>

namespace tsrs::presentation {

OccViewerWidget::OccViewerWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("central_occ_viewer"));
    setMinimumSize(640, 420);
    setStyleSheet(QStringLiteral("background-color: #8f9497;"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* label = new QLabel(QStringLiteral("OCCT Viewer"), this);
    label->setObjectName(QStringLiteral("central_occ_viewer_placeholder"));
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QStringLiteral("color: white; font-size: 18px;"));
    layout->addWidget(label);
}

} // namespace tsrs::presentation
