#include "gui/MainWindow.h"

#include "gui/AppTheme.h"
#include "gui/WorkflowCommands.h"
#include "nodes/ImageNode.h"
#include "nodes/MacroNode.h"
#include "nodes/NodeFactory.h"
#include "workflow/ExecutionEngine.h"
#include "workflow/WorkflowSerializer.h"
#include "workflow/WorkflowValidator.h"

#include <QAction>
#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QCursor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
#include <QFileDialog>
#include <QFormLayout>
#include <QGraphicsEllipseItem>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGuiApplication>
#include <QInputDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPalette>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QSet>
#include <QSignalBlocker>
#include <QSlider>
#include <QSizePolicy>
#include <QSplitter>
#include <QStackedLayout>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTabBar>
#include <QTabWidget>
#include <QTextCursor>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QToolTip>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

namespace {

enum class PortConnectionFeedback {
    None,
    Connectable,
    Blocked,
    Snapped
};

QColor portColor(PortType type)
{
    switch (type) {
    case PortType::ImageRGBA:
    case PortType::ImageGray:
        return QColor("#404040");
    case PortType::Number:
        return QColor("#606060");
    case PortType::Text:
        return QColor("#808080");
    case PortType::Mask:
        return QColor("#a0a0a0");
    case PortType::ImageList:
        return QColor("#505050");
    }
    return QColor("#707070");
}

QString portDirectionName(PortDirection direction)
{
    return direction == PortDirection::Input ? "输入" : "输出";
}

QPainterPath rectPath(const QRectF& rect)
{
    QPainterPath path;
    path.addRect(rect);
    return path;
}

void paintCanvasBackground(QPainter* painter, const QRectF& rect, double uiScale)
{
    const auto colors = AppTheme::colors();
    QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
    gradient.setColorAt(0, colors.canvasTop);
    gradient.setColorAt(1, colors.canvasBottom);
    painter->fillRect(rect, gradient);

    const qreal step = 26.0 * uiScale;
    const qreal dotRadius = std::max<qreal>(1.0, 1.15 * uiScale);
    const qreal left = std::floor(rect.left() / step) * step;
    const qreal top = std::floor(rect.top() / step) * step;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(colors.canvasDot);
    for (qreal x = left; x < rect.right(); x += step) {
        for (qreal y = top; y < rect.bottom(); y += step) {
            painter->drawEllipse(QPointF(x, y), dotRadius, dotRadius);
        }
    }
}

QImage boxBlurred(const QImage& source, int radius, int passes = 3)
{
    if (source.isNull() || radius <= 0) {
        return source;
    }
    QImage current = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage temp(current.size(), current.format());
    const int w = current.width();
    const int h = current.height();

    for (int pass = 0; pass < passes; ++pass) {
        for (int y = 0; y < h; ++y) {
            auto* out = reinterpret_cast<QRgb*>(temp.scanLine(y));
            for (int x = 0; x < w; ++x) {
                int a = 0;
                int r = 0;
                int g = 0;
                int b = 0;
                int count = 0;
                const int x0 = std::max(0, x - radius);
                const int x1 = std::min(w - 1, x + radius);
                for (int sx = x0; sx <= x1; ++sx) {
                    const QRgb pixel = current.pixel(sx, y);
                    a += qAlpha(pixel);
                    r += qRed(pixel);
                    g += qGreen(pixel);
                    b += qBlue(pixel);
                    ++count;
                }
                out[x] = qRgba(r / count, g / count, b / count, a / count);
            }
        }
        for (int y = 0; y < h; ++y) {
            auto* out = reinterpret_cast<QRgb*>(current.scanLine(y));
            for (int x = 0; x < w; ++x) {
                int a = 0;
                int r = 0;
                int g = 0;
                int b = 0;
                int count = 0;
                const int y0 = std::max(0, y - radius);
                const int y1 = std::min(h - 1, y + radius);
                for (int sy = y0; sy <= y1; ++sy) {
                    const QRgb pixel = temp.pixel(x, sy);
                    a += qAlpha(pixel);
                    r += qRed(pixel);
                    g += qGreen(pixel);
                    b += qBlue(pixel);
                    ++count;
                }
                out[x] = qRgba(r / count, g / count, b / count, a / count);
            }
        }
    }
    return current;
}

QPixmap sampledCanvasGlass(const QRectF& sceneRect, double uiScale, const QSize& targetSize)
{
    if (targetSize.isEmpty()) {
        return {};
    }
    QImage sample(targetSize.expandedTo(QSize(1, 1)), QImage::Format_ARGB32_Premultiplied);
    sample.fill(Qt::transparent);

    QPainter painter(&sample);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(-sceneRect.topLeft());
    paintCanvasBackground(&painter, sceneRect, uiScale);
    painter.end();

    QImage blurred = boxBlurred(sample, std::max(2, AppTheme::px(4, uiScale)), 3);
    return QPixmap::fromImage(blurred);
}

QIcon lineIcon(const QString& name)
{
    constexpr int size = 64;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    const QColor ink = AppTheme::isDarkTheme() ? QColor("#e0e0e0") : QColor("#303030");
    const QColor accent = AppTheme::isDarkTheme() ? QColor("#f2f2f2") : QColor("#505050");
    const QColor warm = AppTheme::isDarkTheme() ? QColor("#b8b8b8") : QColor("#707070");
    QPen pen(ink, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    auto page = [&] {
        painter.drawRect(QRectF(18, 10, 28, 42));
        painter.drawLine(QPointF(34, 10), QPointF(46, 22));
        painter.drawLine(QPointF(34, 10), QPointF(34, 22));
        painter.drawLine(QPointF(34, 22), QPointF(46, 22));
    };
    auto folder = [&] {
        painter.drawPath([] {
            QPainterPath path;
            path.moveTo(9, 23);
            path.lineTo(25, 23);
            path.lineTo(30, 17);
            path.lineTo(42, 17);
            path.quadTo(48, 17, 50, 23);
            path.lineTo(55, 23);
            path.lineTo(50, 50);
            path.lineTo(12, 50);
            path.closeSubpath();
            return path;
        }());
    };

    if (name == "new") {
        page();
        painter.setPen(QPen(accent, 4.8, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(32, 29), QPointF(32, 43));
        painter.drawLine(QPointF(25, 36), QPointF(39, 36));
    } else if (name == "open") {
        folder();
        painter.setPen(QPen(accent, 4.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(32, 43), QPointF(32, 28));
        painter.drawLine(QPointF(24, 35), QPointF(32, 27));
        painter.drawLine(QPointF(40, 35), QPointF(32, 27));
    } else if (name == "save") {
        painter.drawRect(QRectF(13, 11, 38, 42));
        painter.drawRect(QRectF(21, 11, 22, 14));
        painter.drawRect(QRectF(22, 35, 20, 14));
    } else if (name == "saveAs") {
        painter.drawRect(QRectF(12, 12, 34, 40));
        painter.drawRect(QRectF(20, 12, 18, 12));
        painter.setPen(QPen(warm, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(40, 41), QPointF(54, 27));
        painter.drawLine(QPointF(47, 27), QPointF(54, 27));
        painter.drawLine(QPointF(54, 27), QPointF(54, 34));
    } else if (name == "run") {
        painter.setBrush(QColor("#34c759"));
        painter.setPen(Qt::NoPen);
        QPainterPath play;
        play.moveTo(23, 14);
        play.lineTo(50, 32);
        play.lineTo(23, 50);
        play.closeSubpath();
        painter.drawPath(play);
    } else if (name == "exportCanvas") {
        painter.drawRect(QRectF(12, 14, 40, 32));
        painter.drawLine(QPointF(20, 24), QPointF(44, 24));
        painter.drawLine(QPointF(20, 32), QPointF(36, 32));
        painter.setPen(QPen(accent, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(32, 49), QPointF(32, 32));
        painter.drawLine(QPointF(24, 41), QPointF(32, 49));
        painter.drawLine(QPointF(40, 41), QPointF(32, 49));
    } else if (name == "exportWorkflow") {
        page();
        painter.setPen(QPen(accent, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(32, 48), QPointF(32, 29));
        painter.drawLine(QPointF(24, 40), QPointF(32, 48));
        painter.drawLine(QPointF(40, 40), QPointF(32, 48));
    } else if (name == "exportPreview") {
        painter.drawRect(QRectF(12, 14, 40, 30));
        painter.drawLine(QPointF(18, 38), QPointF(28, 27));
        painter.drawLine(QPointF(28, 27), QPointF(35, 34));
        painter.drawLine(QPointF(35, 34), QPointF(44, 23));
        painter.setPen(QPen(accent, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(32, 51), QPointF(32, 34));
        painter.drawLine(QPointF(24, 43), QPointF(32, 51));
        painter.drawLine(QPointF(40, 43), QPointF(32, 51));
    } else if (name == "zoomIn" || name == "zoomOut") {
        painter.drawEllipse(QPointF(28, 28), 16, 16);
        painter.drawLine(QPointF(40, 40), QPointF(52, 52));
        painter.setPen(QPen(accent, 4.8, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(20, 28), QPointF(36, 28));
        if (name == "zoomIn") {
            painter.drawLine(QPointF(28, 20), QPointF(28, 36));
        }
    } else if (name == "scaleReset") {
        painter.drawArc(QRectF(16, 16, 32, 32), 45 * 16, 285 * 16);
        painter.drawLine(QPointF(45, 15), QPointF(45, 27));
        painter.drawLine(QPointF(45, 15), QPointF(33, 15));
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(QRectF(18, 24, 28, 18), Qt::AlignCenter, "1:1");
    } else if (name == "dockLeft") {
        painter.drawRect(QRectF(12, 12, 40, 40));
        painter.fillRect(QRectF(12, 12, 13, 40), AppTheme::isDarkTheme() ? QColor(238, 238, 238, 62) : QColor(80, 80, 80, 70));
        painter.drawLine(QPointF(25, 12), QPointF(25, 52));
    } else if (name == "dockRight") {
        painter.drawRect(QRectF(12, 12, 40, 40));
        painter.fillRect(QRectF(39, 12, 13, 40), AppTheme::isDarkTheme() ? QColor(238, 238, 238, 62) : QColor(80, 80, 80, 70));
        painter.drawLine(QPointF(39, 12), QPointF(39, 52));
    } else if (name == "dockBottom") {
        painter.drawRect(QRectF(12, 12, 40, 40));
        painter.fillRect(QRectF(12, 39, 40, 13), AppTheme::isDarkTheme() ? QColor(238, 238, 238, 62) : QColor(80, 80, 80, 70));
        painter.drawLine(QPointF(12, 39), QPointF(52, 39));
    } else if (name == "layoutReset") {
        painter.drawRect(QRectF(12, 12, 40, 40));
        painter.drawLine(QPointF(25, 12), QPointF(25, 52));
        painter.drawLine(QPointF(12, 39), QPointF(52, 39));
        painter.setPen(QPen(accent, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawArc(QRectF(22, 22, 20, 20), 40 * 16, 260 * 16);
    } else if (name == "fullscreen") {
        painter.drawLine(QPointF(14, 26), QPointF(14, 14));
        painter.drawLine(QPointF(14, 14), QPointF(26, 14));
        painter.drawLine(QPointF(38, 14), QPointF(50, 14));
        painter.drawLine(QPointF(50, 14), QPointF(50, 26));
        painter.drawLine(QPointF(50, 38), QPointF(50, 50));
        painter.drawLine(QPointF(50, 50), QPointF(38, 50));
        painter.drawLine(QPointF(26, 50), QPointF(14, 50));
        painter.drawLine(QPointF(14, 50), QPointF(14, 38));
    } else if (name == "settings") {
        painter.drawEllipse(QPointF(32, 32), 8, 8);
        constexpr double pi = 3.14159265358979323846;
        for (int i = 0; i < 8; ++i) {
            const double a = i * pi / 4.0;
            const QPointF inner(32 + std::cos(a) * 16, 32 + std::sin(a) * 16);
            const QPointF outer(32 + std::cos(a) * 23, 32 + std::sin(a) * 23);
            painter.drawLine(inner, outer);
        }
    } else if (name == "clearLog") {
        painter.drawRect(QRectF(16, 14, 32, 38));
        painter.drawLine(QPointF(22, 24), QPointF(42, 24));
        painter.drawLine(QPointF(22, 32), QPointF(38, 32));
        painter.drawLine(QPointF(22, 40), QPointF(34, 40));
        painter.setPen(QPen(warm, 4.8, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(44, 14), QPointF(54, 24));
        painter.drawLine(QPointF(54, 14), QPointF(44, 24));
    } else if (name == "back" || name == "forward") {
        const qreal start = name == "back" ? 42 : 22;
        const qreal tip = name == "back" ? 20 : 44;
        painter.drawLine(QPointF(start, 18), QPointF(tip, 32));
        painter.drawLine(QPointF(tip, 32), QPointF(start, 46));
    } else if (name == "more") {
        painter.setBrush(ink);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(20, 32), 4.2, 4.2);
        painter.drawEllipse(QPointF(32, 32), 4.2, 4.2);
        painter.drawEllipse(QPointF(44, 32), 4.2, 4.2);
    }
    return QIcon(pix);
}

class DelayedToolTipFilter final : public QObject {
public:
    explicit DelayedToolTipFilter(QObject* parent = nullptr) : QObject(parent)
    {
        timer_.setSingleShot(true);
        timer_.setInterval(500);
        connect(&timer_, &QTimer::timeout, this, [this] {
            if (watched_) {
                QToolTip::showText(QCursor::pos() + QPoint(14, 18), watched_->toolTip(), watched_);
            }
        });
    }

protected:
    bool eventFilter(QObject* object, QEvent* event) override
    {
        auto* widget = qobject_cast<QWidget*>(object);
        if (!widget) {
            return QObject::eventFilter(object, event);
        }
        if (event->type() == QEvent::Enter) {
            watched_ = widget;
            timer_.start();
        } else if (event->type() == QEvent::ToolTip) {
            return true;
        } else if (event->type() == QEvent::Leave || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::Hide) {
            timer_.stop();
            QToolTip::hideText();
            if (watched_ == widget) {
                watched_ = nullptr;
            }
        }
        return QObject::eventFilter(object, event);
    }

private:
    QTimer timer_;
    QWidget* watched_ = nullptr;
};

void installDelayedTooltips(QToolBar* toolbar)
{
    if (!toolbar) {
        return;
    }
    auto* filter = new DelayedToolTipFilter(toolbar);
    for (auto* button : toolbar->findChildren<QToolButton*>()) {
        button->setToolTipDuration(5000);
        button->installEventFilter(filter);
    }
}

class PortItem final : public QGraphicsEllipseItem {
public:
    PortItem(QString nodeId, PortInfo port, double uiScale, QGraphicsItem* parent)
        : QGraphicsEllipseItem(parent), nodeId_(std::move(nodeId)), port_(std::move(port)), uiScale_(uiScale)
    {
        const auto metrics = AppTheme::nodeMetrics(uiScale_);
        setRect(-metrics.portRadius, -metrics.portRadius, metrics.portRadius * 2, metrics.portRadius * 2);
        setAcceptHoverEvents(true);
        setToolTip(QString("%1\n方向：%2\n类型：%3")
                       .arg(port_.displayName, portDirectionName(port_.direction), portTypeName(port_.type)));
    }

    QString nodeId() const { return nodeId_; }
    PortInfo port() const { return port_; }
    void setConnectionFeedback(PortConnectionFeedback feedback)
    {
        if (feedback_ == feedback) {
            return;
        }
        feedback_ = feedback;
        update();
    }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
    {
        hovered_ = true;
        update();
        QGraphicsEllipseItem::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override
    {
        hovered_ = false;
        update();
        QGraphicsEllipseItem::hoverLeaveEvent(event);
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        QColor base = portColor(port_.type);
        if (feedback_ == PortConnectionFeedback::Blocked) {
            base = AppTheme::isDarkTheme() ? QColor("#707070") : QColor("#404040");
        } else if (feedback_ == PortConnectionFeedback::Snapped) {
            base = AppTheme::isDarkTheme() ? QColor("#f2f2f2") : QColor("#202020");
        } else if (feedback_ == PortConnectionFeedback::Connectable) {
            base = AppTheme::isDarkTheme() ? QColor("#b8b8b8") : QColor("#808080");
        }
        const QRectF r = rect();
        const qreal halo = feedback_ == PortConnectionFeedback::Snapped ? 5.5 * uiScale_ :
                           feedback_ == PortConnectionFeedback::None ? 3.0 * uiScale_ : 4.0 * uiScale_;

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(base.red(), base.green(), base.blue(),
                                 feedback_ == PortConnectionFeedback::None ? (hovered_ ? 62 : 34) : 78));
        painter->drawEllipse(r.adjusted(-halo, -halo, halo, halo));

        QRadialGradient gradient(r.center() - QPointF(r.width() * 0.22, r.height() * 0.22), r.width());
        gradient.setColorAt(0, base.lighter(165));
        gradient.setColorAt(0.62, base);
        gradient.setColorAt(1, base.darker(125));
        painter->setBrush(gradient);
        painter->setPen(QPen(QColor(255, 255, 255, hovered_ || feedback_ != PortConnectionFeedback::None ? 245 : 210),
                             std::max(1.0, (feedback_ == PortConnectionFeedback::Snapped ? 1.8 : 1.2) * uiScale_)));
        const qreal grow = feedback_ == PortConnectionFeedback::Snapped ? 1.5 * uiScale_ : 0.0;
        painter->drawEllipse(r.adjusted(-grow, -grow, grow, grow));
    }

private:
    QString nodeId_;
    PortInfo port_;
    double uiScale_ = 1.0;
    PortConnectionFeedback feedback_ = PortConnectionFeedback::None;
    bool hovered_ = false;
};

class NodeItem final : public QGraphicsRectItem {
public:
    NodeItem(MainWindow* window, QString nodeId, QSharedPointer<ImageNode> node, double uiScale, NodeExecutionState runState)
        : QGraphicsRectItem(), window_(window), nodeId_(std::move(nodeId)), node_(std::move(node)), uiScale_(uiScale), runState_(runState)
    {
        const auto metrics = AppTheme::nodeMetrics(uiScale_);
        const int rowCount = std::max(node_->inputPorts().size(), node_->outputPorts().size());
        compactHeight_ = metrics.topPadding + rowCount * metrics.rowHeight + metrics.bottomPadding;
        setRect(0, 0, metrics.width, compactHeight_);
        setAcceptHoverEvents(true);
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);

        auto* title = new QGraphicsTextItem(node_->displayName(), this);
        QFont titleFont;
        titleFont.setPointSizeF(metrics.titleSize);
        titleFont.setBold(true);
        title->setFont(titleFont);
        title->setDefaultTextColor(AppTheme::colors().textPrimary);
        title->setPos(14 * uiScale_, 8 * uiScale_);

        qreal y = metrics.topPadding;
        for (const auto& port : node_->inputPorts()) {
            auto* item = new PortItem(nodeId_, port, uiScale_, this);
            item->setPos(0, y);
            ports_.append(item);
            auto* label = new QGraphicsTextItem(port.displayName, this);
            QFont labelFont;
            labelFont.setPointSizeF(metrics.labelSize);
            label->setFont(labelFont);
            label->setDefaultTextColor(AppTheme::colors().textSecondary);
            label->setPos(15 * uiScale_, y - 12 * uiScale_);
            y += metrics.rowHeight;
        }
        y = metrics.topPadding;
        for (const auto& port : node_->outputPorts()) {
            auto* item = new PortItem(nodeId_, port, uiScale_, this);
            item->setPos(rect().width(), y);
            ports_.append(item);
            auto* label = new QGraphicsTextItem(port.displayName, this);
            QFont labelFont;
            labelFont.setPointSizeF(metrics.labelSize);
            label->setFont(labelFont);
            label->setDefaultTextColor(AppTheme::colors().textSecondary);
            const qreal labelWidth = label->boundingRect().width();
            label->setPos(rect().width() - labelWidth - 15 * uiScale_, y - 12 * uiScale_);
            y += metrics.rowHeight;
        }

        buildParameterPanel();
        updateParameterExpansion(false);
    }

    QString nodeId() const { return nodeId_; }
    QVector<PortItem*> ports() const { return ports_; }
    void setRunState(NodeExecutionState state)
    {
        if (runState_ == state) {
            return;
        }
        runState_ = state;
        update();
    }
    void setElapsedMs(qint64 elapsedMs)
    {
        if (elapsedMs_ == elapsedMs) {
            return;
        }
        elapsedMs_ = elapsedMs;
        update();
    }
    void setAnimationPhase(int phase)
    {
        animationPhase_ = phase;
        if (runState_ == NodeExecutionState::Running) {
            update();
        }
    }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
    {
        hovered_ = true;
        update();
        QGraphicsRectItem::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override
    {
        hovered_ = false;
        update();
        QGraphicsRectItem::hoverLeaveEvent(event);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override
    {
        pressed_ = true;
        dragStartPos_ = pos();
        update();
        QGraphicsRectItem::mousePressEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
    {
        pressed_ = false;
        const QPointF dragEndPos = pos();
        if (window_ && (!qFuzzyCompare(dragStartPos_.x(), dragEndPos.x()) || !qFuzzyCompare(dragStartPos_.y(), dragEndPos.y()))) {
            window_->commitNodeMove(nodeId_, dragStartPos_, dragEndPos);
        }
        update();
        QGraphicsRectItem::mouseReleaseEvent(event);
    }

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (window_) {
            window_->enterMacroNode(nodeId_);
            event->accept();
            return;
        }
        QGraphicsRectItem::mouseDoubleClickEvent(event);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override
    {
        if (change == ItemPositionHasChanged && window_) {
            window_->updateNodePosition(nodeId_, value.toPointF());
        } else if (change == ItemSelectedHasChanged) {
            const bool selected = value.toBool();
            setZValue(selected ? 20.0 : 0.0);
            updateParameterExpansion(selected);
        }
        return QGraphicsRectItem::itemChange(change, value);
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        const auto metrics = AppTheme::nodeMetrics(uiScale_);
        const auto colors = AppTheme::colors();
        const QRectF body = rect();
        const QRectF shadowRect = body.translated(0, pressed_ ? 2 * uiScale_ : 5 * uiScale_);
        const QColor stateColor = [&] {
            const bool dark = AppTheme::isDarkTheme();
            switch (runState_) {
            case NodeExecutionState::Running: return dark ? QColor("#d0d0d0") : QColor("#707070");
            case NodeExecutionState::Succeeded: return dark ? QColor("#b8b8b8") : QColor("#505050");
            case NodeExecutionState::Failed: return QColor("#ff453a");
            case NodeExecutionState::CacheHit: return QColor("#909090");
            case NodeExecutionState::NotExecuted: return colors.nodeBorder;
            }
            return colors.nodeBorder;
        }();

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(colors.nodeShadow);
        painter->drawPath(rectPath(shadowRect.adjusted(3 * uiScale_, 3 * uiScale_, -3 * uiScale_, -1 * uiScale_)));

        painter->save();
        painter->setClipPath(rectPath(body));
        const QRectF sampleSceneRect = mapRectToScene(body).adjusted(-12 * uiScale_, -12 * uiScale_, 12 * uiScale_, 12 * uiScale_);
        const QSize backdropSize = body.size().toSize() + QSize(AppTheme::px(24, uiScale_), AppTheme::px(24, uiScale_));
        const bool dark = AppTheme::isDarkTheme();
        if (cachedBackdrop_.isNull() || cachedBackdropSize_ != backdropSize || cachedSampleSceneRect_ != sampleSceneRect ||
            std::abs(cachedUiScale_ - uiScale_) > 0.0001 || cachedDark_ != dark) {
            cachedBackdrop_ = sampledCanvasGlass(sampleSceneRect, uiScale_, backdropSize);
            cachedBackdropSize_ = backdropSize;
            cachedSampleSceneRect_ = sampleSceneRect;
            cachedUiScale_ = uiScale_;
            cachedDark_ = dark;
        }
        if (!cachedBackdrop_.isNull()) {
            painter->drawPixmap(body.adjusted(-12 * uiScale_, -12 * uiScale_, 12 * uiScale_, 12 * uiScale_),
                                cachedBackdrop_,
                                QRectF(QPointF(0, 0), cachedBackdrop_.deviceIndependentSize()));
        }
        painter->restore();

        QLinearGradient glass(body.topLeft(), body.bottomLeft());
        if (AppTheme::isDarkTheme()) {
            glass.setColorAt(0, QColor(46, 46, 47, hovered_ ? 150 : 132));
            glass.setColorAt(0.56, QColor(25, 26, 27, hovered_ ? 116 : 96));
            glass.setColorAt(1, QColor(18, 19, 20, hovered_ ? 150 : 132));
        } else {
            glass.setColorAt(0, QColor(255, 255, 255, hovered_ ? 150 : 128));
            glass.setColorAt(0.55, QColor(238, 238, 238, hovered_ ? 100 : 82));
            glass.setColorAt(1, QColor(214, 214, 214, hovered_ ? 126 : 104));
        }
        painter->setBrush(glass);
        painter->setPen(QPen(isSelected() ? colors.nodeSelected : stateColor,
                             (isSelected() || runState_ != NodeExecutionState::NotExecuted) ? 2.3 * uiScale_ : 1.1 * uiScale_));
        painter->drawPath(rectPath(body));

        QRectF header = body;
        header.setHeight(metrics.headerHeight);
        QLinearGradient highlight(header.topLeft(), header.bottomLeft());
        highlight.setColorAt(0, QColor(255, 255, 255, AppTheme::isDarkTheme() ? 70 : 130));
        highlight.setColorAt(1, QColor(255, 255, 255, AppTheme::isDarkTheme() ? 16 : 34));
        painter->setPen(Qt::NoPen);
        painter->setBrush(highlight);
        painter->drawPath(rectPath(header));

        painter->setPen(QPen(QColor(255, 255, 255, 150), 1));
        painter->drawLine(body.left(), body.top() + 1, body.right(), body.top() + 1);

        if (runState_ != NodeExecutionState::NotExecuted) {
            const QRectF strip(body.left() + 8 * uiScale_, body.bottom() - 6 * uiScale_,
                               body.width() - 16 * uiScale_, 3 * uiScale_);
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(stateColor.red(), stateColor.green(), stateColor.blue(), 110));
            painter->drawRect(strip);
            if (runState_ == NodeExecutionState::Running) {
                const qreal segmentWidth = std::max<qreal>(18.0 * uiScale_, strip.width() * 0.22);
                const qreal travel = strip.width() + segmentWidth;
                const qreal offset = std::fmod(animationPhase_ * 8.0 * uiScale_, travel) - segmentWidth;
                QRectF segment(strip.left() + offset, strip.top(), segmentWidth, strip.height());
                segment = segment.intersected(strip);
                painter->setBrush(stateColor.lighter(135));
                painter->drawRect(segment);
            } else {
                painter->setBrush(stateColor);
                painter->drawRect(strip);
            }
        }

        if (elapsedMs_ >= 0 || runState_ == NodeExecutionState::CacheHit) {
            const QString text = runState_ == NodeExecutionState::CacheHit ? "缓存" : QString("%1 ms").arg(elapsedMs_);
            QFont font = painter->font();
            font.setPointSizeF(std::max<qreal>(7.5, metrics.labelSize - 1.0));
            font.setBold(true);
            painter->setFont(font);
            QFontMetricsF fm(font);
            const QRectF badge(body.right() - fm.horizontalAdvance(text) - 22 * uiScale_,
                               body.top() + 9 * uiScale_,
                               fm.horizontalAdvance(text) + 12 * uiScale_,
                               17 * uiScale_);
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(stateColor.red(), stateColor.green(), stateColor.blue(), AppTheme::isDarkTheme() ? 88 : 54));
            painter->drawRect(badge);
            painter->setPen(stateColor.darker(AppTheme::isDarkTheme() ? 80 : 120));
            painter->drawText(badge, Qt::AlignCenter, text);
        }
    }

private:
    QWidget* makeParameterEditor(const NodeParameter& parameter)
    {
        const QVariant value = node_->parameterValue(parameter.name);
        if (parameter.type == ParameterType::Integer) {
            auto* editor = new GuiCompat::SpinBox;
            editor->setRange(int(parameter.min), int(parameter.max));
            editor->setValue(value.toInt());
            QObject::connect(editor, &QSpinBox::valueChanged, editor, [this, name = parameter.name](int next) {
                if (window_) {
                    window_->setNodeParameterForNode(nodeId_, name, next);
                }
            });
            return editor;
        }
        if (parameter.type == ParameterType::Double) {
            auto* editor = new GuiCompat::DoubleSpinBox;
            editor->setRange(parameter.min, parameter.max);
            editor->setSingleStep(0.05);
            editor->setValue(value.toDouble());
            QObject::connect(editor, &QDoubleSpinBox::valueChanged, editor, [this, name = parameter.name](double next) {
                if (window_) {
                    window_->setNodeParameterForNode(nodeId_, name, next);
                }
            });
            return editor;
        }
        if (parameter.type == ParameterType::Boolean) {
            auto* editor = new GuiCompat::CheckBox;
            editor->setChecked(value.toBool());
            QObject::connect(editor, &QCheckBox::toggled, editor, [this, name = parameter.name](bool next) {
                if (window_) {
                    window_->setNodeParameterForNode(nodeId_, name, next);
                }
            });
            return editor;
        }
        if (parameter.type == ParameterType::Choice) {
            auto* editor = new GuiCompat::ComboBox;
            editor->addItems(parameter.options);
            editor->setCurrentText(value.toString());
            QObject::connect(editor, &QComboBox::currentTextChanged, editor, [this, name = parameter.name](const QString& next) {
                if (window_) {
                    window_->setNodeParameterForNode(nodeId_, name, next);
                }
            });
            return editor;
        }

        auto* container = new QWidget;
        auto* layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(AppTheme::px(6, uiScale_));
        auto* edit = new GuiCompat::LineEdit;
        edit->setText(value.toString());
        layout->addWidget(edit);
        if (parameter.type == ParameterType::FileOpen || parameter.type == ParameterType::FileSave || parameter.type == ParameterType::Color) {
            auto* button = new GuiCompat::PushButton("...");
            button->setFixedWidth(AppTheme::px(34, uiScale_));
            layout->addWidget(button);
            QObject::connect(button, &QPushButton::clicked, button, [this, edit, parameter] {
                if (!window_) {
                    return;
                }
                QString next;
                if (parameter.type == ParameterType::FileOpen) {
                    next = QFileDialog::getOpenFileName(window_, "选择图片", {}, "Images (*.png *.jpg *.jpeg *.bmp *.webp);;All files (*)");
                } else if (parameter.type == ParameterType::FileSave) {
                    next = QFileDialog::getSaveFileName(window_, "选择导出路径", edit->text(), "PNG (*.png);;JPEG (*.jpg);;All files (*)");
                } else {
                    QColor color = QColorDialog::getColor(QColor(edit->text()), window_);
                    if (color.isValid()) {
                        next = color.name();
                    }
                }
                if (!next.isEmpty()) {
                    edit->setText(next);
                    window_->setNodeParameterForNode(nodeId_, parameter.name, next);
                }
            });
        }
        QObject::connect(edit, &QLineEdit::editingFinished, edit, [this, edit, name = parameter.name] {
            if (window_) {
                window_->setNodeParameterForNode(nodeId_, name, edit->text());
            }
        });
        return container;
    }

    void buildParameterPanel()
    {
        const QVector<NodeParameter> parameters = node_->parameterDefinitions();
        if (parameters.isEmpty()) {
            return;
        }
        const auto metrics = AppTheme::nodeMetrics(uiScale_);
        auto* panel = new QWidget;
        panel->setObjectName("nodeInlineParameters");
        panel->setStyleSheet(QString(R"(
            QWidget#nodeInlineParameters {
                background: rgba(0, 0, 0, 68);
                border: 1px solid rgba(255, 255, 255, 42);
                border-radius: 0px;
            }
            QLabel {
                color: %2;
                font-weight: 600;
            }
        )").arg(AppTheme::px(8, uiScale_)).arg(AppTheme::colors().textSecondary.name()));
        auto* form = new QFormLayout(panel);
        form->setContentsMargins(AppTheme::px(10, uiScale_), AppTheme::px(8, uiScale_),
                                  AppTheme::px(10, uiScale_), AppTheme::px(8, uiScale_));
        form->setHorizontalSpacing(AppTheme::px(8, uiScale_));
        form->setVerticalSpacing(AppTheme::px(6, uiScale_));
        for (const auto& parameter : parameters) {
            auto* label = new QLabel(parameter.displayName);
            label->setMinimumWidth(AppTheme::px(54, uiScale_));
            form->addRow(label, makeParameterEditor(parameter));
        }
        panel->setFixedWidth(int(metrics.width - 24 * uiScale_));
        panel->adjustSize();
        const qreal panelHeight = panel->sizeHint().height();
        expandedHeight_ = compactHeight_ + panelHeight + 18 * uiScale_;

        parameterProxy_ = new QGraphicsProxyWidget(this);
        parameterProxy_->setWidget(panel);
        parameterProxy_->setZValue(2);
        parameterProxy_->setPos(12 * uiScale_, compactHeight_ + 4 * uiScale_);
        parameterProxy_->setVisible(false);
    }

    void updateParameterExpansion(bool expanded)
    {
        if (expanded_ == expanded) {
            return;
        }
        expanded_ = expanded;
        const qreal targetHeight = expanded_ && parameterProxy_ ? expandedHeight_ : compactHeight_;
        prepareGeometryChange();
        setRect(0, 0, rect().width(), targetHeight);
        if (parameterProxy_) {
            parameterProxy_->setVisible(expanded_);
        }
        update();
    }

    MainWindow* window_ = nullptr;
    QString nodeId_;
    QSharedPointer<ImageNode> node_;
    double uiScale_ = 1.0;
    QVector<PortItem*> ports_;
    QGraphicsProxyWidget* parameterProxy_ = nullptr;
    QPixmap cachedBackdrop_;
    QRectF cachedSampleSceneRect_;
    QSize cachedBackdropSize_;
    qreal compactHeight_ = 0.0;
    qreal expandedHeight_ = 0.0;
    double cachedUiScale_ = 0.0;
    NodeExecutionState runState_ = NodeExecutionState::NotExecuted;
    qint64 elapsedMs_ = -1;
    int animationPhase_ = 0;
    QPointF dragStartPos_;
    bool hovered_ = false;
    bool pressed_ = false;
    bool expanded_ = false;
    bool cachedDark_ = false;
};

class EdgeItem final : public QGraphicsPathItem {
public:
    EdgeItem(int edgeIndex, const QPointF& start, const QPointF& end, double uiScale)
        : QGraphicsPathItem(), edgeIndex_(edgeIndex), uiScale_(uiScale)
    {
        setFlags(QGraphicsItem::ItemIsSelectable);
        setAcceptHoverEvents(true);
        setToolTip("连线");
        setPath(makePath(start, end));
    }

    int edgeIndex() const { return edgeIndex_; }
    void setFailureRelated(bool failureRelated)
    {
        failureRelated_ = failureRelated;
        update();
    }

    QPainterPath shape() const override
    {
        QPainterPathStroker stroker;
        stroker.setWidth(14.0 * uiScale_);
        stroker.setCapStyle(Qt::RoundCap);
        return stroker.createStroke(path());
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
    {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        const auto colors = AppTheme::colors();
        const QColor lineColor = failureRelated_ ? QColor("#ff453a") : (isSelected() ? colors.edgeSelected : colors.edge);
        const QColor glowColor = failureRelated_ ? QColor(255, 69, 58, isSelected() ? 72 : 42)
                                                 : AppTheme::isDarkTheme() ? QColor(238, 238, 238, isSelected() ? 42 : 14)
                                                                           : QColor(64, 64, 64, isSelected() ? 54 : 18);
        QPen glow(glowColor, isSelected() ? 8 * uiScale_ : 5 * uiScale_);
        glow.setCapStyle(Qt::RoundCap);
        glow.setJoinStyle(Qt::RoundJoin);
        QPen currentPen(lineColor, isSelected() ? 3.2 * uiScale_ : 2.1 * uiScale_);
        currentPen.setCapStyle(Qt::RoundCap);
        currentPen.setJoinStyle(Qt::RoundJoin);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(glow);
        painter->drawPath(path());
        painter->setPen(currentPen);
        painter->drawPath(path());
    }

private:
    QPainterPath makePath(const QPointF& start, const QPointF& end) const
    {
        QPainterPath path(start);
        const qreal dx = std::max<qreal>(80 * uiScale_, std::abs(end.x() - start.x()) * 0.5);
        const QPointF c1(start.x() + dx, start.y());
        const QPointF c2(end.x() - dx, end.y());
        path.cubicTo(c1, c2, end);
        return path;
    }

    int edgeIndex_ = -1;
    double uiScale_ = 1.0;
    bool failureRelated_ = false;
};

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
        imageLabel_->setStyleSheet("background:#111;");
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

class MiniMapWidget final : public QWidget {
public:
    MiniMapWidget(WorkflowGraph* graph, QGraphicsView* view, double* uiScale, QWidget* parent = nullptr)
        : QWidget(parent), graph_(graph), view_(view), uiScale_(uiScale)
    {
        setObjectName("miniMap");
        setCursor(Qt::PointingHandCursor);
        setToolTip("小地图：点击或拖拽快速定位画布");
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const QRectF box = rect().adjusted(1, 1, -1, -1);
        painter.setPen(QPen(QColor(128, 128, 128, 120), 1));
        painter.setBrush(QColor(255, 255, 255, AppTheme::isDarkTheme() ? 36 : 190));
        painter.drawRect(box);

        const QRectF world = worldRect();
        if (!world.isValid() || world.isEmpty()) {
            return;
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(AppTheme::isDarkTheme() ? QColor("#b8b8b8") : QColor("#505050"));
        for (const auto& record : graph_->nodes()) {
            QRectF nodeRect(record.position, QSizeF(AppTheme::nodeMetrics(scale()).width, AppTheme::nodeMetrics(scale()).headerHeight + 78 * scale()));
            painter.drawRect(mapWorldRect(nodeRect, world));
        }

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(AppTheme::isDarkTheme() ? QColor("#f2f2f2") : QColor("#202020"), 1.6));
        painter.drawRect(mapWorldRect(viewSceneRect(), world));
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        centerAt(event->position());
        event->accept();
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (event->buttons().testFlag(Qt::LeftButton)) {
            centerAt(event->position());
            event->accept();
            return;
        }
        QWidget::mouseMoveEvent(event);
    }

private:
    double scale() const
    {
        return uiScale_ ? *uiScale_ : 1.0;
    }

    QRectF viewSceneRect() const
    {
        if (!view_) {
            return {};
        }
        return view_->mapToScene(view_->viewport()->rect()).boundingRect();
    }

    QRectF worldRect() const
    {
        QRectF world = viewSceneRect();
        const auto metrics = AppTheme::nodeMetrics(scale());
        for (const auto& record : graph_->nodes()) {
            QRectF nodeRect(record.position, QSizeF(metrics.width, metrics.headerHeight + 78 * scale()));
            world = world.united(nodeRect);
        }
        return world.adjusted(-120 * scale(), -120 * scale(), 120 * scale(), 120 * scale());
    }

    QRectF contentRect() const
    {
        return rect().adjusted(10, 10, -10, -10);
    }

    QPointF mapWorldPoint(const QPointF& point, const QRectF& world) const
    {
        const QRectF content = contentRect();
        const double sx = content.width() / std::max<qreal>(1.0, world.width());
        const double sy = content.height() / std::max<qreal>(1.0, world.height());
        const double s = std::min(sx, sy);
        const QSizeF mappedSize(world.width() * s, world.height() * s);
        const QPointF offset(content.center().x() - mappedSize.width() / 2.0,
                             content.center().y() - mappedSize.height() / 2.0);
        return QPointF(offset.x() + (point.x() - world.left()) * s,
                       offset.y() + (point.y() - world.top()) * s);
    }

    QRectF mapWorldRect(const QRectF& source, const QRectF& world) const
    {
        const QPointF topLeft = mapWorldPoint(source.topLeft(), world);
        const QPointF bottomRight = mapWorldPoint(source.bottomRight(), world);
        return QRectF(topLeft, bottomRight).normalized();
    }

    QPointF unmapWorldPoint(const QPointF& point, const QRectF& world) const
    {
        const QRectF content = contentRect();
        const double sx = content.width() / std::max<qreal>(1.0, world.width());
        const double sy = content.height() / std::max<qreal>(1.0, world.height());
        const double s = std::min(sx, sy);
        const QSizeF mappedSize(world.width() * s, world.height() * s);
        const QPointF offset(content.center().x() - mappedSize.width() / 2.0,
                             content.center().y() - mappedSize.height() / 2.0);
        return QPointF(world.left() + (point.x() - offset.x()) / s,
                       world.top() + (point.y() - offset.y()) / s);
    }

    void centerAt(const QPointF& widgetPosition)
    {
        const QRectF world = worldRect();
        if (!view_ || !world.isValid()) {
            return;
        }
        view_->centerOn(unmapWorldPoint(widgetPosition, world));
        update();
    }

    WorkflowGraph* graph_ = nullptr;
    QGraphicsView* view_ = nullptr;
    double* uiScale_ = nullptr;
};

class QuickNodePalettePopup final : public QDialog {
public:
    explicit QuickNodePalettePopup(double uiScale, QWidget* parent = nullptr)
        : QDialog(parent, Qt::Popup | Qt::FramelessWindowHint)
    {
        setObjectName("quickNodePalettePopup");
        setAttribute(Qt::WA_DeleteOnClose, false);

        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(AppTheme::px(12, uiScale), AppTheme::px(12, uiScale),
                                 AppTheme::px(12, uiScale), AppTheme::px(12, uiScale));
        root->setSpacing(AppTheme::px(8, uiScale));

        search_ = new GuiCompat::LineEdit;
        search_->setPlaceholderText("搜索节点名称、类型或分类");
        search_->installEventFilter(this);
        root->addWidget(search_);

        list_ = new QListWidget;
        list_->setObjectName("quickNodeList");
        list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list_->setSelectionMode(QAbstractItemView::SingleSelection);
        list_->setUniformItemSizes(false);
        list_->setMinimumWidth(AppTheme::px(340, uiScale));
        list_->setMinimumHeight(AppTheme::px(260, uiScale));
        root->addWidget(list_);

        descriptors_ = NodeFactory::instance().descriptors();
        refilter();

        connect(search_, &QLineEdit::textChanged, this, [this] { refilter(); });
        connect(search_, &QLineEdit::returnPressed, this, [this] { acceptCurrent(); });
        connect(list_, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
            if (!item) {
                return;
            }
            selectedType_ = item->data(Qt::UserRole).toString();
            accept();
        });
    }

    QString selectedType() const { return selectedType_; }

protected:
    bool eventFilter(QObject* object, QEvent* event) override
    {
        if (object == search_ && event->type() == QEvent::KeyPress) {
            auto* key = static_cast<QKeyEvent*>(event);
            if (key->key() == Qt::Key_Down || key->key() == Qt::Key_Up ||
                key->key() == Qt::Key_PageDown || key->key() == Qt::Key_PageUp) {
                QKeyEvent forwarded(key->type(), key->key(), key->modifiers(), key->text(), key->isAutoRepeat(), key->count());
                QApplication::sendEvent(list_, &forwarded);
                return true;
            }
            if (key->key() == Qt::Key_Escape) {
                reject();
                return true;
            }
        }
        return QDialog::eventFilter(object, event);
    }

    void showEvent(QShowEvent* event) override
    {
        QDialog::showEvent(event);
        search_->setFocus(Qt::PopupFocusReason);
        search_->selectAll();
    }

private:
    void refilter()
    {
        list_->clear();
        const QString query = search_->text().trimmed().toLower();
        const QStringList terms = query.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        for (const auto& descriptor : descriptors_) {
            const QString haystack = QString("%1 %2 %3")
                                        .arg(descriptor.displayName, descriptor.typeName, descriptor.category)
                                        .toLower();
            bool matches = true;
            for (const QString& term : terms) {
                if (!haystack.contains(term)) {
                    matches = false;
                    break;
                }
            }
            if (!matches) {
                continue;
            }

            auto* item = new QListWidgetItem(QString("%1\n%2 · %3")
                                                 .arg(descriptor.displayName, descriptor.typeName, descriptor.category));
            item->setData(Qt::UserRole, descriptor.typeName);
            item->setToolTip(QString("%1 / %2").arg(descriptor.category, descriptor.typeName));
            list_->addItem(item);
        }
        if (list_->count() > 0) {
            list_->setCurrentRow(0);
        }
    }

    void acceptCurrent()
    {
        if (list_->count() <= 0) {
            return;
        }
        auto* item = list_->currentItem();
        if (!item) {
            list_->setCurrentRow(0);
            item = list_->currentItem();
        }
        if (!item) {
            return;
        }
        selectedType_ = item->data(Qt::UserRole).toString();
        accept();
    }

    GuiCompat::LineEdit* search_ = nullptr;
    QListWidget* list_ = nullptr;
    QVector<NodeDescriptor> descriptors_;
    QString selectedType_;
};

NodeItem* nodeItemFrom(QGraphicsItem* item)
{
    while (item) {
        if (auto* node = dynamic_cast<NodeItem*>(item)) {
            return node;
        }
        item = item->parentItem();
    }
    return nullptr;
}

EdgeItem* edgeItemFrom(QGraphicsItem* item)
{
    return dynamic_cast<EdgeItem*>(item);
}

bool findPortInfo(const QSharedPointer<ImageNode>& node, const QString& portName, PortDirection direction, PortInfo* out)
{
    if (!node) {
        return false;
    }
    const QVector<PortInfo> ports = direction == PortDirection::Input ? node->inputPorts() : node->outputPorts();
    for (const auto& port : ports) {
        if (port.name == portName) {
            if (out) {
                *out = port;
            }
            return true;
        }
    }
    return false;
}

class WorkflowScene final : public QGraphicsScene {
public:
    explicit WorkflowScene(MainWindow* window) : window_(window) {}

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override
    {
        QGraphicsItem* hit = itemAt(event->scenePos(), QTransform());
        if (auto* port = dynamic_cast<PortItem*>(hit)) {
            if (port->port().direction == PortDirection::Output) {
                startPendingLine(port, event->scenePos());
            } else if (!pendingNode_.isEmpty()) {
                window_->requestConnect(pendingNode_, pendingPort_, port->nodeId(), port->port().name);
                clearPendingLine();
            }
            event->accept();
            return;
        }
        if (!pendingNode_.isEmpty() && event->button() == Qt::LeftButton) {
            if (snappedPort_ && snappedConnectable_) {
                window_->requestConnect(pendingNode_, pendingPort_, snappedPort_->nodeId(), snappedPort_->port().name);
            }
            clearPendingLine();
            event->accept();
            return;
        }
        QGraphicsScene::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (pendingLine_) {
            updatePendingLine(event->scenePos());
        }
        QGraphicsScene::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (!pendingNode_.isEmpty() && event->button() == Qt::LeftButton && snappedPort_ && snappedConnectable_) {
            window_->requestConnect(pendingNode_, pendingPort_, snappedPort_->nodeId(), snappedPort_->port().name);
            clearPendingLine();
            event->accept();
            return;
        }
        QGraphicsScene::mouseReleaseEvent(event);
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override
    {
        if (!pendingNode_.isEmpty()) {
            clearPendingLine();
        }
        QGraphicsItem* hit = itemAt(event->scenePos(), QTransform());
        if (auto* node = nodeItemFrom(hit)) {
            if (!node->isSelected()) {
                clearSelection();
                node->setSelected(true);
            }
            window_->showNodeContextMenu(node->nodeId(), event->screenPos());
            event->accept();
            return;
        }
        if (auto* edge = edgeItemFrom(hit)) {
            clearSelection();
            edge->setSelected(true);
            window_->showEdgeContextMenu(edge->edgeIndex(), event->screenPos());
            event->accept();
            return;
        }
        QMenu menu;
        QAction* addNodeAction = menu.addAction("快捷添加节点...");
        QAction* chosen = menu.exec(event->screenPos());
        if (chosen == addNodeAction) {
            window_->showQuickNodePaletteAt(event->scenePos());
            event->accept();
            return;
        }
        QGraphicsScene::contextMenuEvent(event);
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        if (event->key() == Qt::Key_Escape && !pendingNode_.isEmpty()) {
            clearPendingLine();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
            window_->removeSelectedItems();
            event->accept();
            return;
        }
        if (event->matches(QKeySequence::Copy)) {
            window_->copySelectedNode();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::NoModifier) {
            window_->showQuickNodePalette();
            event->accept();
            return;
        }
        QGraphicsScene::keyPressEvent(event);
    }

private:
    void startPendingLine(PortItem* port, const QPointF& cursorPosition)
    {
        clearPendingLine();
        pendingNode_ = port->nodeId();
        pendingPort_ = port->port().name;
        pendingStart_ = port->mapToScene(port->rect().center());

        auto* line = new QGraphicsPathItem(pendingPath(pendingStart_, cursorPosition));
        QPen pen(AppTheme::colors().pendingEdge, 2.4, Qt::DashLine);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        line->setPen(pen);
        line->setZValue(-0.5);
        addItem(line);
        pendingLine_ = line;
        updatePendingLine(cursorPosition);
    }

    void updatePendingLine(const QPointF& cursorPosition)
    {
        updateSnapTarget(cursorPosition);
        const QPointF end = snappedPort_ && snappedConnectable_
                                ? snappedPort_->mapToScene(snappedPort_->rect().center())
                                : cursorPosition;
        pendingLine_->setPath(pendingPath(pendingStart_, end));
        QPen pen(snappedPort_ && snappedConnectable_ ? (AppTheme::isDarkTheme() ? QColor("#f2f2f2") : QColor("#202020")) :
                     blockedNearPort_ ? (AppTheme::isDarkTheme() ? QColor("#707070") : QColor("#404040")) : AppTheme::colors().pendingEdge,
                 2.4, Qt::DashLine);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        pendingLine_->setPen(pen);
    }

    void updateSnapTarget(const QPointF& cursorPosition)
    {
        clearPortFeedback();
        if (pendingNode_.isEmpty()) {
            return;
        }

        WorkflowValidator validator;
        PortItem* nearestConnectable = nullptr;
        PortItem* nearestBlocked = nullptr;
        qreal nearestConnectableDistance = std::numeric_limits<qreal>::max();
        qreal nearestBlockedDistance = std::numeric_limits<qreal>::max();
        const qreal snapDistance = 20.0 * (window_ ? window_->uiScale() : 1.0);

        for (auto* item : items()) {
            auto* port = dynamic_cast<PortItem*>(item);
            if (!port || port->port().direction != PortDirection::Input) {
                continue;
            }

            const QPointF center = port->mapToScene(port->rect().center());
            const qreal distance = QLineF(cursorPosition, center).length();
            const Edge candidate{pendingNode_, pendingPort_, port->nodeId(), port->port().name};
            const bool connectable = validator.validateEdge(window_->graph(), candidate).isOk();
            if (connectable) {
                port->setConnectionFeedback(PortConnectionFeedback::Connectable);
                highlightedPorts_.append(port);
                if (distance <= snapDistance && distance < nearestConnectableDistance) {
                    nearestConnectableDistance = distance;
                    nearestConnectable = port;
                }
            } else if (distance <= snapDistance && distance < nearestBlockedDistance) {
                nearestBlockedDistance = distance;
                nearestBlocked = port;
            }
        }

        if (nearestConnectable) {
            nearestConnectable->setConnectionFeedback(PortConnectionFeedback::Snapped);
            snappedPort_ = nearestConnectable;
            snappedConnectable_ = true;
            return;
        }

        if (nearestBlocked) {
            nearestBlocked->setConnectionFeedback(PortConnectionFeedback::Blocked);
            highlightedPorts_.append(nearestBlocked);
            blockedNearPort_ = nearestBlocked;
        }
    }

    QPainterPath pendingPath(const QPointF& start, const QPointF& end) const
    {
        QPainterPath path(start);
        const qreal dx = std::max<qreal>(80.0, std::abs(end.x() - start.x()) * 0.5);
        path.cubicTo(QPointF(start.x() + dx, start.y()), QPointF(end.x() - dx, end.y()), end);
        return path;
    }

    void clearPendingLine()
    {
        clearPortFeedback();
        if (pendingLine_) {
            removeItem(pendingLine_);
            delete pendingLine_;
            pendingLine_ = nullptr;
        }
        pendingNode_.clear();
        pendingPort_.clear();
        pendingStart_ = {};
    }

    void clearPortFeedback()
    {
        for (auto* port : highlightedPorts_) {
            if (port) {
                port->setConnectionFeedback(PortConnectionFeedback::None);
            }
        }
        highlightedPorts_.clear();
        snappedPort_ = nullptr;
        blockedNearPort_ = nullptr;
        snappedConnectable_ = false;
    }

    MainWindow* window_ = nullptr;
    QString pendingNode_;
    QString pendingPort_;
    QPointF pendingStart_;
    QGraphicsPathItem* pendingLine_ = nullptr;
    QVector<PortItem*> highlightedPorts_;
    PortItem* snappedPort_ = nullptr;
    PortItem* blockedNearPort_ = nullptr;
    bool snappedConnectable_ = false;
};

class ZoomGraphicsView final : public QGraphicsView {
public:
    ZoomGraphicsView(QGraphicsScene* scene, MainWindow* window)
        : QGraphicsView(scene), window_(window)
    {
    }

protected:
    bool event(QEvent* event) override
    {
        if (event->type() == QEvent::KeyPress) {
            auto* key = static_cast<QKeyEvent*>(event);
            if (key->key() == Qt::Key_Tab && key->modifiers() == Qt::NoModifier) {
                window_->showQuickNodePalette();
                return true;
            }
        }
        return QGraphicsView::event(event);
    }

    void drawBackground(QPainter* painter, const QRectF& rect) override
    {
        const double scale = window_ ? window_->uiScale() : 1.0;
        paintCanvasBackground(painter, rect, scale);
    }

    void wheelEvent(QWheelEvent* event) override
    {
        const int delta = event->angleDelta().y();
        if (delta == 0) {
            event->ignore();
            return;
        }
        const double steps = static_cast<double>(delta) / 120.0;
        window_->applyZoomFactor(std::pow(1.05, steps));
        event->accept();
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton &&
            event->modifiers().testFlag(Qt::ShiftModifier) &&
            !itemAt(event->position().toPoint())) {
            panCandidate_ = false;
            panning_ = false;
            setDragMode(QGraphicsView::RubberBandDrag);
            QGraphicsView::mousePressEvent(event);
            return;
        }
        if (event->button() == Qt::LeftButton && !itemAt(event->position().toPoint())) {
            panCandidate_ = true;
            panning_ = false;
            lastPanPoint_ = event->position().toPoint();
            pressTimer_.start();
            setCursor(Qt::OpenHandCursor);
            event->accept();
            return;
        }
        QGraphicsView::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (panCandidate_ || panning_) {
            const QPoint current = event->position().toPoint();
            const QPoint delta = current - lastPanPoint_;
            if (!panning_ && (pressTimer_.elapsed() >= 160 || delta.manhattanLength() >= QApplication::startDragDistance())) {
                panning_ = true;
                setCursor(Qt::ClosedHandCursor);
            }
            if (panning_) {
                horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
                verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
                lastPanPoint_ = current;
            }
            event->accept();
            return;
        }
        QGraphicsView::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (dragMode() == QGraphicsView::RubberBandDrag) {
            QGraphicsView::mouseReleaseEvent(event);
            setDragMode(QGraphicsView::NoDrag);
            window_->updateMiniMap();
            return;
        }
        if (event->button() == Qt::LeftButton && (panCandidate_ || panning_)) {
            panCandidate_ = false;
            panning_ = false;
            unsetCursor();
            window_->updateMiniMap();
            event->accept();
            return;
        }
        QGraphicsView::mouseReleaseEvent(event);
    }

    void scrollContentsBy(int dx, int dy) override
    {
        QGraphicsView::scrollContentsBy(dx, dy);
        if (window_) {
            window_->updateMiniMap();
        }
    }

private:
    MainWindow* window_ = nullptr;
    bool panCandidate_ = false;
    bool panning_ = false;
    QPoint lastPanPoint_;
    QElapsedTimer pressTimer_;
};

}

class TerminalPanel final : public QWidget {
public:
    explicit TerminalPanel(double uiScale, QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setObjectName("terminalPanel");
        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(AppTheme::px(8, uiScale), AppTheme::px(8, uiScale),
                                 AppTheme::px(8, uiScale), AppTheme::px(8, uiScale));
        root->setSpacing(AppTheme::px(6, uiScale));

        output_ = new QPlainTextEdit;
        output_->setObjectName("terminalOutput");
        output_->setReadOnly(true);
        output_->setLineWrapMode(QPlainTextEdit::NoWrap);
        QFont mono("Menlo");
        mono.setStyleHint(QFont::Monospace);
        mono.setPointSizeF(std::max(10.0, 11.5 * uiScale));
        output_->setFont(mono);
        root->addWidget(output_, 1);

        auto* controls = new QWidget;
        auto* controlsLayout = new QHBoxLayout(controls);
        controlsLayout->setContentsMargins(0, 0, 0, 0);
        controlsLayout->setSpacing(AppTheme::px(6, uiScale));
        input_ = new GuiCompat::LineEdit;
        input_->setPlaceholderText("输入命令后按 Enter");
        auto* runButton = new GuiCompat::PushButton("运行");
        auto* clearButton = new GuiCompat::PushButton("清空");
        auto* restartButton = new GuiCompat::PushButton("重启");
        controlsLayout->addWidget(input_, 1);
        controlsLayout->addWidget(runButton);
        controlsLayout->addWidget(clearButton);
        controlsLayout->addWidget(restartButton);
        root->addWidget(controls);

        process_.setProcessChannelMode(QProcess::MergedChannels);
        process_.setWorkingDirectory(QDir::currentPath());
        connect(&process_, &QProcess::readyReadStandardOutput, this, [this] {
            append(QString::fromLocal8Bit(process_.readAllStandardOutput()));
        });
        connect(&process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            Q_UNUSED(error);
            append(QString("终端错误：%1\n").arg(process_.errorString()));
        });
        connect(&process_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this](int code, QProcess::ExitStatus status) {
            Q_UNUSED(status);
            append(QString("\n[进程结束，退出码 %1]\n").arg(code));
        });
        connect(input_, &QLineEdit::returnPressed, this, [this] { submitCommand(); });
        connect(runButton, &QPushButton::clicked, this, [this] { submitCommand(); });
        connect(clearButton, &QPushButton::clicked, output_, &QPlainTextEdit::clear);
        connect(restartButton, &QPushButton::clicked, this, [this] { restartShell(); });

        startShell();
    }

    void restartShell()
    {
        if (process_.state() != QProcess::NotRunning) {
            process_.kill();
            process_.waitForFinished(800);
        }
        startShell();
    }

private:
    void append(const QString& text)
    {
        output_->moveCursor(QTextCursor::End);
        output_->insertPlainText(text);
        output_->moveCursor(QTextCursor::End);
    }

    void startShell()
    {
        QString program;
        QStringList args;
#ifdef Q_OS_WIN
        program = QStandardPaths::findExecutable("powershell.exe");
        if (!program.isEmpty()) {
            args << "-NoLogo" << "-NoExit";
        } else {
            program = QStandardPaths::findExecutable("cmd.exe");
        }
#else
        program = qEnvironmentVariable("SHELL");
        if (program.isEmpty()) {
            program = QStandardPaths::findExecutable("bash");
        }
        if (program.isEmpty()) {
            program = "/bin/bash";
        }
        args << "-i";
#endif
        append(QString("[启动终端] %1 %2\n").arg(program, args.join(' ')).trimmed() + "\n");
        process_.start(program, args);
    }

    void submitCommand()
    {
        const QString command = input_->text();
        if (command.trimmed().isEmpty()) {
            return;
        }
        if (process_.state() == QProcess::NotRunning) {
            startShell();
        }
        append(QString("> %1\n").arg(command));
        process_.write(command.toLocal8Bit());
        process_.write("\n");
        input_->clear();
    }

    QProcess process_;
    QPlainTextEdit* output_ = nullptr;
    QLineEdit* input_ = nullptr;
};

MainWindow::MainWindow(QWidget* parent)
    : GuiCompat::MainWindowBase(parent)
{
    NodeFactory::instance().registerBuiltins();
    GuiCompat::configureMainWindow(this);
    undoStack_ = new QUndoStack(this);
    connect(undoStack_, &QUndoStack::cleanChanged, this, [this] { updateWindowTitle(); });
    connect(undoStack_, &QUndoStack::indexChanged, this, [this] { updateWindowTitle(); });
    livePreviewTimer_ = new QTimer(this);
    livePreviewTimer_->setSingleShot(true);
    livePreviewTimer_->setInterval(350);
    connect(livePreviewTimer_, &QTimer::timeout, this, [this] { runLivePreview(); });
    runAnimationTimer_ = new QTimer(this);
    runAnimationTimer_->setInterval(90);
    connect(runAnimationTimer_, &QTimer::timeout, this, [this] { updateRunAnimation(); });
    updateWindowTitle();
    resize(1280, 820);
    QSettings settings;
    uiScale_ = AppTheme::clampedScale(settings.value("mainWindow/uiScale", 1.0).toDouble());
    AppTheme::setThemePreference(settings.value("mainWindow/theme", "system").toString());
    if (auto* app = qobject_cast<QApplication*>(QApplication::instance())) {
        AppTheme::apply(*app, uiScale_);
    }
    createActions();
    createLayout();
    initializeWorkbook();
    rebuildNodeMenus();
    applyUiScale();
}

void MainWindow::createActions()
{
    // QMenuBar is intentionally kept: macOS renders it in the system menu bar,
    // while Windows/Linux render it inside the window. The same QAction objects
    // are also exposed through the custom in-window header and toolbars.
    fileMenu_ = menuBar()->addMenu("文件");
    mainToolbar_ = addToolBar("主工具栏");
    mainToolbar_->setObjectName("mainToolbar");
    mainToolbar_->setMovable(false);
    mainToolbar_->setFloatable(false);
    auto makeAction = [&](const QString& text, const QString& iconName, auto slot) {
        auto* action = new QAction(text, this);
        action->setIcon(lineIcon(iconName));
        action->setData(iconName);
        action->setToolTip(text);
        action->setStatusTip(text);
        connect(action, &QAction::triggered, this, slot);
        return action;
    };
    auto* newAction = makeAction("新建", "new", [this] { newWorkflow(); });
    auto* openAction = makeAction("打开", "open", [this] { openWorkflow(); });
    auto* saveAction = makeAction("保存", "save", [this] { saveWorkflow(); });
    auto* saveAsAction = makeAction("另存为", "saveAs", [this] { saveWorkflowAs(); });
    exportWorkflowCopyAction_ = makeAction("导出工作流", "exportWorkflow", [this] { exportWorkflowCopy(); });
    exportPreviewAction_ = makeAction("导出预览结果", "exportPreview", [this] { exportPreviewImage(); });
    exportPreviewAction_->setEnabled(false);
    runAction_ = makeAction("执行", "run", [this] { runWorkflow(); });
    auto* exportCanvasAction = makeAction("导出画布截图", "exportCanvas", [this] { exportCanvasImage(); });
    const QKeySequence exportShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_E);
    exportCanvasAction->setShortcut(exportShortcut);
    exportCanvasAction->setToolTip(QString("导出画布截图（%1）").arg(exportShortcut.toString(QKeySequence::NativeText)));

    fileMenu_->addAction(newAction);
    fileMenu_->addAction(openAction);
    fileMenu_->addAction(saveAction);
    fileMenu_->addAction(saveAsAction);
    fileMenu_->addSeparator();
    fileMenu_->addAction(exportWorkflowCopyAction_);
    fileMenu_->addAction(exportPreviewAction_);
    fileMenu_->addAction(exportCanvasAction);
    fileMenu_->addSeparator();
    fileMenu_->addAction(runAction_);

    mainToolbar_->addAction(newAction);
    mainToolbar_->addAction(saveAction);
    mainToolbar_->addAction(saveAsAction);
    mainToolbar_->addAction(openAction);
    mainToolbar_->addAction(exportWorkflowCopyAction_);
    mainToolbar_->addAction(exportPreviewAction_);

    returnToParentAction_ = new QAction(lineIcon("back"), "返回上一级视图", this);
    returnToParentAction_->setData("back");
    returnToParentAction_->setToolTip("返回上级图");
    returnToParentAction_->setStatusTip("返回宏节点外层图");
    connect(returnToParentAction_, &QAction::triggered, this, [this] { leaveMacroNode(); });
    returnToParentAction_->setEnabled(false);
    forwardToChildAction_ = new QAction(lineIcon("forward"), "前往下一级视图", this);
    forwardToChildAction_->setData("forward");
    forwardToChildAction_->setToolTip("前往刚离开的子图");
    forwardToChildAction_->setStatusTip("按导航历史前进");
    connect(forwardToChildAction_, &QAction::triggered, this, [this] { navigateForwardMacroNode(); });
    forwardToChildAction_->setEnabled(false);

    editMenu_ = menuBar()->addMenu("编辑");
    auto* undoAction = undoStack_->createUndoAction(this, "撤销");
    undoAction->setShortcuts(QKeySequence::Undo);
    auto* redoAction = undoStack_->createRedoAction(this, "重做");
    redoAction->setShortcuts(QKeySequence::Redo);
    editMenu_->addAction(undoAction);
    editMenu_->addAction(redoAction);
    editMenu_->addSeparator();
    auto* macroAction = editMenu_->addAction("封装为宏节点");
    connect(macroAction, &QAction::triggered, this, [this] { encapsulateSelectionAsMacro(); });
    auto* quickNodeAction = new QAction("快捷添加节点", this);
    const QKeySequence quickNodeShortcut(Qt::CTRL | Qt::Key_K);
    quickNodeAction->setShortcut(quickNodeShortcut);
    quickNodeAction->setToolTip(QString("快捷添加节点（%1；画布 Tab）").arg(quickNodeShortcut.toString(QKeySequence::NativeText)));
    quickNodeAction->setStatusTip("搜索并在当前画布位置添加节点");
    connect(quickNodeAction, &QAction::triggered, this, [this] { showQuickNodePalette(); });
    editMenu_->addAction(quickNodeAction);

    nodeOperationMenu_ = menuBar()->addMenu("节点操作");
    commandCenterMenu_ = new QMenu("更多操作", this);
    commandCenterAction_ = new QAction(lineIcon("more"), "更多操作", this);
    commandCenterAction_->setData("more");
    commandCenterAction_->setToolTip("更多操作");

    viewMenu_ = menuBar()->addMenu("视图");
    auto addViewAction = [&](const QString& text, const QString& iconName, const QList<QKeySequence>& shortcuts, auto slot) {
        auto* action = new QAction(text, this);
        action->setIcon(lineIcon(iconName));
        action->setData(iconName);
        action->setToolTip(text);
        action->setStatusTip(text);
        action->setShortcuts(shortcuts);
        connect(action, &QAction::triggered, this, slot);
        viewMenu_->addAction(action);
        return action;
    };
    auto* zoomInAction = addViewAction("放大视图", "zoomIn", {QKeySequence::ZoomIn, QKeySequence(Qt::CTRL | Qt::Key_Equal)}, [this] { increaseUiScale(); });
    auto* zoomOutAction = addViewAction("缩小视图", "zoomOut", {QKeySequence::ZoomOut}, [this] { decreaseUiScale(); });
    auto* resetScaleAction = addViewAction("恢复默认视图", "scaleReset", {QKeySequence(Qt::CTRL | Qt::Key_0)}, [this] { resetUiScale(); });
    auto* mainSpacer = new QWidget;
    mainSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainToolbar_->addWidget(mainSpacer);
    mainToolbar_->addAction(zoomInAction);
    mainToolbar_->addAction(zoomOutAction);
    mainToolbar_->addAction(resetScaleAction);

    settingsMenu_ = menuBar()->addMenu("设置");
    settingsAction_ = new QAction(lineIcon("settings"), "打开设置", this);
    settingsAction_->setData("settings");
    settingsAction_->setToolTip("打开设置");
    settingsAction_->setStatusTip("打开设置");
    connect(settingsAction_, &QAction::triggered, this, [this] { showSettingsDialog(); });
    settingsMenu_->addAction(settingsAction_);

    installDelayedTooltips(mainToolbar_);
}

void MainWindow::createLayout()
{
    scene_ = new WorkflowScene(this);
    scene_->setSceneRect(-2000, -2000, 4000, 4000);
    connect(scene_, &QGraphicsScene::selectionChanged, this, [this] {
        selectedNodeId_.clear();
        for (auto* item : scene_->selectedItems()) {
            if (auto* node = dynamic_cast<NodeItem*>(item)) {
                selectedNodeId_ = node->nodeId();
                break;
            }
        }
        rebuildProperties();
        updatePreviewForSelection();
    });
    view_ = new ZoomGraphicsView(scene_, this);
    view_->setObjectName("workflowView");
    view_->viewport()->setObjectName("workflowViewport");
    view_->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    view_->setDragMode(QGraphicsView::NoDrag);
    view_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    view_->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    view_->setFrameShape(QFrame::NoFrame);

    auto* viewContainer = new QWidget;
    viewContainer->setObjectName("canvasContainer");
    viewContainer->setAttribute(Qt::WA_StyledBackground, true);
    auto* viewStack = new QStackedLayout(viewContainer);
    viewStack->setContentsMargins(0, 0, 0, 0);
    viewStack->setStackingMode(QStackedLayout::StackAll);
    viewStack->addWidget(view_);
    canvasZoomOverlay_ = new QWidget;
    canvasZoomOverlay_->setObjectName("canvasZoomOverlay");
    canvasZoomOverlay_->setAttribute(Qt::WA_TranslucentBackground);
    auto* zoomLayout = new QVBoxLayout(canvasZoomOverlay_);
    zoomLayout->setContentsMargins(0, 0, 0, 0);
    canvasZoomInButton_ = new QToolButton;
    canvasZoomInButton_->setIcon(lineIcon("zoomIn"));
    canvasZoomInButton_->setToolTip("放大画布");
    canvasZoomOutButton_ = new QToolButton;
    canvasZoomOutButton_->setIcon(lineIcon("zoomOut"));
    canvasZoomOutButton_->setToolTip("缩小画布");
    zoomLayout->addWidget(canvasZoomInButton_, 0, Qt::AlignRight);
    zoomLayout->addWidget(canvasZoomOutButton_, 0, Qt::AlignRight);
    viewStack->addWidget(canvasZoomOverlay_);
    viewStack->setAlignment(canvasZoomOverlay_, Qt::AlignRight | Qt::AlignBottom);
    miniMap_ = new MiniMapWidget(&graph_, view_, &uiScale_);
    miniMap_->setAttribute(Qt::WA_TranslucentBackground);
    {
        QSettings settings;
        miniMap_->setVisible(settings.value("mainWindow/showMiniMap", true).toBool());
    }
    viewStack->addWidget(miniMap_);
    viewStack->setAlignment(miniMap_, Qt::AlignLeft | Qt::AlignBottom);
    connect(canvasZoomInButton_, &QToolButton::clicked, this, [this] { zoomIn(); });
    connect(canvasZoomOutButton_, &QToolButton::clicked, this, [this] { zoomOut(); });

    preview_ = new PreviewLabel;
    preview_->setObjectName("previewPanel");
    preview_->setAlignment(Qt::AlignCenter);
    static_cast<PreviewLabel*>(preview_)->setSourceImage({});

    log_ = new QListWidget;
    log_->setObjectName("logPanel");
    log_->setSelectionMode(QAbstractItemView::SingleSelection);
    log_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(log_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        focusLogNode(item);
    });
    problemLog_ = new QListWidget;
    problemLog_->setObjectName("problemPanel");
    problemLog_->setSelectionMode(QAbstractItemView::SingleSelection);
    problemLog_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(problemLog_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        focusLogNode(item);
    });
    terminalPanel_ = new TerminalPanel(uiScale_);

    auto makeLogPage = [this](QListWidget* list, const QString& titleText) {
        auto* page = new QWidget;
        page->setObjectName("bottomTabPage");
        page->setAttribute(Qt::WA_StyledBackground, true);
        auto* layout = new QVBoxLayout(page);
        layout->setContentsMargins(AppTheme::px(8, uiScale_), AppTheme::px(8, uiScale_),
                                   AppTheme::px(8, uiScale_), AppTheme::px(8, uiScale_));
        layout->setSpacing(AppTheme::px(6, uiScale_));
        auto* header = new QWidget;
        auto* headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(0, 0, 0, 0);
        auto* title = new QLabel(titleText);
        auto* clearButton = new QToolButton;
        clearButton->setIcon(lineIcon("clearLog"));
        clearButton->setToolTip(QString("清空%1").arg(titleText));
        clearButton->setAutoRaise(true);
        connect(clearButton, &QToolButton::clicked, list, &QListWidget::clear);
        headerLayout->addWidget(title);
        headerLayout->addStretch(1);
        headerLayout->addWidget(clearButton);
        layout->addWidget(header);
        layout->addWidget(list, 1);
        return page;
    };

    bottomTabs_ = new QTabWidget;
    bottomTabs_->setObjectName("bottomTabs");
    bottomTabs_->setDocumentMode(true);
    bottomTabs_->addTab(terminalPanel_, "终端");
    bottomTabs_->addTab(makeLogPage(problemLog_, "问题"), "问题");
    bottomTabs_->addTab(makeLogPage(log_, "输出"), "输出");

    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks | QMainWindow::AnimatedDocks | QMainWindow::GroupedDragging);
    GuiCompat::setMainContent(this, viewContainer);

    auto makeDock = [this](const QString& title, const QString& objectName, QWidget* widget) {
        auto* dock = new GuiCompat::DockWidget(title, this);
        dock->setObjectName(objectName);
        dock->setWidget(widget);
        dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        return dock;
    };

    previewDock_ = makeDock("预览", "previewDock", preview_);
    bottomDock_ = makeDock("终端 / 问题 / 输出", "bottomDock", bottomTabs_);
    previewDock_->setGraphicsEffect(AppTheme::makeShadow(previewDock_, uiScale_));
    bottomDock_->setGraphicsEffect(AppTheme::makeShadow(bottomDock_, uiScale_));

    addDockWidget(Qt::RightDockWidgetArea, previewDock_);
    addDockWidget(Qt::RightDockWidgetArea, bottomDock_);
    splitDockWidget(previewDock_, bottomDock_, Qt::Vertical);
    resizeDocks({previewDock_, bottomDock_}, {260, 340}, Qt::Vertical);

    layoutMenu_ = menuBar()->addMenu("布局");
    previewToggleAction_ = previewDock_->toggleViewAction();
    bottomToggleAction_ = bottomDock_->toggleViewAction();
    layoutMenu_->addAction(previewToggleAction_);
    layoutMenu_->addAction(bottomToggleAction_);
    layoutMenu_->addSeparator();

    auto addDockMoveAction = [&](const QString& text, QDockWidget* dock, Qt::DockWidgetArea area) {
        auto* action = layoutMenu_->addAction(text);
        connect(action, &QAction::triggered, this, [this, dock, area] {
            addDockWidget(area, dock);
            dock->show();
        });
        return action;
    };
    addDockMoveAction("预览在右侧", previewDock_, Qt::RightDockWidgetArea);
    addDockMoveAction("预览在左侧", previewDock_, Qt::LeftDockWidgetArea);
    addDockMoveAction("底部面板在底部", bottomDock_, Qt::BottomDockWidgetArea);
    addDockMoveAction("底部面板在右侧", bottomDock_, Qt::RightDockWidgetArea);
    layoutMenu_->addSeparator();
    auto* resetAction = layoutMenu_->addAction("重置布局");
    connect(resetAction, &QAction::triggered, this, [this] { resetDockLayout(); });
    auto* autoLayoutAction = layoutMenu_->addAction("自动布局");
    connect(autoLayoutAction, &QAction::triggered, this, [this] { autoLayoutWorkflow(); });
    auto* fullScreenAction = layoutMenu_->addAction("全屏切换");
    connect(fullScreenAction, &QAction::triggered, this, [this] { toggleFullScreenMode(); });
    previewToggleAction_->setIcon(lineIcon("dockRight"));
    previewToggleAction_->setData("dockRight");
    previewToggleAction_->setToolTip("显示或隐藏预览");
    bottomToggleAction_->setIcon(lineIcon("dockBottom"));
    bottomToggleAction_->setData("dockBottom");
    bottomToggleAction_->setToolTip("显示或隐藏底部面板");
    resetAction->setIcon(lineIcon("layoutReset"));
    resetAction->setData("layoutReset");
    resetAction->setToolTip("重置布局");
    autoLayoutAction->setIcon(lineIcon("layoutReset"));
    autoLayoutAction->setData("layoutReset");
    autoLayoutAction->setToolTip("按数据流自动布局当前图");
    fullScreenAction->setIcon(lineIcon("fullscreen"));
    fullScreenAction->setData("fullscreen");
    fullScreenAction->setToolTip("全屏切换");

    headerToolbar_ = addToolBar("窗口标题层");
    headerToolbar_->setObjectName("headerToolbar");
    headerToolbar_->setMovable(false);
    headerToolbar_->setFloatable(false);
    auto* header = new QWidget;
    header->setObjectName("windowHeader");
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* headerLayout = new QGridLayout(header);
    headerLayout->setContentsMargins(AppTheme::px(8, uiScale_), 0, AppTheme::px(8, uiScale_), 0);
    headerLayout->setHorizontalSpacing(AppTheme::px(8, uiScale_));
    headerLayout->setColumnStretch(0, 1);
    headerLayout->setColumnStretch(1, 1);
    headerLayout->setColumnStretch(2, 1);

    auto* leftMenus = new QWidget;
    auto* leftLayout = new QHBoxLayout(leftMenus);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
#if !defined(Q_OS_MACOS)
    auto addHeaderMenu = [leftLayout](QMenu* menu) {
        auto* button = new QToolButton;
        button->setText(menu->title());
        button->setMenu(menu);
        button->setPopupMode(QToolButton::InstantPopup);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        leftLayout->addWidget(button);
    };
    addHeaderMenu(fileMenu_);
    addHeaderMenu(editMenu_);
    addHeaderMenu(nodeOperationMenu_);
    addHeaderMenu(viewMenu_);
    addHeaderMenu(settingsMenu_);
    addHeaderMenu(layoutMenu_);
    menuBar()->setVisible(false);
#endif
    leftLayout->addStretch(1);
    headerLayout->addWidget(leftMenus, 0, 0, Qt::AlignLeft);

    auto* titleCluster = new QWidget;
    auto* titleLayout = new QHBoxLayout(titleCluster);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(AppTheme::px(4, uiScale_));
    documentTitleLabel_ = new QLabel;
    documentTitleLabel_->setObjectName("documentTitleLabel");
    documentTitleLabel_->setAlignment(Qt::AlignCenter);
    titleLayout->addWidget(documentTitleLabel_);
    auto* backButton = new QToolButton;
    backButton->setDefaultAction(returnToParentAction_);
    backButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    titleLayout->addWidget(backButton);
    auto* forwardButton = new QToolButton;
    forwardButton->setDefaultAction(forwardToChildAction_);
    forwardButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    titleLayout->addWidget(forwardButton);
    headerLayout->addWidget(titleCluster, 0, 1, Qt::AlignCenter);

    auto* rightActions = new QWidget;
    auto* rightLayout = new QHBoxLayout(rightActions);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    auto addHeaderAction = [rightLayout](QAction* action) {
        auto* button = new QToolButton;
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        rightLayout->addWidget(button);
    };
    addHeaderAction(previewToggleAction_);
    addHeaderAction(bottomToggleAction_);
    addHeaderAction(settingsAction_);
    headerLayout->addWidget(rightActions, 0, 2, Qt::AlignRight);
    headerToolbar_->addWidget(header);
    insertToolBar(mainToolbar_, headerToolbar_);
    insertToolBarBreak(mainToolbar_);
    installDelayedTooltips(headerToolbar_);

    addToolBarBreak(Qt::TopToolBarArea);
    workbookToolbar_ = addToolBar("工作簿");
    workbookToolbar_->setObjectName("workbookToolbar");
    workbookToolbar_->setMovable(false);
    workbookToolbar_->setFloatable(false);
    newWorkbookAction_ = new QAction(lineIcon("new"), "新建画布标签", this);
    newWorkbookAction_->setData("new");
    newWorkbookAction_->setToolTip("新建画布标签");
    connect(newWorkbookAction_, &QAction::triggered, this, [this] { addWorkbookPage(); });
    workbookToolbar_->addAction(newWorkbookAction_);
    workbookTabs_ = new QTabBar;
    workbookTabs_->setObjectName("workbookTabs");
    workbookTabs_->setDocumentMode(true);
    workbookTabs_->setExpanding(false);
    workbookTabs_->setMovable(true);
    workbookTabs_->setTabsClosable(true);
    workbookTabs_->setElideMode(Qt::ElideRight);
    workbookTabs_->setMinimumWidth(AppTheme::px(220, uiScale_));
    workbookTabs_->setMaximumWidth(AppTheme::px(520, uiScale_));
    connect(workbookTabs_, &QTabBar::currentChanged, this, [this](int index) { switchWorkbookPage(index); });
    connect(workbookTabs_, &QTabBar::tabCloseRequested, this, [this](int index) { closeWorkbookPage(index); });
    connect(workbookTabs_, &QTabBar::tabMoved, this, [this](int from, int to) {
        if (from < 0 || from >= workbookPages_.size() || to < 0 || to >= workbookPages_.size()) {
            return;
        }
        workbookPages_.move(from, to);
        if (currentWorkbookIndex_ == from) {
            currentWorkbookIndex_ = to;
        } else if (from < currentWorkbookIndex_ && to >= currentWorkbookIndex_) {
            --currentWorkbookIndex_;
        } else if (from > currentWorkbookIndex_ && to <= currentWorkbookIndex_) {
            ++currentWorkbookIndex_;
        }
        refreshWorkbookTabs();
    });
    workbookToolbar_->addWidget(workbookTabs_);
    auto* workbookSpacer = new QWidget;
    workbookSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    workbookToolbar_->addWidget(workbookSpacer);
    workbookToolbar_->addAction(runAction_);
    installDelayedTooltips(workbookToolbar_);
    updateNavigationActions();
    updateWindowTitle();

    QSettings settings;
    restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
    if (settings.value("mainWindow/layoutVersion", 0).toInt() >= 5) {
        restoreState(settings.value("mainWindow/state").toByteArray());
    } else {
        resetDockLayout();
    }
}

void MainWindow::rebuildNodeMenus()
{
    auto populateNodeMenu = [this](QMenu* root) {
        if (!root) {
            return;
        }
        QMap<QString, QMenu*> categoryMenus;
        for (const auto& descriptor : NodeFactory::instance().descriptors()) {
            QMenu* categoryMenu = categoryMenus.value(descriptor.category, nullptr);
            if (!categoryMenu) {
                categoryMenu = root->addMenu(descriptor.category);
                categoryMenus.insert(descriptor.category, categoryMenu);
            }
            auto* action = categoryMenu->addAction(descriptor.displayName);
            action->setToolTip(descriptor.typeName);
            connect(action, &QAction::triggered, this, [this, typeName = descriptor.typeName] {
                addNodeFromMenu(typeName);
            });
        }
    };

    if (nodeOperationMenu_) {
        nodeOperationMenu_->clear();
        populateNodeMenu(nodeOperationMenu_);
    }
    if (commandCenterMenu_) {
        commandCenterMenu_->clear();
        auto* fileMenu = commandCenterMenu_->addMenu("文件");
        fileMenu->addAction("新建", this, [this] { newWorkflow(); });
        fileMenu->addAction("打开", this, [this] { openWorkflow(); });
        fileMenu->addAction("保存", this, [this] { saveWorkflow(); });
        fileMenu->addAction("另存为", this, [this] { saveWorkflowAs(); });
        auto* runMenu = commandCenterMenu_->addMenu("执行");
        runMenu->addAction("执行 workflow", this, [this] { runWorkflow(); });
        runMenu->addAction("自动布局", this, [this] { autoLayoutWorkflow(); });
        auto* nodesMenu = commandCenterMenu_->addMenu("节点操作");
        populateNodeMenu(nodesMenu);
        auto* viewMenu = commandCenterMenu_->addMenu("视图");
        viewMenu->addAction("快捷添加节点", this, [this] { showQuickNodePalette(); });
        viewMenu->addAction("界面放大", this, [this] { increaseUiScale(); });
        viewMenu->addAction("界面缩小", this, [this] { decreaseUiScale(); });
        viewMenu->addAction("重置界面大小", this, [this] { resetUiScale(); });
        commandCenterMenu_->addSeparator();
        commandCenterMenu_->addAction("打开设置", this, [this] { showSettingsDialog(); });
    }
}

void MainWindow::addNodeFromMenu(const QString& typeName)
{
    QPointF targetPosition = view_ ? view_->mapToScene(view_->viewport()->rect().center()) : QPointF{};
    if (view_) {
        const QPoint cursorInView = view_->viewport()->mapFromGlobal(QCursor::pos());
        if (view_->viewport()->rect().contains(cursorInView)) {
            targetPosition = view_->mapToScene(cursorInView);
        }
    }
    addNodeFromType(typeName, targetPosition);
}

void MainWindow::addNodeFromType(const QString& typeName, const QPointF& position)
{
    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    auto created = NodeFactory::instance().create(typeName);
    if (created.isFail()) {
        appendLog(created.error());
        return;
    }
    const QString id = graph_.addNode(created.value(), findAvailableNodePosition(position));
    selectedNodeId_ = id;
    resetNodeRunStates();
    appendLog(QString("添加节点：%1").arg(id), id);
    rebuildScene();
    if (auto* item = nodeItems_.value(id)) {
        item->setSelected(true);
    }
    pushGraphEditCommand(QString("添加节点 %1").arg(id), before, selectedBefore,
                         WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
}

void MainWindow::selectNode(const QString& nodeId)
{
    selectedNodeId_ = nodeId;
    rebuildProperties();
}

void MainWindow::commitNodeMove(const QString& nodeId, const QPointF& beforePosition, const QPointF& afterPosition)
{
    if (!graph_.containsNode(nodeId)) {
        return;
    }
    if (qFuzzyCompare(beforePosition.x(), afterPosition.x()) && qFuzzyCompare(beforePosition.y(), afterPosition.y())) {
        return;
    }
    WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    before.setNodePosition(nodeId, beforePosition);
    WorkflowGraph after = WorkflowCommands::cloneGraph(graph_);
    after.setNodePosition(nodeId, afterPosition);
    pushGraphEditCommand(QString("移动节点 %1").arg(nodeId), before, nodeId, after, nodeId, QString("move:%1").arg(nodeId));
}

void MainWindow::showNodeContextMenu(const QString& nodeId, const QPoint& screenPos)
{
    selectedNodeId_ = nodeId;
    rebuildProperties();

    QMenu menu(this);
    QAction* copyAction = menu.addAction("复制执行节点");
    QAction* macroAction = menu.addAction("封装为宏节点");
    QAction* deleteAction = menu.addAction("删除执行节点");
    QAction* editAction = menu.addAction("修改参数");

    QAction* chosen = menu.exec(screenPos);
    if (chosen == copyAction) {
        copySelectedNode();
    } else if (chosen == macroAction) {
        encapsulateSelectionAsMacro();
    } else if (chosen == deleteAction) {
        removeSelectedItems();
    } else if (chosen == editAction) {
        focusParameterPanel();
    }
}

void MainWindow::showEdgeContextMenu(int edgeIndex, const QPoint& screenPos)
{
    QMenu menu(this);
    QAction* deleteAction = menu.addAction("删除连线");

    QAction* chosen = menu.exec(screenPos);
    if (chosen == deleteAction) {
        removeEdgeByIndex(edgeIndex);
    }
}

void MainWindow::copySelectedNode()
{
    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;

    QStringList sourceIds;
    for (auto* item : scene_->selectedItems()) {
        if (auto* node = dynamic_cast<NodeItem*>(item)) {
            sourceIds.append(node->nodeId());
        }
    }
    sourceIds.removeDuplicates();
    if (sourceIds.isEmpty() && !selectedNodeId_.isEmpty()) {
        sourceIds.append(selectedNodeId_);
    }
    if (sourceIds.isEmpty()) {
        appendLog("没有可复制的节点");
        return;
    }

    QStringList newIds;
    for (const QString& sourceId : sourceIds) {
        auto source = graph_.node(sourceId);
        const auto* sourceRecord = graph_.nodeRecord(sourceId);
        if (!source || !sourceRecord) {
            continue;
        }

        auto created = NodeFactory::instance().create(source->typeName());
        if (created.isFail()) {
            appendLog(created.error());
            return;
        }
        auto loaded = created.value()->loadParams(source->saveParams());
        if (loaded.isFail()) {
            appendLog(QString("复制节点参数失败：%1").arg(loaded.error()), sourceId);
            return;
        }

        const QPointF copyPosition = findAvailableNodePosition(sourceRecord->position + QPointF(36, 36));
        const QString newId = graph_.addNode(created.value(), copyPosition);
        newIds.append(newId);
        appendLog(QString("复制节点：%1 -> %2").arg(sourceRecord->id, newId), newId);
    }

    if (newIds.isEmpty()) {
        appendLog("没有可复制的节点");
        return;
    }

    selectedNodeId_ = newIds.last();
    resetNodeRunStates();
    rebuildScene();
    scene_->clearSelection();
    for (const QString& newId : newIds) {
        if (auto* item = nodeItems_.value(newId)) {
            item->setSelected(true);
        }
    }
    pushGraphEditCommand(newIds.size() > 1 ? QString("批量复制节点 %1 个").arg(newIds.size()) : QString("复制节点 %1").arg(newIds.first()),
                         before, selectedBefore,
                         WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
}

void MainWindow::encapsulateSelectionAsMacro()
{
    QStringList selectedIds;
    for (auto* item : scene_->selectedItems()) {
        if (auto* node = dynamic_cast<NodeItem*>(item)) {
            selectedIds.append(node->nodeId());
        }
    }
    selectedIds.removeDuplicates();
    if (selectedIds.isEmpty()) {
        appendLog("请先选择要封装的节点");
        return;
    }

    bool ok = false;
    const QString macroName = QInputDialog::getText(this, "封装为宏节点", "宏节点名称", QLineEdit::Normal, "宏节点", &ok).trimmed();
    if (!ok) {
        return;
    }

    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    QSet<QString> selectedSet;
    for (const QString& id : selectedIds) {
        selectedSet.insert(id);
    }
    WorkflowGraph subgraph;
    QPointF positionSum;
    int positionCount = 0;

    for (const auto& record : graph_.nodes()) {
        if (!selectedSet.contains(record.id)) {
            continue;
        }
        auto created = NodeFactory::instance().create(record.node->typeName());
        if (created.isFail()) {
            appendLog(created.error());
            return;
        }
        auto loaded = created.value()->loadParams(record.node->saveParams());
        if (loaded.isFail()) {
            appendLog(QString("封装节点参数失败：%1").arg(loaded.error()), record.id);
            return;
        }
        auto added = subgraph.addNodeWithId(record.id, created.value(), record.position);
        if (added.isFail()) {
            appendLog(added.error());
            return;
        }
        positionSum += record.position;
        ++positionCount;
    }

    QVector<Edge> incomingEdges;
    QVector<Edge> outgoingEdges;
    QMap<QString, QString> inputMacroPorts;
    QMap<QString, QString> outputMacroPorts;
    QVector<MacroPortMapping> inputMappings;
    QVector<MacroPortMapping> outputMappings;

    for (const auto& edge : graph_.edges()) {
        const bool fromSelected = selectedSet.contains(edge.fromNode);
        const bool toSelected = selectedSet.contains(edge.toNode);
        if (fromSelected && toSelected) {
            subgraph.addEdge(edge);
        } else if (!fromSelected && toSelected) {
            const QString key = QString("%1.%2").arg(edge.toNode, edge.toPort);
            if (!inputMacroPorts.contains(key)) {
                PortInfo port;
                findPortInfo(graph_.node(edge.toNode), edge.toPort, PortDirection::Input, &port);
                const QString macroPort = QString("in%1").arg(inputMappings.size() + 1);
                inputMacroPorts.insert(key, macroPort);
                inputMappings.append(MacroPortMapping{macroPort, port.displayName.isEmpty() ? macroPort : port.displayName,
                                                      edge.toNode, edge.toPort, port.type});
            }
            incomingEdges.append(edge);
        } else if (fromSelected && !toSelected) {
            const QString key = QString("%1.%2").arg(edge.fromNode, edge.fromPort);
            if (!outputMacroPorts.contains(key)) {
                PortInfo port;
                findPortInfo(graph_.node(edge.fromNode), edge.fromPort, PortDirection::Output, &port);
                const QString macroPort = QString("out%1").arg(outputMappings.size() + 1);
                outputMacroPorts.insert(key, macroPort);
                outputMappings.append(MacroPortMapping{macroPort, port.displayName.isEmpty() ? macroPort : port.displayName,
                                                       edge.fromNode, edge.fromPort, port.type});
            }
            outgoingEdges.append(edge);
        }
    }

    auto macro = QSharedPointer<MacroNode>(new MacroNode);
    macro->setDisplayName(macroName.isEmpty() ? "宏节点" : macroName);
    macro->setSubgraph(subgraph);
    macro->setInputMappings(inputMappings);
    macro->setOutputMappings(outputMappings);

    for (const QString& nodeId : selectedIds) {
        graph_.removeNode(nodeId);
    }

    const QPointF macroPosition = positionCount > 0 ? positionSum / qreal(positionCount) : QPointF{};
    const QString macroId = graph_.addNode(macro, findAvailableNodePosition(macroPosition));
    for (const auto& edge : incomingEdges) {
        const QString key = QString("%1.%2").arg(edge.toNode, edge.toPort);
        graph_.addEdge(Edge{edge.fromNode, edge.fromPort, macroId, inputMacroPorts.value(key)});
    }
    for (const auto& edge : outgoingEdges) {
        const QString key = QString("%1.%2").arg(edge.fromNode, edge.fromPort);
        graph_.addEdge(Edge{macroId, outputMacroPorts.value(key), edge.toNode, edge.toPort});
    }

    selectedNodeId_ = macroId;
    resetNodeRunStates();
    rebuildScene();
    if (auto* item = nodeItems_.value(macroId)) {
        scene_->clearSelection();
        item->setSelected(true);
    }
    appendLog(QString("封装宏节点：%1").arg(macroId), macroId);
    pushGraphEditCommand(QString("封装宏节点 %1").arg(macroId), before, selectedBefore,
                         WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
}

void MainWindow::autoLayoutWorkflow()
{
    if (graph_.nodes().isEmpty()) {
        appendLog("当前图没有可布局的节点");
        return;
    }

    QSet<QString> occupiedInputs;
    for (const auto& edge : graph_.edges()) {
        auto from = graph_.node(edge.fromNode);
        auto to = graph_.node(edge.toNode);
        if (!from || !to) {
            appendProblem("自动布局失败：连线引用了不存在的节点");
            QMessageBox::warning(this, "自动布局失败", "连线引用了不存在的节点，布局未修改。");
            return;
        }
        PortInfo fromPort;
        PortInfo toPort;
        if (!findPortInfo(from, edge.fromPort, PortDirection::Output, &fromPort) ||
            !findPortInfo(to, edge.toPort, PortDirection::Input, &toPort)) {
            appendProblem(QString("自动布局失败：连线端口不存在 %1.%2 -> %3.%4")
                              .arg(edge.fromNode, edge.fromPort, edge.toNode, edge.toPort));
            QMessageBox::warning(this, "自动布局失败",
                                 QString("连线端口不存在：%1.%2 -> %3.%4，布局未修改。")
                                     .arg(edge.fromNode, edge.fromPort, edge.toNode, edge.toPort));
            return;
        }
        if (!portTypesCompatible(fromPort.type, toPort.type)) {
            appendProblem(QString("自动布局失败：端口类型不兼容 %1.%2 -> %3.%4")
                              .arg(edge.fromNode, edge.fromPort, edge.toNode, edge.toPort));
            QMessageBox::warning(this, "自动布局失败",
                                 QString("端口类型不兼容：%1.%2 -> %3.%4，布局未修改。")
                                     .arg(edge.fromNode, edge.fromPort, edge.toNode, edge.toPort));
            return;
        }
        const QString inputKey = edge.toNode + "." + edge.toPort;
        if (!toPort.allowMultipleConnections && occupiedInputs.contains(inputKey)) {
            appendProblem(QString("自动布局失败：输入端口被多次连接 %1").arg(inputKey));
            QMessageBox::warning(this, "自动布局失败",
                                 QString("输入端口被多次连接：%1，布局未修改。").arg(inputKey));
            return;
        }
        occupiedInputs.insert(inputKey);
    }

    WorkflowValidator validator;
    auto orderResult = validator.topologicalOrder(graph_);
    if (orderResult.isFail()) {
        appendProblem(QString("自动布局失败：%1").arg(orderResult.error()));
        QMessageBox::warning(this, "自动布局失败", orderResult.error() + "，布局未修改。");
        return;
    }

    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    const QStringList order = orderResult.value();
    QMap<QString, int> layerByNode;
    for (const QString& id : graph_.nodeIds()) {
        layerByNode.insert(id, 0);
    }
    for (const QString& id : order) {
        const int baseLayer = layerByNode.value(id, 0);
        for (const auto& edge : graph_.edges()) {
            if (edge.fromNode == id) {
                layerByNode[edge.toNode] = std::max(layerByNode.value(edge.toNode, 0), baseLayer + 1);
            }
        }
    }

    QMap<int, QStringList> layers;
    for (const QString& id : order) {
        layers[layerByNode.value(id, 0)].append(id);
    }

    QPointF origin(0, 0);
    bool haveOrigin = false;
    for (const auto& record : graph_.nodes()) {
        if (!haveOrigin) {
            origin = record.position;
            haveOrigin = true;
        } else {
            origin.setX(std::min(origin.x(), record.position.x()));
            origin.setY(std::min(origin.y(), record.position.y()));
        }
    }

    const auto metrics = AppTheme::nodeMetrics(uiScale_);
    const qreal xSpacing = metrics.width + 110.0 * uiScale_;
    const qreal ySpacing = std::max<qreal>(metrics.rowHeight * 4.5, 150.0 * uiScale_);
    bool changed = false;
    for (auto layerIt = layers.cbegin(); layerIt != layers.cend(); ++layerIt) {
        const QStringList ids = layerIt.value();
        const qreal columnX = origin.x() + layerIt.key() * xSpacing;
        const qreal columnOffset = -0.5 * (ids.size() - 1) * ySpacing;
        for (int row = 0; row < ids.size(); ++row) {
            const QString& id = ids.at(row);
            const QPointF next(columnX, origin.y() + columnOffset + row * ySpacing);
            const auto* record = graph_.nodeRecord(id);
            if (record && (!qFuzzyCompare(record->position.x(), next.x()) || !qFuzzyCompare(record->position.y(), next.y()))) {
                changed = true;
            }
            graph_.setNodePosition(id, next);
        }
    }

    if (!changed) {
        appendLog("当前图已接近自动布局结果");
        return;
    }

    rebuildScene();
    if (auto* item = nodeItems_.value(selectedNodeId_)) {
        item->setSelected(true);
    }
    appendLog("已自动布局当前图");
    pushGraphEditCommand("自动布局", before, selectedBefore, WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
}

void MainWindow::enterMacroNode(const QString& nodeId)
{
    enterMacroNodeInternal(nodeId, true);
}

void MainWindow::enterMacroNodeInternal(const QString& nodeId, bool clearForwardHistory)
{
    auto macro = dynamic_cast<MacroNode*>(graph_.node(nodeId).data());
    if (!macro) {
        return;
    }
    if (clearForwardHistory) {
        forwardMacroHistory_.clear();
    }
    graphStack_.append(GraphContext{WorkflowCommands::cloneGraph(graph_), nodeId, selectedNodeId_});
    graph_ = WorkflowCommands::cloneGraph(macro->subgraph());
    selectedNodeId_.clear();
    lastResult_ = {};
    setPreviewImage({});
    engine_.clearCache();
    undoStack_->clear();
    resetNodeRunStates();
    rebuildScene();
    rebuildProperties();
    updateNavigationActions();
    updateWindowTitle();
    appendLog(QString("进入宏节点子图：%1").arg(nodeId), nodeId);
}

void MainWindow::leaveMacroNode()
{
    if (graphStack_.isEmpty()) {
        return;
    }
    GraphContext context = graphStack_.takeLast();
    forwardMacroHistory_.append(context.macroNodeId);
    auto macro = dynamic_cast<MacroNode*>(context.graph.node(context.macroNodeId).data());
    if (macro) {
        macro->setSubgraph(WorkflowCommands::cloneGraph(graph_));
    }
    graph_ = WorkflowCommands::cloneGraph(context.graph);
    selectedNodeId_ = context.macroNodeId;
    lastResult_ = {};
    setPreviewImage({});
    engine_.clearCache();
    undoStack_->clear();
    resetNodeRunStates();
    rebuildScene();
    if (auto* item = nodeItems_.value(selectedNodeId_)) {
        item->setSelected(true);
    }
    rebuildProperties();
    updateNavigationActions();
    updateWindowTitle();
    appendLog(QString("返回上级图：%1").arg(selectedNodeId_), selectedNodeId_);
}

void MainWindow::navigateForwardMacroNode()
{
    if (forwardMacroHistory_.isEmpty()) {
        return;
    }
    const QString macroNodeId = forwardMacroHistory_.takeLast();
    if (!graph_.containsNode(macroNodeId) || !dynamic_cast<MacroNode*>(graph_.node(macroNodeId).data())) {
        updateNavigationActions();
        appendLog(QString("前进失败：宏节点已不存在 %1").arg(macroNodeId));
        return;
    }
    enterMacroNodeInternal(macroNodeId, false);
}

WorkflowGraph MainWindow::graphForPersistence() const
{
    WorkflowGraph current = WorkflowCommands::cloneGraph(graph_);
    for (int i = graphStack_.size() - 1; i >= 0; --i) {
        WorkflowGraph parent = WorkflowCommands::cloneGraph(graphStack_.at(i).graph);
        auto macro = dynamic_cast<MacroNode*>(parent.node(graphStack_.at(i).macroNodeId).data());
        if (macro) {
            macro->setSubgraph(current);
        }
        current = parent;
    }
    return current;
}

void MainWindow::showQuickNodePalette()
{
    if (!view_) {
        return;
    }

    QPointF targetPosition = view_->mapToScene(view_->viewport()->rect().center());
    const QPoint cursorInView = view_->viewport()->mapFromGlobal(QCursor::pos());
    if (view_->viewport()->rect().contains(cursorInView)) {
        targetPosition = view_->mapToScene(cursorInView);
    }
    showQuickNodePaletteAt(targetPosition);
}

void MainWindow::showQuickNodePaletteAt(const QPointF& scenePosition)
{
    if (!view_) {
        return;
    }

    QuickNodePalettePopup popup(uiScale_, this);
    popup.adjustSize();
    QPoint viewportPosition = view_->mapFromScene(scenePosition);
    if (!view_->viewport()->rect().contains(viewportPosition)) {
        viewportPosition = view_->viewport()->rect().center();
    }
    QPoint popupPosition = view_->viewport()->mapToGlobal(viewportPosition + QPoint(AppTheme::px(12, uiScale_), AppTheme::px(12, uiScale_)));
    if (auto* screen = QGuiApplication::screenAt(popupPosition)) {
        const QRect available = screen->availableGeometry().adjusted(8, 8, -8, -8);
        const QSize size = popup.sizeHint();
        popupPosition.setX(std::clamp(popupPosition.x(), available.left(), available.right() - size.width()));
        popupPosition.setY(std::clamp(popupPosition.y(), available.top(), available.bottom() - size.height()));
    }
    popup.move(popupPosition);

    if (popup.exec() == QDialog::Accepted && !popup.selectedType().isEmpty()) {
        addNodeFromType(popup.selectedType(), scenePosition);
    }
    view_->setFocus(Qt::ShortcutFocusReason);
}

void MainWindow::focusParameterPanel()
{
    if (selectedNodeId_.isEmpty()) {
        return;
    }
    if (auto* item = nodeItems_.value(selectedNodeId_)) {
        item->setFocus();
        view_->centerOn(item);
    }
    appendLog(QString("修改参数：%1").arg(selectedNodeId_), selectedNodeId_);
}

void MainWindow::zoomIn()
{
    applyZoomFactor(1.10);
}

void MainWindow::zoomOut()
{
    applyZoomFactor(1.0 / 1.10);
}

void MainWindow::updateMiniMap()
{
    if (miniMap_) {
        miniMap_->update();
    }
}

void MainWindow::applyZoomFactor(double factor)
{
    const double nextScale = std::clamp(zoomScale_ * factor, 0.25, 3.0);
    const double actualFactor = nextScale / zoomScale_;
    if (qFuzzyCompare(actualFactor, 1.0)) {
        return;
    }
    zoomScale_ = nextScale;
    view_->scale(actualFactor, actualFactor);
    updateMiniMap();
}

void MainWindow::requestConnect(const QString& fromNode, const QString& fromPort, const QString& toNode, const QString& toPort)
{
    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    Edge edge{fromNode, fromPort, toNode, toPort};
    WorkflowValidator validator;
    auto valid = validator.validateEdge(graph_, edge);
    if (valid.isFail()) {
        appendLog(valid.error());
        appendProblem(valid.error(), toNode);
        return;
    }
    graph_.addEdge(edge);
    resetNodeRunStates();
    appendLog(QString("连接：%1.%2 -> %3.%4").arg(fromNode, fromPort, toNode, toPort), toNode);
    rebuildEdges();
    pushGraphEditCommand("添加连线", before, selectedBefore, WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
}

void MainWindow::updateNodePosition(const QString& nodeId, const QPointF& position)
{
    graph_.setNodePosition(nodeId, position);
    rebuildEdges();
    updateMiniMap();
}

void MainWindow::rebuildScene()
{
    scene_->clear();
    nodeItems_.clear();
    edgeItems_.clear();
    for (const auto& record : graph_.nodes()) {
        auto* item = new NodeItem(this, record.id, record.node, uiScale_,
                                  nodeRunStates_.value(record.id, NodeExecutionState::NotExecuted));
        item->setElapsedMs(nodeElapsedMs_.value(record.id, -1));
        item->setPos(record.position);
        scene_->addItem(item);
        nodeItems_.insert(record.id, item);
    }
    rebuildEdges();
    updateMiniMap();
}

void MainWindow::rebuildEdges()
{
    for (auto* item : edgeItems_) {
        scene_->removeItem(item);
        delete item;
    }
    edgeItems_.clear();

    auto portPosition = [&](const QString& nodeId, const QString& portName, PortDirection direction) -> QPointF {
        auto* item = dynamic_cast<NodeItem*>(nodeItems_.value(nodeId));
        if (!item) return {};
        for (auto* port : item->ports()) {
            if (port->port().name == portName && port->port().direction == direction) {
                return port->mapToScene(port->rect().center());
            }
        }
        return item->scenePos();
    };

    QSet<QString> failedChainNodes;
    if (!lastResult_.failedNodeId.isEmpty()) {
        failedChainNodes.insert(lastResult_.failedNodeId);
        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& edge : graph_.edges()) {
                if (failedChainNodes.contains(edge.toNode) && !failedChainNodes.contains(edge.fromNode)) {
                    failedChainNodes.insert(edge.fromNode);
                    changed = true;
                }
            }
        }
    }

    for (int i = 0; i < graph_.edges().size(); ++i) {
        const auto& edge = graph_.edges().at(i);
        const QPointF a = portPosition(edge.fromNode, edge.fromPort, PortDirection::Output);
        const QPointF b = portPosition(edge.toNode, edge.toPort, PortDirection::Input);
        auto* line = new EdgeItem(i, a, b, uiScale_);
        line->setFailureRelated(failedChainNodes.contains(edge.fromNode) && failedChainNodes.contains(edge.toNode));
        scene_->addItem(line);
        line->setZValue(-1);
        edgeItems_.append(line);
    }
}

void MainWindow::removeSelectedItems()
{
    QVector<int> edgeIndexes;
    QStringList nodeIds;
    for (auto* item : scene_->selectedItems()) {
        if (auto* node = dynamic_cast<NodeItem*>(item)) {
            nodeIds.append(node->nodeId());
        } else if (auto* edge = dynamic_cast<EdgeItem*>(item)) {
            edgeIndexes.append(edge->edgeIndex());
        }
    }
    if (edgeIndexes.isEmpty() && nodeIds.isEmpty()) {
        return;
    }

    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;

    std::sort(edgeIndexes.begin(), edgeIndexes.end(), std::greater<int>());
    edgeIndexes.erase(std::unique(edgeIndexes.begin(), edgeIndexes.end()), edgeIndexes.end());
    for (int edgeIndex : edgeIndexes) {
        if (edgeIndex >= 0 && edgeIndex < graph_.edges().size()) {
            graph_.removeEdge(edgeIndex);
            appendLog(QString("删除连线：%1").arg(edgeIndex));
        }
    }

    nodeIds.removeDuplicates();
    for (const QString& nodeId : nodeIds) {
        graph_.removeNode(nodeId);
        appendLog(QString("删除节点：%1").arg(nodeId), nodeId);
    }

    selectedNodeId_.clear();
    resetNodeRunStates();
    rebuildScene();
    rebuildProperties();
    pushGraphEditCommand("删除选中项", before, selectedBefore, WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
}

void MainWindow::rebuildProperties()
{
    // Parameter editors now live inside the selected node. Keep this as the
    // selection-change synchronization point for older call sites.
}

void MainWindow::appendLog(const QString& message, const QString& nodeId)
{
    if (!log_) {
        return;
    }
    auto* item = new QListWidgetItem(message);
    item->setData(Qt::UserRole, nodeId);
    if (!nodeId.isEmpty()) {
        item->setToolTip(QString("双击定位节点：%1").arg(nodeId));
        item->setForeground(QBrush(AppTheme::colors().nodeSelected));
    }
    log_->addItem(item);
    log_->scrollToBottom();
}

void MainWindow::appendProblem(const QString& message, const QString& nodeId)
{
    if (!problemLog_) {
        return;
    }
    auto* item = new QListWidgetItem(message);
    item->setData(Qt::UserRole, nodeId);
    item->setForeground(QBrush(QColor("#ff453a")));
    if (!nodeId.isEmpty()) {
        item->setToolTip(QString("双击定位节点：%1").arg(nodeId));
    }
    problemLog_->addItem(item);
    problemLog_->scrollToBottom();
    if (bottomTabs_) {
        bottomTabs_->setCurrentIndex(1);
    }
}

void MainWindow::clearLog()
{
    if (log_) {
        log_->clear();
    }
}

void MainWindow::setSelectedNodeParameter(const QString& name, const QVariant& value)
{
    setNodeParameterForNode(selectedNodeId_, name, value);
}

void MainWindow::setNodeParameterForNode(const QString& nodeId, const QString& name, const QVariant& value)
{
    auto node = graph_.node(nodeId);
    if (!node) {
        return;
    }
    const QVariant oldValue = node->parameterValue(name);
    if (oldValue == value) {
        return;
    }
    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    auto status = node->setParameter(name, value);
    if (status.isFail()) {
        appendLog(QString("参数修改失败：%1").arg(status.error()));
        return;
    }
    resetNodeRunStates();
    pushGraphEditCommand(QString("修改参数 %1").arg(name), before, selectedBefore,
                         WorkflowCommands::cloneGraph(graph_), selectedNodeId_,
                         QString("param:%1:%2").arg(nodeId, name));
    scheduleLivePreview();
}

void MainWindow::resetNodeRunStates()
{
    if (runAnimationTimer_) {
        runAnimationTimer_->stop();
    }
    runAnimationPhase_ = 0;
    nodeRunStates_.clear();
    nodeElapsedMs_.clear();
    for (const auto& record : graph_.nodes()) {
        nodeRunStates_.insert(record.id, NodeExecutionState::NotExecuted);
    }
    for (auto it = nodeItems_.begin(); it != nodeItems_.end(); ++it) {
        if (auto* nodeItem = dynamic_cast<NodeItem*>(it.value())) {
            nodeItem->setRunState(NodeExecutionState::NotExecuted);
            nodeItem->setElapsedMs(-1);
            nodeItem->setAnimationPhase(0);
        }
    }
}

void MainWindow::applyNodeRunState(const QString& nodeId, NodeExecutionState state)
{
    if (nodeId.isEmpty()) {
        return;
    }
    nodeRunStates_.insert(nodeId, state);
    if (auto* nodeItem = dynamic_cast<NodeItem*>(nodeItems_.value(nodeId))) {
        nodeItem->setRunState(state);
    }
}

void MainWindow::handleNodeExecutionEvent(const NodeExecutionSummary& summary)
{
    applyNodeRunState(summary.nodeId, summary.state);
    if (!summary.nodeId.isEmpty() && summary.elapsedMs >= 0) {
        nodeElapsedMs_.insert(summary.nodeId, summary.elapsedMs);
        if (auto* nodeItem = dynamic_cast<NodeItem*>(nodeItems_.value(summary.nodeId))) {
            nodeItem->setElapsedMs(summary.elapsedMs);
        }
    }
    if (summary.state == NodeExecutionState::Running && runAnimationTimer_) {
        runAnimationTimer_->start();
    } else if (runAnimationTimer_) {
        bool hasRunning = false;
        for (auto it = nodeRunStates_.cbegin(); it != nodeRunStates_.cend(); ++it) {
            const auto state = it.value();
            if (state == NodeExecutionState::Running) {
                hasRunning = true;
                break;
            }
        }
        if (!hasRunning) {
            runAnimationTimer_->stop();
        }
    }
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void MainWindow::updateRunAnimation()
{
    ++runAnimationPhase_;
    bool hasRunning = false;
    for (auto it = nodeItems_.begin(); it != nodeItems_.end(); ++it) {
        if (nodeRunStates_.value(it.key()) == NodeExecutionState::Running) {
            hasRunning = true;
            if (auto* nodeItem = dynamic_cast<NodeItem*>(it.value())) {
                nodeItem->setAnimationPhase(runAnimationPhase_);
            }
        }
    }
    if (!hasRunning && runAnimationTimer_) {
        runAnimationTimer_->stop();
    }
}

void MainWindow::focusFailedNode(const QString& nodeId)
{
    if (nodeId.isEmpty()) {
        return;
    }
    auto* item = nodeItems_.value(nodeId);
    if (!item) {
        return;
    }
    scene_->clearSelection();
    item->setSelected(true);
    selectedNodeId_ = nodeId;
    view_->centerOn(item);
    rebuildProperties();
}

void MainWindow::focusLogNode(QListWidgetItem* item)
{
    if (!item) {
        return;
    }
    const QString nodeId = item->data(Qt::UserRole).toString();
    if (nodeId.isEmpty() || !nodeItems_.contains(nodeId)) {
        return;
    }
    scene_->clearSelection();
    auto* nodeItem = nodeItems_.value(nodeId);
    nodeItem->setSelected(true);
    selectedNodeId_ = nodeId;
    view_->centerOn(nodeItem);
    rebuildProperties();
    updatePreviewForSelection();
    highlightNodeBriefly(nodeId);
}

void MainWindow::highlightNodeBriefly(const QString& nodeId)
{
    auto* item = nodeItems_.value(nodeId);
    if (!item || !scene_) {
        return;
    }

    auto* highlight = new QGraphicsRectItem(item->sceneBoundingRect().adjusted(-8 * uiScale_, -8 * uiScale_, 8 * uiScale_, 8 * uiScale_));
    QPen pen(AppTheme::isDarkTheme() ? QColor("#f2f2f2") : QColor("#202020"), 3.0 * uiScale_, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
    highlight->setPen(pen);
    highlight->setBrush(Qt::NoBrush);
    highlight->setZValue(10);
    scene_->addItem(highlight);
    QTimer::singleShot(650, scene_, [this, highlight] {
        if (scene_ && highlight->scene() == scene_) {
            scene_->removeItem(highlight);
        }
        delete highlight;
    });
}

void MainWindow::runWorkflow()
{
    if (livePreviewTimer_) {
        livePreviewTimer_->stop();
    }
    resetNodeRunStates();
    auto result = engine_.execute(graph_, [this](const NodeExecutionSummary& summary) {
        handleNodeExecutionEvent(summary);
    });
    if (result.isFail()) {
        lastResult_ = engine_.lastResult();
        rebuildEdges();
        for (const auto& summary : lastResult_.nodeSummaries) {
            appendLog(summary.message, summary.nodeId);
        }
        appendLog(QString("执行失败：%1").arg(result.error()), lastResult_.failedNodeId);
        appendProblem(QString("执行失败：%1").arg(result.error()), lastResult_.failedNodeId);
        focusFailedNode(lastResult_.failedNodeId);
        QMessageBox::warning(this, "执行失败", result.error());
        return;
    }
    lastResult_ = result.value();
    rebuildEdges();
    for (const auto& summary : lastResult_.nodeSummaries) {
        appendLog(summary.message, summary.nodeId);
    }
    appendLog("执行完成");
    updatePreviewForSelection();
}

void MainWindow::scheduleLivePreview()
{
    if (!livePreviewTimer_ || selectedNodeId_.isEmpty()) {
        return;
    }
    livePreviewTimer_->start();
}

void MainWindow::runLivePreview()
{
    if (selectedNodeId_.isEmpty() || !graph_.containsNode(selectedNodeId_)) {
        return;
    }

    const QString previewNodeId = selectedNodeId_;
    resetNodeRunStates();
    auto result = engine_.executeForNode(graph_, previewNodeId, [this](const NodeExecutionSummary& summary) {
        handleNodeExecutionEvent(summary);
    });
    if (result.isFail()) {
        lastResult_ = engine_.lastResult();
        rebuildEdges();
        for (const auto& summary : lastResult_.nodeSummaries) {
            appendLog(summary.message, summary.nodeId);
        }
        const QString failedNode = lastResult_.failedNodeId.isEmpty() ? previewNodeId : lastResult_.failedNodeId;
        appendLog(QString("实时预览失败：%1").arg(result.error()), failedNode);
        appendProblem(QString("实时预览失败：%1").arg(result.error()), failedNode);
        updatePreviewForSelection();
        return;
    }

    lastResult_ = result.value();
    rebuildEdges();
    for (const auto& summary : lastResult_.nodeSummaries) {
        appendLog(summary.message, summary.nodeId);
    }
    appendLog(QString("实时预览完成：%1").arg(previewNodeId), previewNodeId);
    updatePreviewForSelection();
}

void MainWindow::exportWorkflowCopy()
{
    const QString defaultName = currentFile_.isEmpty()
                                    ? "workflow-export.json"
                                    : QString("%1-export.json").arg(QFileInfo(currentFile_).completeBaseName());
    const QString path = QFileDialog::getSaveFileName(this, "导出工作流", defaultName, "Workflow (*.json);;All files (*)");
    if (path.isEmpty()) {
        return;
    }
    WorkflowSerializer serializer;
    auto saved = serializer.saveFile(graphForPersistence(), path);
    if (saved.isFail()) {
        appendProblem(QString("导出工作流失败：%1").arg(saved.error()));
        QMessageBox::warning(this, "导出工作流失败", saved.error());
        return;
    }
    appendLog(QString("已导出工作流副本：%1").arg(path));
}

void MainWindow::exportPreviewImage()
{
    if (currentPreviewImage_.isNull()) {
        appendProblem("导出预览结果失败：当前没有可导出的预览图");
        QMessageBox::information(this, "导出预览结果", "当前没有可导出的预览图。");
        return;
    }
    const QString path = QFileDialog::getSaveFileName(this, "导出预览结果", "preview-result.png", "PNG (*.png);;All files (*)");
    if (path.isEmpty()) {
        return;
    }
    if (!currentPreviewImage_.save(path, "PNG")) {
        appendProblem(QString("导出预览结果失败：无法写入图片 %1").arg(path));
        QMessageBox::warning(this, "导出预览结果失败", QString("无法写入图片：%1").arg(path));
        return;
    }
    appendLog(QString("已导出预览结果：%1").arg(path));
}

void MainWindow::exportCanvasImage()
{
    if (!scene_) {
        return;
    }

    QRectF bounds = scene_->itemsBoundingRect();
    if (bounds.isEmpty()) {
        bounds = view_->mapToScene(view_->viewport()->rect()).boundingRect();
    }
    bounds = bounds.adjusted(-80 * uiScale_, -80 * uiScale_, 80 * uiScale_, 80 * uiScale_);

    const QString path = QFileDialog::getSaveFileName(this, "导出画布截图", "workflow-canvas.png", "PNG (*.png);;All files (*)");
    if (path.isEmpty()) {
        return;
    }

    const auto selectedItems = scene_->selectedItems();
    scene_->clearSelection();

    constexpr qreal exportScale = 2.0;
    QImage image((bounds.size() * exportScale).toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(AppTheme::colors().canvasTop);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.scale(exportScale, exportScale);

    const auto colors = AppTheme::colors();
    QLinearGradient gradient(QPointF(0, 0), QPointF(bounds.width(), bounds.height()));
    gradient.setColorAt(0, colors.canvasTop);
    gradient.setColorAt(1, colors.canvasBottom);
    painter.fillRect(QRectF(QPointF(0, 0), bounds.size()), gradient);
    scene_->render(&painter, QRectF(QPointF(0, 0), bounds.size()), bounds);
    painter.end();

    for (auto* item : selectedItems) {
        if (item) {
            item->setSelected(true);
        }
    }

    if (!image.save(path, "PNG")) {
        appendProblem(QString("导出失败：无法写入图片 %1").arg(path));
        QMessageBox::warning(this, "导出失败", QString("无法写入图片：%1").arg(path));
        return;
    }
    appendLog(QString("已导出画布截图：%1").arg(path));
}

void MainWindow::newWorkflow()
{
    if (!confirmSaveIfNeeded()) {
        return;
    }
    if (livePreviewTimer_) {
        livePreviewTimer_->stop();
    }
    graph_.clear();
    graphStack_.clear();
    forwardMacroHistory_.clear();
    currentFile_.clear();
    engine_.clearCache();
    lastResult_ = {};
    setPreviewImage({});
    nodeRunStates_.clear();
    nodeElapsedMs_.clear();
    selectedNodeId_.clear();
    updateNavigationActions();
    undoStack_->clear();
    rebuildScene();
    rebuildProperties();
    if (currentWorkbookIndex_ >= 0 && currentWorkbookIndex_ < workbookPages_.size()) {
        workbookPages_[currentWorkbookIndex_].graph = WorkflowCommands::cloneGraph(graph_);
        workbookPages_[currentWorkbookIndex_].selectedNodeId.clear();
        workbookPages_[currentWorkbookIndex_].filePath.clear();
        workbookPages_[currentWorkbookIndex_].title = QString("画布 %1").arg(currentWorkbookIndex_ + 1);
        workbookPages_[currentWorkbookIndex_].dirty = false;
        refreshWorkbookTabs();
    }
    updateWindowTitle();
    appendLog("新建 workflow");
}

void MainWindow::openWorkflow()
{
    if (!confirmSaveIfNeeded()) {
        return;
    }
    if (livePreviewTimer_) {
        livePreviewTimer_->stop();
    }
    const QString path = QFileDialog::getOpenFileName(this, "打开 workflow", {}, "Workflow (*.json);;All files (*)");
    if (path.isEmpty()) return;
    WorkflowSerializer serializer;
    auto loaded = serializer.loadFile(path);
    if (loaded.isFail()) {
        appendProblem(QString("打开失败：%1").arg(loaded.error()));
        QMessageBox::warning(this, "打开失败", loaded.error());
        return;
    }
    graph_ = loaded.value();
    graphStack_.clear();
    forwardMacroHistory_.clear();
    currentFile_ = path;
    engine_.clearCache();
    lastResult_ = {};
    setPreviewImage({});
    nodeRunStates_.clear();
    nodeElapsedMs_.clear();
    selectedNodeId_.clear();
    updateNavigationActions();
    undoStack_->clear();
    rebuildScene();
    rebuildProperties();
    if (currentWorkbookIndex_ >= 0 && currentWorkbookIndex_ < workbookPages_.size()) {
        workbookPages_[currentWorkbookIndex_].graph = WorkflowCommands::cloneGraph(graph_);
        workbookPages_[currentWorkbookIndex_].selectedNodeId.clear();
        workbookPages_[currentWorkbookIndex_].filePath = currentFile_;
        workbookPages_[currentWorkbookIndex_].title = QFileInfo(currentFile_).fileName();
        workbookPages_[currentWorkbookIndex_].dirty = false;
        refreshWorkbookTabs();
    }
    updateWindowTitle();
    appendLog(QString("已打开：%1").arg(path));
}

bool MainWindow::confirmSaveIfNeeded()
{
    if (!currentWorkbookDirty()) {
        return true;
    }
    const auto choice = QMessageBox::warning(this, "保存修改",
                                             "当前 workflow 有未保存修改，是否先保存？",
                                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                             QMessageBox::Save);
    if (choice == QMessageBox::Cancel) {
        return false;
    }
    if (choice == QMessageBox::Discard) {
        return true;
    }
    saveWorkflow();
    return undoStack_->isClean();
}

bool MainWindow::ensureSavePath()
{
    if (!currentFile_.isEmpty()) return true;
    currentFile_ = QFileDialog::getSaveFileName(this, "保存 workflow", "workflow.json", "Workflow (*.json);;All files (*)");
    return !currentFile_.isEmpty();
}

void MainWindow::removeEdgeByIndex(int edgeIndex)
{
    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    auto removed = graph_.removeEdge(edgeIndex);
    if (removed.isFail()) {
        appendLog(removed.error());
        return;
    }
    appendLog(QString("删除连线：%1").arg(edgeIndex));
    resetNodeRunStates();
    rebuildEdges();
    pushGraphEditCommand("删除连线", before, selectedBefore, WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
}

QPointF MainWindow::findAvailableNodePosition(const QPointF& requested) const
{
    const auto metrics = AppTheme::nodeMetrics(uiScale_);
    const qreal kNodeWidth = metrics.width + 18.0 * uiScale_;
    const qreal kNodeHeight = metrics.topPadding + 4 * metrics.rowHeight + metrics.bottomPadding + 18.0 * uiScale_;
    constexpr qreal kStep = 34.0;
    QPointF candidate = requested;

    for (int attempt = 0; attempt < 80; ++attempt) {
        QRectF candidateRect(candidate, QSizeF(kNodeWidth, kNodeHeight));
        bool overlaps = false;
        for (const auto& record : graph_.nodes()) {
            QRectF existingRect(record.position, QSizeF(kNodeWidth, kNodeHeight));
            if (candidateRect.intersects(existingRect)) {
                overlaps = true;
                break;
            }
        }
        if (!overlaps) {
            return candidate;
        }
        candidate = requested + QPointF(kStep * (attempt + 1), kStep * (attempt + 1));
    }

    return candidate;
}

bool MainWindow::saveWorkflow()
{
    if (!ensureSavePath()) return false;
    WorkflowSerializer serializer;
    const WorkflowGraph graphToSave = graphForPersistence();
    auto saved = serializer.saveFile(graphToSave, currentFile_);
    if (saved.isFail()) {
        appendProblem(QString("保存失败：%1").arg(saved.error()));
        QMessageBox::warning(this, "保存失败", saved.error());
        return false;
    }
    if (undoStack_) {
        undoStack_->setClean();
    }
    if (currentWorkbookIndex_ >= 0 && currentWorkbookIndex_ < workbookPages_.size()) {
        workbookPages_[currentWorkbookIndex_].graph = graphToSave;
        workbookPages_[currentWorkbookIndex_].selectedNodeId = selectedNodeId_;
        workbookPages_[currentWorkbookIndex_].filePath = currentFile_;
        workbookPages_[currentWorkbookIndex_].title = QFileInfo(currentFile_).fileName();
        workbookPages_[currentWorkbookIndex_].dirty = false;
        refreshWorkbookTabs();
    }
    updateWindowTitle();
    appendLog(QString("已保存：%1").arg(currentFile_));
    return true;
}

void MainWindow::saveWorkflowAs()
{
    const QString previousFile = currentFile_;
    currentFile_.clear();
    if (!saveWorkflow()) {
        currentFile_ = previousFile;
        updateWindowTitle();
    }
}

void MainWindow::restoreGraphFromUndo(const WorkflowGraph& graph, const QString& selectedNodeId)
{
    graph_ = WorkflowCommands::cloneGraph(graph);
    selectedNodeId_ = selectedNodeId;
    lastResult_ = {};
    resetNodeRunStates();
    rebuildScene();
    if (auto* item = nodeItems_.value(selectedNodeId_)) {
        item->setSelected(true);
    }
    rebuildProperties();
    updatePreviewForSelection();
}

void MainWindow::pushGraphEditCommand(const QString& text,
                                      const WorkflowGraph& before,
                                      const QString& selectedBefore,
                                      const WorkflowGraph& after,
                                      const QString& selectedAfter,
                                      const QString& mergeKey)
{
    if (!undoStack_) {
        return;
    }
    if (!forwardMacroHistory_.isEmpty()) {
        forwardMacroHistory_.clear();
        updateNavigationActions();
    }
    undoStack_->push(WorkflowCommands::makeSnapshotCommand(this, text, before, after, selectedBefore, selectedAfter, mergeKey));
}

void MainWindow::initializeWorkbook()
{
    if (!workbookTabs_ || !workbookPages_.isEmpty()) {
        return;
    }
    WorkbookPage page;
    page.graph = WorkflowCommands::cloneGraph(graph_);
    page.selectedNodeId = selectedNodeId_;
    page.title = "画布 1";
    workbookPages_.append(page);
    currentWorkbookIndex_ = 0;
    refreshWorkbookTabs();
}

bool MainWindow::currentWorkbookDirty() const
{
    if (undoStack_ && !undoStack_->isClean()) {
        return true;
    }
    if (currentWorkbookIndex_ >= 0 && currentWorkbookIndex_ < workbookPages_.size()) {
        return workbookPages_.at(currentWorkbookIndex_).dirty;
    }
    return false;
}

void MainWindow::syncCurrentWorkbookPage()
{
    if (currentWorkbookIndex_ < 0 || currentWorkbookIndex_ >= workbookPages_.size()) {
        return;
    }
    auto& page = workbookPages_[currentWorkbookIndex_];
    page.graph = graphForPersistence();
    page.selectedNodeId = selectedNodeId_;
    page.filePath = currentFile_;
    page.dirty = currentWorkbookDirty();
    if (!currentFile_.isEmpty()) {
        page.title = QFileInfo(currentFile_).fileName();
    } else if (page.title.trimmed().isEmpty()) {
        page.title = QString("画布 %1").arg(currentWorkbookIndex_ + 1);
    }
}

void MainWindow::switchWorkbookPage(int index)
{
    if (switchingWorkbook_ || index < 0 || index >= workbookPages_.size() || index == currentWorkbookIndex_) {
        return;
    }
    syncCurrentWorkbookPage();
    switchingWorkbook_ = true;
    currentWorkbookIndex_ = index;
    const auto page = workbookPages_.at(index);
    graph_ = WorkflowCommands::cloneGraph(page.graph);
    selectedNodeId_ = page.selectedNodeId;
    currentFile_ = page.filePath;
    graphStack_.clear();
    forwardMacroHistory_.clear();
    lastResult_ = {};
    engine_.clearCache();
    nodeRunStates_.clear();
    nodeElapsedMs_.clear();
    updateNavigationActions();
    if (undoStack_) {
        undoStack_->clear();
    }
    resetNodeRunStates();
    rebuildScene();
    if (auto* item = nodeItems_.value(selectedNodeId_)) {
        item->setSelected(true);
    }
    rebuildProperties();
    updatePreviewForSelection();
    refreshWorkbookTabs();
    updateWindowTitle();
    switchingWorkbook_ = false;
}

void MainWindow::addWorkbookPage()
{
    syncCurrentWorkbookPage();
    WorkbookPage page;
    page.title = QString("画布 %1").arg(workbookPages_.size() + 1);
    workbookPages_.append(page);
    refreshWorkbookTabs();
    if (workbookTabs_) {
        workbookTabs_->setCurrentIndex(workbookPages_.size() - 1);
    }
}

void MainWindow::closeWorkbookPage(int index)
{
    if (index < 0 || index >= workbookPages_.size()) {
        return;
    }
    if (index == currentWorkbookIndex_ && !confirmSaveIfNeeded()) {
        return;
    }

    if (workbookPages_.size() == 1) {
        graph_.clear();
        graphStack_.clear();
        forwardMacroHistory_.clear();
        currentFile_.clear();
        selectedNodeId_.clear();
        lastResult_ = {};
        setPreviewImage({});
        engine_.clearCache();
        nodeRunStates_.clear();
        nodeElapsedMs_.clear();
        updateNavigationActions();
        workbookPages_[0] = WorkbookPage{};
        workbookPages_[0].title = "画布 1";
        currentWorkbookIndex_ = 0;
        if (undoStack_) {
            undoStack_->clear();
        }
        rebuildScene();
        rebuildProperties();
        refreshWorkbookTabs();
        updateWindowTitle();
        return;
    }

    workbookPages_.removeAt(index);
    if (currentWorkbookIndex_ > index) {
        --currentWorkbookIndex_;
    } else if (currentWorkbookIndex_ == index) {
        currentWorkbookIndex_ = std::min(index, int(workbookPages_.size()) - 1);
        const auto page = workbookPages_.at(currentWorkbookIndex_);
        graph_ = WorkflowCommands::cloneGraph(page.graph);
        selectedNodeId_ = page.selectedNodeId;
        currentFile_ = page.filePath;
        graphStack_.clear();
        forwardMacroHistory_.clear();
        lastResult_ = {};
        engine_.clearCache();
        nodeRunStates_.clear();
        nodeElapsedMs_.clear();
        updateNavigationActions();
        if (undoStack_) {
            undoStack_->clear();
        }
        rebuildScene();
        if (auto* item = nodeItems_.value(selectedNodeId_)) {
            item->setSelected(true);
        }
        rebuildProperties();
        updatePreviewForSelection();
    }
    refreshWorkbookTabs();
    updateWindowTitle();
}

void MainWindow::refreshWorkbookTabs()
{
    if (!workbookTabs_) {
        return;
    }
    QSignalBlocker blocker(workbookTabs_);
    while (workbookTabs_->count() > 0) {
        workbookTabs_->removeTab(0);
    }
    for (int i = 0; i < workbookPages_.size(); ++i) {
        const auto& page = workbookPages_.at(i);
        QString title = page.title.trimmed().isEmpty() ? QString("画布 %1").arg(i + 1) : page.title;
        if (i == currentWorkbookIndex_ && currentWorkbookDirty()) {
            title.prepend("*");
        } else if (page.dirty) {
            title.prepend("*");
        }
        workbookTabs_->addTab(title);
        workbookTabs_->setTabToolTip(i, page.filePath.isEmpty() ? title : page.filePath);
    }
    if (currentWorkbookIndex_ >= 0 && currentWorkbookIndex_ < workbookTabs_->count()) {
        workbookTabs_->setCurrentIndex(currentWorkbookIndex_);
    }
}

void MainWindow::updateWindowTitle()
{
    const QString fileName = currentFile_.isEmpty() ? "未命名" : QFileInfo(currentFile_).fileName();
    const QString dirty = currentWorkbookDirty() ? "*" : "";
    const QString title = QString("%1%2").arg(fileName, dirty);
    setWindowTitle(QString("%1 - ImageNodeEditor").arg(title));
    if (documentTitleLabel_) {
        documentTitleLabel_->setText(title);
    }
    if (!switchingWorkbook_) {
        syncCurrentWorkbookPage();
        refreshWorkbookTabs();
    }
}

void MainWindow::updateNavigationActions()
{
    if (returnToParentAction_) {
        returnToParentAction_->setEnabled(!graphStack_.isEmpty());
    }
    if (forwardToChildAction_) {
        forwardToChildAction_->setEnabled(!forwardMacroHistory_.isEmpty());
    }
}

void MainWindow::setPreviewImage(const QImage& image)
{
    currentPreviewImage_ = image;
    if (exportPreviewAction_) {
        exportPreviewAction_->setEnabled(!currentPreviewImage_.isNull());
    }
    if (auto* label = dynamic_cast<PreviewLabel*>(preview_)) {
        label->setSourceImage(image);
    }
}

void MainWindow::updatePreviewForSelection()
{
    if (!selectedNodeId_.isEmpty() && lastResult_.nodeOutputs.contains(selectedNodeId_)) {
        for (const auto& data : lastResult_.nodeOutputs.value(selectedNodeId_)) {
            if (portTypesCompatible(data.type, PortType::ImageRGBA)) {
                setPreviewImage(data.value.value<QImage>());
                return;
            }
        }
    }
    QStringList ids = lastResult_.nodeOutputs.keys();
    for (auto idIt = ids.rbegin(); idIt != ids.rend(); ++idIt) {
        for (const auto& data : lastResult_.nodeOutputs.value(*idIt)) {
            if (portTypesCompatible(data.type, PortType::ImageRGBA)) {
                setPreviewImage(data.value.value<QImage>());
                return;
            }
        }
    }
    setPreviewImage({});
}

void MainWindow::resetDockLayout()
{
    if (!previewDock_ || !bottomDock_) {
        return;
    }
    previewDock_->setFloating(false);
    bottomDock_->setFloating(false);
    addDockWidget(Qt::RightDockWidgetArea, previewDock_);
    addDockWidget(Qt::RightDockWidgetArea, bottomDock_);
    splitDockWidget(previewDock_, bottomDock_, Qt::Vertical);
    previewDock_->show();
    bottomDock_->show();
    if (miniMap_) {
        miniMap_->show();
        QSettings settings;
        settings.setValue("mainWindow/showMiniMap", true);
    }
    const auto metrics = AppTheme::metrics(uiScale_);
    resizeDocks({previewDock_, bottomDock_}, {metrics.previewMinHeight, metrics.logMaxHeight + metrics.previewMinHeight}, Qt::Vertical);
}

void MainWindow::toggleFullScreenMode()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void MainWindow::showSettingsDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("设置");
    dialog.resize(AppTheme::px(420, uiScale_), AppTheme::px(280, uiScale_));

    auto* root = new QVBoxLayout(&dialog);
    root->setContentsMargins(AppTheme::px(18, uiScale_), AppTheme::px(18, uiScale_), AppTheme::px(18, uiScale_), AppTheme::px(14, uiScale_));
    root->setSpacing(AppTheme::px(12, uiScale_));

    auto* title = new QLabel("软件设置");
    QFont titleFont = title->font();
    titleFont.setPointSizeF(titleFont.pointSizeF() + 2);
    titleFont.setBold(true);
    title->setFont(titleFont);
    root->addWidget(title);

    auto* formHost = new QWidget;
    formHost->setObjectName("glassPanel");
    auto* form = new QFormLayout(formHost);
    form->setContentsMargins(AppTheme::px(14, uiScale_), AppTheme::px(14, uiScale_), AppTheme::px(14, uiScale_), AppTheme::px(14, uiScale_));
    form->setHorizontalSpacing(AppTheme::px(14, uiScale_));
    form->setVerticalSpacing(AppTheme::px(10, uiScale_));

    auto* uiScaleSpin = new GuiCompat::DoubleSpinBox;
    uiScaleSpin->setRange(AppTheme::kMinUiScale * 100.0, AppTheme::kMaxUiScale * 100.0);
    uiScaleSpin->setSingleStep(AppTheme::kUiScaleStep * 100.0);
    uiScaleSpin->setDecimals(0);
    uiScaleSpin->setSuffix("%");
    uiScaleSpin->setValue(uiScale_ * 100.0);
    form->addRow("界面大小", uiScaleSpin);

    auto* themeCombo = new GuiCompat::ComboBox;
    themeCombo->addItem("跟随系统", "system");
    themeCombo->addItem("浅色", "light");
    themeCombo->addItem("深色", "dark");
    const int themeIndex = themeCombo->findData(AppTheme::themePreferenceName());
    themeCombo->setCurrentIndex(themeIndex >= 0 ? themeIndex : 0);
    form->addRow("界面主题", themeCombo);

    auto* canvasRow = new QWidget;
    auto* canvasLayout = new QHBoxLayout(canvasRow);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    auto* canvasZoomSlider = new QSlider(Qt::Horizontal);
    canvasZoomSlider->setRange(25, 300);
    canvasZoomSlider->setValue(int(std::lround(zoomScale_ * 100.0)));
    auto* canvasZoomLabel = new QLabel(QString("%1%").arg(canvasZoomSlider->value()));
    canvasZoomLabel->setMinimumWidth(AppTheme::px(48, uiScale_));
    canvasLayout->addWidget(canvasZoomSlider);
    canvasLayout->addWidget(canvasZoomLabel);
    connect(canvasZoomSlider, &QSlider::valueChanged, canvasZoomLabel, [canvasZoomLabel](int value) {
        canvasZoomLabel->setText(QString("%1%").arg(value));
    });
    form->addRow("画布缩放", canvasRow);

    auto* previewVisible = new GuiCompat::CheckBox;
    previewVisible->setChecked(!previewDock_ || previewDock_->isVisible());
    form->addRow("显示预览", previewVisible);
    auto* bottomVisible = new GuiCompat::CheckBox;
    bottomVisible->setChecked(!bottomDock_ || bottomDock_->isVisible());
    form->addRow("显示底部面板", bottomVisible);
    auto* miniMapVisible = new GuiCompat::CheckBox;
    miniMapVisible->setChecked(!miniMap_ || miniMap_->isVisible());
    form->addRow("显示小地图", miniMapVisible);

    root->addWidget(formHost);

    auto* utilityRow = new QWidget;
    auto* utilityLayout = new QHBoxLayout(utilityRow);
    utilityLayout->setContentsMargins(0, 0, 0, 0);
    auto* resetScaleButton = new GuiCompat::PushButton("重置界面大小");
    auto* resetLayoutButton = new GuiCompat::PushButton("重置布局");
    utilityLayout->addWidget(resetScaleButton);
    utilityLayout->addWidget(resetLayoutButton);
    utilityLayout->addStretch();
    root->addWidget(utilityRow);

    connect(resetScaleButton, &QPushButton::clicked, this, [uiScaleSpin] {
        uiScaleSpin->setValue(100.0);
    });
    connect(resetLayoutButton, &QPushButton::clicked, this, [this, previewVisible, bottomVisible, miniMapVisible] {
        resetDockLayout();
        previewVisible->setChecked(true);
        bottomVisible->setChecked(true);
        miniMapVisible->setChecked(true);
    });

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    root->addWidget(buttons);

    auto applySettings = [&] {
        AppTheme::setThemePreference(themeCombo->currentData().toString());
        QSettings settings;
        settings.setValue("mainWindow/theme", AppTheme::themePreferenceName());
        setUiScale(uiScaleSpin->value() / 100.0);
        applyUiScale();
        const double requestedZoom = canvasZoomSlider->value() / 100.0;
        if (!qFuzzyCompare(requestedZoom, zoomScale_)) {
            applyZoomFactor(requestedZoom / zoomScale_);
        }
        if (previewDock_) previewDock_->setVisible(previewVisible->isChecked());
        if (bottomDock_) bottomDock_->setVisible(bottomVisible->isChecked());
        if (miniMap_) miniMap_->setVisible(miniMapVisible->isChecked());
        settings.setValue("mainWindow/showMiniMap", miniMapVisible->isChecked());
    };

    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        applySettings();
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::clicked, &dialog, [&](QAbstractButton* button) {
        if (buttons->standardButton(button) == QDialogButtonBox::Apply) {
            applySettings();
        }
    });

    dialog.exec();
}

void MainWindow::increaseUiScale()
{
    setUiScale(uiScale_ + AppTheme::kUiScaleStep);
}

void MainWindow::decreaseUiScale()
{
    setUiScale(uiScale_ - AppTheme::kUiScaleStep);
}

void MainWindow::resetUiScale()
{
    setUiScale(1.0);
}

void MainWindow::setUiScale(double scale)
{
    const double nextScale = AppTheme::clampedScale(scale);
    if (qFuzzyCompare(nextScale, uiScale_)) {
        return;
    }
    uiScale_ = nextScale;
    applyUiScale();
    QSettings settings;
    settings.setValue("mainWindow/uiScale", uiScale_);
}

void MainWindow::applyUiScale()
{
    if (auto* app = qobject_cast<QApplication*>(QApplication::instance())) {
        AppTheme::apply(*app, uiScale_);
    }
    const auto themeColors = AppTheme::colors();
    auto applyBackground = [](QWidget* widget, const QColor& color) {
        if (!widget) {
            return;
        }
        QPalette palette = widget->palette();
        palette.setColor(QPalette::Window, color);
        palette.setColor(QPalette::Base, color);
        widget->setPalette(palette);
        widget->setAutoFillBackground(true);
        widget->update();
    };
    applyBackground(this, themeColors.canvasBottom);
    if (view_) {
        applyBackground(view_, themeColors.canvasBottom);
        applyBackground(view_->viewport(), themeColors.canvasBottom);
    }

    auto refreshToolbar = [](QToolBar* toolbar) {
        if (!toolbar) {
            return;
        }
        toolbar->setMovable(false);
        toolbar->setFloatable(false);
        for (auto* action : toolbar->actions()) {
            const QString iconName = action->data().toString();
            if (!iconName.isEmpty()) {
                action->setIcon(lineIcon(iconName));
            }
        }
    };
    auto refreshActionIcon = [](QAction* action) {
        if (!action) {
            return;
        }
        const QString iconName = action->data().toString();
        if (!iconName.isEmpty()) {
            action->setIcon(lineIcon(iconName));
        }
    };

    const auto metrics = AppTheme::metrics(uiScale_);
    if (mainToolbar_) {
        refreshToolbar(mainToolbar_);
        mainToolbar_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        mainToolbar_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    if (headerToolbar_) {
        refreshToolbar(headerToolbar_);
        headerToolbar_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        headerToolbar_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    if (workbookToolbar_) {
        refreshToolbar(workbookToolbar_);
        workbookToolbar_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        workbookToolbar_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    refreshActionIcon(returnToParentAction_);
    refreshActionIcon(forwardToChildAction_);
    refreshActionIcon(previewToggleAction_);
    refreshActionIcon(bottomToggleAction_);
    refreshActionIcon(settingsAction_);
    if (workbookTabs_) {
        workbookTabs_->setMinimumWidth(AppTheme::px(220, uiScale_));
        workbookTabs_->setMaximumWidth(AppTheme::px(520, uiScale_));
    }

    if (canvasZoomOverlay_) {
        canvasZoomOverlay_->setFixedSize(metrics.canvasZoomOverlayW, metrics.canvasZoomOverlayH);
        canvasZoomOverlay_->setGraphicsEffect(AppTheme::makeShadow(canvasZoomOverlay_, uiScale_, 40));
    }
    if (canvasZoomInButton_) {
        canvasZoomInButton_->setFixedSize(metrics.canvasZoomButtonW, metrics.canvasZoomButtonH);
        canvasZoomInButton_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        canvasZoomInButton_->setIcon(lineIcon("zoomIn"));
    }
    if (canvasZoomOutButton_) {
        canvasZoomOutButton_->setFixedSize(metrics.canvasZoomButtonW, metrics.canvasZoomButtonH);
        canvasZoomOutButton_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        canvasZoomOutButton_->setIcon(lineIcon("zoomOut"));
    }
    if (miniMap_) {
        miniMap_->setFixedSize(AppTheme::px(180, uiScale_), AppTheme::px(126, uiScale_));
        miniMap_->update();
    }

    if (preview_) {
        preview_->setMinimumHeight(metrics.previewMinHeight);
    }
    if (log_) {
        log_->setMinimumHeight(metrics.logMaxHeight);
    }
    if (problemLog_) {
        problemLog_->setMinimumHeight(metrics.logMaxHeight);
    }
    if (previewDock_) {
        previewDock_->setGraphicsEffect(AppTheme::makeShadow(previewDock_, uiScale_));
    }
    if (bottomDock_) {
        bottomDock_->setGraphicsEffect(AppTheme::makeShadow(bottomDock_, uiScale_));
    }

    if (scene_) {
        const QString selected = selectedNodeId_;
        rebuildScene();
        if (auto* item = nodeItems_.value(selected)) {
            item->setSelected(true);
            selectedNodeId_ = selected;
        }
    }
    rebuildProperties();
    updatePreviewForSelection();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!confirmSaveIfNeeded()) {
        event->ignore();
        return;
    }
    QSettings settings;
    settings.setValue("mainWindow/geometry", saveGeometry());
    settings.setValue("mainWindow/state", saveState());
    settings.setValue("mainWindow/uiScale", uiScale_);
    settings.setValue("mainWindow/theme", AppTheme::themePreferenceName());
    settings.setValue("mainWindow/layoutVersion", 5);
    QMainWindow::closeEvent(event);
}
