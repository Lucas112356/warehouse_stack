# ROS2 Warehouse Management Simulation

## Overview
A ROS2 (C++) warehouse automation simulation designed to model task scheduling, mission execution, location tracking, and operational reporting within a multi-node robotic system.

The project demonstrates distributed ROS2 node communication through a publisher-subscriber architecture while simulating warehouse workflows and generating automated mission performance reports.

## Latest Update: v1.1
- **Closing Status:** Added a dedicated operational state for system shutdown.
- **Return-to-Home:** Robot now returns to origin before finalizing during closing status.
- **JSON Reporting:** Mission reports are now triggered automatically upon returning to origin instead of task completion basis.
- **Proper Termination:** All nodes now gracefully shutdown after the report is generated.

## Features
- Multi-node ROS2 architecture
- Warehouse task scheduling and execution
- Continuous task queue cycling
- Real-time location tracking
- Robot status monitoring
- Automated JSON mission report generation
- Mission duration and timing analysis
- Performance logging and task completion metrics
- Automated mission finalization and graceful node termination

## System Architecture
```text
                  User Input
                      ↓
                Status Monitor
                      ↓
                 Task Manager
                ↙          ↘
               ↓             ↕
       Data Logger     Task Execution
                             ↕
                      Location Tracker
```


## Node Responsibilities
| Node                 | Responsibility                                                                                                                    |
| -------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| **Status Monitor**   | Receives operator commands and manages the robot's current operational state. |
| **Task Manager**     | Central coordinator responsible for task scheduling, mission progression and workflow decisions.                     |
| **Task Execution**   | Executes warehouse tasks based on current state and reports task completion status.                                         |
| **Location Tracker** | Maintains the robot's current position and provides navigation feedback.                                        |
| **Data Logger**      | Records mission metrics, task completion statistics, recording frequency and generates JSON mission reports.                      |

## Warehouse Layout
```text
Y
^
10|  . . . . . . . . . . .
9 |  . . . . . . . . . . .
8 |  . W . . . S . . . . .
7 |  . . . . . . . . . . .
6 |  . . . . . . . . . . .
5 |  . . . . . . . . . . D
4 |  . . . . . . . . . . .
3 |  . . . . . . . . . . .
2 |  . . . . . . . . . P .
1 |  . . . U . . . B . . .
0 |  O . . . . . . . . . .
  +------------------------> X
     0 1 2 3 4 5 6 7 8 9 10
```

### Legend
| Symbol | Location | Coordinates |
|----------|----------|----------|
| O | Origin / Home Base | (0,0) |
| U | Package Pick Up | (3,1) |
| B | Box Station | (7,1) |
| P | Package Popcorn | (9,2) |
| W | Wrapping Station | (1,8) |
| S | Sealing Station | (5,8) |
| D | Delivery Station | (10,5) |

## Example Mission Report
```json
{
  "report_timestamp": "2026-06-12_10-26-29",
  "mission_duration": "3m 2.15s",
  "target_publishing_rate": "4.00 Hz",
  "average_publishing_rate": "3.78 Hz",
  "tasks_completed": {
    "Package Pick Up": 2,
    "Box": 2,
    "Package Popcorn": 2,
    "Wrapping": 2,
    "Sealing": 2,
    "Delivery": 2
  }
}
```
## Verification Results
| Condition | Result |
| :--- | :--- |
| Target Publishing Rate | 4.00 Hz |
| Observed Publishing Rate | 3.65 - 4.00 Hz |
| Longest Validation Run | 4 mission cycles |
| Maximum Tasks Completed | 24 consecutive tasks |
| Graceful Shutdown Verrified | Passed |

## Technologies
- ROS2
- C++
- CMake
- JSON
- Publisher-Subscriber Communication
- State Management
- Task Scheduling

## Version History
- **v1.0:** Initial release featuring task looping + basic JSON reporting.
- **v1.1:** Closing status, return-to-home functionality and proper node termination.

## Roadmap
- **v1.2:** Battery management + charging station
- **v1.3:** Mission interruption + recovery
- **v2.0:** Fleet communication

## Author
Lucas Kwan