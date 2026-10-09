// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// OpenGL valve rendering driven by actual plant position.

#include "valveview.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QQuaternion>
#include <QQuickOpenGLUtils>
#include <QtEndian>
#include <QtMath>
#include <algorithm>
#include <cstring>
#include <functional>
#include <stdexcept>
namespace
{
QMatrix4x4 transform(const QJsonObject &node)
{
    QMatrix4x4 matrix;
    if (node.contains("matrix"))
    {
        auto a = node["matrix"].toArray();
        if (a.size() != 16)
            throw std::runtime_error("Invalid matrix");
        for (int i = 0; i < 16; ++i)
            matrix(i % 4, i / 4) = float(a[i].toDouble());
        return matrix;
    }
    auto t = node["translation"].toArray(), r = node["rotation"].toArray(), s = node["scale"].toArray();
    if (t.size() == 3)
        matrix.translate(t[0].toDouble(), t[1].toDouble(), t[2].toDouble());
    if (r.size() == 4)
        matrix.rotate(QQuaternion(r[3].toDouble(), r[0].toDouble(), r[1].toDouble(), r[2].toDouble()));
    if (s.size() == 3)
        matrix.scale(s[0].toDouble(), s[1].toDouble(), s[2].toDouble());
    return matrix;
}
QVector<ValvePart> loadGlb(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        throw std::runtime_error("Valve GLB not found");
    const auto bytes = file.readAll();
    if (bytes.size() < 28)
        throw std::runtime_error("Invalid GLB header length");
    auto word = [&](qsizetype offset)
    {
        return quint32(quint8(bytes.at(offset))) | quint32(quint8(bytes.at(offset + 1))) << 8 |
               quint32(quint8(bytes.at(offset + 2))) << 16 | quint32(quint8(bytes.at(offset + 3))) << 24;
    };
    if (word(0) != 0x46546c67 || word(4) != 2)
        throw std::runtime_error("Invalid GLB header");
    const quint32 length = qFromLittleEndian<quint32>(bytes.constData() + 12);
    if (length > quint32(bytes.size() - 28))
        throw std::runtime_error("Invalid GLB JSON bounds");
    const auto root = QJsonDocument::fromJson(bytes.mid(20, length)).object();
    const qsizetype start = 28 + length;
    const auto accessors = root["accessors"].toArray(), views = root["bufferViews"].toArray(),
               nodes = root["nodes"].toArray(), meshes = root["meshes"].toArray(),
               materials = root["materials"].toArray();
    auto address = [&](int accessor, int index, int component) -> const char *
    {
        if (accessor < 0 || accessor >= accessors.size())
            throw std::runtime_error("Invalid GLB accessor");
        const auto a = accessors[accessor].toObject();
        const int viewIndex = a["bufferView"].toInt(-1);
        if (viewIndex < 0 || viewIndex >= views.size())
            throw std::runtime_error("Invalid GLB buffer view");
        auto v = views[viewIndex].toObject();
        const int type = a["componentType"].toInt();
        const int size = type == 5126 || type == 5125 ? 4 : type == 5123 ? 2 : 1;
        const int width = a["type"].toString() == "VEC3" ? 3 : 1;
        const int stride = v["byteStride"].toInt(size * width);
        const qsizetype offset =
            start + v["byteOffset"].toInt() + a["byteOffset"].toInt() + index * stride + component * size;
        if (index < 0 || index >= a["count"].toInt() || offset < start || offset + size > bytes.size())
            throw std::runtime_error("GLB accessor out of bounds");
        return bytes.constData() + offset;
    };
    QVector<ValvePart> result;
    bool foundRotor = false;
    std::function<void(int, QMatrix4x4, bool, QMatrix4x4)> visit;
    visit = [&](int index, QMatrix4x4 prefix, bool rotor, QMatrix4x4 suffix)
    {
        if (index < 0 || index >= nodes.size())
            throw std::runtime_error("Invalid GLB node");
        const auto node = nodes[index].toObject();
        auto local = transform(node);
        if (node["name"].toString() == "VALVE_ROTOR")
        {
            if (rotor)
                throw std::runtime_error("Nested rotor");
            rotor = true;
            foundRotor = true;
            suffix = local;
        }
        else if (rotor)
            suffix *= local;
        else
            prefix *= local;
        if (node.contains("mesh"))
        {
            const int meshIndex = node["mesh"].toInt(-1);
            if (meshIndex < 0 || meshIndex >= meshes.size())
                throw std::runtime_error("Invalid mesh");
            for (const auto &primitiveValue : meshes[meshIndex].toObject()["primitives"].toArray())
            {
                const auto primitive = primitiveValue.toObject();
                if (primitive["mode"].toInt(4) != 4)
                    throw std::runtime_error("Only triangles supported");
                const auto attributes = primitive["attributes"].toObject();
                const int p = attributes["POSITION"].toInt(-1), n = attributes["NORMAL"].toInt(-1),
                          indices = primitive["indices"].toInt(-1);
                if (p < 0 || n < 0 || indices < 0 || indices >= accessors.size())
                    throw std::runtime_error("Missing geometry accessors");
                const auto ia = accessors[indices].toObject();
                const int count = ia["count"].toInt();
                ValvePart part;
                part.name = node["name"].toString();
                part.rotor = rotor;
                part.prefix = prefix;
                part.suffix = suffix;
                const auto material =
                    materials[primitive["material"].toInt()].toObject()["pbrMetallicRoughness"].toObject();
                auto color = material["baseColorFactor"].toArray();
                part.color = color.size() >= 3
                                 ? QVector3D(color[0].toDouble(), color[1].toDouble(), color[2].toDouble())
                                 : QVector3D(.5, .5, .5);
                part.metallic = material["metallicFactor"].toDouble(.2);
                part.roughness = material["roughnessFactor"].toDouble(.5);
                if (count < 0 || count > 2000000)
                    throw std::runtime_error("Geometry exceeds import limit");
                part.vertices.resize(count * 6 * sizeof(float));
                for (int i = 0; i < count; ++i)
                {
                    const char *ptr = address(indices, i, 0);
                    const int componentType = ia["componentType"].toInt();
                    const quint32 vertex = componentType == 5125   ? qFromLittleEndian<quint32>(ptr)
                                           : componentType == 5123 ? qFromLittleEndian<quint16>(ptr)
                                                                   : quint8(*ptr);
                    for (int c = 0; c < 6; ++c)
                    {
                        const quint32 raw =
                            qFromLittleEndian<quint32>(address(c < 3 ? p : n, int(vertex), c % 3));
                        float v;
                        std::memcpy(&v, &raw, 4);
                        std::memcpy(part.vertices.data() + (i * 6 + c) * 4, &v, 4);
                    }
                }
                result.append(part);
            }
        }
        for (const auto &child : node["children"].toArray())
            visit(child.toInt(), prefix, rotor, suffix);
    };
    for (const auto &node : root["scenes"].toArray()[root["scene"].toInt(0)].toObject()["nodes"].toArray())
        visit(node.toInt(), QMatrix4x4(), false, QMatrix4x4());
    if (!foundRotor)
        throw std::runtime_error("VALVE_ROTOR is missing");
    // Keep rigid and moving groups separate; combine parts sharing material and transform.
    // The supplied asset needs far fewer GPU submissions than its CAD part count.
    QVector<ValvePart> batches;
    for (const auto &part : result)
    {
        auto matching = std::find_if(batches.begin(), batches.end(),
                                     [&](const ValvePart &batch)
                                     {
                                         return part.rotor == batch.rotor && part.prefix == batch.prefix &&
                                                part.suffix == batch.suffix && part.color == batch.color &&
                                                part.metallic == batch.metallic &&
                                                part.roughness == batch.roughness;
                                     });
        if (matching == batches.end())
            batches.append(part);
        else
            matching->vertices.append(part.vertices);
    }
    return batches;
}
class Renderer final : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions_3_3_Core
{
    struct Gpu
    {
        GLuint vao = 0, vbo = 0;
        GLsizei count = 0;
    };
    std::shared_ptr<const ValveAssets> assets;
    QVector<Gpu> normal, cutaway;
    QOpenGLShaderProgram program;
    bool initialized = false, section = true;
    double position = 0, azimuth = 34, elevation = 23, zoom = 1;

  public:
    explicit Renderer(std::shared_ptr<const ValveAssets> a) : assets(std::move(a)) {}
    ~Renderer() override
    {
        if (initialized)
            for (auto *list : {&normal, &cutaway})
                for (auto &g : *list)
                {
                    glDeleteBuffers(1, &g.vbo);
                    glDeleteVertexArrays(1, &g.vao);
                }
    }
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override
    {
        QOpenGLFramebufferObjectFormat f;
        f.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        f.setSamples(4);
        return new QOpenGLFramebufferObject(size, f);
    }
    void synchronize(QQuickFramebufferObject *item) override
    {
        auto *v = static_cast<ValveView *>(item);
        position = v->position();
        section = v->cutaway();
        azimuth = v->azimuth();
        elevation = v->elevation();
        zoom = v->zoom();
    }
    void upload(const QVector<ValvePart> &parts, QVector<Gpu> &output)
    {
        for (const auto &part : parts)
        {
            Gpu g;
            glGenVertexArrays(1, &g.vao);
            glBindVertexArray(g.vao);
            glGenBuffers(1, &g.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
            glBufferData(GL_ARRAY_BUFFER, part.vertices.size(), part.vertices.constData(), GL_STATIC_DRAW);
            for (int a = 0; a < 2; ++a)
            {
                glEnableVertexAttribArray(a);
                glVertexAttribPointer(a, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                                      reinterpret_cast<void *>(a * 3 * sizeof(float)));
            }
            g.count = part.vertices.size() / (6 * sizeof(float));
            output.append(g);
        }
    }
    void render() override
    {
        if (!initialized)
        {
            if (!initializeOpenGLFunctions())
                return;
            program.addShaderFromSourceCode(QOpenGLShader::Vertex, R"(#version 330 core
                layout(location=0) in vec3 position; layout(location=1) in vec3 normal;
                uniform mat4 mvp; uniform mat4 model; uniform mat3 normalMatrix;
                out vec3 world; out vec3 n;
                void main(){gl_Position=mvp*vec4(position,1);world=(model*vec4(position,1)).xyz;n=normalize(normalMatrix*normal);}
            )");
            program.addShaderFromSourceCode(QOpenGLShader::Fragment, R"(#version 330 core
                in vec3 world; in vec3 n; uniform vec3 color;uniform vec3 eye;uniform float metallic;uniform float roughness;out vec4 outColor;
                void main(){vec3 N=normalize(n);if(!gl_FrontFacing)N=-N;vec3 V=normalize(eye-world);vec3 L=normalize(vec3(-.4,.8,1));
                    float diff=max(dot(N,L),0);float rim=max(dot(N,normalize(vec3(1,.3,-1))),0);
                    float spec=pow(max(dot(N,normalize(L+V)),0),mix(90,12,roughness));
                    vec3 c=color*(.42+.65*diff+.26*rim)+mix(vec3(.16),color,metallic)*spec*.7;
                    outColor=vec4(pow(clamp(c,0,1),vec3(1.0/2.2)),1);}
            )");
            if (!program.link())
            {
                qWarning() << program.log();
                return;
            }
            upload(assets->normal, normal);
            upload(assets->cutaway, cutaway);
            initialized = true;
        }
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(.063f, .086f, .145f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        const float az = qDegreesToRadians(float(azimuth)), el = qDegreesToRadians(float(elevation));
        const QVector3D center(0, .040, 0),
            direction(std::sin(az) * std::cos(el), std::sin(el), std::cos(az) * std::cos(el));
        const QVector3D eye = center + direction * .48;
        QMatrix4x4 view;
        view.lookAt(eye, center, QVector3D(0, 1, 0));
        QMatrix4x4 projection;
        const float aspect = float(framebufferObject()->width()) / framebufferObject()->height();
        const float half = .115f / zoom;
        projection.ortho(-half * aspect, half * aspect, -half, half, .01f, 2.f);
        program.bind();
        program.setUniformValue("eye", eye);
        const auto &parts = section ? assets->cutaway : assets->normal;
        const auto &gpu = section ? cutaway : normal;
        QMatrix4x4 rotation;
        rotation.rotate(float(std::clamp(position, 0.0, 100.0) * .9), 0, 0, 1);
        for (qsizetype i = 0; i < parts.size(); ++i)
        {
            const auto &part = parts[i];
            const auto model = part.rotor ? part.prefix * rotation * part.suffix : part.prefix;
            program.setUniformValue("model", model);
            program.setUniformValue("normalMatrix", model.normalMatrix());
            program.setUniformValue("mvp", projection * view * model);
            program.setUniformValue("color", part.color);
            program.setUniformValue("metallic", part.metallic);
            program.setUniformValue("roughness", part.roughness);
            glBindVertexArray(gpu[i].vao);
            glDrawArrays(GL_TRIANGLES, 0, gpu[i].count);
        }
        glBindVertexArray(0);
        program.release();
        QQuickOpenGLUtils::resetOpenGLState();
    }
};
} // namespace
ValveView::ValveView(QQuickItem *parent) : QQuickFramebufferObject(parent)
{
    setMirrorVertically(true);
    auto assets = std::make_shared<ValveAssets>();
    try
    {
        QFile manifestFile(":/assets/asset_manifest.json");
        if (!manifestFile.open(QIODevice::ReadOnly))
            throw std::runtime_error("Asset manifest is missing");
        const auto manifest = QJsonDocument::fromJson(manifestFile.readAll()).object();
        const auto mapping = manifest["position_mapping"].toObject();
        if (manifest["moving_group"].toString() != "VALVE_ROTOR" ||
            manifest["gltf_units"].toString() != "m" ||
            manifest["axis_in_rotor_local_coordinates"].toArray() != QJsonArray{0, 0, 1} ||
            manifest["pivot_cad_mm"].toArray() != QJsonArray{0, 0, 0} ||
            mapping["closed_angle_deg"].toDouble() != 0 || mapping["open_angle_deg"].toDouble() != 90)
            throw std::runtime_error("Unsupported valve units, pivot or rotation mapping");
        assets->normal = loadGlb(":/assets/valve-normal.glb");
        assets->cutaway = loadGlb(":/assets/valve-cutaway.glb");
    }
    catch (const std::exception &error)
    {
        assets->error = QString::fromUtf8(error.what());
        qWarning() << assets->error;
    }
    m_assets = assets;
}
QQuickFramebufferObject::Renderer *ValveView::createRenderer() const
{
    return new ::Renderer(m_assets);
}
void ValveView::setPosition(double v)
{
    v = std::clamp(v, 0.0, 100.0);
    if (m_position == v)
        return;
    m_position = v;
    emit positionChanged();
    update();
}
void ValveView::setCutaway(bool v)
{
    if (m_cutaway == v)
        return;
    m_cutaway = v;
    emit cutawayChanged();
    update();
}
void ValveView::setAzimuth(double v)
{
    if (!std::isfinite(v) || m_azimuth == v)
        return;
    m_azimuth = v;
    emit cameraChanged();
    update();
}
void ValveView::setElevation(double v)
{
    if (!std::isfinite(v))
        return;
    v = std::clamp(v, -10.0, 75.0);
    if (m_elevation == v)
        return;
    m_elevation = v;
    emit cameraChanged();
    update();
}
void ValveView::setZoom(double v)
{
    if (!std::isfinite(v))
        return;
    v = std::clamp(v, .6, 2.0);
    if (m_zoom == v)
        return;
    m_zoom = v;
    emit cameraChanged();
    update();
}
void ValveView::orbit(double deltaX, double deltaY)
{
    if (!std::isfinite(deltaX) || !std::isfinite(deltaY))
        return;
    const double azimuth = std::remainder(m_azimuth + deltaX * .4, 360.0);
    const double elevation = std::clamp(m_elevation + deltaY * .3, -10.0, 75.0);
    if (azimuth == m_azimuth && elevation == m_elevation)
        return;
    m_azimuth = azimuth;
    m_elevation = elevation;
    emit cameraChanged();
    update();
}
