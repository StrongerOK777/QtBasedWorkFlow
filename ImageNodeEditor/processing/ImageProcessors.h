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
Result<QImage> textOverlay(const QImage& image, const QString& text, int x, int y, int size, const QColor& color);
Result<QImage> blend(const QImage& first, const QImage& second, double opacity);
Result<QImage> merge(const QVector<QImage>& images, const QString& mode);

}
