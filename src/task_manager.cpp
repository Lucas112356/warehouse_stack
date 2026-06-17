#include <chrono>
#include <memory>
#include <string>
#include <sstream>  
#include <iomanip>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class TaskManager : public rclcpp::Node 
{
public:
    TaskManager() : 
    Node("task_manager_node"), current_task_index(0) 
    {
        // List of tasks done in the warehouse
        task_list = {
            "Package Pick Up", 
            "Box", 
            "Package Popcorn", 
            "Wrapping", 
            "Sealing", 
            "Delivery"
        };

        // Subscribe to the topics "status_update" and "swap_task" from status_manager.cpp and task_execution.cpp
        status_subscription = this->create_subscription<std_msgs::msg::String>(
            "status_update", 10, std::bind(&TaskManager::topic_callback, this, std::placeholders::_1));
        swap_subscription = this->create_subscription<std_msgs::msg::String>(
            "swap_task", 10, std::bind(&TaskManager::swap_callback, this, std::placeholders::_1));
        
        // Publish the current task to the topic "current_task" to task_execution.cpp and data_logger.cpp
        task_publisher = this->create_publisher<std_msgs::msg::String>("current_task", 10);
    }

private:
   void topic_callback(const std_msgs::msg::String::SharedPtr msg) const
{
    auto message = std_msgs::msg::String();
    // When the status is "Active", publish the current task
    if (msg->data == "Active")
    {
        message.data = task_list[current_task_index];
        RCLCPP_INFO(this->get_logger(), "Current Task Set: '%s'", message.data.c_str());
    }

    // When the status is "Closing", publish "Closing"
    else if (msg->data == "Closing")
    {
        message.data = "Origin";
    }

    // When status is "Charge", publish "Charging"
    else if (msg->data == "Charge")
    {
        message.data = "Charging";
    }

    // When the status is invalid, publish "Unknown"
    else
    {
        message.data = "Unknown";
    }

    // Publish the current task to the topic "current_task"
    task_publisher->publish(message);
}

    void swap_callback(const std_msgs::msg::String::SharedPtr msg) 
{
    // When the message is "swap", advance the task index
    if (msg->data == "swap") 
    {
        current_task_index = (current_task_index + 1) % task_list.size();
    }

    else if (msg->data == "finished")
    {
        auto message = std_msgs::msg::String();
        message.data = "finish";
        // Publish the finish message to the topic "current_task"
        task_publisher->publish(message);
    }

    else
    {
        // Ignore unknown messages
    }
}

    std::vector<std::string> task_list;
    size_t current_task_index;

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr status_subscription;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr swap_subscription;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr task_publisher;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TaskManager>()); 
  rclcpp::shutdown();
  return 0;
}