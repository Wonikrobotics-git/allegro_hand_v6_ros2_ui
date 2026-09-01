# Allegro Hand V6 UI

Allegro Hand V6의 상태 확인, pose 제어 및 실시간 데이터 모니터링을 제공하는 ROS 2 UI 패키지 모음이다.

이 저장소는 V6 core 및 `allegro_hand_common`과 별도로 배포된다. UI 패키지는 V6 hardware 라이브러리에 직접 링크하지 않으며, 별도로 실행된 V6 bringup과 ROS 2 topic, TF 및 controller interface를 통해 연결된다.

## 패키지 구성

```text
V6_gui/
├── allegro_hand_v6_dashboard/   # RViz robot model 및 촉각 센서 Dashboard
├── allegro_hand_v6_gui/         # Pose 저장, 실행 및 sequence 제어 GUI
└── allegro_hand_v6_web_plot/    # 관절 및 촉각 데이터 Web Plot
```

| 패키지 | 기능 | 상세 문서 |
|---|---|---|
| `allegro_hand_v6_dashboard` | 3D robot model과 18채널 촉각 센서 표시 | [README](allegro_hand_v6_dashboard/README_KR.md) |
| `allegro_hand_v6_gui` | 현재 pose 저장, 저장 pose 실행, pose sequence 실행 | [README](allegro_hand_v6_gui/README_KR.md) |
| `allegro_hand_v6_web_plot` | 관절 위치, 목표 위치, effort 및 촉각 데이터 표시 | [README](allegro_hand_v6_web_plot/README_KR.md) |

## V6 Core와의 관계

```text
allegro_hand_common
        ↓
allegro_hand_v6_hardware / allegro_hand_v6_bringup
        ↓  ROS 2 topic, TF, controller interface
V6_gui packages
```

- `allegro_hand_common`은 V6 hardware 및 bringup이 사용하는 공통 IO, controller, message 및 utility 패키지를 제공한다.
- `allegro_hand_v6_bringup`은 hardware interface, controller, `robot_state_publisher` 및 joint state broadcaster를 실행한다.
- 이 저장소의 UI 패키지는 `allegro_hand_common` 패키지나 V6 hardware library를 직접 사용하지 않는다.
- UI와 V6 core는 동일한 ROS 2 graph에서 topic과 TF를 통해 연결된다.
- V6 bringup과 UI는 각각 별도 terminal에서 실행한다. V6 bringup launch에 UI 선택 인자를 전달할 필요가 없다.

## ROS 2 Interface

| Interface | 제공 측 | 사용 패키지 | 용도 |
|---|---|---|---|
| `/joint_states` | V6 bringup | GUI, Web Plot | 20관절 위치와 effort 수신 |
| `/allegro_hand_position_controller/commands` | V6 controller | GUI, Web Plot | 목표 관절 위치 발행 및 표시 |
| `/robot_description` | V6 bringup | Dashboard | URDF robot model 표시 |
| `/tf`, `/tf_static` | V6 bringup | Dashboard | Robot link transform 표시 |
| `/allegro_hand/tactile_pressures` | V6 hardware | Dashboard, Web Plot | 18채널 촉각 압력 표시 |
| `/allegro_hand_v6/joint_temperatures` | 선택적 publisher | Web Plot | 20관절 온도 표시 |

기본 V6 bringup은 `/allegro_hand_v6/joint_temperatures`를 발행하지 않는다. 별도 temperature publisher 또는 broadcaster가 없으면 Web Plot의 Temperature 탭은 비어 있다.

## 지원 환경

- ROS 2 Humble / Ubuntu 22.04
- ROS 2 Jazzy / Ubuntu 24.04
- Qt 5 및 화면을 표시할 수 있는 X11 또는 Wayland 환경
- Web Plot용 Python 3 및 `websockets`

V6 hardware를 구동하려면 별도로 배포되는 `allegro_hand_common` 및 `allegro_hand_v6`가 설치되어 있어야 한다.

## Workspace 구성

### 하나의 Workspace에서 빌드

개발 환경에서는 common, V6 core 및 UI 저장소를 동일한 workspace의 `src` 아래에 배치할 수 있다.

```text
~/allegro_ws/src/
├── allegro_hand_common/
├── allegro_hand_v6/
└── V6_gui/
```

```bash
cd ~/allegro_ws
# 설치된 ROS 2 배포판 하나를 선택한다.
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

### 별도 Workspace에서 빌드

V6 core와 UI를 서로 다른 workspace에 둘 경우 V6 core를 underlay로 먼저 빌드하고 source한다.

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

새 terminal에서는 다음 순서로 환경을 불러온다.

```bash
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
source ~/allegro_v6_ws/install/setup.bash
source ~/allegro_v6_ui_ws/install/setup.bash
```

## 실행

### Terminal 1: V6 Bringup

V6 core workspace를 source한 후 기본 bringup을 실행한다.

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

왼손은 `HAND:=left`를 사용한다. 3D robot model을 표시하려면 Terminal 1의 bringup이 실행 중이어야 한다.

### Terminal 3: Pose GUI

```bash
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
source ~/allegro_v6_ws/install/setup.bash
source ~/allegro_v6_ui_ws/install/setup.bash

ros2 run allegro_hand_v6_gui allegro_hand_v6_gui_node
```

GUI는 `/joint_states`를 수신하고 `/allegro_hand_position_controller/commands`로 목표 위치를 발행한다.

### Terminal 4: Web Plot

```bash
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
source ~/allegro_v6_ws/install/setup.bash
source ~/allegro_v6_ui_ws/install/setup.bash

ros2 run allegro_hand_v6_web_plot web_plot_bridge
```

실행 후 browser에서 다음 주소를 연다.

```text
http://localhost:8080
```

Dashboard, Pose GUI 및 Web Plot은 필요한 항목만 선택하여 실행할 수 있으며 서로 다른 terminal에서 동시에 실행할 수도 있다.

## 연결 확인

UI에 데이터가 표시되지 않을 경우 다음 항목을 확인한다.

```bash
ros2 pkg prefix allegro_hand_v6_bringup
ros2 pkg prefix allegro_hand_v6_dashboard
ros2 control list_controllers
ros2 topic list
ros2 topic echo /joint_states --once
ros2 topic info /allegro_hand_position_controller/commands
ros2 topic info /allegro_hand/tactile_pressures
```

V6 core와 UI를 서로 다른 PC에서 실행하는 경우 양쪽 PC의 ROS 배포판, `ROS_DOMAIN_ID` 및 RMW 설정이 호환되어야 하며 DDS discovery가 가능한 동일 network에 연결되어 있어야 한다.

## 안전 주의사항

Pose GUI의 `Move` 및 sequence 실행은 실제 hand controller에 목표 관절 위치를 발행한다. 실제 장비에서 실행하기 전에 주변 간섭물을 제거하고 비상 정지 방법을 확인하며, 새로운 pose는 안전한 관절 범위와 충분히 긴 interval로 시험한다.
