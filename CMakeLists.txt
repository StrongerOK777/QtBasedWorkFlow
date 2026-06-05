cmake_minimum_required(VERSION 3.16)

if(APPLE AND "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
    set(CMAKE_OSX_ARCHITECTURES "${CMAKE_HOST_SYSTEM_PROCESSOR}" CACHE STRING "macOS target architecture" FORCE)
endif()

project(ImageNodeEditor VERSION 0.1.0 LANGUAGES CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL Qml Quick QuickControls2 QuickWidgets)

qt_standard_project_setup()

set(QT_NODES_DEVELOPER_DEFAULTS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(USE_QT6 ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/QtNodes)

qt_add_executable(ImageNodeEditor
    WIN32
    MACOSX_BUNDLE
    ImageNodeEditor/main.cpp
    ImageNodeEditor/app/CommandLineRunner.h
    ImageNodeEditor/app/CommandLineRunner.cpp
    ImageNodeEditor/core/Edge.h
    ImageNodeEditor/core/NodeData.h
    ImageNodeEditor/core/NodeLabel.h
    ImageNodeEditor/core/NodeParameter.h
    ImageNodeEditor/core/PortType.h
    ImageNodeEditor/core/Result.h
    ImageNodeEditor/gui/AppTheme.h
    ImageNodeEditor/gui/AppTheme.cpp
    ImageNodeEditor/gui/MainWindow.h
    ImageNodeEditor/gui/MainWindow.cpp
    ImageNodeEditor/gui/AppIcon.h
    ImageNodeEditor/gui/AppIcon.cpp
    ImageNodeEditor/gui/CanvasTabBar.h
    ImageNodeEditor/gui/CanvasTabBar.cpp
    ImageNodeEditor/gui/MiniMapWidget.h
    ImageNodeEditor/gui/PreviewWidgets.h
    ImageNodeEditor/gui/TerminalPanel.h
    ImageNodeEditor/gui/NativeWindowChrome.h
    ImageNodeEditor/gui/NativeWindowChrome.cpp
    ImageNodeEditor/gui/WorkbenchHostWidget.h
    ImageNodeEditor/gui/WorkbenchHostWidget.cpp
    ImageNodeEditor/gui/WorkbenchModels.h
    ImageNodeEditor/gui/WorkbenchModels.cpp
    ImageNodeEditor/gui/WorkflowCanvas.h
    ImageNodeEditor/gui/WorkflowCanvas.cpp
    ImageNodeEditor/gui/WorkflowCommands.h
    ImageNodeEditor/gui/WorkflowCommands.cpp
    ImageNodeEditor/gui/WorkflowNodeDelegate.h
    ImageNodeEditor/gui/WorkflowNodeDelegate.cpp
    ImageNodeEditor/gui/WorkflowNodePainter.h
    ImageNodeEditor/gui/WorkflowNodePainter.cpp
    ImageNodeEditor/nodes/ImageNode.h
    ImageNodeEditor/nodes/ImageNode.cpp
    ImageNodeEditor/nodes/NodeFactory.h
    ImageNodeEditor/nodes/NodeFactory.cpp
    ImageNodeEditor/nodes/BasicNodes.h
    ImageNodeEditor/nodes/BasicNodes.cpp
    ImageNodeEditor/nodes/MacroNode.h
    ImageNodeEditor/nodes/MacroNode.cpp
    ImageNodeEditor/processing/ImageProcessors.h
    ImageNodeEditor/processing/ImageProcessors.cpp
    ImageNodeEditor/util/PathUtils.h
    ImageNodeEditor/util/PathUtils.cpp
    ImageNodeEditor/workflow/ExecutionEngine.h
    ImageNodeEditor/workflow/ExecutionEngine.cpp
    ImageNodeEditor/workflow/WorkflowGraph.h
    ImageNodeEditor/workflow/WorkflowGraph.cpp
    ImageNodeEditor/workflow/WorkflowSerializer.h
    ImageNodeEditor/workflow/WorkflowSerializer.cpp
    ImageNodeEditor/workflow/WorkflowValidator.h
    ImageNodeEditor/workflow/WorkflowValidator.cpp
)

target_include_directories(ImageNodeEditor PRIVATE ImageNodeEditor)

target_link_libraries(ImageNodeEditor PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickWidgets
    QtNodes::QtNodes
)

qt_add_resources(ImageNodeEditor workbench_qml
    PREFIX "/workbench"
    BASE "ImageNodeEditor/qml"
    FILES
        ImageNodeEditor/qml/WorkbenchActivityBar.qml
        ImageNodeEditor/qml/WorkbenchIcon.qml
        ImageNodeEditor/qml/WorkbenchTitleBar.qml
        ImageNodeEditor/qml/WorkbenchEditorHeader.qml
        ImageNodeEditor/qml/WorkbenchSidebar.qml
        ImageNodeEditor/qml/WorkbenchStatusBar.qml
        ImageNodeEditor/qml/WorkbenchQuickAccess.qml
        ImageNodeEditor/qml/WorkbenchTooltip.qml
)

qt_add_resources(ImageNodeEditor codicons_assets
    PREFIX "/codicons"
    BASE "third_party/codicons"
    FILES
        third_party/codicons/codicon.ttf
)

set_target_properties(ImageNodeEditor PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME ImageNodeEditor
    MACOSX_BUNDLE_GUI_IDENTIFIER com.alpd.ImageNodeEditor
    WIN32_EXECUTABLE TRUE
)

if(MINGW)
    target_link_options(ImageNodeEditor PRIVATE -mwindows)
endif()

if(APPLE)
    target_sources(ImageNodeEditor PRIVATE ImageNodeEditor/gui/NativeWindowChrome_mac.mm)
    target_link_libraries(ImageNodeEditor PRIVATE "-framework AppKit")
endif()

if(MSVC)
    target_compile_options(ImageNodeEditor PRIVATE /permissive- /W4)
else()
    target_compile_options(ImageNodeEditor PRIVATE -Wall -Wextra -Wpedantic)
endif()
