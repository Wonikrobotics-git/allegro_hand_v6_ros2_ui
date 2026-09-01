// Copyright (c) 2026 Wonik Robotics
// SPDX-License-Identifier: BSD-3-Clause

#include "allegro_hand_v6_dashboard/sensor_panel.hpp"

#include <algorithm>
#include <cmath>

#include <QFont>
#include <QPainter>
#include <QPaintEvent>

namespace allegro_hand_v6_dashboard
{

QColor pressure_color(double normalized, bool valid)
{
  if (!valid) {
    return QColor(125, 135, 150);
  }
  const double t = std::clamp(normalized, 0.0, 1.0);
  constexpr int max_color = 255;
  if (t <= 0.5) {
    const double local = t * 2.0;
    return QColor(
      0,
      static_cast<int>(std::round(local * max_color)),
      static_cast<int>(std::round((1.0 - local) * max_color)));
  }
  const double local = (t - 0.5) * 2.0;
  return QColor(
    static_cast<int>(std::round(local * max_color)),
    static_cast<int>(std::round((1.0 - local) * max_color)),
    0);
}

SensorPanel::SensorPanel(
  SensorLayout layout, const QString & svg_path,
  double stale_timeout_seconds, QWidget * parent)
: QWidget(parent),
  layout_(std::move(layout)),
  hand_pixmap_(svg_path),
  values_(layout_.channel_count(), std::numeric_limits<float>::quiet_NaN()),
  stale_timeout_seconds_(stale_timeout_seconds)
{
  setMinimumSize(420, 620);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setAutoFillBackground(false);
}

void SensorPanel::set_readings(const std::vector<float> & values)
{
  values_ = values;
  size_matches_ = values.size() == layout_.channel_count();
  last_update_ = std::chrono::steady_clock::now();
  has_data_ = true;
  update();
}

void SensorPanel::refresh_status()
{
  update();
}

QRectF SensorPanel::image_rect() const
{
  const QRectF available(24.0, 82.0, std::max(1, width() - 48), std::max(1, height() - 150));
  const double scale = layout_.image_scale * std::min(
    available.width() / layout_.image_crop[2],
    available.height() / layout_.image_crop[3]);
  const QSizeF size(layout_.image_crop[2] * scale, layout_.image_crop[3] * scale);
  const QPointF center = available.center() + QPointF(
    layout_.image_offset[0] * scale, layout_.image_offset[1] * scale);
  return QRectF(
    center.x() - size.width() / 2.0,
    center.y() - size.height() / 2.0,
    size.width(), size.height());
}

void SensorPanel::paintEvent(QPaintEvent * event)
{
  QWidget::paintEvent(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(rect(), QColor("#f7f9fc"));

  QFont title_font = painter.font();
  title_font.setPointSize(16);
  title_font.setBold(true);
  painter.setFont(title_font);
  painter.setPen(QColor("#16357d"));
  painter.drawText(QRectF(20, 14, width() - 40, 32), Qt::AlignCenter, "V6 Tactile Sensor Map");

  const auto now = std::chrono::steady_clock::now();
  const bool stale = has_data_ &&
    std::chrono::duration<double>(now - last_update_).count() > stale_timeout_seconds_;
  QString status = "NO DATA · expected " + QString::number(layout_.channel_count()) + " channels";
  QColor status_color("#7d8796");
  if (has_data_ && !size_matches_) {
    status = "CHANNEL MISMATCH · received " + QString::number(values_.size()) +
      " / expected " + QString::number(layout_.channel_count());
    status_color = QColor("#d43d37");
  } else if (stale) {
    status = "STALE · no recent tactile message";
    status_color = QColor("#d98918");
  } else if (has_data_) {
    status = "LIVE · " + QString::number(layout_.channel_count()) + " channels · " +
      QString::fromStdString(layout_.unit);
    status_color = QColor("#24925e");
  }
  QFont status_font = painter.font();
  status_font.setPointSize(10);
  status_font.setBold(true);
  painter.setFont(status_font);
  painter.setPen(status_color);
  painter.drawText(QRectF(20, 49, width() - 40, 24), Qt::AlignCenter, status);

  const QRectF target = image_rect();
  if (!hand_pixmap_.isNull()) {
    painter.drawPixmap(
      target, hand_pixmap_,
      QRectF(
        layout_.image_crop[0], layout_.image_crop[1],
        layout_.image_crop[2], layout_.image_crop[3]));
  } else {
    painter.setPen(QPen(QColor("#16357d"), 3));
    painter.setBrush(QColor("#dce5f3"));
    painter.drawRoundedRect(target.adjusted(target.width() * .2, target.height() * .3,
      -target.width() * .2, -target.height() * .1), 50, 50);
  }

  QFont value_font = painter.font();
  value_font.setPointSize(
    std::clamp(static_cast<int>(target.width() / 45.0), 11, 16));
  value_font.setBold(true);
  painter.setFont(value_font);

  for (const auto & sensor : layout_.sensors) {
    const bool value_present = sensor.index < values_.size() && std::isfinite(values_[sensor.index]);
    const double value = value_present ? values_[sensor.index] : 0.0;
    const double normalized = (value - layout_.pressure_min) /
      (layout_.pressure_max - layout_.pressure_min);
    const bool valid = value_present && !stale && size_matches_;
    const QColor color = pressure_color(normalized, valid);
    const QPointF center(
      target.left() + (sensor.svg[0] - layout_.image_crop[0]) /
      layout_.image_crop[2] * target.width(),
      target.top() + (sensor.svg[1] - layout_.image_crop[1]) /
      layout_.image_crop[3] * target.height());
    const QString value_text = valid ? QString::number(value, 'f', 1) : "--";
    const QRectF text_rect(-34.0, -15.0, 68.0, 30.0);

    // Keep the overlay text-only. A one-pixel halo preserves readability on the
    // black sensor surfaces without adding a circle or another marker shape.
    painter.save();
    painter.translate(center);
    painter.rotate(sensor.text_rotation);
    painter.setPen(QColor(255, 255, 255, 220));
    for (const QPointF & offset : {
        QPointF(-1.0, 0.0), QPointF(1.0, 0.0),
        QPointF(0.0, -1.0), QPointF(0.0, 1.0)})
    {
      painter.drawText(text_rect.translated(offset), Qt::AlignCenter, value_text);
    }
    painter.setPen(color);
    painter.drawText(text_rect, Qt::AlignCenter, value_text);
    painter.restore();
  }

  const QRectF legend(30, height() - 50, width() - 60, 22);
  QLinearGradient gradient(legend.topLeft(), legend.topRight());
  gradient.setColorAt(0.0, pressure_color(0.0));
  gradient.setColorAt(0.5, pressure_color(0.5));
  gradient.setColorAt(1.0, pressure_color(1.0));
  painter.setPen(Qt::NoPen);
  painter.setBrush(gradient);
  painter.drawRoundedRect(legend, 5, 5);
  painter.setPen(QColor("#26364f"));
  QFont legend_font = painter.font();
  legend_font.setPointSize(8);
  painter.setFont(legend_font);
  painter.drawText(legend.adjusted(0, 23, 0, 40), Qt::AlignLeft,
    QString::number(layout_.pressure_min, 'f', 1));
  painter.drawText(legend.adjusted(0, 23, 0, 40), Qt::AlignCenter,
    QString::number((layout_.pressure_min + layout_.pressure_max) / 2.0, 'f', 1));
  painter.drawText(legend.adjusted(0, 23, 0, 40), Qt::AlignRight,
    QString::number(layout_.pressure_max, 'f', 1) + " " + QString::fromStdString(layout_.unit));
}

}  // namespace allegro_hand_v6_dashboard
