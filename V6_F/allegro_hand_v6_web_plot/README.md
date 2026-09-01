# Allegro Hand V6 Web Plot

[한국어](README_KR.md)

This ROS 2 monitoring package displays Allegro Hand V6 joint positions, position targets, effort, temperature, and tactile values in a web browser. A Python bridge collects ROS topics and converts them to WebSocket JSON, while a static HTML page renders the data as real-time charts.

## Directory and File Structure

```text
allegro_hand_v6_web_plot/
├── scripts/
│   └── web_plot_bridge.py
├── web/
│   ├── index.html
│   └── WR_EN_high.png
├── CMakeLists.txt
├── package.xml
├── README.md
└── README_KR.md
```

| Path | Purpose |
|---|---|
| `scripts/` | ROS-to-Web Python bridge |
| `web/` | Static browser page and logo copied into the install space |
| `scripts/web_plot_bridge.py` | ROS subscriptions, HTTP server, WebSocket server, and 30 Hz JSON broadcast |
| `web/index.html` | Joint selection, 10-second canvas charts, and tactile bar view |
| `web/WR_EN_high.png` | WONIK Robotics logo displayed by the browser page |
| `CMakeLists.txt` | Python executable and web-asset installation |
| `package.xml` | ROS 2 metadata and Python runtime dependencies |
| `README.md` | English documentation |
| `README_KR.md` | Korean documentation |

`scripts/__pycache__/` and `*.pyc` are Python-generated cache files, not package source files.

## Additional Dependencies and Installation

This package supports ROS 2 Humble on Ubuntu 22.04 Jammy and ROS 2 Jazzy on Ubuntu 24.04 Noble. ROS 2 must already be installed, and `/opt/ros/$ROS_DISTRO/setup.bash` must be available. See the official platform information for [Humble](https://docs.ros.org/en/humble/Releases/Release-Humble-Hawksbill.html) and [Jazzy](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html).

| Category | Packages |
|---|---|
| Build tools | `ament_cmake`, `colcon` |
| ROS 2 Python | `rclpy`, `ament_index_python` |
| ROS messages | `sensor_msgs`, `std_msgs` |
| WebSocket | `python3-websockets` |
| HTTP/browser UI | Python 3 standard library and a JavaScript-capable web browser |

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
rosdep install --from-paths allegro_hand_v6_web_plot \
  --ignore-src --rosdistro "$ROS_DISTRO" -r -y
```

If `rosdep` is unavailable, install the principal runtime packages directly:

```bash
sudo apt install python3-websockets \
  ros-${ROS_DISTRO}-ament-index-python \
  ros-${ROS_DISTRO}-rclpy \
  ros-${ROS_DISTRO}-sensor-msgs \
  ros-${ROS_DISTRO}-std-msgs
```

| ROS 2 | Recommended Ubuntu | Ubuntu `python3-websockets` version |
|---|---|---:|
| Humble | 22.04 Jammy | 9.1 |
| Jazzy | 24.04 Noble | 10.4 |

The bridge accepts the handler calling conventions used by both distribution-provided versions. Prefer Ubuntu's `python3-websockets` package instead of overriding it with `pip`. The browser needs no additional JavaScript libraries, but it must be able to reach both the HTTP and WebSocket ports.

## Package Responsibilities

- Collects four ROS 2 topics into one state object.
- Reorders `/joint_states` by joint name into `joint00..joint43` order.
- Serves static web assets over HTTP.
- Broadcasts the latest state as WebSocket JSON at approximately 30 Hz.
- Displays the most recent 10 seconds of selected joint data in the browser.
- Displays tactile data in a separate `Sensors` tab.

Data flow:

```text
ROS 2 topics
    ↓
web_plot_bridge.py
    ├── HTTP :8080 ─────→ index.html
    └── WebSocket :9091 → real-time JSON → browser chart
```

| Item | Name |
|---|---|
| Executable | `web_plot_bridge` |
| Node | `allegro_hand_v6_web_plot` |

## ROS 2 Topics

This package publishes no ROS topics. It subscribes to the following topics:

| Direction | Topic | Type | Fields used | Purpose |
|---|---|---|---|---|
| Subscribe | `/joint_states` | `sensor_msgs/msg/JointState` | `name`, `position`, `effort` | Current joint positions and effort |
| Subscribe | `/allegro_hand_position_controller/commands` | `std_msgs/msg/Float64MultiArray` | `data[0:20]` | Latest 20-joint position target |
| Subscribe | `/allegro_hand_v6/joint_temperatures` | `std_msgs/msg/Float32MultiArray` | `data[0:20]` | Latest 20-joint temperature values (see note below) |
| Subscribe | `/allegro_hand/tactile_pressures` | `std_msgs/msg/Float64MultiArray` | `data[0:18]` | Tactile pressure values in hPa |

### Joint order and units

- Joint order: `joint00..joint03`, `joint10..joint13`, ..., `joint40..joint43`.
- `position`: `/joint_states.position`, in radians.
- `desired`: position controller command, in radians.
- `effort`: `/joint_states.effort`; the V6 hardware currently reports joint torque in `N·m`.
- `temperature`: `/allegro_hand_v6/joint_temperatures`, in °C.
- `pressure`: `/allegro_hand/tactile_pressures`, an 18-channel hPa array.

Because `/joint_states` resource order is not guaranteed, values are reordered by name. Missing joints and NaN values are sent as `null` in WebSocket JSON.

> **Note:** No node in the standard bringup publishes `/allegro_hand_v6/joint_temperatures`. The hardware interface exposes per-joint temperature only as a `ros2_control` state interface, and the configured `joint_state_broadcaster` does not forward it to a topic. With the stock stack the `Temperature` tab therefore stays empty and its timestamp remains `null`. To populate it, publish that topic from your own node or add a broadcaster that exports the `temperature` state interface.

## WebSocket JSON Format

The WebSocket server broadcasts snapshots with this structure:

```json
{
  "names": ["joint00", "joint01", "...", "joint43"],
  "position": [0.0, 0.0, "... 20 values ..."],
  "effort": [0.0, 0.0, "... 20 values ..."],
  "desired": [0.0, 0.0, "... 20 values ..."],
  "temperature": [25.0, 25.0, "... 20 values ..."],
  "pressure": ["... all received tactile values ..."],
  "timestamps": {
    "joint": 0.0,
    "desired": 0.0,
    "temperature": 0.0,
    "pressure": 0.0
  }
}
```

Each timestamp is the ROS-clock time, in seconds, of the latest message of that type. A data type that has not yet been received is `null`.

## Browser Interface

| Tab | Display |
|---|---|
| `Angles` | Measured position as a solid line and target position as a dashed line |
| `Torque` | Joint torque from `/joint_states.effort`, in N·m |
| `Temperature` | Temperatures for all 20 joints; empty unless a temperature publisher is supplied |
| `Sensors` | Tactile sensor bars |

- Charts retain the latest 10 seconds.
- Data can be selected by Thumb, Index, Middle, Ring, Pinky, or individual joint.
- The four thumb joints are selected initially.
- The Y axis adapts to the visible finite values.
- A disconnected WebSocket retries after 1.5 seconds.

### Tactile bars

The browser displays the 18 channels in `Thumb 0..2 → Index 3..5 → Middle 6..8 → Ring 9..11 → Pinky 12..14 → Palm 15..17` order. Bar heights use the default V6 pressure range of `1000..1200 hPa` and are clamped to the visible area when values are outside that range.

## ROS Parameters and Network Ports

| Parameter | Default | Purpose |
|---|---:|---|
| `http_port` | `8080` | HTTP port serving `index.html` and its assets |
| `ws_port` | `9091` | WebSocket port carrying real-time JSON |

Both servers bind to `0.0.0.0`, so devices on the same network can connect.

> **Security warning:** HTTP and WebSocket have no authentication or TLS. Use them only on a trusted internal network and do not expose the ports directly to an external network.

The WebSocket URL in `web/index.html` currently fixes the port at `9091`. If `ws_port` is changed, update the URL in `index.html` as well.

## Running

Start the V6 core bringup in a separate terminal:

```bash
ros2 launch allegro_hand_v6_bringup bringup.launch.py
```

Then start Web Plot in another terminal:

```bash
ros2 run allegro_hand_v6_web_plot web_plot_bridge
```

Open this address on the same computer:

```text
http://localhost:8080
```

Example with a different HTTP port:

```bash
ros2 run allegro_hand_v6_web_plot web_plot_bridge \
  --ros-args -p http_port:=8088
```

This package has no standalone launch file and does not start hardware or controllers. It displays data published by the separately running V6 bringup process.

## Build

```bash
colcon build --packages-select allegro_hand_v6_web_plot --symlink-install
```

## License

`package.xml` declares this package as BSD. No `LICENSE` file is currently distributed with this package.
