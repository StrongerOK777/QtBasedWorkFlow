@echo off
REM ============================================================
REM Regenerate Qt moc/rcc files (ASCII-only comments on purpose:
REM non-ASCII comments can break under a mismatched console codepage).
REM Run this after editing a Q_OBJECT header (CanvasTabBar.h /
REM WorkbenchHostWidget.h / WorkbenchModels.h) or any qml resource,
REM then rebuild the solution in Visual Studio / MSBuild.
REM Qt path defaults to C:\Qt\6.8.3\msvc2022_64; override via QTDIR.
REM ============================================================
setlocal
if "%QTDIR%"=="" set "QTDIR=C:\Qt\6.8.3\msvc2022_64"
set "MOC=%QTDIR%\bin\moc.exe"
set "RCC=%QTDIR%\bin\rcc.exe"
REM cd to repo root (this script lives in ImageNodeEditor\GeneratedFiles\).
cd /d "%~dp0..\.."
REM NOTE: no surrounding quotes on INC, otherwise the inner path quotes break cmd parsing.
set INC=-I ImageNodeEditor -I third_party\QtNodes\include -I "%QTDIR%\include" -I "%QTDIR%\include\QtCore" -I "%QTDIR%\include\QtGui" -I "%QTDIR%\include\QtWidgets" -I "%QTDIR%\include\QtQml" -I "%QTDIR%\include\QtQuick" -I "%QTDIR%\include\QtQuickWidgets"
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
