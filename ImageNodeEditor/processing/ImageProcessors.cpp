#include "processing/ImageProcessors.h"

#include <QPainter>
#include <QTransform>
#include <QtGlobal>
#include <algorithm>

namespace {

constexpr int kMaxPixels = 40 * 1000 * 1000;

Result<QImage> requireImage(const QImage& image)
{
    if (image.isNull()) {
        return Result<QImage>::fail("输入图片为空");
    }
    if (qint64(image.width()) * image.height() > kMaxPixels) {
        return Result<QImage>::fail("图片过大，基础版本拒绝处理超过 4000 万像素的图片");
    }
    return Result<QImage>::ok(image);
}

int clampByte(int value)
{
    return std::max(0, std::min(255, value));
}

}

namespace ImageProcessors {

Result<QImage> crop(const QImage& image, int x, int y, int width, int height)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (width <= 0 || height <= 0) {
        return Result<QImage>::fail("裁切宽高必须大于 0");
    }
    QRect rect(x, y, width, height);
    QRect bounds(0, 0, image.width(), image.height());
    QRect clipped = rect.intersected(bounds);
    if (clipped.isEmpty()) {
        return Result<QImage>::fail("裁切区域不在图片范围内");
    }
    return Result<QImage>::ok(image.copy(clipped));
}

Result<QImage> resize(const QImage& image, int width, int height, bool keepAspect)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (width <= 0 || height <= 0 || width * height > kMaxPixels) {
        return Result<QImage>::fail("缩放尺寸非法");
    }
    const auto mode = keepAspect ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio;
    return Result<QImage>::ok(image.scaled(width, height, mode, Qt::SmoothTransformation));
}

Result<QImage> grayscale(const QImage& image)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    return Result<QImage>::ok(image.convertToFormat(QImage::Format_Grayscale8).convertToFormat(QImage::Format_ARGB32));
}

Result<QImage> brightnessContrast(const QImage& image, int brightness, double contrast)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (brightness < -255 || brightness > 255 || contrast < 0.0 || contrast > 4.0) {
        return Result<QImage>::fail("亮度或对比度参数超出范围");
    }

    QImage out = image.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < out.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < out.width(); ++x) {
            QColor c(line[x]);
            const int r = clampByte(int((c.red() - 128) * contrast + 128 + brightness));
            const int g = clampByte(int((c.green() - 128) * contrast + 128 + brightness));
            const int b = clampByte(int((c.blue() - 128) * contrast + 128 + brightness));
            line[x] = qRgba(r, g, b, c.alpha());
        }
    }
    return Result<QImage>::ok(out);
}

Result<QImage> boxBlur(const QImage& image, int radius)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (radius < 0 || radius > 20) {
        return Result<QImage>::fail("模糊半径必须在 0 到 20 之间");
    }
    if (radius == 0) {
        return Result<QImage>::ok(image);
    }

    QImage src = image.convertToFormat(QImage::Format_ARGB32);
    QImage tmp(src.size(), QImage::Format_ARGB32);
    QImage out(src.size(), QImage::Format_ARGB32);

    for (int y = 0; y < src.height(); ++y) {
        auto* dst = reinterpret_cast<QRgb*>(tmp.scanLine(y));
        for (int x = 0; x < src.width(); ++x) {
            int r = 0, g = 0, b = 0, a = 0, count = 0;
            for (int xx = std::max(0, x - radius); xx <= std::min(src.width() - 1, x + radius); ++xx) {
                QColor c(src.pixel(xx, y));
                r += c.red(); g += c.green(); b += c.blue(); a += c.alpha(); ++count;
            }
            dst[x] = qRgba(r / count, g / count, b / count, a / count);
        }
    }
    for (int y = 0; y < src.height(); ++y) {
        auto* dst = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < src.width(); ++x) {
            int r = 0, g = 0, b = 0, a = 0, count = 0;
            for (int yy = std::max(0, y - radius); yy <= std::min(src.height() - 1, y + radius); ++yy) {
                QColor c(tmp.pixel(x, yy));
                r += c.red(); g += c.green(); b += c.blue(); a += c.alpha(); ++count;
            }
            dst[x] = qRgba(r / count, g / count, b / count, a / count);
        }
    }
    return Result<QImage>::ok(out);
}

Result<QImage> rotateFlip(const QImage& image, int angle, bool flipHorizontal, bool flipVertical)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (angle % 90 != 0) {
        return Result<QImage>::fail("旋转角度必须是 90 的倍数");
    }
    QTransform transform;
    transform.rotate(angle);
    QImage out = image.transformed(transform, Qt::SmoothTransformation);
    if (flipHorizontal || flipVertical) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 11, 0)
        Qt::Orientations orientations;
        if (flipHorizontal) orientations |= Qt::Horizontal;
        if (flipVertical) orientations |= Qt::Vertical;
        out = out.flipped(orientations);
#else
        out = out.mirrored(flipHorizontal, flipVertical);
#endif
    }
    return Result<QImage>::ok(out);
}

Result<QImage> textOverlay(const QImage& image, const QString& text, int x, int y, int size, const QColor& color)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (size <= 0 || size > 256) {
        return Result<QImage>::fail("文字字号非法");
    }
    QImage out = image.convertToFormat(QImage::Format_ARGB32);
    QPainter painter(&out);
    painter.setRenderHint(QPainter::Antialiasing);
    QFont font = painter.font();
    font.setPointSize(size);
    painter.setFont(font);
    painter.setPen(color);
    painter.drawText(QPoint(x, y), text);
    return Result<QImage>::ok(out);
}

Result<QImage> blend(const QImage& first, const QImage& second, double opacity)
{
    auto valid = requireImage(first);
    if (valid.isFail()) return valid;
    valid = requireImage(second);
    if (valid.isFail()) return valid;
    if (opacity < 0.0 || opacity > 1.0) {
        return Result<QImage>::fail("混合透明度必须在 0 到 1 之间");
    }
    QImage base = first.convertToFormat(QImage::Format_ARGB32);
    QImage top = second.scaled(base.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                     .convertToFormat(QImage::Format_ARGB32);
    QPainter painter(&base);
    painter.setOpacity(opacity);
    painter.drawImage(0, 0, top);
    return Result<QImage>::ok(base);
}

Result<QImage> merge(const QVector<QImage>& images, const QString& mode)
{
    QVector<QImage> validImages;
    for (const auto& image : images) {
        if (!image.isNull()) validImages.append(image.convertToFormat(QImage::Format_ARGB32));
    }
    if (validImages.isEmpty()) {
        return Result<QImage>::fail("拼接节点没有有效输入图片");
    }

    const bool vertical = mode == "vertical";
    int width = 0;
    int height = 0;
    for (const auto& image : validImages) {
        if (vertical) {
            width = std::max(width, image.width());
            height += image.height();
        } else {
            width += image.width();
            height = std::max(height, image.height());
        }
    }
    if (qint64(width) * height > kMaxPixels) {
        return Result<QImage>::fail("拼接结果过大");
    }

    QImage out(QSize(width, height), QImage::Format_ARGB32);
    out.fill(Qt::transparent);
    QPainter painter(&out);
    int offset = 0;
    for (const auto& image : validImages) {
        if (vertical) {
            painter.drawImage(0, offset, image);
            offset += image.height();
        } else {
            painter.drawImage(offset, 0, image);
            offset += image.width();
        }
    }
    return Result<QImage>::ok(out);
}

}
