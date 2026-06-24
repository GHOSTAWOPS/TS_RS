#include "application/import/ImportedModelStore.h"

#include <utility>

namespace tsrs::application {

const StepSession& ImportedModelStore::addSession(StepSession session)
{
    if (session.sessionId.empty()) {
        session.sessionId = "step-session-" + std::to_string(nextSessionOrdinal_++);
    }
    sessions_.push_back(std::move(session));
    return sessions_.back();
}

const StepSession* ImportedModelStore::findSession(
    const std::string& sessionId) const
{
    for (const StepSession& session : sessions_) {
        if (session.sessionId == sessionId) {
            return &session;
        }
    }
    return nullptr;
}

} // namespace tsrs::application
