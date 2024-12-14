# Distributed Sorting System and Copy-on-Write Fork in xv6

This README outlines the implementation details, performance analysis, and usage of two projects: a distributed sorting system and a copy-on-write (CoW) fork mechanism in xv6.

## Some pointers
- main xv6 source code is present inside `initial_xv6/src` directory. This is where you will be making all the additions/modifications necessary for the xv6 part of the Mini Project. 
- work inside the `concurrency/` directory for the concurrency part of the Mini Project.
---

## Part 1: Distributed Sorting System

This project implements two sorting algorithms for a distributed system: **Distributed Merge Sort** and **Distributed Count Sort**. Both were evaluated for performance in terms of execution time and memory usage across datasets of varying sizes.

### Implementation Details

#### Distributed Merge Sort
- **Approach**: Each merge operation is handled by a separate thread for concurrent processing.
  
**Pros**:
- Scales well with large datasets.
- Utilizes multi-core systems effectively.

**Cons**:
- Higher memory overhead.
- Thread management complexity for large data sizes.

#### Distributed Count Sort
- **Approach**: Optimized for datasets with a limited range of values.

**Pros**:
- Lower memory usage.
- Faster for smaller or moderately sized data ranges.

**Cons**:
- Does not scale well for datasets with a broad range of values.

### Input Format

The input begins with an integer specifying the total number of files. Each file is represented on a separate line with its attributes separated by spaces in the following order: `name`, `id`, and `timestamp`. The final row contains the column to be used for sorting (one of ["Name", "ID", "Timestamp"]).

- **Timestamp**: ISO 8601 format (YYYY-MM-DDTHH:MM:SS)
- **File name**: String with a maximum length of 128 characters

#### Example Input
```
5
fileA.txt 101 2023-10-01T14:30:00
fileB.txt 102 2023-10-01T12:15:00
fileC.txt 103 2023-09-29T09:45:00
fileD.txt 104 2023-10-02T17:05:00
fileE.txt 105 2023-09-30T10:20:00
ID
```

### Sorting Criteria
- **Distributed Count Sort**: Used when the total number of files is below a threshold of 42.
- **Distributed Merge Sort**: Used when the total number of files exceeds the threshold of 42.

### Output Format

The output begins with the name of the sorting column used, indicated on the first line. The sorted list of files follows, each on a new line with attributes in the same order as the input.

- Attributes are separated by spaces.
- Each file entry is separated by a newline.

#### Example Output
Assuming sorting by `ID`:
```
ID
fileA.txt 101 2023-10-01T14:30:00
fileB.txt 102 2023-10-01T12:15:00
fileC.txt 103 2023-09-29T09:45:00
fileD.txt 104 2023-10-02T17:05:00
fileE.txt 105 2023-09-30T10:20:00
```

### Performance Analysis

#### Execution Time
- **Distributed Merge Sort**:
  - 31 files: 0.0102 seconds
  - 101 files: 0.0136 seconds
  - 1929 files: 0.0277 seconds
- **Distributed Count Sort**:
  - 31 files: 0.0025 seconds
  - 101 files: 0.0027 seconds
  - 1929 files: 0.0119 seconds

#### Memory Usage
- **Distributed Merge Sort**:
  - 31 files: 14,155,776 bytes
  - 101 files: 14,155,776 bytes
  - 1929 files: 14,417,920 bytes
- **Distributed Count Sort**:
  - 31 files: 14,024,704 bytes
  - 101 files: 14,024,704 bytes
  - 1929 files: 14,286,848 bytes

### Summary
- **Distributed Count Sort**: More efficient for smaller datasets.
- **Distributed Merge Sort**: Scales better for larger datasets.
- Potential optimization: A hybrid approach using Count Sort for smaller subproblems within Merge Sort.

---

## Part 2: Copy-on-Write Fork in xv6

This part of the project implements a **Copy-on-Write (CoW)** fork mechanism in xv6, enhancing memory efficiency during process creation.

### Implementation Highlights
- **Spinlock**: Added a reference counter for each physical page.
- **kalloc.c**: Managed reference counters for memory pages.
- **Trap.c**: Added a handler for CoW page faults.
- **Vm.c**:
  - `uvmcopy`: Reuses physical memory pages and adjusts page table entries (PTEs).
  - `copyout`: Handles CoW pages for write operations.

### Performance Analysis
#### Page Faults
- **Read-Only**:
  - With `fork()`: 13 page faults for 10 pages.
  - Without `fork()`: 0 page faults.
- **Write**:
  - With `fork()`: 23 page faults for 10 pages.
  - Without `fork()`: 1 page fault.

### Benefits
- Reduced memory usage during process creation.
- Deferred copying conserves resources and reduces overhead.

### Optimization Suggestions
- Free unshared pages proactively.
- Prefetch memory for anticipated write operations.

---
### How to run :
- clone this repository
- cd initial-xv6
- cd src
- make qemu
- run cowtest command



## Conclusion

- The distributed sorting system and CoW fork in xv6 address efficiency and scalability challenges in their respective domains.
- Future work includes refining hybrid approaches for sorting and optimizing CoW memory management further.

---

Read Report for more clarity
