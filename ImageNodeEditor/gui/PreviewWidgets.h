#pragma once

#include "gui/AppTheme.h"

#include <QGuiApplication>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>

// 预览相关的内部控件：
// - ImagePopupWindow：点击预览图后弹出的大图查看窗口，点击任意处关闭。
// - PreviewLabel：右侧预览栏标签，保存原图并按容器尺寸等比缩放显示，点击可放大查看。
// 两者无信号/槽（不需要 Q_OBJECT），配色取自 AppTheme，随主题自适应。

class ImagePopupWindow final : public QWidget {
public:
    explicit ImagePopupWindow(const QImage& image, QWidget* parent = nullptr)
        : QWidget(parent, Qt::Window), image_(image)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowTitle("图片预览");
        setCursor(Qt::PointingHandCursor);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(8, 8, 8, 8);
        imageLabel_ = new QLabel;
        imageLabel_->setAlignment(Qt::AlignCenter);
        imageLabel_->setStyleSheet(QString("background:%1;").arg(AppTheme::palette().base.name()));
        layout->addWidget(imageLabel_);

        QSize targetSize = image_.size();
        if (auto* screen = QGuiApplication::primaryScreen()) {
            const QSize maxSize = screen->availableGeometry().size() * 0.82;
            targetSize.scale(maxSize, Qt::KeepAspectRatio);
        }
        resize(targetSize.expandedTo(QSize(320, 240)));
        updatePixmap();
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        event->accept();
        close();
    }

    void resizeEvent(QResizeEvent* event) override
    {
        QWidget::resizeEvent(event);
        updatePixmap();
    }

private:
    void updatePixmap()
    {
        if (!imageLabel_ || image_.isNull()) {
            return;
        }
        imageLabel_->setPixmap(QPixmap::fromImage(image_).scaled(imageLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    QImage image_;
    QLabel* imageLabel_ = nullptr;
};

class PreviewLabel final : public QLabel {
public:
    explicit PreviewLabel(QWidget* parent = nullptr) : QLabel(parent)
    {
        setCursor(Qt::ArrowCursor);
    }

    void setSourceImage(const QImage& image)
    {
        image_ = image;
        setCursor(image_.isNull() ? Qt::ArrowCursor : Qt::PointingHandCursor);
        if (image_.isNull()) {
            setText("暂无预览");
            setPixmap({});
            return;
        }
        setText({});
        updatePixmap();
    }

protected:
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

    void resizeEvent(QResizeEvent* event) override
    {
        QLabel::resizeEvent(event);
        updatePixmap();
    }

private:
    void updatePixmap()
    {
        if (image_.isNull()) {
            return;
        }
        setPixmap(QPixmap::fromImage(image_).scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    QImage image_;
};
