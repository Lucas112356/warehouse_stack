#include <chrono>
#include <memory>
#include <string>
#include <sstream>  
#include <iomanip>
#include <algorithm>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class TaskExecution : public rclcpp::Node 
{
public:
    TaskExecution() :
    Node("task_execution_node") 
    {
        this->declare_parameter("task_names", std::vector<std::string>{});
        this->declare_parameter("waypoint_x", std::vector<double>{});
        this->declare_parameter("waypoint_y", std::vector<double>{});

        task_names = this->get_parameter("task_names").as_string_array();
        waypoint_x = this->get_parameter("waypoint_x").as_double_array();
        waypoint_y = this->get_parameter("waypoint_y").as_double_array();

        index = 0;
        progress = 0;
        at_task = false;
        
        // Subscribe to the topics "current_task" and "updated_location" from task_manager.cpp and location.cpp
        task_subscription = this->create_subscription<std_msgs::msg::String>(
            "current_task", 10, std::bind(&TaskExecution::topic_callback, this, std::placeholders::_1));
        location_subscription = this->create_subscription<std_msgs::msg::String>(
            "updated_location", 10, std::bind(&TaskExecution::location_callback, this, std::placeholders::_1));

        // Publish to "task_coordinates" and "swap_task" for location.cpp and task_manager.cpp
        action_publisher = this->create_publisher<std_msgs::msg::String>("task_coordinates", 10);
        swap_publisher = this->create_publisher<std_msgs::msg::String>("swap_task", 10);
    }

private:
   void topic_callback(const std_msgs::msg::String::SharedPtr msg) 
   {
       // Reset progress and state when interrupted or changing tasks
       if(task_now != msg->data) 
       {
           task_now = msg->data;
           swap_msg.data = "stay";
           
           // Wipe the old progress clean so it doesn't resume later
           progress = 0.0;
           at_task = false; 
       }

       double target_x = 0.0;
       double target_y = 0.0;
       
       // Move to origin
       if (msg->data == "Origin") {
           target_x = waypoint_x[0]; 
           target_y = waypoint_y[0];
        
       } 

       // Move to a specific task
       else 
       {
           auto it = std::find(task_names.begin(), task_names.end(), msg->data);
           if (it != task_names.end()) 
           {
               index = std::distance(task_names.begin(), it);
               target_x = waypoint_x[index];
               target_y = waypoint_y[index];
           }
       }
       
       auto message = std_msgs::msg::String();
       message.data = "(" + std::to_string(target_x) + ", " + std::to_string(target_y) + ")";
       action_publisher->publish(message);        
   }

    void location_callback(const std_msgs::msg::String::SharedPtr msg) 
    {
        double current_x, current_y;
        std::sscanf(msg->data.c_str(), "Location: %lf,%lf", &current_x, &current_y);
        
        double target_x = (task_now == "Origin") ? 0.0 : waypoint_x[index];
        double target_y = (task_now == "Origin") ? 0.0 : waypoint_y[index];

        double dist = std::sqrt(std::pow(current_x - target_x, 2) + 
                                std::pow(current_y - target_y, 2));

        // If the robot is less distance from its target than its full step, then pre-flag its at the task
        if (dist < 0.375 && dist > 0.01){
            at_task = true;
        }

        // Complete the task when the robot is at the location and isn't in closing mode
        if (dist < 0.01 && at_task && task_now != "Origin" && task_now != "Charging") {
            if(progress < 100.0) 
            {
                progress+=2.5;
                RCLCPP_INFO(this->get_logger(), "Completing task: \"%s\" (%.2f%%)", 
                    task_names[index].c_str(), progress);
            }
            // Signal to move ot the next task
            else 
            {
                RCLCPP_INFO(this->get_logger(), "%s completed at 100%%!", task_names[index].c_str());
                progress = 0.0; 
                at_task = false;
                swap_msg.data = "swap";
            }
        }
        
        // Signal to finish logging then shut down
        if (task_now == "Origin" && at_task) 
        {
            swap_msg.data = "finished";
        }
        
        // Publish to the topic "swap_task"
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