cmake_minimum_required(VERSION 3.24)

project(qwen_asr_qt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Network DBus)
find_package(KF6GlobalAccel REQUIRED)

qt_add_executable(qwen-asr-qt
    src/app/app_controller.cpp
    src/app/app_controller.h
    src/app/app_settings.cpp
    src/app/app_settings.h
    src/app/app_state.h
    src/domain/asr_types.h
    src/domain/backend_contract.cpp
    src/domain/backend_contract.h
    src/domain/transcript_cleaner.cpp
    src/domain/transcript_cleaner.h
    src/main.cpp
    src/platform/clipboard_writer.cpp
    src/platform/clipboard_writer.h
    src/platform/global_shortcut.cpp
    src/platform/global_shortcut.h
    src/services/asr_client.cpp
    src/services/asr_client.h
    src/services/backend_monitor.cpp
    src/services/backend_monitor.h
    src/services/backend_process_manager.cpp
    src/services/backend_process_manager.h
    src/services/history_store.cpp
    src/services/history_store.h
    src/services/wav_recorder.cpp
    src/services/wav_recorder.h
    src/ui/recording_overlay.cpp
    src/ui/recording_overlay.h
    src/ui/tray_controller.cpp
    src/ui/tray_controller.h
)

target_include_directories(qwen-asr-qt PRIVATE src)

target_link_libraries(qwen-asr-qt PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Network
    Qt6::DBus
    KF6::GlobalAccel
)

install(TARGETS qwen-asr-qt RUNTIME DESTINATION bin)
