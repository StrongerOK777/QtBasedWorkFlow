#include "gui/MainWindow.h"

#include "core/NodeLabel.h"
#include "gui/AppIcon.h"
#include "gui/AppTheme.h"
#include "gui/NativeWindowChrome.h"
#include "gui/WorkbenchHostWidget.h"
#include "gui/WorkbenchModels.h"
#include "gui/WorkflowCanvas.h"
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
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileInfo>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
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
#include <QJsonObject>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QSet>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSlider>
#include <QSizePolicy>
#include <QSplitter>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTabBar>
#include <QTabWidget>
#include <QTextCursor>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QToolTip>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWindow>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

namespace {

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
    } else if (name == "nodes") {
        painter.drawRect(QRectF(11, 12, 17, 15));
        painter.drawRect(QRectF(36, 12, 17, 15));
        painter.drawRect(QRectF(23, 38, 18, 15));
        painter.drawLine(QPointF(20, 27), QPointF(29, 38));
        painter.drawLine(QPointF(45, 27), QPointF(35, 38));
    } else if (name == "workflow") {
        painter.drawRect(QRectF(13, 12, 16, 13));
        painter.drawRect(QRectF(35, 26, 16, 13));
        painter.drawRect(QRectF(13, 41, 16, 13));
        painter.drawLine(QPointF(29, 18), QPointF(35, 31));
        painter.drawLine(QPointF(35, 35), QPointF(29, 47));
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

void suppressMenuIcons(QMenu* menu)
{
    if (!menu) {
        return;
    }
    for (auto* action : menu->actions()) {
        if (!action) {
            continue;
        }
        action->setIconVisibleInMenu(false);
        if (auto* subMenu = action->menu()) {
            suppressMenuIcons(subMenu);
        }
    }
}

QString workflowDataDir(const QString& child)
{
    const QString root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(root.isEmpty() ? QDir::homePath() + "/.imagenodeeditor" : root);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    if (!child.isEmpty() && !dir.exists(child)) {
        dir.mkpath(child);
    }
    return child.isEmpty() ? dir.absolutePath() : dir.absoluteFilePath(child);
}

QString workflowSnapshotPath(const QString& group, const QString& id)
{
    return QDir(workflowDataDir(group)).absoluteFilePath(id + ".json");
}

Result<WorkflowGraph> builtInTemplateGraph(const QString& id)
{
    WorkflowGraph graph;
    auto& factory = NodeFactory::instance();
    auto add = [&](const QString& nodeId, const QString& typeName, const QPointF& pos, const QJsonObject& params = {}) -> Status {
        auto created = factory.create(typeName);
        if (created.isFail()) {
            return Status::fail(created.error());
        }
        if (!params.isEmpty()) {
            auto loaded = created.value()->loadParams(params);
            if (loaded.isFail()) {
                return loaded;
            }
        }
        return graph.addNodeWithId(nodeId, created.value(), pos);
    };

    if (id == "builtin:gray-preview") {
        auto status = add("ImageInput_1", "ImageInput", {-320, -80});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        status = add("Grayscale_2", "Grayscale", {-40, -80});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        status = add("Preview_3", "Preview", {240, -80});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        graph.addEdge({"ImageInput_1", "output", "Grayscale_2", "image"});
        graph.addEdge({"Grayscale_2", "output", "Preview_3", "image"});
        return Result<WorkflowGraph>::ok(graph);
    }
    if (id == "builtin:resize-export") {
        auto status = add("ImageInput_1", "ImageInput", {-340, -80});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        status = add("Resize_2", "Resize", {-40, -80});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        status = add("ImageOutput_3", "ImageOutput", {270, -80});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        graph.addEdge({"ImageInput_1", "output", "Resize_2", "image"});
        graph.addEdge({"Resize_2", "output", "ImageOutput_3", "image"});
        return Result<WorkflowGraph>::ok(graph);
    }
    if (id == "builtin:text-preview") {
        auto status = add("ImageInput_1", "ImageInput", {-340, -80});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        status = add("TextOverlay_2", "TextOverlay", {-40, -80});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        status = add("Preview_3", "Preview", {270, -80});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        graph.addEdge({"ImageInput_1", "output", "TextOverlay_2", "image"});
        graph.addEdge({"TextOverlay_2", "output", "Preview_3", "image"});
        return Result<WorkflowGraph>::ok(graph);
    }
    if (id == "builtin:blend-preview") {
        auto status = add("ImageInput_A", "ImageInput", {-360, -150});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        status = add("ImageInput_B", "ImageInput", {-360, 30});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        status = add("Blend_3", "Blend", {-40, -60});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        status = add("Preview_4", "Preview", {270, -60});
        if (status.isFail()) return Result<WorkflowGraph>::fail(status.error());
        graph.addEdge({"ImageInput_A", "output", "Blend_3", "first"});
        graph.addEdge({"ImageInput_B", "output", "Blend_3", "second"});
        graph.addEdge({"Blend_3", "output", "Preview_4", "image"});
        return Result<WorkflowGraph>::ok(graph);
    }
    return Result<WorkflowGraph>::fail("未知内置模板");
}

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

        search_ = new QLineEdit;
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

    QLineEdit* search_ = nullptr;
    QListWidget* list_ = nullptr;
    QVector<NodeDescriptor> descriptors_;
    QString selectedType_;
};

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
        input_ = new QLineEdit;
        input_->setPlaceholderText("输入命令后按 Enter");
        auto* runButton = new QPushButton("运行");
        auto* clearButton = new QPushButton("清空");
        auto* restartButton = new QPushButton("重启");
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
    : QMainWindow(parent)
{
    setWindowIcon(AppIcon::makeAppIcon());
    NodeFactory::instance().registerBuiltins();
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
    wheelZoomStep_ = std::clamp(settings.value("mainWindow/wheelZoomStep", 1.04).toDouble(), 1.015, 1.08);
    AppTheme::setThemePreference(AppTheme::ThemePreference::Dark);
    settings.setValue("mainWindow/theme", "dark");
    if (auto* app = qobject_cast<QApplication*>(QApplication::instance())) {
        AppTheme::apply(*app, uiScale_);
    }
    createActions();
    createLayout();
    NativeWindowChrome::configure(this);
    initializeWorkbook();
    rebuildNodeMenus();
    applyUiScale();
}

MainWindow::~MainWindow()
{
    ++executionGeneration_;
    ++livePreviewGeneration_;
    if (livePreviewTimer_) {
        livePreviewTimer_->stop();
    }
    if (runAnimationTimer_) {
        runAnimationTimer_->stop();
    }
    // 先等后台执行线程结束，避免它们在对象销毁后回调 this。
    const auto threads = workerThreads_;
    for (auto* thread : threads) {
        if (thread && thread->isRunning()) {
            thread->wait();
        }
    }
    // 在 bridge / 各 Model（均 parent 给 this，会先于 host 析构）被销毁前，
    // 先拆除 QML 表面，避免 QML 绑定访问悬空对象导致退出报错。
    if (workbenchHost_) {
        workbenchHost_->teardownSurfaces();
    }
    hideWorkbenchTooltip();
    delete tooltipPopup_;
    tooltipPopup_ = nullptr;
    tooltipLabel_ = nullptr;
}

void MainWindow::createActions()
{
    // QMenuBar is intentionally kept: macOS renders it in the system menu bar,
    // while Windows/Linux render it inside the window. The same QAction objects
    // are also exposed through the custom in-window header and toolbars.
    fileMenu_ = menuBar()->addMenu("文件");
    workbenchCommands_ = new WorkbenchCommandRegistry(this);
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
    workbenchCommands_->addAction("file.new", "文件", newAction);
    workbenchCommands_->addAction("file.open", "文件", openAction);
    workbenchCommands_->addAction("file.save", "文件", saveAction);
    workbenchCommands_->addAction("file.saveAs", "文件", saveAsAction);
    workbenchCommands_->addAction("file.exportWorkflow", "文件", exportWorkflowCopyAction_);
    workbenchCommands_->addAction("file.exportPreview", "文件", exportPreviewAction_);
    workbenchCommands_->addAction("file.exportCanvas", "文件", exportCanvasAction);
    workbenchCommands_->addAction("workflow.run", "执行", runAction_);

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
    workbenchCommands_->addAction("macro.back", "导航", returnToParentAction_);
    workbenchCommands_->addAction("macro.forward", "导航", forwardToChildAction_);

    editMenu_ = menuBar()->addMenu("编辑");
    auto* undoAction = undoStack_->createUndoAction(this, "撤销");
    undoAction->setShortcuts(QKeySequence::Undo);
    auto* redoAction = undoStack_->createRedoAction(this, "重做");
    redoAction->setShortcuts(QKeySequence::Redo);
    editMenu_->addAction(undoAction);
    editMenu_->addAction(redoAction);
    workbenchCommands_->addAction("edit.undo", "编辑", undoAction);
    workbenchCommands_->addAction("edit.redo", "编辑", redoAction);
    editMenu_->addSeparator();
    auto* macroAction = editMenu_->addAction("封装为宏节点");
    connect(macroAction, &QAction::triggered, this, [this] { encapsulateSelectionAsMacro(); });
    workbenchCommands_->addAction("macro.encapsulate", "节点", macroAction);
    auto* quickNodeAction = new QAction("快捷添加节点", this);
    const QKeySequence quickNodeShortcut(Qt::CTRL | Qt::Key_K);
    quickNodeAction->setShortcut(quickNodeShortcut);
    quickNodeAction->setToolTip(QString("快捷添加节点（%1；画布 Tab）").arg(quickNodeShortcut.toString(QKeySequence::NativeText)));
    quickNodeAction->setStatusTip("搜索并在当前画布位置添加节点");
    connect(quickNodeAction, &QAction::triggered, this, [this] { showQuickNodePalette(); });
    editMenu_->addAction(quickNodeAction);
    workbenchCommands_->addAction("node.quickAdd", "节点", quickNodeAction);

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
    auto* commandPaletteAction = addViewAction("命令面板", "more", {QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P)}, [this] {
        if (workbenchBridge_) {
            workbenchBridge_->showQuickAccess();
        }
    });
    workbenchCommands_->addAction("view.zoomIn", "视图", zoomInAction);
    workbenchCommands_->addAction("view.zoomOut", "视图", zoomOutAction);
    workbenchCommands_->addAction("view.resetScale", "视图", resetScaleAction);
    workbenchCommands_->addAction("workbench.quickAccess", "视图", commandPaletteAction);
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
    workbenchCommands_->addAction("settings.open", "设置", settingsAction_);

    installDelayedTooltips(mainToolbar_);
    suppressMenuIcons(fileMenu_);
    suppressMenuIcons(editMenu_);
    suppressMenuIcons(nodeOperationMenu_);
    suppressMenuIcons(viewMenu_);
    suppressMenuIcons(settingsMenu_);
}

void MainWindow::createLayout()
{
    WorkflowCanvas::Callbacks canvasCallbacks;
    canvasCallbacks.edgeAdded = [this](const Edge& edge) { return acceptCanvasEdge(edge); };
    canvasCallbacks.edgeRemoved = [this](const Edge& edge) { removeCanvasEdge(edge); };
    canvasCallbacks.nodeMoved = [this](const QString& nodeId, const QPointF& before, const QPointF& after) {
        commitNodeMove(nodeId, before, after);
        updateMiniMap();
    };
    canvasCallbacks.selectionChanged = [this](const QString& nodeId) {
        selectedNodeId_ = nodeId;
        if (workflowList_) {
            workflowList_->clearSelection();
            for (int row = 0; row < workflowList_->count(); ++row) {
                auto* outlineItem = workflowList_->item(row);
                if (outlineItem && outlineItem->data(Qt::UserRole).toString() == selectedNodeId_) {
                    workflowList_->setCurrentItem(outlineItem);
                    break;
                }
            }
        }
        rebuildProperties();
        updatePreviewForSelection();
        if (workbenchBridge_) {
            const auto node = graph_.node(selectedNodeId_);
            workbenchBridge_->setSelectedNodeText(node ? node->displayName() : QString("未选择节点"));
        }
    };
    canvasCallbacks.nodeContextMenu = [this](const QString& nodeId, const QPoint& screenPos) {
        if (!nodeId.isEmpty()) {
            showNodeContextMenu(nodeId, screenPos);
        }
    };
    canvasCallbacks.nodeDoubleClicked = [this](const QString& nodeId) { enterMacroNode(nodeId); };
    canvasCallbacks.deleteSelection = [this] { removeSelectedItems(); };
    canvasCallbacks.copySelection = [this] { copySelectedNode(); };
    canvasCallbacks.quickPalette = [this](const QPointF& pos) { showQuickNodePaletteAt(pos); };
    canvasCallbacks.sceneContextMenu = [this](const QPointF& pos) { return createCanvasContextMenu(pos); };
    canvasCallbacks.nodeDropped = [this](const QString& typeName, const QPointF& pos) {
        addNodeFromType(typeName, pos);
    };
    canvasCallbacks.wheelZoomRequested = [this](double wheelSteps) {
        applyZoomFactor(std::pow(wheelZoomStep_, wheelSteps));
    };
    canvasCallbacks.parameterChanged = [this](const QString& nodeId, const QString& name, const QVariant& value) {
        setNodeParameterForNode(nodeId, name, value);
    };
    canvas_ = std::make_unique<WorkflowCanvas>(this, uiScale_, std::move(canvasCallbacks));
    view_ = canvas_->view();
    view_->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    view_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    view_->setResizeAnchor(QGraphicsView::AnchorViewCenter);

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
        connect(clearButton, &QToolButton::clicked, this, [this, list] {
            list->clear();
            if (list == problemLog_ && problemModel_) {
                problemModel_->clear();
            }
        });
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
    bottomTabs_->tabBar()->setDrawBase(false);
    bottomTabs_->setStyleSheet(R"(
        QTabWidget#bottomTabs::pane {
            border: 0px;
            background: #1e1e1e;
        }
        QTabWidget#bottomTabs QTabBar::tab {
            min-height: 26px;
            padding: 0px 10px;
            margin: 0px;
            border: 0px;
            border-right: 1px solid #2d2d2d;
            background: #1e1e1e;
            color: #969696;
        }
        QTabWidget#bottomTabs QTabBar::tab:selected {
            background: #252526;
            color: #ffffff;
            border-top: 1px solid #3794ff;
        }
        QTabWidget#bottomTabs QTabBar::tab:hover {
            background: #2a2d2e;
            color: #cccccc;
        }
    )");
    bottomTabs_->addTab(terminalPanel_, "终端");
    bottomTabs_->addTab(makeLogPage(problemLog_, "问题"), "问题");
    bottomTabs_->addTab(makeLogPage(log_, "输出"), "输出");

    layoutMenu_ = menuBar()->addMenu("布局");
    previewToggleAction_ = new QAction(lineIcon("dockRight"), "显示预览", this);
    previewToggleAction_->setCheckable(true);
    previewToggleAction_->setChecked(true);
    bottomToggleAction_ = new QAction(lineIcon("dockBottom"), "显示底部面板", this);
    bottomToggleAction_->setCheckable(true);
    bottomToggleAction_->setChecked(true);
    layoutMenu_->addAction(previewToggleAction_);
    layoutMenu_->addAction(bottomToggleAction_);
    layoutMenu_->addSeparator();
    auto* resetAction = layoutMenu_->addAction("重置布局");
    connect(resetAction, &QAction::triggered, this, [this] { resetWorkbenchLayout(); });
    auto* autoLayoutAction = layoutMenu_->addAction("整理画布");
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
    autoLayoutAction->setToolTip("按拓扑顺序整理当前画布");
    fullScreenAction->setIcon(lineIcon("fullscreen"));
    fullScreenAction->setData("fullscreen");
    fullScreenAction->setToolTip("全屏切换");
    workbenchCommands_->addAction("workbench.preview", "布局", previewToggleAction_);
    workbenchCommands_->addAction("workbench.panel", "布局", bottomToggleAction_);
    workbenchCommands_->addAction("workbench.resetLayout", "布局", resetAction);
    workbenchCommands_->addAction("workflow.autoLayout", "布局", autoLayoutAction);
    workbenchCommands_->addAction("view.fullscreen", "布局", fullScreenAction);
    suppressMenuIcons(layoutMenu_);

    bottomPanel_ = new QWidget;
    bottomPanel_->setObjectName("bottomPanel");
    bottomPanel_->setAttribute(Qt::WA_StyledBackground, true);
    auto* bottomLayout = new QVBoxLayout(bottomPanel_);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->addWidget(bottomTabs_);

    nodeCatalogModel_ = new NodeCatalogModel(this);
    workflowOutlineModel_ = new WorkflowOutlineModel(this);
    problemModel_ = new ProblemModel(this);
    workflowTemplateModel_ = new WorkflowTemplateModel(this);
    workflowCheckpointModel_ = new WorkflowCheckpointModel(this);
    quickAccessModel_ = new QuickAccessModel(workbenchCommands_, nodeCatalogModel_, workflowOutlineModel_, problemModel_, this);
    workbenchBridge_ = new WorkbenchBridge(workbenchCommands_, quickAccessModel_, this);
    workbenchBridge_->setPreviewVisible(previewToggleAction_->isChecked());
    workbenchBridge_->setPanelVisible(bottomToggleAction_->isChecked());
    workbenchHost_ = new WorkbenchHostWidget(workbenchBridge_,
                                             workbenchCommands_,
                                             nodeCatalogModel_,
                                             workflowOutlineModel_,
                                             problemModel_,
                                             workflowTemplateModel_,
                                             workflowCheckpointModel_,
                                             quickAccessModel_,
                                             viewContainer,
                                             preview_,
                                             bottomPanel_,
                                             uiScale_,
                                             this);
    setCentralWidget(workbenchHost_);
    primarySidebar_ = workbenchHost_->primarySidebar();
    previewSidebar_ = workbenchHost_->previewSidebar();
    workbenchSplitter_ = workbenchHost_->workbenchSplitter();
    editorSplitter_ = workbenchHost_->editorSplitter();

    connect(previewToggleAction_, &QAction::toggled, previewSidebar_, &QWidget::setVisible);
    connect(bottomToggleAction_, &QAction::toggled, bottomPanel_, &QWidget::setVisible);
    connect(previewToggleAction_, &QAction::toggled, workbenchBridge_, &WorkbenchBridge::setPreviewVisible);
    connect(bottomToggleAction_, &QAction::toggled, workbenchBridge_, &WorkbenchBridge::setPanelVisible);
    connect(workbenchBridge_, &WorkbenchBridge::previewVisibilityRequested, previewToggleAction_, &QAction::setChecked);
    connect(workbenchBridge_, &WorkbenchBridge::panelVisibilityRequested, bottomToggleAction_, &QAction::setChecked);
    connect(workbenchBridge_, &WorkbenchBridge::nodeCreationRequested, this, [this](const QString& typeName) {
        addNodeFromMenu(typeName);
    });
    connect(workbenchBridge_, &WorkbenchBridge::nodeFocusRequested, this, [this](const QString& nodeId) {
        selectNode(nodeId);
        highlightNodeBriefly(nodeId);
    });
    connect(workbenchBridge_, &WorkbenchBridge::recentWorkflowRequested, this, [this](const QString& path) {
        openWorkflowPath(path, true);
    });
    connect(workbenchBridge_, &WorkbenchBridge::workflowTemplateSaveRequested,
            this, &MainWindow::saveCurrentWorkflowAsTemplate);
    connect(workbenchBridge_, &WorkbenchBridge::workflowTemplateApplyRequested,
            this, &MainWindow::applyWorkflowTemplate);
    connect(workbenchBridge_, &WorkbenchBridge::checkpointCreateRequested,
            this, &MainWindow::createWorkflowCheckpoint);
    connect(workbenchBridge_, &WorkbenchBridge::checkpointRestoreRequested,
            this, &MainWindow::restoreWorkflowCheckpoint);
    connect(workbenchBridge_, &WorkbenchBridge::checkpointBranchRequested,
            this, &MainWindow::branchFromWorkflowCheckpoint);
    connect(workbenchBridge_, &WorkbenchBridge::windowMoveRequested, this, [this] {
        if (auto* handle = windowHandle()) {
            handle->startSystemMove();
        }
    });
    connect(workbenchBridge_, &WorkbenchBridge::windowMinimizeRequested, this, [this] { showMinimized(); });
    connect(workbenchBridge_, &WorkbenchBridge::windowMaximizeToggleRequested, this, [this] {
        isMaximized() ? showNormal() : showMaximized();
    });
    connect(workbenchBridge_, &WorkbenchBridge::windowCloseRequested, this, [this] { close(); });
    connect(workbenchBridge_, &WorkbenchBridge::tooltipRequested, this, &MainWindow::showWorkbenchTooltip);
    connect(workbenchBridge_, &WorkbenchBridge::tooltipHideRequested, this, &MainWindow::hideWorkbenchTooltip);
    refreshRecentWorkflowModel();
    refreshWorkflowTemplateModel();
    refreshWorkflowCheckpointModel();

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
    const QByteArray workbenchState = settings.value("mainWindow/workbenchSplitter").toByteArray();
    const QByteArray editorState = settings.value("mainWindow/editorSplitter").toByteArray();
    if (workbenchState.isEmpty() || editorState.isEmpty() ||
        !workbenchSplitter_->restoreState(workbenchState) || !editorSplitter_->restoreState(editorState)) {
        resetWorkbenchLayout();
    }
    menuBar()->show();
    if (mainToolbar_) {
        mainToolbar_->hide();
    }
    if (headerToolbar_) {
        headerToolbar_->hide();
    }
    if (workbookToolbar_) {
        workbookToolbar_->hide();
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
        suppressMenuIcons(nodeOperationMenu_);
    }
    if (commandCenterMenu_) {
        commandCenterMenu_->clear();
        auto* fileMenu = commandCenterMenu_->addMenu("文件");
        fileMenu->addAction("新建", this, [this] { newWorkflow(); });
        fileMenu->addAction("打开", this, [this] { openWorkflow(); });
        fileMenu->addAction("保存", this, [this] { saveWorkflow(); });
        fileMenu->addAction("另存为", this, [this] { saveWorkflowAs(); });
        auto* runMenu = commandCenterMenu_->addMenu("执行");
        runMenu->addAction("执行工作流", this, [this] { runWorkflow(); });
        runMenu->addAction("整理画布", this, [this] { autoLayoutWorkflow(); });
        auto* nodesMenu = commandCenterMenu_->addMenu("节点操作");
        populateNodeMenu(nodesMenu);
        auto* viewMenu = commandCenterMenu_->addMenu("视图");
        viewMenu->addAction("快捷添加节点", this, [this] { showQuickNodePalette(); });
        viewMenu->addAction("界面放大", this, [this] { increaseUiScale(); });
        viewMenu->addAction("界面缩小", this, [this] { decreaseUiScale(); });
        viewMenu->addAction("重置界面大小", this, [this] { resetUiScale(); });
        commandCenterMenu_->addSeparator();
        commandCenterMenu_->addAction("打开设置", this, [this] { showSettingsDialog(); });
        suppressMenuIcons(commandCenterMenu_);
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
    if (rejectGraphReplacementWhileBusy("添加节点")) {
        return;
    }
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
    if (canvas_) {
        canvas_->selectNode(nodeId);
    }
    if (workbenchBridge_) {
        const auto node = graph_.node(selectedNodeId_);
        workbenchBridge_->setSelectedNodeText(node ? node->displayName() : QString("未选择节点"));
    }
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
    pushGraphEditCommand(QString("移动节点 %1").arg(nodeLabel(nodeId)), before, nodeId, after, nodeId, QString("move:%1").arg(nodeId));
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
    if (rejectGraphReplacementWhileBusy("复制节点")) {
        return;
    }
    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;

    QStringList sourceIds = canvas_ ? canvas_->selectedNodeIds() : QStringList{};
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
    if (rejectGraphReplacementWhileBusy("封装宏节点")) {
        return;
    }
    QStringList selectedIds = canvas_ ? canvas_->selectedNodeIds() : QStringList{};
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
    if (rejectGraphReplacementWhileBusy("整理画布")) {
        return;
    }
    if (graph_.nodes().isEmpty()) {
        appendLog("当前图没有可布局的节点");
        return;
    }

    QSet<QString> occupiedInputs;
    for (const auto& edge : graph_.edges()) {
        auto from = graph_.node(edge.fromNode);
        auto to = graph_.node(edge.toNode);
        if (!from || !to) {
            appendProblem("整理画布失败：连线引用了不存在的节点");
            QMessageBox::warning(this, "整理画布失败", "连线引用了不存在的节点，布局未修改。");
            return;
        }
        PortInfo fromPort;
        PortInfo toPort;
        if (!findPortInfo(from, edge.fromPort, PortDirection::Output, &fromPort) ||
            !findPortInfo(to, edge.toPort, PortDirection::Input, &toPort)) {
            appendProblem(QString("整理画布失败：连线端口不存在 %1.%2 -> %3.%4")
                              .arg(edge.fromNode, edge.fromPort, edge.toNode, edge.toPort));
            QMessageBox::warning(this, "整理画布失败",
                                 QString("连线端口不存在：%1.%2 -> %3.%4，布局未修改。")
                                     .arg(edge.fromNode, edge.fromPort, edge.toNode, edge.toPort));
            return;
        }
        if (!portTypesCompatible(fromPort.type, toPort.type)) {
            appendProblem(QString("整理画布失败：端口类型不兼容 %1.%2 -> %3.%4")
                              .arg(edge.fromNode, edge.fromPort, edge.toNode, edge.toPort));
            QMessageBox::warning(this, "整理画布失败",
                                 QString("端口类型不兼容：%1.%2 -> %3.%4，布局未修改。")
                                     .arg(edge.fromNode, edge.fromPort, edge.toNode, edge.toPort));
            return;
        }
        const QString inputKey = edge.toNode + "." + edge.toPort;
        if (!toPort.allowMultipleConnections && occupiedInputs.contains(inputKey)) {
            appendProblem(QString("整理画布失败：输入端口被多次连接 %1").arg(inputKey));
            QMessageBox::warning(this, "整理画布失败",
                                 QString("输入端口被多次连接：%1，布局未修改。").arg(inputKey));
            return;
        }
        occupiedInputs.insert(inputKey);
    }

    WorkflowValidator validator;
    auto orderResult = validator.topologicalOrder(graph_);
    if (orderResult.isFail()) {
        appendProblem(QString("整理画布失败：%1").arg(orderResult.error()));
        QMessageBox::warning(this, "整理画布失败", orderResult.error() + "，布局未修改。");
        return;
    }

    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    const QStringList order = orderResult.value();
    QMap<QString, QStringList> incoming;
    QMap<QString, QStringList> outgoing;
    QMap<QString, int> topoIndex;
    for (int index = 0; index < order.size(); ++index) {
        topoIndex.insert(order.at(index), index);
    }
    for (const auto& edge : graph_.edges()) {
        outgoing[edge.fromNode].append(edge.toNode);
        incoming[edge.toNode].append(edge.fromNode);
    }

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
    for (auto it = layers.begin(); it != layers.end(); ++it) {
        QStringList ids = it.value();
        std::stable_sort(ids.begin(), ids.end(), [this, &topoIndex](const QString& left, const QString& right) {
            const auto* leftRecord = graph_.nodeRecord(left);
            const auto* rightRecord = graph_.nodeRecord(right);
            const qreal leftY = leftRecord ? leftRecord->position.y() : 0.0;
            const qreal rightY = rightRecord ? rightRecord->position.y() : 0.0;
            if (!qFuzzyCompare(leftY + 1.0, rightY + 1.0)) {
                return leftY < rightY;
            }
            return topoIndex.value(left, 0) < topoIndex.value(right, 0);
        });
        it.value() = ids;
    }

    const QList<int> layerKeys = layers.keys();
    auto rowIndex = [&layers](const QString& id) {
        for (auto it = layers.cbegin(); it != layers.cend(); ++it) {
            const int index = it.value().indexOf(id);
            if (index >= 0) {
                return index;
            }
        }
        return 0;
    };
    auto barycenter = [&](const QString& id, bool fromInputs) {
        const QStringList neighbors = fromInputs ? incoming.value(id) : outgoing.value(id);
        double sum = 0.0;
        int count = 0;
        for (const QString& neighbor : neighbors) {
            if (!layerByNode.contains(neighbor)) {
                continue;
            }
            sum += rowIndex(neighbor);
            ++count;
        }
        return count > 0 ? sum / count : double(rowIndex(id));
    };
    auto sortLayerByBarycenter = [&](int layer, bool fromInputs) {
        QStringList ids = layers.value(layer);
        std::stable_sort(ids.begin(), ids.end(), [&](const QString& left, const QString& right) {
            const double leftScore = barycenter(left, fromInputs);
            const double rightScore = barycenter(right, fromInputs);
            if (std::abs(leftScore - rightScore) > 0.001) {
                return leftScore < rightScore;
            }
            return topoIndex.value(left, 0) < topoIndex.value(right, 0);
        });
        layers[layer] = ids;
    };
    for (int pass = 0; pass < 4; ++pass) {
        for (int i = 1; i < layerKeys.size(); ++i) {
            sortLayerByBarycenter(layerKeys.at(i), true);
        }
        for (int i = layerKeys.size() - 2; i >= 0; --i) {
            sortLayerByBarycenter(layerKeys.at(i), false);
        }
    }

    QPointF origin(0, 0);
    bool haveOrigin = false;
    qreal minY = 0.0;
    qreal maxY = 0.0;
    for (const auto& record : graph_.nodes()) {
        if (!haveOrigin) {
            origin = record.position;
            minY = record.position.y();
            maxY = record.position.y();
            haveOrigin = true;
        } else {
            origin.setX(std::min(origin.x(), record.position.x()));
            origin.setY(std::min(origin.y(), record.position.y()));
            minY = std::min(minY, record.position.y());
            maxY = std::max(maxY, record.position.y());
        }
    }

    const auto metrics = AppTheme::nodeMetrics(uiScale_);
    const qreal xSpacing = metrics.width + 170.0 * uiScale_;
    const qreal rowGap = std::max<qreal>(metrics.rowHeight * 1.4, 46.0 * uiScale_);
    const qreal graphCenterY = haveOrigin ? (minY + maxY) * 0.5 : 0.0;
    auto estimatedHeight = [&](const QString& id) {
        const auto node = graph_.node(id);
        if (!node) {
            return metrics.headerHeight + metrics.bottomPadding + metrics.rowHeight * 2.0;
        }
        const int portRows = std::max(node->inputPorts().size(), node->outputPorts().size());
        const int parameterRows = node->parameterDefinitions().size();
        const int visibleRows = std::max(2, portRows + std::min(parameterRows, 4));
        return metrics.headerHeight + metrics.bottomPadding + metrics.rowHeight * visibleRows;
    };

    bool changed = false;
    for (auto layerIt = layers.cbegin(); layerIt != layers.cend(); ++layerIt) {
        const QStringList ids = layerIt.value();
        const qreal columnX = origin.x() + layerIt.key() * xSpacing;
        qreal columnHeight = 0.0;
        for (const QString& id : ids) {
            columnHeight += estimatedHeight(id);
        }
        if (!ids.isEmpty()) {
            columnHeight += rowGap * (ids.size() - 1);
        }
        qreal nextY = graphCenterY - columnHeight * 0.5;
        for (const QString& id : ids) {
            const QPointF next(columnX, nextY);
            const auto* record = graph_.nodeRecord(id);
            if (record && (!qFuzzyCompare(record->position.x(), next.x()) || !qFuzzyCompare(record->position.y(), next.y()))) {
                changed = true;
            }
            graph_.setNodePosition(id, next);
            nextY += estimatedHeight(id) + rowGap;
        }
    }

    if (!changed) {
        fitGraphInView();
        appendLog("当前图已接近整理结果，已居中显示");
        return;
    }

    rebuildScene();
    if (auto* item = nodeItems_.value(selectedNodeId_)) {
        item->setSelected(true);
    }
    fitGraphInView();
    appendLog("已整理当前画布");
    pushGraphEditCommand("整理画布", before, selectedBefore, WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
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
    appendLog(QString("进入宏节点子图：%1").arg(nodeLabel(nodeId)), nodeId);
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
    appendLog(QString("返回上级图：%1").arg(nodeLabel(selectedNodeId_)), selectedNodeId_);
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

QMenu* MainWindow::createCanvasContextMenu(const QPointF& scenePosition)
{
    auto* menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->setStyleSheet(R"(
        QMenu {
            background: #252526;
            border: 1px solid #454545;
            color: #cccccc;
            padding: 4px 0px;
        }
        QMenu::item {
            min-height: 26px;
            padding: 4px 28px 4px 24px;
        }
        QMenu::item:selected {
            background: #094771;
            color: #ffffff;
        }
        QMenu::item:disabled {
            color: #6a6a6a;
        }
        QMenu::separator {
            height: 1px;
            background: #3c3c3c;
            margin: 5px 8px;
        }
    )");

    auto* quickAdd = menu->addAction("快捷添加节点");
    quickAdd->setEnabled(!executionBusy_);
    connect(quickAdd, &QAction::triggered, this, [this, scenePosition] {
        showQuickNodePaletteAt(scenePosition);
    });

    auto* layoutAction = menu->addAction("整理画布");
    layoutAction->setEnabled(!executionBusy_);
    connect(layoutAction, &QAction::triggered, this, [this] { autoLayoutWorkflow(); });

    menu->addAction("重置视图（适应全部节点）", this, [this] { fitGraphInView(); });
    menu->addSeparator();

    if (previewToggleAction_) {
        menu->addAction(previewToggleAction_);
    }
    if (bottomToggleAction_) {
        menu->addAction(bottomToggleAction_);
    }
    menu->addSeparator();

    auto* checkpoint = menu->addAction("保存当前进度");
    checkpoint->setEnabled(!executionBusy_);
    connect(checkpoint, &QAction::triggered, this, [this] { createWorkflowCheckpoint(); });

    auto* saveTemplate = menu->addAction("保存当前为模板");
    saveTemplate->setEnabled(!executionBusy_);
    connect(saveTemplate, &QAction::triggered, this, [this] { saveCurrentWorkflowAsTemplate(); });
    return menu;
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
    appendLog(QString("修改参数：%1").arg(nodeLabel(selectedNodeId_)), selectedNodeId_);
}

void MainWindow::zoomIn()
{
    applyZoomFactor(1.06);
}

void MainWindow::zoomOut()
{
    applyZoomFactor(1.0 / 1.06);
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
    if (workbenchBridge_) {
        workbenchBridge_->setZoomText(QString("%1%").arg(int(std::lround(zoomScale_ * 100.0))));
    }
    updateMiniMap();
}

void MainWindow::fitGraphInView()
{
    if (!view_ || !scene_) {
        return;
    }
    QRectF bounds = scene_->itemsBoundingRect();
    if (bounds.isEmpty()) {
        return;
    }
    const qreal pad = 60.0 * uiScale_;
    bounds.adjust(-pad, -pad, pad, pad);
    const QSize viewport = view_->viewport()->size();
    if (viewport.width() <= 0 || viewport.height() <= 0 || bounds.width() <= 0 || bounds.height() <= 0) {
        return;
    }
    const double scale = std::clamp(std::min(viewport.width() / bounds.width(),
                                             viewport.height() / bounds.height()),
                                    0.25, 3.0);
    view_->resetTransform();
    view_->scale(scale, scale);
    zoomScale_ = scale;
    view_->centerOn(bounds.center());
    if (workbenchBridge_) {
        workbenchBridge_->setZoomText(QString("%1%").arg(int(std::lround(zoomScale_ * 100.0))));
    }
    updateMiniMap();
}

void MainWindow::requestConnect(const QString& fromNode, const QString& fromPort, const QString& toNode, const QString& toPort)
{
    acceptCanvasEdge(Edge{fromNode, fromPort, toNode, toPort});
}

bool MainWindow::acceptCanvasEdge(const Edge& edge)
{
    if (rejectGraphReplacementWhileBusy("连接节点")) {
        return false;
    }
    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    WorkflowValidator validator;
    auto valid = validator.validateEdge(graph_, edge);
    if (valid.isFail()) {
        appendLog(valid.error());
        appendProblem(valid.error(), edge.toNode);
        return false;
    }
    graph_.addEdge(edge);
    resetNodeRunStates();
    appendLog(QString("连接：%1.%2 -> %3.%4").arg(edge.fromNode, edge.fromPort, edge.toNode, edge.toPort), edge.toNode);
    rebuildEdges();
    pushGraphEditCommand("添加连线", before, selectedBefore, WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
    return true;
}

void MainWindow::removeCanvasEdge(const Edge& edge)
{
    for (int index = 0; index < graph_.edges().size(); ++index) {
        const Edge current = graph_.edges().at(index);
        if (current.fromNode == edge.fromNode && current.fromPort == edge.fromPort &&
            current.toNode == edge.toNode && current.toPort == edge.toPort) {
            removeEdgeByIndex(index);
            return;
        }
    }
}

void MainWindow::updateNodePosition(const QString& nodeId, const QPointF& position)
{
    graph_.setNodePosition(nodeId, position);
    rebuildEdges();
    updateMiniMap();
}

void MainWindow::rebuildScene()
{
    nodeItems_.clear();
    if (workflowList_) {
        workflowList_->clear();
    }
    if (canvas_) {
        canvas_->setUiScale(uiScale_);
        canvas_->rebuild(graph_, nodeRunStates_, nodeElapsedMs_);
        scene_ = canvas_->scene();
        nodeItems_ = canvas_->nodeItems();
    }
    for (const auto& record : graph_.nodes()) {
        if (workflowList_) {
            const QString displayName = record.node ? record.node->displayName() : QString("未知节点");
            auto* outlineItem = new QListWidgetItem(formatNodeLabel(displayName, record.id));
            outlineItem->setData(Qt::UserRole, record.id);
            outlineItem->setToolTip(formatNodeLabel(displayName, record.id));
            workflowList_->addItem(outlineItem);
        }
    }
    if (workflowOutlineModel_) {
        workflowOutlineModel_->setGraph(graph_);
    }
    rebuildEdges();
    updateMiniMap();
}

void MainWindow::rebuildEdges()
{
    if (scene_) {
        scene_->update();
    }
    updateMiniMap();
}

void MainWindow::removeSelectedItems()
{
    if (rejectGraphReplacementWhileBusy("删除节点或连线")) {
        return;
    }
    QVector<int> edgeIndexes = canvas_ ? canvas_->selectedEdgeIndexes(graph_) : QVector<int>{};
    QStringList nodeIds = canvas_ ? canvas_->selectedNodeIds() : QStringList{};
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
        const QString label = nodeLabel(nodeId);
        graph_.removeNode(nodeId);
        appendLog(QString("删除节点：%1").arg(label), nodeId);
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

QString MainWindow::nodeLabel(const QString& nodeId) const
{
    const auto node = graph_.node(nodeId);
    return formatNodeLabel(node ? node->displayName() : QString(), nodeId);
}

void MainWindow::appendLog(const QString& message, const QString& nodeId)
{
    if (!log_) {
        return;
    }
    auto* item = new QListWidgetItem(message);
    item->setData(Qt::UserRole, nodeId);
    if (!nodeId.isEmpty()) {
        item->setToolTip("双击定位对应节点");
        item->setForeground(QBrush(AppTheme::colors().nodeSelected));
    }
    log_->addItem(item);
    log_->scrollToBottom();
    if (workbenchBridge_) {
        workbenchBridge_->setStatusText(message);
    }
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
        item->setToolTip("双击定位对应节点");
    }
    problemLog_->addItem(item);
    problemLog_->scrollToBottom();
    if (problemModel_) {
        problemModel_->append(message, nodeId);
    }
    if (workbenchBridge_) {
        workbenchBridge_->setStatusText(message);
    }
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
    if (rejectGraphReplacementWhileBusy("修改参数")) {
        return;
    }
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
        if (canvas_) {
            canvas_->setExecutionState(it.key(), NodeExecutionState::NotExecuted);
            canvas_->setElapsedMs(it.key(), -1);
            canvas_->setAnimationPhase(it.key(), 0);
        }
    }
}

void MainWindow::applyNodeRunState(const QString& nodeId, NodeExecutionState state)
{
    if (nodeId.isEmpty()) {
        return;
    }
    nodeRunStates_.insert(nodeId, state);
    if (canvas_) {
        canvas_->setExecutionState(nodeId, state);
    }
}

void MainWindow::handleNodeExecutionEvent(const NodeExecutionSummary& summary)
{
    applyNodeRunState(summary.nodeId, summary.state);
    if (!summary.nodeId.isEmpty() && summary.elapsedMs >= 0) {
        nodeElapsedMs_.insert(summary.nodeId, summary.elapsedMs);
        if (canvas_) {
            canvas_->setElapsedMs(summary.nodeId, summary.elapsedMs);
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
}

void MainWindow::setExecutionBusy(bool busy)
{
    if (executionBusy_ == busy) {
        return;
    }
    executionBusy_ = busy;
    if (runAction_) {
        runAction_->setEnabled(!busy);
    }
    if (workbenchBridge_) {
        workbenchBridge_->setStatusText(busy ? QString("正在后台执行，画布仍可浏览") : QString("就绪"));
    }
}

bool MainWindow::rejectGraphReplacementWhileBusy(const QString& actionName)
{
    if (!executionBusy_) {
        return false;
    }
    const QString message = QString("正在执行工作流，暂不能%1。请等待本次执行完成。").arg(actionName);
    appendLog(message);
    if (workbenchBridge_) {
        workbenchBridge_->setStatusText(message);
    }
    return true;
}

void MainWindow::trackWorkerThread(QThread* thread)
{
    if (!thread) {
        return;
    }
    workerThreads_.append(thread);
    connect(thread, &QThread::finished, this, [this, thread] {
        workerThreads_.removeAll(thread);
        thread->deleteLater();
    });
}

void MainWindow::showWorkbenchTooltip(const QString& text, const QString& placement)
{
    if (text.trimmed().isEmpty()) {
        hideWorkbenchTooltip();
        return;
    }
    if (!tooltipPopup_) {
        tooltipPopup_ = new QFrame(nullptr,
                                   Qt::ToolTip | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint |
                                       Qt::WindowStaysOnTopHint);
        tooltipPopup_->setObjectName("workbenchTooltip");
        tooltipPopup_->setAttribute(Qt::WA_ShowWithoutActivating);
        tooltipPopup_->setAttribute(Qt::WA_TransparentForMouseEvents);
        tooltipPopup_->setStyleSheet(R"(
            QFrame#workbenchTooltip {
                background: #252526;
                border: 1px solid #454545;
                color: #cccccc;
            }
            QLabel {
                color: #cccccc;
                font-size: 12px;
                padding: 7px 9px;
            }
        )");
        auto* shadow = new QGraphicsDropShadowEffect(tooltipPopup_);
        shadow->setBlurRadius(16);
        shadow->setOffset(0, 3);
        shadow->setColor(QColor(0, 0, 0, 140));
        tooltipPopup_->setGraphicsEffect(shadow);
        auto* layout = new QVBoxLayout(tooltipPopup_);
        layout->setContentsMargins(0, 0, 0, 0);
        tooltipLabel_ = new QLabel(tooltipPopup_);
        tooltipLabel_->setWordWrap(true);
        tooltipLabel_->setMaximumWidth(AppTheme::px(280, uiScale_));
        layout->addWidget(tooltipLabel_);
    }

    tooltipLabel_->setText(text);
    tooltipLabel_->setMaximumWidth(AppTheme::px(280, uiScale_));
    tooltipPopup_->adjustSize();

    QPoint cursor = QCursor::pos();
    QPoint pos = cursor + QPoint(AppTheme::px(14, uiScale_), AppTheme::px(16, uiScale_));
    const QSize size = tooltipPopup_->sizeHint();
    if (placement == "right") {
        pos = cursor + QPoint(AppTheme::px(18, uiScale_), -size.height() / 2);
    } else if (placement == "bottom") {
        pos = cursor + QPoint(-size.width() / 2, AppTheme::px(20, uiScale_));
    }

    if (auto* screen = QGuiApplication::screenAt(cursor)) {
        const QRect available = screen->availableGeometry().adjusted(8, 8, -8, -8);
        pos.setX(std::clamp(pos.x(), available.left(), available.right() - size.width()));
        pos.setY(std::clamp(pos.y(), available.top(), available.bottom() - size.height()));
    }
    tooltipPopup_->move(pos);
    tooltipPopup_->raise();
    tooltipPopup_->show();
}

void MainWindow::hideWorkbenchTooltip()
{
    if (tooltipPopup_) {
        tooltipPopup_->hide();
    }
}

void MainWindow::updateRunAnimation()
{
    ++runAnimationPhase_;
    bool hasRunning = false;
    for (auto it = nodeItems_.begin(); it != nodeItems_.end(); ++it) {
        if (nodeRunStates_.value(it.key()) == NodeExecutionState::Running) {
            hasRunning = true;
            if (canvas_) {
                canvas_->setAnimationPhase(it.key(), runAnimationPhase_);
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
    if (!canvas_ || !canvas_->focusNode(nodeId)) {
        return;
    }
    selectedNodeId_ = nodeId;
    rebuildProperties();
}

void MainWindow::focusLogNode(QListWidgetItem* item)
{
    if (!item) {
        return;
    }
    const QString nodeId = item->data(Qt::UserRole).toString();
    if (nodeId.isEmpty() || !canvas_ || !canvas_->focusNode(nodeId)) {
        return;
    }
    selectedNodeId_ = nodeId;
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

void MainWindow::finishWorkflowExecution(int generation,
                                         const Result<ExecutionResult>& result,
                                         const ExecutionResult& engineLastResult,
                                         const ExecutionEngine& updatedEngine)
{
    if (generation != executionGeneration_) {
        return;
    }
    engine_ = updatedEngine;
    lastResult_ = result.isFail() ? engineLastResult : result.value();
    rebuildEdges();
    for (const auto& summary : lastResult_.nodeSummaries) {
        appendLog(summary.message, summary.nodeId);
    }

    if (result.isFail()) {
        appendLog(QString("执行失败：%1").arg(result.error()), lastResult_.failedNodeId);
        appendProblem(QString("执行失败：%1").arg(result.error()), lastResult_.failedNodeId);
        focusFailedNode(lastResult_.failedNodeId);
        setExecutionBusy(false);
        QMessageBox::warning(this, "执行失败", result.error());
        return;
    }

    appendLog("执行完成");
    setExecutionBusy(false);
    updatePreviewForSelection();
}

void MainWindow::finishLivePreview(int generation,
                                   const QString& previewNodeId,
                                   const Result<ExecutionResult>& result,
                                   const ExecutionResult& engineLastResult,
                                   const ExecutionEngine& updatedEngine)
{
    if (generation != livePreviewGeneration_ || executionBusy_) {
        return;
    }
    engine_ = updatedEngine;
    lastResult_ = result.isFail() ? engineLastResult : result.value();
    rebuildEdges();
    for (const auto& summary : lastResult_.nodeSummaries) {
        appendLog(summary.message, summary.nodeId);
    }

    if (result.isFail()) {
        const QString failedNode = lastResult_.failedNodeId.isEmpty() ? previewNodeId : lastResult_.failedNodeId;
        appendLog(QString("实时预览失败：%1").arg(result.error()), failedNode);
        appendProblem(QString("实时预览失败：%1").arg(result.error()), failedNode);
        updatePreviewForSelection();
        return;
    }

    appendLog(QString("实时预览完成：%1").arg(nodeLabel(previewNodeId)), previewNodeId);
    updatePreviewForSelection();
}

void MainWindow::runWorkflow()
{
    if (executionBusy_) {
        appendLog("已有工作流正在执行，已忽略重复执行请求");
        return;
    }
    if (livePreviewTimer_) {
        livePreviewTimer_->stop();
    }
    ++livePreviewGeneration_;
    const int generation = ++executionGeneration_;
    resetNodeRunStates();
    if (workbenchBridge_) {
        workbenchBridge_->setStatusText("正在执行工作流");
    }
    setExecutionBusy(true);
    appendLog("开始后台执行工作流");

    WorkflowGraph graphSnapshot = WorkflowCommands::cloneGraph(graph_);
    ExecutionEngine engineSnapshot = engine_;
    QPointer<MainWindow> self(this);
    auto* thread = QThread::create([self, generation, graphSnapshot = std::move(graphSnapshot), engineSnapshot = std::move(engineSnapshot)]() mutable {
        auto postSummary = [self, generation](const NodeExecutionSummary& summary) {
            if (!self) {
                return;
            }
            QMetaObject::invokeMethod(self.data(), [self, generation, summary] {
                if (self && generation == self->executionGeneration_) {
                    self->handleNodeExecutionEvent(summary);
                }
            }, Qt::QueuedConnection);
        };
        const auto result = engineSnapshot.execute(graphSnapshot, postSummary);
        const ExecutionResult threadLastResult = engineSnapshot.lastResult();
        if (!self) {
            return;
        }
        QMetaObject::invokeMethod(self.data(), [self, generation, result, threadLastResult, engineSnapshot] {
            if (self) {
                self->finishWorkflowExecution(generation, result, threadLastResult, engineSnapshot);
            }
        }, Qt::QueuedConnection);
    });
    trackWorkerThread(thread);
    thread->start();
}

void MainWindow::scheduleLivePreview()
{
    if (!livePreviewTimer_ || selectedNodeId_.isEmpty() || executionBusy_) {
        return;
    }
    ++livePreviewGeneration_;
    livePreviewTimer_->start();
}

void MainWindow::runLivePreview()
{
    if (executionBusy_ || selectedNodeId_.isEmpty() || !graph_.containsNode(selectedNodeId_)) {
        return;
    }

    const QString previewNodeId = selectedNodeId_;
    const int generation = livePreviewGeneration_ == 0 ? ++livePreviewGeneration_ : livePreviewGeneration_;
    resetNodeRunStates();
    if (workbenchBridge_) {
        workbenchBridge_->setStatusText(QString("实时预览 %1").arg(nodeLabel(previewNodeId)));
    }
    WorkflowGraph graphSnapshot = WorkflowCommands::cloneGraph(graph_);
    ExecutionEngine engineSnapshot = engine_;
    QPointer<MainWindow> self(this);
    auto* thread = QThread::create([self,
                                    generation,
                                    previewNodeId,
                                    graphSnapshot = std::move(graphSnapshot),
                                    engineSnapshot = std::move(engineSnapshot)]() mutable {
        auto postSummary = [self, generation](const NodeExecutionSummary& summary) {
            if (!self) {
                return;
            }
            QMetaObject::invokeMethod(self.data(), [self, generation, summary] {
                if (self && generation == self->livePreviewGeneration_ && !self->executionBusy_) {
                    self->handleNodeExecutionEvent(summary);
                }
            }, Qt::QueuedConnection);
        };
        const auto result = engineSnapshot.executeForNode(graphSnapshot, previewNodeId, postSummary);
        const ExecutionResult threadLastResult = engineSnapshot.lastResult();
        if (!self) {
            return;
        }
        QMetaObject::invokeMethod(self.data(), [self, generation, previewNodeId, result, threadLastResult, engineSnapshot] {
            if (self) {
                self->finishLivePreview(generation, previewNodeId, result, threadLastResult, engineSnapshot);
            }
        }, Qt::QueuedConnection);
    });
    trackWorkerThread(thread);
    thread->start();
}

void MainWindow::exportWorkflowCopy()
{
    const QString defaultName = currentFile_.isEmpty()
                                    ? "workflow-export.json"
                                    : QString("%1-export.json").arg(QFileInfo(currentFile_).completeBaseName());
    const QString path = QFileDialog::getSaveFileName(this, "导出工作流", defaultName, "工作流 (*.json);;所有文件 (*)");
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
    const QString path = QFileDialog::getSaveFileName(this, "导出预览结果", "preview-result.png", "PNG (*.png);;所有文件 (*)");
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

    const QString path = QFileDialog::getSaveFileName(this, "导出画布截图", "workflow-canvas.png", "PNG (*.png);;所有文件 (*)");
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
    if (rejectGraphReplacementWhileBusy("新建工作流")) {
        return;
    }
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
    currentWorkflowBranch_ = "main";
    engine_.clearCache();
    lastResult_ = {};
    setPreviewImage({});
    nodeRunStates_.clear();
    nodeElapsedMs_.clear();
    selectedNodeId_.clear();
    if (problemModel_) {
        problemModel_->clear();
    }
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
    appendLog("新建工作流");
}

void MainWindow::openWorkflow()
{
    if (rejectGraphReplacementWhileBusy("打开工作流")) {
        return;
    }
    if (!confirmSaveIfNeeded()) {
        return;
    }
    const QString path = QFileDialog::getOpenFileName(this, "打开工作流", {}, "工作流 (*.json);;所有文件 (*)");
    if (path.isEmpty()) {
        return;
    }
    openWorkflowPath(path, false);
}

void MainWindow::openWorkflowPath(const QString& path, bool confirmCurrentWorkflow)
{
    if (path.trimmed().isEmpty()) {
        return;
    }
    if (rejectGraphReplacementWhileBusy("打开工作流")) {
        return;
    }
    if (confirmCurrentWorkflow && !confirmSaveIfNeeded()) {
        return;
    }
    if (livePreviewTimer_) {
        livePreviewTimer_->stop();
    }
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
    currentWorkflowBranch_ = "main";
    engine_.clearCache();
    lastResult_ = {};
    setPreviewImage({});
    nodeRunStates_.clear();
    nodeElapsedMs_.clear();
    selectedNodeId_.clear();
    if (problemModel_) {
        problemModel_->clear();
    }
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
    rememberRecentWorkflow(path);
    appendLog(QString("已打开：%1").arg(path));
}

bool MainWindow::confirmSaveIfNeeded()
{
    if (!currentWorkbookDirty()) {
        return true;
    }
    const auto choice = QMessageBox::warning(this, "保存修改",
                                             "当前工作流有未保存修改，是否先保存？",
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
    currentFile_ = QFileDialog::getSaveFileName(this, "保存工作流", "workflow.json", "工作流 (*.json);;所有文件 (*)");
    return !currentFile_.isEmpty();
}

void MainWindow::removeEdgeByIndex(int edgeIndex)
{
    if (rejectGraphReplacementWhileBusy("删除连线")) {
        return;
    }
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
    rememberRecentWorkflow(currentFile_);
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

void MainWindow::rememberRecentWorkflow(const QString& path)
{
    if (path.trimmed().isEmpty()) {
        return;
    }
    const QString absolutePath = QFileInfo(path).absoluteFilePath();
    QSettings settings;
    QStringList recent = settings.value("mainWindow/recentWorkflows").toStringList();
    recent.removeAll(absolutePath);
    recent.prepend(absolutePath);
    while (recent.size() > 12) {
        recent.removeLast();
    }
    settings.setValue("mainWindow/recentWorkflows", recent);
    if (quickAccessModel_) {
        quickAccessModel_->setRecentWorkflowPaths(recent);
    }
}

void MainWindow::refreshRecentWorkflowModel()
{
    if (!quickAccessModel_) {
        return;
    }
    QSettings settings;
    quickAccessModel_->setRecentWorkflowPaths(settings.value("mainWindow/recentWorkflows").toStringList());
}

void MainWindow::refreshWorkflowTemplateModel()
{
    if (!workflowTemplateModel_) {
        return;
    }
    QVector<WorkflowTemplateModel::Entry> entries;
    entries.append({"builtin:gray-preview", "灰度预览流程", "读入图片 → 灰度化 → 预览图片", "预设方案", true});
    entries.append({"builtin:resize-export", "缩放导出流程", "读入图片 → 缩放 → 导出图片", "预设方案", true});
    entries.append({"builtin:text-preview", "文字叠加预览", "读入图片 → 文字叠加 → 预览图片", "预设方案", true});
    entries.append({"builtin:blend-preview", "双图混合预览", "两张图片 → 双图混合 → 预览图片", "预设方案", true});

    QSettings settings;
    settings.beginGroup("workflowTemplates");
    const QStringList ids = settings.value("ids").toStringList();
    for (const QString& id : ids) {
        const QString path = settings.value("file/" + id).toString();
        if (!QFileInfo::exists(path)) {
            continue;
        }
        const QFileInfo info(path);
        const QString title = settings.value("title/" + id, info.baseName()).toString();
        entries.append({QString("user:%1").arg(id),
                        title,
                        QString("保存于 %1").arg(info.lastModified().toString("yyyy-MM-dd HH:mm")),
                        "我的模板",
                        false});
    }
    settings.endGroup();
    workflowTemplateModel_->setEntries(entries);
}

void MainWindow::refreshWorkflowCheckpointModel()
{
    if (!workflowCheckpointModel_) {
        return;
    }
    QVector<WorkflowCheckpointModel::Entry> entries;
    QSettings settings;
    settings.beginGroup("workflowCheckpoints");
    const QStringList ids = settings.value("ids").toStringList();
    for (const QString& id : ids) {
        const QString path = settings.value("file/" + id).toString();
        if (!QFileInfo::exists(path)) {
            continue;
        }
        const QFileInfo info(path);
        const QString branch = settings.value("branch/" + id, "main").toString();
        const QString title = settings.value("title/" + id, QString("保存点 %1").arg(id)).toString();
        entries.append({id,
                        title,
                        QString("保存于 %1").arg(info.lastModified().toString("yyyy-MM-dd HH:mm")),
                        branch});
    }
    settings.endGroup();
    workflowCheckpointModel_->setEntries(entries);
}

void MainWindow::saveCurrentWorkflowAsTemplate()
{
    if (rejectGraphReplacementWhileBusy("保存模板")) {
        return;
    }
    const QString title = QInputDialog::getText(this, "保存为模板", "模板名称：", QLineEdit::Normal,
                                               QFileInfo(currentFile_).completeBaseName().isEmpty()
                                                   ? QString("我的图像处理模板")
                                                   : QFileInfo(currentFile_).completeBaseName());
    if (title.trimmed().isEmpty()) {
        return;
    }
    const QString id = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmsszzz");
    const QString path = workflowSnapshotPath("templates", id);
    WorkflowSerializer serializer;
    auto saved = serializer.saveFile(graphForPersistence(), path);
    if (saved.isFail()) {
        appendProblem(QString("模板保存失败：%1").arg(saved.error()));
        QMessageBox::warning(this, "模板保存失败", saved.error());
        return;
    }

    QSettings settings;
    settings.beginGroup("workflowTemplates");
    QStringList ids = settings.value("ids").toStringList();
    ids.removeAll(id);
    ids.prepend(id);
    settings.setValue("ids", ids);
    settings.setValue("title/" + id, title.trimmed());
    settings.setValue("file/" + id, path);
    settings.endGroup();
    refreshWorkflowTemplateModel();
    appendLog(QString("已保存模板：%1").arg(title.trimmed()));
}

void MainWindow::applyWorkflowTemplate(const QString& templateId)
{
    if (templateId.trimmed().isEmpty()) {
        return;
    }
    if (rejectGraphReplacementWhileBusy("套用模板")) {
        return;
    }
    if (QMessageBox::question(this, "套用模板", "套用模板会覆盖当前画布，但可以通过撤销回到当前状态。是否继续？",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    Result<WorkflowGraph> loaded = Result<WorkflowGraph>::fail("未知模板");
    QString title = templateId;
    if (templateId.startsWith("builtin:")) {
        loaded = builtInTemplateGraph(templateId);
        if (workflowTemplateModel_) {
            title = workflowTemplateModel_->entryById(templateId).title;
        }
    } else if (templateId.startsWith("user:")) {
        const QString id = templateId.mid(QString("user:").size());
        QSettings settings;
        settings.beginGroup("workflowTemplates");
        const QString path = settings.value("file/" + id).toString();
        title = settings.value("title/" + id, id).toString();
        settings.endGroup();
        WorkflowSerializer serializer;
        loaded = serializer.loadFile(path);
    }

    if (loaded.isFail()) {
        appendProblem(QString("套用模板失败：%1").arg(loaded.error()));
        QMessageBox::warning(this, "套用模板失败", loaded.error());
        return;
    }

    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    graph_ = loaded.value();
    graphStack_.clear();
    forwardMacroHistory_.clear();
    selectedNodeId_.clear();
    currentWorkflowBranch_ = "main";
    engine_.clearCache();
    lastResult_ = {};
    setPreviewImage({});
    resetNodeRunStates();
    updateNavigationActions();
    rebuildScene();
    rebuildProperties();
    pushGraphEditCommand(QString("套用模板 %1").arg(title), before, selectedBefore,
                         WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
    appendLog(QString("已套用模板：%1").arg(title));
}

void MainWindow::createWorkflowCheckpoint()
{
    if (rejectGraphReplacementWhileBusy("保存当前进度")) {
        return;
    }
    const QString defaultTitle = QString("保存点 %1").arg(QDateTime::currentDateTime().toString("MM-dd HH:mm"));
    const QString title = QInputDialog::getText(this, "保存当前进度", "记录名称：", QLineEdit::Normal, defaultTitle);
    if (title.trimmed().isEmpty()) {
        return;
    }
    const QString id = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmsszzz");
    const QString path = workflowSnapshotPath("checkpoints", id);
    WorkflowSerializer serializer;
    auto saved = serializer.saveFile(graphForPersistence(), path);
    if (saved.isFail()) {
        appendProblem(QString("进度保存失败：%1").arg(saved.error()));
        QMessageBox::warning(this, "进度保存失败", saved.error());
        return;
    }

    QSettings settings;
    settings.beginGroup("workflowCheckpoints");
    QStringList ids = settings.value("ids").toStringList();
    ids.removeAll(id);
    ids.prepend(id);
    while (ids.size() > 40) {
        ids.removeLast();
    }
    settings.setValue("ids", ids);
    settings.setValue("title/" + id, title.trimmed());
    settings.setValue("branch/" + id, currentWorkflowBranch_);
    settings.setValue("file/" + id, path);
    settings.endGroup();
    refreshWorkflowCheckpointModel();
    appendLog(QString("已保存进度：%1 [%2]").arg(title.trimmed(), currentWorkflowBranch_));
}

void MainWindow::restoreWorkflowCheckpoint(const QString& checkpointId)
{
    if (rejectGraphReplacementWhileBusy("恢复进度")) {
        return;
    }
    QSettings settings;
    settings.beginGroup("workflowCheckpoints");
    const QString path = settings.value("file/" + checkpointId).toString();
    const QString title = settings.value("title/" + checkpointId, checkpointId).toString();
    const QString branch = settings.value("branch/" + checkpointId, "main").toString();
    settings.endGroup();

    if (path.isEmpty()) {
        return;
    }
    if (QMessageBox::question(this, "恢复进度", "恢复进度会覆盖当前画布，但可以通过撤销回到当前状态。是否继续？",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    WorkflowSerializer serializer;
    auto loaded = serializer.loadFile(path);
    if (loaded.isFail()) {
        appendProblem(QString("恢复进度失败：%1").arg(loaded.error()));
        QMessageBox::warning(this, "恢复进度失败", loaded.error());
        return;
    }

    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    graph_ = loaded.value();
    graphStack_.clear();
    forwardMacroHistory_.clear();
    selectedNodeId_.clear();
    currentWorkflowBranch_ = branch;
    engine_.clearCache();
    lastResult_ = {};
    setPreviewImage({});
    resetNodeRunStates();
    updateNavigationActions();
    rebuildScene();
    rebuildProperties();
    pushGraphEditCommand(QString("恢复进度 %1").arg(title), before, selectedBefore,
                         WorkflowCommands::cloneGraph(graph_), selectedNodeId_);
    appendLog(QString("已恢复进度：%1 [%2]").arg(title, currentWorkflowBranch_));
}

void MainWindow::branchFromWorkflowCheckpoint(const QString& checkpointId)
{
    if (rejectGraphReplacementWhileBusy("创建分支")) {
        return;
    }
    QSettings settings;
    settings.beginGroup("workflowCheckpoints");
    const QString path = settings.value("file/" + checkpointId).toString();
    const QString oldTitle = settings.value("title/" + checkpointId, checkpointId).toString();
    settings.endGroup();

    if (path.isEmpty()) {
        return;
    }
    const QString branch = QInputDialog::getText(this, "从保存点创建分支", "新分支名称：", QLineEdit::Normal,
                                                 QString("分支-%1").arg(QDateTime::currentDateTime().toString("MMdd-HHmm")));
    if (branch.trimmed().isEmpty()) {
        return;
    }

    WorkflowSerializer serializer;
    auto loaded = serializer.loadFile(path);
    if (loaded.isFail()) {
        appendProblem(QString("创建分支失败：%1").arg(loaded.error()));
        QMessageBox::warning(this, "创建分支失败", loaded.error());
        return;
    }

    const WorkflowGraph before = WorkflowCommands::cloneGraph(graph_);
    const QString selectedBefore = selectedNodeId_;
    graph_ = loaded.value();
    graphStack_.clear();
    forwardMacroHistory_.clear();
    selectedNodeId_.clear();
    currentWorkflowBranch_ = branch.trimmed();
    engine_.clearCache();
    lastResult_ = {};
    setPreviewImage({});
    resetNodeRunStates();
    updateNavigationActions();
    rebuildScene();
    rebuildProperties();
    pushGraphEditCommand(QString("创建分支 %1").arg(currentWorkflowBranch_), before, selectedBefore,
                         WorkflowCommands::cloneGraph(graph_), selectedNodeId_);

    const QString id = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmsszzz");
    const QString branchPath = workflowSnapshotPath("checkpoints", id);
    auto saved = serializer.saveFile(graphForPersistence(), branchPath);
    if (saved.isOk()) {
        QSettings branchSettings;
        branchSettings.beginGroup("workflowCheckpoints");
        QStringList ids = branchSettings.value("ids").toStringList();
        ids.prepend(id);
        branchSettings.setValue("ids", ids);
        branchSettings.setValue("title/" + id, QString("创建分支：%1").arg(currentWorkflowBranch_));
        branchSettings.setValue("branch/" + id, currentWorkflowBranch_);
        branchSettings.setValue("file/" + id, branchPath);
        branchSettings.endGroup();
        refreshWorkflowCheckpointModel();
    }
    appendLog(QString("从保存点创建分支：%1 ← %2").arg(currentWorkflowBranch_, oldTitle));
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
    if (workbenchBridge_) {
        workbenchBridge_->setDocumentTitle(title);
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

void MainWindow::resetWorkbenchLayout()
{
    if (!workbenchSplitter_ || !editorSplitter_) {
        return;
    }
    const auto metrics = AppTheme::metrics(uiScale_);
    const int sidebarWidth = std::max(metrics.paletteMinWidth, AppTheme::px(240, uiScale_));
    const int previewWidth = std::max(metrics.propertyMinWidth, AppTheme::px(320, uiScale_));
    workbenchSplitter_->setSizes({sidebarWidth, AppTheme::px(760, uiScale_), previewWidth});
    editorSplitter_->setSizes({AppTheme::px(560, uiScale_), std::max(metrics.logMaxHeight, AppTheme::px(220, uiScale_))});
    if (primarySidebar_) {
        primarySidebar_->show();
    }
    if (previewSidebar_) {
        previewSidebar_->show();
    }
    if (bottomPanel_) {
        bottomPanel_->show();
    }
    if (previewToggleAction_) {
        previewToggleAction_->setChecked(true);
    }
    if (bottomToggleAction_) {
        bottomToggleAction_->setChecked(true);
    }
    if (miniMap_) {
        miniMap_->show();
        QSettings settings;
        settings.setValue("mainWindow/showMiniMap", true);
    }
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
    dialog.setObjectName("settingsDialog");
    dialog.setMinimumSize(AppTheme::px(860, uiScale_), AppTheme::px(600, uiScale_));
    dialog.resize(AppTheme::px(900, uiScale_), AppTheme::px(620, uiScale_));
    dialog.setStyleSheet(R"(
        QDialog#settingsDialog {
            background: #1e1e1e;
            color: #cccccc;
        }
        QLabel#settingsTitle {
            color: #ffffff;
            font-size: 20px;
            font-weight: 600;
        }
        QLabel#settingsSubtitle,
        QLabel#settingsHint {
            color: #969696;
        }
        QLabel#settingsSectionTitle {
            color: #ffffff;
            font-size: 13px;
            font-weight: 600;
        }
        QWidget#settingsSection {
            background: #252526;
            border: 1px solid #3c3c3c;
        }
        QScrollArea#settingsScroll {
            background: #1e1e1e;
            border: 0px;
        }
        QScrollArea#settingsScroll > QWidget > QWidget {
            background: #1e1e1e;
        }
        QListWidget#settingsNav,
        QListWidget#shortcutList {
            background: #252526;
            border: 1px solid #3c3c3c;
            color: #cccccc;
            outline: 0px;
        }
        QListWidget#settingsNav::item {
            min-height: 32px;
            padding: 0px 10px;
            border-left: 2px solid transparent;
        }
        QListWidget#settingsNav::item:selected {
            background: #37373d;
            border-left: 2px solid #3794ff;
            color: #ffffff;
        }
        QListWidget#settingsNav::item:hover,
        QListWidget#shortcutList::item:hover {
            background: #2a2d2e;
        }
        QListWidget#shortcutList::item {
            min-height: 34px;
            padding: 5px 8px;
        }
        QLineEdit, QDoubleSpinBox, QSlider, QCheckBox {
            color: #cccccc;
        }
        QCheckBox {
            min-height: 32px;
            spacing: 9px;
            padding: 2px 0px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 1px solid #6a6a6a;
            background: #313131;
        }
        QCheckBox::indicator:hover {
            border: 1px solid #cccccc;
            background: #3c3c3c;
        }
        QCheckBox::indicator:checked {
            image: none;
            background: #0e639c;
            border: 1px solid #3794ff;
        }
        QLineEdit, QDoubleSpinBox {
            background: #3c3c3c;
            border: 1px solid #555555;
            padding: 5px 7px;
            selection-background-color: #264f78;
        }
        QLineEdit:focus, QDoubleSpinBox:focus {
            border: 1px solid #3794ff;
        }
        QPushButton {
            background: #2d2d30;
            border: 1px solid #3c3c3c;
            color: #cccccc;
            padding: 6px 12px;
            min-height: 36px;
            min-width: 108px;
        }
        QPushButton:hover {
            background: #3a3d41;
        }
        QPushButton:pressed {
            background: #094771;
        }
        QDialogButtonBox QPushButton {
            min-width: 112px;
            min-height: 38px;
        }
    )");

    auto* root = new QVBoxLayout(&dialog);
    root->setContentsMargins(AppTheme::px(18, uiScale_), AppTheme::px(16, uiScale_),
                             AppTheme::px(18, uiScale_), AppTheme::px(14, uiScale_));
    root->setSpacing(AppTheme::px(12, uiScale_));

    auto* title = new QLabel("设置");
    title->setObjectName("settingsTitle");
    auto* subtitle = new QLabel("调整工作台、画布和快捷键查看方式。所有改动保持深色工作台风格。");
    subtitle->setObjectName("settingsSubtitle");
    root->addWidget(title);
    root->addWidget(subtitle);

    auto* content = new QWidget;
    auto* contentLayout = new QHBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(AppTheme::px(12, uiScale_));

    auto* nav = new QListWidget;
    nav->setObjectName("settingsNav");
    nav->setFixedWidth(AppTheme::px(180, uiScale_));
    nav->addItem("常用");
    nav->addItem("画布");
    nav->addItem("工作台");
    nav->addItem("快捷键");
    nav->setCurrentRow(0);

    auto* pages = new QStackedWidget;
    pages->setObjectName("settingsPages");

    auto makePage = [this] {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(AppTheme::px(10, uiScale_));
        return page;
    };

    auto makeSection = [this](QWidget* page, const QString& titleText, const QString& hintText = {}) {
        auto* section = new QWidget;
        section->setObjectName("settingsSection");
        auto* sectionLayout = new QVBoxLayout(section);
        sectionLayout->setContentsMargins(AppTheme::px(14, uiScale_), AppTheme::px(12, uiScale_),
                                           AppTheme::px(14, uiScale_), AppTheme::px(12, uiScale_));
        sectionLayout->setSpacing(AppTheme::px(10, uiScale_));
        auto* titleLabel = new QLabel(titleText);
        titleLabel->setObjectName("settingsSectionTitle");
        sectionLayout->addWidget(titleLabel);
        if (!hintText.isEmpty()) {
            auto* hint = new QLabel(hintText);
            hint->setObjectName("settingsHint");
            hint->setWordWrap(true);
            sectionLayout->addWidget(hint);
        }
        page->layout()->addWidget(section);
        return sectionLayout;
    };

    auto makeRow = [this](QLayout* parent, const QString& labelText, QWidget* control, const QString& hintText = {}) {
        auto* row = new QWidget;
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(AppTheme::px(12, uiScale_));
        auto* labels = new QWidget;
        auto* labelsLayout = new QVBoxLayout(labels);
        labelsLayout->setContentsMargins(0, 0, 0, 0);
        labelsLayout->setSpacing(2);
        auto* label = new QLabel(labelText);
        label->setObjectName("settingsSectionTitle");
        labelsLayout->addWidget(label);
        if (!hintText.isEmpty()) {
            auto* hint = new QLabel(hintText);
            hint->setObjectName("settingsHint");
            hint->setWordWrap(true);
            labelsLayout->addWidget(hint);
        }
        rowLayout->addWidget(labels, 1);
        rowLayout->addWidget(control, 0);
        parent->addWidget(row);
    };

    auto* uiScaleSpin = new QDoubleSpinBox;
    uiScaleSpin->setRange(AppTheme::kMinUiScale * 100.0, AppTheme::kMaxUiScale * 100.0);
    uiScaleSpin->setSingleStep(AppTheme::kUiScaleStep * 100.0);
    uiScaleSpin->setDecimals(0);
    uiScaleSpin->setSuffix("%");
    uiScaleSpin->setValue(uiScale_ * 100.0);
    uiScaleSpin->setFixedWidth(AppTheme::px(112, uiScale_));
    uiScaleSpin->setToolTip("调整工作台整体缩放比例");

    auto* canvasRow = new QWidget;
    auto* canvasLayout = new QHBoxLayout(canvasRow);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->setSpacing(AppTheme::px(8, uiScale_));
    auto* canvasZoomSlider = new QSlider(Qt::Horizontal);
    canvasZoomSlider->setFixedWidth(AppTheme::px(200, uiScale_));
    canvasZoomSlider->setRange(25, 300);
    canvasZoomSlider->setValue(int(std::lround(zoomScale_ * 100.0)));
    canvasZoomSlider->setToolTip("调整当前画布缩放比例");
    auto* canvasZoomLabel = new QLabel(QString("%1%").arg(canvasZoomSlider->value()));
    canvasZoomLabel->setMinimumWidth(AppTheme::px(48, uiScale_));
    canvasLayout->addWidget(canvasZoomSlider);
    canvasLayout->addWidget(canvasZoomLabel);
    connect(canvasZoomSlider, &QSlider::valueChanged, canvasZoomLabel, [canvasZoomLabel](int value) {
        canvasZoomLabel->setText(QString("%1%").arg(value));
    });

    auto* wheelZoomSpin = new QDoubleSpinBox;
    wheelZoomSpin->setRange(1.5, 8.0);
    wheelZoomSpin->setSingleStep(0.5);
    wheelZoomSpin->setDecimals(1);
    wheelZoomSpin->setSuffix("% / 格");
    wheelZoomSpin->setValue((wheelZoomStep_ - 1.0) * 100.0);
    wheelZoomSpin->setFixedWidth(AppTheme::px(126, uiScale_));
    wheelZoomSpin->setToolTip("控制鼠标滚轮缩放速度，数值越小越慢");

    auto* previewVisible = new QCheckBox;
    previewVisible->setText("显示右侧预览");
    previewVisible->setChecked(!previewSidebar_ || previewSidebar_->isVisible());
    previewVisible->setToolTip("显示或隐藏右侧预览区域");
    auto* bottomVisible = new QCheckBox;
    bottomVisible->setText("显示底部面板");
    bottomVisible->setChecked(!bottomPanel_ || bottomPanel_->isVisible());
    bottomVisible->setToolTip("显示或隐藏底部终端、问题和输出面板");
    auto* miniMapVisible = new QCheckBox;
    miniMapVisible->setText("显示画布小地图");
    miniMapVisible->setChecked(!miniMap_ || miniMap_->isVisible());
    miniMapVisible->setToolTip("显示或隐藏画布左下角小地图");

    auto* generalPage = makePage();
    auto* generalAppearance = makeSection(generalPage, "外观", "当前版本固定使用深色工作台风格，避免浅色主题和工作台视觉分叉。");
    makeRow(generalAppearance, "界面大小", uiScaleSpin, "控制菜单、侧栏、按钮和节点参数控件的整体缩放。");
    auto* generalActions = makeSection(generalPage, "常用操作", "快速触发常用命令；工作台区域布局相关设置在“工作台”页。");
    auto* openCommandPaletteButton = new QPushButton("打开命令面板");
    auto* organizeButton = new QPushButton("整理画布");
    openCommandPaletteButton->setToolTip("打开命令、节点和问题快速搜索入口");
    organizeButton->setToolTip("整理画布并自动居中显示全部节点");
    auto* actionsRow = new QWidget;
    auto* actionsLayout = new QHBoxLayout(actionsRow);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(AppTheme::px(8, uiScale_));
    actionsLayout->addWidget(openCommandPaletteButton);
    actionsLayout->addWidget(organizeButton);
    actionsLayout->addStretch(1);
    generalActions->addWidget(actionsRow);
    generalPage->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    auto* canvasPage = makePage();
    auto* zoomSection = makeSection(canvasPage, "缩放", "鼠标滚轮缩放已改为低速步进，适合精细调整节点图。");
    makeRow(zoomSection, "当前画布缩放", canvasRow, "只影响中央画布，不改变工作台控件尺寸。");
    makeRow(zoomSection, "滚轮缩放速度", wheelZoomSpin, "每滚动一格的缩放比例。默认 4%，数值越小越慢。");
    canvasPage->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    auto* workbenchPage = makePage();
    auto* areaSection = makeSection(workbenchPage, "区域显示");
    areaSection->addWidget(previewVisible);
    areaSection->addWidget(bottomVisible);
    areaSection->addWidget(miniMapVisible);
    auto* workbenchActionSection = makeSection(workbenchPage, "布局", "把侧栏、预览和底部面板恢复到默认排布。");
    auto* resetLayoutButton2 = new QPushButton("重置工作台布局");
    resetLayoutButton2->setToolTip("恢复侧栏、预览和底部面板的默认布局");
    auto* workbenchActionRow = new QWidget;
    auto* workbenchActionLayout = new QHBoxLayout(workbenchActionRow);
    workbenchActionLayout->setContentsMargins(0, 0, 0, 0);
    workbenchActionLayout->setSpacing(AppTheme::px(8, uiScale_));
    workbenchActionLayout->addWidget(resetLayoutButton2);
    workbenchActionLayout->addStretch(1);
    workbenchActionSection->addWidget(workbenchActionRow);
    workbenchPage->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    auto* shortcutsPage = makePage();
    auto* shortcutSection = makeSection(shortcutsPage, "快捷键", "本页用于查询当前命令和快捷键；本轮先不做快捷键重绑定，避免破坏跨平台默认习惯。");
    auto* shortcutSearch = new QLineEdit;
    shortcutSearch->setPlaceholderText("搜索命令、分类或快捷键");
    shortcutSection->addWidget(shortcutSearch);
    auto* shortcutList = new QListWidget;
    shortcutList->setObjectName("shortcutList");
    shortcutSection->addWidget(shortcutList, 1);

    // 每页各自包一个 QScrollArea，让自动换行的提示标签在各自视口宽度下正确计算
    // 高度，避免整页塞进单个 QScrollArea 时 heightForWidth 无法穿过 QStackedWidget
    // 传递而导致文字重叠。
    auto addScrollablePage = [](QStackedWidget* stack, QWidget* page) {
        auto* scroll = new QScrollArea;
        scroll->setObjectName("settingsScroll");
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setWidget(page);
        stack->addWidget(scroll);
    };
    addScrollablePage(pages, generalPage);
    addScrollablePage(pages, canvasPage);
    addScrollablePage(pages, workbenchPage);
    addScrollablePage(pages, shortcutsPage);
    contentLayout->addWidget(nav);
    contentLayout->addWidget(pages, 1);
    root->addWidget(content, 1);

    auto populateShortcutList = [this, shortcutSearch, shortcutList] {
        shortcutList->clear();
        const QString filter = shortcutSearch->text().trimmed();
        if (!workbenchCommands_) {
            return;
        }
        for (const auto& entry : workbenchCommands_->entries()) {
            const QString haystack = QString("%1 %2 %3 %4").arg(entry.title, entry.category, entry.shortcut, entry.id);
            if (!filter.isEmpty() && !haystack.contains(filter, Qt::CaseInsensitive)) {
                continue;
            }
            const QString shortcut = entry.shortcut.isEmpty() ? "未设置" : entry.shortcut;
            auto* item = new QListWidgetItem(QString("%1\n%2  ·  %3").arg(entry.title, entry.category, shortcut));
            item->setToolTip(entry.id);
            if (!entry.action || !entry.action->isEnabled()) {
                item->setForeground(QColor("#858585"));
            }
            shortcutList->addItem(item);
        }
    };
    populateShortcutList();

    connect(nav, &QListWidget::currentRowChanged, pages, &QStackedWidget::setCurrentIndex);
    connect(shortcutSearch, &QLineEdit::textChanged, &dialog, [populateShortcutList] {
        populateShortcutList();
    });
    connect(openCommandPaletteButton, &QPushButton::clicked, &dialog, [this, &dialog] {
        dialog.accept();
        if (workbenchBridge_) {
            workbenchBridge_->showQuickAccess();
        }
    });
    auto organizeNow = [this, &dialog] {
        dialog.accept();
        autoLayoutWorkflow();
    };
    connect(organizeButton, &QPushButton::clicked, &dialog, organizeNow);
    connect(resetLayoutButton2, &QPushButton::clicked, this, [this, previewVisible, bottomVisible, miniMapVisible] {
        resetWorkbenchLayout();
        previewVisible->setChecked(true);
        bottomVisible->setChecked(true);
        miniMapVisible->setChecked(true);
    });
    auto* resetScaleButton = new QPushButton("重置为 100%");
    resetScaleButton->setToolTip("将工作台控件缩放恢复为 100%");
    generalAppearance->addWidget(resetScaleButton);
    connect(resetScaleButton, &QPushButton::clicked, this, [uiScaleSpin] {
        uiScaleSpin->setValue(100.0);
    });

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    if (auto* button = buttons->button(QDialogButtonBox::Apply)) {
        button->setText("应用");
        button->setToolTip("应用当前设置但不关闭窗口");
    }
    if (auto* button = buttons->button(QDialogButtonBox::Cancel)) {
        button->setText("取消");
    }
    if (auto* button = buttons->button(QDialogButtonBox::Ok)) {
        button->setText("确定");
    }
    root->addWidget(buttons);

    auto applySettings = [&] {
        QSettings settings;
        AppTheme::setThemePreference(AppTheme::ThemePreference::Dark);
        settings.setValue("mainWindow/theme", "dark");
        wheelZoomStep_ = 1.0 + wheelZoomSpin->value() / 100.0;
        settings.setValue("mainWindow/wheelZoomStep", wheelZoomStep_);
        setUiScale(uiScaleSpin->value() / 100.0);
        applyUiScale();
        const double requestedZoom = canvasZoomSlider->value() / 100.0;
        if (!qFuzzyCompare(requestedZoom, zoomScale_)) {
            applyZoomFactor(requestedZoom / zoomScale_);
        }
        if (previewToggleAction_) {
            previewToggleAction_->setChecked(previewVisible->isChecked());
        }
        if (bottomToggleAction_) {
            bottomToggleAction_->setChecked(bottomVisible->isChecked());
        }
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
    if (activityBar_) {
        activityBar_->setFixedWidth(AppTheme::px(50, uiScale_));
        const auto buttons = activityBar_->findChildren<QToolButton*>();
        for (auto* button : buttons) {
            button->setFixedSize(AppTheme::px(50, uiScale_), AppTheme::px(50, uiScale_));
            button->setIconSize(QSize(metrics.toolbarIcon, metrics.toolbarIcon));
            const QString tooltip = button->toolTip();
            button->setIcon(lineIcon(tooltip == "工作流" ? "workflow" : "nodes"));
        }
    }
    if (primarySidebar_) {
        primarySidebar_->setMinimumWidth(metrics.paletteMinWidth);
    }
    if (previewSidebar_) {
        previewSidebar_->setMinimumWidth(metrics.propertyMinWidth);
    }
    if (workbenchHost_) {
        workbenchHost_->setUiScale(uiScale_);
    }
    if (workbenchBridge_) {
        workbenchBridge_->setZoomText(QString("%1%").arg(int(std::lround(zoomScale_ * 100.0))));
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

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    NativeWindowChrome::configure(this);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!confirmSaveIfNeeded()) {
        event->ignore();
        return;
    }
    // 关闭后到来的后台执行/实时预览回调按 generation 检查丢弃，避免回调进入正在销毁的窗口。
    ++executionGeneration_;
    ++livePreviewGeneration_;
    if (livePreviewTimer_) {
        livePreviewTimer_->stop();
    }
    if (runAnimationTimer_) {
        runAnimationTimer_->stop();
    }
    QSettings settings;
    settings.setValue("mainWindow/geometry", saveGeometry());
    if (workbenchSplitter_) {
        settings.setValue("mainWindow/workbenchSplitter", workbenchSplitter_->saveState());
    }
    if (editorSplitter_) {
        settings.setValue("mainWindow/editorSplitter", editorSplitter_->saveState());
    }
    settings.setValue("mainWindow/uiScale", uiScale_);
    settings.setValue("mainWindow/theme", AppTheme::themePreferenceName());
    settings.setValue("mainWindow/layoutVersion", 6);
    QMainWindow::closeEvent(event);
}
