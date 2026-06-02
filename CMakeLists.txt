cmake_minimum_required(VERSION 3.24)

project(qwen_asr_qt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Network DBus)
find_package(KF6GlobalAccel REQUIRED)

qt_add_executable(qwen-asr-qt
    src/app_controller.cpp
    src/app_controller.h
    src/asr_client.cpp
    src/asr_client.h
    src/backend_monitor.cpp
    src/backend_monitor.h
    src/clipboard_writer.cpp
    src/clipboard_writer.h
    src/global_shortcut.cpp
    src/global_shortcut.h
    src/history_store.cpp
    src/history_store.h
    src/main.cpp
    src/recording_overlay.cpp
    src/recording_overlay.h
    src/tray_controller.cpp
    src/tray_controller.h
    src/transcript_cleaner.cpp
    src/transcript_cleaner.h
    src/types.h
    src/wav_recorder.cpp
    src/wav_recorder.h
)

target_link_libraries(qwen-asr-qt PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Network
    Qt6::DBus
    KF6::GlobalAccel
)

install(TARGETS qwen-asr-qt RUNTIME DESTINATION bin)
