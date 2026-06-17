#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class Location : public rclcpp::Node 
{
public:
    Location() : Node("location_node") 
    {
        location_list = {0.0, 0.0}; 
        target_location = {0.0, 0.0};

         // Subscriber to listen to actions from task_execution.cpp
        task_subscription = this->create_subscription<std_msgs::msg::String>(
            "task_coordinates", 10, std::bind(&Location::topic_callback, this, std::placeholders::_1));

        // Publishes to the topic "updated_location" for task_execution.cpp
        location_publisher = this->create_publisher<std_msgs::msg::String>("updated_location", 10);
    }

private:
    void topic_callback(const std_msgs::msg::String::SharedPtr msg) 
    {
        std::sscanf(msg->data.c_str(), "(%lf, %lf)", &target_location[0], &target_location[1]);

        // Calculate the distance to the target
        double dx = target_location[0] - location_list[0];
        double dy = target_location[1] - location_list[1];
        double distance = std::sqrt(dx * dx + dy * dy);

        // Only move if we are not already at the target
        if (distance > 0.001) 
        { 
            // Moves vector distance of 1.5 units/s
            double step = 0.375;

            if (distance <= step) 
            {
                // If the step is larger than the distance, just snap to target location
                location_list[0] = target_location[0];
                location_list[1] = target_location[1];
            }

            else {
                // Otherwise, move a full step of 0.375
                double angle = std::atan2(dy, dx);
                location_list[0] += std::cos(angle) * step;
                location_list[1] += std::sin(angle) * step;
            }
        }

        auto message = std_msgs::msg::String();
        message.data = "Location: " + std::to_string(location_list[0]) + "," + std::to_string(location_list[1]);
        RCLCPP_INFO(this->get_logger(), "Publishing Real Time Location: '%s'", message.data.c_str());
        // Publishes location to "updated_location"
        location_publisher->publish(message);
    }

    std::vector<double> location_list;
    std::vector<double> target_location;

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr task_subscription;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr location_publisher;
    rclcpp::TimerBase::SharedPtr timer;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Location>()); 
    rclcpp::shutdown();
    return 0;
}