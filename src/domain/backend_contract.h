#pragma once

#include <QString>

namespace BackendContract {

QString audioTranscriptionsPath();
QString healthPath();
QString modelsPath();
QString expectedHealthStatus();
QString requiredModelCapability();

QString llamaCppRepository();
QString llamaCppPinnedCommit();
QString llamaCppPinnedDescription();

} // namespace BackendContract
