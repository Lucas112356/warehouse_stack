# ROS2 Warehouse Management Simulation

## Overview

A ROS2 (C++) warehouse automation simulation designed to model task scheduling, mission execution, location tracking, and operational reporting within a multi-node robotic system.

The project demonstrates distributed ROS2 node communication through a publisher-subscriber architecture while simulating warehouse workflows and generating automated mission performance reports.

## Features

- Multi-node ROS2 architecture
- Warehouse task scheduling and execution
- Continuous task queue cycling
- Real-time location tracking
- Robot status monitoring
- Automated JSON mission report generation
- Mission duration and timing analysis
- Performance logging and task completion metrics

## System Architecture

```text
Status Monitor Node
        ↓
Task Manager Node
        ↓
Task Execution Node
        ↓
Location Tracker Node
        ↓
Data Logger Node
```

The Task Manager coordinates warehouse operations while other nodes communicate task information, status updates, and mission data through ROS2 topics.

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
  "report_timestamp": "2026-06-10_16-30-56",
  "mission_duration": "4m 28.09s",
  "scheduled_runtime": "4m 6.75s",
  "latency_drift": "8.65%",
  "tasks_completed": {
    "Package Pick Up": 3,
    "Box": 3,
    "Package Popcorn": 3,
    "Wrapping": 3,
    "Sealing": 3,
    "Delivery": 3
  }
}
```

## Technologies

- ROS2
- C++
- CMake
- JSON
- Publisher-Subscriber Communication
- State Management
- Task Scheduling

## Future Development

- Return-to-base behavior (v1.1)
- Battery management system (v1.2)
- Mission interruption and recover (v1.3)

## Author

Lucas Kwan