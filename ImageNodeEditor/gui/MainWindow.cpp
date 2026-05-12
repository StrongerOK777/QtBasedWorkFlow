#include "gui/MainWindow.h"

#include "gui/AppTheme.h"
#include "gui/WorkflowCommands.h"
#include "nodes/ImageNode.h"
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
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGuiApplication>
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
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QSlider>
#include <QSplitter>
#include <QStackedLayout>
#include <QSpinBox>
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
        return QColor("#0a84ff");
    case PortType::Number:
        return QColor("#34c759");
    case PortType::Text:
        return QColor("#af52de");
    case PortType::Mask:
        return QColor("#ff9f0a");
    case PortType::ImageList:
        return QColor("#14c9c9");
    }
    return QColor("#8e9aab");
}

QString portDirectionName(PortDirection direction)
{
    return direction == PortDirection::Input ? "输入" : "输出";
}

QPainterPath roundedRectPath(const QRectF& rect, qreal radius)
{
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    return path;
}

QIcon lineIcon(const QString& name)
{
    constexpr int size = 64;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    const QColor ink = AppTheme::isDarkTheme() ? QColor("#dbe7f5") : QColor("#23405f");
    const QColor accent = AppTheme::isDarkTheme() ? QColor("#60a5fa") : QColor("#0a84ff");
    const QColor warm("#ff9f0a");
    QPen pen(ink, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    auto page = [&] {
        painter.drawRoundedRect(QRectF(18, 10, 28, 42), 4, 4);
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
        painter.drawRoundedRect(QRectF(13, 11, 38, 42), 5, 5);
        painter.drawRect(QRectF(21, 11, 22, 14));
        painter.drawRoundedRect(QRectF(22, 35, 20, 14), 3, 3);
    } else if (name == "saveAs") {
        painter.drawRoundedRect(QRectF(12, 12, 34, 40), 5, 5);
        painter.drawRect(QRectF(20, 12, 18, 12));
        painter.setPen(QPen(warm, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(40, 41), QPointF(54, 27));
        painter.drawLine(QPointF(47, 27), QPointF(54, 27));
        painter.drawLine(QPointF(54, 27), QPointF(54, 34));
    } else if (name == "run") {
        painter.setBrush(accent);
        painter.setPen(Qt::NoPen);
        QPainterPath play;
        play.moveTo(23, 14);
        play.lineTo(50, 32);
        play.lineTo(23, 50);
        play.closeSubpath();
        painter.drawPath(play);
    } else if (name == "exportCanvas") {
        painter.drawRoundedRect(QRectF(12, 14, 40, 32), 4, 4);
        painter.drawLine(QPointF(20, 24), QPointF(44, 24));
        painter.drawLine(QPointF(20, 32), QPointF(36, 32));
        painter.setPen(QPen(accent, 4.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(32, 49), QPointF(32, 32));
        painter.drawLine(QPointF(24, 41), QPointF(32, 49));
        painter.drawLine(QPointF(40, 41), QPointF(32, 49));
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
        painter.drawRoundedRect(QRectF(12, 12, 40, 40), 4, 4);
        painter.fillRect(QRectF(12, 12, 13, 40), QColor(10, 132, 255, 70));
        painter.drawLine(QPointF(25, 12), QPointF(25, 52));
    } else if (name == "dockRight") {
        painter.drawRoundedRect(QRectF(12, 12, 40, 40), 4, 4);
        painter.fillRect(QRectF(39, 12, 13, 40), QColor(10, 132, 255, 70));
        painter.drawLine(QPointF(39, 12), QPointF(39, 52));
    } else if (name == "dockBottom") {
        painter.drawRoundedRect(QRectF(12, 12, 40, 40), 4, 4);
        painter.fillRect(QRectF(12, 39, 40, 13), QColor(10, 132, 255, 70));
        painter.drawLine(QPointF(12, 39), QPointF(52, 39));
    } else if (name == "layoutReset") {
        painter.drawRoundedRect(QRectF(12, 12, 40, 40), 4, 4);
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
            base = QColor("#ff3b30");
        } else if (feedback_ == PortConnectionFeedback::Snapped) {
            base = QColor("#ff9f0a");
        } else if (feedback_ == PortConnectionFeedback::Connectable) {
            base = QColor("#34c759");
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
        setRect(0, 0, metrics.width, metrics.topPadding + rowCount * metrics.rowHeight + metrics.bottomPadding);
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

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override
    {
        if (change == ItemPositionHasChanged && window_) {
            window_->updateNodePosition(nodeId_, value.toPointF());
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
            switch (runState_) {
            case NodeExecutionState::Running: return QColor("#f5a623");
            case NodeExecutionState::Succeeded: return QColor("#34c759");
            case NodeExecutionState::Failed: return QColor("#ff3b30");
            case NodeExecutionState::CacheHit: return QColor("#7c3aed");
            case NodeExecutionState::NotExecuted: return colors.nodeBorder;
            }
            return colors.nodeBorder;
        }();

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(colors.nodeShadow);
        painter->drawPath(roundedRectPath(shadowRect.adjusted(3 * uiScale_, 3 * uiScale_, -3 * uiScale_, -1 * uiScale_), metrics.cornerRadius));

        QLinearGradient glass(body.topLeft(), body.bottomLeft());
        glass.setColorAt(0, hovered_ ? colors.nodeTop.lighter(104) : colors.nodeTop);
        glass.setColorAt(0.52, QColor(248, 251, 255, 226));
        glass.setColorAt(1, hovered_ ? colors.nodeBottom.lighter(104) : colors.nodeBottom);
        painter->setBrush(glass);
        painter->setPen(QPen(isSelected() ? colors.nodeSelected : stateColor,
                             (isSelected() || runState_ != NodeExecutionState::NotExecuted) ? 2.3 * uiScale_ : 1.1 * uiScale_));
        painter->drawPath(roundedRectPath(body, metrics.cornerRadius));

        QRectF header = body;
        header.setHeight(metrics.headerHeight);
        QLinearGradient highlight(header.topLeft(), header.bottomLeft());
        highlight.setColorAt(0, QColor(255, 255, 255, 190));
        highlight.setColorAt(1, QColor(255, 255, 255, 38));
        painter->setPen(Qt::NoPen);
        painter->setBrush(highlight);
        painter->drawPath(roundedRectPath(header, metrics.cornerRadius));

        painter->setPen(QPen(QColor(255, 255, 255, 150), 1));
        painter->drawLine(body.left() + metrics.cornerRadius, body.top() + 1, body.right() - metrics.cornerRadius, body.top() + 1);

        if (runState_ != NodeExecutionState::NotExecuted) {
            const QRectF strip(body.left() + 8 * uiScale_, body.bottom() - 6 * uiScale_,
                               body.width() - 16 * uiScale_, 3 * uiScale_);
            painter->setPen(Qt::NoPen);
            painter->setBrush(stateColor);
            painter->drawRoundedRect(strip, 1.5 * uiScale_, 1.5 * uiScale_);
        }
    }

private:
    MainWindow* window_ = nullptr;
    QString nodeId_;
    QSharedPointer<ImageNode> node_;
    double uiScale_ = 1.0;
    QVector<PortItem*> ports_;
    NodeExecutionState runState_ = NodeExecutionState::NotExecuted;
    QPointF dragStartPos_;
    bool hovered_ = false;
    bool pressed_ = false;
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
        QPen glow(QColor(10, 132, 255, isSelected() ? 54 : 18), isSelected() ? 8 * uiScale_ : 5 * uiScale_);
        glow.setCapStyle(Qt::RoundCap);
        glow.setJoinStyle(Qt::RoundJoin);
        QPen currentPen(isSelected() ? colors.edgeSelected : colors.edge, isSelected() ? 3.2 * uiScale_ : 2.1 * uiScale_);
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
        painter.setPen(QPen(QColor(126, 154, 192, 120), 1));
        painter.setBrush(QColor(255, 255, 255, AppTheme::isDarkTheme() ? 36 : 190));
        painter.drawRoundedRect(box, 8, 8);

        const QRectF world = worldRect();
        if (!world.isValid() || world.isEmpty()) {
            return;
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#0a84ff"));
        for (const auto& record : graph_->nodes()) {
            QRectF nodeRect(record.position, QSizeF(AppTheme::nodeMetrics(scale()).width, AppTheme::nodeMetrics(scale()).headerHeight + 78 * scale()));
            painter.drawRoundedRect(mapWorldRect(nodeRect, world), 2, 2);
        }

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor("#ff9f0a"), 1.6));
        painter.drawRoundedRect(mapWorldRect(viewSceneRect(), world), 2, 2);
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
            clearSelection();
            node->setSelected(true);
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
        QPen pen(snappedPort_ && snappedConnectable_ ? QColor("#34c759") :
                     blockedNearPort_ ? QColor("#ff3b30") : AppTheme::colors().pendingEdge,
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
        const auto colors = AppTheme::colors();
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0, colors.canvasTop);
        gradient.setColorAt(1, colors.canvasBottom);
        painter->fillRect(rect, gradient);

        const double scale = window_ ? window_->uiScale() : 1.0;
        const qreal step = 26.0 * scale;
        const qreal dotRadius = std::max<qreal>(1.0, 1.15 * scale);
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
    rebuildPalette();
    applyUiScale();
}

void MainWindow::createActions()
{
    auto* file = menuBar()->addMenu("文件");
    mainToolbar_ = addToolBar("主工具栏");
    mainToolbar_->setObjectName("mainToolbar");
    mainToolbar_->setMovable(false);
    mainToolbar_->setFloatable(false);
    auto addAction = [&](const QString& text, const QString& iconName, auto slot) {
        auto* action = new QAction(text, this);
        action->setIcon(lineIcon(iconName));
        action->setData(iconName);
        action->setToolTip(text);
        action->setStatusTip(text);
        connect(action, &QAction::triggered, this, slot);
        file->addAction(action);
        mainToolbar_->addAction(action);
        return action;
    };
    addAction("新建", "new", [this] { newWorkflow(); });
    addAction("打开", "open", [this] { openWorkflow(); });
    addAction("保存", "save", [this] { saveWorkflow(); });
    addAction("另存为", "saveAs", [this] { saveWorkflowAs(); });
    addAction("执行", "run", [this] { runWorkflow(); });
    auto* exportCanvasAction = addAction("导出画布截图", "exportCanvas", [this] { exportCanvasImage(); });
    const QKeySequence exportShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_E);
    exportCanvasAction->setShortcut(exportShortcut);
    exportCanvasAction->setToolTip(QString("导出画布截图（%1）").arg(exportShortcut.toString(QKeySequence::NativeText)));

    auto* editMenu = menuBar()->addMenu("编辑");
    auto* undoAction = undoStack_->createUndoAction(this, "撤销");
    undoAction->setShortcuts(QKeySequence::Undo);
    auto* redoAction = undoStack_->createRedoAction(this, "重做");
    redoAction->setShortcuts(QKeySequence::Redo);
    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    auto* quickNodeAction = new QAction("快捷添加节点", this);
    const QKeySequence quickNodeShortcut(Qt::CTRL | Qt::Key_K);
    quickNodeAction->setShortcut(quickNodeShortcut);
    quickNodeAction->setToolTip(QString("快捷添加节点（%1；画布 Tab）").arg(quickNodeShortcut.toString(QKeySequence::NativeText)));
    quickNodeAction->setStatusTip("搜索并在当前画布位置添加节点");
    connect(quickNodeAction, &QAction::triggered, this, [this] { showQuickNodePalette(); });
    editMenu->addAction(quickNodeAction);

    auto* viewMenu = menuBar()->addMenu("视图");
    viewToolbar_ = addToolBar("界面缩放");
    viewToolbar_->setObjectName("viewScaleToolbar");
    viewToolbar_->setMovable(false);
    viewToolbar_->setFloatable(false);
    auto addViewAction = [&](const QString& text, const QString& iconName, const QList<QKeySequence>& shortcuts, auto slot) {
        auto* action = new QAction(text, this);
        action->setIcon(lineIcon(iconName));
        action->setData(iconName);
        action->setToolTip(text);
        action->setStatusTip(text);
        action->setShortcuts(shortcuts);
        connect(action, &QAction::triggered, this, slot);
        viewMenu->addAction(action);
        viewToolbar_->addAction(action);
        return action;
    };
    addViewAction("界面放大", "zoomIn", {QKeySequence::ZoomIn, QKeySequence(Qt::CTRL | Qt::Key_Equal)}, [this] { increaseUiScale(); });
    addViewAction("界面缩小", "zoomOut", {QKeySequence::ZoomOut}, [this] { decreaseUiScale(); });
    addViewAction("重置界面大小", "scaleReset", {QKeySequence(Qt::CTRL | Qt::Key_0)}, [this] { resetUiScale(); });

    auto* settingsMenu = menuBar()->addMenu("设置");
    auto* settingsAction = new QAction(lineIcon("settings"), "打开设置", this);
    settingsAction->setData("settings");
    settingsAction->setToolTip("打开设置");
    settingsAction->setStatusTip("打开设置");
    connect(settingsAction, &QAction::triggered, this, [this] { showSettingsDialog(); });
    settingsMenu->addAction(settingsAction);
    viewToolbar_->addSeparator();
    viewToolbar_->addAction(settingsAction);

    installDelayedTooltips(mainToolbar_);
    installDelayedTooltips(viewToolbar_);
}

void MainWindow::createLayout()
{
    palette_ = new QListWidget;
    palette_->setMinimumWidth(180);
    connect(palette_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        addNodeFromType(item->data(Qt::UserRole).toString(), findAvailableNodePosition(view_->mapToScene(view_->viewport()->rect().center())));
    });

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
    view_->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    view_->setDragMode(QGraphicsView::NoDrag);
    view_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    view_->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    view_->setFrameShape(QFrame::NoFrame);

    auto* viewContainer = new QWidget;
    viewContainer->setObjectName("canvasContainer");
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
    viewStack->addWidget(miniMap_);
    viewStack->setAlignment(miniMap_, Qt::AlignLeft | Qt::AlignBottom);
    connect(canvasZoomInButton_, &QToolButton::clicked, this, [this] { zoomIn(); });
    connect(canvasZoomOutButton_, &QToolButton::clicked, this, [this] { zoomOut(); });

    propertyPanel_ = new QWidget;
    propertyPanel_->setObjectName("glassPanel");
    propertyLayout_ = new QFormLayout(propertyPanel_);

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

    auto* bottom = new QWidget;
    bottom->setObjectName("glassPanel");
    auto* bottomLayout = new QVBoxLayout(bottom);
    bottomLayout->addWidget(preview_);
    bottomLayout->addWidget(log_);

    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks | QMainWindow::AnimatedDocks | QMainWindow::GroupedDragging);
    GuiCompat::setMainContent(this, viewContainer);

    auto makeDock = [this](const QString& title, const QString& objectName, QWidget* widget) {
        auto* dock = new GuiCompat::DockWidget(title, this);
        dock->setObjectName(objectName);
        dock->setWidget(widget);
        dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        return dock;
    };

    paletteDock_ = makeDock("节点栏", "paletteDock", palette_);
    propertyDock_ = makeDock("属性", "propertyDock", propertyPanel_);
    bottomDock_ = makeDock("预览与日志", "bottomDock", bottom);
    paletteDock_->setGraphicsEffect(AppTheme::makeShadow(paletteDock_, uiScale_));
    propertyDock_->setGraphicsEffect(AppTheme::makeShadow(propertyDock_, uiScale_));
    bottomDock_->setGraphicsEffect(AppTheme::makeShadow(bottomDock_, uiScale_));

    addDockWidget(Qt::LeftDockWidgetArea, paletteDock_);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock_);
    addDockWidget(Qt::RightDockWidgetArea, bottomDock_);
    splitDockWidget(propertyDock_, bottomDock_, Qt::Vertical);
    resizeDocks({paletteDock_, propertyDock_}, {220, 300}, Qt::Horizontal);
    resizeDocks({propertyDock_, bottomDock_}, {360, 360}, Qt::Vertical);

    auto* layoutMenu = menuBar()->addMenu("布局");
    layoutMenu->addAction(paletteDock_->toggleViewAction());
    layoutMenu->addAction(propertyDock_->toggleViewAction());
    layoutMenu->addAction(bottomDock_->toggleViewAction());
    layoutMenu->addSeparator();

    auto addDockMoveAction = [&](const QString& text, QDockWidget* dock, Qt::DockWidgetArea area) {
        auto* action = layoutMenu->addAction(text);
        connect(action, &QAction::triggered, this, [this, dock, area] {
            addDockWidget(area, dock);
            dock->show();
        });
        return action;
    };
    addDockMoveAction("节点栏在左侧", paletteDock_, Qt::LeftDockWidgetArea);
    addDockMoveAction("节点栏在右侧", paletteDock_, Qt::RightDockWidgetArea);
    addDockMoveAction("属性在左侧", propertyDock_, Qt::LeftDockWidgetArea);
    addDockMoveAction("属性在右侧", propertyDock_, Qt::RightDockWidgetArea);
    addDockMoveAction("预览日志在底部", bottomDock_, Qt::BottomDockWidgetArea);
    addDockMoveAction("预览日志在右侧", bottomDock_, Qt::RightDockWidgetArea);
    layoutMenu->addSeparator();
    auto* resetAction = layoutMenu->addAction("重置布局");
    connect(resetAction, &QAction::triggered, this, [this] { resetDockLayout(); });
    auto* fullScreenAction = layoutMenu->addAction("全屏切换");
    connect(fullScreenAction, &QAction::triggered, this, [this] { toggleFullScreenMode(); });

    layoutToolbar_ = addToolBar("布局");
    layoutToolbar_->setObjectName("layoutToolbar");
    layoutToolbar_->setMovable(false);
    layoutToolbar_->setFloatable(false);
    paletteDock_->toggleViewAction()->setIcon(lineIcon("dockLeft"));
    paletteDock_->toggleViewAction()->setData("dockLeft");
    paletteDock_->toggleViewAction()->setToolTip("显示或隐藏节点栏");
    propertyDock_->toggleViewAction()->setIcon(lineIcon("dockRight"));
    propertyDock_->toggleViewAction()->setData("dockRight");
    propertyDock_->toggleViewAction()->setToolTip("显示或隐藏属性");
    bottomDock_->toggleViewAction()->setIcon(lineIcon("dockBottom"));
    bottomDock_->toggleViewAction()->setData("dockBottom");
    bottomDock_->toggleViewAction()->setToolTip("显示或隐藏预览与日志");
    resetAction->setIcon(lineIcon("layoutReset"));
    resetAction->setData("layoutReset");
    resetAction->setToolTip("重置布局");
    fullScreenAction->setIcon(lineIcon("fullscreen"));
    fullScreenAction->setData("fullscreen");
    fullScreenAction->setToolTip("全屏切换");
    layoutToolbar_->addAction(paletteDock_->toggleViewAction());
    layoutToolbar_->addAction(propertyDock_->toggleViewAction());
    layoutToolbar_->addAction(bottomDock_->toggleViewAction());
    layoutToolbar_->addAction(resetAction);
    layoutToolbar_->addAction(fullScreenAction);
    installDelayedTooltips(layoutToolbar_);

    QSettings settings;
    restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
    if (settings.value("mainWindow/layoutVersion", 0).toInt() >= 2) {
        restoreState(settings.value("mainWindow/state").toByteArray());
    } else {
        resetDockLayout();
    }
}

void MainWindow::rebuildPalette()
{
    palette_->clear();
    for (const auto& descriptor : NodeFactory::instance().descriptors()) {
        auto* item = new QListWidgetItem(QString("%1 / %2").arg(descriptor.category, descriptor.displayName));
        item->setData(Qt::UserRole, descriptor.typeName);
        palette_->addItem(item);
    }
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
    QAction* deleteAction = menu.addAction("删除执行节点");
    QAction* editAction = menu.addAction("修改参数");

    QAction* chosen = menu.exec(screenPos);
    if (chosen == copyAction) {
        copySelectedNode();
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
    propertyPanel_->setFocus(Qt::OtherFocusReason);
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

    for (int i = 0; i < graph_.edges().size(); ++i) {
        const auto& edge = graph_.edges().at(i);
        const QPointF a = portPosition(edge.fromNode, edge.fromPort, PortDirection::Output);
        const QPointF b = portPosition(edge.toNode, edge.toPort, PortDirection::Input);
        auto* line = new EdgeItem(i, a, b, uiScale_);
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
    while (auto* item = propertyLayout_->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    auto node = graph_.node(selectedNodeId_);
    if (!node) {
        propertyLayout_->addRow(new QLabel("未选中节点"));
        return;
    }
    auto* title = new QLabel(QString("%1\n%2").arg(node->displayName(), selectedNodeId_));
    title->setWordWrap(true);
    propertyLayout_->addRow(title);

    for (const auto& p : node->parameterDefinitions()) {
        const QVariant value = node->parameterValue(p.name);
        QWidget* editor = nullptr;
        if (p.type == ParameterType::Integer) {
            auto* w = new GuiCompat::SpinBox;
            w->setRange(int(p.min), int(p.max));
            w->setValue(value.toInt());
            connect(w, &QSpinBox::valueChanged, this, [this, name = p.name](int v) {
                setSelectedNodeParameter(name, v);
            });
            editor = w;
        } else if (p.type == ParameterType::Double) {
            auto* w = new GuiCompat::DoubleSpinBox;
            w->setRange(p.min, p.max);
            w->setSingleStep(0.05);
            w->setValue(value.toDouble());
            connect(w, &QDoubleSpinBox::valueChanged, this, [this, name = p.name](double v) {
                setSelectedNodeParameter(name, v);
            });
            editor = w;
        } else if (p.type == ParameterType::Boolean) {
            auto* w = new GuiCompat::CheckBox;
            w->setChecked(value.toBool());
            connect(w, &QCheckBox::toggled, this, [this, name = p.name](bool v) {
                setSelectedNodeParameter(name, v);
            });
            editor = w;
        } else if (p.type == ParameterType::Choice) {
            auto* w = new GuiCompat::ComboBox;
            w->addItems(p.options);
            w->setCurrentText(value.toString());
            connect(w, &QComboBox::currentTextChanged, this, [this, name = p.name](const QString& v) {
                setSelectedNodeParameter(name, v);
            });
            editor = w;
        } else {
            auto* container = new QWidget;
            auto* layout = new QHBoxLayout(container);
            layout->setContentsMargins(0, 0, 0, 0);
            auto* edit = new GuiCompat::LineEdit;
            edit->setText(value.toString());
            layout->addWidget(edit);
            if (p.type == ParameterType::FileOpen || p.type == ParameterType::FileSave || p.type == ParameterType::Color) {
                auto* button = new GuiCompat::PushButton("...");
                layout->addWidget(button);
                connect(button, &QPushButton::clicked, this, [this, edit, p] {
                    QString v;
                    if (p.type == ParameterType::FileOpen) {
                        v = QFileDialog::getOpenFileName(this, "选择图片", {}, "Images (*.png *.jpg *.jpeg *.bmp *.webp);;All files (*)");
                    } else if (p.type == ParameterType::FileSave) {
                        v = QFileDialog::getSaveFileName(this, "选择导出路径", edit->text(), "PNG (*.png);;JPEG (*.jpg);;All files (*)");
                    } else {
                        QColor c = QColorDialog::getColor(QColor(edit->text()), this);
                        if (c.isValid()) v = c.name();
                    }
                    if (!v.isEmpty()) {
                        edit->setText(v);
                        setSelectedNodeParameter(p.name, v);
                    }
                });
            }
            connect(edit, &QLineEdit::editingFinished, this, [this, edit, name = p.name] {
                setSelectedNodeParameter(name, edit->text());
            });
            editor = container;
        }
        propertyLayout_->addRow(p.displayName, editor);
    }
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

void MainWindow::setSelectedNodeParameter(const QString& name, const QVariant& value)
{
    auto node = graph_.node(selectedNodeId_);
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
                         QString("param:%1:%2").arg(selectedNodeId_, name));
    scheduleLivePreview();
}

void MainWindow::resetNodeRunStates()
{
    nodeRunStates_.clear();
    for (const auto& record : graph_.nodes()) {
        nodeRunStates_.insert(record.id, NodeExecutionState::NotExecuted);
    }
    for (auto it = nodeItems_.begin(); it != nodeItems_.end(); ++it) {
        if (auto* nodeItem = dynamic_cast<NodeItem*>(it.value())) {
            nodeItem->setRunState(NodeExecutionState::NotExecuted);
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
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
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
    QPen pen(QColor("#ff9f0a"), 3.0 * uiScale_, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
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
        for (const auto& summary : lastResult_.nodeSummaries) {
            appendLog(summary.message, summary.nodeId);
        }
        appendLog(QString("执行失败：%1").arg(result.error()), lastResult_.failedNodeId);
        focusFailedNode(lastResult_.failedNodeId);
        QMessageBox::warning(this, "执行失败", result.error());
        return;
    }
    lastResult_ = result.value();
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
        for (const auto& summary : lastResult_.nodeSummaries) {
            appendLog(summary.message, summary.nodeId);
        }
        const QString failedNode = lastResult_.failedNodeId.isEmpty() ? previewNodeId : lastResult_.failedNodeId;
        appendLog(QString("实时预览失败：%1").arg(result.error()), failedNode);
        updatePreviewForSelection();
        return;
    }

    lastResult_ = result.value();
    for (const auto& summary : lastResult_.nodeSummaries) {
        appendLog(summary.message, summary.nodeId);
    }
    appendLog(QString("实时预览完成：%1").arg(previewNodeId), previewNodeId);
    updatePreviewForSelection();
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
    currentFile_.clear();
    engine_.clearCache();
    lastResult_ = {};
    nodeRunStates_.clear();
    selectedNodeId_.clear();
    undoStack_->clear();
    rebuildScene();
    rebuildProperties();
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
        QMessageBox::warning(this, "打开失败", loaded.error());
        return;
    }
    graph_ = loaded.value();
    currentFile_ = path;
    engine_.clearCache();
    lastResult_ = {};
    nodeRunStates_.clear();
    selectedNodeId_.clear();
    undoStack_->clear();
    rebuildScene();
    rebuildProperties();
    updateWindowTitle();
    appendLog(QString("已打开：%1").arg(path));
}

bool MainWindow::confirmSaveIfNeeded()
{
    if (!undoStack_ || undoStack_->isClean()) {
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
    auto saved = serializer.saveFile(graph_, currentFile_);
    if (saved.isFail()) {
        QMessageBox::warning(this, "保存失败", saved.error());
        return false;
    }
    if (undoStack_) {
        undoStack_->setClean();
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
    undoStack_->push(WorkflowCommands::makeSnapshotCommand(this, text, before, after, selectedBefore, selectedAfter, mergeKey));
}

void MainWindow::updateWindowTitle()
{
    const QString fileName = currentFile_.isEmpty() ? "未命名" : QFileInfo(currentFile_).fileName();
    const QString dirty = undoStack_ && !undoStack_->isClean() ? "*" : "";
    setWindowTitle(QString("%1%2 - ImageNodeEditor").arg(fileName, dirty));
}

void MainWindow::setPreviewImage(const QImage& image)
{
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
    if (!paletteDock_ || !propertyDock_ || !bottomDock_) {
        return;
    }
    paletteDock_->setFloating(false);
    propertyDock_->setFloating(false);
    bottomDock_->setFloating(false);
    addDockWidget(Qt::LeftDockWidgetArea, paletteDock_);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock_);
    addDockWidget(Qt::RightDockWidgetArea, bottomDock_);
    splitDockWidget(propertyDock_, bottomDock_, Qt::Vertical);
    paletteDock_->show();
    propertyDock_->show();
    bottomDock_->show();
    const auto metrics = AppTheme::metrics(uiScale_);
    resizeDocks({paletteDock_, propertyDock_}, {metrics.paletteMinWidth, metrics.propertyMinWidth}, Qt::Horizontal);
    resizeDocks({propertyDock_, bottomDock_}, {metrics.previewMinHeight + metrics.logMaxHeight, metrics.previewMinHeight + metrics.logMaxHeight}, Qt::Vertical);
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

    auto* paletteVisible = new GuiCompat::CheckBox;
    paletteVisible->setChecked(!paletteDock_ || paletteDock_->isVisible());
    form->addRow("显示节点栏", paletteVisible);
    auto* propertyVisible = new GuiCompat::CheckBox;
    propertyVisible->setChecked(!propertyDock_ || propertyDock_->isVisible());
    form->addRow("显示属性栏", propertyVisible);
    auto* bottomVisible = new GuiCompat::CheckBox;
    bottomVisible->setChecked(!bottomDock_ || bottomDock_->isVisible());
    form->addRow("显示预览与日志", bottomVisible);

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
    connect(resetLayoutButton, &QPushButton::clicked, this, [this, paletteVisible, propertyVisible, bottomVisible] {
        resetDockLayout();
        paletteVisible->setChecked(true);
        propertyVisible->setChecked(true);
        bottomVisible->setChecked(true);
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
        if (paletteDock_) paletteDock_->setVisible(paletteVisible->isChecked());
        if (propertyDock_) propertyDock_->setVisible(propertyVisible->isChecked());
        if (bottomDock_) bottomDock_->setVisible(bottomVisible->isChecked());
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

    const auto metrics = AppTheme::metrics(uiScale_);
    if (mainToolbar_) {
        refreshToolbar(mainToolbar_);
        mainToolbar_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        mainToolbar_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    if (layoutToolbar_) {
        refreshToolbar(layoutToolbar_);
        layoutToolbar_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        layoutToolbar_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    if (viewToolbar_) {
        refreshToolbar(viewToolbar_);
        viewToolbar_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        viewToolbar_->setToolButtonStyle(Qt::ToolButtonIconOnly);
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

    if (palette_) {
        palette_->setMinimumWidth(metrics.paletteMinWidth);
    }
    if (propertyPanel_) {
        propertyPanel_->setMinimumWidth(metrics.propertyMinWidth);
    }
    if (propertyLayout_) {
        propertyLayout_->setContentsMargins(metrics.formMargin, metrics.formMargin, metrics.formMargin, metrics.formMargin);
        propertyLayout_->setHorizontalSpacing(metrics.formSpacing);
        propertyLayout_->setVerticalSpacing(metrics.formSpacing);
    }
    if (preview_) {
        preview_->setMinimumHeight(metrics.previewMinHeight);
    }
    if (log_) {
        log_->setMaximumHeight(metrics.logMaxHeight);
    }
    if (paletteDock_) {
        paletteDock_->setGraphicsEffect(AppTheme::makeShadow(paletteDock_, uiScale_));
    }
    if (propertyDock_) {
        propertyDock_->setGraphicsEffect(AppTheme::makeShadow(propertyDock_, uiScale_));
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
    settings.setValue("mainWindow/layoutVersion", 2);
    QMainWindow::closeEvent(event);
}
