# Estimation of the Required LX7 Cores for USB High Speed (480 Mbps) Throughput

## Overview
We aim to estimate the number of LX7 cores required to handle USB High Speed (480 Mbps) throughput. The USB interface will be utilized in its most basic form, functioning similarly to a glorified UART, serving as a fast point-to-point communication channel without the full complexity of the USB protocol.

### Key Question:
**How many LX7 cores do we really need?**

## 1. Hardware Aspects (Rough Estimations)

- **USB ASIC Efficiency**: 
  - No hardware is perfect; there may be bugs, wasted cycles, and other inefficiencies.
  
- **Interfacing with USB Peripheral**: 
  - Considerations include using DMA, direct ISR, or register access.

## 2. Breaking Down 'Throughput'

- **BPS Processing Capacity**: 
  - We need to determine how many bits per second (BPS) the core can process.
  
- **MCTP Frames**:
  - Bits are essentially MCTP frames being processed by a core in a given timeframe.
  
- **MCTP Operation Time**:
  - We need to calculate the time it takes for the core to perform an 'MCTP operation'.

## 3. Setting Up the Test Environment

- **Efficient Cycle Estimation**:
  - We need a mechanism to estimate the cycles used per 'logical operation'. In this case, the operation involves receiving an MCTP packet via USB, performing an integrity check, and either storing the frame or passing it to another entity (pass-through mode).
  
- **Focus**:
  - The primary focus is on setting up an efficient test environment for accurate cycle estimation.

## 4. Tools

- **Xtensa LX7 SDK**:
  - **Compiler, Linker**: `xt-clang`
  - **Debugger**: `xt-gdb` (also capable of emulation)
  - **Simulator**: `xt-run`
  - **Object Dumper**: `xt_objdump`

## 5. Testing Methodology

### A. Long, Not 100% Accurate Method

1. **Create a `hello_world.c` Project**:
   - Avoid using the standard library (e.g., no `printf()`, `memcpy()`).
   - If necessary, create your own versions of these functions.

2. **Compilation with `xt-clang`**:
   - **Linker Flags**: `-nostdlib -nodefaultlibs`
     - This removes all standard libraries, leaving only your code.
   - **Compiler Flags**: `-O3 -DNDEBUG -ffreestanding -nostartfiles -c -save-temps=obj`
     - This optimizes the code and saves the assembly output (`.s` file).

3. **Assembly Output**:
   - Use the `.s` file or `xt_objdump` to generate an assembly export of the ELF executable.

4. **Python Script**:
   - Use Python to clean the output and retain only the instructions.

5. **Cycle Counting**:
   - Use Python along with a JSON file that acts as a dictionary mapping the LX7 instruction set to the associated CPU cycles.

6. **Final Calculation**:
   - Count the cycles to estimate the required cores.

7. **Completion**:
   - Done!


### B. Using Emulation (Probably the Best Method)

1. **Compile a Standard Bare (Empty `main()`) Project**:
   - This project will serve as your baseline for comparison.

2. **Compile with `xt-clang`**:
   - Use maximum optimization flags: `-O3 -DNDEBUG`.
   - The output will be an ELF file representing your baseline project.

3. **Run the Baseline in Simulation**:
   - Use `xt-run` to simulate the execution of the baseline project.
   - Record the number of cycles used by the baseline project.

4. **Add the Rest of Your Code**:
   - Integrate the full code into your project.

5. **Compile and Simulate Again**:
   - Compile the full project and run it through the simulator using `xt-run`.

6. **Calculate the Difference**:
   - Subtract the number of cycles recorded in Step 3 (baseline) from the cycles recorded in Step 5 (full project).

7. **Conclusion**:
   - The difference in cycles gives you an estimate of the additional processing required by your code.

   - You're done!

