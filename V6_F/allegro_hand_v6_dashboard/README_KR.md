# Allegro Hand V6 Dashboard

[English](README.md)

Allegro Hand V6의 로봇 모델과 촉각 센서 상태를 하나의 Qt 창에서 확인하는 ROS 2 패키지이다. 왼쪽에는 임베디드 RViz 3D 화면, 오른쪽에는 손 이미지 기반 센서 패널을 표시한다.

## 디렉터리 및 파일 구성

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

| 경로 | 기능 |
|---|---|
| `assets/` | 2D 센서 패널용 손 이미지 보관 |
| `config/` | 손 방향별 촉각 channel 및 시각화 layout 보관 |
| `include/allegro_hand_v6_dashboard/` | Dashboard C++ public header 보관 |
| `launch/` | ROS 2 launch 파일 보관 |
| `src/` | Dashboard C++ 구현 보관 |
| `test/` | 단위 테스트 보관 |
| `assets/V6_right_new.png` | 오른손 2D 센서 패널 배경 이미지 |
| `assets/V6_left_new.png` | 왼손 2D 센서 패널 배경 이미지 |
| `config/tactile_layout.yaml` | 오른손 센서 채널, TF frame, 3D marker 위치/방향, 2D 텍스트 위치 정의 |
| `config/tactile_layout_left.yaml` | 왼손용 센서 layout 및 좌우 대칭 좌표 정의 |
| `include/.../dashboard_window.hpp` | Qt 메인 창, ROS subscriber/publisher, RViz 객체 인터페이스 |
| `include/.../sensor_layout.hpp` | 센서 정의와 layout 데이터 구조 |
| `include/.../sensor_panel.hpp` | 2D 센서 패널과 압력 색상 변환 인터페이스 |
| `launch/dashboard.launch.py` | 3D/2D panel 선택, 손 방향과 camera 설정을 포함한 Dashboard 실행 |
| `src/dashboard_main.cpp` | ROS 2와 Qt 초기화, 손 방향별 config/image 선택, 메인 창 실행 |
| `src/dashboard_window.cpp` | 임베디드 RViz 구성, 촉각 수신, sphere/arrow marker 생성 |
| `src/sensor_layout.cpp` | YAML layout 로드, 값 검증, 센서 normal 정규화 |
| `src/sensor_panel.cpp` | 손 이미지 렌더링, 센서 수치/상태/색상 표시 |
| `test/test_sensor_layout.cpp` | 색상 mapping, 양손 layout, 센서 순서와 좌표 검증 |
| `CMakeLists.txt` | C++ node/library 빌드, asset/config/launch 설치, 테스트 등록 |
| `package.xml` | ROS 2 package 정보와 의존성 |
| `LICENSE` | 패키지 코드 라이선스 |
| `THIRD_PARTY_NOTICES.md` | Qt, 이미지 등 제3자 구성요소 고지 |
| `README.md` | 패키지 구조, interface, 실행법 영문 문서 |
| `README_KR.md` | 패키지 구조, interface, 실행법 국문 문서 |

## 추가 의존성 및 설치

이 패키지는 ROS 2 Humble/Ubuntu 22.04(Jammy)와 ROS 2 Jazzy/Ubuntu 24.04(Noble)를 지원한다. ROS 2가 이미 설치되어 있고 `/opt/ros/$ROS_DISTRO/setup.bash`를 source할 수 있어야 한다.

직접 사용하는 추가 의존성은 다음과 같다.

| 구분 | 패키지 |
|---|---|
| 빌드 도구 | C++17 compiler, `ament_cmake`, `colcon` |
| GUI | Qt 5 Core/Gui/Widgets, `qtbase5-dev` |
| 설정 | `yaml-cpp`, `yaml_cpp_vendor` |
| ROS 2 | `ament_index_cpp`, `rclcpp`, `std_msgs`, `visualization_msgs` |
| 3D 화면 | `rviz_common`, `rviz_default_plugins`, `rviz_rendering` |
| Launch/runtime | Python 3, `launch`, `launch_ros`, Qt 5 SVG runtime |
| 테스트 | `ament_cmake_gtest` |

Workspace root에서 `rosdep`으로 현재 ROS 배포판에 맞는 패키지를 설치하는 방법을 권장한다.

```bash
# 설치한 배포판 하나를 선택한다:
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
sudo apt update
sudo apt install python3-rosdep python3-colcon-common-extensions
# 시스템에서 rosdep을 처음 사용하는 경우에만: sudo rosdep init
rosdep update
rosdep install --from-paths allegro_hand_v6_dashboard \
  --ignore-src --rosdistro "$ROS_DISTRO" -r -y
```

`rosdep`을 사용하지 않을 경우 핵심 패키지는 다음처럼 직접 설치할 수 있다. `$ROS_DISTRO`는 `humble` 또는 `jazzy`여야 한다.

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

배포판별 source 명령과 ROS package 접두사만 다르며 소스 코드는 동일하다.

| ROS 2 | 권장 Ubuntu | 차이점 |
|---|---|---|
| Humble | 22.04 Jammy | `ros-humble-*`; Qt runtime은 `libqt5core5a`, `libqt5gui5`, `libqt5widgets5` 계열 |
| Jazzy | 24.04 Noble | `ros-jazzy-*`; Ubuntu의 time64 전환으로 Qt runtime은 `libqt5core5t64`, `libqt5gui5t64`, `libqt5widgets5t64` 계열 |

런타임 Qt 라이브러리 이름을 직접 지정하지 말고 `qtbase5-dev`와 `rosdep`에 맡기면 두 배포판 차이를 자동으로 처리한다. 3D 패널을 사용하려면 RViz 및 정상적인 OpenGL/X11(또는 호환 display server) 환경도 필요하다.

## 패키지 담당 기능

- `RobotModel`, grid, 촉각 marker를 포함한 RViz 화면을 Qt 창에 내장한다.
- 18채널 촉각 값을 손 이미지 위의 숫자와 색상으로 표시한다.
- 손끝 센서는 반투명 구, 나머지 손가락/손바닥 센서는 화살표로 표현한다.
- 오른손과 왼손의 이미지, sensor 좌표, marker 좌표를 별도 YAML로 관리한다.
- 마지막 촉각 수신 후 일정 시간이 지나면 2D 패널에 stale 상태를 표시한다.

## 실행 노드

### `allegro_hand_v6_dashboard_node`

Qt Dashboard 본체이다. 실행 node 이름은 `allegro_hand_v6_dashboard`이다.

- `show_visualization=true`: RViz 3D 패널 표시
- `show_sensor_panel=true`: PNG 기반 2D 센서 패널 표시
- 두 값이 모두 `false`이면 오류로 종료

두 값은 launch 인자가 아니라 ROS parameter이다. `dashboard.launch.py`는 대신 `VISUALIZE`와 `DEMO`를 제공하며, `show_visualization`은 `VISUALIZE`를 따르고 `show_sensor_panel`은 `VISUALIZE` 또는 `DEMO` 중 하나라도 true이면 활성화된다. `show_sensor_panel`만 따로 지정하려면 실행 파일을 직접 실행한다.

```bash
ros2 run allegro_hand_v6_dashboard allegro_hand_v6_dashboard_node \
  --ros-args -p show_visualization:=false -p show_sensor_panel:=true
```

## ROS 2 토픽

### 직접 사용하는 토픽

| 방향 | 토픽 | 자료형 | 기능 |
|---|---|---|---|
| Subscribe | `/allegro_hand/tactile_pressures` | `std_msgs/msg/Float64MultiArray` | 18채널 촉각 압력 입력. 기본 단위는 hPa |
| Publish | `/allegro_hand_v6/tactile_markers` | `visualization_msgs/msg/MarkerArray` | RViz에 표시할 손끝 sphere 5개와 나머지 센서 arrow 13개 |

토픽명은 `pressure_topic`, `marker_topic` parameter로 변경할 수 있다.

### 임베디드 RViz가 사용하는 토픽

| 방향 | 토픽 | 자료형 | 기능 |
|---|---|---|---|
| Subscribe | `/robot_description` | `std_msgs/msg/String` | URDF robot model 수신 |
| Subscribe | `/tf` | `tf2_msgs/msg/TFMessage` | 움직이는 link transform 수신 |
| Subscribe | `/tf_static` | `tf2_msgs/msg/TFMessage` | 정적 transform 수신 |
| Subscribe | `/allegro_hand_v6/tactile_markers` | `visualization_msgs/msg/MarkerArray` | 이 node가 생성한 촉각 marker 표시 |

3D marker는 `/tf` publisher가 존재할 때부터 발행된다. 독립 실행 시 robot model과 TF publisher가 없으면 2D 패널은 동작하지만 3D hand/marker는 정상 표시되지 않는다.

## 촉각 데이터 배열

`Float64MultiArray.data`는 정확히 18개 값을 기대한다. `layout` 필드는 사용하지 않으며 배열 index가 센서 위치를 결정한다. Marker ID도 같은 channel index를 사용하고 marker frame과 위치는 tactile layout YAML에서 읽는다.

| Index | 센서 | Index | 센서 |
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

채널 순서는 `thumb → index → middle → ring → pinky → palm`이다. 각 손가락 내부 순서는 `tip → middle → base`이고, 손바닥은 `bottom → center → upper/left`이다.

## Tactile layout YAML

`hand` 또는 `sensor_config`로 선택되는 layout 파일이 압력 범위와 센서 배치를 함께 정의한다. 압력 범위는 ROS parameter가 **아니며** 이 파일에서 변경한다.

| Key | 자료형 | 기본값 | 기능 |
|---|---|---|---|
| `unit` | string | `hPa` | 2D 색상 scale에 표시할 단위 |
| `pressure_min` | double | `1000.0` | 색상 및 marker 크기 scale 하한 |
| `pressure_max` | double | `1200.0` | 색상 및 marker 크기 scale 상한 |
| `svg_width` | double | `600.0` | `svg` 좌표가 사용하는 좌표계 너비 |
| `svg_height` | double | `900.0` | `svg` 좌표가 사용하는 좌표계 높이 |
| `image_crop` | `[x, y, w, h]` | 전체 이미지 | 렌더링 전 적용할 원본 이미지 crop. 원본 이미지 범위 안이어야 한다 |
| `image_scale` | double | `1.0` | 2D 이미지 배율. 양수여야 한다 |
| `image_offset` | `[x, y]` | `[0, 0]` | 2D 이미지 그리기 offset |
| `sensors` | list | 필수 | 센서 정의. loader는 최소 1개를 요구하며 dashboard는 18채널 전체를 기대한다 |

`sensors` 항목의 각 field는 다음과 같다.

| Field | 자료형 | 기본값 | 기능 |
|---|---|---|---|
| `index` | int | 필수 | 촉각 배열의 channel index `0..17` |
| `name` | string | 필수 | 내부 센서 식별자 |
| `label` | string | `name` | 2D 패널에 표시할 짧은 label |
| `group` | string | 필수 | `Thumb`, `Index`, `Middle`, `Ring`, `Pinky`, `Palm` |
| `frame_id` | string | 필수 | 3D marker가 부착될 TF frame |
| `position` | `[x, y, z]` | 필수 | `frame_id` 기준 marker 위치(m) |
| `normal` | `[x, y, z]` | 필수 | Marker 방향. 로드 시 정규화된다 |
| `svg` | `[x, y]` | 필수 | 2D 패널의 텍스트 위치 |
| `text_rotation` | double | `0.0` | 2D label 회전 각도(도) |

```yaml
unit: hPa
pressure_min: 1000.0
pressure_max: 1200.0
sensors:
  - {index: 0, name: thumb_tip, label: TT, group: Thumb, frame_id: R14_Link,
     position: [0.0125, -0.0217, 0.001], normal: [0, 0, 1], svg: [1147, 688], text_rotation: 59}
```

## 압력 시각화 규칙

기본 pressure 범위는 `1000..1200 hPa`이며 위의 tactile layout YAML에서 읽는다.

| 정규화 위치 | 색상 |
|---:|---|
| 최소값 이하 | 파랑 |
| 25% | 청록 |
| 50% | 초록 |
| 75% | 황록 |
| 최대값 이상 | 빨강 |

- 실제 숫자는 범위 밖 값도 그대로 표시한다.
- 색상과 marker 크기 계산만 `pressure_min..pressure_max`로 clamp한다.
- 손끝 구 기본 지름은 `20 mm`, 최대 지름은 `55 mm`이다.
- 유효한 손끝 구 alpha는 `0.4`이다.
- 유효하지 않거나 누락된 값은 회색으로 표시한다.
- 손끝 이외 센서는 압력에 따라 arrow 길이와 굵기가 증가한다.

## 주요 ROS parameter

| Parameter | 기본값 | 기능 |
|---|---:|---|
| `hand` | `right` | `right` 또는 `left`; 기본 config와 image 선택 |
| `sensor_config` | 빈 문자열 | 비어 있으면 손 방향별 기본 tactile layout 사용 |
| `svg_path` | 빈 문자열 | 비어 있으면 손 방향별 기본 PNG 사용. 이름은 호환성을 위해 유지됨 |
| `show_visualization` | `true` | RViz 3D 패널 표시 여부 |
| `show_sensor_panel` | `true` | 2D 센서 패널 표시 여부 |
| `fixed_frame` | `world` | RViz fixed frame |
| `robot_description_topic` | `/robot_description` | RobotModel description 토픽 |
| `pressure_topic` | `/allegro_hand/tactile_pressures` | 촉각 입력 토픽 |
| `marker_topic` | `/allegro_hand_v6/tactile_markers` | marker 출력 토픽 |
| `stale_timeout` | `1.0` | 2D 패널이 stale로 판단하는 수신 중단 시간(초) |
| `view_yaw` | `3.403392` | RViz Orbit yaw(rad) |
| `view_pitch` | `0.15` | RViz Orbit pitch(rad) |
| `view_distance` | `0.60` | RViz camera 거리(m) |
| `view_focal_z` | `0.055` | RViz focal point Z(m) |

## 실행 방법

별도 terminal에서 V6 core bringup을 먼저 실행한다.

```bash
ros2 launch allegro_hand_v6_bringup bringup.launch.py
```

다른 terminal에서 Dashboard를 실행한다.

```bash
ros2 launch allegro_hand_v6_dashboard dashboard.launch.py \
  VISUALIZE:=true HAND:=right
```

2D 센서 패널만 열려면:

```bash
ros2 launch allegro_hand_v6_dashboard dashboard.launch.py \
  DEMO:=true VISUALIZE:=false HAND:=right
```

Dashboard launch는 hardware, controller 또는 `robot_state_publisher`를 시작하지 않는다. 3D robot model을 표시하려면 별도의 V6 bringup이 실행 중이어야 한다. 2D 센서 패널만 실행할 수는 있지만 촉각 message가 없으면 stale 상태로 유지된다.

## 빌드 및 테스트

```bash
colcon build --packages-select allegro_hand_v6_dashboard --symlink-install
colcon test --packages-select allegro_hand_v6_dashboard
colcon test-result --test-result-base build/allegro_hand_v6_dashboard --verbose
```

## 라이선스

패키지 코드는 BSD-3-Clause이다. Qt는 운영체제의 LGPL 동적 라이브러리를 사용한다. 세부 고지는 `LICENSE`와 `THIRD_PARTY_NOTICES.md`를 참고한다.
