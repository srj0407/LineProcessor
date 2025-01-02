#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_LINES 50             // Maximum number of lines a buffer can hold
#define MAX_LINE_LENGTH 1000     // Maximum length of a single line
#define OUTPUT_LINE_LENGTH 80    // Length of an output line

// Buffer structure to manage communication between threads
typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH]; // Storage for lines
    int count;                              // Number of lines in the buffer
    int in;                                 // Index for the producer to insert a line
    int out;                                // Index for the consumer to retrieve a line
    pthread_mutex_t mutex;                  // Mutex for thread synchronization
    pthread_cond_t not_empty;               // Variable to check Buffer is not empty
    pthread_cond_t not_full;                // Variable to check Buffer is not full
} Buffer;

// Shared buffers between threads
Buffer buffer1, buffer2, buffer3;

// Function Prototypes
void initialize_buffer(Buffer *buffer);  // Initialize a buffer
void add_to_buffer(Buffer *buffer, const char *line); // Add a line to a buffer
void remove_from_buffer(Buffer *buffer, char *line);  // Remove a line from a buffer

// Thread functions
void *input_thread(void *arg);           // Input thread function
void *line_separator_thread(void *arg); // Line separator thread function
void *plus_sign_thread(void *arg);      // Plus sign processing thread function
void *output_thread(void *arg);         // Output thread function

int main() {
    // Initialize buffers for thread communication
    initialize_buffer(&buffer1);
    initialize_buffer(&buffer2);
    initialize_buffer(&buffer3);

    // Create threads for the pipeline
    pthread_t threads[4];
    pthread_create(&threads[0], NULL, input_thread, NULL);
    pthread_create(&threads[1], NULL, line_separator_thread, NULL);
    pthread_create(&threads[2], NULL, plus_sign_thread, NULL);
    pthread_create(&threads[3], NULL, output_thread, NULL);

    // Wait for all threads to complete
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

// Initialize a buffer
// Input: Pointer to a Buffer structure
// Output: None
// Purpose: Sets initial values for buffer variables and initializes mutex/condition variables.
void initialize_buffer(Buffer *buffer) {
    buffer->count = 0;
    buffer->in = 0;
    buffer->out = 0;
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
}

// Add a line to a buffer
// Input: Pointer to a Buffer and the line to add
// Output: None
// Purpose: Adds a line to the buffer, waits if the buffer is full.
void add_to_buffer(Buffer *buffer, const char *line) {
    pthread_mutex_lock(&buffer->mutex);

    // Wait if buffer is full
    while (buffer->count == MAX_LINES) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }

    // Add the line and update the buffer
    strcpy(buffer->lines[buffer->in], line);
    buffer->in = (buffer->in + 1) % MAX_LINES;
    buffer->count++;

    // Signal that the buffer is not empty
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
}

// Remove a line from a buffer
// Input: Pointer to a Buffer and a char array to store the removed line
// Output: None
// Purpose: Removes a line from the buffer, waits if the buffer is empty.
void remove_from_buffer(Buffer *buffer, char *line) {
    pthread_mutex_lock(&buffer->mutex);

    // Wait if buffer is empty
    while (buffer->count == 0) {
        pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
    }

    // Retrieve the line and update the buffer
    strcpy(line, buffer->lines[buffer->out]);
    buffer->out = (buffer->out + 1) % MAX_LINES;
    buffer->count--;

    // Signal that the buffer is not full
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->mutex);
}

// Input Thread: Reads input lines and places them into buffer1
// Input: None
// Output: None
// Purpose: Reads lines from stdin, stops at "STOP", and sends them to buffer1.
void *input_thread(void *arg) {
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(line, "STOP\n") == 0) break;
        add_to_buffer(&buffer1, line);
    }

    // Add a termination signal to buffer1
    add_to_buffer(&buffer1, "STOP");
    return NULL;
}

// Line Separator Thread: Replaces '\n' with space and adds to buffer2
// Input: None
// Output: None
// Purpose: Processes lines from buffer1 to replace '\n' with a space.
void *line_separator_thread(void *arg) {
    char line[MAX_LINE_LENGTH];
    while (true) {
        remove_from_buffer(&buffer1, line);

        if (strcmp(line, "STOP") == 0) {
            add_to_buffer(&buffer2, "STOP");
            break;
        }

        // Replace newline characters with spaces
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == '\n') {
                line[i] = ' ';
            }
        }

        add_to_buffer(&buffer2, line);
    }
    return NULL;
}

// Plus Sign Thread: Replaces "++" with "^" and adds to buffer3
// Input: None
// Output: None
// Purpose: Processes lines from buffer2 to replace "++" with "^".
void *plus_sign_thread(void *arg) {
    char line[MAX_LINE_LENGTH];
    while (true) {
        remove_from_buffer(&buffer2, line);

        if (strcmp(line, "STOP") == 0) {
            add_to_buffer(&buffer3, "STOP");
            break;
        }

        char processed_line[MAX_LINE_LENGTH];
        int j = 0;
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == '+' && line[i + 1] == '+') {
                processed_line[j++] = '^';
                i++; // Skip the next '+'
            } else {
                processed_line[j++] = line[i];
            }
        }
        processed_line[j] = '\0';

        add_to_buffer(&buffer3, processed_line);
    }
    return NULL;
}

// Output Thread: Outputs 80-character lines from buffer3
// Input: None
// Output: None
// Purpose: Processes lines from buffer3 to produce 80-character output lines.
void *output_thread(void *arg) {
    char line[MAX_LINE_LENGTH];
    char output_line[OUTPUT_LINE_LENGTH + 1]; // +1 for null terminator
    int output_index = 0;

    while (true) {
        remove_from_buffer(&buffer3, line);

        // Stop processing if the termination signal is encountered
        if (strcmp(line, "STOP") == 0) {
            break;
        }

        // Process characters from the line
        for (int i = 0; line[i] != '\0'; i++) {
            output_line[output_index++] = line[i];

            // Output the line when it reaches 80 characters
            if (output_index == OUTPUT_LINE_LENGTH) {
                output_line[OUTPUT_LINE_LENGTH] = '\0'; // Null-terminate
                printf("%s\n", output_line);
                output_index = 0;
            }
        }
    }

    // Do not flush remaining characters after STOP is encountered
    return NULL;
}
