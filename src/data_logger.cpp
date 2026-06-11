#include <memory>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>

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

    // Subscribes to the topic "current_task"
    subscription = this->create_subscription<std_msgs::msg::String>(
        "current_task", 10, std::bind(&DataLogger::topic_callback, this, _1));
  }

private:
  void topic_callback(const std_msgs::msg::String::SharedPtr msg) 
  {
    // Simulates theoretical time 
    code_time += 0.25; 

    // Updates task completion count
    if(current_task_name != msg->data) {
      auto it = std::find(task_list.begin(), task_list.end(), current_task_name);
      if (it != task_list.end()) {
          int index = std::distance(task_list.begin(), it);
          task_completions[index]++;
      }
      current_task_name = msg->data;
    }

    // Calculate the elapsed time
    auto now = this->now();
    auto elapsed = now - start_time;

    //Shows in terminal completion status report 
    RCLCPP_INFO(this->get_logger(), "--- Current Task Report ---");
    RCLCPP_INFO(this->get_logger(), "Total Mission Time: %.2f seconds", elapsed.seconds());
    for(size_t i = 0; i < task_list.size(); ++i) {
        RCLCPP_INFO(this->get_logger(), "%s: %d", task_list[i].c_str(), task_completions[i]);
    }

    // If delivery has been completed at least twice, write JSON file report
    if(task_completions[5] > 2 && !report_saved) { 
      auto now = std::chrono::system_clock::now();
      auto time = std::chrono::system_clock::to_time_t(now);

      std::stringstream ss;

      ss << std::put_time(
        std::localtime(&time),
        "%Y-%m-%d_%H-%M-%S"
      );
      std::string filename =
        "src/warehouse_stack/src/performance_logs/mission_report_" +
        ss.str() +
        ".json";

      std::ofstream file(filename);
      
      file << "{\n";
      file << "  \"report_timestamp\": \"" << ss.str() << "\",\n";

      double total_seconds = elapsed.seconds();
      int minutes = static_cast<int>(total_seconds) / 60;
      double seconds = std::fmod(total_seconds, 60.0);

      int code_minutes = static_cast<int>(code_time) / 60;
      double code_seconds = std::fmod(code_time, 60.0);

      double latency_drift_percent = (total_seconds - code_time) / code_time * 100;

      file << "  \"mission_duration\": \""<< minutes << "m "<< std::fixed << std::setprecision(2)<< seconds << "s\",\n";
      file << "  \"scheduled_runtime\": \""<< code_minutes << "m "<< std::fixed << std::setprecision(2)<< code_seconds << "s\",\n";
      file << "  \"latency_drift\": \""<< std::fixed << std::setprecision(2)<< latency_drift_percent << "%\",\n";
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
      file << "  }\n";
      file << "}\n";
      file.close();
      report_saved = true;
    }

    if(report_saved) 
    {
      rclcpp::shutdown();   
      RCLCPP_INFO(this->get_logger(), "Logging finished. Shutting down node.");
    }
  }

  bool report_saved = false;
  double code_time = 0.0;

  std::string current_task_name;
  std::vector<std::string> task_list;
  std::vector<int> task_completions;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription;
  rclcpp::Time start_time;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DataLogger>());
  rclcpp::shutdown();
  return 0;
} 

