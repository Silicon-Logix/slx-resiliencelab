// Silicon LogiX / SLX ResilienceLab
// Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
// Author: Marco Pezzullo
// License: Silicon LogiX Evaluation License 1.0. See LICENSE.
// OpenGL valve rendering driven by actual plant position.

#pragma once
#include <QByteArray>
#include <QMatrix4x4>
#include <QQuickFramebufferObject>
#include <QVector3D>
#include <memory>
struct ValvePart
{
    QString name;
    QByteArray vertices;
    QMatrix4x4 prefix, suffix;
    QVector3D color;
    float metallic = .2f, roughness = .5f;
    bool rotor = false;
};
struct ValveAssets
{
    QVector<ValvePart> normal, cutaway;
    QString error;
};
class ValveView : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(double position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool cutaway READ cutaway WRITE setCutaway NOTIFY cutawayChanged)
    Q_PROPERTY(double azimuth READ azimuth WRITE setAzimuth NOTIFY cameraChanged)
    Q_PROPERTY(double elevation READ elevation WRITE setElevation NOTIFY cameraChanged)
    Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY cameraChanged)
    Q_PROPERTY(QString assetError READ assetError CONSTANT)
  public:
    explicit ValveView(QQuickItem *parent = nullptr);
    Renderer *createRenderer() const override;
    double position() const
    {
        return m_position;
    }
    void setPosition(double v);
    bool cutaway() const
    {
        return m_cutaway;
    }
    void setCutaway(bool v);
    double azimuth() const
    {
        return m_azimuth;
    }
    void setAzimuth(double v);
    double elevation() const
    {
        return m_elevation;
    }
    void setElevation(double v);
    double zoom() const
    {
        return m_zoom;
    }
    void setZoom(double v);
    Q_INVOKABLE void orbit(double deltaX, double deltaY);
    QString assetError() const
    {
        return m_assets->error;
    }
    std::shared_ptr<const ValveAssets> assets() const
    {
        return m_assets;
    }
  signals:
    void positionChanged();
    void cutawayChanged();
    void cameraChanged();

  private:
    double m_position = 0, m_azimuth = 34, m_elevation = 23, m_zoom = 1;
    bool m_cutaway = true;
    std::shared_ptr<const ValveAssets> m_assets;
};
