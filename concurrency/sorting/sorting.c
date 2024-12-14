#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_FILES 100009
#define MAX_FILENAME 129
#define MAX_TIMESTAMP 21
#define NUM_THREADS 8
#define THRESHOLD 42
// Mutex for thread synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure to store file information
typedef struct {
    char filename[MAX_FILENAME];
    int id;
    char timestamp[MAX_TIMESTAMP];
} FileInfo;

// Structure to pass data to threads
typedef struct {
    FileInfo* files;
    int start;
    int end;
    int* count;
    int* positions;  // Array to track positions for each ID
    int total_files;
    const char *sorting_cloumn;
} ThreadData;

// Global variables for sorted result
FileInfo* sorted_files;
int compute_hash_for_name(const char* s) {
    long long hash_value = 0;

    // Iterate over each character in the string
    for (int i = 0; s[i] != '\0'; i++) {
        int char_value = (s[i] - 'a') + 11;  // Map 'a' to 11, 'b' to 12, ..., 'z' to 36
        hash_value = hash_value * 100 + char_value;  // Shift existing digits and append char_value
    }

    return hash_value/100000000;
}
int map_timestamp(FileInfo file) {
    int year, month, day, hour, minute, second;
    sscanf(file.timestamp, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);

    // Convert timestamp to total seconds since Unix epoch (1970-01-01)
    long long int total_seconds = ((((year - 1970) * 365 + (year - 1970) / 4 + month * 31 + day) * 24 + hour) * 60 + minute) * 60 + second;

    // Return the total seconds as the hash value
    return total_seconds%100000;
}
// Thread function for counting
void* count_elements(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int* local_count = calloc(MAX_FILES, sizeof(int));
    
    // Count IDs in local array
    if (strcmp(data->sorting_cloumn, "ID") == 0) {
        for (int i = data->start; i < data->end; i++) {
            int index = data->files[i].id % MAX_FILES;
            local_count[index]++;
        }
    }
    // Count Names in local array
    else if (strcmp(data->sorting_cloumn, "Name") == 0) {
        for (int i = data->start; i < data->end; i++) {
            int index = compute_hash_for_name(data->files[i].filename);
            // printf("Hash for %s: %d\n", data->files[i].filename, index);  // Debugging line
            local_count[index]++;
        }
    }
    else if (strcmp(data->sorting_cloumn, "Timestamp") == 0) {
        for (int i = data->start; i < data->end; i++) {
            int index = map_timestamp(data->files[i]);
            local_count[index]++;
        }
    }
    //critical section update global count
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_FILES; i++) {
        if (local_count[i] > 0) {
            data->count[i] += local_count[i];
        }
    }
    pthread_mutex_unlock(&mutex);
    free(local_count);
    return NULL;
}
// // Thread function for placing elements
void* place_elements(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    if (strcmp(data->sorting_cloumn, "ID") == 0) {
        for (int i = data->start; i < data->end; i++) {
            int index = data->files[i].id % MAX_FILES;
            int pos = __sync_fetch_and_add(&data->positions[index], 1);
            memcpy(&sorted_files[pos], &data->files[i], sizeof(FileInfo));   
        }
    }
    // Count Names in local array
    else if (strcmp(data->sorting_cloumn, "Name") == 0) {
        for (int i = data->start; i < data->end; i++) {
            int index = compute_hash_for_name(data->files[i].filename);
            // printf("Hash for %s: %d\n", data->files[i].filename, index);  // Debugging line
            int pos = __sync_fetch_and_add(&data->positions[index], 1);
            memcpy(&sorted_files[pos], &data->files[i], sizeof(FileInfo)); 
            
        }
    }
    else if (strcmp(data->sorting_cloumn, "Timestamp") == 0) {
        for (int i = data->start; i < data->end; i++) {
            int index = map_timestamp(data->files[i]);
            // printf("Hash for %s: %d\n", data->files[i].timestamp, index);  // Debugging line
            int pos = __sync_fetch_and_add(&data->positions[index], 1);
            memcpy(&sorted_files[pos], &data->files[i], sizeof(FileInfo));     
        }
    }
    return NULL;
}
// Function to set positions array based on cumulative counts
void set_positions(int* count, int* positions, int total_files) {
    int cumulative_position = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        positions[i] = cumulative_position;
        cumulative_position += count[i];
    }
}
void countsort(int total_files, FileInfo* files, const char* sort_column) {
       // Initialize count and positions arrays
    int* count = calloc(MAX_FILES, sizeof(int));
    int* positions = calloc(MAX_FILES, sizeof(int));
    if (!count || !positions) {
        printf("Memory allocation failed\n");
        return ;
    }
    
    // Create threads for counting
    pthread_t count_threads[NUM_THREADS];
    ThreadData count_thread_data[NUM_THREADS];
    int chunk_size = total_files / NUM_THREADS;
    
    for (int i = 0; i < NUM_THREADS; i++) {
        count_thread_data[i].files = files;
        count_thread_data[i].start = i * chunk_size;
        count_thread_data[i].end = (i == NUM_THREADS - 1) ? total_files : (i + 1) * chunk_size;
        count_thread_data[i].count = count;
        count_thread_data[i].positions = positions;
        count_thread_data[i].total_files = total_files;
        count_thread_data[i].sorting_cloumn = sort_column;
        
        if (pthread_create(&count_threads[i], NULL, count_elements, &count_thread_data[i]) != 0) {
            printf("Thread creation failed\n");
            return ;
        }
    }
    
    // Wait for counting threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(count_threads[i], NULL) != 0) {
            printf("Thread join failed\n");
            return ;
        }
    }
    
    // Set positions array for placements based on cumulative counts
    set_positions(count, positions, total_files);
    
    // Create threads for placing elements
    pthread_t place_threads[NUM_THREADS];
    ThreadData place_thread_data[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        place_thread_data[i].files = files;
        place_thread_data[i].start = i * chunk_size;
        place_thread_data[i].end = (i == NUM_THREADS - 1) ? total_files : (i + 1) * chunk_size;
        place_thread_data[i].count = count;
        place_thread_data[i].positions = positions;
        place_thread_data[i].total_files = total_files;
        place_thread_data[i].sorting_cloumn = sort_column;
        
        if (pthread_create(&place_threads[i], NULL, place_elements, &place_thread_data[i]) != 0) {
            printf("Thread creation failed\n");
            return ;
        }
    }
    
    // Wait for placing threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(place_threads[i], NULL) != 0) {
            printf("Thread join failed\n");
            return ;
        }
    }

    // Print results
    printf("%s\n", sort_column);
    for (int i = 0; i < total_files; i++) {
        printf("%s %d %s\n", sorted_files[i].filename, sorted_files[i].id, sorted_files[i].timestamp);
    }

}

// Function to merge two halves
void merge(FileInfo* files, int left, int mid, int right, const char* sort_column) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    FileInfo* L = malloc(n1 * sizeof(FileInfo));
    FileInfo* R = malloc(n2 * sizeof(FileInfo));

    for (i = 0; i < n1; i++)
        L[i] = files[left + i];
    for (j = 0; j < n2; j++)
        R[j] = files[mid + 1 + j];

    i = 0; 
    j = 0; 
    k = left; 

    while (i < n1 && j < n2) {
        if ((strcmp(sort_column, "ID") == 0 && L[i].id <= R[j].id) ||
            (strcmp(sort_column, "Name") == 0 && strcmp(L[i].filename, R[j].filename) <= 0) ||
            (strcmp(sort_column, "Timestamp") == 0 && strcmp(L[i].timestamp, R[j].timestamp) <= 0)) {
            files[k] = L[i];
            i++;
        } else {
            files[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        files[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        files[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

// Function to implement merge sort
void merge_sort(FileInfo* files, int left, int right, const char* sort_column) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        merge_sort(files, left, mid, sort_column);
        merge_sort(files, mid + 1, right, sort_column);
        merge(files, left, mid, right, sort_column);
    }
}

// New thread function for merge sort
void* merge_sort_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    merge_sort(data->files, data->start, data->end - 1, data->sorting_cloumn);
    return NULL;
}

int main() {
    int total_files;
    scanf("%d", &total_files);
    
    // Allocate memory for file information
    FileInfo* files = malloc(total_files * sizeof(FileInfo));
    sorted_files = malloc(total_files * sizeof(FileInfo));
    if (!files || !sorted_files) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Read file information
    for (int i = 0; i < total_files; i++) {
        scanf("%s %d %s", files[i].filename, &files[i].id, files[i].timestamp);
    }
    
    // Read sorting column (optional, not used here for simplicity)
    char sort_column[20];
    scanf("%s", sort_column);
    
    if (total_files <= THRESHOLD)
     {
        countsort(total_files, files, sort_column);
    } else {
        // Create threads for merge sort
        pthread_t merge_threads[NUM_THREADS];
        ThreadData merge_thread_data[NUM_THREADS];
        int chunk_size = total_files / NUM_THREADS;

        for (int i = 0; i < NUM_THREADS; i++) {
            merge_thread_data[i].files = files;
            merge_thread_data[i].start = i * chunk_size;
            merge_thread_data[i].end = (i == NUM_THREADS - 1) ? total_files : (i + 1) * chunk_size;
            merge_thread_data[i].sorting_cloumn = sort_column;

            if (pthread_create(&merge_threads[i], NULL, merge_sort_thread, &merge_thread_data[i]) != 0) {
                printf("Thread creation failed\n");
                return 1;
            }
        }

        // Wait for merge sort threads to complete
        for (int i = 0; i < NUM_THREADS; i++) {
            if (pthread_join(merge_threads[i], NULL) != 0) {
                printf("Thread join failed\n");
                return 1;
            }
        }

        // Copy sorted files to sorted_files
        memcpy(sorted_files, files, total_files * sizeof(FileInfo));
        
        // Print results
        printf("%s\n", sort_column);
        for (int i = 0; i < total_files; i++) {
            printf("%s %d %s\n", sorted_files[i].filename, sorted_files[i].id, sorted_files[i].timestamp);
        }
    }



    // Cleanup
    free(files);
    free(sorted_files);
    pthread_mutex_destroy(&mutex);
    return 0;
}
