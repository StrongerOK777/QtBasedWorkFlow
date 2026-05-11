#pragma once

#include "PortType.h"

#include <QVariant>

struct NodeData {
    PortType type = PortType::ImageRGBA;
    QVariant value;
};
