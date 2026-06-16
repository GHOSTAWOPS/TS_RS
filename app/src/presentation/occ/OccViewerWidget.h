#pragma once

#include <QWidget>

namespace tsrs::presentation {

class OccViewerWidget final : public QWidget {
    Q_OBJECT

public:
    explicit OccViewerWidget(QWidget* parent = nullptr);
};

} // namespace tsrs::presentation
