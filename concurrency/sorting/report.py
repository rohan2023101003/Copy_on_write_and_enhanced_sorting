import time
import psutil
import subprocess
import random

# Constants
FILE_COUNTS = [31, 101, 192, 500, 1000, 2000, 5000, 10000, 20000]

def run_sorting_algorithm(count):
    """Run the sorting algorithm and return execution time and memory usage."""
    print(f"Generating {count} random files...")
    files = generate_random_files(count)
    
    # Write files to a temporary input file
    try:
        with open('temp_input.txt', 'w') as f:
            f.write(f"{count}\n")
            for file in files:
                f.write(f"{file[0]} {file[1]} {file[2]}\n")
            f.write("ID\n")  # Assuming sorting by ID
        print("Temporary input file created successfully.")
    except Exception as e:
        print(f"Error writing to temp_input.txt: {e}")
        return None, None

    # Measure memory before running the algorithm
    process = psutil.Process()
    memory_before = process.memory_info().rss
    print(f"Memory before execution: {memory_before} bytes")

    # Run the sorting algorithm with input redirection
    start_time = time.time()
    try:
        with open('temp_input.txt', 'r') as input_file:
            subprocess.run(['./countsort'], stdin=input_file, check=True)  # Execute the sorting algorithm
    except subprocess.CalledProcessError as e:
        print(f"Error executing countsort: {e}")
        return None, None
    execution_time = time.time() - start_time

    # Measure memory after running the algorithm
    memory_after = process.memory_info().rss
    memory_usage = memory_after
    print(f"Memory after execution: {memory_after} bytes")
    print(f"Execution time: {execution_time:.4f} seconds")

    return execution_time, memory_usage

def generate_random_files(num_files):
    """Generate random file data for testing."""
    files = []
    for _ in range(num_files):
        filename = f"file{random.randint(1, 1000)}.txt"
        file_id = random.randint(1, 1000)
        timestamp = f"2023-10-01T{random.randint(0, 23):02}:{random.randint(0, 59):02}:{random.randint(0, 59):02}"
        files.append((filename, file_id, timestamp))
    # print(f"Generated files: {files}")
    return files

def main():
    results = []

    for count in FILE_COUNTS:
        time_taken, memory_used = run_sorting_algorithm(count)
        if time_taken is not None and memory_used is not None:
            results.append(f"Files: {count}, Time: {time_taken:.4f} seconds, Memory: {memory_used} bytes")
        else:
            print(f"Failed to process {count} files.")

    # Write results to a text file
    try:
        with open('performance_report.txt', 'w') as report_file:
            for result in results:
                report_file.write(result + '\n')
        print("Performance report written successfully.")
    except Exception as e:
        print(f"Error writing performance report: {e}")

if __name__ == "__main__":
    main()
