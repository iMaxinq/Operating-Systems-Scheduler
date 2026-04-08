# Operating Systems Scheduler

A small C-based operating systems project that simulates CPU scheduling by launching worker processes and managing them with Unix process control primitives such as `fork`, `execl`, `SIGSTOP`, `SIGCONT`, `SIGCHLD`, and `wait`. The repository includes a scheduler program plus a set of CPU-bound workload binaries used as test jobs.

## What this project does

This project implements a **basic process scheduler simulator** in C.

At a high level, it:

- reads a list of task/program names from an input file
- stores them in a custom queue implemented as a doubly linked list
- launches each task as a child process
- schedules those child processes using either:
  - **Round Robin (RR)** logic, or
  - **First Come, First Served (FCFS)** logic
- measures elapsed execution time for each task
- includes a second I/O-oriented workload and a placeholder for an extended scheduler with I/O support

The worker programs are artificial CPU loads that spend time doing repeated floating-point work so the scheduler has something observable to run.

## Repository structure

```text
Operating-Systems-Scheduler/
├── scheduler/
│   ├── Makefile
│   ├── scheduler.c
│   └── scheduler_io.c
├── work/
│   ├── Makefile
│   ├── work.c
│   └── work_io.c
└── report.pdf
