# Allegro Hand V6 GUI

[한국어](README_KR.md)

This Qt-based ROS 2 control package saves the current Allegro Hand V6 posture, commands saved postures, and runs repeated pose sequences.

## Directory and File Structure

```text
allegro_hand_v6_gui/
├── include/allegro_hand_v6_gui/
│   └── main_window.hpp
├── poses/
│   ├── .gitkeep
│   └── 1.yaml
├── src/
│   ├── main.cpp
│   └── main_window.cpp
├── CMakeLists.txt
├── package.xml
├── README.md
└── README_KR.md
```

| Path | Purpose |
|---|---|
| `include/allegro_hand_v6_gui/` | GUI C++ headers |
| `poses/` | Saved and example poses |
| `src/` | Qt GUI and ROS integration |
| `include/.../main_window.hpp` | Main window, ROS publisher/subscriber, and pose/sequence state declarations |
| `poses/.gitkeep` | Keeps the pose directory in Git when empty |
| `poses/1.yaml` | Example 20-joint zero pose |
| `src/main.cpp` | ROS 2/Qt initialization, 20 ms `rclcpp::spin_some()`, and GUI event loop |
| `src/main_window.cpp` | UI, joint-state handling, pose I/O, publishing, and sequence control |
| `CMakeLists.txt` | Qt/yaml-cpp build, default pose path, and installation rules |
| `package.xml` | ROS 2 package metadata and dependencies |
| `README.md` | English documentation |
| `README_KR.md` | Korean documentation |

## Additional Dependencies and Installation

This package supports ROS 2 Humble on Ubuntu 22.04 Jammy and ROS 2 Jazzy on Ubuntu 24.04 Noble. ROS 2 must already be installed, and `/opt/ros/$ROS_DISTRO/setup.bash` must be available. See the official platform information for [Humble](https://docs.ros.org/en/humble/Releases/Release-Humble-Hawksbill.html) and [Jazzy](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html).

| Category | Packages |
|---|---|
| Build tools | C++17 compiler, `ament_cmake`, `colcon` |
| GUI | Qt 5 Core/Gui/Widgets and `qtbase5-dev` |
| Pose YAML | `yaml-cpp`, `yaml_cpp_vendor` |
| ROS 2 | `rclcpp`, `sensor_msgs`, `std_msgs` |

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
rosdep install --from-paths allegro_hand_v6_gui \
  --ignore-src --rosdistro "$ROS_DISTRO" -r -y
```

If `rosdep` is unavailable, install the principal packages directly:

```bash
sudo apt install qtbase5-dev libyaml-cpp-dev \
  ros-${ROS_DISTRO}-rclcpp \
  ros-${ROS_DISTRO}-sensor-msgs \
  ros-${ROS_DISTRO}-std-msgs \
  ros-${ROS_DISTRO}-yaml-cpp-vendor
```

Humble uses `ros-humble-*`, while Jazzy uses `ros-jazzy-*`. Both distributions provide the Qt 5 development package as `qtbase5-dev`; however, Ubuntu 24.04 uses time64 runtime names such as `libqt5core5t64`, `libqt5gui5t64`, and `libqt5widgets5t64`. Let `qtbase5-dev` or `rosdep` select these transitive runtime packages instead of pinning them directly.

Running the GUI requires an X11/Wayland display environment. Moving a physical hand also requires a controller and hardware or mock-hardware bringup that publishes `/joint_states` and subscribes to `/allegro_hand_position_controller/commands`.

## Package Responsibilities

- Receives the latest 20-joint positions from `/joint_states`.
- Saves the current joint positions as a YAML pose.
- Loads saved YAML poses and sends them to the position controller.
- Builds an ordered list of poses and repeats it at a configured interval.
- Reports pose and sequence status and errors in the GUI log.
- Stops an active sequence when the GUI closes.

| Item | Name |
|---|---|
| Executable | `allegro_hand_v6_gui_node` |
| Node | `allegro_hand_v6_gui` |

## User Interface

### Joint State

- `Joint state: ready (20/20)` appears after a complete, valid 20-joint state has been received.

### Saved Poses

- Pose name and `Save Current`: save the current 20-joint position.
- `Refresh`: reload `*.yaml` and `*.yml` from the pose directory.
- `Move`: publish the selected pose once to the position controller.
- Double-click a pose: same behavior as `Move`.
- `Add to Sequence`: append the selected pose to the sequence.

### Pose Sequence

- `Remove`: delete the selected sequence item.
- `Up`, `Down`: reorder the selected item.
- `Clear`: remove the complete sequence.
- `Pose interval`: delay between pose commands, `0.1..60.0 s`, default `2.0 s`.
- `Repeat count`: number of complete sequence repetitions, `1..999`, default `1`.
- `Start`, `Stop`: start or stop the sequence.

### Log

The log timestamps pose save/command actions, sequence start/completion/stop events, and errors. It retains at most 500 blocks.

## ROS 2 Topics

| Direction | Default topic | Type | Data and purpose |
|---|---|---|---|
| Subscribe | `/joint_states` | `sensor_msgs/msg/JointState` | Current 20-joint positions in radians from `name[]` and `position[]` |
| Publish | `/allegro_hand_position_controller/commands` | `std_msgs/msg/Float64MultiArray` | Target positions for `joint00..joint43` in `data[0..19]`, in radians |

The GUI does not trust the ordering of `/joint_states.name`. It looks up each value by name and rearranges the state into this physical order:

```text
joint00 joint01 joint02 joint03
joint10 joint11 joint12 joint13
joint20 joint21 joint22 joint23
joint30 joint31 joint32 joint33
joint40 joint41 joint42 joint43
```

If `JointState.name` is empty, the first 20 positions are assumed to already follow this order. A message is rejected if it contains fewer than 20 positions, is missing any named joint, or contains NaN or infinity.

## ROS Parameters

| Parameter | Default | Purpose |
|---|---|---|
| `joint_state_topic` | `/joint_states` | Current joint-state input |
| `position_command_topic` | `/allegro_hand_position_controller/commands` | Pose target output |
| `pose_directory` | empty | Override for the pose save/search directory |

When `pose_directory` is empty, the GUI uses the source workspace's `allegro_hand_v6_gui/poses/` path recorded at build time in `ALLEGRO_HAND_V6_GUI_DEFAULT_POSE_DIR`. Rebuild after moving the workspace or pass an absolute directory parameter.

```bash
ros2 run allegro_hand_v6_gui allegro_hand_v6_gui_node \
  --ros-args -p pose_directory:=/absolute/path/to/poses
```

## Saving a Pose

1. Verify that all 20 joints are arriving on `/joint_states`.
2. Move the hand to the posture to save.
3. Enter a pose name under `Saved Poses`.
4. Select `Save Current` or press Enter.
5. The list refreshes automatically after a successful save.

Save requirements:

- A pose name may contain only letters, digits, `_`, and `-`.
- At least one complete 20-joint state must have been received.
- The newest joint state must be no more than two seconds old.
- The GUI asks for confirmation before replacing an existing name.
- Files use the `.yaml` extension.

## Pose YAML Format

```yaml
joint_names:
  - joint00
  - joint01
  - joint02
  - joint03
  - joint10
  - joint11
  - joint12
  - joint13
  - joint20
  - joint21
  - joint22
  - joint23
  - joint30
  - joint31
  - joint32
  - joint33
  - joint40
  - joint41
  - joint42
  - joint43
position: [0.0, 0.0, 0.0, 0.0, 0.0,
           0.0, 0.0, 0.0, 0.0, 0.0,
           0.0, 0.0, 0.0, 0.0, 0.0,
           0.0, 0.0, 0.0, 0.0, 0.0]
```

- `position` must contain exactly 20 finite numbers.
- If present, `joint_names` must contain exactly 20 names. Positions are reordered by these names.
- Without `joint_names`, `position` is assumed to already follow the standard order.
- Unknown YAML, missing joints, incorrect array lengths, NaN, or infinity prevent execution.

## Loading and Commanding a Saved Pose

1. Select `Refresh` to reload pose files.
2. Select a pose in the left-hand list.
3. Select `Move` or double-click the item.
4. The GUI validates the YAML and publishes one standard-order, 20-position command to `/allegro_hand_position_controller/commands`.

`Move` does not interpolate a trajectory or confirm arrival. It publishes one target-position frame; actual speed and response depend on the controller and hardware configuration.

## Building a Pose Sequence

1. Select a saved pose and choose `Add to Sequence`.
2. Add the required poses in order.
3. Use `Up`, `Down`, and `Remove` to edit the list.
4. Configure `Pose interval` and `Repeat count`.
5. Select `Start`.

The first pose is published immediately after `Start`; later poses are published at the configured interval. The same interval applies between the final pose of one repetition and the first pose of the next.

Immediately consecutive duplicates are rejected. A pose may be reused after an intervening pose, as in `open → fist → open`. The active list is copied when `Start` is selected, so editing the visible list does not affect a sequence already in progress.

The sequence list is not saved to disk and disappears when the GUI exits. Only the individual pose YAML files persist.

`Move` and `Start` publish position targets only. Hardware activation and safety state remain the responsibility of bringup and the hardware interface.

## Running

Start the V6 core bringup in a separate terminal:

```bash
ros2 launch allegro_hand_v6_bringup bringup.launch.py
```

Then start the GUI node in another terminal:

```bash
ros2 run allegro_hand_v6_gui allegro_hand_v6_gui_node
```

The GUI does not start hardware or controllers. The separate bringup process must provide `/joint_states` and an active subscriber for `/allegro_hand_position_controller/commands`.

> **Warning:** Before moving a physical hand, clear the surrounding workspace and verify an emergency-stop method. Test new poses and sequences with long intervals and safe joint limits.

## Build

```bash
colcon build --packages-select allegro_hand_v6_gui --symlink-install
```

## License

`package.xml` declares this package as BSD. No `LICENSE` file is currently distributed with this package.
