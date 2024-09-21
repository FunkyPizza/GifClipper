cmake_minimum_required(VERSION 3.5)

project(GifClipper VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia MultimediaWidgets)

set(FFMPEG_EXE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/ffmpeg.exe
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/avcodec-59.dll
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/avdevice-59.dll
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/avfilter-8.dll
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/avformat-59.dll
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/avutil-57.dll
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/ffmpeg_LICENSE
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/ffmpeg_README.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/ffplay.exe
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/ffprobe.exe
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/postproc-56.dll
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/swresample-4.dll
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/bin/swscale-6.dll
)

file(COPY ${FFMPEG_EXE_DIR} DESTINATION ${CMAKE_BINARY_DIR})

set(FFMPEG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/include)
set(FFMPEG_LIBRARIES
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/lib/libavcodec.dll.a
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/lib/libavfilter.dll.a
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/lib/libavformat.dll.a
    ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg/ffmpeg-5.1.2-full_build-shared/lib/libavutil.dll.a
)

set(PROJECT_SOURCES
        main.cpp

        gifc_mainwindow.cpp
        gifc_mainwindow.h
        gifc_mainwindow.ui

        #Resources
        GifClipper_Resources.qrc

        #Style manager
        QStyleManager/dark/darkstyle.qrc
        QStyleManager/dark/darkstyle.qss
        QStyleManager/light/lightstyle.qrc
        QStyleManager/light/lightstyle.qss
        QStyleManager/QStyleManager.h
        QStyleManager/QStyleManager.cpp

        #Media Player
        QVideoPlayer/QVideoPlayer.h
        QVideoPlayer/QVideoPlayer.cpp
        QVideoPlayer/QVideoPlayerWidget.h
        QVideoPlayer/QVideoPlayerWidget.cpp

        #Timeline
        QSimpleTimeline/QSimpleTimeline.h
        QSimpleTimeline/QSimpleTimeline.cpp

        #FFMPEG
        QFFmpeg/qffmpegfunctionlib.h
        QFFmpeg/qffmpegfunctionlib.cpp
        QFFmpeg/TranscoderFactory.h
        QFFmpeg/TranscoderInterface.h
        QFFmpeg/TranscoderQualityStatics.h
        QFFmpeg/Transcoders/GifTranscoder.h
        QFFmpeg/Transcoders/GifTranscoder.cpp
        QFFmpeg/Transcoders/GifCmdTranscoder.h
        QFFmpeg/Transcoders/GifCmdTranscoder.cpp
        QFFmpeg/Transcoders/MP4Transcoder.h
        QFFmpeg/Transcoders/MP4Transcoder.cpp
        QFFmpeg/Transcoders/MP4CmdTranscoder.h
        QFFmpeg/Transcoders/MP4CmdTranscoder.cpp
        ${TS_FILES}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/Assets/gifclipper_icon.rc")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(GifClipper
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_ICON_RESOURCE_WINDOWS}
    )
endif()


target_include_directories(GifClipper PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_include_directories(GifClipper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/QFFmpeg)
target_include_directories(GifClipper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/QSimpleTimeline)
target_include_directories(GifClipper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/QStyleManager)
target_include_directories(GifClipper PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/QVideoPlayer)

target_link_libraries(GifClipper PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(GifClipper PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
target_link_libraries(GifClipper PRIVATE Qt${QT_VERSION_MAJOR}::MultimediaWidgets)
target_link_libraries(GifClipper PRIVATE ${FFMPEG_LIBRARIES})


set_target_properties(GifClipper PROPERTIES
    WIN32_EXECUTABLE TRUE
)

install(TARGETS GifClipper
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(GifClipper)
endif()
