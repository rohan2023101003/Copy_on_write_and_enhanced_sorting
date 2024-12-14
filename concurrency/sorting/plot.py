import matplotlib.pyplot as plt
import os

# Function to read performance data from a file
def read_performance_data(file_path):
    file_counts = []
    execution_times = []
    memory_usages = []

    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split(', ')
            file_count = int(parts[0].split(': ')[1])
            execution_time = float(parts[1].split(': ')[1].split(' ')[0])
            memory_usage = int(parts[2].split(': ')[1].split(' ')[0])
            
            file_counts.append(file_count)
            execution_times.append(execution_time)
            memory_usages.append(memory_usage)

    return file_counts, execution_times, memory_usages

# Print current working directory
print("Current working directory:", os.getcwd())

# Read data from both performance report files
file_counts_merge, execution_times_merge, memory_usages_merge = read_performance_data('/home/akmal-ali/2nd_year/osn/mps/mini-project-3-49/concurrency/performance_report_merge.txt')
file_counts_count, execution_times_count, memory_usages_count = read_performance_data('/home/akmal-ali/2nd_year/osn/mps/mini-project-3-49/concurrency/performance_report_count.txt')

# Plotting execution time
plt.figure(figsize=(12, 6))

# Line graph for execution time
plt.subplot(1, 1, 1)
plt.plot(file_counts_merge, execution_times_merge, marker='o', label='Distributed Merge Sort', color='blue')
plt.plot(file_counts_count, execution_times_count, marker='o', label='Distributed Count Sort', color='orange')
plt.title('Execution Time vs File Count')
plt.xlabel('Number of Files')
plt.ylabel('Execution Time (seconds)')
plt.legend()
plt.grid()

# Save the execution time graph as an image
plt.savefig('execution_time_graph.png')
plt.close()  # Close the figure to free memory

# Plotting memory usage
plt.figure(figsize=(12, 6))

# Bar chart for memory usage
bar_width = 0.35
x = range(len(file_counts_merge))

# Bar positions
bar1 = [memory_usages_merge[i] for i in range(len(memory_usages_merge))]
bar2 = [memory_usages_count[i] for i in range(len(memory_usages_count))]

# Create bars
plt.bar(x, bar1, width=bar_width, label='Distributed Merge Sort', color='blue', alpha=0.6)
plt.bar([p + bar_width for p in x], bar2, width=bar_width, label='Distributed Count Sort', color='orange', alpha=0.6)

plt.title('Memory Usage Comparison')
plt.xlabel('Number of Files')
plt.ylabel('Memory Usage (bytes)')
plt.xticks([p + bar_width / 2 for p in x], file_counts_merge)  # Center the x-ticks
plt.legend()
plt.grid()

# Save the memory usage graph as an image
plt.savefig('memory_usage_graph.png')
plt.close()  # Close the figure to free memory

print("Graphs saved as 'execution_time_graph.png' and 'memory_usage_graph.png'.")
