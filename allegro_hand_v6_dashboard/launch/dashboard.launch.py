# Copyright (c) 2026 Wonik Robotics
# SPDX-License-Identifier: BSD-3-Clause

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    visualize = LaunchConfiguration("VISUALIZE")
    demo = LaunchConfiguration("DEMO")
    sensor_config = LaunchConfiguration("sensor_config")
    hand = LaunchConfiguration("HAND")
    view_yaw = LaunchConfiguration("view_yaw")
    view_pitch = LaunchConfiguration("view_pitch")
    view_distance = LaunchConfiguration("view_distance")
    view_focal_z = LaunchConfiguration("view_focal_z")
    any_panel = PythonExpression(
        [
            "'", visualize, "'.lower() in ['true', '1', 'yes', 'on'] or '",
            demo, "'.lower() in ['true', '1', 'yes', 'on']",
        ]
    )

    return LaunchDescription(
        [
            DeclareLaunchArgument("VISUALIZE", default_value="true"),
            DeclareLaunchArgument(
                "DEMO",
                default_value="false",
                description="Compatibility flag for the PNG-only dashboard",
            ),
            DeclareLaunchArgument("HAND", default_value="right"),
            DeclareLaunchArgument(
                "view_yaw",
                default_value="3.403392",
                description="RViz Orbit yaw in radians (195 degrees)",
            ),
            DeclareLaunchArgument(
                "view_pitch",
                default_value="0.15",
                description="RViz Orbit pitch in radians",
            ),
            DeclareLaunchArgument(
                "view_distance",
                default_value="0.60",
                description="RViz Orbit camera distance in meters",
            ),
            DeclareLaunchArgument(
                "view_focal_z",
                default_value="0.055",
                description="RViz Orbit focal-point Z coordinate in meters",
            ),
            DeclareLaunchArgument(
                "sensor_config",
                default_value="",
                description="Optional layout override; selected from hand when empty",
            ),
            Node(
                package="allegro_hand_v6_dashboard",
                executable="allegro_hand_v6_dashboard_node",
                name="allegro_hand_v6_dashboard",
                output="screen",
                condition=IfCondition(any_panel),
                parameters=[
                    {
                        "show_visualization": ParameterValue(visualize, value_type=bool),
                        "show_sensor_panel": ParameterValue(
                            any_panel, value_type=bool
                        ),
                        "sensor_config": sensor_config,
                        "hand": hand,
                        "view_yaw": ParameterValue(view_yaw, value_type=float),
                        "view_pitch": ParameterValue(
                            view_pitch, value_type=float
                        ),
                        "view_distance": ParameterValue(
                            view_distance, value_type=float
                        ),
                        "view_focal_z": ParameterValue(
                            view_focal_z, value_type=float
                        ),
                    }
                ],
                arguments=[
                    "--ros-args",
                    "--log-level",
                    "rviz_common:=warn",
                    "--log-level",
                    "rviz_rendering:=warn",
                ],
                additional_env={"RCUTILS_LOGGING_MIN_SEVERITY": "20"},
            ),
        ]
    )
