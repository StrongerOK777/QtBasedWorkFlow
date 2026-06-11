#include "processing/ImageProcessors.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QTransform>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

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

    // 预计算 256 项查找表，循环内只做查表，避免逐像素构造 QColor 的开销。
    uchar lut[256];
    for (int i = 0; i < 256; ++i) {
        lut[i] = static_cast<uchar>(clampByte(int((i - 128) * contrast + 128 + brightness)));
    }

    QImage out = image.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < out.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < out.width(); ++x) {
            const QRgb p = line[x];
            line[x] = qRgba(lut[qRed(p)], lut[qGreen(p)], lut[qBlue(p)], qAlpha(p));
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

    // 两次一维滑动窗口均值（水平 + 垂直），整体 O(w*h)，与半径无关；
    // 直接用 scanLine 读写，避免 QImage::pixel() / QColor 的逐像素开销。
    QImage src = image.convertToFormat(QImage::Format_ARGB32);
    QImage tmp(src.size(), QImage::Format_ARGB32);
    QImage out(src.size(), QImage::Format_ARGB32);
    const int w = src.width();
    const int h = src.height();

    // 水平方向：每行维护 [x-radius, x+radius] 的累加窗口。
    for (int y = 0; y < h; ++y) {
        const auto* srcLine = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        auto* dstLine = reinterpret_cast<QRgb*>(tmp.scanLine(y));
        qint64 r = 0, g = 0, b = 0, a = 0;
        int count = std::min(radius, w - 1) + 1;
        for (int x = 0; x < count; ++x) {
            const QRgb p = srcLine[x];
            r += qRed(p); g += qGreen(p); b += qBlue(p); a += qAlpha(p);
        }
        for (int x = 0; x < w; ++x) {
            dstLine[x] = qRgba(int(r / count), int(g / count), int(b / count), int(a / count));
            const int add = x + radius + 1;
            if (add < w) {
                const QRgb p = srcLine[add];
                r += qRed(p); g += qGreen(p); b += qBlue(p); a += qAlpha(p);
                ++count;
            }
            const int remove = x - radius;
            if (remove >= 0) {
                const QRgb p = srcLine[remove];
                r -= qRed(p); g -= qGreen(p); b -= qBlue(p); a -= qAlpha(p);
                --count;
            }
        }
    }

    // 垂直方向：每列维护 [y-radius, y+radius] 的累加窗口。
    const int tmpStride = tmp.bytesPerLine() / int(sizeof(QRgb));
    const int outStride = out.bytesPerLine() / int(sizeof(QRgb));
    const auto* tmpBase = reinterpret_cast<const QRgb*>(tmp.constScanLine(0));
    auto* outBase = reinterpret_cast<QRgb*>(out.scanLine(0));
    for (int x = 0; x < w; ++x) {
        qint64 r = 0, g = 0, b = 0, a = 0;
        int count = std::min(radius, h - 1) + 1;
        for (int y = 0; y < count; ++y) {
            const QRgb p = tmpBase[y * tmpStride + x];
            r += qRed(p); g += qGreen(p); b += qBlue(p); a += qAlpha(p);
        }
        for (int y = 0; y < h; ++y) {
            outBase[y * outStride + x] = qRgba(int(r / count), int(g / count), int(b / count), int(a / count));
            const int add = y + radius + 1;
            if (add < h) {
                const QRgb p = tmpBase[add * tmpStride + x];
                r += qRed(p); g += qGreen(p); b += qBlue(p); a += qAlpha(p);
                ++count;
            }
            const int remove = y - radius;
            if (remove >= 0) {
                const QRgb p = tmpBase[remove * tmpStride + x];
                r -= qRed(p); g -= qGreen(p); b -= qBlue(p); a -= qAlpha(p);
                --count;
            }
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

Result<QImage> textOverlay(const QImage& image, const QString& text, int x, int y, int size, const QColor& color,
                           const QString& fontFamily, bool bold, const QString& anchor, double opacity,
                           bool outline, const QColor& outlineColor)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (size <= 0 || size > 256) {
        return Result<QImage>::fail("文字字号非法");
    }
    if (opacity < 0.0 || opacity > 1.0) {
        return Result<QImage>::fail("文字不透明度必须在 0 到 1 之间");
    }
    QImage out = image.convertToFormat(QImage::Format_ARGB32);
    if (text.isEmpty()) {
        return Result<QImage>::ok(out);
    }
    QPainter painter(&out);
    painter.setRenderHint(QPainter::Antialiasing);
    QFont font = painter.font();
    if (!fontFamily.trimmed().isEmpty()) {
        font.setFamily(fontFamily.trimmed());
    }
    // 用 pixelSize 而不是 pointSize：与图片 DPI 元数据无关，跨平台 / 跨图片结果一致。
    font.setPixelSize(size);
    font.setBold(bold);
    painter.setFont(font);
    painter.setOpacity(opacity);

    const QFontMetrics metrics(font);
    const int textWidth = metrics.horizontalAdvance(text);
    const int margin = 12;
    QPoint baseline(x, y);
    if (anchor == "topLeft") {
        baseline = QPoint(margin + x, margin + metrics.ascent() + y);
    } else if (anchor == "topRight") {
        baseline = QPoint(out.width() - textWidth - margin + x, margin + metrics.ascent() + y);
    } else if (anchor == "bottomLeft") {
        baseline = QPoint(margin + x, out.height() - margin - metrics.descent() + y);
    } else if (anchor == "bottomRight") {
        baseline = QPoint(out.width() - textWidth - margin + x, out.height() - margin - metrics.descent() + y);
    } else if (anchor == "center") {
        baseline = QPoint((out.width() - textWidth) / 2 + x, (out.height() + metrics.ascent() - metrics.descent()) / 2 + y);
    }
    // anchor == "custom"（默认）：保持旧行为，(x, y) 即文字基线位置。

    if (outline) {
        QPainterPath path;
        path.addText(baseline, font, text);
        painter.setPen(QPen(outlineColor, std::max(1.0, size / 12.0)));
        painter.setBrush(color);
        painter.drawPath(path);
    } else {
        painter.setPen(color);
        painter.drawText(baseline, text);
    }
    return Result<QImage>::ok(out);
}

Result<QImage> blend(const QImage& first, const QImage& second, double opacity, const QString& mode,
                     const QString& sizeMode)
{
    auto valid = requireImage(first);
    if (valid.isFail()) return valid;
    valid = requireImage(second);
    if (valid.isFail()) return valid;
    if (opacity < 0.0 || opacity > 1.0) {
        return Result<QImage>::fail("混合透明度必须在 0 到 1 之间");
    }

    QImage base = first.convertToFormat(QImage::Format_ARGB32);
    QImage top;
    if (sizeMode == "error") {
        if (second.size() != base.size()) {
            return Result<QImage>::fail(QString("混合节点两张图片尺寸不一致：%1x%2 与 %3x%4")
                                            .arg(base.width()).arg(base.height())
                                            .arg(second.width()).arg(second.height()));
        }
        top = second.convertToFormat(QImage::Format_ARGB32);
    } else if (sizeMode == "fit") {
        top = second.scaled(base.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
                  .convertToFormat(QImage::Format_ARGB32);
    } else { // stretch（默认，兼容旧行为）
        top = second.scaled(base.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                  .convertToFormat(QImage::Format_ARGB32);
    }

    QPainter::CompositionMode composition = QPainter::CompositionMode_SourceOver;
    if (mode == "multiply") composition = QPainter::CompositionMode_Multiply;
    else if (mode == "screen") composition = QPainter::CompositionMode_Screen;
    else if (mode == "overlay") composition = QPainter::CompositionMode_Overlay;
    else if (mode == "darken") composition = QPainter::CompositionMode_Darken;
    else if (mode == "lighten") composition = QPainter::CompositionMode_Lighten;
    else if (mode == "difference") composition = QPainter::CompositionMode_Difference;

    QPainter painter(&base);
    painter.setCompositionMode(composition);
    painter.setOpacity(opacity);
    const QPoint topLeft((base.width() - top.width()) / 2, (base.height() - top.height()) / 2);
    painter.drawImage(topLeft, top);
    return Result<QImage>::ok(base);
}

Result<QImage> merge(const QVector<QImage>& images, const QString& mode, int columns, const QColor& background)
{
    QVector<QImage> validImages;
    for (const auto& image : images) {
        if (!image.isNull()) validImages.append(image.convertToFormat(QImage::Format_ARGB32));
    }
    if (validImages.isEmpty()) {
        return Result<QImage>::fail("拼接节点没有有效输入图片");
    }

    if (mode == "grid") {
        if (columns <= 0) {
            return Result<QImage>::fail("网格拼接的列数必须大于 0");
        }
        const int cols = std::min(columns, int(validImages.size()));
        const int rows = (int(validImages.size()) + cols - 1) / cols;
        int cellWidth = 0;
        int cellHeight = 0;
        for (const auto& image : validImages) {
            cellWidth = std::max(cellWidth, image.width());
            cellHeight = std::max(cellHeight, image.height());
        }
        if (qint64(cellWidth) * cols * cellHeight * rows > kMaxPixels) {
            return Result<QImage>::fail("拼接结果过大");
        }
        QImage out(QSize(cellWidth * cols, cellHeight * rows), QImage::Format_ARGB32);
        out.fill(background.isValid() ? background : QColor(Qt::transparent));
        QPainter painter(&out);
        for (int i = 0; i < validImages.size(); ++i) {
            const QImage& image = validImages[i];
            const int cellX = (i % cols) * cellWidth;
            const int cellY = (i / cols) * cellHeight;
            painter.drawImage(cellX + (cellWidth - image.width()) / 2,
                              cellY + (cellHeight - image.height()) / 2, image);
        }
        return Result<QImage>::ok(out);
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
    out.fill(background.isValid() ? background : QColor(Qt::transparent));
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

Result<QImage> sharpen(const QImage& image, double amount, int radius)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (amount < 0.0 || amount > 3.0) {
        return Result<QImage>::fail("锐化强度必须在 0 到 3 之间");
    }
    if (radius < 1 || radius > 10) {
        return Result<QImage>::fail("锐化半径必须在 1 到 10 之间");
    }
    auto blurred = boxBlur(image, radius);
    if (blurred.isFail()) return blurred;

    // unsharp mask：out = src + amount * (src - blur)
    QImage src = image.convertToFormat(QImage::Format_ARGB32);
    const QImage& blur = blurred.value();
    QImage out(src.size(), QImage::Format_ARGB32);
    for (int y = 0; y < src.height(); ++y) {
        const auto* srcLine = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        const auto* blurLine = reinterpret_cast<const QRgb*>(blur.constScanLine(y));
        auto* outLine = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < src.width(); ++x) {
            const QRgb s = srcLine[x];
            const QRgb b = blurLine[x];
            outLine[x] = qRgba(clampByte(int(qRed(s) + amount * (qRed(s) - qRed(b)))),
                               clampByte(int(qGreen(s) + amount * (qGreen(s) - qGreen(b)))),
                               clampByte(int(qBlue(s) + amount * (qBlue(s) - qBlue(b)))),
                               qAlpha(s));
        }
    }
    return Result<QImage>::ok(out);
}

Result<QImage> edgeDetect(const QImage& image)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    const QImage gray = image.convertToFormat(QImage::Format_Grayscale8);
    const int w = gray.width();
    const int h = gray.height();
    QImage out(gray.size(), QImage::Format_ARGB32);

    // Sobel 梯度幅值；边界按最近像素取样（clamp）。
    auto sample = [&](int x, int y) -> int {
        x = std::max(0, std::min(w - 1, x));
        y = std::max(0, std::min(h - 1, y));
        return gray.constScanLine(y)[x];
    };
    for (int y = 0; y < h; ++y) {
        auto* outLine = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < w; ++x) {
            const int gx = -sample(x - 1, y - 1) - 2 * sample(x - 1, y) - sample(x - 1, y + 1)
                           + sample(x + 1, y - 1) + 2 * sample(x + 1, y) + sample(x + 1, y + 1);
            const int gy = -sample(x - 1, y - 1) - 2 * sample(x, y - 1) - sample(x + 1, y - 1)
                           + sample(x - 1, y + 1) + 2 * sample(x, y + 1) + sample(x + 1, y + 1);
            const int magnitude = clampByte(int(std::sqrt(double(gx) * gx + double(gy) * gy)));
            outLine[x] = qRgba(magnitude, magnitude, magnitude, 255);
        }
    }
    return Result<QImage>::ok(out);
}

Result<QImage> invert(const QImage& image)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    QImage out = image.convertToFormat(QImage::Format_ARGB32);
    out.invertPixels(QImage::InvertRgb);
    return Result<QImage>::ok(out);
}

Result<QImage> threshold(const QImage& image, int level)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (level < 0 || level > 255) {
        return Result<QImage>::fail("阈值必须在 0 到 255 之间");
    }
    const QImage gray = image.convertToFormat(QImage::Format_Grayscale8);
    QImage out(gray.size(), QImage::Format_ARGB32);
    for (int y = 0; y < gray.height(); ++y) {
        const uchar* grayLine = gray.constScanLine(y);
        auto* outLine = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < gray.width(); ++x) {
            const int v = grayLine[x] >= level ? 255 : 0;
            outLine[x] = qRgba(v, v, v, 255);
        }
    }
    return Result<QImage>::ok(out);
}

Result<QImage> hueSaturation(const QImage& image, int hueShift, int saturation, int lightness)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (hueShift < -180 || hueShift > 180 || saturation < -100 || saturation > 100
        || lightness < -100 || lightness > 100) {
        return Result<QImage>::fail("色相/饱和度/明度参数超出范围");
    }
    const double satFactor = 1.0 + saturation / 100.0;
    const double lightFactor = 1.0 + lightness / 100.0;

    // 内联 RGB↔HSL 浮点转换，避免逐像素构造两次 QColor 的开销（大图约可快一个数量级）。
    auto hueToChannel = [](double p, double q, double t) {
        if (t < 0.0) t += 1.0;
        if (t > 1.0) t -= 1.0;
        if (t < 1.0 / 6.0) return p + (q - p) * 6.0 * t;
        if (t < 0.5) return q;
        if (t < 2.0 / 3.0) return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
        return p;
    };

    QImage out = image.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < out.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < out.width(); ++x) {
            const QRgb pixel = line[x];
            const double rf = qRed(pixel) / 255.0;
            const double gf = qGreen(pixel) / 255.0;
            const double bf = qBlue(pixel) / 255.0;
            const double maxc = std::max({rf, gf, bf});
            const double minc = std::min({rf, gf, bf});

            // RGB → HSL
            double h = 0.0;
            double s = 0.0;
            double l = (maxc + minc) / 2.0;
            const double delta = maxc - minc;
            if (delta > 0.0) {
                s = l > 0.5 ? delta / (2.0 - maxc - minc) : delta / (maxc + minc);
                if (maxc == rf) {
                    h = 60.0 * std::fmod((gf - bf) / delta + 6.0, 6.0);
                } else if (maxc == gf) {
                    h = 60.0 * ((bf - rf) / delta + 2.0);
                } else {
                    h = 60.0 * ((rf - gf) / delta + 4.0);
                }
                h = std::fmod(h + hueShift + 360.0, 360.0);
            }
            s = std::min(1.0, std::max(0.0, s * satFactor));
            l = std::min(1.0, std::max(0.0, l * lightFactor));

            // HSL → RGB
            int r8, g8, b8;
            if (s <= 0.0) {
                r8 = g8 = b8 = clampByte(int(std::lround(l * 255.0)));
            } else {
                const double q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
                const double p = 2.0 * l - q;
                const double hk = h / 360.0;
                r8 = clampByte(int(std::lround(hueToChannel(p, q, hk + 1.0 / 3.0) * 255.0)));
                g8 = clampByte(int(std::lround(hueToChannel(p, q, hk) * 255.0)));
                b8 = clampByte(int(std::lround(hueToChannel(p, q, hk - 1.0 / 3.0) * 255.0)));
            }
            line[x] = qRgba(r8, g8, b8, qAlpha(pixel));
        }
    }
    return Result<QImage>::ok(out);
}

Result<QVector<QImage>> channelSplit(const QImage& image)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return Result<QVector<QImage>>::fail(valid.error());
    const QImage src = image.convertToFormat(QImage::Format_ARGB32);
    QImage r(src.size(), QImage::Format_ARGB32);
    QImage g(src.size(), QImage::Format_ARGB32);
    QImage b(src.size(), QImage::Format_ARGB32);
    for (int y = 0; y < src.height(); ++y) {
        const auto* srcLine = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        auto* rLine = reinterpret_cast<QRgb*>(r.scanLine(y));
        auto* gLine = reinterpret_cast<QRgb*>(g.scanLine(y));
        auto* bLine = reinterpret_cast<QRgb*>(b.scanLine(y));
        for (int x = 0; x < src.width(); ++x) {
            const QRgb p = srcLine[x];
            rLine[x] = qRgba(qRed(p), qRed(p), qRed(p), 255);
            gLine[x] = qRgba(qGreen(p), qGreen(p), qGreen(p), 255);
            bLine[x] = qRgba(qBlue(p), qBlue(p), qBlue(p), 255);
        }
    }
    return Result<QVector<QImage>>::ok(QVector<QImage>{r, g, b});
}

Result<QImage> channelMerge(const QImage& red, const QImage& green, const QImage& blue)
{
    auto valid = requireImage(red);
    if (valid.isFail()) return Result<QImage>::fail(QString("红通道：%1").arg(valid.error()));
    valid = requireImage(green);
    if (valid.isFail()) return Result<QImage>::fail(QString("绿通道：%1").arg(valid.error()));
    valid = requireImage(blue);
    if (valid.isFail()) return Result<QImage>::fail(QString("蓝通道：%1").arg(valid.error()));
    if (red.size() != green.size() || red.size() != blue.size()) {
        return Result<QImage>::fail("通道合并要求三张通道图尺寸一致");
    }
    const QImage r = red.convertToFormat(QImage::Format_Grayscale8);
    const QImage g = green.convertToFormat(QImage::Format_Grayscale8);
    const QImage b = blue.convertToFormat(QImage::Format_Grayscale8);
    QImage out(r.size(), QImage::Format_ARGB32);
    for (int y = 0; y < out.height(); ++y) {
        const uchar* rLine = r.constScanLine(y);
        const uchar* gLine = g.constScanLine(y);
        const uchar* bLine = b.constScanLine(y);
        auto* outLine = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < out.width(); ++x) {
            outLine[x] = qRgba(rLine[x], gLine[x], bLine[x], 255);
        }
    }
    return Result<QImage>::ok(out);
}

Result<QImage> maskBlend(const QImage& foreground, const QImage& background, const QImage& mask)
{
    auto valid = requireImage(foreground);
    if (valid.isFail()) return Result<QImage>::fail(QString("前景：%1").arg(valid.error()));
    valid = requireImage(background);
    if (valid.isFail()) return Result<QImage>::fail(QString("背景：%1").arg(valid.error()));
    valid = requireImage(mask);
    if (valid.isFail()) return Result<QImage>::fail(QString("蒙版：%1").arg(valid.error()));

    // 以背景尺寸为准；前景与蒙版缩放到背景尺寸。蒙版白色取前景、黑色取背景。
    QImage bg = background.convertToFormat(QImage::Format_ARGB32);
    const QImage fg = foreground.scaled(bg.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                          .convertToFormat(QImage::Format_ARGB32);
    const QImage m = mask.scaled(bg.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                         .convertToFormat(QImage::Format_Grayscale8);
    for (int y = 0; y < bg.height(); ++y) {
        auto* bgLine = reinterpret_cast<QRgb*>(bg.scanLine(y));
        const auto* fgLine = reinterpret_cast<const QRgb*>(fg.constScanLine(y));
        const uchar* maskLine = m.constScanLine(y);
        for (int x = 0; x < bg.width(); ++x) {
            const int weight = maskLine[x];
            const int inverse = 255 - weight;
            const QRgb f = fgLine[x];
            const QRgb b = bgLine[x];
            bgLine[x] = qRgba((qRed(f) * weight + qRed(b) * inverse) / 255,
                              (qGreen(f) * weight + qGreen(b) * inverse) / 255,
                              (qBlue(f) * weight + qBlue(b) * inverse) / 255,
                              (qAlpha(f) * weight + qAlpha(b) * inverse) / 255);
        }
    }
    return Result<QImage>::ok(bg);
}

Result<QImage> imageOverlay(const QImage& base, const QImage& overlay, const QString& anchor, int offsetX,
                            int offsetY, double scalePercent, double opacity)
{
    auto valid = requireImage(base);
    if (valid.isFail()) return Result<QImage>::fail(QString("底图：%1").arg(valid.error()));
    valid = requireImage(overlay);
    if (valid.isFail()) return Result<QImage>::fail(QString("水印图：%1").arg(valid.error()));
    if (scalePercent < 1.0 || scalePercent > 400.0) {
        return Result<QImage>::fail("水印缩放必须在 1% 到 400% 之间");
    }
    if (opacity < 0.0 || opacity > 1.0) {
        return Result<QImage>::fail("水印不透明度必须在 0 到 1 之间");
    }

    QImage out = base.convertToFormat(QImage::Format_ARGB32);
    const QSize scaledSize(std::max(1, int(overlay.width() * scalePercent / 100.0)),
                           std::max(1, int(overlay.height() * scalePercent / 100.0)));
    const QImage top = overlay.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                           .convertToFormat(QImage::Format_ARGB32);

    const int margin = 16;
    QPoint pos(margin, margin);
    if (anchor == "topRight") {
        pos = QPoint(out.width() - top.width() - margin, margin);
    } else if (anchor == "bottomLeft") {
        pos = QPoint(margin, out.height() - top.height() - margin);
    } else if (anchor == "bottomRight") {
        pos = QPoint(out.width() - top.width() - margin, out.height() - top.height() - margin);
    } else if (anchor == "center") {
        pos = QPoint((out.width() - top.width()) / 2, (out.height() - top.height()) / 2);
    }
    pos += QPoint(offsetX, offsetY);

    QPainter painter(&out);
    painter.setOpacity(opacity);
    painter.drawImage(pos, top);
    return Result<QImage>::ok(out);
}

Result<QVector<QImage>> gridSplit(const QImage& image, int rows, int columns)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return Result<QVector<QImage>>::fail(valid.error());
    if (rows < 1 || rows > 3 || columns < 1 || columns > 3) {
        return Result<QVector<QImage>>::fail("切分行列数必须在 1 到 3 之间");
    }
    const QImage src = image.convertToFormat(QImage::Format_ARGB32);
    if (src.width() < columns || src.height() < rows) {
        return Result<QVector<QImage>>::fail(QString("图片尺寸 %1x%2 不足以切成 %3x%4 网格")
                                                 .arg(src.width()).arg(src.height()).arg(rows).arg(columns));
    }
    QVector<QImage> cells;
    cells.reserve(rows * columns);
    // 均匀切分；除不尽的余数并入最后一行/列，保证像素全覆盖且不重叠。
    const int cellWidth = src.width() / columns;
    const int cellHeight = src.height() / rows;
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const int x = column * cellWidth;
            const int y = row * cellHeight;
            const int w = (column == columns - 1) ? src.width() - x : cellWidth;
            const int h = (row == rows - 1) ? src.height() - y : cellHeight;
            cells.append(src.copy(x, y, w, h));
        }
    }
    return Result<QVector<QImage>>::ok(cells);
}

Result<QImage> pixelate(const QImage& image, int blockSize)
{
    auto valid = requireImage(image);
    if (valid.isFail()) return valid;
    if (blockSize < 2 || blockSize > 100) {
        return Result<QImage>::fail("像素块大小必须在 2 到 100 之间");
    }
    const QImage src = image.convertToFormat(QImage::Format_ARGB32);
    QImage out(src.size(), QImage::Format_ARGB32);
    const int w = src.width();
    const int h = src.height();
    for (int by = 0; by < h; by += blockSize) {
        const int blockH = std::min(blockSize, h - by);
        for (int bx = 0; bx < w; bx += blockSize) {
            const int blockW = std::min(blockSize, w - bx);
            qint64 r = 0, g = 0, b = 0, a = 0;
            for (int y = by; y < by + blockH; ++y) {
                const auto* line = reinterpret_cast<const QRgb*>(src.constScanLine(y));
                for (int x = bx; x < bx + blockW; ++x) {
                    const QRgb p = line[x];
                    r += qRed(p); g += qGreen(p); b += qBlue(p); a += qAlpha(p);
                }
            }
            const int count = blockW * blockH;
            const QRgb avg = qRgba(int(r / count), int(g / count), int(b / count), int(a / count));
            for (int y = by; y < by + blockH; ++y) {
                auto* line = reinterpret_cast<QRgb*>(out.scanLine(y));
                for (int x = bx; x < bx + blockW; ++x) {
                    line[x] = avg;
                }
            }
        }
    }
    return Result<QImage>::ok(out);
}

}
