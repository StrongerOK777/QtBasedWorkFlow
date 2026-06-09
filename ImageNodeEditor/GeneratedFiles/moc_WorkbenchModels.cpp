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
    "checkpointCreateRequested",
    "checkpointRestoreRequested",
    "checkpointId",
    "checkpointBranchRequested",
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
    "createCheckpoint",
    "restoreCheckpoint",
    "branchFromCheckpoint",
    "restoreTimeline",
    "requestWindowMove",
    "requestWindowMinimize",
    "requestWindowMaximizeToggle",
    "requestWindowClose",
    "showTooltip",
    "hideTooltip",
    "documentTitle",
    "statusText",
    "selectedNodeText",
    "zoomText",
    "activeSidebar",
    "previewVisible",
    "panelVisible",
    "windowMaximized"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN15WorkbenchBridgeE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      45,   14, // methods
       8,  377, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      27,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  284,    2, 0x06,    9 /* Public */,
       3,    0,  285,    2, 0x06,   10 /* Public */,
       4,    0,  286,    2, 0x06,   11 /* Public */,
       5,    0,  287,    2, 0x06,   12 /* Public */,
       6,    0,  288,    2, 0x06,   13 /* Public */,
       7,    0,  289,    2, 0x06,   14 /* Public */,
       8,    0,  290,    2, 0x06,   15 /* Public */,
       9,    0,  291,    2, 0x06,   16 /* Public */,
      10,    1,  292,    2, 0x06,   17 /* Public */,
      12,    1,  295,    2, 0x06,   19 /* Public */,
      14,    1,  298,    2, 0x06,   21 /* Public */,
      16,    0,  301,    2, 0x06,   23 /* Public */,
      17,    1,  302,    2, 0x06,   24 /* Public */,
      19,    0,  305,    2, 0x06,   26 /* Public */,
      20,    1,  306,    2, 0x06,   27 /* Public */,
      22,    1,  309,    2, 0x06,   29 /* Public */,
      23,    1,  312,    2, 0x06,   31 /* Public */,
      25,    1,  315,    2, 0x06,   33 /* Public */,
      27,    1,  318,    2, 0x06,   35 /* Public */,
      28,    0,  321,    2, 0x06,   37 /* Public */,
      29,    0,  322,    2, 0x06,   38 /* Public */,
      30,    0,  323,    2, 0x06,   39 /* Public */,
      31,    0,  324,    2, 0x06,   40 /* Public */,
      32,    0,  325,    2, 0x06,   41 /* Public */,
      33,    0,  326,    2, 0x06,   42 /* Public */,
      34,    2,  327,    2, 0x06,   43 /* Public */,
      37,    0,  332,    2, 0x06,   46 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      38,    1,  333,    2, 0x02,   47 /* Public */,
      39,    1,  336,    2, 0x02,   49 /* Public */,
      40,    1,  339,    2, 0x02,   51 /* Public */,
      42,    0,  342,    2, 0x02,   53 /* Public */,
      43,    1,  343,    2, 0x02,   54 /* Public */,
      45,    3,  346,    2, 0x02,   56 /* Public */,
      48,    0,  353,    2, 0x02,   60 /* Public */,
      49,    1,  354,    2, 0x02,   61 /* Public */,
      50,    0,  357,    2, 0x02,   63 /* Public */,
      51,    1,  358,    2, 0x02,   64 /* Public */,
      52,    1,  361,    2, 0x02,   66 /* Public */,
      53,    1,  364,    2, 0x02,   68 /* Public */,
      54,    0,  367,    2, 0x02,   70 /* Public */,
      55,    0,  368,    2, 0x02,   71 /* Public */,
      56,    0,  369,    2, 0x02,   72 /* Public */,
      57,    0,  370,    2, 0x02,   73 /* Public */,
      58,    2,  371,    2, 0x02,   74 /* Public */,
      59,    0,  376,    2, 0x02,   77 /* Public */,

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
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   21,
    QMetaType::Void, QMetaType::QString,   21,
    QMetaType::Void, QMetaType::QString,   24,
    QMetaType::Void, QMetaType::Bool,   26,
    QMetaType::Void, QMetaType::Bool,   26,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   35,   36,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   41,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   44,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString,   11,   46,   47,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   21,
    QMetaType::Void, QMetaType::QString,   21,
    QMetaType::Void, QMetaType::QString,   24,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   35,   36,
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
      60, QMetaType::QString, 0x00015103, uint(0), 0,
      61, QMetaType::QString, 0x00015103, uint(1), 0,
      62, QMetaType::QString, 0x00015103, uint(2), 0,
      63, QMetaType::QString, 0x00015103, uint(3), 0,
      64, QMetaType::QString, 0x00015103, uint(4), 0,
      65, QMetaType::Bool, 0x00015103, uint(5), 0,
      66, QMetaType::Bool, 0x00015103, uint(6), 0,
      67, QMetaType::Bool, 0x00015001, uint(7), 0,

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
        // method 'checkpointCreateRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'checkpointRestoreRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'checkpointBranchRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
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
        // method 'createCheckpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'restoreCheckpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'branchFromCheckpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
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
        case 13: _t->checkpointCreateRequested(); break;
        case 14: _t->checkpointRestoreRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->checkpointBranchRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->timelineRestoreRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->previewVisibilityRequested((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 18: _t->panelVisibilityRequested((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 19: _t->quickAccessRequested(); break;
        case 20: _t->quickAccessFinished(); break;
        case 21: _t->windowMoveRequested(); break;
        case 22: _t->windowMinimizeRequested(); break;
        case 23: _t->windowMaximizeToggleRequested(); break;
        case 24: _t->windowCloseRequested(); break;
        case 25: _t->tooltipRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 26: _t->tooltipHideRequested(); break;
        case 27: _t->createNode((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 28: _t->focusNode((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 29: _t->triggerCommand((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 30: _t->showQuickAccess(); break;
        case 31: _t->activateQuickAccess((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 32: _t->startNodeDrag((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 33: _t->saveWorkflowTemplate(); break;
        case 34: _t->applyWorkflowTemplate((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 35: _t->createCheckpoint(); break;
        case 36: _t->restoreCheckpoint((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 37: _t->branchFromCheckpoint((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 38: _t->restoreTimeline((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 39: _t->requestWindowMove(); break;
        case 40: _t->requestWindowMinimize(); break;
        case 41: _t->requestWindowMaximizeToggle(); break;
        case 42: _t->requestWindowClose(); break;
        case 43: _t->showTooltip((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 44: _t->hideTooltip(); break;
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
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::checkpointCreateRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::checkpointRestoreRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 14;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::checkpointBranchRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 15;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::timelineRestoreRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 16;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(bool );
            if (_q_method_type _q_method = &WorkbenchBridge::previewVisibilityRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 17;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(bool );
            if (_q_method_type _q_method = &WorkbenchBridge::panelVisibilityRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 18;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::quickAccessRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 19;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::quickAccessFinished; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 20;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::windowMoveRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 21;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::windowMinimizeRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 22;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::windowMaximizeToggleRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 23;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::windowCloseRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 24;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)(const QString & , const QString & );
            if (_q_method_type _q_method = &WorkbenchBridge::tooltipRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 25;
                return;
            }
        }
        {
            using _q_method_type = void (WorkbenchBridge::*)();
            if (_q_method_type _q_method = &WorkbenchBridge::tooltipHideRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 26;
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
        if (_id < 45)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 45;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 45)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 45;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
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
void WorkbenchBridge::checkpointCreateRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void WorkbenchBridge::checkpointRestoreRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void WorkbenchBridge::checkpointBranchRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void WorkbenchBridge::timelineRestoreRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void WorkbenchBridge::previewVisibilityRequested(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}

// SIGNAL 18
void WorkbenchBridge::panelVisibilityRequested(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 18, _a);
}

// SIGNAL 19
void WorkbenchBridge::quickAccessRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 19, nullptr);
}

// SIGNAL 20
void WorkbenchBridge::quickAccessFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 20, nullptr);
}

// SIGNAL 21
void WorkbenchBridge::windowMoveRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 21, nullptr);
}

// SIGNAL 22
void WorkbenchBridge::windowMinimizeRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 22, nullptr);
}

// SIGNAL 23
void WorkbenchBridge::windowMaximizeToggleRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 23, nullptr);
}

// SIGNAL 24
void WorkbenchBridge::windowCloseRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 24, nullptr);
}

// SIGNAL 25
void WorkbenchBridge::tooltipRequested(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 25, _a);
}

// SIGNAL 26
void WorkbenchBridge::tooltipHideRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 26, nullptr);
}
QT_WARNING_POP
