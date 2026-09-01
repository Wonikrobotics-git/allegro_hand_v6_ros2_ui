# Allegro Hand V6 GUI

[English](README.md)

Allegro Hand V6의 현재 자세 저장, 저장 자세 실행, 반복 pose sequence 실행을 제공하는 Qt 기반 ROS 2 제어 패키지이다.

## 디렉터리 및 파일 구성

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

| 경로 | 기능 |
|---|---|
| `include/allegro_hand_v6_gui/` | GUI C++ header 보관 |
| `poses/` | 저장 pose와 예제 pose 보관 |
| `src/` | Qt GUI와 ROS 연동 구현 보관 |
| `include/.../main_window.hpp` | Qt MainWindow, ROS publisher/subscriber, pose와 sequence 상태 정의 |
| `poses/` | 기본 pose YAML 저장 디렉터리 |
| `poses/.gitkeep` | 빈 pose 디렉터리를 Git에 유지하기 위한 파일 |
| `poses/1.yaml` | 20관절 zero pose 예제 |
| `src/main.cpp` | ROS 2/Qt 초기화, 20 ms 주기 `rclcpp::spin_some()`, GUI event loop 실행 |
| `src/main_window.cpp` | UI 생성, joint state 수신, pose 저장/로드/발행과 sequence 제어 구현 |
| `CMakeLists.txt` | Qt/yaml-cpp 기반 node 빌드, 기본 pose 경로 정의, 실행 파일과 pose 설치 |
| `package.xml` | ROS 2 package 정보와 의존성 |
| `README.md` | 패키지 구조, interface, pose 사용법 영문 문서 |
| `README_KR.md` | 패키지 구조, interface, pose 사용법 국문 문서 |

## 추가 의존성 및 설치

이 패키지는 ROS 2 Humble/Ubuntu 22.04(Jammy)와 ROS 2 Jazzy/Ubuntu 24.04(Noble)를 지원한다. ROS 2가 이미 설치되어 있고 `/opt/ros/$ROS_DISTRO/setup.bash`를 source할 수 있어야 한다.

| 구분 | 패키지 |
|---|---|
| 빌드 도구 | C++17 compiler, `ament_cmake`, `colcon` |
| GUI | Qt 5 Core/Gui/Widgets, `qtbase5-dev` |
| Pose YAML | `yaml-cpp`, `yaml_cpp_vendor` |
| ROS 2 | `rclcpp`, `sensor_msgs`, `std_msgs` |

Workspace root에서 `rosdep`으로 설치하는 방법을 권장한다.

```bash
# 설치한 배포판 하나를 선택한다:
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
sudo apt update
sudo apt install python3-rosdep python3-colcon-common-extensions
# 시스템에서 rosdep을 처음 사용하는 경우에만: sudo rosdep init
rosdep update
rosdep install --from-paths allegro_hand_v6_gui \
  --ignore-src --rosdistro "$ROS_DISTRO" -r -y
```

`rosdep`을 사용하지 않을 경우 핵심 패키지는 다음처럼 직접 설치할 수 있다.

```bash
sudo apt install qtbase5-dev libyaml-cpp-dev \
  ros-${ROS_DISTRO}-rclcpp \
  ros-${ROS_DISTRO}-sensor-msgs \
  ros-${ROS_DISTRO}-std-msgs \
  ros-${ROS_DISTRO}-yaml-cpp-vendor
```

Humble에서는 `ros-humble-*`, Jazzy에서는 `ros-jazzy-*` 패키지를 사용한다. 두 배포판 모두 Qt 5 개발 패키지 이름은 `qtbase5-dev`이지만, Ubuntu 24.04의 Qt runtime은 `libqt5core5t64`, `libqt5gui5t64`, `libqt5widgets5t64`처럼 `t64` 이름을 사용한다. 이 전이 의존성은 `qtbase5-dev` 또는 `rosdep`이 자동으로 선택하므로 runtime package를 직접 고정하지 않는다.

GUI를 실행하려면 X11/Wayland display 환경이 필요하다. Pose를 실제 손에 적용하려면 이 패키지 외에 `/joint_states` publisher와 `/allegro_hand_position_controller/commands` subscriber를 제공하는 controller 및 hardware/mock-hardware bringup이 실행 중이어야 한다.

## 패키지 담당 기능

- `/joint_states`에서 최신 20관절 위치 수신
- 현재 위치를 YAML pose로 저장
- 저장된 YAML pose를 읽어 position controller에 전달
- 여러 pose를 순서대로 구성하고 일정 간격으로 반복 실행
- pose 저장/실행 오류와 sequence 상태를 GUI log에 표시
- GUI 종료 시 실행 중인 sequence 정지

실행 node와 executable 이름은 다음과 같다.

| 구분 | 이름 |
|---|---|
| Executable | `allegro_hand_v6_gui_node` |
| Node | `allegro_hand_v6_gui` |

## 화면 구성

### Joint State

- `Joint state: ready (20/20)`: 완전하고 유효한 20관절 상태를 받은 경우 표시

### Saved Poses

- Pose 이름 입력 + `Save Current`: 현재 20관절 위치 저장
- `Refresh`: pose 디렉터리의 `*.yaml`, `*.yml` 목록 새로 읽기
- `Move`: 선택한 pose를 position controller에 한 번 발행
- Pose 목록 더블클릭: `Move`와 동일
- `Add to Sequence`: 선택한 pose를 오른쪽 sequence 끝에 추가

### Pose Sequence

- `Remove`: 선택 항목 제거
- `Up`, `Down`: 선택 항목 순서 변경
- `Clear`: sequence 전체 삭제
- `Pose interval`: pose 명령 사이 간격, `0.1..60.0 s`, 기본 `2.0 s`
- `Repeat count`: 전체 sequence 반복 횟수, `1..999`, 기본 `1`
- `Start`, `Stop`: sequence 시작/중단

### Log

Pose 저장/실행, sequence 시작/완료/중단과 오류를 시각과 함께 기록한다. 최대 500개 block을 유지한다.

## ROS 2 토픽

| 방향 | 기본 토픽 | 자료형 | 데이터 및 기능 |
|---|---|---|---|
| Subscribe | `/joint_states` | `sensor_msgs/msg/JointState` | `name[]`과 `position[]`에서 현재 20관절 위치(rad) 수신 |
| Publish | `/allegro_hand_position_controller/commands` | `std_msgs/msg/Float64MultiArray` | `data[0..19]`에 `joint00..joint43` 목표 위치(rad) 발행 |

GUI는 `/joint_states.name` 순서를 신뢰하지 않는다. 관절 이름으로 값을 찾아 아래 물리 순서로 재배열한다.

```text
joint00 joint01 joint02 joint03
joint10 joint11 joint12 joint13
joint20 joint21 joint22 joint23
joint30 joint31 joint32 joint33
joint40 joint41 joint42 joint43
```

`JointState.name`이 비어 있으면 `position`의 처음 20개가 이미 위 순서라고 가정한다. 위치가 20개보다 적거나 관절 이름이 하나라도 없거나 값에 NaN/Infinity가 있으면 해당 message를 사용하지 않는다.

## ROS parameter

| Parameter | 기본값 | 기능 |
|---|---|---|
| `joint_state_topic` | `/joint_states` | 현재 관절 상태 입력 토픽 |
| `position_command_topic` | `/allegro_hand_position_controller/commands` | Pose 목표 출력 토픽 |
| `pose_directory` | 빈 문자열 | Pose 저장/검색 디렉터리 override |

`pose_directory`가 비어 있으면 빌드 시 `ALLEGRO_HAND_V6_GUI_DEFAULT_POSE_DIR`에 기록된 source workspace의 `allegro_hand_v6_gui/poses/`를 사용한다. Workspace를 이동했다면 다시 빌드하거나 절대경로 parameter를 지정해야 한다.

예:

```bash
ros2 run allegro_hand_v6_gui allegro_hand_v6_gui_node \
  --ros-args -p pose_directory:=/absolute/path/to/poses
```

## 자세 저장 방법

1. `/joint_states`에서 20개 관절 상태가 모두 들어오는지 확인한다.
2. 저장할 자세로 손을 이동한다.
3. `Saved Poses` 입력란에 pose 이름을 입력한다.
4. `Save Current`를 누르거나 Enter를 누른다.
5. 저장 성공 후 목록이 자동 갱신된다.

저장 조건:

- Pose 이름은 영문자, 숫자, `_`, `-`만 허용한다.
- 완전한 20관절 상태를 한 번 이상 받아야 한다.
- 마지막 joint state가 2초보다 오래되었으면 저장하지 않는다.
- 같은 이름이 존재하면 덮어쓸지 확인한다.
- 파일 확장자는 `.yaml`이다.

## Pose YAML 형식

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

- `position`은 반드시 유한한 숫자 20개를 포함해야 한다.
- `joint_names`가 있으면 반드시 이름 20개를 포함해야 한다. GUI는 이름을 기준으로 `position`을 표준 관절 순서로 재정렬한다.
- `joint_names`가 없으면 `position`이 이미 표준 순서라고 간주한다.
- 알 수 없는 YAML, 누락 관절, 잘못된 배열 길이, NaN/Infinity가 있으면 실행하지 않는다.

## 저장 자세 불러오기 및 실행

1. `Refresh`를 눌러 pose 파일 목록을 갱신한다.
2. 왼쪽 목록에서 pose를 선택한다.
3. `Move`를 누르거나 항목을 더블클릭한다.
4. GUI가 YAML을 검증하고 표준 순서의 20개 위치를 `/allegro_hand_position_controller/commands`로 한 번 발행한다.

`Move`는 trajectory interpolation이나 도달 완료 확인을 수행하지 않는다. 목표 position 한 frame을 발행하며 실제 이동 속도와 응답은 controller/hardware 설정에 따른다.

## Pose sequence 만들기

1. 왼쪽 pose를 선택하고 `Add to Sequence`를 누른다.
2. 필요한 pose를 순서대로 반복 추가한다.
3. `Up`, `Down`, `Remove`로 목록을 편집한다.
4. `Pose interval`과 `Repeat count`를 설정한다.
5. `Start`를 누른다.

첫 pose는 `Start` 직후 발행되고 이후 pose는 설정한 interval마다 발행된다. 마지막 pose와 다음 반복의 첫 pose 사이에도 같은 interval이 적용된다.

같은 pose를 바로 연속해서 추가하는 것은 막혀 있다. `open → fist → open`처럼 중간에 다른 pose가 있으면 다시 추가할 수 있다. `Start` 시점의 목록을 복사하여 실행하므로 실행 중 화면 목록을 수정해도 현재 실행 목록에는 반영되지 않는다.

Sequence 목록 자체는 파일로 저장되지 않는다. GUI를 종료하면 사라지며 pose YAML 파일만 유지된다.

`Move`와 `Start`는 position controller에 목표 위치만 발행한다. 하드웨어 활성화와 안전 상태는 bringup 및 하드웨어 interface에서 관리한다.

## 실행 방법

별도 terminal에서 V6 core bringup을 먼저 실행한다.

```bash
ros2 launch allegro_hand_v6_bringup bringup.launch.py
```

다른 terminal에서 GUI node를 실행한다.

```bash
ros2 run allegro_hand_v6_gui allegro_hand_v6_gui_node
```

GUI는 hardware나 controller를 시작하지 않는다. 별도로 실행한 bringup이 `/joint_states`와 `/allegro_hand_position_controller/commands` subscriber를 제공해야 한다.

> **주의:** 실제 손을 움직이기 전에 주변 간섭물과 비상 정지 수단을 확인한다. 처음 실행하는 pose와 sequence는 충분히 긴 interval과 안전한 관절 범위로 시험한다.

## 빌드

```bash
colcon build --packages-select allegro_hand_v6_gui --symlink-install
```

## 라이선스

`package.xml`에 BSD로 선언되어 있다. 현재 이 패키지에는 `LICENSE` 파일이 포함되어 있지 않다.
