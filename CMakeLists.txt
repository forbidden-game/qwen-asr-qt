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

set(QWEN_ASR_BACKEND_SOURCE_DIR "$ENV{QWEN_ASR_BACKEND_SOURCE_DIR}" CACHE PATH "Path to the qwen-asr C backend source tree")
if(NOT QWEN_ASR_BACKEND_SOURCE_DIR AND EXISTS "${CMAKE_SOURCE_DIR}/third_party/qwen-asr/Makefile")
    set(QWEN_ASR_BACKEND_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/qwen-asr" CACHE PATH "Path to the qwen-asr C backend source tree" FORCE)
endif()

if(QWEN_ASR_BACKEND_SOURCE_DIR)
    find_program(MAKE_EXECUTABLE NAMES gmake make REQUIRED)
    set(QWEN_ASR_BACKEND_BINARY "${QWEN_ASR_BACKEND_SOURCE_DIR}/qwen_asr_server")
    add_custom_target(qwen-asr-backend
        COMMAND "${MAKE_EXECUTABLE}" server
        WORKING_DIRECTORY "${QWEN_ASR_BACKEND_SOURCE_DIR}"
        COMMENT "Building qwen_asr_server backend"
        VERBATIM
    )
    add_dependencies(qwen-asr-qt qwen-asr-backend)
    add_custom_command(TARGET qwen-asr-qt POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${QWEN_ASR_BACKEND_BINARY}"
            "$<TARGET_FILE_DIR:qwen-asr-qt>/qwen_asr_server"
        COMMENT "Copying qwen_asr_server next to qwen-asr-qt"
        VERBATIM
    )
    install(PROGRAMS "${QWEN_ASR_BACKEND_BINARY}" DESTINATION bin OPTIONAL)
else()
    message(STATUS "QWEN_ASR_BACKEND_SOURCE_DIR not set; qwen_asr_server will not be built or bundled")
endif()

install(TARGETS qwen-asr-qt RUNTIME DESTINATION bin)
