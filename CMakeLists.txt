cmake_minimum_required(VERSION 3.16)

project(ImageNodeEditor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

qt_add_executable(ImageNodeEditor
    WIN32
    MACOSX_BUNDLE
    ImageNodeEditor/main.cpp
    ImageNodeEditor/app/CommandLineRunner.h
    ImageNodeEditor/app/CommandLineRunner.cpp
    ImageNodeEditor/core/Edge.h
    ImageNodeEditor/core/NodeData.h
    ImageNodeEditor/core/NodeParameter.h
    ImageNodeEditor/core/PortType.h
    ImageNodeEditor/core/Result.h
    ImageNodeEditor/gui/AppTheme.h
    ImageNodeEditor/gui/AppTheme.cpp
    ImageNodeEditor/gui/MainWindow.h
    ImageNodeEditor/gui/MainWindow.cpp
    ImageNodeEditor/nodes/ImageNode.h
    ImageNodeEditor/nodes/ImageNode.cpp
    ImageNodeEditor/nodes/NodeFactory.h
    ImageNodeEditor/nodes/NodeFactory.cpp
    ImageNodeEditor/nodes/BasicNodes.h
    ImageNodeEditor/nodes/BasicNodes.cpp
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

if(MSVC)
    target_compile_options(ImageNodeEditor PRIVATE /permissive- /W4)
else()
    target_compile_options(ImageNodeEditor PRIVATE -Wall -Wextra -Wpedantic)
endif()
