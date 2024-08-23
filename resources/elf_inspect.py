#!/usr/bin/env python3

"""
Intel Corporation © 2024
All rights reserved.

Description:
------------
This script processes the output of `xt_objdump` for a compiled LX7 binary,
purifies the captured output to create a clean instruction listing, summarizes
the count of instructions found in various `.s` files, and estimates the total
CPU cycles used by the binary based on a JSON dictionary of instruction cycles.

Steps:
------
1. Execute `xt_objdump` on the compiled ELF binary.
2. Filter and purify the `xt_objdump` output to generate a clean instruction
   listing.
3. Count the instructions found in the `.s` files.
4. Estimate the total CPU cycles for the binary using a JSON dictionary
   of instruction cycles.

Input:
------
- Path to the ELF file for disassembly.
- Path to the build output directory containing `.s` files.

Output:
-------
- A raw disassembly file: `<elf_path>.raw.txt`
- A filtered disassembly file: `<elf_path>.raw.filtered.txt`
- A summary of instruction counts and estimated CPU cycles.

Usage:
------
    python elf_inspect.py <path_to_elf_file> <path_to_build_output>

Arguments:
    <path_to_elf_file>      - The path to the ELF file to disassemble and filter.
    <path_to_build_output>  - The path to the build output directory containing `.s` files.

"""

import subprocess
import re
import sys
import os
import json
from prettytable import PrettyTable

instruction_json_file = "resources/xtensa_lx7_instruction_cycles.json"

# Define ANSI color codes
COLOR_RESET = "\033[0m"
COLOR_CYAN = "\033[36m"
COLOR_YELLOW = "\033[33m"


def load_instruction_cycles(json_file):
    """Load the instruction cycle dictionary from a JSON file."""
    with open(json_file, 'r') as f:
        return json.load(f)


def estimate_cpu_cycles(instruction_file, instruction_cycles):
    """Estimate the total CPU cycles used by instructions in the given file."""
    total_cycles = 0
    line_number = 0
    instruction_count = 0

    with open(instruction_file, 'r') as file:
        for line in file:
            match = re.search(r'^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]+\s+([a-zA-Z0-9.]+)', line)
            if match:
                instruction = match.group(1)
                instruction_count += 1

                if instruction in instruction_cycles:
                    cycles = instruction_cycles[instruction]["cycles"]
                else:
                    cycles = 1  # Default cycle count if not found
                    print(f"Warning: Instruction '{instruction}' at line {line_number} "
                          f"not found in JSON, counting as 1 cycle.")

                total_cycles += cycles
            else:
                print(f"Skipping non-instructional line: '{line.strip()}'")

            line_number += 1

    return total_cycles


def find_s_files_recursive(directory):
    """Recursively find all `.s` files in a given directory."""
    s_files = []
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith('.s'):
                s_files.append(os.path.join(root, file))
    return s_files

def count_instructions_in_s_file(func_body):
    """Placeholder function for counting instructions in a function body."""
    # Implement your instruction counting logic here.
    # This is a placeholder and should be replaced with actual logic.
    return len(re.findall(r'\n', func_body))  # Example: Count number of lines as a placeholder.


def analyze_s_file(s_file_path):
    """Analyze a `.s` file to count instructions in each function."""
    with open(s_file_path, 'r') as file:
        content = file.read()

    c_file_match = re.search(r'\.file\s+"(.+?\.c)"', content)
    c_file_name = os.path.basename(c_file_match.group(1)) if c_file_match else "Unknown"

    functions = re.findall(r'\.type\s+([^\s,]+),@function', content)
    instruction_counts = {}

    for func in functions:
        pattern = rf'{func}:\n(.*?)(?=\.size|\.type|\Z)'
        func_body = re.search(pattern, content, re.DOTALL)
        if func_body:
            instruction_counts[func] = (count_instructions_in_s_file(func_body.group(0)), c_file_name)

    return instruction_counts


def analyze_s_files_in_directory(directory):
    """Analyze all `.s` files in a directory recursively."""
    s_files = find_s_files_recursive(directory)
    all_instruction_counts = {}

    for s_file in s_files:
        instruction_counts = analyze_s_file(s_file)
        all_instruction_counts[s_file] = instruction_counts

    return all_instruction_counts


def summarize_instruction_counts(build_out_path, total_elf_instructions, instruction_cycles):
    """Summarize the instruction counts from `.s` files and estimate CPU cycles."""
    table = PrettyTable()
    table.field_names = ["Function Name", "C File", "Instruction Count"]

    total_s_instructions = 0

    # Analyze all `.s` files in the directory recursively
    all_functions = analyze_s_files_in_directory(build_out_path)
    for s_file, functions in all_functions.items():
        for func_name, (instr_count, c_file_name) in functions.items():
            table.add_row([func_name, c_file_name, instr_count])
            total_s_instructions += instr_count

    # Add summary rows
    table.add_row(["Instructions (.s)", "", f"{COLOR_CYAN}{total_s_instructions}{COLOR_RESET}"])
    table.add_row(["Instructions (.elf)", "", f"{COLOR_CYAN}{total_elf_instructions}{COLOR_RESET}"])

    # Estimate cycles
    total_cycles = estimate_cpu_cycles(filtered_output_file, instruction_cycles)
    table.add_row(["Cycles", "", f"{COLOR_YELLOW}{total_cycles}{COLOR_RESET}"])

    # Calculate and display CPI
    if total_s_instructions:
        cpi = total_cycles / total_s_instructions
        print(f"{COLOR_YELLOW}Cycles Per Instruction (CPI):{COLOR_RESET} {COLOR_CYAN}{cpi:.2f}{COLOR_RESET}")
    
    print(f"\n{table}")


def run_xt_objdump(elf_path):
    """Run `xt-objdump` on the ELF file and filter the output."""
    raw_output_file = f"{elf_path}.raw.txt"
    global filtered_output_file
    filtered_output_file = f"{elf_path}.raw.filtered.txt"

    try:
        # Execute xt-objdump
        with open(raw_output_file, 'w') as raw_file:
            subprocess.run(['xt-objdump', '-d', elf_path], stdout=raw_file, check=True)

        # Read and filter the output
        with open(raw_output_file, 'r') as file:
            lines = file.readlines()

        # Filter lines to only include those that start with two spaces, a six-digit hex address, and a colon
        valid_line_pattern = re.compile(r'^\s{2}[0-9a-fA-F]{6}:')

        lines = [line for line in lines if valid_line_pattern.match(line)]

        # Additional filtering: remove lines ending with ":"
        lines = [line for line in lines if not line.strip().endswith(":")]

        # Remove lines ending with ".."
        lines = [line for line in lines if not line.strip().endswith("..")]

        # Remove lines with a specific hex pattern (likely raw data)
        hex_pattern = re.compile(r'^\s*[0-9a-fA-F]+:\s+([0-9a-fA-F]{8}\s+){3}[0-9a-fA-F]{8}\s+.*$')
        lines = [line for line in lines if not hex_pattern.match(line)]

        # Remove lines that do not contain any alphabetical characters (non-instructions)
        lines = [line for line in lines if any(char.isalpha() for char in line)]

        # Save the filtered output
        with open(filtered_output_file, 'w') as filtered_file:
            filtered_file.writelines(lines)

        # Return the line count of filtered instructions
        return len(lines)

    except subprocess.CalledProcessError as e:
        print(f"Error: Failed to run xt-objdump: {e}")
        return 0
    except Exception as e:
        print(f"An unexpected error occurred: {e}")


if __name__ == "__main__":
    # Ensure the script is run with exactly two arguments (the ELF file path and build output path)
    if len(sys.argv) != 3:
        print("Usage: python elf_inspect.py <path_to_elf_file> <path_to_build_output>")
        sys.exit(1)

    elf_path = sys.argv[1]
    build_out_path = sys.argv[2]

    # Run the main process
    total_elf_instructions = run_xt_objdump(elf_path)
    instruction_cycles = load_instruction_cycles(instruction_json_file)
    summarize_instruction_counts(build_out_path, total_elf_instructions, instruction_cycles)
