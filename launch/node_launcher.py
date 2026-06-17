from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import RegisterEventHandler, EmitEvent
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    config = os.path.join(
        get_package_share_directory('warehouse_stack'),
        'config',
        'parameter.yaml'
    )

    data_logger_node = Node(
        package='warehouse_stack',
        executable='data_logger',
        parameters=[config]
    )

    # Terminates all nodes when data_logger finishes
    sys_shutdown_handler = RegisterEventHandler(
        OnProcessExit(
            target_action=data_logger_node,
            on_exit=[
                EmitEvent(event=Shutdown(reason='Data Logger finished and requested system shutdown.'))
            ]
        )
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
        
        data_logger_node,
        sys_shutdown_handler
    ])