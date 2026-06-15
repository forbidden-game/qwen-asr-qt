#include "domain/backend_contract.h"

namespace BackendContract {

QString audioTranscriptionsPath()
{
    return QStringLiteral("/v1/audio/transcriptions");
}

QString healthPath()
{
    return QStringLiteral("/health");
}

QString modelsPath()
{
    return QStringLiteral("/v1/models");
}

QString expectedHealthStatus()
{
    return QStringLiteral("ok");
}

QString requiredModelCapability()
{
    return QStringLiteral("multimodal");
}

QString backendExecutableName()
{
    return QStringLiteral("qwen_asr_server");
}

QString backendRepository()
{
    return QStringLiteral("antirez/qwen-asr");
}

QString backendDescription()
{
    return QStringLiteral("Qwen3-ASR pure C HTTP backend with lazy model loading");
}

} // namespace BackendContract
