/****************************************************************************
** Meta object code from reading C++ file 'WorkbenchModels.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../gui/WorkbenchModels.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WorkbenchModels.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN14WorkbenchThemeE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN14WorkbenchThemeE = QtMocHelpers::stringData(
    "WorkbenchTheme",
    "changed",
    "",
    "scaleChanged",
    "base",
    "panel",
    "elevated",
    "elevatedHover",
    "input",
    "hairline",
    "border",
    "textPrimary",
    "textSecondary",
    "textMuted",
    "accent",
    "accentHover",
    "selection",
    "selectionText",
    "onAccent",
    "warning",
    "danger",
    "scale"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN14WorkbenchThemeE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
      18,   28, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   26,    2, 0x06,   19 /* Public */,
       3,    0,   27,    2, 0x06,   20 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
       4, QMetaType::QColor, 0x00015001, uint(0), 0,
       5, QMetaType::QColor, 0x00015001, uint(0), 0,
       6, QMetaType::QColor, 0x00015001, uint(0), 0,
       7, QMetaType::QColor, 0x00015001, uint(0), 0,
       8, QMetaType::QColor, 0x00015001, uint(0), 0,
       9, QMetaType::QColor, 0x00015001, uint(0), 0,
      10, QMetaType::QColor, 0x00015001, uint(0), 0,
      11, QMetaType::QColor, 0x00015001, uint(0), 0,
      12, QMetaType::QColor, 0x00015001, uint(0), 0,
      13, QMetaType::QColor, 0x00015001, uint(0), 0,
      14, QMetaType::QColor, 0x00015001, uint(0), 0,
      15, QMetaType::QColor, 0x00015001, uint(0), 0,
      16, QMetaType::QColor, 0x00015001, uint(0), 0,
      17, QMetaType::QColor, 0x00015001, uint(0), 0,
      18, QMetaType::QColor, 0x00015001, uint(0), 0,
      19, QMetaType::QColor, 0x00015001, uint(0), 0,
      20, QMetaType::QColor, 0x00015001, uint(0), 0,
      21, QMetaType::Double, 0x00015001, uint(1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject WorkbenchTheme::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN14WorkbenchThemeE.offsetsAndSizes,
    qt_meta_data_ZN14WorkbenchThemeE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN14WorkbenchThemeE_t,
        // property 'base'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'panel'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'elevated'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'elevatedHover'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'input'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'hairline'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'border'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'textPrimary'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'textSecondary'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'textMuted'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'accent'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'accentHover'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'selection'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'selectionText'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'onAccent'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'warning'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'danger'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'scale'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<WorkbenchTheme, std::true_type>,
        // method 'changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'scaleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void WorkbenchTheme::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WorkbenchTheme *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->changed(); break;
        case 1: _t->scaleChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (WorkbenchTheme::*)();
            if (_q_method_type _q_method = &WorkbenchTheme::changed; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchTheme::*)();
            if (_q_method_type _q_method = &WorkbenchTheme::scaleChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QColor*>(_v) = _t->base(); break;
        case 1: *reinterpret_cast< QColor*>(_v) = _t->panel(); break;
        case 2: *reinterpret_cast< QColor*>(_v) = _t->elevated(); break;
        case 3: *reinterpret_cast< QColor*>(_v) = _t->elevatedHover(); break;
        case 4: *reinterpret_cast< QColor*>(_v) = _t->input(); break;
        case 5: *reinterpret_cast< QColor*>(_v) = _t->hairline(); break;
        case 6: *reinterpret_cast< QColor*>(_v) = _t->border(); break;
        case 7: *reinterpret_cast< QColor*>(_v) = _t->textPrimary(); break;
        case 8: *reinterpret_cast< QColor*>(_v) = _t->textSecondary(); break;
        case 9: *reinterpret_cast< QColor*>(_v) = _t->textMuted(); break;
        case 10: *reinterpret_cast< QColor*>(_v) = _t->accent(); break;
        case 11: *reinterpret_cast< QColor*>(_v) = _t->accentHover(); break;
        case 12: *reinterpret_cast< QColor*>(_v) = _t->selection(); break;
        case 13: *reinterpret_cast< QColor*>(_v) = _t->selectionText(); break;
        case 14: *reinterpret_cast< QColor*>(_v) = _t->onAccent(); break;
        case 15: *reinterpret_cast< QColor*>(_v) = _t->warning(); break;
        case 16: *reinterpret_cast< QColor*>(_v) = _t->danger(); break;
        case 17: *reinterpret_cast< double*>(_v) = _t->scale(); break;
        default: break;
        }
    }
}

const QMetaObject *WorkbenchTheme::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WorkbenchTheme::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN14WorkbenchThemeE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WorkbenchTheme::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void WorkbenchTheme::changed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void WorkbenchTheme::scaleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
namespace {
struct qt_meta_tag_ZN21WorkflowTemplateModelE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN21WorkflowTemplateModelE = QtMocHelpers::stringData(
    "WorkflowTemplateModel"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN21WorkflowTemplateModelE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject WorkflowTemplateModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_ZN21WorkflowTemplateModelE.offsetsAndSizes,
    qt_meta_data_ZN21WorkflowTemplateModelE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN21WorkflowTemplateModelE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<WorkflowTemplateModel, std::true_type>
    >,
    nullptr
} };

void WorkflowTemplateModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WorkflowTemplateModel *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *WorkflowTemplateModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WorkflowTemplateModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN21WorkflowTemplateModelE.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int WorkflowTemplateModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN23WorkflowCheckpointModelE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN23WorkflowCheckpointModelE = QtMocHelpers::stringData(
    "WorkflowCheckpointModel"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN23WorkflowCheckpointModelE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject WorkflowCheckpointModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_ZN23WorkflowCheckpointModelE.offsetsAndSizes,
    qt_meta_data_ZN23WorkflowCheckpointModelE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN23WorkflowCheckpointModelE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<WorkflowCheckpointModel, std::true_type>
    >,
    nullptr
} };

void WorkflowCheckpointModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WorkflowCheckpointModel *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *WorkflowCheckpointModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WorkflowCheckpointModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN23WorkflowCheckpointModelE.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int WorkflowCheckpointModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN24WorkbenchCommandRegistryE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN24WorkbenchCommandRegistryE = QtMocHelpers::stringData(
    "WorkbenchCommandRegistry",
    "trigger",
    "",
    "id"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN24WorkbenchCommandRegistryE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   20,    2, 0x02,    1 /* Public */,

 // methods: parameters
    QMetaType::Bool, QMetaType::QString,    3,

       0        // eod
};

Q_CONSTINIT const QMetaObject WorkbenchCommandRegistry::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_ZN24WorkbenchCommandRegistryE.offsetsAndSizes,
    qt_meta_data_ZN24WorkbenchCommandRegistryE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN24WorkbenchCommandRegistryE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<WorkbenchCommandRegistry, std::true_type>,
        // method 'trigger'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void WorkbenchCommandRegistry::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WorkbenchCommandRegistry *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { bool _r = _t->trigger((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

const QMetaObject *WorkbenchCommandRegistry::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WorkbenchCommandRegistry::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN24WorkbenchCommandRegistryE.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int WorkbenchCommandRegistry::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}
namespace {
struct qt_meta_tag_ZN16NodeCatalogModelE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN16NodeCatalogModelE = QtMocHelpers::stringData(
    "NodeCatalogModel",
    "filterTextChanged",
    "",
    "filterText"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN16NodeCatalogModelE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       1,   21, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   20,    2, 0x06,    2 /* Public */,

 // signals: parameters
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
       3, QMetaType::QString, 0x00015103, uint(0), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject NodeCatalogModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_ZN16NodeCatalogModelE.offsetsAndSizes,
    qt_meta_data_ZN16NodeCatalogModelE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN16NodeCatalogModelE_t,
        // property 'filterText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<NodeCatalogModel, std::true_type>,
        // method 'filterTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void NodeCatalogModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<NodeCatalogModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->filterTextChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (NodeCatalogModel::*)();
            if (_q_method_type _q_method = &NodeCatalogModel::filterTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->filterText(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setFilterText(*reinterpret_cast< QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *NodeCatalogModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NodeCatalogModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN16NodeCatalogModelE.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int NodeCatalogModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void NodeCatalogModel::filterTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
namespace {
struct qt_meta_tag_ZN20WorkflowOutlineModelE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN20WorkflowOutlineModelE = QtMocHelpers::stringData(
    "WorkflowOutlineModel",
    "filterTextChanged",
    "",
    "filterText"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN20WorkflowOutlineModelE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       1,   21, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   20,    2, 0x06,    2 /* Public */,

 // signals: parameters
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
       3, QMetaType::QString, 0x00015103, uint(0), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject WorkflowOutlineModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_ZN20WorkflowOutlineModelE.offsetsAndSizes,
    qt_meta_data_ZN20WorkflowOutlineModelE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN20WorkflowOutlineModelE_t,
        // property 'filterText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<WorkflowOutlineModel, std::true_type>,
        // method 'filterTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void WorkflowOutlineModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WorkflowOutlineModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->filterTextChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (WorkflowOutlineModel::*)();
            if (_q_method_type _q_method = &WorkflowOutlineModel::filterTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->filterText(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setFilterText(*reinterpret_cast< QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *WorkflowOutlineModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WorkflowOutlineModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN20WorkflowOutlineModelE.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int WorkflowOutlineModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void WorkflowOutlineModel::filterTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
namespace {
struct qt_meta_tag_ZN12ProblemModelE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN12ProblemModelE = QtMocHelpers::stringData(
    "ProblemModel"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN12ProblemModelE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject ProblemModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_ZN12ProblemModelE.offsetsAndSizes,
    qt_meta_data_ZN12ProblemModelE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN12ProblemModelE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ProblemModel, std::true_type>
    >,
    nullptr
} };

void ProblemModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ProblemModel *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *ProblemModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProblemModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN12ProblemModelE.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int ProblemModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN16QuickAccessModelE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN16QuickAccessModelE = QtMocHelpers::stringData(
    "QuickAccessModel",
    "queryChanged",
    "",
    "refresh",
    "query"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN16QuickAccessModelE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       1,   28, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   26,    2, 0x06,    2 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       3,    0,   27,    2, 0x02,    3 /* Public */,

 // signals: parameters
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
       4, QMetaType::QString, 0x00015103, uint(0), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject QuickAccessModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_ZN16QuickAccessModelE.offsetsAndSizes,
    qt_meta_data_ZN16QuickAccessModelE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN16QuickAccessModelE_t,
        // property 'query'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<QuickAccessModel, std::true_type>,
        // method 'queryChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void QuickAccessModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QuickAccessModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->queryChanged(); break;
        case 1: _t->refresh(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (QuickAccessModel::*)();
            if (_q_method_type _q_method = &QuickAccessModel::queryChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->query(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setQuery(*reinterpret_cast< QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *QuickAccessModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QuickAccessModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN16QuickAccessModelE.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int QuickAccessModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void QuickAccessModel::queryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
namespace {
struct qt_meta_tag_ZN15WorkbenchBridgeE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN15WorkbenchBridgeE = QtMocHelpers::stringData(
    "WorkbenchBridge",
    "documentTitleChanged",
    "",
    "statusTextChanged",
    "selectedNodeTextChanged",
    "zoomTextChanged",
    "activeSidebarChanged",
    "previewVisibleChanged",
    "panelVisibleChanged",
    "windowMaximizedChanged",
    "nodeCreationRequested",
    "typeName",
    "nodeFocusRequested",
    "nodeId",
    "recentWorkflowRequested",
    "path",
    "workflowTemplateSaveRequested",
    "workflowTemplateApplyRequested",
    "templateId",
    "workflowTemplateRenameRequested",
    "workflowTemplateDeleteRequested",
    "workflowTemplateExportRequested",
    "workflowTemplateImportRequested",
    "checkpointCreateRequested",
    "checkpointRestoreRequested",
    "checkpointId",
    "checkpointBranchRequested",
    "checkpointRenameRequested",
    "checkpointDeleteRequested",
    "checkpointExportRequested",
    "checkpointCompareRequested",
    "timelineRestoreRequested",
    "timelineId",
    "previewVisibilityRequested",
    "visible",
    "panelVisibilityRequested",
    "quickAccessRequested",
    "quickAccessFinished",
    "windowMoveRequested",
    "windowMinimizeRequested",
    "windowMaximizeToggleRequested",
    "windowCloseRequested",
    "headerMenuTitlesChanged",
    "headerMenuRequested",
    "index",
    "globalPos",
    "tooltipRequested",
    "text",
    "placement",
    "tooltipHideRequested",
    "createNode",
    "focusNode",
    "triggerCommand",
    "id",
    "showQuickAccess",
    "activateQuickAccess",
    "row",
    "startNodeDrag",
    "title",
    "category",
    "saveWorkflowTemplate",
    "applyWorkflowTemplate",
    "renameWorkflowTemplate",
    "deleteWorkflowTemplate",
    "exportWorkflowTemplate",
    "importWorkflowTemplate",
    "createCheckpoint",
    "restoreCheckpoint",
    "branchFromCheckpoint",
    "renameCheckpoint",
    "deleteCheckpoint",
    "exportCheckpoint",
    "compareCheckpoints",
    "restoreTimeline",
    "requestWindowMove",
    "requestWindowMinimize",
    "requestWindowMaximizeToggle",
    "requestWindowClose",
    "openHeaderMenu",
    "showTooltip",
    "hideTooltip",
    "documentTitle",
    "statusText",
    "selectedNodeText",
    "zoomText",
    "activeSidebar",
    "previewVisible",
    "panelVisible",
    "windowMaximized",
    "headerMenuTitles"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN15WorkbenchBridgeE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      64,   14, // methods
       9,  542, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      37,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  398,    2, 0x06,   10 /* Public */,
       3,    0,  399,    2, 0x06,   11 /* Public */,
       4,    0,  400,    2, 0x06,   12 /* Public */,
       5,    0,  401,    2, 0x06,   13 /* Public */,
       6,    0,  402,    2, 0x06,   14 /* Public */,
       7,    0,  403,    2, 0x06,   15 /* Public */,
       8,    0,  404,    2, 0x06,   16 /* Public */,
       9,    0,  405,    2, 0x06,   17 /* Public */,
      10,    1,  406,    2, 0x06,   18 /* Public */,
      12,    1,  409,    2, 0x06,   20 /* Public */,
      14,    1,  412,    2, 0x06,   22 /* Public */,
      16,    0,  415,    2, 0x06,   24 /* Public */,
      17,    1,  416,    2, 0x06,   25 /* Public */,
      19,    1,  419,    2, 0x06,   27 /* Public */,
      20,    1,  422,    2, 0x06,   29 /* Public */,
      21,    1,  425,    2, 0x06,   31 /* Public */,
      22,    0,  428,    2, 0x06,   33 /* Public */,
      23,    0,  429,    2, 0x06,   34 /* Public */,
      24,    1,  430,    2, 0x06,   35 /* Public */,
      26,    1,  433,    2, 0x06,   37 /* Public */,
      27,    1,  436,    2, 0x06,   39 /* Public */,
      28,    1,  439,    2, 0x06,   41 /* Public */,
      29,    1,  442,    2, 0x06,   43 /* Public */,
      30,    0,  445,    2, 0x06,   45 /* Public */,
      31,    1,  446,    2, 0x06,   46 /* Public */,
      33,    1,  449,    2, 0x06,   48 /* Public */,
      35,    1,  452,    2, 0x06,   50 /* Public */,
      36,    0,  455,    2, 0x06,   52 /* Public */,
      37,    0,  456,    2, 0x06,   53 /* Public */,
      38,    0,  457,    2, 0x06,   54 /* Public */,
      39,    0,  458,    2, 0x06,   55 /* Public */,
      40,    0,  459,    2, 0x06,   56 /* Public */,
      41,    0,  460,    2, 0x06,   57 /* Public */,
      42,    0,  461,    2, 0x06,   58 /* Public */,
      43,    2,  462,    2, 0x06,   59 /* Public */,
      46,    2,  467,    2, 0x06,   62 /* Public */,
      49,    0,  472,    2, 0x06,   65 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      50,    1,  473,    2, 0x02,   66 /* Public */,
      51,    1,  476,    2, 0x02,   68 /* Public */,
      52,    1,  479,    2, 0x02,   70 /* Public */,
      54,    0,  482,    2, 0x02,   72 /* Public */,
      55,    1,  483,    2, 0x02,   73 /* Public */,
      57,    3,  486,    2, 0x02,   75 /* Public */,
      60,    0,  493,    2, 0x02,   79 /* Public */,
      61,    1,  494,    2, 0x02,   80 /* Public */,
      62,    1,  497,    2, 0x02,   82 /* Public */,
      63,    1,  500,    2, 0x02,   84 /* Public */,
      64,    1,  503,    2, 0x02,   86 /* Public */,
      65,    0,  506,    2, 0x02,   88 /* Public */,
      66,    0,  507,    2, 0x02,   89 /* Public */,
      67,    1,  508,    2, 0x02,   90 /* Public */,
      68,    1,  511,    2, 0x02,   92 /* Public */,
      69,    1,  514,    2, 0x02,   94 /* Public */,
      70,    1,  517,    2, 0x02,   96 /* Public */,
      71,    1,  520,    2, 0x02,   98 /* Public */,
      72,    0,  523,    2, 0x02,  100 /* Public */,
      73,    1,  524,    2, 0x02,  101 /* Public */,
      74,    0,  527,    2, 0x02,  103 /* Public */,
      75,    0,  528,    2, 0x02,  104 /* Public */,
      76,    0,  529,    2, 0x02,  105 /* Public */,
      77,    0,  530,    2, 0x02,  106 /* Public */,
      78,    2,  531,    2, 0x02,  107 /* Public */,
      79,    2,  536,    2, 0x02,  110 /* Public */,
      80,    0,  541,    2, 0x02,  113 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   32,
    QMetaType::Void, QMetaType::Bool,   34,
    QMetaType::Void, QMetaType::Bool,   34,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QPointF,   44,   45,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   47,   48,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   53,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   56,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString,   11,   58,   59,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   32,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QPointF,   44,   45,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   47,   48,
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
      81, QMetaType::QString, 0x00015103, uint(0), 0,
      82, QMetaType::QString, 0x00015103, uint(1), 0,
      83, QMetaType::QString, 0x00015103, uint(2), 0,
      84, QMetaType::QString, 0x00015103, uint(3), 0,
      85, QMetaType::QString, 0x00015103, uint(4), 0,
      86, QMetaType::Bool, 0x00015103, uint(5), 0,
      87, QMetaType::Bool, 0x00015103, uint(6), 0,
      88, QMetaType::Bool, 0x00015001, uint(7), 0,
      89, QMetaType::QStringList, 0x00015103, uint(33), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject WorkbenchBridge::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN15WorkbenchBridgeE.offsetsAndSizes,
    qt_meta_data_ZN15WorkbenchBridgeE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN15WorkbenchBridgeE_t,
        // property 'documentTitle'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'statusText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'selectedNodeText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'zoomText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'activeSidebar'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'previewVisible'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'panelVisible'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'windowMaximized'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'headerMenuTitles'
        QtPrivate::TypeAndForceComplete<QStringList, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<WorkbenchBridge, std::true_type>,
        // method 'documentTitleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'statusTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'selectedNodeTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'zoomTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'activeSidebarChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'previewVisibleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'panelVisibleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'windowMaximizedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'nodeCreationRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'nodeFocusRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'recentWorkflowRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'workflowTemplateSaveRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'workflowTemplateApplyRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'workflowTemplateRenameRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'workflowTemplateDeleteRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'workflowTemplateExportRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'workflowTemplateImportRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'checkpointCreateRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'checkpointRestoreRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'checkpointBranchRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'checkpointRenameRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'checkpointDeleteRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'checkpointExportRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'checkpointCompareRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'timelineRestoreRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'previewVisibilityRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'panelVisibilityRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'quickAccessRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'quickAccessFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'windowMoveRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'windowMinimizeRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'windowMaximizeToggleRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'windowCloseRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'headerMenuTitlesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'headerMenuRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPointF &, std::false_type>,
        // method 'tooltipRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'tooltipHideRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'createNode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'focusNode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'triggerCommand'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'showQuickAccess'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'activateQuickAccess'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'startNodeDrag'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'saveWorkflowTemplate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'applyWorkflowTemplate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'renameWorkflowTemplate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'deleteWorkflowTemplate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'exportWorkflowTemplate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'importWorkflowTemplate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'createCheckpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'restoreCheckpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'branchFromCheckpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'renameCheckpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'deleteCheckpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'exportCheckpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'compareCheckpoints'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'restoreTimeline'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'requestWindowMove'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'requestWindowMinimize'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'requestWindowMaximizeToggle'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'requestWindowClose'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openHeaderMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPointF &, std::false_type>,
        // method 'showTooltip'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'hideTooltip'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void WorkbenchBridge::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WorkbenchBridge *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->documentTitleChanged(); break;
        case 1: _t->statusTextChanged(); break;
        case 2: _t->selectedNodeTextChanged(); break;
        case 3: _t->zoomTextChanged(); break;
        case 4: _t->activeSidebarChanged(); break;
        case 5: _t->previewVisibleChanged(); break;
        case 6: _t->panelVisibleChanged(); break;
        case 7: _t->windowMaximizedChanged(); break;
        case 8: _t->nodeCreationRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->nodeFocusRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->recentWorkflowRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->workflowTemplateSaveRequested(); break;
        case 12: _t->workflowTemplateApplyRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->workflowTemplateRenameRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->workflowTemplateDeleteRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->workflowTemplateExportRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->workflowTemplateImportRequested(); break;
        case 17: _t->checkpointCreateRequested(); break;
        case 18: _t->checkpointRestoreRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 19: _t->checkpointBranchRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 20: _t->checkpointRenameRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: _t->checkpointDeleteRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 22: _t->checkpointExportRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 23: _t->checkpointCompareRequested(); break;
        case 24: _t->timelineRestoreRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 25: _t->previewVisibilityRequested((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 26: _t->panelVisibilityRequested((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 27: _t->quickAccessRequested(); break;
        case 28: _t->quickAccessFinished(); break;
        case 29: _t->windowMoveRequested(); break;
        case 30: _t->windowMinimizeRequested(); break;
        case 31: _t->windowMaximizeToggleRequested(); break;
        case 32: _t->windowCloseRequested(); break;
        case 33: _t->headerMenuTitlesChanged(); break;
        case 34: _t->headerMenuRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[2]))); break;
        case 35: _t->tooltipRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 36: _t->tooltipHideRequested(); break;
        case 37: _t->createNode((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 38: _t->focusNode((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 39: _t->triggerCommand((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 40: _t->showQuickAccess(); break;
        case 41: _t->activateQuickAccess((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 42: _t->startNodeDrag((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 43: _t->saveWorkflowTemplate(); break;
        case 44: _t->applyWorkflowTemplate((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 45: _t->renameWorkflowTemplate((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 46: _t->deleteWorkflowTemplate((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 47: _t->exportWorkflowTemplate((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 48: _t->importWorkflowTemplate(); break;
        case 49: _t->createCheckpoint(); break;
        case 50: _t->restoreCheckpoint((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 51: _t->branchFromCheckpoint((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 52: _t->renameCheckpoint((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 53: _t->deleteCheckpoint((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 54: _t->exportCheckpoint((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 55: _t->compareCheckpoints(); break;
        case 56: _t->restoreTimeline((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 57: _t->requestWindowMove(); break;
        case 58: _t->requestWindowMinimize(); break;
        case 59: _t->requestWindowMaximizeToggle(); break;
        case 60: _t->requestWindowClose(); break;
        case 61: _t->openHeaderMenu((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[2]))); break;
        case 62: _t->showTooltip((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 63: _t->hideTooltip(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::documentTitleChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::statusTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::selectedNodeTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::zoomTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::activeSidebarChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::previewVisibleChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::panelVisibleChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::windowMaximizedChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::nodeCreationRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::nodeFocusRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::recentWorkflowRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::workflowTemplateSaveRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::workflowTemplateApplyRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::workflowTemplateRenameRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::workflowTemplateDeleteRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 14;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::workflowTemplateExportRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 15;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::workflowTemplateImportRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 16;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::checkpointCreateRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 17;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::checkpointRestoreRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 18;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::checkpointBranchRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 19;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::checkpointRenameRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 20;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::checkpointDeleteRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 21;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::checkpointExportRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 22;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::checkpointCompareRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 23;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::timelineRestoreRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 24;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(bool );
            if (_q_method_type _q_method = &WorkbenchBridge::previewVisibilityRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 25;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(bool );
            if (_q_method_type _q_method = &WorkbenchBridge::panelVisibilityRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 26;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::quickAccessRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 27;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::quickAccessFinished; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 28;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::windowMoveRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 29;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::windowMinimizeRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 30;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::windowMaximizeToggleRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 31;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::windowCloseRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 32;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::headerMenuTitlesChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 33;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(int , const QPointF & );
            if (_q_method_type _q_method = &WorkbenchBridge::headerMenuRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 34;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & , const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::tooltipRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 35;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::tooltipHideRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 36;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->documentTitle(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->statusText(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->selectedNodeText(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->zoomText(); break;
        case 4: *reinterpret_cast< QString*>(_v) = _t->activeSidebar(); break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->previewVisible(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->panelVisible(); break;
        case 7: *reinterpret_cast< bool*>(_v) = _t->windowMaximized(); break;
        case 8: *reinterpret_cast< QStringList*>(_v) = _t->headerMenuTitles(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setDocumentTitle(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->setStatusText(*reinterpret_cast< QString*>(_v)); break;
        case 2: _t->setSelectedNodeText(*reinterpret_cast< QString*>(_v)); break;
        case 3: _t->setZoomText(*reinterpret_cast< QString*>(_v)); break;
        case 4: _t->setActiveSidebar(*reinterpret_cast< QString*>(_v)); break;
        case 5: _t->setPreviewVisible(*reinterpret_cast< bool*>(_v)); break;
        case 6: _t->setPanelVisible(*reinterpret_cast< bool*>(_v)); break;
        case 8: _t->setHeaderMenuTitles(*reinterpret_cast< QStringList*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *WorkbenchBridge::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WorkbenchBridge::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN15WorkbenchBridgeE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WorkbenchBridge::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 64)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 64;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 64)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 64;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void WorkbenchBridge::documentTitleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void WorkbenchBridge::statusTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void WorkbenchBridge::selectedNodeTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void WorkbenchBridge::zoomTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void WorkbenchBridge::activeSidebarChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void WorkbenchBridge::previewVisibleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void WorkbenchBridge::panelVisibleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void WorkbenchBridge::windowMaximizedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void WorkbenchBridge::nodeCreationRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void WorkbenchBridge::nodeFocusRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void WorkbenchBridge::recentWorkflowRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void WorkbenchBridge::workflowTemplateSaveRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void WorkbenchBridge::workflowTemplateApplyRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void WorkbenchBridge::workflowTemplateRenameRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void WorkbenchBridge::workflowTemplateDeleteRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void WorkbenchBridge::workflowTemplateExportRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void WorkbenchBridge::workflowTemplateImportRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 16, nullptr);
}

// SIGNAL 17
void WorkbenchBridge::checkpointCreateRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 17, nullptr);
}

// SIGNAL 18
void WorkbenchBridge::checkpointRestoreRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 18, _a);
}

// SIGNAL 19
void WorkbenchBridge::checkpointBranchRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 19, _a);
}

// SIGNAL 20
void WorkbenchBridge::checkpointRenameRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 20, _a);
}

// SIGNAL 21
void WorkbenchBridge::checkpointDeleteRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 21, _a);
}

// SIGNAL 22
void WorkbenchBridge::checkpointExportRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 22, _a);
}

// SIGNAL 23
void WorkbenchBridge::checkpointCompareRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 23, nullptr);
}

// SIGNAL 24
void WorkbenchBridge::timelineRestoreRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 24, _a);
}

// SIGNAL 25
void WorkbenchBridge::previewVisibilityRequested(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 25, _a);
}

// SIGNAL 26
void WorkbenchBridge::panelVisibilityRequested(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 26, _a);
}

// SIGNAL 27
void WorkbenchBridge::quickAccessRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 27, nullptr);
}

// SIGNAL 28
void WorkbenchBridge::quickAccessFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 28, nullptr);
}

// SIGNAL 29
void WorkbenchBridge::windowMoveRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 29, nullptr);
}

// SIGNAL 30
void WorkbenchBridge::windowMinimizeRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 30, nullptr);
}

// SIGNAL 31
void WorkbenchBridge::windowMaximizeToggleRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 31, nullptr);
}

// SIGNAL 32
void WorkbenchBridge::windowCloseRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 32, nullptr);
}

// SIGNAL 33
void WorkbenchBridge::headerMenuTitlesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 33, nullptr);
}

// SIGNAL 34
void WorkbenchBridge::headerMenuRequested(int _t1, const QPointF & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 34, _a);
}

// SIGNAL 35
void WorkbenchBridge::tooltipRequested(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 35, _a);
}

// SIGNAL 36
void WorkbenchBridge::tooltipHideRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 36, nullptr);
}
QT_WARNING_POP
