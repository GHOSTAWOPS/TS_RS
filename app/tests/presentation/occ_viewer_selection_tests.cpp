#include "presentation/occ/OccViewerWidget.h"

#include <QApplication>

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

} // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    tsrs::presentation::OccViewerWidget viewer;

    if (const int code = expect(
            viewer.selectionMode()
                == tsrs::presentation::ViewerSelectionMode::None,
            "Viewer selection mode must default to None")) {
        return code;
    }

    viewer.setSelectionMode(tsrs::presentation::ViewerSelectionMode::Edge);
    if (const int code = expect(
            viewer.selectionMode()
                == tsrs::presentation::ViewerSelectionMode::Edge,
            "Viewer must store Edge selection mode")) {
        return code;
    }
    if (const int code = expect(
            viewer.selectionModeStatusText().contains(QStringLiteral("边")),
            "Viewer Edge selection mode must expose Chinese status text")) {
        return code;
    }

    const tsrs::presentation::ViewerSelectionColors colors =
        tsrs::presentation::defaultViewerSelectionColors();
    if (const int code = expect(colors.selectedSemantic == "selection-red",
                                "Selected geometry must use red semantic")) {
        return code;
    }
    if (const int code = expect(colors.highlightSemantic == "highlight-magenta",
                                "Hovered/highlighted geometry must use magenta semantic")) {
        return code;
    }
    return 0;
}
