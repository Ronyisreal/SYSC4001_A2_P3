/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts_TimurGrigoryev_RounakMukherjee.hpp>

// Global PID counter
unsigned int next_pid = 1;

// Random number generator for execution times (1-10ms)
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1, 10);

int get_random_time() {
    return dis(gen);
}

std::tuple<std::string, std::string, int> simulate_trace(
    std::vector<std::string> trace_file, 
    int time, 
    std::vector<std::string> vectors, 
    std::vector<int> delays, 
    std::vector<external_file> external_files, 
    PCB current, 
    std::vector<PCB> wait_queue) {

    std::string trace;      //!< string to store single line of trace file
    std::string execution = "";  //!< string to accumulate the execution output
    std::string system_status = "";  //!< string to accumulate the system status output
    int current_time = time;

    //parse each line of the input trace file. 'for' loop to keep track of indices.
    for(size_t i = 0; i < trace_file.size(); i++) {
        auto trace = trace_file[i];

        auto [activity, duration_intr, program_name] = parse_trace(trace);

        if(activity == "CPU") { //As per Assignment 1
            execution += std::to_string(current_time) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
            current_time += duration_intr;
        } else if(activity == "SYSCALL") { //As per Assignment 1
            auto [intr, time] = intr_boilerplate(current_time, duration_intr, 10, vectors);
            execution += intr;
            current_time = time;

            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", SYSCALL ISR\n";
            current_time += delays[duration_intr];

            execution +=  std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
        } else if(activity == "END_IO") {
            auto [intr, time] = intr_boilerplate(current_time, duration_intr, 10, vectors);
            current_time = time;
            execution += intr;

            execution += std::to_string(current_time) + ", " + std::to_string(delays[duration_intr]) + ", ENDIO ISR\n";
            current_time += delays[duration_intr];

            execution +=  std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;
        } else if(activity == "FORK") {
            auto [intr, time] = intr_boilerplate(current_time, 2, 10, vectors);
            execution += intr;
            current_time = time;

            ///////////////////////////////////////////////////////////////////////////////////////////
            // FORK Implementation
            
            // Step 1: Clone the PCB
            int clone_time = get_random_time();
            execution += std::to_string(current_time) + ", " + std::to_string(clone_time) + ", cloning the PCB\n";
            current_time += clone_time;
            
            // Create child PCB
            PCB child(next_pid++, current.PID, current.program_name, current.size, current.partition_number);
            
            // Step 2: Call scheduler (empty for now)
            int sched_time = 0; // Assume 0 time for scheduling
            execution += std::to_string(current_time) + ", " + std::to_string(sched_time) + ", scheduler called\n";
            current_time += sched_time;
            
            // Step 3: IRET
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;

            // Add child to wait queue
            wait_queue.push_back(child);

            // Create system status snapshot
            system_status += "time: " + std::to_string(current_time) + "; current trace: FORK, " + std::to_string(duration_intr) + "\n";
            system_status += print_PCB(current, wait_queue);
            system_status += "\n";

            ///////////////////////////////////////////////////////////////////////////////////////////

            //The following loop helps you do 2 things:
            // * Collect the trace of the child (and only the child, skip parent)
            // * Get the index of where the parent is supposed to start executing from
            std::vector<std::string> child_trace;
            bool skip = true;
            bool exec_flag = false;
            int parent_index = 0;

            for(size_t j = i + 1; j < trace_file.size(); j++) {
                auto [_activity, _duration, _pn] = parse_trace(trace_file[j]);
                if(skip && _activity == "IF_CHILD") {
                    skip = false;
                    continue;
                } else if(_activity == "IF_PARENT"){
                    skip = true;
                    parent_index = j;
                    if(exec_flag) {
                        break;
                    }
                } else if(skip && _activity == "ENDIF") {
                    skip = false;
                    parent_index = j;
                    continue;
                } else if(!skip && _activity == "EXEC") {
                    skip = true;
                    child_trace.push_back(trace_file[j]);
                    exec_flag = true;
                }

                if(!skip) {
                    child_trace.push_back(trace_file[j]);
                }
            }
            i = parent_index;

            ///////////////////////////////////////////////////////////////////////////////////////////
            // Run the child process
            
            // Child has higher priority - execute child first
            auto [child_exec, child_status, child_end_time] = simulate_trace(
                child_trace,
                current_time,
                vectors,
                delays,
                external_files,
                child,
                std::vector<PCB>() // Child starts with empty wait queue
            );
            
            execution += child_exec;
            system_status += child_status;
            current_time = child_end_time;
            
            // After child completes, remove from wait queue and free memory
            wait_queue.pop_back();
            free_memory(&child);

            ///////////////////////////////////////////////////////////////////////////////////////////


        } else if(activity == "EXEC") {
            auto [intr, time] = intr_boilerplate(current_time, 3, 10, vectors);
            current_time = time;
            execution += intr;

            ///////////////////////////////////////////////////////////////////////////////////////////
            // EXEC Implementation
            
            // Step 1: Get size of program from external files
            unsigned int program_size = get_size(program_name, external_files);
            if(program_size == (unsigned int)-1) {
                std::cerr << "Error: Program " << program_name << " not found in external files!" << std::endl;
                break;
            }
            
            int size_time = get_random_time();
            execution += std::to_string(current_time) + ", " + std::to_string(size_time) + ", Program is " + std::to_string(program_size) + " Mb large\n";
            current_time += size_time;
            
            // Step 2: Find empty partition
            int find_time = get_random_time();
            execution += std::to_string(current_time) + ", " + std::to_string(find_time) + ", find empty partition\n";
            current_time += find_time;
            
            // Step 3: Free old memory if needed
            if(current.partition_number != -1) {
                free_memory(&current);
            }
            
            // Step 4: Update PCB with new program info
            current.program_name = program_name;
            current.size = program_size;
            
            // Step 5: Allocate new partition
            if(!allocate_memory(&current)) {
                std::cerr << "Error: Cannot allocate memory for program " << program_name << std::endl;
                break;
            }
            
            // Step 6: Simulate loader (15ms per MB)
            int load_duration = program_size * 15;
            execution += std::to_string(current_time) + ", " + std::to_string(load_duration) + ", loading program into memory\n";
            current_time += load_duration;
            
            // Step 7: Mark partition as occupied
            int mark_time = get_random_time();
            execution += std::to_string(current_time) + ", " + std::to_string(mark_time) + ", marking partition as occupied\n";
            current_time += mark_time;
            
            // Step 8: Update PCB
            int update_time = get_random_time();
            execution += std::to_string(current_time) + ", " + std::to_string(update_time) + ", updating PCB\n";
            current_time += update_time;
            
            // Step 9: Call scheduler
            int sched_time = 0;
            execution += std::to_string(current_time) + ", " + std::to_string(sched_time) + ", scheduler called\n";
            current_time += sched_time;
            
            // Step 10: IRET
            execution += std::to_string(current_time) + ", 1, IRET\n";
            current_time += 1;

            // Create system status snapshot
            system_status += "time: " + std::to_string(current_time) + "; current trace: EXEC " + program_name + ", " + std::to_string(duration_intr) + "\n";
            system_status += print_PCB(current, wait_queue);
            system_status += "\n";

            ///////////////////////////////////////////////////////////////////////////////////////////

            // Load the external program trace
            std::ifstream exec_trace_file(program_name + ".txt");
            
            if(!exec_trace_file.is_open()) {
                std::cerr << "Error: Cannot open trace file for program " << program_name << std::endl;
                break;
            }

            std::vector<std::string> exec_traces;
            std::string exec_trace;
            while(std::getline(exec_trace_file, exec_trace)) {
                exec_traces.push_back(exec_trace);
            }
            exec_trace_file.close();

            ///////////////////////////////////////////////////////////////////////////////////////////
            // Run the exec program
            
            auto [exec_exec, exec_status, exec_end_time] = simulate_trace(
                exec_traces,
                current_time,
                vectors,
                delays,
                external_files,
                current,
                wait_queue
            );
            
            execution += exec_exec;
            system_status += exec_status;
            current_time = exec_end_time;

            ///////////////////////////////////////////////////////////////////////////////////////////

            break; // Break because EXEC replaces current process - no more trace to execute

        } else if(activity == "IF_CHILD" || activity == "IF_PARENT" || activity == "ENDIF") {
            // These are handled in the FORK logic, skip them here
            continue;
        }
    }

    return {execution, system_status, current_time};
}

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    //external_files is a C++ std::vector of the struct 'external_file'. Check the struct in 
    //interrupt.hpp to know more.
    auto [vectors, delays, external_files] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    //Just a sanity check to know what files you have
    print_external_files(external_files);

    //Make initial PCB (notice how partition is not assigned yet)
    PCB current(0, -1, "init", 1, -1);
    //Update memory (partition is assigned here, you must implement this function)
    if(!allocate_memory(&current)) {
        std::cerr << "ERROR! Memory allocation failed!" << std::endl;
    }

    std::vector<PCB> wait_queue;

    /******************ADD YOUR VARIABLES HERE*************************/
    
    // PID counter is now global
    // Random number generator is now global

    /******************************************************************/

    //Converting the trace file into a vector of strings.
    std::vector<std::string> trace_file;
    std::string trace;
    while(std::getline(input_file, trace)) {
        trace_file.push_back(trace);
    }

    auto [execution, system_status, _] = simulate_trace(   
        trace_file, 
        0, 
        vectors, 
        delays,
        external_files, 
        current, 
        wait_queue
    );

    input_file.close();

    write_output(execution, "execution.txt");
    write_output(system_status, "system_status.txt");

    return 0;
}
