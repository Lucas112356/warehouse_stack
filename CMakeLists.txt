cmake_minimum_required(VERSION 3.8)
project(warehouse_stack)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# executables
add_executable(task_manager src/task_manager.cpp)
ament_target_dependencies(task_manager rclcpp std_msgs)

add_executable(status_manager src/status_manager.cpp)
ament_target_dependencies(status_manager rclcpp std_msgs)

add_executable(task_execution src/task_execution.cpp)
ament_target_dependencies(task_execution rclcpp std_msgs)

add_executable(location src/location.cpp)
ament_target_dependencies(location rclcpp std_msgs)

add_executable(data_logger src/data_logger.cpp)
ament_target_dependencies(data_logger rclcpp std_msgs)

# install executables
install(TARGETS
  task_manager
  status_manager
  task_execution
  location
  data_logger
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}
)
ament_package()
