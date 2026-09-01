# Allegro Hand V6 Web Plot

[English](README.md)

Allegro Hand V6의 관절 위치, 목표 위치, effort, 온도, 촉각 값을 웹 브라우저에서 실시간 확인하는 ROS 2 monitoring 패키지이다. Python bridge가 ROS 토픽을 수집해 WebSocket JSON으로 변환하고 정적 HTML 화면이 이를 그래프로 표시한다.

## 디렉터리 및 파일 구성

```text
allegro_hand_v6_web_plot/
├── scripts/
│   └── web_plot_bridge.py
├── web/
│   └── index.html
├── CMakeLists.txt
├── package.xml
├── README.md
└── README_KR.md
```

| 경로 | 기능 |
|---|---|
| `scripts/` | ROS-to-Web bridge Python node 보관 |
| `web/` | 브라우저 정적 page와 설치 시 복사되는 logo 보관 |
| `scripts/web_plot_bridge.py` | ROS subscriber, HTTP server, WebSocket server, 30 Hz JSON broadcast 구현 |
| `web/index.html` | 관절 선택 UI, 10초 시계열 canvas plot, 촉각 bar 화면 구현 |
| `CMakeLists.txt` | Python executable과 web asset 설치 |
| `package.xml` | ROS 2 package 정보와 Python runtime 의존성 |
| `README.md` | 패키지 구조, interface, web monitor 실행법 영문 문서 |
| `README_KR.md` | 패키지 구조, interface, web monitor 실행법 국문 문서 |

`scripts/__pycache__/`와 `*.pyc`는 Python이 만든 cache이며 패키지 기능 파일이 아니다.

Logo `WR_EN_high.png`는 source package의 `web/` 디렉터리에 있으며 `CMakeLists.txt`가 install space의 `share/allegro_hand_v6_web_plot/web/`로 복사한다.

```text
web/WR_EN_high.png
```

## 추가 의존성 및 설치

이 패키지는 ROS 2 Humble/Ubuntu 22.04(Jammy)와 ROS 2 Jazzy/Ubuntu 24.04(Noble)를 지원한다. ROS 2가 이미 설치되어 있고 `/opt/ros/$ROS_DISTRO/setup.bash`를 source할 수 있어야 한다.

| 구분 | 패키지 |
|---|---|
| 빌드 도구 | `ament_cmake`, `colcon` |
| ROS 2 Python | `rclpy`, `ament_index_python` |
| ROS message | `sensor_msgs`, `std_msgs` |
| WebSocket | `python3-websockets` |
| HTTP server/browser UI | Python 3 standard library 및 JavaScript 지원 web browser |

Workspace root에서 `rosdep`으로 설치하는 방법을 권장한다.

```bash
# 설치한 배포판 하나를 선택한다:
source /opt/ros/humble/setup.bash
# source /opt/ros/jazzy/setup.bash
sudo apt update
sudo apt install python3-rosdep python3-colcon-common-extensions
# 시스템에서 rosdep을 처음 사용하는 경우에만: sudo rosdep init
rosdep update
rosdep install --from-paths allegro_hand_v6_web_plot \
  --ignore-src --rosdistro "$ROS_DISTRO" -r -y
```

`rosdep`을 사용하지 않을 경우 핵심 runtime은 다음처럼 직접 설치할 수 있다.

```bash
sudo apt install python3-websockets \
  ros-${ROS_DISTRO}-ament-index-python \
  ros-${ROS_DISTRO}-rclpy \
  ros-${ROS_DISTRO}-sensor-msgs \
  ros-${ROS_DISTRO}-std-msgs
```

| ROS 2 | 권장 Ubuntu | Ubuntu 기본 `python3-websockets` |
|---|---|---:|
| Humble | 22.04 Jammy | 9.1 |
| Jazzy | 24.04 Noble | 10.4 |

Bridge는 두 배포판의 handler 호출 형식을 모두 처리한다. `pip`로 별도 버전을 덮어쓰기보다 Ubuntu의 `python3-websockets`를 사용하는 것을 권장한다. Browser는 별도 JavaScript library가 필요하지 않지만 HTTP와 WebSocket port에 접근할 수 있어야 한다.

## 패키지 담당 기능

- 네 개 ROS 2 토픽을 하나의 상태 구조로 수집한다.
- `/joint_states` 배열을 관절 이름 기준으로 `joint00..joint43` 순서로 정렬한다.
- 정적 web 파일을 HTTP로 제공한다.
- 최신 상태를 WebSocket JSON으로 약 30 Hz 전송한다.
- Browser에서 최근 10초 데이터를 관절별로 선택해 표시한다.
- 촉각 데이터는 별도 Sensors 탭에서 bar로 표시한다.

데이터 흐름:

```text
ROS 2 topics
    ↓
web_plot_bridge.py
    ├── HTTP :8080 ─────→ index.html
    └── WebSocket :9091 → 실시간 JSON → browser chart
```

실행 node와 executable 이름:

| 구분 | 이름 |
|---|---|
| Executable | `web_plot_bridge` |
| Node | `allegro_hand_v6_web_plot` |

## ROS 2 토픽

이 패키지는 ROS 토픽을 발행하지 않고 다음 토픽을 구독한다.

| 방향 | 토픽 | 자료형 | 사용하는 필드 | 기능 |
|---|---|---|---|---|
| Subscribe | `/joint_states` | `sensor_msgs/msg/JointState` | `name`, `position`, `effort` | 현재 관절 위치와 effort 수집 |
| Subscribe | `/allegro_hand_position_controller/commands` | `std_msgs/msg/Float64MultiArray` | `data[0:20]` | 최근 20관절 목표 위치 수집 |
| Subscribe | `/allegro_hand_v6/joint_temperatures` | `std_msgs/msg/Float32MultiArray` | `data[0:20]` | 최근 20관절 온도 수집(아래 주의 참고) |
| Subscribe | `/allegro_hand/tactile_pressures` | `std_msgs/msg/Float64MultiArray` | `data[0:18]` | 촉각 센서 압력(hPa) 수집 |

### 관절 데이터 순서와 단위

- 관절 이름 순서: `joint00..joint03`, `joint10..joint13`, ..., `joint40..joint43`
- `position`: `/joint_states.position`, 단위 rad
- `desired`: position controller command, 단위 rad
- `effort`: `/joint_states.effort`, 현재 V6 hardware에서는 관절 토크 `N·m`이다.
- `temperature`: `/allegro_hand_v6/joint_temperatures`, 단위 °C
- `pressure`: `/allegro_hand/tactile_pressures`, 18채널 hPa 배열이다.

`/joint_states`의 resource 순서는 보장되지 않으므로 이름을 기준으로 재정렬한다. 누락된 관절이나 NaN 값은 WebSocket JSON에서 `null`로 전달된다.

> **주의:** 기본 bringup에는 `/allegro_hand_v6/joint_temperatures`를 발행하는 노드가 없다. Hardware interface는 관절 온도를 `ros2_control` state interface로만 노출하고, 설정된 `joint_state_broadcaster`는 이를 토픽으로 내보내지 않는다. 따라서 기본 구성에서는 `Temperature` 탭이 비어 있고 해당 timestamp도 `null`로 유지된다. 온도를 표시하려면 이 토픽을 발행하는 노드를 직접 실행하거나 `temperature` state interface를 내보내는 broadcaster를 추가해야 한다.

## WebSocket JSON 형식

WebSocket server는 다음 형태의 최신 snapshot을 전송한다.

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

각 timestamp는 ROS clock 기준 초 단위이며 해당 종류의 마지막 message 수신 시각이다. 아직 수신하지 않은 데이터 종류는 `null`이다.

## Browser 화면

| 탭 | 표시 내용 |
|---|---|
| `Angles` | 현재 position 실선과 desired position 점선 |
| `Torque` | `/joint_states.effort` 관절 토크(N·m) |
| `Temperature` | 20관절 온도. 온도 publisher가 없으면 비어 있다 |
| `Sensors` | 촉각 배열 bar |

- 그래프 window는 최근 10초이다.
- Thumb, Index, Middle, Ring, Pinky 단위 또는 개별 관절을 선택할 수 있다.
- 초기 선택은 Thumb의 네 관절이다.
- Y축은 현재 보이는 유효 값 범위에 따라 자동 조절된다.
- WebSocket 연결이 끊기면 1.5초 후 재연결한다.

### 촉각 bar 동작

Browser는 18채널을 `Thumb 0~2 → Index 3~5 → Middle 6~8 → Ring 9~11 → Pinky 12~14 → Palm 15~17` 순서로 표시한다. Bar 높이는 V6 기본 압력 범위인 `1000..1200 hPa`를 기준으로 계산하고 범위 밖 값은 표시 영역에서 clamp한다.

## ROS parameter 및 network port

| Parameter | 기본값 | 기능 |
|---|---:|---|
| `http_port` | `8080` | `index.html`과 asset을 제공하는 HTTP port |
| `ws_port` | `9091` | 실시간 JSON WebSocket server port |

두 server는 `0.0.0.0`에 bind하므로 같은 network의 다른 장치에서도 접속할 수 있다.

> **보안 주의:** HTTP와 WebSocket에 인증 및 TLS가 없다. 신뢰할 수 있는 내부 network에서만 사용하고 외부 network에 직접 노출하지 않는다.

`web/index.html`의 WebSocket URL은 현재 port `9091`로 고정되어 있다. `ws_port` parameter만 변경하면 browser가 새 port를 알 수 없으므로 `index.html`의 URL도 함께 수정해야 한다.

## 실행 방법

별도 terminal에서 V6 core bringup을 먼저 실행한다.

```bash
ros2 launch allegro_hand_v6_bringup bringup.launch.py
```

다른 terminal에서 Web Plot을 실행한다.

```bash
ros2 run allegro_hand_v6_web_plot web_plot_bridge
```

실행 후 같은 PC에서 다음 주소를 연다.

```text
http://localhost:8080
```

Port 지정 예:

```bash
ros2 run allegro_hand_v6_web_plot web_plot_bridge \
  --ros-args -p http_port:=8088
```

이 package에는 자체 launch 파일이 없으며 hardware나 controller를 시작하지 않는다. 별도로 실행한 V6 bringup이 발행하는 데이터를 표시한다.

## 빌드

```bash
colcon build --packages-select allegro_hand_v6_web_plot --symlink-install
```

Runtime에는 `rclpy`, `ament_index_python`, `sensor_msgs`, `std_msgs`, Python `websockets`가 필요하다.

## 라이선스

`package.xml`에 BSD로 선언되어 있다. 현재 이 패키지에는 `LICENSE` 파일이 포함되어 있지 않다.
