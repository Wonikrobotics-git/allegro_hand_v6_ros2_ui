# Allegro Hand V6 Dashboard

[한국어](README_KR.md)

This ROS 2 package displays the Allegro Hand V6 robot model and tactile sensor state in a single Qt window. The left side contains an embedded RViz 3D view, and the right side shows a hand-image-based tactile sensor panel.

## Directory and File Structure

```text
allegro_hand_v6_dashboard/
├── assets/
│   ├── V6_right_new.png
│   └── V6_left_new.png
├── config/
│   ├── tactile_layout.yaml
│   └── tactile_layout_left.yaml
├── include/allegro_hand_v6_dashboard/
│   ├── dashboard_window.hpp
│   ├── sensor_layout.hpp
│   └── sensor_panel.hpp
├── launch/
│   └── dashboard.launch.py
├── src/
│   ├── dashboard_main.cpp
│   ├── dashboard_window.cpp
│   ├── sensor_layout.cpp
│   └── sensor_panel.cpp
├── test/
│   └── test_sensor_layout.cpp
├── CMakeLists.txt
├── package.xml
├── LICENSE
├── THIRD_PARTY_NOTICES.md
├── README.md
└── README_KR.md
```

| Path | Purpose |
|---|---|
| `assets/` | Hand images used by the 2D tactile panel |
| `config/` | Per-hand tactile channel and visualization layouts |
| `include/allegro_hand_v6_dashboard/` | Public C++ headers |
| `launch/` | Dashboard launch file |
| `src/` | Dashboard implementation |
| `test/` | Unit tests |
| `assets/V6_right_new.png` | Right-hand 2D panel background |
| `assets/V6_left_new.png` | Left-hand 2D panel background |
| `config/tactile_layout.yaml` | Right-hand channels, TF frames, 3D markers, and 2D text coordinates |
| `config/tactile_layout_left.yaml` | Left-hand mirrored layout and coordinates |
| `include/.../dashboard_window.hpp` | Qt main window, ROS interfaces, and RViz object declarations |
| `include/.../sensor_layout.hpp` | Tactile sensor layout data structures |
| `include/.../sensor_panel.hpp` | 2D panel and pressure-color interface |
| `launch/dashboard.launch.py` | Hand selection, 2D/3D panel options, and camera settings |
| `src/dashboard_main.cpp` | ROS 2 and Qt initialization and per-hand asset selection |
| `src/dashboard_window.cpp` | Embedded RViz, tactile subscription, and marker generation |
| `src/sensor_layout.cpp` | YAML loading, validation, and normal-vector normalization |
| `src/sensor_panel.cpp` | Hand image, sensor values, status, and color rendering |
| `test/test_sensor_layout.cpp` | Color mapping and left/right layout tests |
| `CMakeLists.txt` | Build, installation, asset installation, and test configuration |
| `package.xml` | ROS 2 package metadata and dependencies |
| `LICENSE` | Package source-code license |
| `THIRD_PARTY_NOTICES.md` | Third-party notices for Qt, RViz, images, and yaml-cpp |
| `README.md` | English documentation |
| `README_KR.md` | Korean documentation |

## Additional Dependencies and Installation

This package supports ROS 2 Humble on Ubuntu 22.04 Jammy and ROS 2 Jazzy on Ubuntu 24.04 Noble. ROS 2 must already be installed, and `/opt/ros/$ROS_DISTRO/setup.bash` must be available. See the official platform information for [Humble](https://docs.ros.org/en/humble/Releases/Release-Humble-Hawksbill.html) and [Jazzy](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html).

Direct dependencies are listed below.

| Category | Packages |
|---|---|
| Build tools | C++17 compiler, `ament_cmake`, `colcon` |
| GUI | Qt 5 Core/Gui/Widgets and `qtbase5-dev` |
| Configuration | `yaml-cpp`, `yaml_cpp_vendor` |
| ROS 2 | `ament_index_cpp`, `rclcpp`, `std_msgs`, `visualization_msgs` |
| 3D view | `rviz_common`, `rviz_default_plugins`, `rviz_rendering` |
| Launch/runtime | Python 3, `launch`, `launch_ros`, Qt 5 SVG runtime |
| Tests | `ament_cmake_gtest` |

Installing dependencies with `rosdep` from the workspace root is recommended:

```bash
# Select the installed distribution (choose one):
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
sudo apt update
sudo apt install python3-rosdep python3-colcon-common-extensions
# Run only once per system if rosdep has not been initialized:
# sudo rosdep init
rosdep update
rosdep install --from-paths allegro_hand_v6_dashboard \
  --ignore-src --rosdistro "$ROS_DISTRO" -r -y
```

If `rosdep` is unavailable, install the principal packages directly. `$ROS_DISTRO` must be `humble` or `jazzy`.

```bash
sudo apt install qtbase5-dev libqt5svg5 \
  ros-${ROS_DISTRO}-ament-index-cpp \
  ros-${ROS_DISTRO}-rclcpp \
  ros-${ROS_DISTRO}-rviz-common \
  ros-${ROS_DISTRO}-rviz-default-plugins \
  ros-${ROS_DISTRO}-rviz-rendering \
  ros-${ROS_DISTRO}-std-msgs \
  ros-${ROS_DISTRO}-visualization-msgs \
  ros-${ROS_DISTRO}-yaml-cpp-vendor \
  ros-${ROS_DISTRO}-launch \
  ros-${ROS_DISTRO}-launch-ros
```

The source code is the same for both distributions; the ROS package prefix and Ubuntu platform differ.

| ROS 2 | Recommended Ubuntu | Notable difference |
|---|---|---|
| Humble | 22.04 Jammy | `ros-humble-*`; Qt runtime packages use names such as `libqt5core5a`, `libqt5gui5`, and `libqt5widgets5` |
| Jazzy | 24.04 Noble | `ros-jazzy-*`; Ubuntu's time64 transition uses names such as `libqt5core5t64`, `libqt5gui5t64`, and `libqt5widgets5t64` |

Do not pin the low-level Qt runtime names. Installing `qtbase5-dev` through APT or using `rosdep` resolves the correct packages for each Ubuntu release. The 3D panel also requires RViz and a working OpenGL/X11 or compatible display-server environment.

## Package Responsibilities

- Embeds an RViz view containing `RobotModel`, a grid, and tactile markers.
- Displays 18 tactile values as numbers and colors over a hand image.
- Draws fingertip sensors as translucent spheres and the remaining sensors as arrows.
- Keeps right- and left-hand images, sensor coordinates, and marker coordinates in separate YAML files.
- Reports a stale state when tactile messages stop arriving.

## Executable

### `allegro_hand_v6_dashboard_node`

This is the Qt dashboard executable. Its ROS node name is `allegro_hand_v6_dashboard`.

- `show_visualization=true`: show the RViz 3D panel.
- `show_sensor_panel=true`: show the PNG-based 2D tactile panel.
- The process exits with an error if both options are `false`.

These are ROS parameters, not launch arguments. `dashboard.launch.py` exposes `VISUALIZE` and `DEMO` instead: `show_visualization` follows `VISUALIZE`, and `show_sensor_panel` is enabled when either `VISUALIZE` or `DEMO` is true. To set `show_sensor_panel` independently, run the executable directly:

```bash
ros2 run allegro_hand_v6_dashboard allegro_hand_v6_dashboard_node \
  --ros-args -p show_visualization:=false -p show_sensor_panel:=true
```

## ROS 2 Topics

### Topics used directly

| Direction | Topic | Type | Purpose |
|---|---|---|---|
| Subscribe | `/allegro_hand/tactile_pressures` | `std_msgs/msg/Float64MultiArray` | 18-channel tactile pressure input, in hPa by default |
| Publish | `/allegro_hand_v6/tactile_markers` | `visualization_msgs/msg/MarkerArray` | Five fingertip spheres and 13 arrow markers for RViz |

The `pressure_topic` and `marker_topic` parameters can override these names.

### Topics used by embedded RViz

| Direction | Topic | Type | Purpose |
|---|---|---|---|
| Subscribe | `/robot_description` | `std_msgs/msg/String` | URDF robot model |
| Subscribe | `/tf` | `tf2_msgs/msg/TFMessage` | Moving-link transforms |
| Subscribe | `/tf_static` | `tf2_msgs/msg/TFMessage` | Static transforms |
| Subscribe | `/allegro_hand_v6/tactile_markers` | `visualization_msgs/msg/MarkerArray` | Tactile markers published by this node |

3D markers are published after a `/tf` publisher becomes available. When the dashboard is run by itself without a robot model and TF publisher, the 2D panel still works, but the 3D hand and markers do not render correctly.

## Tactile Data Array

`Float64MultiArray.data` must contain exactly 18 values. The `layout` field is ignored; the array index determines the sensor location. Marker IDs use the same channel indexes, while marker frames and coordinates come from the tactile layout YAML.

| Index | Sensor | Index | Sensor |
|---:|---|---:|---|
| 0 | Thumb tip | 9 | Ring tip |
| 1 | Thumb middle | 10 | Ring middle |
| 2 | Thumb base | 11 | Ring base |
| 3 | Index tip | 12 | Pinky tip |
| 4 | Index middle | 13 | Pinky middle |
| 5 | Index base | 14 | Pinky base |
| 6 | Middle tip | 15 | Palm bottom |
| 7 | Middle middle | 16 | Palm center |
| 8 | Middle base | 17 | Palm upper/left |

The channel group order is `thumb → index → middle → ring → pinky → palm`. Finger-local order is `tip → middle → base`, while palm order is `bottom → center → upper/left`.

## Tactile Layout YAML

The layout file selected by `hand` or `sensor_config` defines both the pressure scale and the sensor placement. The pressure range is **not** a ROS parameter; change it here.

| Key | Type | Default | Purpose |
|---|---|---|---|
| `unit` | string | `hPa` | Unit label drawn on the 2D color scale |
| `pressure_min` | double | `1000.0` | Lower bound of the color and marker-size scale |
| `pressure_max` | double | `1200.0` | Upper bound of the color and marker-size scale |
| `svg_width` | double | `600.0` | Coordinate width the `svg` entries are expressed in |
| `svg_height` | double | `900.0` | Coordinate height the `svg` entries are expressed in |
| `image_crop` | `[x, y, w, h]` | full image | Source-image crop applied before drawing; must lie inside the source image |
| `image_scale` | double | `1.0` | 2D image scale factor; must be positive |
| `image_offset` | `[x, y]` | `[0, 0]` | 2D image draw offset |
| `sensors` | list | required | Sensor entries; the loader requires at least one, and the dashboard expects the full 18 channels |

Each `sensors` entry uses these fields:

| Field | Type | Default | Purpose |
|---|---|---|---|
| `index` | int | required | Channel index `0..17` in the tactile array |
| `name` | string | required | Internal sensor identifier |
| `label` | string | `name` | Short label drawn on the 2D panel |
| `group` | string | required | `Thumb`, `Index`, `Middle`, `Ring`, `Pinky`, or `Palm` |
| `frame_id` | string | required | TF frame the 3D marker is attached to |
| `position` | `[x, y, z]` | required | Marker position within `frame_id`, in meters |
| `normal` | `[x, y, z]` | required | Marker direction; normalized at load time |
| `svg` | `[x, y]` | required | Text position on the 2D panel |
| `text_rotation` | double | `0.0` | 2D label rotation in degrees |

```yaml
unit: hPa
pressure_min: 1000.0
pressure_max: 1200.0
sensors:
  - {index: 0, name: thumb_tip, label: TT, group: Thumb, frame_id: R14_Link,
     position: [0.0125, -0.0217, 0.001], normal: [0, 0, 1], svg: [1147, 688], text_rotation: 59}
```

## Pressure Visualization

The default pressure range is `1000..1200 hPa`, taken from the tactile layout YAML above.

| Normalized position | Color |
|---:|---|
| At or below minimum | Blue |
| 25% | Cyan-green |
| 50% | Green |
| 75% | Yellow-green |
| At or above maximum | Red |

- Numeric values outside the configured range are still shown unchanged.
- Only color and marker-size calculations are clamped to `pressure_min..pressure_max`.
- Fingertip sphere diameter grows from `20 mm` to `55 mm`.
- Valid fingertip spheres use alpha `0.4`.
- Invalid or missing readings are shown in gray.
- Non-fingertip arrows grow in length and width with pressure.

## ROS Parameters

| Parameter | Default | Purpose |
|---|---:|---|
| `hand` | `right` | `right` or `left`; selects the default layout and image |
| `sensor_config` | empty | Custom tactile layout; an empty value selects the per-hand default |
| `svg_path` | empty | Custom background PNG; the legacy parameter name is retained for compatibility |
| `show_visualization` | `true` | Show the RViz 3D panel |
| `show_sensor_panel` | `true` | Show the 2D tactile panel |
| `fixed_frame` | `world` | RViz fixed frame |
| `robot_description_topic` | `/robot_description` | RobotModel description topic |
| `pressure_topic` | `/allegro_hand/tactile_pressures` | Tactile input topic |
| `marker_topic` | `/allegro_hand_v6/tactile_markers` | Marker output topic |
| `stale_timeout` | `1.0` | Seconds without tactile input before the 2D panel becomes stale |
| `view_yaw` | `3.403392` | RViz Orbit yaw in radians |
| `view_pitch` | `0.15` | RViz Orbit pitch in radians |
| `view_distance` | `0.60` | RViz camera distance in meters |
| `view_focal_z` | `0.055` | RViz focal-point Z coordinate in meters |

## Running

Start the V6 core bringup in a separate terminal:

```bash
ros2 launch allegro_hand_v6_bringup bringup.launch.py
```

Then start the dashboard in another terminal:

```bash
ros2 launch allegro_hand_v6_dashboard dashboard.launch.py \
  VISUALIZE:=true HAND:=right
```

To open only the 2D tactile panel:

```bash
ros2 launch allegro_hand_v6_dashboard dashboard.launch.py \
  DEMO:=true VISUALIZE:=false HAND:=right
```

The dashboard launch file does not start the hardware, controllers, or `robot_state_publisher`. The separate V6 bringup process is required to display the 3D robot model. The 2D-only panel can run without bringup, but it remains stale until tactile messages are available.

## Build and Test

```bash
colcon build --packages-select allegro_hand_v6_dashboard --symlink-install
colcon test --packages-select allegro_hand_v6_dashboard
colcon test-result --test-result-base build/allegro_hand_v6_dashboard --verbose
```

## License

The package source code is licensed under BSD-3-Clause. Qt is used through the operating system's LGPL shared libraries. See `LICENSE` and `THIRD_PARTY_NOTICES.md` for details.
