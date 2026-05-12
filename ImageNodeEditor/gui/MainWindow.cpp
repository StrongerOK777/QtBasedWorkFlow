#include "gui/MainWindow.h"

#include "gui/AppTheme.h"
#include "nodes/ImageNode.h"
#include "nodes/NodeFactory.h"
#include "workflow/ExecutionEngine.h"
#include "workflow/WorkflowSerializer.h"
#include "workflow/WorkflowValidator.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
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
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QSplitter>
#include <QStackedLayout>
#include <QSpinBox>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <functional>

namespace {

QPainterPath roundedRectPath(const QRectF& rect, qreal radius)
{
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    return path;
}

class PortItem final : public QGraphicsEllipseItem {
public:
    PortItem(QString nodeId, PortInfo port, double uiScale, QGraphicsItem* parent)
        : QGraphicsEllipseItem(parent), nodeId_(std::move(nodeId)), port_(std::move(port)), uiScale_(uiScale)
    {
        const auto metrics = AppTheme::nodeMetrics(uiScale_);
        setRect(-metrics.portRadius, -metrics.portRadius, metrics.portRadius * 2, metrics.portRadius * 2);
        setAcceptHoverEvents(true);
        setToolTip(QString("%1 (%2)").arg(port_.displayName, portTypeName(port_.type)));
    }

    QString nodeId() const { return nodeId_; }
    PortInfo port() const { return port_; }

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
        const auto colors = AppTheme::colors();
        const QColor base = port_.direction == PortDirection::Output ? colors.outputPort : colors.inputPort;
        const QRectF r = rect();

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(base.red(), base.green(), base.blue(), hovered_ ? 62 : 34));
        painter->drawEllipse(r.adjusted(-3 * uiScale_, -3 * uiScale_, 3 * uiScale_, 3 * uiScale_));

        QRadialGradient gradient(r.center() - QPointF(r.width() * 0.22, r.height() * 0.22), r.width());
        gradient.setColorAt(0, base.lighter(165));
        gradient.setColorAt(0.62, base);
        gradient.setColorAt(1, base.darker(125));
        painter->setBrush(gradient);
        painter->setPen(QPen(QColor(255, 255, 255, hovered_ ? 245 : 210), std::max(1.0, 1.2 * uiScale_)));
        painter->drawEllipse(r);
    }

private:
    QString nodeId_;
    PortInfo port_;
    double uiScale_ = 1.0;
    bool hovered_ = false;
};

class NodeItem final : public QGraphicsRectItem {
public:
    NodeItem(MainWindow* window, QString nodeId, QSharedPointer<ImageNode> node, double uiScale)
        : QGraphicsRectItem(), window_(window), nodeId_(std::move(nodeId)), node_(std::move(node)), uiScale_(uiScale)
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
        update();
        QGraphicsRectItem::mousePressEvent(event);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
    {
        pressed_ = false;
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

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(colors.nodeShadow);
        painter->drawPath(roundedRectPath(shadowRect.adjusted(3 * uiScale_, 3 * uiScale_, -3 * uiScale_, -1 * uiScale_), metrics.cornerRadius));

        QLinearGradient glass(body.topLeft(), body.bottomLeft());
        glass.setColorAt(0, hovered_ ? colors.nodeTop.lighter(104) : colors.nodeTop);
        glass.setColorAt(0.52, QColor(248, 251, 255, 226));
        glass.setColorAt(1, hovered_ ? colors.nodeBottom.lighter(104) : colors.nodeBottom);
        painter->setBrush(glass);
        painter->setPen(QPen(isSelected() ? colors.nodeSelected : colors.nodeBorder, isSelected() ? 2.3 * uiScale_ : 1.1 * uiScale_));
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
    }

private:
    MainWindow* window_ = nullptr;
    QString nodeId_;
    QSharedPointer<ImageNode> node_;
    double uiScale_ = 1.0;
    QVector<PortItem*> ports_;
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
            clearPendingLine();
            event->accept();
            return;
        }
        QGraphicsScene::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override
    {
        if (pendingLine_) {
            pendingLine_->setPath(pendingPath(pendingStart_, event->scenePos()));
        }
        QGraphicsScene::mouseMoveEvent(event);
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
        if (pendingLine_) {
            removeItem(pendingLine_);
            delete pendingLine_;
            pendingLine_ = nullptr;
        }
        pendingNode_.clear();
        pendingPort_.clear();
        pendingStart_ = {};
    }

    MainWindow* window_ = nullptr;
    QString pendingNode_;
    QString pendingPort_;
    QPointF pendingStart_;
    QGraphicsPathItem* pendingLine_ = nullptr;
};

class ZoomGraphicsView final : public QGraphicsView {
public:
    ZoomGraphicsView(QGraphicsScene* scene, MainWindow* window)
        : QGraphicsView(scene), window_(window)
    {
    }

protected:
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
        if (event->button() == Qt::LeftButton && (panCandidate_ || panning_)) {
            panCandidate_ = false;
            panning_ = false;
            unsetCursor();
            event->accept();
            return;
        }
        QGraphicsView::mouseReleaseEvent(event);
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
    : QMainWindow(parent)
{
    NodeFactory::instance().registerBuiltins();
    setWindowTitle("ImageNodeEditor");
    resize(1280, 820);
    QSettings settings;
    uiScale_ = AppTheme::clampedScale(settings.value("mainWindow/uiScale", 1.0).toDouble());
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
    auto addAction = [&](const QString& text, auto slot) {
        auto* action = new QAction(text, this);
        connect(action, &QAction::triggered, this, slot);
        file->addAction(action);
        mainToolbar_->addAction(action);
        return action;
    };
    addAction("新建", [this] { newWorkflow(); });
    addAction("打开", [this] { openWorkflow(); });
    addAction("保存", [this] { saveWorkflow(); });
    addAction("另存为", [this] { saveWorkflowAs(); });
    addAction("执行", [this] { runWorkflow(); });

    auto* viewMenu = menuBar()->addMenu("视图");
    viewToolbar_ = addToolBar("界面缩放");
    viewToolbar_->setObjectName("viewScaleToolbar");
    auto addViewAction = [&](const QString& text, const QString& toolText, const QList<QKeySequence>& shortcuts, auto slot) {
        auto* action = new QAction(text, this);
        action->setToolTip(text);
        if (!toolText.isEmpty()) {
            action->setIconText(toolText);
        }
        action->setShortcuts(shortcuts);
        connect(action, &QAction::triggered, this, slot);
        viewMenu->addAction(action);
        viewToolbar_->addAction(action);
        return action;
    };
    addViewAction("界面放大", "A+", {QKeySequence::ZoomIn, QKeySequence(Qt::CTRL | Qt::Key_Equal)}, [this] { increaseUiScale(); });
    addViewAction("界面缩小", "A-", {QKeySequence::ZoomOut}, [this] { decreaseUiScale(); });
    addViewAction("重置界面大小", "100%", {QKeySequence(Qt::CTRL | Qt::Key_0)}, [this] { resetUiScale(); });
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
    canvasZoomInButton_->setText("+");
    canvasZoomInButton_->setToolTip("放大画布");
    canvasZoomOutButton_ = new QToolButton;
    canvasZoomOutButton_->setText("-");
    canvasZoomOutButton_->setToolTip("缩小画布");
    zoomLayout->addWidget(canvasZoomInButton_, 0, Qt::AlignRight);
    zoomLayout->addWidget(canvasZoomOutButton_, 0, Qt::AlignRight);
    viewStack->addWidget(canvasZoomOverlay_);
    viewStack->setAlignment(canvasZoomOverlay_, Qt::AlignRight | Qt::AlignBottom);
    connect(canvasZoomInButton_, &QToolButton::clicked, this, [this] { zoomIn(); });
    connect(canvasZoomOutButton_, &QToolButton::clicked, this, [this] { zoomOut(); });

    propertyPanel_ = new QWidget;
    propertyPanel_->setObjectName("glassPanel");
    propertyLayout_ = new QFormLayout(propertyPanel_);

    preview_ = new PreviewLabel;
    preview_->setObjectName("previewPanel");
    preview_->setAlignment(Qt::AlignCenter);
    static_cast<PreviewLabel*>(preview_)->setSourceImage({});
    log_ = new QTextEdit;
    log_->setObjectName("logPanel");
    log_->setReadOnly(true);

    auto* bottom = new QWidget;
    bottom->setObjectName("glassPanel");
    auto* bottomLayout = new QVBoxLayout(bottom);
    bottomLayout->addWidget(preview_);
    bottomLayout->addWidget(log_);

    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks | QMainWindow::AnimatedDocks | QMainWindow::GroupedDragging);
    setCentralWidget(viewContainer);

    auto makeDock = [this](const QString& title, const QString& objectName, QWidget* widget) {
        auto* dock = new QDockWidget(title, this);
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
    addDockWidget(Qt::BottomDockWidgetArea, bottomDock_);
    resizeDocks({paletteDock_, propertyDock_}, {220, 300}, Qt::Horizontal);
    resizeDocks({bottomDock_}, {250}, Qt::Vertical);

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
    layoutToolbar_->addAction(paletteDock_->toggleViewAction());
    layoutToolbar_->addAction(propertyDock_->toggleViewAction());
    layoutToolbar_->addAction(bottomDock_->toggleViewAction());
    layoutToolbar_->addAction(resetAction);
    layoutToolbar_->addAction(fullScreenAction);

    QSettings settings;
    restoreGeometry(settings.value("mainWindow/geometry").toByteArray());
    restoreState(settings.value("mainWindow/state").toByteArray());
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
    auto created = NodeFactory::instance().create(typeName);
    if (created.isFail()) {
        appendLog(created.error());
        return;
    }
    const QString id = graph_.addNode(created.value(), findAvailableNodePosition(position));
    selectedNodeId_ = id;
    appendLog(QString("添加节点：%1").arg(id));
    rebuildScene();
    if (auto* item = nodeItems_.value(id)) {
        item->setSelected(true);
    }
}

void MainWindow::selectNode(const QString& nodeId)
{
    selectedNodeId_ = nodeId;
    rebuildProperties();
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
    auto source = graph_.node(selectedNodeId_);
    const auto* sourceRecord = graph_.nodeRecord(selectedNodeId_);
    if (!source || !sourceRecord) {
        appendLog("没有可复制的节点");
        return;
    }

    auto created = NodeFactory::instance().create(source->typeName());
    if (created.isFail()) {
        appendLog(created.error());
        return;
    }
    auto loaded = created.value()->loadParams(source->saveParams());
    if (loaded.isFail()) {
        appendLog(QString("复制节点参数失败：%1").arg(loaded.error()));
        return;
    }

    const QPointF copyPosition = findAvailableNodePosition(sourceRecord->position + QPointF(36, 36));
    const QString newId = graph_.addNode(created.value(), copyPosition);
    selectedNodeId_ = newId;
    appendLog(QString("复制节点：%1 -> %2").arg(sourceRecord->id, newId));
    rebuildScene();
    if (auto* item = nodeItems_.value(newId)) {
        item->setSelected(true);
    }
}

void MainWindow::focusParameterPanel()
{
    if (selectedNodeId_.isEmpty()) {
        return;
    }
    propertyPanel_->setFocus(Qt::OtherFocusReason);
    appendLog(QString("修改参数：%1").arg(selectedNodeId_));
}

void MainWindow::zoomIn()
{
    applyZoomFactor(1.10);
}

void MainWindow::zoomOut()
{
    applyZoomFactor(1.0 / 1.10);
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
}

void MainWindow::requestConnect(const QString& fromNode, const QString& fromPort, const QString& toNode, const QString& toPort)
{
    Edge edge{fromNode, fromPort, toNode, toPort};
    WorkflowValidator validator;
    auto valid = validator.validateEdge(graph_, edge);
    if (valid.isFail()) {
        appendLog(valid.error());
        return;
    }
    graph_.addEdge(edge);
    appendLog(QString("连接：%1.%2 -> %3.%4").arg(fromNode, fromPort, toNode, toPort));
    rebuildEdges();
}

void MainWindow::updateNodePosition(const QString& nodeId, const QPointF& position)
{
    graph_.setNodePosition(nodeId, position);
    rebuildEdges();
}

void MainWindow::rebuildScene()
{
    scene_->clear();
    nodeItems_.clear();
    edgeItems_.clear();
    for (const auto& record : graph_.nodes()) {
        auto* item = new NodeItem(this, record.id, record.node, uiScale_);
        item->setPos(record.position);
        scene_->addItem(item);
        nodeItems_.insert(record.id, item);
    }
    rebuildEdges();
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
        appendLog(QString("删除节点：%1").arg(nodeId));
    }

    rebuildScene();
    rebuildProperties();
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
            auto* w = new QSpinBox;
            w->setRange(int(p.min), int(p.max));
            w->setValue(value.toInt());
            connect(w, &QSpinBox::valueChanged, this, [this, name = p.name](int v) {
                if (auto node = graph_.node(selectedNodeId_)) node->setParameter(name, v);
            });
            editor = w;
        } else if (p.type == ParameterType::Double) {
            auto* w = new QDoubleSpinBox;
            w->setRange(p.min, p.max);
            w->setSingleStep(0.05);
            w->setValue(value.toDouble());
            connect(w, &QDoubleSpinBox::valueChanged, this, [this, name = p.name](double v) {
                if (auto node = graph_.node(selectedNodeId_)) node->setParameter(name, v);
            });
            editor = w;
        } else if (p.type == ParameterType::Boolean) {
            auto* w = new QCheckBox;
            w->setChecked(value.toBool());
            connect(w, &QCheckBox::toggled, this, [this, name = p.name](bool v) {
                if (auto node = graph_.node(selectedNodeId_)) node->setParameter(name, v);
            });
            editor = w;
        } else if (p.type == ParameterType::Choice) {
            auto* w = new QComboBox;
            w->addItems(p.options);
            w->setCurrentText(value.toString());
            connect(w, &QComboBox::currentTextChanged, this, [this, name = p.name](const QString& v) {
                if (auto node = graph_.node(selectedNodeId_)) node->setParameter(name, v);
            });
            editor = w;
        } else {
            auto* container = new QWidget;
            auto* layout = new QHBoxLayout(container);
            layout->setContentsMargins(0, 0, 0, 0);
            auto* edit = new QLineEdit(value.toString());
            layout->addWidget(edit);
            if (p.type == ParameterType::FileOpen || p.type == ParameterType::FileSave || p.type == ParameterType::Color) {
                auto* button = new QPushButton("...");
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
                        if (auto node = graph_.node(selectedNodeId_)) node->setParameter(p.name, v);
                    }
                });
            }
            connect(edit, &QLineEdit::editingFinished, this, [this, edit, name = p.name] {
                if (auto node = graph_.node(selectedNodeId_)) node->setParameter(name, edit->text());
            });
            editor = container;
        }
        propertyLayout_->addRow(p.displayName, editor);
    }
}

void MainWindow::appendLog(const QString& message)
{
    log_->append(message);
}

void MainWindow::runWorkflow()
{
    ExecutionEngine engine;
    auto result = engine.execute(graph_);
    if (result.isFail()) {
        appendLog(QString("执行失败：%1").arg(result.error()));
        QMessageBox::warning(this, "执行失败", result.error());
        return;
    }
    lastResult_ = result.value();
    for (const QString& line : lastResult_.log) appendLog(line);
    appendLog("执行完成");
    updatePreviewForSelection();
}

void MainWindow::newWorkflow()
{
    graph_.clear();
    currentFile_.clear();
    lastResult_ = {};
    selectedNodeId_.clear();
    rebuildScene();
    rebuildProperties();
    appendLog("新建 workflow");
}

void MainWindow::openWorkflow()
{
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
    lastResult_ = {};
    rebuildScene();
    appendLog(QString("已打开：%1").arg(path));
}

bool MainWindow::ensureSavePath()
{
    if (!currentFile_.isEmpty()) return true;
    currentFile_ = QFileDialog::getSaveFileName(this, "保存 workflow", "workflow.json", "Workflow (*.json);;All files (*)");
    return !currentFile_.isEmpty();
}

void MainWindow::removeEdgeByIndex(int edgeIndex)
{
    auto removed = graph_.removeEdge(edgeIndex);
    if (removed.isFail()) {
        appendLog(removed.error());
        return;
    }
    appendLog(QString("删除连线：%1").arg(edgeIndex));
    rebuildEdges();
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

void MainWindow::saveWorkflow()
{
    if (!ensureSavePath()) return;
    WorkflowSerializer serializer;
    auto saved = serializer.saveFile(graph_, currentFile_);
    if (saved.isFail()) {
        QMessageBox::warning(this, "保存失败", saved.error());
        return;
    }
    appendLog(QString("已保存：%1").arg(currentFile_));
}

void MainWindow::saveWorkflowAs()
{
    currentFile_.clear();
    saveWorkflow();
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
    addDockWidget(Qt::BottomDockWidgetArea, bottomDock_);
    paletteDock_->show();
    propertyDock_->show();
    bottomDock_->show();
    const auto metrics = AppTheme::metrics(uiScale_);
    resizeDocks({paletteDock_, propertyDock_}, {metrics.paletteMinWidth, metrics.propertyMinWidth}, Qt::Horizontal);
    resizeDocks({bottomDock_}, {metrics.previewMinHeight + metrics.logMaxHeight}, Qt::Vertical);
}

void MainWindow::toggleFullScreenMode()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
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

    const auto metrics = AppTheme::metrics(uiScale_);
    if (mainToolbar_) {
        mainToolbar_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        mainToolbar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    if (layoutToolbar_) {
        layoutToolbar_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        layoutToolbar_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }
    if (viewToolbar_) {
        viewToolbar_->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
        viewToolbar_->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }

    if (canvasZoomOverlay_) {
        canvasZoomOverlay_->setFixedSize(metrics.canvasZoomOverlayW, metrics.canvasZoomOverlayH);
        canvasZoomOverlay_->setGraphicsEffect(AppTheme::makeShadow(canvasZoomOverlay_, uiScale_, 40));
    }
    if (canvasZoomInButton_) {
        canvasZoomInButton_->setFixedSize(metrics.canvasZoomButtonW, metrics.canvasZoomButtonH);
    }
    if (canvasZoomOutButton_) {
        canvasZoomOutButton_->setFixedSize(metrics.canvasZoomButtonW, metrics.canvasZoomButtonH);
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
    QSettings settings;
    settings.setValue("mainWindow/geometry", saveGeometry());
    settings.setValue("mainWindow/state", saveState());
    settings.setValue("mainWindow/uiScale", uiScale_);
    QMainWindow::closeEvent(event);
}
