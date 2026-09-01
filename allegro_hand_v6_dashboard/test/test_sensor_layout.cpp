// Copyright (c) 2026 Wonik Robotics
// SPDX-License-Identifier: BSD-3-Clause

#include <set>
#include <string>

#include <gtest/gtest.h>

#include "allegro_hand_v6_dashboard/sensor_layout.hpp"
#include "allegro_hand_v6_dashboard/sensor_panel.hpp"

using allegro_hand_v6_dashboard::pressure_color;

TEST(PressureColor, MatchesFirmwareBlueGreenRedSweep)
{
  EXPECT_EQ(pressure_color(-1.0), QColor(0, 0, 255));
  EXPECT_EQ(pressure_color(0.0), QColor(0, 0, 255));
  EXPECT_EQ(pressure_color(0.25), QColor(0, 128, 128));
  EXPECT_EQ(pressure_color(0.5), QColor(0, 255, 0));
  EXPECT_EQ(pressure_color(0.75), QColor(128, 128, 0));
  EXPECT_EQ(pressure_color(1.0), QColor(255, 0, 0));
  EXPECT_EQ(pressure_color(2.0), QColor(255, 0, 0));
  EXPECT_EQ(pressure_color(0.5, false), QColor(125, 135, 150));
}

TEST(SensorLayout, LoadsEighteenChannelContract)
{
  const auto layout = allegro_hand_v6_dashboard::SensorLayout::load(TEST_CONFIG_PATH);
  EXPECT_EQ(layout.channel_count(), 18U);
  EXPECT_EQ(layout.sensors.size(), 18U);
  EXPECT_EQ(layout.unit, "hPa");
  EXPECT_EQ(layout.pressure_min, 1000.0);
  EXPECT_EQ(layout.pressure_max, 1200.0);
  EXPECT_EQ(layout.svg_width, 1254.0);
  EXPECT_EQ(layout.svg_height, 1254.0);
  EXPECT_EQ(layout.image_crop, (std::array<double, 4>{198.0, 0.0, 1056.0, 1254.0}));
  EXPECT_EQ(layout.image_scale, 1.0);
  EXPECT_EQ(layout.image_offset, (std::array<double, 2>{0.0, 0.0}));
  EXPECT_EQ(layout.sensors.at(0).text_rotation, 59.0);
  EXPECT_EQ(layout.sensors.at(1).text_rotation, 59.0);
  EXPECT_EQ(layout.sensors.at(2).text_rotation, 59.0);
  EXPECT_EQ(layout.sensors.at(3).text_rotation, 0.0);
  EXPECT_EQ(layout.sensors.at(0).position,
    (std::array<double, 3>{0.0125, -0.0217, 0.001}));
  for (const std::size_t index : {3U, 6U, 9U, 12U}) {
    EXPECT_EQ(layout.sensors.at(index).position,
      (std::array<double, 3>{0.022, 0.0, 0.006}));
  }

  std::set<std::string> groups;
  for (const auto & sensor : layout.sensors) {
    groups.insert(sensor.group);
    EXPECT_GE(sensor.svg[0], layout.image_crop[0]);
    EXPECT_LE(sensor.svg[0], layout.image_crop[0] + layout.image_crop[2]);
    EXPECT_GE(sensor.svg[1], layout.image_crop[1]);
    EXPECT_LE(sensor.svg[1], layout.image_crop[1] + layout.image_crop[3]);
  }
  EXPECT_EQ(groups, (std::set<std::string>{"Palm", "Thumb", "Index", "Middle", "Ring", "Pinky"}));
  const std::array<std::string, 18> expected_order = {
    "thumb_tip", "thumb_mid", "thumb_base",
    "index_tip", "index_mid", "index_base",
    "middle_tip", "middle_mid", "middle_base",
    "ring_tip", "ring_mid", "ring_base",
    "pinky_tip", "pinky_mid", "pinky_base",
    "palm_bottom", "palm_center", "palm_left"};
  for (std::size_t index = 0; index < expected_order.size(); ++index) {
    EXPECT_EQ(layout.sensors.at(index).index, index);
    EXPECT_EQ(layout.sensors.at(index).name, expected_order[index]);
  }
}

TEST(SensorLayout, LoadsLeftHandImageLayout)
{
  const auto layout = allegro_hand_v6_dashboard::SensorLayout::load(LEFT_TEST_CONFIG_PATH);
  EXPECT_EQ(layout.channel_count(), 18U);
  EXPECT_EQ(layout.unit, "hPa");
  EXPECT_EQ(layout.pressure_min, 1000.0);
  EXPECT_EQ(layout.pressure_max, 1200.0);
  EXPECT_EQ(layout.svg_width, 1322.0);
  EXPECT_EQ(layout.svg_height, 1190.0);
  EXPECT_EQ(layout.image_crop, (std::array<double, 4>{0.0, 0.0, 1322.0, 1190.0}));
  EXPECT_EQ(layout.image_scale, 0.95);
  EXPECT_EQ(layout.image_offset, (std::array<double, 2>{85.0, 0.0}));
  EXPECT_EQ(layout.sensors.at(0).text_rotation, -56.0);
  EXPECT_EQ(layout.sensors.at(1).text_rotation, -56.0);
  EXPECT_EQ(layout.sensors.at(2).text_rotation, -56.0);
  EXPECT_EQ(layout.sensors.at(3).text_rotation, 0.0);
  EXPECT_EQ(layout.sensors.at(0).position,
    (std::array<double, 3>{0.0125, 0.0217, 0.001}));
  for (const std::size_t index : {3U, 6U, 9U, 12U}) {
    EXPECT_EQ(layout.sensors.at(index).position,
      (std::array<double, 3>{0.022, 0.0, 0.006}));
  }
  for (const auto & sensor : layout.sensors) {
    EXPECT_GE(sensor.svg[0], layout.image_crop[0]);
    EXPECT_LE(sensor.svg[0], layout.image_crop[0] + layout.image_crop[2]);
    EXPECT_GE(sensor.svg[1], layout.image_crop[1]);
    EXPECT_LE(sensor.svg[1], layout.image_crop[1] + layout.image_crop[3]);
  }
  EXPECT_EQ(layout.sensors.at(15).name, "palm_bottom");
  EXPECT_EQ(layout.sensors.at(16).name, "palm_center");
  EXPECT_EQ(layout.sensors.at(17).name, "palm_left");
}

TEST(SensorLayout, NormalizesMarkerDirections)
{
  const auto layout = allegro_hand_v6_dashboard::SensorLayout::load(TEST_CONFIG_PATH);
  for (const auto & sensor : layout.sensors) {
    const double length_squared = sensor.normal[0] * sensor.normal[0] +
      sensor.normal[1] * sensor.normal[1] + sensor.normal[2] * sensor.normal[2];
    EXPECT_NEAR(length_squared, 1.0, 1e-12) << sensor.name;
  }
}
