// Copyright (c) 2026 Wonik Robotics
// SPDX-License-Identifier: BSD-3-Clause

#include "allegro_hand_v6_dashboard/sensor_layout.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>

#include <yaml-cpp/yaml.h>

namespace allegro_hand_v6_dashboard
{
namespace
{

template<std::size_t N>
std::array<double, N> read_array(const YAML::Node & node, const char * field)
{
  const auto value = node[field];
  if (!value || !value.IsSequence() || value.size() != N) {
    throw std::runtime_error(std::string("sensor field '") + field + "' must contain " +
      std::to_string(N) + " numbers");
  }
  std::array<double, N> result{};
  for (std::size_t i = 0; i < N; ++i) {
    result[i] = value[i].as<double>();
    if (!std::isfinite(result[i])) {
      throw std::runtime_error(std::string("sensor field '") + field + "' contains a non-finite value");
    }
  }
  return result;
}

}  // namespace

std::size_t SensorLayout::channel_count() const
{
  std::size_t result = 0;
  for (const auto & sensor : sensors) {
    result = std::max(result, sensor.index + 1);
  }
  return result;
}

SensorLayout SensorLayout::load(const std::string & path)
{
  const YAML::Node root = YAML::LoadFile(path);
  SensorLayout result;
  result.unit = root["unit"].as<std::string>("hPa");
  result.pressure_min = root["pressure_min"].as<double>(1000.0);
  result.pressure_max = root["pressure_max"].as<double>(1200.0);
  result.svg_width = root["svg_width"].as<double>(600.0);
  result.svg_height = root["svg_height"].as<double>(900.0);
  result.image_crop = root["image_crop"] ?
    read_array<4>(root, "image_crop") :
    std::array<double, 4>{0.0, 0.0, result.svg_width, result.svg_height};
  result.image_scale = root["image_scale"].as<double>(1.0);
  result.image_offset = root["image_offset"] ?
    read_array<2>(root, "image_offset") : std::array<double, 2>{0.0, 0.0};

  if (!std::isfinite(result.pressure_min) || !std::isfinite(result.pressure_max) ||
    result.pressure_max <= result.pressure_min)
  {
    throw std::runtime_error("pressure_max must be greater than pressure_min");
  }
  if (result.svg_width <= 0.0 || result.svg_height <= 0.0) {
    throw std::runtime_error("source image dimensions must be positive");
  }
  if (result.image_crop[0] < 0.0 || result.image_crop[1] < 0.0 ||
    result.image_crop[2] <= 0.0 || result.image_crop[3] <= 0.0 ||
    result.image_crop[0] + result.image_crop[2] > result.svg_width ||
    result.image_crop[1] + result.image_crop[3] > result.svg_height)
  {
    throw std::runtime_error("image_crop must be inside the source image");
  }
  if (!std::isfinite(result.image_scale) || result.image_scale <= 0.0) {
    throw std::runtime_error("image_scale must be positive");
  }

  const auto sensors = root["sensors"];
  if (!sensors || !sensors.IsSequence() || sensors.size() == 0) {
    throw std::runtime_error("sensor layout must contain at least one sensor");
  }

  std::set<std::size_t> indices;
  std::set<std::string> names;
  for (const auto & node : sensors) {
    SensorDefinition sensor;
    sensor.index = node["index"].as<std::size_t>();
    sensor.name = node["name"].as<std::string>();
    sensor.label = node["label"].as<std::string>(sensor.name);
    sensor.group = node["group"].as<std::string>();
    sensor.frame_id = node["frame_id"].as<std::string>();
    sensor.position = read_array<3>(node, "position");
    sensor.normal = read_array<3>(node, "normal");
    sensor.svg = read_array<2>(node, "svg");
    sensor.text_rotation = node["text_rotation"].as<double>(0.0);

    if (sensor.index >= 256) {
      throw std::runtime_error("sensor index must be less than 256");
    }
    if (sensor.name.empty() || sensor.group.empty() || sensor.frame_id.empty()) {
      throw std::runtime_error("sensor name, group and frame_id must not be empty");
    }
    if (!std::isfinite(sensor.text_rotation)) {
      throw std::runtime_error("sensor text_rotation must be finite: " + sensor.name);
    }
    if (sensor.svg[0] < result.image_crop[0] ||
      sensor.svg[0] > result.image_crop[0] + result.image_crop[2] ||
      sensor.svg[1] < result.image_crop[1] ||
      sensor.svg[1] > result.image_crop[1] + result.image_crop[3])
    {
      throw std::runtime_error("sensor overlay is outside image_crop: " + sensor.name);
    }
    if (!indices.insert(sensor.index).second) {
      throw std::runtime_error("duplicate sensor index: " + std::to_string(sensor.index));
    }
    if (!names.insert(sensor.name).second) {
      throw std::runtime_error("duplicate sensor name: " + sensor.name);
    }
    const double length = std::sqrt(
      sensor.normal[0] * sensor.normal[0] + sensor.normal[1] * sensor.normal[1] +
      sensor.normal[2] * sensor.normal[2]);
    if (length < 1e-9) {
      throw std::runtime_error("sensor normal must be non-zero: " + sensor.name);
    }
    for (double & component : sensor.normal) {
      component /= length;
    }
    result.sensors.push_back(std::move(sensor));
  }

  std::sort(result.sensors.begin(), result.sensors.end(),
    [](const auto & left, const auto & right) {return left.index < right.index;});
  if (result.channel_count() != result.sensors.size()) {
    throw std::runtime_error("sensor indices must be contiguous and start at zero");
  }
  return result;
}

}  // namespace allegro_hand_v6_dashboard
