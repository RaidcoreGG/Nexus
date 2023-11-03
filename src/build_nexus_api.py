import os
import sys
import argparse
import re

ERROR_FILE_NOT_EXIST = -1

HEADER_PROCESS_BLACKLIST = ["#ifndef", "#define", "#include", "#endif", "#pragma", "#if", "#error", "extern \"C\" {;"]
#HEADER_PROCESS_BLACKLIST = []

# Find all headers' absolute paths recursively
def collect_headers(source_file_path, blacklist):
    headers = list()
    
    with open(source_file_path, 'r') as source_file:
        for line in source_file:
            # Reject <> libraries
            if re.match(r'#include <(.*?)>', line):
                continue
            
            match = re.search(r'#include "(.*?)"', line)
            
            # Reject non #include lines
            if not match:
                continue
            
            # Get included header relative path
            included_file = match.group(1)
            
            # Check if file name is on blacklist
            if any(item in included_file for item in blacklist):
                print(f"\tSkipping blacklisted file: {included_file}")
                continue
            
            # Source directory
            source_directory = os.path.dirname(source_file_path)  
            
            # Get included header absolute path
            included_file_path = os.path.abspath(os.path.join(source_directory, included_file))
            
            # Check if file exist 
            if not os.path.exists(included_file_path):
                print(f"\tSkipping non existing file: {included_file_path}")
                continue
                
            print(f"\tFound header for processing: {included_file_path}")
            
            # Add found headers
            headers.append(included_file_path)
            
            # Add headers recursive (merge lists)
            for item in collect_headers(included_file_path, blacklist):
                if item not in headers:
                    headers.append(item)
            
        
    return headers   

# Process all found unique headers
def process_headers(target_file_path, headers, write_mode):
    blank_line_counter = 0

    mapping = {"write": "w", "append": "a"}
    mode = mapping.get(write_mode, "a") 

    print(f"[Processing {len(headers)} headers ({write_mode})]")
        
    with open(target_file_path, mode) as target_file:
        for header_path in headers:
            print(f"\tProcessing {header_path}")
            
            with open(header_path, 'r') as header_file:
                for line in header_file:
                    # Skip any lines that contains words from processing blacklist
                    if any(item in line for item in HEADER_PROCESS_BLACKLIST):
                        continue
                        
                    # Count multiple lines
                    if line.isspace():
                        blank_line_counter = blank_line_counter + 1
                        # Skip blank line if there was blank line before
                        if not blank_line_counter > 1:
                            target_file.write(line)
                    else:
                        # Reset counter
                        blank_line_counter = 0
                        # Write line
                        target_file.write(line)
                   
def process_source(source_file_path, target_file_path):
    blank_line_counter = 0
    
    print(f"[Adding original source file to API: {source_file_path}]")
    
    with open(target_file_path, 'a') as target_file:
        with open(source_file_path, 'r') as source_file:
            for line in source_file:
                # Skip any preprocessor directives
                if any(item in line for item in HEADER_PROCESS_BLACKLIST):
                    continue;
                
                # Count multiple lines
                if line.isspace():
                    blank_line_counter = blank_line_counter + 1
                    # Skip blank line if there was blank line before
                    if not blank_line_counter > 1:
                        target_file.write(line)
                else:
                    # Reset counter
                    blank_line_counter = 0
                    # Write line
                    target_file.write(line)
                        
                    
                   
def main(args):
    # Blacklist
    blacklist = args.blacklisted_includes
    # Absolute path of source file
    source_file_paths = [os.path.abspath(source_path) for source_path in args.source_path]
    # Absolute path of target file
    target_file_path =  os.path.abspath(args.target_path)
    # write mode
    write_mode = args.write_mode

    # Collect all headers recursively
    headers = list()
    for source_file_path in source_file_paths:
        print(f"[Detecting headers for {source_file_path}]")
        for item in collect_headers(source_file_path, blacklist):
            if item not in headers:
                print(f"\tFound unique header {item}")
                headers.append(item)
            else:
                print(f"\tSkipped duplicate header {item}")
        
    # Process all headers 
    process_headers(target_file_path, headers, write_mode)
    
    # Add original sources to API
    for source_file_path in source_file_paths:
        process_source(source_file_path, target_file_path)
    
    print(f"[Completed: {target_file_path}]")

if __name__ == "__main__":
    # Argument parsing
    parser = argparse.ArgumentParser(description="Nexus.h builder")
    parser.add_argument("source_path", nargs="+", help="Paths to the multiple source files")
    parser.add_argument("--target_path", help="Path to the target Nexus.h", default=".\\Nexus.h")
    parser.add_argument("--blacklisted_includes", nargs='+', default=[], help="List of blacklisted include files")
    parser.add_argument(
    "--write_mode",
    choices=["write", "append"],
    default="write",
    help="Specify the write mode (choices: write, append; default: write)",
)
    args = parser.parse_args()
    
    # Main process
    main(args)