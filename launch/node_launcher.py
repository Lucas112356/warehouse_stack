from launch import LaunchDescription
from launch_ros.actions import Node
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    config = os.path.join(
        get_package_share_directory('warehouse_stack'),
        'config',
        'parameter.yaml'
    )

    return LaunchDescription([
        Node(
            package='warehouse_stack',
            executable='status_manager',
            parameters=[config]
        ),
        Node(
            package='warehouse_stack',
            executable='task_manager', 
            parameters=[config]
        ),
        Node(
            package='warehouse_stack',
            executable='task_execution',
            parameters=[config]
        ),
        Node(
            package='warehouse_stack',
            executable='location',
            parameters=[config]
        ),
        
        Node(
            package='warehouse_stack',
            executable='data_logger',
            parameters=[config]
        )
    ])