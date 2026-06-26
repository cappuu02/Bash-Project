# Dining Philosophers Simulation (Concurrency & IPC)

A comprehensive educational implementation of the classic **Dining Philosophers Problem** designed to demonstrate core operating systems concepts, including process synchronization, deadlock conditions, starvation detection, and Inter-Process Communication (IPC) under Linux.

This project was developed as a final laboratory project for the **University of Perugia (UNIPG)**.

---

## 🛠️ System Architecture & IPC Features

The application simulates $N$ philosophers sitting at a round table, alternating between thinking and eating. To achieve robust concurrent execution and precise condition modeling, it leverages:

- **Multi-processing (`fork`)**: The main parent process manages initialization and spawns independent child processes for each philosopher.
- **POSIX Named Semaphores (`sem_open`/`sem_timedwait`)**: Chopsticks are represented as binary semaphores initialized to `1`, mapped dynamically via the virtual file system (`/dev/shm`).
- **UNIX Pipes (`pipe`)**: A shared IPC channel used as a synchronization mechanism to atomically inspect and modify a global deadlock-tracking counter across isolated process spaces.
- **Advanced Signal Handling (`sigaction`)**: Gracious cleanup of IPC primitives and active semaphores upon receiving termination signals (`SIGINT` / `Ctrl+C`).

---

## 🕹️ Execution Modes & Flags

The behavior of the simulation is entirely deterministic and modular, controlled via command-line arguments to simulate specific concurrency anomalies and their architectural fixes.

### 1. Baseline Run (All flags disabled)
Philosophers independently attempt to lock the right chopstick and then the left chopstick using standard blocking calls. 

### 2. Deadlock Generation (`flag_stallo = 1`)
Forces a circular-wait condition by introducing an artificial delay (`sleep(1)`) immediately after a philosopher successfully picks up their right chopstick. This guarantees that every philosopher holds one resource while blocking indefinitely on the next. 

### 3. Deadlock Detection Routine
When the deadlock flag is enabled, a monitoring routine reads/writes via the pipe to increment a central counter. If the counter reaches the total number of philosophers, a deadlock is explicitly detected, an error log is printed, and a `SIGINT` broadcast (`kill(0, SIGINT)`) tears down the frozen processes cleanly.

### 4. Asymmetric Deadlock Resolution (`flag_soluzione = 1`)
Implements Dijkstra's classic resource-ordering solution by breaking the symmetry of the table. The last philosopher (`i == num_filosofi - 1`) is programmatically modified to request chopsticks in reverse order (left then right). This breaks the circular wait loop, making deadlocks mathematically impossible.

### 5. Starvation Handling (`flag_starvation = 1`)
Replaces indefinite blocking hooks with `sem_timedwait`, bounded to a strict 3-second absolute timeout. If a philosopher is blocked from eating for too long due to neighbor resource hogging, the system throws an `ETIMEDOUT` error, logs a starvation alert, and safely triggers an execution exit.

---

## 🚀 Getting Started & Automation

### Prerequisites
- Linux Environment (Ubuntu, Debian, or any POSIX-compliant OS)
- GCC Compiler

### Fast Track: Interactive Automation Script
An interactive Bash script is provided to automate the compilation and execution phases without manually typing complex positional arguments.

1. Make the script executable:
   ```bash
   chmod +x run_project.sh
