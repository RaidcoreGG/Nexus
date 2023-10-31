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
                print(f"Skipping blacklisted file: {included_file}")
                continue
            
            # Source directory
            source_directory = os.path.dirname(source_file_path)  
            
            # Get included header absolute path
            included_file_path = os.path.abspath(os.path.join(source_directory, included_file))
            
            # Check if file exist 
            if not os.path.exists(included_file_path):
                print(f"Skipping non existing file: {included_file_path}")
                continue
                
            print(f"Found header for processing: {included_file_path}")
            
            # Add found headers
            headers.append(included_file_path)
            
            # Add headers recursive (merge lists)
            headers = headers + collect_headers(included_file_path, blacklist)
            
        
    return list(set(headers))    

# Process all found unique headers
def process_headers(target_file_path, headers):
    blank_line_counter = 0

    print(f"[Processing {len(headers)} headers]")
        
    with open(target_file_path, 'w') as target_file:
        for header_path in headers:
            print(f"Processing {header_path}")
            
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
                   
                    
                   
def main(args):
    # Blacklist
    blacklist = args.blacklisted_includes
    # Absolute path of source file
    source_file_path = os.path.abspath(args.source_path)
    # Absolute path of target file
    target_file_path =  os.path.abspath(args.target_path)

    # Collect all headers recursively
    headers = collect_headers(source_file_path, blacklist)
    # Process all headers 
    process_headers(target_file_path, headers)
    
    print("Completed")

if __name__ == "__main__":
    # Argument parsing
    parser = argparse.ArgumentParser(description="Nexus.h builder")
    parser.add_argument("source_path", help="Path to the source APIDefs.h")
    parser.add_argument("--target_path", help="Path to the target Nexus.h", default=".\\Nexus.h")
    parser.add_argument("--blacklisted_includes", nargs='+', default=[], help="List of blacklisted include files")
    args = parser.parse_args()
    
    # Main process
    main(args)