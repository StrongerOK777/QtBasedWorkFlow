@echo off
REM ============================================================
REM 重新生成 Qt 的 moc / rcc 文件。
REM 当你编辑了含 Q_OBJECT 的头文件（CanvasTabBar.h / WorkbenchHostWidget.h /
REM WorkbenchModels.h）或 qml 资源后，双击运行本脚本即可刷新本目录下的生成文件，
REM 然后在 Visual Studio 里重新生成解决方案。
REM Qt 路径默认 C:\Qt\6.8.3\msvc2022_64，可用环境变量 QTDIR 覆盖。
REM ============================================================
setlocal
if "%QTDIR%"=="" set "QTDIR=C:\Qt\6.8.3\msvc2022_64"
set "MOC=%QTDIR%\bin\moc.exe"
set "RCC=%QTDIR%\bin\rcc.exe"
REM 切到仓库根目录（本脚本位于 ImageNodeEditor\GeneratedFiles\）。
cd /d "%~dp0..\.."
set "INC=-I ImageNodeEditor -I third_party\QtNodes\include -I "%QTDIR%\include" -I "%QTDIR%\include\QtCore" -I "%QTDIR%\include\QtGui" -I "%QTDIR%\include\QtWidgets" -I "%QTDIR%\include\QtQml" -I "%QTDIR%\include\QtQuick" -I "%QTDIR%\include\QtQuickWidgets""
set "GEN=ImageNodeEditor\GeneratedFiles"
echo [moc] CanvasTabBar
"%MOC%" %INC% ImageNodeEditor\gui\CanvasTabBar.h -o "%GEN%\moc_CanvasTabBar.cpp"
echo [moc] WorkbenchHostWidget
"%MOC%" %INC% ImageNodeEditor\gui\WorkbenchHostWidget.h -o "%GEN%\moc_WorkbenchHostWidget.cpp"
echo [moc] WorkbenchModels
"%MOC%" %INC% ImageNodeEditor\gui\WorkbenchModels.h -o "%GEN%\moc_WorkbenchModels.cpp"
echo [rcc] workbench_qml
"%RCC%" --name workbench_qml ImageNodeEditor\qml\workbench.qrc -o "%GEN%\qrc_workbench_qml.cpp"
echo [rcc] codicons_assets
"%RCC%" --name codicons_assets third_party\codicons\codicons.qrc -o "%GEN%\qrc_codicons_assets.cpp"
echo Done.
endlocal
