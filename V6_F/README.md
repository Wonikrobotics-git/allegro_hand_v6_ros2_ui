# Allegro Hand V6 UI

This repository contains ROS 2 UI packages for monitoring Allegro Hand V6 status, controlling poses, and plotting real-time data.

It is released separately from the V6 core and `allegro_hand_common` repositories. The UI packages do not link directly against the V6 hardware library. They communicate with a separately running V6 bringup process through ROS 2 topics, TF, and controller interfaces.

## Package Structure

```text
V6_F/
├── allegro_hand_v6_dashboard/   # RViz robot model and tactile sensor dashboard
├── allegro_hand_v6_gui/         # Pose storage, execution, and sequence control GUI
└── allegro_hand_v6_web_plot/    # Joint and tactile data web plot
```

| Package | Purpose | Documentation |
|---|---|---|
| `allegro_hand_v6_dashboard` | Displays the 3D robot model and 18 tactile sensor channels | [README](allegro_hand_v6_dashboard/README.md) |
| `allegro_hand_v6_gui` | Saves and executes poses and pose sequences | [README](allegro_hand_v6_gui/README.md) |
| `allegro_hand_v6_web_plot` | Displays joint position, target position, effort, and tactile data | [README](allegro_hand_v6_web_plot/README.md) |

## Relationship with the V6 Core

```text
allegro_hand_common
        ↓
allegro_hand_v6_hardware / allegro_hand_v6_bringup
        ↓  ROS 2 topics, TF, and controller interfaces
V6 UI packages
```

- `allegro_hand_common` provides common IO, controller, message, and utility packages used by the V6 hardware and bringup packages.
- `allegro_hand_v6_bringup` starts the hardware interface, controllers, `robot_state_publisher`, and joint state broadcaster.
- The UI packages in this repository do not directly use packages from `allegro_hand_common` or the V6 hardware library.
- The UI and V6 core communicate through topics and TF in the same ROS 2 graph.
- Run V6 bringup and each UI application in separate terminals. No UI-selection argument needs to be passed to the V6 bringup launch file.

## ROS 2 Interfaces

| Interface | Provider | Consumer | Purpose |
|---|---|---|---|
| `/joint_states` | V6 bringup | GUI, Web Plot | Receives positions and effort for 20 joints |
| `/allegro_hand_position_controller/commands` | V6 controller | GUI, Web Plot | Publishes and displays target joint positions |
| `/robot_description` | V6 bringup | Dashboard | Displays the URDF robot model |
| `/tf`, `/tf_static` | V6 bringup | Dashboard | Displays robot link transforms |
| `/allegro_hand/tactile_pressures` | V6 hardware | Dashboard, Web Plot | Displays 18 tactile pressure channels |
| `/allegro_hand_v6/joint_temperatures` | Optional publisher | Web Plot | Displays temperatures for 20 joints |

The default V6 bringup does not publish `/allegro_hand_v6/joint_temperatures`. The Temperature tab in Web Plot remains empty unless a separate temperature publisher or broadcaster is running.

## Supported Environments

- ROS 2 Humble on Ubuntu 22.04
- ROS 2 Jazzy on Ubuntu 24.04
- Qt 5 with an X11 or Wayland display environment
- Python 3 and `websockets` for Web Plot

The separately released `allegro_hand_common` and `allegro_hand_v6` repositories must be installed to operate the V6 hardware.

## Workspace Setup

### Build in One Workspace

For development, place the common, V6 core, and UI repositories under the same workspace `src` directory.

```text
~/allegro_ws/src/
├── allegro_hand_common/
├── allegro_hand_v6/
└── allegro_hand_v6_ros2_ui/
    └── V6_F/
```

```bash
cd ~/allegro_ws
# Select one installed ROS 2 distribution.
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash

rosdep install \
  --from-paths src \
  --ignore-src \
  --rosdistro "$ROS_DISTRO" \
  -r -y

colcon build --symlink-install
source install/setup.bash
```

### Build in Separate Workspaces

When the V6 core and UI are stored in separate workspaces, build and source the V6 core as the underlay first.

```bash
# Core workspace
cd ~/allegro_v6_ws
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
rosdep install --from-paths src --ignore-src --rosdistro "$ROS_DISTRO" -r -y
colcon build --symlink-install
```

```bash
# UI workspace
cd ~/allegro_v6_ui_ws
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
source ~/allegro_v6_ws/install/setup.bash

rosdep install --from-paths src --ignore-src --rosdistro "$ROS_DISTRO" -r -y
colcon build --symlink-install
```

Source the environments in the following order in every new terminal:

```bash
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
source ~/allegro_v6_ws/install/setup.bash
source ~/allegro_v6_ui_ws/install/setup.bash
```

## Running

### Terminal 1: V6 Bringup

Source the V6 core workspace and start the default bringup.

```bash
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
source ~/allegro_v6_ws/install/setup.bash

ros2 launch allegro_hand_v6_bringup bringup.launch.py
```

### Terminal 2: Dashboard

```bash
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
source ~/allegro_v6_ws/install/setup.bash
source ~/allegro_v6_ui_ws/install/setup.bash

ros2 launch allegro_hand_v6_dashboard dashboard.launch.py \
  VISUALIZE:=true HAND:=right
```

Use `HAND:=left` for a left hand. The bringup process in Terminal 1 must be running to display the 3D robot model.

### Terminal 3: Pose GUI

```bash
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
source ~/allegro_v6_ws/install/setup.bash
source ~/allegro_v6_ui_ws/install/setup.bash

ros2 run allegro_hand_v6_gui allegro_hand_v6_gui_node
```

The GUI subscribes to `/joint_states` and publishes target positions to `/allegro_hand_position_controller/commands`.

### Terminal 4: Web Plot

```bash
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
source ~/allegro_v6_ws/install/setup.bash
source ~/allegro_v6_ui_ws/install/setup.bash

ros2 run allegro_hand_v6_web_plot web_plot_bridge
```

Open the following address in a browser after starting Web Plot:

```text
http://localhost:8080
```

Run only the UI applications you need. Dashboard, Pose GUI, and Web Plot can also run simultaneously in separate terminals.

## Connection Checks

Use the following commands if an application does not display data:

```bash
ros2 pkg prefix allegro_hand_v6_bringup
ros2 pkg prefix allegro_hand_v6_dashboard
ros2 control list_controllers
ros2 topic list
ros2 topic echo /joint_states --once
ros2 topic info /allegro_hand_position_controller/commands
ros2 topic info /allegro_hand/tactile_pressures
```

When the V6 core and UI run on different computers, both systems must use compatible ROS distributions, `ROS_DOMAIN_ID` values, and RMW configurations. They must also be connected to the same network with DDS discovery available.

## Safety

The Pose GUI `Move` action and pose sequences publish target joint positions to the active hand controller. Before operating physical hardware, clear the surrounding workspace, verify the emergency-stop procedure, and test new poses with safe joint ranges and sufficiently long intervals.
