#pragma once

#include <QString>

namespace PathUtils {

QString resolveAgainstFile(const QString& path, const QString& workflowFile);
QString relativeToFile(const QString& path, const QString& workflowFile);

}
