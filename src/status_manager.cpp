#include <chrono>
#include <memory>
#include <string>
#include <vector> 

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class StatusManager : public rclcpp::Node
{
public:
  StatusManager(): 
    Node("status_manager_node"), current_index(0)
  {
    // Status of robot is Active (Closing, Charging, and Idling are future states)
    status_list = {"Active", "Closing", "Charging", "Idling"};

    // Publishes to the topic "status_update" the current status of the robot
    status_publisher = this->create_publisher<std_msgs::msg::String>("status_update", 10);
    timer = this->create_wall_timer(250ms, std::bind(&StatusManager::timer_callback, this));
  }

private:
  void timer_callback()
  {
    auto message = std_msgs::msg::String();
    
    // Access the current status using the index
    message.data = status_list[current_index];
    RCLCPP_INFO(this->get_logger(), "Robot Status: '%s'", message.data.c_str());
    // Publish status to the topic
    status_publisher->publish(message);
  }

  rclcpp::TimerBase::SharedPtr timer;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_publisher;
  
  std::vector<std::string> status_list;
  size_t current_index;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<StatusManager>());
  rclcpp::shutdown();
  return 0;
}