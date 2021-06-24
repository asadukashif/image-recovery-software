#include "utils.h"
#include <windows.h>

#define FILE_TYPES_COUNT 3

void file_check(int iteration, bool* p_progress, char* file_ext, const int trailer_size, bool (*is_header)(byte_t*, int), bool (*is_trailer)(byte_t*, int), void (*get_trailer)(byte_t*));

// Defines the order in which file are being tested
enum {
    JPEG, PNG, GIF
};

// Keeps track of the file progresses in order above.
bool file_progresses[FILE_TYPES_COUNT] = { };

char* file_exts[FILE_TYPES_COUNT] = {
    "jpeg", "png", "gif"
};

// An array that keeps the track of trailer sizes 
int trailer_sizes[FILE_TYPES_COUNT] = {
    JPEG_TRAILER_SIZE, PNG_TRAILER_SIZE, GIF_TRAILER_SIZE
};

// An array of function pointer for checking the header order of different file types.
bool (*is_header_funcs[])(byte_t*, int) = {
    is_JPEG_header, is_PNG_header, is_GIF_header
};

// An array of function pointer for checking the trailer order of different file types.
bool (*is_trailer_funcs[])(byte_t*, int) = {
    is_JPEG_trailer, is_PNG_trailer, is_GIF_trailer
};

// An array of function pointers for getting the trailer functions
void (*get_trailer_funcs[])(byte_t*) = {
    get_JPEG_trailer, get_PNG_trailer, get_GIF_trailer
};

int file_count = 0;                          // Counts the file found.
char new_filename[FILENAME_MAX] = { };       // A place for holding the new filename generated
int BUFFER_SIZE;                             // The buffer size chosen by the user in command line args
byte_t* buffer;                              // The buffer itself where the data is stored for an iteration

int main(int argc, char* argv[]) {
    cl_args args;                       // Holds the commands line args
    validate_args(&args, argc, argv);   // Handles, validates and stores those command line args in args.

    BUFFER_SIZE = args.buffer_size;     // The buffer size 
    size_t object_size;                 // For storing the filename or the drive name in subject
    size_t bytes_read = 0L;

    // File Specific    
    FILE* file;                         // File pointer

    // Drive Specific
    HANDLE device = NULL;               // Drive handle 
    char drivepath[64] = {};            // Drive path
    int num_sectors = 5;                // Sectors offset

    switch (args.mode) {
    case MODE_FILE:     // In case of file mode is selected       
        file = fopen(args.filename, "rb");                  // Opens the file in read-bytes mode
        CHECK_OR_EXIT(file);                                // Checks if the pointer is not NULL
        object_size = get_file_size(file);                  // Gets the file size of the object
        break;
    case MODE_DRIVE:    // In case of drive mode
        sprintf(drivepath, "\\\\.\\%s", args.drivename);    // Generating the custom drivepath

        // Opening the file and readying it for access
        device = CreateFile(drivepath,                      // Drive to open
            GENERIC_READ,                                   // Access mode
            FILE_SHARE_READ | FILE_SHARE_WRITE,             // Share Mode
            NULL,                                           // Security Descriptor
            OPEN_EXISTING,                                  // How to create
            0,                                              // File attributes
            NULL);                                          // Handle to template

        // Checking if the drive was opened correctly
        if (device == INVALID_HANDLE_VALUE) {
            printf("Error opening the file: %lu\n", GetLastError());    // Printing the error code in case of error
            return EXIT_FAILURE;                                        // Exits the program with non-zero exit code.
        }

        // Setting the file pointer to `num_sectors` * SECTOR_SIZE bytes offset
        SetFilePointer(device, num_sectors * SECTOR_SIZE, NULL, FILE_BEGIN);
        object_size = GetFileSize(device, NULL);    // Getting the drive's total size
        break;
    }

    buffer = calloc(BUFFER_SIZE, sizeof(byte_t));   // Declaring BUFFER_SIZE bytes on the heap
    CHECK_OR_EXIT(buffer);                          // Checking if the pointer returned is not NULL


    // Prints the inital logs
    char drivename[] = "Drive ";
    char filename[] = "File  ";
    printf("\t\t--- Image Recovery Software ---\n");
    printf("Reading from '%s' with buffer size '%d' bytes\n",
        (args.mode == MODE_DRIVE) ? strcat(drivename, args.drivename) : strcat(filename, args.filename),
        BUFFER_SIZE);

    for (;;) {  // Infinite for loop                          
        switch (args.mode) {
        case MODE_FILE:     // File mode
            fread(buffer, BUFFER_SIZE, 1, file);    // Reading BUFFER_SIZE bytes from the file and storing it in buffer
            break;                                  // Exiting the switch statement
        case MODE_DRIVE:    // Drive mode
            if (!ReadFile(device, buffer, BUFFER_SIZE, NULL, NULL)) {       // Reading the BUFFER_SIZE bytes from the drive and storing it in buffer
                printf("Error reading the file: %lu\n", GetLastError());    // Printing the error if any
                break;                                                      // Exiting the switch statement
            }
        }
        // Iterating over each byte loaded in the buffer
        for (int i = 0; i < BUFFER_SIZE; i++) {
            // Checking for each type of file i.e JPEG, PNG and GIF
            for (int j = 0; j < FILE_TYPES_COUNT; j++) {
                // Passing the relavent function pointer and header files.
                file_check(i, &file_progresses[j], file_exts[j], trailer_sizes[j], is_header_funcs[j], is_trailer_funcs[j], get_trailer_funcs[j]);
            }
        }

        // If the bytes read > size of the object being read, then break the loop.
        if (bytes_read > object_size) { break; }
        bytes_read += BUFFER_SIZE;                          // Incrementing the bytes_read by BUFFER_SIZE
        memset(buffer, 0x0, BUFFER_SIZE * sizeof(byte_t));  // Setting the buffer back to 0
    }

    // Prints the total bytes read.
    printf("Ended reading the file %I64d bytes\n", bytes_read);
    fclose(file);   // Closes the file
    free(buffer);   // Frees the memory taken up by the buffer
}

void file_check(int iteration, bool* p_progress, char* file_ext, const int trailer_size, bool (*is_header)(byte_t*, int), bool (*is_trailer)(byte_t*, int), void (*get_trailer)(byte_t*)) {
    // Checks if the current byte is the start of any file type or
    // if theres already a file of the current type in progress
    if (is_header(buffer, iteration) || *p_progress) {
        // If no file is in the progress then it must be the start of the file
        if (!*p_progress) {
            printf("\nFound '%s' Header!\n", file_ext);             // Prints that a certain type of file has been found.
            generate_filename(file_count, file_ext, new_filename);  // Generates a filename for it.
            create_file(new_filename, "wb");                        // Creates a file for storing it.
            printf("Starting to write to %s\n", new_filename);      // Prints a few log messages

            file_count++;                                           // Increments the file_counter
            *p_progress = true;                                     // Setting the progress of the current file_type to true
        }

        // Checks if its a trailer of the current file type
        if (is_trailer(buffer, iteration)) {
            *p_progress = false;            // Sets its progress to false

            byte_t trailer[trailer_size];   // Creates an empty array to store the trailer.
            get_trailer(trailer);           // Gets the trailer content for the current file type.

            // Writes the trailer to the end of the file.
            for (int j = 0; j < trailer_size; j++) {
                append_char_to_file(trailer[j], new_filename, "ab");
            }
            printf("Ended Writing to %s\n", new_filename);  // Logs that the file is done being written
            memset(&new_filename[0], 0x0, FILENAME_MAX);    // Resetting the new filename to NULL
        }

        // If the file in the process of being written then appends the current byte to the end of the file.
        if (*p_progress) {
            append_char_to_file(buffer[iteration], new_filename, "ab");
        }
    }
}
