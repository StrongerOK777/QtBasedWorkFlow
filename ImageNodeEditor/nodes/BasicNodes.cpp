#include "nodes/BasicNodes.h"

#include "nodes/ImageNode.h"
#include "nodes/NodeFactory.h"
#include "processing/ImageProcessors.h"

#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QJsonValue>
#include <QPainter>
#include <QSharedPointer>
#include <functional>

namespace {

using OutputMap = QMap<QString, NodeData>;
using Executor = std::function<Result<OutputMap>(const QJsonObject&, const QMap<QString, NodeData>&)>;

PortInfo in(const QString& name, const QString& label, bool required = true,
            PortType type = PortType::ImageRGBA)
{
    return PortInfo{name, label, PortDirection::Input, type, required, false};
}

PortInfo out(const QString& name = "output", const QString& label = "输出",
             PortType type = PortType::ImageRGBA)
{
    return PortInfo{name, label, PortDirection::Output, type, true, true};
}

NodeParameter param(const QString& name, const QString& label, ParameterType type, const QVariant& def,
                    double min = 0.0, double max = 0.0, const QStringList& options = {})
{
    return NodeParameter{name, label, type, def, min, max, options, false};
}

Result<QImage> inputImage(const QMap<QString, NodeData>& inputs, const QString& port)
{
    if (!inputs.contains(port)) {
        return Result<QImage>::fail(QString("缺少输入端口：%1").arg(port));
    }
    const NodeData data = inputs.value(port);
    if (!portTypesCompatible(data.type, PortType::ImageRGBA)) {
        return Result<QImage>::fail(QString("输入端口 %1 类型不兼容").arg(port));
    }
    const QImage image = data.value.value<QImage>();
    if (image.isNull()) {
        return Result<QImage>::fail(QString("输入端口 %1 图片为空").arg(port));
    }
    return Result<QImage>::ok(image);
}

Result<OutputMap> imageOutput(const QImage& image)
{
    return Result<OutputMap>::ok(OutputMap{{"output", NodeData{PortType::ImageRGBA, QVariant::fromValue(image)}}});
}

class SimpleNode final : public ImageNode {
public:
    SimpleNode(QString typeName, QString displayName, QVector<PortInfo> inputs, QVector<PortInfo> outputs,
               QVector<NodeParameter> parameters, Executor executor, bool cacheable)
        : typeName_(std::move(typeName)),
          displayName_(std::move(displayName)),
          inputs_(std::move(inputs)),
          outputs_(std::move(outputs)),
          parameters_(std::move(parameters)),
          executor_(std::move(executor)),
          cacheable_(cacheable)
    {
        for (const auto& p : parameters_) {
            params_.insert(p.name, QJsonValue::fromVariant(p.defaultValue));
        }
    }

    QString typeName() const override { return typeName_; }
    QString displayName() const override { return displayName_; }
    QVector<PortInfo> inputPorts() const override { return inputs_; }
    QVector<PortInfo> outputPorts() const override { return outputs_; }
    QVector<NodeParameter> parameterDefinitions() const override { return parameters_; }
    QJsonObject saveParams() const override { return params_; }

    Status loadParams(const QJsonObject& object) override
    {
        QJsonObject next;
        for (const auto& p : parameters_) {
            QJsonValue value = object.contains(p.name) ? object.value(p.name) : QJsonValue::fromVariant(p.defaultValue);
            if (p.type == ParameterType::Integer || p.type == ParameterType::Double) {
                if (!value.isDouble()) {
                    return Status::fail(QString("%1 参数必须是数字").arg(p.displayName));
                }
                // 声明了范围（min < max）时拒绝越界值，防止手改 JSON 静默通过。
                if (p.min < p.max && (value.toDouble() < p.min || value.toDouble() > p.max)) {
                    return Status::fail(QString("%1 参数超出范围 [%2, %3]：%4")
                                            .arg(p.displayName)
                                            .arg(p.min)
                                            .arg(p.max)
                                            .arg(value.toDouble()));
                }
            }
            if ((p.type == ParameterType::Text || p.type == ParameterType::FileOpen || p.type == ParameterType::FileSave
                 || p.type == ParameterType::Choice || p.type == ParameterType::Color) && !value.isString()) {
                return Status::fail(QString("%1 参数必须是字符串").arg(p.displayName));
            }
            if (p.type == ParameterType::Choice && !p.options.isEmpty() && !p.options.contains(value.toString())) {
                return Status::fail(QString("%1 参数取值非法：%2（可选：%3）")
                                        .arg(p.displayName, value.toString(), p.options.join("/")));
            }
            if (p.type == ParameterType::Boolean && !value.isBool()) {
                return Status::fail(QString("%1 参数必须是布尔值").arg(p.displayName));
            }
            next.insert(p.name, value);
        }
        params_ = next;
        return Status::ok();
    }

    Result<OutputMap> execute(const QMap<QString, NodeData>& inputs) override
    {
        return executor_(params_, inputs);
    }

    bool isCacheable() const override { return cacheable_; }

private:
    QString typeName_;
    QString displayName_;
    QVector<PortInfo> inputs_;
    QVector<PortInfo> outputs_;
    QVector<NodeParameter> parameters_;
    QJsonObject params_;
    Executor executor_;
    bool cacheable_ = true;
};

QSharedPointer<ImageNode> make(QString typeName, QString displayName, QVector<PortInfo> inputs, QVector<PortInfo> outputs,
                               QVector<NodeParameter> parameters, Executor executor, bool cacheable = true)
{
    return QSharedPointer<ImageNode>(new SimpleNode(std::move(typeName), std::move(displayName), std::move(inputs),
                                                    std::move(outputs), std::move(parameters), std::move(executor), cacheable));
}

void registerNode(NodeFactory& factory, const QString& type, const QString& label, const QString& category,
                  QVector<PortInfo> inputs, QVector<PortInfo> outputs, QVector<NodeParameter> parameters, Executor executor,
                  bool cacheable = true)
{
    factory.registerNode(type, label, category, [=]() {
        return make(type, label, inputs, outputs, parameters, executor, cacheable);
    });
}

}

void registerBasicNodes(NodeFactory& factory)
{
    registerNode(factory, "ImageInput", "读入图片", "输入输出", {}, {out()},
                 {param("filePath", "图片路径", ParameterType::FileOpen, QString())},
                 [](const QJsonObject& p, const QMap<QString, NodeData>&) {
                     const QString path = p.value("filePath").toString();
                     if (path.isEmpty()) return Result<OutputMap>::fail("读入图片节点缺少图片路径");
                     QImage image(path);
                     if (image.isNull()) return Result<OutputMap>::fail(QString("无法读取图片：%1").arg(path));
                     return imageOutput(image.convertToFormat(QImage::Format_ARGB32));
                 });

    registerNode(factory, "ImageOutput", "导出图片", "输入输出", {in("image", "图片")}, {},
                 {param("outputPath", "导出路径", ParameterType::FileSave, QString("output.png"))},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     const QString path = p.value("outputPath").toString();
                     if (path.isEmpty()) return Result<OutputMap>::fail("导出节点缺少输出路径");
                     QFileInfo info(path);
                     if (!info.dir().exists() && !info.dir().mkpath(".")) {
                         return Result<OutputMap>::fail(QString("无法创建导出目录：%1").arg(info.dir().absolutePath()));
                     }
                     if (!img.value().save(path)) {
                         return Result<OutputMap>::fail(QString("图片保存失败：%1").arg(path));
                     }
                     return Result<OutputMap>::ok(OutputMap{});
                 },
                 false);

    registerNode(factory, "Preview", "预览图片", "输入输出", {in("image", "图片")}, {out()},
                 {}, [](const QJsonObject&, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     return imageOutput(img.value());
                 });

    registerNode(factory, "Crop", "裁切", "几何变换", {in("image", "图片")}, {out()},
                 {param("x", "X", ParameterType::Integer, 0, 0, 100000),
                  param("y", "Y", ParameterType::Integer, 0, 0, 100000),
                  param("width", "宽度", ParameterType::Integer, 200, 1, 100000),
                  param("height", "高度", ParameterType::Integer, 200, 1, 100000)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::crop(img.value(), p.value("x").toInt(), p.value("y").toInt(),
                                                         p.value("width").toInt(), p.value("height").toInt());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "Resize", "缩放", "几何变换", {in("image", "图片")}, {out()},
                 {param("width", "宽度", ParameterType::Integer, 800, 1, 100000),
                  param("height", "高度", ParameterType::Integer, 600, 1, 100000),
                  param("keepAspect", "保持比例", ParameterType::Boolean, true)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::resize(img.value(), p.value("width").toInt(), p.value("height").toInt(),
                                                           p.value("keepAspect").toBool());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "Grayscale", "灰度化", "色彩处理", {in("image", "图片")}, {out()},
                 {}, [](const QJsonObject&, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::grayscale(img.value());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "BrightnessContrast", "亮度/对比度", "色彩处理", {in("image", "图片")}, {out()},
                 {param("brightness", "亮度", ParameterType::Integer, 0, -255, 255),
                  param("contrast", "对比度", ParameterType::Double, 1.0, 0, 4)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::brightnessContrast(img.value(), p.value("brightness").toInt(),
                                                                        p.value("contrast").toDouble());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "Blur", "模糊", "滤波处理", {in("image", "图片")}, {out()},
                 {param("radius", "半径", ParameterType::Integer, 3, 0, 20)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::boxBlur(img.value(), p.value("radius").toInt());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "RotateFlip", "旋转/翻转", "几何变换", {in("image", "图片")}, {out()},
                 {param("angle", "角度", ParameterType::Choice, QString("0"), 0, 0, {"0", "90", "180", "270"}),
                  param("flipHorizontal", "水平翻转", ParameterType::Boolean, false),
                  param("flipVertical", "垂直翻转", ParameterType::Boolean, false)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::rotateFlip(img.value(), p.value("angle").toString().toInt(),
                                                               p.value("flipHorizontal").toBool(),
                                                               p.value("flipVertical").toBool());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "TextOverlay", "文字叠加", "合成处理", {in("image", "图片")}, {out()},
                 {param("text", "文字", ParameterType::Text, QString("示例文字")),
                  param("anchor", "锚点", ParameterType::Choice, QString("custom"), 0, 0,
                        {"custom", "topLeft", "topRight", "bottomLeft", "bottomRight", "center"}),
                  param("x", "X 偏移", ParameterType::Integer, 30, -100000, 100000),
                  param("y", "Y 偏移", ParameterType::Integer, 60, -100000, 100000),
                  param("size", "字号(px)", ParameterType::Integer, 32, 1, 256),
                  param("color", "颜色", ParameterType::Color, QString("#ffffff")),
                  param("fontFamily", "字体", ParameterType::Text, QString()),
                  param("bold", "粗体", ParameterType::Boolean, false),
                  param("opacity", "不透明度", ParameterType::Double, 1.0, 0, 1),
                  param("outline", "描边", ParameterType::Boolean, false),
                  param("outlineColor", "描边颜色", ParameterType::Color, QString("#000000"))},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::textOverlay(
                         img.value(), p.value("text").toString(), p.value("x").toInt(), p.value("y").toInt(),
                         p.value("size").toInt(), QColor(p.value("color").toString()),
                         p.value("fontFamily").toString(), p.value("bold").toBool(), p.value("anchor").toString(),
                         p.value("opacity").toDouble(), p.value("outline").toBool(),
                         QColor(p.value("outlineColor").toString()));
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "Blend", "双图混合", "合成处理", {in("first", "图片 A"), in("second", "图片 B")}, {out()},
                 {param("opacity", "B 透明度", ParameterType::Double, 0.5, 0, 1),
                  param("mode", "混合模式", ParameterType::Choice, QString("normal"), 0, 0,
                        {"normal", "multiply", "screen", "overlay", "darken", "lighten", "difference"}),
                  param("sizeMode", "尺寸策略", ParameterType::Choice, QString("stretch"), 0, 0,
                        {"stretch", "fit", "error"})},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto a = inputImage(inputs, "first");
                     auto b = inputImage(inputs, "second");
                     if (a.isFail()) return Result<OutputMap>::fail(a.error());
                     if (b.isFail()) return Result<OutputMap>::fail(b.error());
                     auto result = ImageProcessors::blend(a.value(), b.value(), p.value("opacity").toDouble(),
                                                          p.value("mode").toString(), p.value("sizeMode").toString());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "ImageMerge", "图片拼接", "合成处理",
                 {in("first", "图片 A", false), in("second", "图片 B", false), in("third", "图片 C", false)}, {out()},
                 {param("mode", "布局", ParameterType::Choice, QString("horizontal"), 0, 0,
                        {"horizontal", "vertical", "grid"}),
                  param("columns", "网格列数", ParameterType::Integer, 2, 1, 9),
                  param("background", "背景色", ParameterType::Color, QString("#00000000"))},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     QVector<QImage> images;
                     // 已连接的输入若为空图或类型错误必须报错，不允许静默丢弃。
                     for (const QString& port : {"first", "second", "third"}) {
                         if (inputs.contains(port)) {
                             auto img = inputImage(inputs, port);
                             if (img.isFail()) return Result<OutputMap>::fail(QString("拼接节点：%1").arg(img.error()));
                             images.append(img.value());
                         }
                     }
                     auto result = ImageProcessors::merge(images, p.value("mode").toString(),
                                                          p.value("columns").toInt(),
                                                          QColor::fromString(p.value("background").toString()));
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    // ---- 扩展节点 ----

    registerNode(factory, "Sharpen", "锐化", "滤波处理", {in("image", "图片")}, {out()},
                 {param("amount", "强度", ParameterType::Double, 1.0, 0, 3),
                  param("radius", "半径", ParameterType::Integer, 2, 1, 10)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::sharpen(img.value(), p.value("amount").toDouble(),
                                                            p.value("radius").toInt());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "EdgeDetect", "边缘检测", "滤波处理", {in("image", "图片")},
                 {out("output", "输出", PortType::ImageGray)}, {},
                 [](const QJsonObject&, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::edgeDetect(img.value());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return Result<OutputMap>::ok(OutputMap{
                         {"output", NodeData{PortType::ImageGray, QVariant::fromValue(result.value())}}});
                 });

    registerNode(factory, "Invert", "反色", "色彩处理", {in("image", "图片")}, {out()},
                 {}, [](const QJsonObject&, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::invert(img.value());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "Threshold", "阈值二值化", "色彩处理", {in("image", "图片")},
                 {out("output", "输出", PortType::ImageGray)},
                 {param("level", "阈值", ParameterType::Integer, 128, 0, 255)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::threshold(img.value(), p.value("level").toInt());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return Result<OutputMap>::ok(OutputMap{
                         {"output", NodeData{PortType::ImageGray, QVariant::fromValue(result.value())}}});
                 });

    registerNode(factory, "HueSaturation", "色相/饱和度", "色彩处理", {in("image", "图片")}, {out()},
                 {param("hue", "色相偏移", ParameterType::Integer, 0, -180, 180),
                  param("saturation", "饱和度", ParameterType::Integer, 0, -100, 100),
                  param("lightness", "明度", ParameterType::Integer, 0, -100, 100)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::hueSaturation(img.value(), p.value("hue").toInt(),
                                                                  p.value("saturation").toInt(),
                                                                  p.value("lightness").toInt());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "ChannelSplit", "通道拆分", "色彩处理", {in("image", "图片")},
                 {out("red", "红通道", PortType::ImageGray), out("green", "绿通道", PortType::ImageGray),
                  out("blue", "蓝通道", PortType::ImageGray)},
                 {}, [](const QJsonObject&, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::channelSplit(img.value());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     const auto& channels = result.value();
                     return Result<OutputMap>::ok(OutputMap{
                         {"red", NodeData{PortType::ImageGray, QVariant::fromValue(channels[0])}},
                         {"green", NodeData{PortType::ImageGray, QVariant::fromValue(channels[1])}},
                         {"blue", NodeData{PortType::ImageGray, QVariant::fromValue(channels[2])}}});
                 });

    registerNode(factory, "ChannelMerge", "通道合并", "色彩处理",
                 {in("red", "红通道", true, PortType::ImageGray), in("green", "绿通道", true, PortType::ImageGray),
                  in("blue", "蓝通道", true, PortType::ImageGray)},
                 {out()}, {},
                 [](const QJsonObject&, const QMap<QString, NodeData>& inputs) {
                     auto r = inputImage(inputs, "red");
                     auto g = inputImage(inputs, "green");
                     auto b = inputImage(inputs, "blue");
                     if (r.isFail()) return Result<OutputMap>::fail(r.error());
                     if (g.isFail()) return Result<OutputMap>::fail(g.error());
                     if (b.isFail()) return Result<OutputMap>::fail(b.error());
                     auto result = ImageProcessors::channelMerge(r.value(), g.value(), b.value());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "MaskBlend", "蒙版混合", "合成处理",
                 {in("foreground", "前景"), in("background", "背景"),
                  in("mask", "蒙版", true, PortType::ImageGray)},
                 {out()}, {},
                 [](const QJsonObject&, const QMap<QString, NodeData>& inputs) {
                     auto fg = inputImage(inputs, "foreground");
                     auto bg = inputImage(inputs, "background");
                     auto mask = inputImage(inputs, "mask");
                     if (fg.isFail()) return Result<OutputMap>::fail(fg.error());
                     if (bg.isFail()) return Result<OutputMap>::fail(bg.error());
                     if (mask.isFail()) return Result<OutputMap>::fail(mask.error());
                     auto result = ImageProcessors::maskBlend(fg.value(), bg.value(), mask.value());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "ImageOverlay", "图片水印", "合成处理",
                 {in("base", "底图"), in("overlay", "水印图")}, {out()},
                 {param("anchor", "位置", ParameterType::Choice, QString("bottomRight"), 0, 0,
                        {"topLeft", "topRight", "bottomLeft", "bottomRight", "center"}),
                  param("offsetX", "X 偏移", ParameterType::Integer, 0, -10000, 10000),
                  param("offsetY", "Y 偏移", ParameterType::Integer, 0, -10000, 10000),
                  param("scale", "缩放(%)", ParameterType::Double, 100.0, 1, 400),
                  param("opacity", "不透明度", ParameterType::Double, 0.8, 0, 1)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto base = inputImage(inputs, "base");
                     auto overlay = inputImage(inputs, "overlay");
                     if (base.isFail()) return Result<OutputMap>::fail(base.error());
                     if (overlay.isFail()) return Result<OutputMap>::fail(overlay.error());
                     auto result = ImageProcessors::imageOverlay(base.value(), overlay.value(),
                                                                 p.value("anchor").toString(),
                                                                 p.value("offsetX").toInt(), p.value("offsetY").toInt(),
                                                                 p.value("scale").toDouble(),
                                                                 p.value("opacity").toDouble());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "Pixelate", "像素化", "滤波处理", {in("image", "图片")}, {out()},
                 {param("blockSize", "像素块", ParameterType::Integer, 8, 2, 100)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::pixelate(img.value(), p.value("blockSize").toInt());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });
}
