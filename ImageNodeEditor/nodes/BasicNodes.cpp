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

PortInfo in(const QString& name, const QString& label, bool required = true)
{
    return PortInfo{name, label, PortDirection::Input, PortType::ImageRGBA, required, false};
}

PortInfo out(const QString& name = "output", const QString& label = "输出")
{
    return PortInfo{name, label, PortDirection::Output, PortType::ImageRGBA, true, true};
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
            if ((p.type == ParameterType::Integer || p.type == ParameterType::Double) && !value.isDouble()) {
                return Status::fail(QString("%1 参数必须是数字").arg(p.displayName));
            }
            if ((p.type == ParameterType::Text || p.type == ParameterType::FileOpen || p.type == ParameterType::FileSave
                 || p.type == ParameterType::Choice || p.type == ParameterType::Color) && !value.isString()) {
                return Status::fail(QString("%1 参数必须是字符串").arg(p.displayName));
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
                  param("x", "X", ParameterType::Integer, 30, 0, 100000),
                  param("y", "Y", ParameterType::Integer, 60, 0, 100000),
                  param("size", "字号", ParameterType::Integer, 32, 1, 256),
                  param("color", "颜色", ParameterType::Color, QString("#ffffff"))},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto img = inputImage(inputs, "image");
                     if (img.isFail()) return Result<OutputMap>::fail(img.error());
                     auto result = ImageProcessors::textOverlay(img.value(), p.value("text").toString(), p.value("x").toInt(),
                                                                p.value("y").toInt(), p.value("size").toInt(),
                                                                QColor(p.value("color").toString()));
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "Blend", "双图混合", "合成处理", {in("first", "图片 A"), in("second", "图片 B")}, {out()},
                 {param("opacity", "B 透明度", ParameterType::Double, 0.5, 0, 1)},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     auto a = inputImage(inputs, "first");
                     auto b = inputImage(inputs, "second");
                     if (a.isFail()) return Result<OutputMap>::fail(a.error());
                     if (b.isFail()) return Result<OutputMap>::fail(b.error());
                     auto result = ImageProcessors::blend(a.value(), b.value(), p.value("opacity").toDouble());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });

    registerNode(factory, "ImageMerge", "图片拼接", "合成处理",
                 {in("first", "图片 A", false), in("second", "图片 B", false), in("third", "图片 C", false)}, {out()},
                 {param("mode", "方向", ParameterType::Choice, QString("horizontal"), 0, 0, {"horizontal", "vertical"})},
                 [](const QJsonObject& p, const QMap<QString, NodeData>& inputs) {
                     QVector<QImage> images;
                     for (const QString& port : {"first", "second", "third"}) {
                         if (inputs.contains(port)) {
                             auto img = inputImage(inputs, port);
                             if (img.isOk()) images.append(img.value());
                         }
                     }
                     auto result = ImageProcessors::merge(images, p.value("mode").toString());
                     if (result.isFail()) return Result<OutputMap>::fail(result.error());
                     return imageOutput(result.value());
                 });
}
