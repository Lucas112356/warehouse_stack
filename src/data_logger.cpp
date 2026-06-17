#include <memory>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using std::placeholders::_1;

class DataLogger : public rclcpp::Node
{
public:
  DataLogger() : Node("data_logger_node")
  {
    // Initialize the task list and completion counts
    task_list = {
      "Package Pick Up", 
      "Box", 
      "Package Popcorn", 
      "Wrapping", 
      "Sealing", 
      "Delivery"};
    task_completions = {0, 0, 0, 0, 0, 0};
    current_task_name = "None"; 
    //  Real timer
    start_time = this->now();

    // Subscribes to the topic "current_task" from task_manager.cpp
    task_subscription = this->create_subscription<std_msgs::msg::String>(
        "current_task", 10, std::bind(&DataLogger::topic_callback, this, _1));
    // Subscribes to the topic "battery_update" from status_manager.cpp
    battery_subscription = this->create_subscription<std_msgs::msg::String>(
        "battery_update", 10, std::bind(&DataLogger::battery_callback, this, _1));
  }

private:
  void topic_callback(const std_msgs::msg::String::SharedPtr msg) 
  {
    // Counts the number of times the node has received a message
    packet_number++; 

    RCLCPP_INFO(this->get_logger(), "Warehouse timer: %.2f seconds", packet_number / 4.0);
    // Calculates total tasks completed and completed rounds
    int total_tasks_sum = std::accumulate(task_completions.begin(), task_completions.end(), 0);
    int completed_rounds = total_tasks_sum / task_list.size(); // 6

    RCLCPP_INFO(this->get_logger(), "Completed rounds: %d", completed_rounds);

    auto it_old = std::find(task_list.begin(), task_list.end(), current_task_name);
    auto it_new = std::find(task_list.begin(), task_list.end(), msg->data);
    
    // Check if the task has changed
    if (it_old != task_list.end() && it_new != task_list.end() && current_task_name != msg->data) 
    {
        int index = std::distance(task_list.begin(), it_old);
        task_completions[index]++;
    }
    current_task_name = msg->data;
    
    // Calculate the elapsed time
    auto now = this->now();
    auto elapsed = now - start_time;

    // If told to finish, write JSON file report
    if(msg->data == "finish") 
    { 
      auto system_now = std::chrono::system_clock::now();
      auto time = std::chrono::system_clock::to_time_t(system_now);

      std::stringstream ss;

      ss << std::put_time(
        std::localtime(&time),
        "%Y-%m-%d_%H-%M-%S"
      );

      // Create a filename with the current timestamp
      std::string filename =
        "src/warehouse_stack/src/performance_logs/mission_report_v1.2_" +
        ss.str() +
        ".json";

      std::ofstream file(filename);
      // Write the JSON file
      file << "{\n";
      file << "  \"report_timestamp\": \"" << ss.str() << "\",\n";

      double total_seconds = elapsed.seconds();
      // Format total time elapsed
      int minutes = static_cast<int>(total_seconds) / 60;
      double seconds = std::fmod(total_seconds, 60.0);
      // Calculate frequency of average publishing in Hz
      double average_frequency = packet_number / total_seconds; 

      file << "  \"mission_duration\": \""<< minutes << "m "<< std::fixed << std::setprecision(2)<< seconds << "s\",\n";
      file << "  \"target_publishing_rate\": \"" << "4.00 Hz" << "\",\n";
      file << "  \"average_publishing_rate\": \"" << average_frequency << " Hz\",\n";
      
      // Task completion
      file << "  \"total_tasks_completed\": " << total_tasks_sum << ",\n";
      file << "  \"full_rounds_completed\": " << completed_rounds << ",\n";
      file << "  \"tasks_completed\": {\n";

      for(size_t i = 0; i < task_list.size(); ++i)
      {
        file << "    \"" << task_list[i] << "\": "
             << task_completions[i];
        if(i < task_list.size() - 1)
        {
            file << ",";
        }
        file << "\n";
      }
      file << "  },\n";
      
      // Battery report
      file << "  \"battery_report\": {\n";
      file << "    \"initial_battery_level\": \"100.00%\",\n";
      file << "    \"final_battery_level\": \"" << final_battery_level << "%\",\n";
      file << "    \"lowest_battery_level\": \"" << lowest_battery_level << "%\",\n";
      file << "    \"charging_cycles\": " << charging_cycle << "\n";
      file << "  }\n";
      file << "}\n";
      file.close();
      report_saved = true;
    }

    if(report_saved) 
    {
      rclcpp::shutdown();   
    }
  }

  void battery_callback(const std_msgs::msg::String::SharedPtr msg)
  {
    // Parse the battery update message
    std::string label;
    double f, l;
    int c;

    std::stringstream ss(msg->data);
    ss >> label >> f >> label >> l >> label >> c;

    final_battery_level = f;
    lowest_battery_level = l;
    charging_cycle = c;
  }

  bool report_saved = false;
  double packet_number = 0.0;
  double final_battery_level;
  double lowest_battery_level;
  int charging_cycle;

  std::string current_task_name;
  std::vector<std::string> task_list;
  std::vector<int> task_completions;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr task_subscription;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr battery_subscription;
  rclcpp::Time start_time;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DataLogger>());
  rclcpp::shutdown();
  return 0;
}