#pragma once

#include <QTabBar>

// Chrome 风画布标签栏：自绘标签形状（激活标签为顶部圆角、底部外扩的梯形，并与下方
// 画布内容融为一体），配色取自 AppTheme palette，随深色 / 浅色主题切换。close 按钮仍由
// QTabBar 负责（tabsClosable）。仅负责绘制，不改变标签业务逻辑。
class CanvasTabBar final : public QTabBar {
    Q_OBJECT
public:
    explicit CanvasTabBar(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize tabSizeHint(int index) const override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void setHoverIndex(int index);

    int hoverIndex_ = -1;
};
