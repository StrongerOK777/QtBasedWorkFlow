cmake_minimum_required(VERSION 3.16)

if(APPLE AND "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
    set(CMAKE_OSX_ARCHITECTURES "${CMAKE_HOST_SYSTEM_PROCESSOR}" CACHE STRING "macOS target architecture" FORCE)
endif()

project(ImageNodeEditor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

option(IMAGENODEEDITOR_USE_ELA "Use ElaWidgetTools for the widget shell when available." ON)

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
    ImageNodeEditor/gui/WorkflowCommands.h
    ImageNodeEditor/gui/WorkflowCommands.cpp
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
)

if(IMAGENODEEDITOR_USE_ELA)
    set(ELA_WIDGET_TOOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ElaWidgetTools/ElaWidgetTools")
    if(EXISTS "${ELA_WIDGET_TOOLS_DIR}/CMakeLists.txt")
        find_package(Qt6 COMPONENTS WidgetsPrivate QUIET)
        if(TARGET Qt6::WidgetsPrivate)
            set(CMAKE_AUTOMOC ON)
            set(CMAKE_AUTORCC ON)
            set(CMAKE_AUTOUIC ON)
            add_subdirectory("${ELA_WIDGET_TOOLS_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/ElaWidgetTools")
            target_link_libraries(ImageNodeEditor PRIVATE ElaWidgetTools)
            target_compile_definitions(ImageNodeEditor PRIVATE IMAGENODEEDITOR_USE_ELA=1)
            if(APPLE)
                set_target_properties(ImageNodeEditor PROPERTIES
                    BUILD_RPATH "@executable_path/../Frameworks;$<TARGET_FILE_DIR:ElaWidgetTools>"
                    INSTALL_RPATH "@executable_path/../Frameworks"
                )
                add_custom_command(TARGET ImageNodeEditor POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:ImageNodeEditor>/Contents/Frameworks"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "$<TARGET_FILE:ElaWidgetTools>"
                        "$<TARGET_BUNDLE_DIR:ImageNodeEditor>/Contents/Frameworks/$<TARGET_FILE_NAME:ElaWidgetTools>"
                    VERBATIM
                )
            endif()
            message(STATUS "ElaWidgetTools integration enabled.")
        else()
            message(WARNING "ElaWidgetTools was found, but Qt6::WidgetsPrivate is unavailable; building with the built-in Qt Widgets shell.")
        endif()
    else()
        message(WARNING "ElaWidgetTools source not found at ${ELA_WIDGET_TOOLS_DIR}; building with the built-in Qt Widgets shell.")
    endif()
endif()

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
