#pragma once

#include "presentation/occ/StepDisplayModel.h"
#include "step/ShapeStore.h"
#include "step/TopologyBindingRegistry.h"

#include <string>
#include <vector>

namespace tsrs::application {

struct StepSession {
    std::string sessionId;
    std::string sourcePath;
    tsrs::presentation::StepDisplayModel displayModel;
    tsrs::step::ShapeStore shapeStore;
    tsrs::step::TopologyBindingRegistry topologyBindings;
};

class ImportedModelStore final {
public:
    [[nodiscard]] const StepSession& addSession(StepSession session);

    [[nodiscard]] const StepSession* findSession(
        const std::string& sessionId) const;

    [[nodiscard]] std::size_t size() const { return sessions_.size(); }

private:
    std::vector<StepSession> sessions_;
    int nextSessionOrdinal_{1};
};

} // namespace tsrs::application
