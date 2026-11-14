#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""用 launch.yaml 当参数，运行 nav2_costmap_2d 包里的同名程序 nav2_costmap_2d。

    ros2 launch /home/hqtebk08/launch.yaml 的目录/costmap_runtime.launch.py
    ros2 launch costmap_runtime.launch.py params_file:=$HOME/launch.yaml

三件事值得注意：

1. 可执行程序内部创建出来的节点默认叫 /costmap/costmap，
   而 launch.yaml 的顶层键是 global_costmap（对应节点全名 /global_costmap）。
   所以这里把节点重命名成 global_costmap、命名空间拉回根，
   参数文件的段名才对得上（对不上就静默用默认值）。

2. costmap 是生命周期节点：直接 run 起来是 unconfigured，不会发任何话题，
   必须由 lifecycle_manager 推成 active。下面单独给它配了一个 manager。

3. static_layer 靠订阅 /map 取地图，所以顺手起了 map_server（参数文件里
   已经有 map_server 段）。如果别处已经在发 /map，用 map_server:=false 关掉。
"""
import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    params_file = LaunchConfiguration('params_file')
    use_map_server = LaunchConfiguration('map_server')
    use_tf = LaunchConfiguration('tf')

    declare_params = DeclareLaunchArgument(
        'params_file',
        default_value='/home/hqtebk08/Code/tjurm2027-ros/launch.yaml',
        description='参数文件（YAML）')

    declare_map_server = DeclareLaunchArgument(
        'map_server',
        default_value='true',
        description='是否顺带启动 map_server 提供 /map')

    declare_tf = DeclareLaunchArgument(
        'tf',
        default_value='true',
        description='是否补一条 map->base_link 静态变换（真机上有 TF 就别开）')

    # ---- 1. costmap 节点本身：nav2_costmap_2d 包里的同名程序 ----
    costmap = Node(
        package='nav2_costmap_2d',
        executable='nav2_costmap_2d',
        output='screen',
        parameters=[params_file],
        # 程序内部会把节点放进同名命名空间，这里拉回根，保证全名是 /global_costmap
        arguments=['--ros-args', '-r', '__ns:=/'],
    )

    # ---- 2. 生命周期管理：把它推到 active ----
    lifecycle_manager_costmap = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_costmap',
        output='screen',
        parameters=[{
            'autostart': True,
            'node_names': ['costmap/costmap'],
            'bond_timeout': 0.0,
        }],
    )

    # ---- 3. 地图：static_layer 需要 /map ----
    map_server = Node(
        package='nav2_map_server',
        executable='map_server',
        name='map_server',
        output='screen',
        parameters=[params_file],
        condition=IfCondition(use_map_server),
    )

    lifecycle_manager_map = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_map',
        output='screen',
        parameters=[{
            'autostart': True,
            'node_names': ['map_server'],
            'bond_timeout': 0.0,
        }],
        condition=IfCondition(use_map_server),
    )

    # ---- 4. 没有真实机器人时补一条 TF ----
    static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        output='screen',
        arguments=['--x', '0', '--y', '0', '--z', '0',
                   '--roll', '0', '--pitch', '0', '--yaw', '0',
                   '--frame-id', 'map', '--child-frame-id', 'base_link'],
        condition=IfCondition(use_tf),
    )

    return LaunchDescription([
        declare_params,
        declare_map_server,
        declare_tf,
        costmap,
        lifecycle_manager_costmap,
        map_server,
        lifecycle_manager_map,
        static_tf,
    ])
