#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <algorithm> // Required for std::find

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class StatusManager : public rclcpp::Node
{
public:
  StatusManager(): 
    Node("status_manager_node"), current_index(0)
  {
    status_list = {"Active", "Closing", "Charging", "Idling"};
  
    // Subscribers for command on the "user_command" topic
    subscription = this->create_subscription<std_msgs::msg::String>(
      "user_command", 10, std::bind(&StatusManager::command_callback, this, std::placeholders::_1));

    // Publishes the current status to the "status_update" topic for task_manager.cpp at 250ms intervals
    status_publisher = this->create_publisher<std_msgs::msg::String>("status_update", 10);  
    timer = this->create_wall_timer(250ms, std::bind(&StatusManager::timer_callback, this));
  }

private:
  // Triggered when a message is received on "user_command"
  void command_callback(const std_msgs::msg::String::SharedPtr msg)
  {
    // Allowed manual commands
    const std::vector<std::string> allowed_commands = {"Closing"};

    // Check if the incoming command is in the allowed list
    auto it_allowed = std::find(allowed_commands.begin(), allowed_commands.end(), msg->data);
    
    if (it_allowed != allowed_commands.end()) 
    {
        // If allowed, find where that status exists in your main status_list
        auto it_status = std::find(status_list.begin(), status_list.end(), msg->data);
        
        if (it_status != status_list.end()) 
        {
            current_index = std::distance(status_list.begin(), it_status);
            RCLCPP_INFO(this->get_logger(), "Manual override: Switching status to '%s'", msg->data.c_str());
        }
    } 
    else 
    {
        // Reject invalid commands
        RCLCPP_WARN(this->get_logger(), "Command '%s' is not allowed for manual override. Valid options is: Closing", msg->data.c_str());
    }
  }

  void timer_callback()
  {
    auto message = std_msgs::msg::String();
    message.data = status_list[current_index];
    RCLCPP_INFO(this->get_logger(), "Current status: %s", message.data.c_str());
    // Publishes the current status
    status_publisher->publish(message);
  }

  rclcpp::TimerBase::SharedPtr timer;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_publisher;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription;
  
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