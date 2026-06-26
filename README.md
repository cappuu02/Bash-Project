# Dining Philosophers — Operating Systems Project

> **Author:** Luca Capuccini — Student ID 347711 — University of Perugia  
> **Course:** Operating Systems, 2023

---

## Overview

This project simulates the classic **Dining Philosophers Problem** using Unix processes, POSIX semaphores, pipes, and signals. It was built as a final assignment to explore concurrency concepts in C and Bash.

The simulation lets you toggle three flags at runtime to observe different concurrency scenarios — normal execution, deadlock, deadlock solution, and starvation — without recompiling.

---

## The Dining Philosophers Problem

N philosophers sit around a circular table. Each one alternates between thinking and eating. Between every pair of adjacent philosophers there is one shared fork. To eat, a philosopher needs both the left and right fork.

This setup naturally produces three classic problems:

| Problem | Description |
|---------|-------------|
| **Deadlock** | Every philosopher grabs one fork and waits forever for the other — nobody proceeds. |
| **Starvation** | One or more philosophers never manage to acquire both forks. |
| **Race condition** | Two processes access a shared resource simultaneously without proper synchronisation. |

---

## Technologies Used

- **C** — process creation (`fork`), named POSIX semaphores, anonymous pipes, signal handling (`sigaction`, `kill`)
- **Bash** — interactive launcher that compiles the program and collects user input

---

## Project Structure

```
Bash-Project/
├── script.sh      # Bash launcher: compiles and prompts for parameters
└── capuccini.c    # Core simulation: processes, semaphores, pipes, signals
```

---

## How It Works

**`script.sh`** compiles the C file with GCC and interactively asks the user for four values, then launches the binary with those arguments.

**`capuccini.c`** spawns one child process per philosopher using `fork`. Each fork on the table is represented by a named POSIX semaphore (initialised to 1). Philosophers try to acquire their right and left semaphore before eating, then release both.

Depending on the flags passed:

- **Deadlock mode** adds a `sleep(1)` after grabbing the first fork to force the circular-wait condition. A shared counter in a pipe tracks how many philosophers are waiting; when all of them are blocked, deadlock is detected and `SIGINT` is broadcast to terminate the simulation.
- **Deadlock solution** applies the asymmetric-philosopher fix: the last philosopher picks up forks in reverse order, breaking the circular-wait cycle.
- **Starvation detection** replaces `sem_wait` with `sem_timedwait` (3-second timeout). If any philosopher times out waiting for a fork, starvation is reported and the simulation ends.

On termination (either normal or via `SIGINT`), all semaphores are closed and unlinked to free OS resources.

---

## How to Use

### Prerequisites

- GCC
- Linux or macOS with POSIX semaphore support

### Run with the script (recommended)

```bash
chmod +x script.sh
./script.sh
```

The script will compile the program and prompt you for:

1. Number of philosophers
2. Deadlock flag (`0` or `1`)
3. Deadlock solution flag (`0` or `1`)
4. Starvation flag (`0` or `1`)

### Run manually

```bash
gcc -o progetto capuccini.c -pthread
./progetto <num_philosophers> <deadlock> <solution> <starvation>
```

---

## Flag Reference

| Argument | Effect when set to `1` |
|---|---|
| `num_philosophers` | Number of philosopher processes to spawn (minimum 2) |
| `deadlock` | Induces deadlock via artificial delay; detects and reports it |
| `solution` | Applies fork-order reversal for the last philosopher to prevent deadlock |
| `starvation` | Enables timed semaphore waits; reports starvation on timeout |

### Example scenarios

| deadlock | solution | starvation | Result |
|:---:|:---:|:---:|---|
| 0 | 0 | 0 | Normal run — philosophers eat indefinitely |
| 1 | 0 | 0 | Deadlock is induced and detected |
| 1 | 1 | 0 | Deadlock scenario with the fix applied — no deadlock occurs |
| 0 | 1 | 1 | Safe run with starvation detection active |
| 1 | 1 | 1 | All safeguards active simultaneously |
