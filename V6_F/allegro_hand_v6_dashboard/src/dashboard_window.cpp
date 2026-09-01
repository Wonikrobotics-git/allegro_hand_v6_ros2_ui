// Copyright (c) 2026 Wonik Robotics
// SPDX-License-Identifier: BSD-3-Clause

#include "allegro_hand_v6_dashboard/dashboard_window.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#include <QCoreApplication>
#include <QLabel>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>

#include "geometry_msgs/msg/point.hpp"
#include "rviz_common/display.hpp"
#include "rviz_common/properties/property.hpp"
#include "rviz_common/render_panel.hpp"
#include "rviz_common/view_controller.hpp"
#include "rviz_common/view_manager.hpp"
#include "rviz_common/visualization_manager.hpp"
#include "rviz_rendering/render_window.hpp"
#include "rviz_rendering/render_system.hpp"

#include "allegro_hand_v6_dashboard/sensor_panel.hpp"

namespace allegro_hand_v6_dashboard
{
namespace
{

constexpr double kFingertipBaseDiameterM = 0.020;
constexpr double kFingertipDiameterGrowthM = 0.035;
constexpr float kFingertipAlpha = 0.4F;

QWidget * titled_panel(const QString & title, QWidget * content)
{
  auto * panel = new QWidget;
  auto * layout = new QVBoxLayout(panel);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  auto * label = new QLabel(title);
  label->setAlignment(Qt::AlignCenter);
  label->setMinimumHeight(38);
  label->setStyleSheet(
    "QLabel { background: #16357d; color: white; font-size: 15px; font-weight: 700; }");
  layout->addWidget(label);
  layout->addWidget(content, 1);
  return panel;
}

std_msgs::msg::ColorRGBA marker_color(double normalized, bool valid)
{
  const QColor color = pressure_color(normalized, valid);
  std_msgs::msg::ColorRGBA result;
  result.r = static_cast<float>(color.redF());
  result.g = static_cast<float>(color.greenF());
  result.b = static_cast<float>(color.blueF());
  result.a = valid ? 0.92F : 0.35F;
  return result;
}

}  // namespace

DashboardWindow::DashboardWindow(
  std::shared_ptr<rviz_common::ros_integration::RosNodeAbstraction> node_abstraction,
  SensorLayout sensor_layout, const QString & svg_path, QWidget * parent)
: QMainWindow(parent),
  node_abstraction_(std::move(node_abstraction)),
  node_(node_abstraction_->get_raw_node()),
  sensor_layout_(std::move(sensor_layout))
{
  show_visualization_ = node_->declare_parameter<bool>("show_visualization", true);
  show_sensor_panel_ = node_->declare_parameter<bool>("show_sensor_panel", true);
  fixed_frame_ = node_->declare_parameter<std::string>("fixed_frame", "world");
  robot_description_topic_ = node_->declare_parameter<std::string>(
    "robot_description_topic", "/robot_description");
  pressure_topic_ = node_->declare_parameter<std::string>(
    "pressure_topic", "/allegro_hand/tactile_pressures");
  marker_topic_ = node_->declare_parameter<std::string>(
    "marker_topic", "/allegro_hand_v6/tactile_markers");
  stale_timeout_seconds_ = node_->declare_parameter<double>("stale_timeout", 1.0);
  view_yaw_ = node_->declare_parameter<double>("view_yaw", 3.403392);
  view_pitch_ = node_->declare_parameter<double>("view_pitch", 0.15);
  view_distance_ = node_->declare_parameter<double>("view_distance", 0.60);
  view_focal_z_ = node_->declare_parameter<double>("view_focal_z", 0.055);
  if (!show_visualization_ && !show_sensor_panel_) {
    throw std::runtime_error("at least one dashboard panel must be enabled");
  }

  setWindowTitle("Allegro Hand V6 Dashboard");
  resize(show_visualization_ && show_sensor_panel_ ? 1600 : 850, 900);
  setMinimumSize(show_visualization_ && show_sensor_panel_ ? 1050 : 520, 680);

  auto * splitter = new QSplitter(Qt::Horizontal);
  splitter->setChildrenCollapsible(false);
  splitter->setHandleWidth(5);
  splitter->setStyleSheet("QSplitter::handle { background: #cbd5e5; }");

  if (show_visualization_) {
    auto * rviz_host = new QWidget;
    auto * rviz_layout = new QVBoxLayout(rviz_host);
    rviz_layout->setContentsMargins(0, 0, 0, 0);
    render_panel_ = new rviz_common::RenderPanel(rviz_host);
    rviz_layout->addWidget(render_panel_);
    splitter->addWidget(titled_panel("Live 3D Hand & Pressure", rviz_host));
  }

  if (show_sensor_panel_) {
    sensor_panel_ = new SensorPanel(
      sensor_layout_, svg_path, stale_timeout_seconds_);
    splitter->addWidget(sensor_panel_);
  }
  if (splitter->count() == 2) {
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({800, 800});
  }
  setCentralWidget(splitter);

  if (show_visualization_) {
    // OGRE needs the RenderPanel to have its final Qt/X11 hierarchy and native
    // window before it initializes. The zero-delay callback runs after
    // DashboardWindow::show() has delivered those events.
    QTimer::singleShot(0, this, [this]() {setup_rviz();});
  }

  marker_publisher_ = node_->create_publisher<visualization_msgs::msg::MarkerArray>(
    marker_topic_, rclcpp::QoS(5));
  tactile_subscription_ = node_->create_subscription<std_msgs::msg::Float64MultiArray>(
    pressure_topic_, rclcpp::SensorDataQoS(),
    [this](std_msgs::msg::Float64MultiArray::SharedPtr message) {on_tactile(std::move(message));});

  ros_timer_ = new QTimer(this);
  connect(ros_timer_, &QTimer::timeout, this, [this]() {
      if (!rclcpp::ok()) {
        QCoreApplication::quit();
        return;
      }
      // VisualizationManager owns an executor for this same node. In
      // sensor-only mode there is no manager, so the Qt timer spins it.
      if (!visualization_manager_) {
        rclcpp::spin_some(node_);
      }
      if (sensor_panel_) {
        sensor_panel_->refresh_status();
      }
    });
  ros_timer_->start(16);

  RCLCPP_INFO(
    node_->get_logger(), "Dashboard ready: visualization=%s sensor_panel=%s, expecting %zu channels on %s",
    show_visualization_ ? "true" : "false", show_sensor_panel_ ? "true" : "false",
    sensor_layout_.channel_count(), pressure_topic_.c_str());
}

DashboardWindow::~DashboardWindow()
{
  if (ros_timer_) {
    ros_timer_->stop();
  }
  tactile_subscription_.reset();
  marker_publisher_.reset();
  if (visualization_manager_) {
    visualization_manager_->stopUpdate();
    visualization_manager_->removeAllDisplays();
    visualization_manager_.reset();
  }
}

void DashboardWindow::setup_rviz()
{
  rviz_rendering::RenderSystem::get();
  // Humble's VisualizationManager initializes its root display in the
  // constructor and therefore requires a live scene manager first. This is
  // the same ordering used by rviz_common::VisualizationFrame.
  render_panel_->getRenderWindow()->initialize();
  visualization_manager_ = std::make_unique<rviz_common::VisualizationManager>(
    render_panel_, node_abstraction_, nullptr, node_->get_clock());
  render_panel_->initialize(visualization_manager_.get());
  visualization_manager_->initialize();
  visualization_manager_->setFixedFrame(QString::fromStdString(fixed_frame_));

  auto * grid = visualization_manager_->createDisplay("rviz_default_plugins/Grid", "Grid", true);
  if (grid) {
    grid->subProp("Cell Size")->setValue(0.05);
    grid->subProp("Plane Cell Count")->setValue(10);
    grid->subProp("Alpha")->setValue(0.25);
  }

  auto * robot = visualization_manager_->createDisplay(
    "rviz_default_plugins/RobotModel", "Allegro Hand V6", true);
  if (robot) {
    if (auto * property = robot->subProp("Description Source")) {
      property->setValue("Topic");
    }
    if (auto * property = robot->subProp("Description Topic")) {
      property->setValue(QString::fromStdString(robot_description_topic_));
    }
  }

  auto * markers = visualization_manager_->createDisplay(
    "rviz_default_plugins/MarkerArray", "Tactile Pressure", true);
  if (markers) {
    markers->setTopic(
      QString::fromStdString(marker_topic_), "visualization_msgs/msg/MarkerArray");
  }

  auto * views = visualization_manager_->getViewManager();
  views->setCurrentViewControllerType("rviz_default_plugins/Orbit");
  if (auto * view = views->getCurrent()) {
    if (auto * property = view->subProp("Distance")) {
      property->setValue(view_distance_);
    }
    if (auto * property = view->subProp("Pitch")) {
      property->setValue(view_pitch_);
    }
    if (auto * property = view->subProp("Yaw")) {
      property->setValue(view_yaw_);
    }
    if (auto * focal = view->subProp("Focal Point")) {
      focal->subProp("X")->setValue(0.07);
      focal->subProp("Y")->setValue(0.0);
      focal->subProp("Z")->setValue(view_focal_z_);
    }
  }
  visualization_manager_->startUpdate();
}

void DashboardWindow::on_tactile(const std_msgs::msg::Float64MultiArray::SharedPtr message)
{
  std::vector<float> values(message->data.begin(), message->data.end());
  if (sensor_panel_) {
    sensor_panel_->set_readings(values);
  }
  if (show_visualization_) {
    publish_markers(values);
  }
}

void DashboardWindow::publish_markers(const std::vector<float> & values)
{
  // A standalone sensor-panel launch has no robot_state_publisher. Avoid
  // flooding RViz's message filter with untransformable link-frame markers;
  // publication begins automatically once the V6 TF tree is present.
  if (node_->count_publishers("/tf") == 0U) {
    return;
  }
  visualization_msgs::msg::MarkerArray array;
  array.markers.reserve(sensor_layout_.sensors.size());
  const auto stamp = node_->now();
  for (const auto & sensor : sensor_layout_.sensors) {
    const bool valid = sensor.index < values.size() && std::isfinite(values[sensor.index]);
    const double value = valid ? values[sensor.index] : sensor_layout_.pressure_min;
    const double normalized = std::clamp(
      (value - sensor_layout_.pressure_min) /
      (sensor_layout_.pressure_max - sensor_layout_.pressure_min), 0.0, 1.0);
    const bool is_tip = sensor.name.size() >= 4 &&
      sensor.name.compare(sensor.name.size() - 4, 4, "_tip") == 0;

    if (is_tip) {
      visualization_msgs::msg::Marker sphere;
      sphere.header.frame_id = sensor.frame_id;
      sphere.header.stamp = stamp;
      sphere.ns = "tactile_spheres";
      sphere.id = static_cast<int>(sensor.index);
      sphere.type = visualization_msgs::msg::Marker::SPHERE;
      sphere.action = visualization_msgs::msg::Marker::ADD;
      sphere.frame_locked = true;
      sphere.pose.position.x = sensor.position[0];
      sphere.pose.position.y = sensor.position[1];
      sphere.pose.position.z = sensor.position[2];
      sphere.pose.orientation.w = 1.0;
      // Match the V5 visualization: even minimum-pressure (blue) readings
      // retain a visible, translucent sphere around the fingertip. Marker
      // scale is diameter, so 0.020 m corresponds to a 10 mm base radius.
      const double diameter =
        kFingertipBaseDiameterM + kFingertipDiameterGrowthM * normalized;
      sphere.scale.x = diameter;
      sphere.scale.y = diameter;
      sphere.scale.z = diameter;
      sphere.color = marker_color(normalized, valid);
      if (valid) {
        sphere.color.a = kFingertipAlpha;
      }
      array.markers.push_back(sphere);
      continue;
    }

    visualization_msgs::msg::Marker arrow;
    arrow.header.frame_id = sensor.frame_id;
    arrow.header.stamp = stamp;
    arrow.ns = "tactile_arrows";
    arrow.id = static_cast<int>(sensor.index);
    arrow.type = visualization_msgs::msg::Marker::ARROW;
    arrow.action = valid && normalized > 0.005 ?
      visualization_msgs::msg::Marker::ADD : visualization_msgs::msg::Marker::DELETE;
    arrow.frame_locked = true;
    geometry_msgs::msg::Point start;
    start.x = sensor.position[0];
    start.y = sensor.position[1];
    start.z = sensor.position[2];
    geometry_msgs::msg::Point end = start;
    const double length = 0.012 + 0.075 * normalized;
    end.x += sensor.normal[0] * length;
    end.y += sensor.normal[1] * length;
    end.z += sensor.normal[2] * length;
    arrow.points = {start, end};
    arrow.scale.x = 0.0025 + 0.0035 * normalized;
    arrow.scale.y = 0.006 + 0.007 * normalized;
    arrow.scale.z = 0.008 + 0.010 * normalized;
    arrow.color = marker_color(normalized, valid);
    array.markers.push_back(arrow);
  }
  marker_publisher_->publish(array);
}

}  // namespace allegro_hand_v6_dashboard
