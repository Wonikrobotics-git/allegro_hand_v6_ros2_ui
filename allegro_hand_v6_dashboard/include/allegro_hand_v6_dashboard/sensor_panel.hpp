// Copyright (c) 2026 Wonik Robotics
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ALLEGRO_HAND_V6_DASHBOARD__SENSOR_PANEL_HPP_
#define ALLEGRO_HAND_V6_DASHBOARD__SENSOR_PANEL_HPP_

#include <chrono>
#include <string>
#include <vector>

#include <QColor>
#include <QPixmap>
#include <QWidget>

#include "allegro_hand_v6_dashboard/sensor_layout.hpp"

namespace allegro_hand_v6_dashboard
{

QColor pressure_color(double normalized, bool valid = true);

class SensorPanel : public QWidget
{
public:
  SensorPanel(
    SensorLayout layout, const QString & svg_path,
    double stale_timeout_seconds, QWidget * parent = nullptr);

  void set_readings(const std::vector<float> & values);
  void refresh_status();

protected:
  void paintEvent(QPaintEvent * event) override;

private:
  QRectF image_rect() const;

  SensorLayout layout_;
  QPixmap hand_pixmap_;
  std::vector<float> values_;
  std::chrono::steady_clock::time_point last_update_{};
  bool has_data_{false};
  bool size_matches_{false};
  double stale_timeout_seconds_{1.0};
};

}  // namespace allegro_hand_v6_dashboard

#endif  // ALLEGRO_HAND_V6_DASHBOARD__SENSOR_PANEL_HPP_
