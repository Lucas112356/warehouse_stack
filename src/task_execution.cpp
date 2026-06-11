#include <chrono>
#include <memory>
#include <string>
#include <sstream>  
#include <iomanip>
#include <algorithm>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class TaskExecution : public rclcpp::Node {
public:
    TaskExecution() :
    Node("task_execution_node") {
        // Pulls variable information from parameter.yaml for task information and waypoint coordinates
        this->declare_parameter("task_names", std::vector<std::string>{});
        this->declare_parameter("waypoint_x", std::vector<double>{});
        this->declare_parameter("waypoint_y", std::vector<double>{});

        // Assigns variables to the values from the parameters
        task_names = this->get_parameter("task_names").as_string_array();
        waypoint_x = this->get_parameter("waypoint_x").as_double_array();
        waypoint_y = this->get_parameter("waypoint_y").as_double_array();

        index = 0;
        progress = 0;
        at_task = false;
        
        // Subscribe to the topic "current_task" and "updated_location"
        task_subscription = this->create_subscription<std_msgs::msg::String>(
            "current_task", 10, std::bind(&TaskExecution::topic_callback, this, std::placeholders::_1));
        location_subscription = this->create_subscription<std_msgs::msg::String>(
            "updated_location", 10, std::bind(&TaskExecution::location_callback, this, std::placeholders::_1));

        // Publishes to the topic "task_coordinates" and "swap_task"
        action_publisher = this->create_publisher<std_msgs::msg::String>("task_coordinates", 10);
        swap_publisher = this->create_publisher<std_msgs::msg::String>("swap_task", 10);
    }

private:
   void topic_callback(const std_msgs::msg::String::SharedPtr msg) 
{
    // When the current task changes, update the task name and send a "stay" message to the "swap_task" topic
    if(task_now != msg->data) {
        task_now = msg->data;
        swap_msg.data = "stay";
    }

    auto it = std::find(task_names.begin(), task_names.end(), msg->data);
    index = std::distance(task_names.begin(), it);

    // Retrieve coordinates of the task
    double target_x = waypoint_x[index];
    double target_y = waypoint_y[index];
    
    auto message = std_msgs::msg::String();
    message.data = "(" + std::to_string(target_x) + ", " + std::to_string(target_y) + ")";
    action_publisher->publish(message);        
}

    void location_callback(const std_msgs::msg::String::SharedPtr msg) 
{
        double current_x, current_y;
        // Parse "Location: x,y"
        std::sscanf(msg->data.c_str(), "Location: %lf,%lf", &current_x, &current_y);
        double dist = std::sqrt(std::pow(current_x - waypoint_x[index], 2) + 
                                std::pow(current_y - waypoint_y[index], 2));

        // If the robot is close to the target, mark it as being at the task
        if (dist < 0.375 && dist > 0.01){
            at_task = true;
        }

        // When the robot is at the task, start completing the task
        if (dist < 0.01 && at_task) {
            if(progress < 100.0) {
                progress+=2.5;
                RCLCPP_INFO(this->get_logger(), "Completing task: \"%s\" (%.2f%%)", 
                    task_names[index].c_str(), progress);
            }
            else {
                // Mark the task as completed
                RCLCPP_INFO(this->get_logger(), "%s completed at 100%%!", task_names[index].c_str());
                // Reset for next task
                progress = 0.0; 
                at_task = false;
                // Send a "swap" message to the "swap_task" topic
                swap_msg.data = "swap";
            }
        }
        swap_publisher->publish(swap_msg);
}

    int index;
    double progress;
    bool at_task;
    bool swap_task; 

    std::vector<std::string> task_names;
    std::vector<double> waypoint_x;
    std::vector<double> waypoint_y;
    std::string task_now;
    std_msgs::msg::String swap_msg;    

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr task_subscription;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr location_subscription;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr action_publisher;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr swap_publisher;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TaskExecution>());
  rclcpp::shutdown();
  return 0;
}