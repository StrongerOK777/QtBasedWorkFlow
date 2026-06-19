#pragma once

#include "gui/AppTheme.h"

#include <QColor>
#include <QGuiApplication>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QScreen>
#include <QWheelEvent>
#include <QWidget>

#include <algorithm>
#include <cmath>

// 预览相关的内部控件：
// - ImagePopupWindow：大图查看窗口。滚轮缩放（指向光标）、左键拖拽平移、双击在「适应窗口/100%」
//   间切换、Esc 关闭；透明区域用棋盘格底显示。
// - PreviewLabel：右侧预览栏标签。等比缩放显示、透明棋盘格底、悬停显示像素坐标和颜色值，
//   点击打开大图查看窗口。
// 两者无信号/槽（不需要 Q_OBJECT），配色取自 AppTheme，随主题自适应。

namespace PreviewInternal {

// 透明图片的棋盘格底（图像编辑器惯例），亮度随主题调整。
inline void drawCheckerboard(QPainter& painter, const QRectF& rect)
{
    const bool dark = AppTheme::isDarkTheme();
    const QColor a = dark ? QColor(58, 60, 64) : QColor(214, 213, 206);
    const QColor b = dark ? QColor(44, 46, 50) : QColor(244, 243, 238);
    constexpr int cell = 10;
    painter.save();
    painter.setClipRect(rect);
    painter.fillRect(rect, b);
    const int left = int(std::floor(rect.left() / cell));
    const int top = int(std::floor(rect.top() / cell));
    const int right = int(std::ceil(rect.right() / cell));
    const int bottom = int(std::ceil(rect.bottom() / cell));
    painter.setPen(Qt::NoPen);
    painter.setBrush(a);
    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            if ((x + y) % 2 == 0) {
                painter.drawRect(QRectF(x * cell, y * cell, cell, cell));
            }
        }
    }
    painter.restore();
}

}

class ImagePopupWindow final : public QWidget {
public:
    explicit ImagePopupWindow(const QImage& image, QWidget* parent = nullptr)
        : QWidget(parent, Qt::Window), image_(image)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);

        QSize targetSize = image_.size();
        if (auto* screen = QGuiApplication::primaryScreen()) {
            const QSize maxSize = screen->availableGeometry().size() * 0.82;
            targetSize.scale(maxSize, Qt::KeepAspectRatio);
        }
        resize(targetSize.expandedTo(QSize(420, 320)));
        fitToWindow();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), AppTheme::palette().base);
        if (image_.isNull()) {
            return;
        }
        const QRectF target(offset_, QSizeF(image_.size()) * scale_);
        PreviewInternal::drawCheckerboard(painter, target);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, scale_ < 4.0);
        painter.drawImage(target, image_);

        // 顶部状态条：缩放比例 + 操作提示
        const QString info = QString("%1%  ·  滚轮缩放 / 拖拽平移 / 双击切换 100%% / Esc 关闭")
                                 .arg(int(std::lround(scale_ * 100)));
        const AppTheme::Palette p = AppTheme::palette();
        QFont font = painter.font();
        font.setPixelSize(12);
        painter.setFont(font);
        const QRectF bar(0, 0, width(), 26);
        painter.fillRect(bar, QColor(p.panel.red(), p.panel.green(), p.panel.blue(), 215));
        painter.setPen(p.textSecondary);
        painter.drawText(bar.adjusted(10, 0, -10, 0), Qt::AlignVCenter | Qt::AlignLeft, info);
        updateWindowTitle();
    }

    void wheelEvent(QWheelEvent* event) override
    {
        if (image_.isNull()) {
            return;
        }
        const double steps = event->angleDelta().y() / 120.0;
        if (std::abs(steps) < 0.01) {
            return;
        }
        const double factor = std::pow(1.15, steps);
        const double newScale = std::clamp(scale_ * factor, 0.05, 32.0);
        // 缩放锚定在光标位置：保持光标下的图像点不动。
        const QPointF mouse = event->position();
        offset_ = mouse - (mouse - offset_) * (newScale / scale_);
        scale_ = newScale;
        fitted_ = false;
        update();
        event->accept();
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton) {
            panning_ = true;
            lastPanPos_ = event->position();
            setCursor(Qt::ClosedHandCursor);
        }
        event->accept();
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (panning_) {
            offset_ += event->position() - lastPanPos_;
            lastPanPos_ = event->position();
            update();
        }
        event->accept();
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        panning_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        // 适应窗口 ↔ 100% 像素切换；切到 100% 时以光标为锚点。
        if (fitted_) {
            const QPointF mouse = event->position();
            offset_ = mouse - (mouse - offset_) * (1.0 / scale_);
            scale_ = 1.0;
            fitted_ = false;
        } else {
            fitToWindow();
        }
        update();
        event->accept();
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        if (event->key() == Qt::Key_Escape) {
            close();
            return;
        }
        QWidget::keyPressEvent(event);
    }

    void resizeEvent(QResizeEvent* event) override
    {
        QWidget::resizeEvent(event);
        if (fitted_) {
            fitToWindow();
        }
    }

private:
    void fitToWindow()
    {
        if (image_.isNull() || width() <= 0 || height() <= 0) {
            return;
        }
        const double sx = double(width()) / image_.width();
        const double sy = double(height()) / image_.height();
        scale_ = std::min(1.0, std::min(sx, sy));
        offset_ = QPointF((width() - image_.width() * scale_) / 2.0,
                          (height() - image_.height() * scale_) / 2.0);
        fitted_ = true;
        update();
    }

    void updateWindowTitle()
    {
        setWindowTitle(QString("图片预览 - %1x%2 - %3%")
                           .arg(image_.width())
                           .arg(image_.height())
                           .arg(int(std::lround(scale_ * 100))));
    }

    QImage image_;
    double scale_ = 1.0;
    QPointF offset_;
    bool fitted_ = true;
    bool panning_ = false;
    QPointF lastPanPos_;
};

class PreviewLabel final : public QLabel {
public:
    explicit PreviewLabel(QWidget* parent = nullptr) : QLabel(parent)
    {
        setCursor(Qt::ArrowCursor);
        setMouseTracking(true);
    }

    void setSourceImage(const QImage& image)
    {
        image_ = image;
        hoverText_.clear();
        setCursor(image_.isNull() ? Qt::ArrowCursor : Qt::PointingHandCursor);
        if (image_.isNull()) {
            setText("暂无预览");
        } else {
            setText({});
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        if (image_.isNull()) {
            QLabel::paintEvent(event);
            return;
        }
        QPainter painter(this);
        const QRectF target = imageTargetRect();
        PreviewInternal::drawCheckerboard(painter, target);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.drawImage(target, image_);

        // 悬停像素信息：坐标 + 颜色值（左下角小标签）
        if (!hoverText_.isEmpty()) {
            const AppTheme::Palette p = AppTheme::palette();
            QFont font = painter.font();
            font.setPixelSize(11);
            painter.setFont(font);
            const QFontMetrics metrics(font);
            const QRectF info(6, height() - metrics.height() - 12,
                              metrics.horizontalAdvance(hoverText_) + 14, metrics.height() + 6);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(p.panel.red(), p.panel.green(), p.panel.blue(), 225));
            painter.drawRoundedRect(info, 4, 4);
            if (hoverColor_.isValid()) {
                painter.setBrush(hoverColor_);
                painter.setPen(QPen(p.hairline, 1));
                painter.drawRect(QRectF(info.left() + 5, info.center().y() - 4, 8, 8));
            }
            painter.setPen(p.textPrimary);
            painter.drawText(info.adjusted(18, 0, -4, 0), Qt::AlignVCenter | Qt::AlignLeft, hoverText_);
        }
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (!image_.isNull()) {
            auto* popup = new ImagePopupWindow(image_, window());
            popup->show();
            event->accept();
            return;
        }
        QLabel::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (image_.isNull()) {
            QLabel::mouseMoveEvent(event);
            return;
        }
        const QRectF target = imageTargetRect();
        const QPointF pos = event->position();
        QString next;
        if (target.contains(pos) && target.width() > 0 && target.height() > 0) {
            const int px = std::clamp(int((pos.x() - target.left()) / target.width() * image_.width()), 0,
                                      image_.width() - 1);
            const int py = std::clamp(int((pos.y() - target.top()) / target.height() * image_.height()), 0,
                                      image_.height() - 1);
            const QColor color = image_.pixelColor(px, py);
            hoverColor_ = color;
            next = QString("%1, %2   %3").arg(px).arg(py).arg(color.name(QColor::HexRgb).toUpper());
        }
        if (next != hoverText_) {
            hoverText_ = next;
            update();
        }
        QLabel::mouseMoveEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        if (!hoverText_.isEmpty()) {
            hoverText_.clear();
            update();
        }
        QLabel::leaveEvent(event);
    }

    void resizeEvent(QResizeEvent* event) override
    {
        QLabel::resizeEvent(event);
        update();
    }

private:
    QRectF imageTargetRect() const
    {
        if (image_.isNull() || width() <= 0 || height() <= 0) {
            return {};
        }
        QSizeF scaled = QSizeF(image_.size());
        scaled.scale(QSizeF(size()), Qt::KeepAspectRatio);
        return QRectF(QPointF((width() - scaled.width()) / 2.0, (height() - scaled.height()) / 2.0), scaled);
    }

    QImage image_;
    QString hoverText_;
    QColor hoverColor_;
};
