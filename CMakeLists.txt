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
    ImageNodeEditor/app/CommandLineApp.h
    ImageNodeEditor/app/CommandLineApp.cpp
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
    ImageNodeEditor/workflow/WorkflowHistory.h
    ImageNodeEditor/workflow/WorkflowHistory.cpp
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

# 让 CMake 生成的 Visual Studio 解决方案默认以可运行的 ImageNodeEditor 为启动项目，
# 否则默认会选中不可运行的 ALL_BUILD，F5 启动时报“无法启动程序 ... ALL_BUILD / 拒绝访问”。
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ImageNodeEditor)
endif()

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

# 在构建根目录生成便捷的 picdeal 命令（同一二进制，直接 `./build/picdeal <子命令>`）。
# 非 Windows 用符号链接指向可执行（macOS 指向 .app 内的真实二进制）；Windows 复制一份 picdeal.exe。
if(WIN32)
    add_custom_command(TARGET ImageNodeEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:ImageNodeEditor>" "${CMAKE_BINARY_DIR}/picdeal.exe"
        VERBATIM
        COMMENT "生成 picdeal.exe 便捷命令")
else()
    add_custom_command(TARGET ImageNodeEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink "$<TARGET_FILE:ImageNodeEditor>" "${CMAKE_BINARY_DIR}/picdeal"
        VERBATIM
        COMMENT "生成 picdeal 便捷命令（符号链接）")
endif()

# ===== 单元测试（QtTest，可选）=====
# 只依赖核心层（nodes/processing/workflow/util），不链接 GUI 与 QtNodes；
# 找不到 Qt6Test 时静默跳过，不影响主程序构建，也不影响原生 VS 工程。
# tests/ 仅存在于本地开发环境（已 gitignore，不进入提交仓库），
# 因此目录缺失时也要静默跳过，保证全新克隆能直接配置。
option(IMAGENODEEDITOR_BUILD_TESTS "构建 ImageNodeEditor 单元测试" ON)
if(IMAGENODEEDITOR_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_workflow_json.cpp")
    find_package(Qt6 QUIET COMPONENTS Test)
    if(Qt6Test_FOUND)
        enable_testing()
        set(INE_TEST_CORE_SOURCES
            ImageNodeEditor/core/Edge.h
            ImageNodeEditor/core/NodeData.h
            ImageNodeEditor/core/NodeLabel.h
            ImageNodeEditor/core/NodeParameter.h
            ImageNodeEditor/core/PortType.h
            ImageNodeEditor/core/Result.h
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
        foreach(test_name IN ITEMS test_workflow_json test_graph_validation test_nodes)
            qt_add_executable(${test_name} tests/${test_name}.cpp ${INE_TEST_CORE_SOURCES})
            target_include_directories(${test_name} PRIVATE ImageNodeEditor)
            target_link_libraries(${test_name} PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
            add_test(NAME ${test_name} COMMAND ${test_name})
            # 离屏平台运行，便于 CI / 无显示环境执行（文字叠加等仍可渲染）。
            set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
        endforeach()
    else()
        message(STATUS "未找到 Qt6Test，跳过单元测试目标")
    endif()
endif()
