#include "utils.h"

/**
 * @brief Validates the command line arguments, handles flags both short and long flags for buffer size, filename or drive name.
 *
 * @param args The place where all the arguments and relavent information will be stored.
 * @param argc The argc passed by main
 * @param argv The argv passed by main
 */
void validate_args(cl_args* args, int argc, char* argv[]) {
    // Setting up the required and compatible command line arguments
    // they support both long and short order arguments like (--buffer 1024 or -b 1024) are equivalent.
    struct option options[] = {
        {.name = "buffer", .has_arg = required_argument, NULL, .val = 'b' },    // For the setting of buffer length
        {.name = "file", .has_arg = required_argument, NULL, .val = 'f' },      // For providing the dumpname/image file from which images will be extracted
        {.name = "drive", .has_arg = required_argument, NULL, .val = 'd' },     // For providing the Drive that needs to parsed for deleted images
        {.name = "help", .has_arg = no_argument, NULL, .val = 'h'}              // Help option
    };

    args->buffer_size = MIN_BUFFER_SIZE;    // Setting the default buffer size of MIN_BUFFER_SIZE
    char ch;                                // Character for storing the current command line character
    bool method_selected = false;           // Checks if either the file or the drive methods have been set
    while ((ch = getopt_long(argc, argv, "b:f:d:h", options, NULL)) != -1) {     // Defining the arguments
        switch (ch) {

        case 'b':   // For buffer
            args->buffer_size = atoi(optarg);           // Converts the buffer_size to an integer
            if (args->buffer_size < MIN_BUFFER_SIZE) {  // If the buffer size < MIN_BUFFER_SIZE then it needs to change to a higher value
                usage();                // Prints the usage
                exit(EXIT_FAILURE);     // Exits with status code EXIT_FAILURE or non-zero.
            }
            break;

        case 'f':   // For filename
            // If no method is selected yet
            if (!method_selected) {
                strip(optarg);                                  // Stripping the filename of any whitespace
                strncpy(args->filename, optarg, FILENAME_MAX);  // Copying the string to the args->filename
                method_selected = true;                         // Setting the method_selected to true
                args->mode = MODE_FILE;                         // Setting the mode in which the file or drive will be read
            }
            break;

        case 'd':   // For drive
            if (!method_selected) {                             // Stripping the filename of any whitespace
                strip(optarg);                                  // Copying the string to the args->filename
                strncpy(args->drivename, optarg, DRIVE_MAX);    // Copies the drivename args->drivename
                args->drivename[1] = ':';                       // Ensuring the Drive is in the correct order.
                method_selected = true;                         // Setting the method_selected to true
                args->mode = MODE_DRIVE;                        // Setting the mode in which the file or drive will be read
            }
            break;
        case 'h':   // For printing the help
        default:
            usage();                                            // If nothing correct is selected then it prints the usage and exits.
            exit(EXIT_FAILURE);
        }
    }

    // If no correct option is selected then print usage and exit.
    if (!method_selected) {
        usage();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Prints the usage of the program. The Usage string is in utils.h file.
 *
 */
void usage() {
    printf("%s\n", USAGE_STR);
}

/**
* @brief Returns the size of the file in bytes
* @param file The pointer to the file whose size if to be found.
* @return Returns the size.
*/
size_t get_file_size(FILE* file) {
    size_t initial_pt = ftell(file);        // Gets the initial location of the file. i.e. The start
    fseek(file, 0L, SEEK_END);              // Moves to the end
    size_t file_size = ftell(file);         // Gets the byte-number there, this is the size of the file.

    fseek(file, initial_pt, SEEK_SET);      // Goes back to the start of the file.

    return file_size;                       // Returns the file size
}

/**
* @brief Creates a file with filename as the name in the current working directory
* @param filename The filename of the file to be created
* @param mode The mode the mode in which the file must be created
*/
void create_file(char* filename, char* mode) {
    FILE* file = fopen(filename, mode); // Creates a file with a `filename` filename and in a `mode` mode
    fclose(file);                       // Closes the file.
}

/**
* @brief Appends a single character at the end of the file.
* @param character The character itself
* @param filename The filename of the file to which it must be appended
* @param mode The mode in which the file must be created.
*/
void append_char_to_file(byte_t character, char* filename, char* mode) {
    FILE* file = fopen(filename, mode); // Opens the file in the `mode` mode
    putc(character, file);              // Appends `character` to the end of the file
    fclose(file);                       // Closes the file.
}

/**
* @brief Generates a filename considering the file extension and current file number
*
*/
void generate_filename(int file_count, char* ext, char* filename_holder) {
    sprintf(filename_holder, "%03d.%s", file_count, ext); // Genertes the filename depending upon the total count of the images recovered
}

/**
* @brief Returns true if PNG file header is found
* @param block The block where to look for the block
* @param i The current iteration from which to start
* @return Returns true if a header is found else returns false
*/
bool is_PNG_header(byte_t* block, int i) {
    return block[i] == 0x89 &&
        block[i + 1] == 0x50 &&
        block[i + 2] == 0x4E &&
        block[i + 3] == 0x47 &&
        block[i + 4] == 0x0D &&
        block[i + 5] == 0x0A &&
        block[i + 6] == 0x1A &&
        block[i + 7] == 0x0A;
}

/**
 * @brief Checks if the current if the block at index i is the start of a PNG trailer.
 *
 * @param block The current buffer block
 * @param i The current index of iteration of the block
 * @return true
 * @return false
 */
bool is_PNG_trailer(byte_t* block, int i) {
    return block[i] == 0x49 &&
        block[i + 1] == 0x45 &&
        block[i + 2] == 0x4E &&
        block[i + 3] == 0x44 &&
        block[i + 4] == 0xAE &&
        block[i + 5] == 0x42 &&
        block[i + 6] == 0x60 &&
        block[i + 7] == 0x82;
}

/**
 * @brief Stores the PNG trailer object in the trailer_holder array
 *
 * @param trailer_holder The place where the trailer will be stored.
 */
void get_PNG_trailer(byte_t trailer_holder[PNG_TRAILER_SIZE]) {
    byte_t PNG_trailer[] = { 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82 };

    // Setting the trailer of PNG in the trailer holder array.
    for (int i = 0; i < PNG_TRAILER_SIZE; ++i) {
        trailer_holder[i] = PNG_trailer[i];
    }
}

/**
* @brief Returns true if JPEG file header is found
* @param block The block where to look for the block
* @param i The current iteration from which to start
* @return Returns true if a header is found else returns false
*/
bool is_JPEG_header(byte_t* block, int i) {
    return block[i] == 0xFF &&
        block[i + 1] == 0xD8 &&
        block[i + 2] == 0xFF &&
        (block[i + 3] & 0xF0) == 0xE0;
}

/**
 * @brief Checks if the current if the block at index i is the start of a JPEG trailer.
 *
 * @param block The current buffer block
 * @param i The current index of iteration of the block
 * @return true
 * @return false
 */
bool is_JPEG_trailer(byte_t* block, int i) {
    return block[i] == 0xff &&
        block[i + 1] == 0xd9;
}

/**
 * @brief Stores the JPEG trailer object in the trailer_holder array
 *
 * @param trailer_holder The place where the trailer will be stored.
 */
void get_JPEG_trailer(byte_t trailer_holder[JPEG_TRAILER_SIZE]) {
    byte_t JPEG_trailer[] = { 0xFF, 0xD9 };

    // Setting the trailer of JPEG in the trailer holder array.
    for (int i = 0; i < JPEG_TRAILER_SIZE; ++i) {
        trailer_holder[i] = JPEG_trailer[i];
    }
}

/**
* @brief Returns true if GIF file header is found
* @param block The block where to look for the block
* @param i The current iteration from which to start
* @return Returns true if a header is found else returns false
*/
bool is_GIF_header(byte_t* block, int i) {
    return block[i] == 0x47 &&
        block[i + 1] == 0x49 &&
        block[i + 2] == 0x46 &&
        block[i + 3] == 0x38 &&
        (block[i + 4] == 0x37 || block[i + 4] == 0x39) &&
        block[i + 5] == 0x61;
}

/**
 * @brief Checks if the current if the block at index i is the start of a GIF trailer.
 *
 * @param block The current buffer block
 * @param i The current index of iteration of the block
 * @return true
 * @return false
 */
bool is_GIF_trailer(byte_t* block, int i) {
    return block[i] == 0x00 &&
        block[i + 1] == 0x3B;

}

/**
 * @brief Stores the GIF trailer object in the trailer_holder array
 *
 * @param trailer_holder The place where the trailer will be stored.
 */
void get_GIF_trailer(byte_t trailer_holder[GIF_TRAILER_SIZE]) {
    byte_t GIF_trailer[] = { 0x00, 0x3B };

    // Setting the trailer of GIF in the trailer holder array.
    for (int i = 0; i < GIF_TRAILER_SIZE; ++i) {
        trailer_holder[i] = GIF_trailer[i];
    }
}

/**
 * @brief Strips the current string of all the whitespaces from the right direction
 *
 * @param string The string to be stripped
 */
void rstrip(char string[]) {
    // Iterating the string in reverse order
    for (int n = strlen(string), i = n - 1; i >= 0; i--) {
        if (iswspace(string[i])) {
            string[i] = '\0';   // Setting the whitespaces to NULL characters
        }
        else { break; }         // Breaks when it first finds a non-whitespace character from the right
    }
}

/**
 * @brief Strips the current string of all the whitespaces from the left direction
 *
 * @param string The string to be stripped
 */
void lstrip(char string[]) {
    int length = strlen(string);    // Getting the string's lenght
    int whitespace_count = 0;       // Variable for counting the whitespaces in the start of the string
    // Iterating over the string character by character
    for (int i = 0; i < length; i++) {
        if (iswspace(string[i])) {
            whitespace_count++;     // Counting the whitespace from the left
        }
        else { break; }             // Breaks when it finds the first non-whitespace character
    }

    // Shifting the non whitespace characters to the start of the string where the whitespace used to be 
    for (int i = 0; i < length - whitespace_count; i++) {
        string[i] = string[i + whitespace_count];
    }

    // Setting the last `whitespace_count` number of character to NULL
    for (int i = length; i >= length - whitespace_count;i--) {
        string[i] = '\0';
    }
}

/**
 * @brief Strips the string from both the left and right direction
 *
 * @param string The subject string
 */
void strip(char string[]) {
    lstrip(string); // For left strip
    rstrip(string); // For right strip
}
