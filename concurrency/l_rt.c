#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Colors for output
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_PINK "\033[1;35m"
#define COLOR_WHITE "\033[1;37m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_RED "\033[1;31m"
#define COLOR_RESET "\033[0m"

// Maximum number of files and users
#define MAX_FILES 1000
#define MAX_USERS 1000

// Struct to hold request information
typedef struct {
    int user_id;
    int file_id;
    char operation[10];
    int arrival_time;
} Request;

// File control structure
typedef struct {
    int is_deleted;
    int read_count;
    int visitor_count;
    sem_t access_control;
    pthread_mutex_t mutex;
} FileControl;

// Simulation parameters
int r_time, w_time, d_time;
int num_files, concurrency_limit, max_wait_time;
int current_time = 0;
FileControl files[MAX_FILES];
Request requests[MAX_USERS];
int num_requests = 0;
int stop_simulation = 0;

// Mutex to control access to simulation time
pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to simulate time passage in real-time
void *time_keeper(void *arg) {
    while (!stop_simulation) {
        sleep(1);
        pthread_mutex_lock(&time_mutex);
        current_time++;
        pthread_mutex_unlock(&time_mutex);
    }
    return NULL;
}

// Helper function to print colored output
void print_color(const char *color, const char *message) {
    printf("%s%s%s\n", color, message, COLOR_RESET);
}

// Helper function to delay based on operation type
void delay_operation(const char *operation) {
    if (strcmp(operation, "READ") == 0)
        sleep(r_time);
    else if (strcmp(operation, "WRITE") == 0)
        sleep(w_time);
    else if (strcmp(operation, "DELETE") == 0)
        sleep(d_time);
}

// Function to handle requests for each user
void *handle_request(void *arg) {
    Request *req = (Request *)arg;

    // Wait until current_time matches request's arrival time
    while (1) {
        pthread_mutex_lock(&time_mutex);
        int current = current_time;
        pthread_mutex_unlock(&time_mutex);
        if (current >= req->arrival_time) break;
        usleep(1000); // Small delay to reduce busy-waiting
    }

    // Print the request arrival message
    char msg[100];
    snprintf(msg, sizeof(msg), "User %d has made request for performing %s on file %d at %d seconds", 
             req->user_id, req->operation, req->file_id, req->arrival_time);
    print_color(COLOR_YELLOW, msg);

    // Wait for 1 second before processing
    sleep(1);

    // Check if the request should be cancelled
    pthread_mutex_lock(&time_mutex);
    int current = current_time;
    pthread_mutex_unlock(&time_mutex);
    if (current - req->arrival_time >= max_wait_time) {
        snprintf(msg, sizeof(msg), "User %d canceled the request due to no response at %d seconds.", 
                 req->user_id, current);
        print_color(COLOR_RED, msg);
        return NULL;
    }

    // Check if file is invalid
    if (req->file_id >= num_files || files[req->file_id].is_deleted) {
        snprintf(msg, sizeof(msg), "LAZY has declined the request of User %d at %d seconds because an invalid/deleted file was requested.", 
                 req->user_id, req->arrival_time);
        print_color(COLOR_WHITE, msg);
        return NULL;
    }

    FileControl *file = &files[req->file_id];

    // Process request based on operation type
    if (strcmp(req->operation, "READ") == 0) {
        pthread_mutex_lock(&file->mutex);
        // Check if there is a writer
        while (file->visitor_count >= concurrency_limit || (file->read_count == 0 && file->visitor_count > 0)) {
            pthread_mutex_unlock(&file->mutex);
            pthread_mutex_lock(&time_mutex);
            current = current_time;
            pthread_mutex_unlock(&time_mutex);
            if (current - req->arrival_time >= max_wait_time) {
                snprintf(msg, sizeof(msg), "User %d canceled the request due to no response at %d seconds.", 
                         req->user_id, current);
                print_color(COLOR_RED, msg);
                return NULL;
            }
            usleep(1000); // Wait and check again
            pthread_mutex_lock(&file->mutex);
        }

        // Allow the new reader
        file->visitor_count++;
        file->read_count++; // Increment read count
        pthread_mutex_unlock(&file->mutex);

        snprintf(msg, sizeof(msg), "LAZY has taken up the request of User %d at %d seconds for READ operation.", req->user_id, current);
        print_color(COLOR_PINK, msg);

        // Simulate read operation
        delay_operation("READ");

        pthread_mutex_lock(&file->mutex);
        file->read_count--;
        file->visitor_count--; // Decrement visitor count
        if (file->read_count == 0) {
            sem_post(&file->access_control); // Last reader unlocks the file
        }
        pthread_mutex_unlock(&file->mutex);

    } else if (strcmp(req->operation, "WRITE") == 0) {
        // Wait for the write operation to be available
        while (file->visitor_count > 0) {
            pthread_mutex_lock(&time_mutex);
            current = current_time;
            pthread_mutex_unlock(&time_mutex);
            if (current - req->arrival_time >= max_wait_time) {
                snprintf(msg, sizeof(msg), "User %d canceled the request due to no response at %d seconds.", 
                         req->user_id, current);
                print_color(COLOR_RED, msg);
                return NULL;
            }
            usleep(1000); // Wait and check again
        }

        // Now we can safely perform the WRITE operation
        sem_wait(&file->access_control);

        // Print message indicating the WRITE request is being processed
        snprintf(msg, sizeof(msg), "LAZY has taken up the request of User %d at %d seconds for WRITE operation.", req->user_id, current);
        print_color(COLOR_PINK, msg);

        // Simulate write operation
        delay_operation("WRITE");

        sem_post(&file->access_control);

    } else if (strcmp(req->operation, "DELETE") == 0) {
        // Wait for the delete operation to be available
        while (file->read_count > 0 || file->visitor_count > 0 || sem_trywait(&file->access_control) != 0) {
            pthread_mutex_lock(&time_mutex);
            current = current_time;
            pthread_mutex_unlock(&time_mutex);
            if (current - req->arrival_time >= max_wait_time) {
                snprintf(msg, sizeof(msg), "User %d canceled the request due to no response at %d seconds.", 
                         req->user_id, current);
                print_color(COLOR_RED, msg);
                return NULL;
            }
            usleep(1000); // Wait and check again
        }

        // Print message indicating the DELETE request is being processed
        snprintf(msg, sizeof(msg), "LAZY has taken up the request of User %d at %d seconds for DELETE operation.", req->user_id, current);
        print_color(COLOR_PINK, msg);

        // Simulate delete operation
        delay_operation("DELETE");

        file->is_deleted = 1;
        sem_post(&file->access_control);
    }

    // Print completion message with synchronized current time
    pthread_mutex_lock(&time_mutex);
    int completion_time = current_time;
    pthread_mutex_unlock(&time_mutex);
    snprintf(msg, sizeof(msg), "The request for User %d was completed at %d seconds", req->user_id, completion_time);
    print_color(COLOR_GREEN, msg);

    return NULL;
}

// Function to initialize file controls
void initialize_files() {
    for (int i = 0; i < num_files; i++) {
        files[i].is_deleted = 0;
        files[i].read_count = 0;
        files[i].visitor_count = 0; // Initialize visitor count
        sem_init(&files[i].access_control, 0, 1); // Only one write at a time
        pthread_mutex_init(&files[i].mutex, NULL);
    }
}

// Main function
int main() {
    // Input parameters
    scanf("%d %d %d", &r_time, &w_time, &d_time);
    scanf("%d %d %d", &num_files, &concurrency_limit, &max_wait_time);

    // Read requests
    while (1) {
        int user_id, file_id, arrival_time;
        char operation[10];
        
        if(scanf("%d %d %s %d", &user_id,&file_id, operation, &arrival_time)!=4){
            break;
        }

        requests[num_requests++] = (Request){user_id, file_id, "", arrival_time};
        strcpy(requests[num_requests - 1].operation, operation);
    }

    // Initialize files
    initialize_files();

    // Start time_keeper thread
    pthread_t time_thread;
    pthread_create(&time_thread, NULL, time_keeper, NULL);

    // Process each request
    pthread_t request_threads[MAX_USERS];
    for (int i = 0; i < num_requests; i++) {
        pthread_create(&request_threads[i], NULL, handle_request, &requests[i]);
    }

    // Join all threads
    for (int i = 0; i < num_requests; i++) {
        pthread_join(request_threads[i], NULL);
    }

    // Stop the simulation
    stop_simulation = 1;
    pthread_join(time_thread, NULL);

    // Final message
    print_color(COLOR_YELLOW, "LAZY has no more pending requests and is going back to sleep!");
    
    return 0;
}