#ifndef __UTILS_H__
#define __UTILS_H__

#include "getopt/getopt.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The Usage string, printed when called for help or incorrect command line args
#define USAGE_STR "Usage: ./recover.exe --filename <filename, usb.dmp> | --drive <drive, C:> --buffer <buffer_size, >=512> (optional)"

// Minimum and default buffer size.
#define MIN_BUFFER_SIZE 512

// Sector size
#define SECTOR_SIZE 512

// Different trailer sizes
#define PNG_TRAILER_SIZE 8
#define GIF_TRAILER_SIZE 2
#define JPEG_TRAILER_SIZE 2

// The max number of character for a drive name
#define DRIVE_MAX 4

// Macro for checking if the pointer is NULL then exit the program
#define CHECK_OR_EXIT(ptr)      \
    {                           \
        if (ptr == NULL)        \
            exit(EXIT_FAILURE); \
    }

typedef uint8_t byte_t;

// Recovery modes, either from an Image/Dump File or from a Drive directly
#define MODE_DRIVE 1
#define MODE_FILE 2

// A Struct for holding the command-line-args information
typedef struct cl_args
{
    int buffer_size;             // The buffer chosen from the user.
    char filename[FILENAME_MAX]; // The filename of the image/dump file.
    char drivename[DRIVE_MAX];   // The drive name if the drive option is selected
    int mode;                    // The mode in which the data is to be recovered (File or Drive)
} cl_args;

void validate_args(cl_args *args, int argc, char *argv[]);
size_t get_file_size(FILE *file);

void generate_filename(int file_count, char *ext, char *filename_holder);
void create_file(char *filename, char *mode);
void append_char_to_file(byte_t character, char *filename, char *mode);

// PNG functions
bool is_PNG_header(byte_t *block, int i);
bool is_PNG_trailer(byte_t *block, int i);
void get_PNG_trailer(byte_t trailer_holder[PNG_TRAILER_SIZE]);

// JPEG functions
bool is_JPEG_header(byte_t *block, int i);
bool is_JPEG_trailer(byte_t *block, int i);
void get_JPEG_trailer(byte_t trailer_holder[JPEG_TRAILER_SIZE]);

// GIF functions
bool is_GIF_header(byte_t *block, int i);
bool is_GIF_trailer(byte_t *block, int i);
void get_GIF_trailer(byte_t trailer_holder[GIF_TRAILER_SIZE]);

// Prints the usage function
void usage();
// Whitespace stripping functions
void lstrip(char string[]);
void rstrip(char string[]);
void strip(char string[]);

#endif //__UTILS_H__
