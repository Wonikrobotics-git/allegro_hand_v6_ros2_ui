cmake_minimum_required(VERSION 3.8)
project(allegro_hand_v6_gui)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(yaml-cpp REQUIRED)

add_executable(allegro_hand_v6_gui_node
  src/main.cpp
  src/main_window.cpp
  include/allegro_hand_v6_gui/main_window.hpp
)

target_include_directories(allegro_hand_v6_gui_node PRIVATE include)
target_link_libraries(allegro_hand_v6_gui_node Qt5::Widgets yaml-cpp)
target_compile_definitions(allegro_hand_v6_gui_node PRIVATE
  ALLEGRO_HAND_V6_GUI_DEFAULT_POSE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/poses"
)
ament_target_dependencies(allegro_hand_v6_gui_node
  rclcpp
  sensor_msgs
  std_msgs
)

install(TARGETS allegro_hand_v6_gui_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY poses/
  DESTINATION share/${PROJECT_NAME}/poses
)

install(FILES README.md README_KR.md
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
