#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""只把地图发布到 /map，不启动任何规划器。

    ros2 launch my_nav2_astar map_only.launch.py
    ros2 launch my_nav2_astar map_only.launch.py yaml:=/abs/path/mymap.yaml
    ros2 launch my_nav2_astar map_only.launch.py frame_id:=map

原理：nav2_map_server 的 map_server 节点读 yaml（yaml 里的 image: 指向 pgm），
解析成 nav_msgs/OccupancyGrid，用 transient_local 的 QoS 发布到 /map。
map_server 是生命周期节点，光 ros2 run 起来它是 unconfigured 状态、不会发数据，
所以必须再起一个 lifecycle_manager 把它 configure + activate。

想确认发布成功：
    ros2 lifecycle get /map_server            # active [3]
    ros2 topic info /map -v                   # 1 publisher, TRANSIENT_LOCAL
    ros2 topic echo /map --once --field info
"""
import os

###from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    ###pkg_share = get_package_share_directory('my_nav2_astar')

    yaml_file = LaunchConfiguration('yaml')
    frame_id = LaunchConfiguration('frame_id')

    declare_yaml = DeclareLaunchArgument(
        'yaml',
        default_value='/home/hqtebk08/Code/tjurm2027-ros/map/mymap.yaml',
        description='地图 yaml 的绝对路径；同目录下要有 image: 指向的 .pgm')

    declare_frame = DeclareLaunchArgument(
        'frame_id',
        default_value='map',
        description='地图消息的坐标系（RViz 里 Fixed Frame 要填这个）')

    # 读 yaml + pgm，发布 /map
    map_server = Node(
        package='nav2_map_server',
        executable='map_server',
        name='map_server',
        output='screen',
        parameters=[{
            'yaml_filename': yaml_file,
            'frame_id': frame_id,
            'use_sim_time': False,
        }],
    )

    # 生命周期管理：把 map_server 从 unconfigured 推到 active
    lifecycle_manager = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_map',
        output='screen',
        parameters=[{
            'autostart': True,
            'node_names': ['map_server'],
            'bond_timeout': 0.0,   # 单节点场景关掉心跳检测，少一类报错
        }],
    )

    return LaunchDescription([
        declare_yaml,
        declare_frame,
        map_server,
        lifecycle_manager,
    ])
