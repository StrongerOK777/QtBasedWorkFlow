#pragma once

#include <QVariant>
#include <QString>
#include <QStringList>

enum class ParameterType {
    Integer,
    Double,
    Text,
    FileOpen,
    FileSave,
    Directory,
    Choice,
    Boolean,
    Color
};

struct NodeParameter {
    QString name;
    QString displayName;
    ParameterType type = ParameterType::Text;
    QVariant defaultValue;
    double min = 0.0;
    double max = 0.0;
    QStringList options;
    bool required = false;
};
