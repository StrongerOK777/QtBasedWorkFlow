#include "util/PathUtils.h"

#include <QDir>
#include <QFileInfo>

namespace PathUtils {

QString resolveAgainstFile(const QString& path, const QString& workflowFile)
{
    if (path.isEmpty() || QFileInfo(path).isAbsolute() || workflowFile.isEmpty()) {
        return path;
    }
    return QDir(QFileInfo(workflowFile).absolutePath()).absoluteFilePath(path);
}

QString relativeToFile(const QString& path, const QString& workflowFile)
{
    if (path.isEmpty() || workflowFile.isEmpty() || !QFileInfo(path).isAbsolute()) {
        return path;
    }
    return QDir(QFileInfo(workflowFile).absolutePath()).relativeFilePath(path);
}

}
