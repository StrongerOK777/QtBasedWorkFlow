#pragma once

#include <QString>
#include <QVector>

enum class PortType {
    ImageRGBA,
    ImageGray,
    Number,
    Text,
    Mask,
    ImageList
};

enum class PortDirection {
    Input,
    Output
};

struct PortInfo {
    QString name;
    QString displayName;
    PortDirection direction = PortDirection::Input;
    PortType type = PortType::ImageRGBA;
    bool required = true;
    bool allowMultipleConnections = false;
};

inline QString portTypeName(PortType type)
{
    switch (type) {
    case PortType::ImageRGBA: return "ImageRGBA";
    case PortType::ImageGray: return "ImageGray";
    case PortType::Number: return "Number";
    case PortType::Text: return "Text";
    case PortType::Mask: return "Mask";
    case PortType::ImageList: return "ImageList";
    }
    return "Unknown";
}

inline bool portTypesCompatible(PortType from, PortType to)
{
    if (from == to) {
        return true;
    }
    return (from == PortType::ImageRGBA && to == PortType::ImageGray)
        || (from == PortType::ImageGray && to == PortType::ImageRGBA);
}
