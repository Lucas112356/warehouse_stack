#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <algorithm> 

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class StatusManager : public rclcpp::Node
{
public:
  StatusManager(): 
    Node("status_manager_node"), current_index(0) // Starts off as Active
  {
    status_list = {"Active", "Closing", "Charge", "Idling"};
  
    // Subscribers for command on the "user_command" and "updated_location" topic from location.cpp 
    subscription = this->create_subscription<std_msgs::msg::String>(
      "user_command", 10, std::bind(&StatusManager::command_callback, this, std::placeholders::_1));
    location_subscription = this->create_subscription<std_msgs::msg::String>(
            "updated_location", 10, std::bind(&StatusManager::location_callback, this, std::placeholders::_1));

    // Publishes the current status to the "status_update" topic for task_manager.cpp at 250ms intervals
    status_publisher = this->create_publisher<std_msgs::msg::String>("status_update", 10);  
    battery_publisher = this->create_publisher<std_msgs::msg::String>("battery_update", 10);
    timer = this->create_wall_timer(250ms, std::bind(&StatusManager::timer_callback, this));
    
    charging_cycle = 0; // Initialized with 0 cycles
    battery = 100.0; // Initialize starting battery level
    lowest_battery = battery; 
    this->declare_parameter("battery_low_threshold", 10.0);
    battery_low_threshold = this->get_parameter("battery_low_threshold").as_double();
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

  void location_callback(const std_msgs::msg::String::SharedPtr msg)
  {
    // Store the latest location or trigger logic based on it
    current_location = msg->data;
    double current_x, current_y;
    std::sscanf(msg->data.c_str(), "Location: %lf,%lf", &current_x, &current_y);

    // Swap to charging state if battery is low
    if (battery <= battery_low_threshold) 
    {
      current_index = 2; // Switch to Charge status
      RCLCPP_INFO(this->get_logger(), "Moving to charging station");
    }

    // Charging at station
    if(status_list[current_index] == "Charge" && current_x == 0.0 && current_y == 5.0)
    {
      battery += 1; // Charging 1% per 250ms
      if (battery >= 80.0) // Starts working again when battery is at 80%
      {
        charging_cycle++;
        current_index = 0; // Switch back to Active mode
        RCLCPP_INFO(this->get_logger(), "Charging complete, returning to normal operation");
      }
    }

    // Regular battery burn
    else
    {
      battery -= 0.1; // Battery drain rate at 0.x per 250ms 
    }

    // update lowest battery level
    if (battery < lowest_battery) {
      lowest_battery = battery;
    }
    auto message = std_msgs::msg::String();
    message.data = "final: " + std::to_string(battery) 
      + " lowest: " + std::to_string(lowest_battery) 
      + " cycles: " + std::to_string(charging_cycle);
    // Publish the battery data to the topic "battery_status"
    battery_publisher->publish(message);
    RCLCPP_INFO(this->get_logger(), "Battery level: %.2f%%", battery);
  }

  void timer_callback()
  {
    auto message = std_msgs::msg::String();
    message.data = status_list[current_index];

    RCLCPP_INFO(this->get_logger(), "--------------------------------------------------");
    RCLCPP_INFO(this->get_logger(), "Current status: %s", message.data.c_str());
   
    // Publishes the current status
    status_publisher->publish(message);
  }

  double battery;
  double battery_low_threshold;
  double lowest_battery;
  int charging_cycle;

  rclcpp::TimerBase::SharedPtr timer;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_publisher;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr battery_publisher;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr location_subscription;

  std::string current_location; 
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