#pragma once

#include "core/Result.h"

#include <QColor>
#include <QImage>
#include <QString>
#include <QVector>

namespace ImageProcessors {

Result<QImage> crop(const QImage& image, int x, int y, int width, int height);
Result<QImage> resize(const QImage& image, int width, int height, bool keepAspect);
Result<QImage> grayscale(const QImage& image);
Result<QImage> brightnessContrast(const QImage& image, int brightness, double contrast);
Result<QImage> boxBlur(const QImage& image, int radius);
Result<QImage> rotateFlip(const QImage& image, int angle, bool flipHorizontal, bool flipVertical);
Result<QImage> textOverlay(const QImage& image, const QString& text, int x, int y, int size, const QColor& color,
                           const QString& fontFamily, bool bold, const QString& anchor, double opacity,
                           bool outline, const QColor& outlineColor);
Result<QImage> blend(const QImage& first, const QImage& second, double opacity, const QString& mode,
                     const QString& sizeMode);
Result<QImage> merge(const QVector<QImage>& images, const QString& mode, int columns, const QColor& background);

// 扩展节点算法（全部 Qt 原生实现）
Result<QImage> sharpen(const QImage& image, double amount, int radius);
Result<QImage> edgeDetect(const QImage& image);
Result<QImage> invert(const QImage& image);
Result<QImage> threshold(const QImage& image, int level);
Result<QImage> hueSaturation(const QImage& image, int hueShift, int saturation, int lightness);
Result<QVector<QImage>> channelSplit(const QImage& image);
Result<QImage> channelMerge(const QImage& red, const QImage& green, const QImage& blue);
Result<QImage> maskBlend(const QImage& foreground, const QImage& background, const QImage& mask);
Result<QImage> imageOverlay(const QImage& base, const QImage& overlay, const QString& anchor, int offsetX,
                            int offsetY, double scalePercent, double opacity);
Result<QImage> pixelate(const QImage& image, int blockSize);

}
