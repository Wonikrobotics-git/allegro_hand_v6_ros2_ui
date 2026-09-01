// Copyright (c) 2026 Wonik Robotics
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ALLEGRO_HAND_V6_DASHBOARD__SENSOR_LAYOUT_HPP_
#define ALLEGRO_HAND_V6_DASHBOARD__SENSOR_LAYOUT_HPP_

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace allegro_hand_v6_dashboard
{

struct SensorDefinition
{
  std::size_t index{};
  std::string name;
  std::string label;
  std::string group;
  std::string frame_id;
  std::array<double, 3> position{};
  std::array<double, 3> normal{};
  std::array<double, 2> svg{};
  double text_rotation{0.0};
};

struct SensorLayout
{
  std::string unit{"hPa"};
  double pressure_min{1000.0};
  double pressure_max{1200.0};
  double svg_width{600.0};
  double svg_height{900.0};
  std::array<double, 4> image_crop{0.0, 0.0, 600.0, 900.0};
  double image_scale{1.0};
  std::array<double, 2> image_offset{0.0, 0.0};
  std::vector<SensorDefinition> sensors;

  std::size_t channel_count() const;
  static SensorLayout load(const std::string & path);
};

}  // namespace allegro_hand_v6_dashboard

#endif  // ALLEGRO_HAND_V6_DASHBOARD__SENSOR_LAYOUT_HPP_
