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

QString llamaCppRepository()
{
    return QStringLiteral("ggml-org/llama.cpp");
}

QString llamaCppPinnedCommit()
{
    return QStringLiteral("60130d18f9ac7f42cb4d7f6060b088a45d8f242e");
}

QString llamaCppPinnedDescription()
{
    return QStringLiteral("b9478");
}

} // namespace BackendContract
