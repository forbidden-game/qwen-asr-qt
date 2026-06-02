#pragma once

#include <QString>

enum class AppState {
    Idle,
    Recording,
    Transcribing,
    Done,
    Error,
};

enum class BackendState {
    Unknown,
    Connected,
    Disconnected,
    ModelMissing,
};

enum class SetupState {
    Unknown,
    MissingModel,
    DownloadingModel,
    BackendStarting,
    BackendLoadingModel,
    Ready,
    Error,
};

struct RuntimeStatus {
    SetupState setup = SetupState::Unknown;
    AppState app = AppState::Idle;
    BackendState backend = BackendState::Unknown;
    QString message;
    int downloadPercent = -1;
};

inline QString appStateText(AppState state)
{
    switch (state) {
    case AppState::Idle:
        return QStringLiteral("Idle");
    case AppState::Recording:
        return QStringLiteral("Recording");
    case AppState::Transcribing:
        return QStringLiteral("Transcribing");
    case AppState::Done:
        return QStringLiteral("Done");
    case AppState::Error:
        return QStringLiteral("Error");
    }
    return QStringLiteral("Unknown");
}

inline QString backendStateText(BackendState state)
{
    switch (state) {
    case BackendState::Unknown:
        return QStringLiteral("Unknown");
    case BackendState::Connected:
        return QStringLiteral("Connected");
    case BackendState::Disconnected:
        return QStringLiteral("Disconnected");
    case BackendState::ModelMissing:
        return QStringLiteral("ModelMissing");
    }
    return QStringLiteral("Unknown");
}

inline QString setupStateText(SetupState state)
{
    switch (state) {
    case SetupState::Unknown:
        return QStringLiteral("Unknown");
    case SetupState::MissingModel:
        return QStringLiteral("MissingModel");
    case SetupState::DownloadingModel:
        return QStringLiteral("DownloadingModel");
    case SetupState::BackendStarting:
        return QStringLiteral("BackendStarting");
    case SetupState::BackendLoadingModel:
        return QStringLiteral("BackendLoadingModel");
    case SetupState::Ready:
        return QStringLiteral("Ready");
    case SetupState::Error:
        return QStringLiteral("Error");
    }
    return QStringLiteral("Unknown");
}
