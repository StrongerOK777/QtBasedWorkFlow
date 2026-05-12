#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QString>

#if defined(IMAGENODEEDITOR_USE_ELA)
#include "ElaApplication.h"
#include "ElaCheckBox.h"
#include "ElaComboBox.h"
#include "ElaDockWidget.h"
#include "ElaDoubleSpinBox.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "ElaSpinBox.h"
#include "ElaWindow.h"
#endif

namespace GuiCompat {

#if defined(IMAGENODEEDITOR_USE_ELA)
using MainWindowBase = ElaWindow;
using DockWidget = ElaDockWidget;
using PushButton = ElaPushButton;
using SpinBox = ElaSpinBox;
using DoubleSpinBox = ElaDoubleSpinBox;
using CheckBox = ElaCheckBox;
using ComboBox = ElaComboBox;
using LineEdit = ElaLineEdit;

inline constexpr bool usingEla()
{
    return true;
}

inline void initializeApplication()
{
    eApp->init();
}

inline void configureMainWindow(ElaWindow* window)
{
    window->setUserInfoCardVisible(false);
    window->setIsNavigationBarEnable(false);
    window->setIsCentralStackedWidgetTransparent(true);
    window->setWindowButtonFlags(ElaAppBarType::CloseButtonHint);
}

inline void setMainContent(ElaWindow* window, QWidget* widget)
{
    window->setCentralCustomWidget(widget);
}
#else
using MainWindowBase = QMainWindow;
using DockWidget = QDockWidget;
using PushButton = QPushButton;
using SpinBox = QSpinBox;
using DoubleSpinBox = QDoubleSpinBox;
using CheckBox = QCheckBox;
using ComboBox = QComboBox;
using LineEdit = QLineEdit;

inline constexpr bool usingEla()
{
    return false;
}

inline void initializeApplication()
{
}

inline void configureMainWindow(QMainWindow*)
{
}

inline void setMainContent(QMainWindow* window, QWidget* widget)
{
    window->setCentralWidget(widget);
}
#endif

}
