# Scheduler A Project

## Project Description

- This project involves the design and implementation of a scheduler simulation in C. The program supports three scheduling algorithms:

  - First-Come-First-Served (FCFS)

  - External Priorities (EP) - Non-preemptive

  - Round Robin (RR) - Time Quantum: 100 ms

- The simulation manages process control blocks (PCBs), memory allocation, and state transitions. It also generates reports detailing the memory and process states during execution.

## Features

- Scheduling Algorithms:

  - FCFS: Executes processes in the order of arrival.

  - EP: Assigns priorities externally; processes with higher priority (e.g., shortest CPU burst) execute first.

  - RR: Cycles through processes with a time slice of 100 ms.

## Memory Management

- Fixed partitioning scheme with predefined sizes.

- Tracks memory usage and state changes.

## Simulation Outputs

- State changes of processes.

- Memory usage at each event.

- Metrics: throughput, average turnaround time, and wait time.

## Bonus Feature:

 - Memory usage analysis.

## Project Structure

- Files:

  - interrupts_101261583_101273477.c: Main program file.

  - interrupts_101261583_101273477.h: Data Structures and fixed paritions.
 
  - input_data_1.txt: The input data with the PCB information.
 
  - test_1.txt: The file to run the program in the terminal. 

  - execution_101261583_101273477.txt: Output file for state changes.

  - memory_status_101261583_101273477.txt: Output file for memory usage.
 
  - Part1_report_101261583_101273477.pdf: Report analysis on memory usage, scheduling algorith analysis, etc. 

## Data Structures

- PCB (Process Control Block):

- Stores process-specific details (e.g., PID, arrival time, CPU burst, priority).

## Memory Partitions

- Fixed partitions initialized with sizes and availability.

## Tables

- State Change Table: Logs state transitions.

- Memory Status Table: Tracks memory allocation and usage.

- Usage Instructions

## Compilation

 - gcc scheduler_sim.c -o scheduler_sim

## Execution

 - ./scheduler_sim <input_file> <algorithm_type>

  - <input_file>: Path to the input file containing process details.

  - <algorithm_type>: Scheduling algorithm (e.g., FCFS, EP, RR).

## Example

 - ./scheduler_sim input_data_1.txt FCFS

## Running with the Script

- sh test_1.sh

- Automates the execution of different test scenarios.

## Input Format

- Each line in the input file represents a process:

  - <arrival_time> <cpu_burst> <io_frequency> <io_duration>

- Example:

  - 0 20 5 3
  - 2 15 4 2

## Metrics and Analysis

- The simulation outputs metrics to compare the performance of scheduling algorithms:

  - Throughput: Number of processes completed per unit time.

  - Average Turnaround Time: Total time taken for process completion.

  - Wait Time: Time spent in the ready queue.

## Memory Usage Report

- The memory status table provides details:

  - Memory used and free at each event.

  - Partition states.

## Future Enhancements

- Add preemptive scheduling algorithms (e.g., SRTF, Priority with Preemption).

## Acknowledgments

- This project was developed as part of the SYSC 4001 coursework. Special thanks to the course instructor for guidance.

 
