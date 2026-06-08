#include "util/PathUtils.h"

#include <QDir>
#include <QFileInfo>

namespace PathUtils {

QString resolveAgainstFile(const QString& path, const QString& workflowFile)
{
    if (path.isEmpty() || QFileInfo(path).isAbsolute() || workflowFile.isEmpty()) {
        return path;
    }
    // cleanPath 先在字符串层面消去 ".." 段，避免出现 "/tmp/../Users/..." 这类路径——
    // 否则操作系统会先解析 /tmp 符号链接再处理 ..，落到错误的目录。
    return QDir::cleanPath(QDir(QFileInfo(workflowFile).absolutePath()).absoluteFilePath(path));
}

QString relativeToFile(const QString& path, const QString& workflowFile)
{
    if (path.isEmpty() || workflowFile.isEmpty() || !QFileInfo(path).isAbsolute()) {
        return path;
    }
    return QDir(QFileInfo(workflowFile).absolutePath()).relativeFilePath(path);
}

}
