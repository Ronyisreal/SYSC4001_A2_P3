# SYSC4001 Assignment 2 - Part 3: Fork/Exec API Simulator

**Course:** SYSC 4001 - Operating Systems  
**Assignment:** Part 3 - Design and Implementation of an API Simulator  
**Authors:** Timur Grigoryev & Rounak Mukherjee  
**Deadline:** November 7,2025

---

## Table of Contents

- [Overview](#overview)
- [Project Structure](#project-structure)
- [Implementation Details](#implementation-details)
- [How to Compile and Run](#how-to-compile-and-run)
- [Test Scenarios](#test-scenarios)
- [Output Files](#output-files)
- [System Design](#system-design)
- [Memory Management](#memory-management)
- [Known Issues](#known-issues)

---

## Overview

This project implements a simulator for the `fork()` and `exec()` system calls in an operating system. The simulator demonstrates:

- Process creation via fork
- Program loading via exec
- Memory partition management (fixed partitions)
- Process scheduling (child priority)
- Interrupt handling and mode switching

### Key Features

 **Fork System Call** - Creates child processes that inherit parent's PCB  
 **Exec System Call** - Loads external programs into memory  
 **Memory Management** - Best-fit allocation with 6 fixed partitions  
 **Process Hierarchy** - Child processes execute before parent resumes  
 **Execution Logging** - Detailed timing and event tracking  
 **System Status Snapshots** - PCB state captured after each fork/exec

---

##  Project Structure

SYSC4001_A2_P3/
│
├── README.md                                    # This file
├── report.pdf                                   # Analysis report (1-3 pages)
│
├── Interrupts_TimurGrigoryev_RounakMukherjee.cpp   # Main implementation
├── interrupts_TimurGrigoryev_RounakMukherjee.hpp   # Header file with data structures
├── build.sh                                        # Compilation script
│
├── vector_table.txt                             # ISR addresses (26 entries)
├── device_table.txt                             # Device delays (20 entries)
├── external_files.txt                           # Available programs
│
├── input_files/                                 # Test scenarios
│   ├── trace_scenario1.txt                      # Mandatory Test 1
│   ├── trace_scenario2.txt                      # Mandatory Test 2
│   ├── trace_scenario3.txt                      # Mandatory Test 3
│   ├── trace_scenario4.txt                      # Custom Test 1
│   ├── trace_scenario5.txt                      # Custom Test 2
│   ├── program1.txt                             # Program 1 execution trace
│   └── program2.txt                             # Program 2 execution trace
│
└── output_files/                                # Generated outputs
    ├── execution_scenario1.txt                  # Execution log - Test 1
    ├── system_status_scenario1.txt              # System snapshots - Test 1
    ├── execution_scenario2.txt                  # Execution log - Test 2
    ├── system_status_scenario2.txt              # System snapshots - Test 2
    ├── execution_scenario3.txt                  # Execution log - Test 3
    ├── system_status_scenario3.txt              # System snapshots - Test 3
    ├── execution_scenario4.txt                  # Execution log - Test 4
    ├── system_status_scenario4.txt              # System snapshots - Test 4
    ├── execution_scenario5.txt                  # Execution log - Test 5
    └── system_status_scenario5.txt              # System snapshots - Test 5

---

##  Implementation Details

### Data Structures

#### 1. Memory Partition Table
struct memory_partition_t {
    const unsigned int partition_number;  // 1-6
    const unsigned int size;              // MB
    std::string code;                     // "empty", "init", or program name
};

**Six Fixed Partitions:**
- Partition 1: 40 MB
- Partition 2: 25 MB
- Partition 3: 15 MB
- Partition 4: 10 MB
- Partition 5: 8 MB
- Partition 6: 2 MB (initially allocated to `init`)

#### 2. Process Control Block (PCB)
struct PCB {
    unsigned int PID;              // Process ID
    int PPID;                      // Parent Process ID
    std::string program_name;      // Program currently running
    unsigned int size;             // Memory size required
    int partition_number;          // Allocated partition
};

#### 3. External Files Table
struct external_file {
    std::string program_name;      // Program name
    unsigned int size;             // Size in MB
};

### Key Functions

#### Fork Implementation
// 1. Switch to kernel mode (1ms)
// 2. Save context (10ms)
// 3. Find vector 2 in interrupt vector table
// 4. Load ISR address
// 5. Clone parent PCB to create child (1-10ms random)
// 6. Call scheduler (0ms - empty)
// 7. Return from interrupt (IRET)
// 8. Execute child process (higher priority)
// 9. Take system status snapshot

#### Exec Implementation
// 1. Switch to kernel mode (1ms)
// 2. Save context (10ms)
// 3. Find vector 3 in interrupt vector table
// 4. Load ISR address
// 5. Get program size from external_files table (1-10ms random)
// 6. Find empty partition (1-10ms random)
// 7. Free old memory if needed
// 8. Allocate new partition using best-fit algorithm
// 9. Simulate loader: 15ms per MB (fixed)
// 10. Mark partition as occupied (1-10ms random)
// 11. Update PCB (1-10ms random)
// 12. Call scheduler (0ms - empty)
// 13. Return from interrupt (IRET)
// 14. Execute loaded program
// 15. Take system status snapshot

### Memory Management

**Algorithm:** Best-fit (searches from smallest to largest partition)

bool allocate_memory(PCB* current) {
    for(int i = 5; i >= 0; i--) { // Start from smallest
        if(memory[i].size >= current->size && memory[i].code == "empty") {
            current->partition_number = memory[i].partition_number;
            memory[i].code = current->program_name;
            return true;
        }
    }
    return false;
}

---

## How to Compile and Run

### Prerequisites
- C++ compiler (g++ or clang++)
- macOS, Linux, or Windows with WSL

### Compilation

# Make build script executable
chmod +x build.sh

# Compile
./build.sh

# Or manually:
mkdir -p bin
g++ -g -O0 -I . -o bin/interrupts Interrupts_TimurGrigoryev_RounakMukherjee.cpp

### Running Individual Test

./bin/interrupts input_files/trace_scenario1.txt vector_table.txt device_table.txt external_files.txt


# Output files generated:
- `execution.txt` - Execution log
- `system_status.txt` - System snapshots

### Running All Tests

# Run all 5 scenarios
for i in {1..5}; do
    echo "Running scenario $i..."
    ./bin/interrupts input_files/trace_scenario${i}.txt vector_table.txt device_table.txt external_files.txt
    mv execution.txt output_files/execution_scenario${i}.txt
    mv system_status.txt output_files/system_status_scenario${i}.txt
    echo "✓ Scenario $i complete"
done

# View results
ls -lh output_files/

---

## 🧪 Test Scenarios

### Scenario 1: Basic Fork/Exec (Mandatory)
**Trace:**
FORK,10
IF_CHILD,0
EXEC program1,50
IF_PARENT,0
EXEC program2,25
ENDIF,0

**What it tests:**
- Basic fork functionality
- Child executes program1 (10 MB)
- Parent executes program2 (15 MB)
- Memory allocation for both processes

**Expected behavior:**
1. Init (PID 0) forks, creating child (PID 1)
2. Child loads and runs program1 (CPU → SYSCALL → CPU → END_IO)
3. Parent loads and runs program2 (SYSCALL → CPU)

---

### Scenario 2: Fork with CPU Burst (Mandatory)
**Trace:**
FORK,17
IF_CHILD,0
EXEC program1,16
IF_PARENT,0
ENDIF,0
CPU,205

**What it tests:**
- Fork where child executes program
- Parent continues with CPU burst
- Both execute code after ENDIF

**Expected behavior:**
1. Init forks
2. Child loads and runs program1
3. Parent does CPU burst (205ms)

---

### Scenario 3: Parent-Only Exec (Mandatory)
**Trace:**
FORK,20
IF_CHILD,0
IF_PARENT,0
EXEC program1,60
ENDIF,0
CPU,10

**What it tests:**
- Fork where only parent executes EXEC
- Child skips to ENDIF
- Both execute CPU burst after ENDIF

**Expected behavior:**
1. Init forks
2. Child skips to ENDIF, does CPU burst
3. Parent loads and runs program1

---

### Scenario 4: Complex with Pre-Fork CPU (Custom)
**Trace:**
CPU,50
FORK,25
IF_CHILD,0
CPU,30
EXEC program1,40
IF_PARENT,0
CPU,20
EXEC program2,35
ENDIF,0

**What it tests:**
- CPU work before forking
- Both processes do CPU bursts before EXEC
- More realistic scenario with mixed operations

---

### Scenario 5: Nested Fork (Custom)
**Trace:**
FORK,15
IF_CHILD,0
CPU,100
SYSCALL,4
EXEC program2,20
IF_PARENT,0
FORK,12
IF_CHILD,0
EXEC program1,18
IF_PARENT,0
CPU,150
ENDIF,0
ENDIF,0

**What it tests:**
- First fork creates child1
- Child1 does CPU, SYSCALL, then executes program2
- Parent forks again (creates child2)
- Child2 executes program1
- Parent does CPU burst
- Complex process hierarchy

**Process tree:**
Init (PID 0)
├── Child1 (PID 1) → executes program2
└── Child2 (PID 2) → executes program1

---

## Output Files

### execution.txt Format
<time>, <duration>, <event_description>

**Example:**
0, 1, switch to kernel mode
1, 10, context saved
11, 1, find vector 2 in memory position 0x0004
12, 1, load address 0X0695 into the PC
13, 7, cloning the PCB
20, 0, scheduler called
21, 1, IRET

### system_status.txt Format
time: <timestamp>; current trace: <operation>
+-------------------------------------------------------+
| PID | program name | partition number | size | state |
+-------------------------------------------------------+
|   0 |         init |                6 |    1 | running |
|   1 |         init |                6 |    1 | waiting |
+-------------------------------------------------------+

---

## System Design

### Process Execution Flow

User Mode → System Call → Kernel Mode
    ↓           ↓              ↓
  FORK      Interrupt      ISR Execution
  EXEC      Handler        PCB Management
                          Memory Allocation
                               ↓
                          Return to User Mode
                               ↓
                          Execute Process

### Interrupt Handling

**Vectors:**
- Vector 2: FORK system call
- Vector 3: EXEC system call
- Vectors 4, 6, etc.: Device interrupts (SYSCALL, END_IO)

**Timing:**
- Mode switch: 1ms
- Context save: 10ms
- Vector lookup: 1ms
- ISR address load: 1ms
- Random operations: 1-10ms
- Loader: 15ms per MB (fixed)

### Process Priority

**Rule:** Child processes have higher priority than parent

**Implementation:**
1. When fork occurs, child is added to wait queue
2. Child's trace is extracted and executed recursively
3. Child runs to completion
4. Parent resumes only after child finishes
5. Memory is freed by child before parent continues

---

## Memory Management

### Allocation Strategy

**Best-Fit Algorithm:**
- Searches from smallest partition (6) to largest (1)
- Allocates smallest partition that fits the program
- Minimizes wasted space and fragmentation

### Example Allocation Sequence

**Initial State:**
Partition 6 (2 MB): init
All others: empty


**After child EXEC program1 (10 MB):**
Partition 4 (10 MB): program1
Partition 6 (2 MB): init


**After parent EXEC program2 (15 MB):**
Partition 3 (15 MB): program2
Partition 4 (10 MB): program1 (freed)
Partition 6 (2 MB): init (freed)

### Memory States in Partitions
- `"empty"` - Available for allocation
- `"init"` - Initial process
- `"program1"` or `"program2"` - Program name

---

## Known Issues

### Compilation Warnings
- `sprintf` deprecation warnings on macOS (harmless)
- Can be suppressed with `-Wno-deprecated-declarations`

### Limitations
1. **No Real Scheduler:** Empty scheduler function (0ms)
2. **No Preemption:** Processes run to completion
3. **Fixed Timing:** Some operations use random times, others fixed
4. **Simplified Memory:** No dynamic allocation or paging
5. **No Orphan Handling:** Assumes all children complete

---

## Report Analysis

The accompanying `report.pdf` includes:

1. **Implementation Overview**
   - How fork and exec are implemented
   - Data structures and algorithms used

2. **Test Results Analysis**
   - Detailed walkthrough of scenarios
   - Timing calculations and memory allocation patterns

3. **Critical Questions**
   - Why `break` after EXEC is necessary
   - How recursion simulates process hierarchy
   - Process scheduling and priority handling

4. **Conclusions**
   - Lessons learned about OS design
   - Challenges faced during implementation
   - Comparison with real operating systems

---

## Learning Outcome

This project demonstrates understanding of:

 **System Calls:** How user processes invoke kernel services  
 **Process Management:** Creation, execution, and termination  
 **Memory Management:** Partition allocation and best-fit algorithms  
 **Interrupt Handling:** Mode switching and ISR execution  
 **Process Hierarchy:** Parent-child relationships and priorities  
 **OS Simulation:** Modeling complex system behaviors

---

## Authors

**Timur Grigoryev** -  Implementation and Documentation  
**Rounak Mukherjee** - Implementation and Testing

**Course:** SYSC 4001 - Operating Systems  
**Institution:** Carleton University  
**Semester:** Fall 2025

---


