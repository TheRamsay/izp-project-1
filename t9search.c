#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define LINE_SIZE 102
#define MAX_CONTACTS 42
#define ERROR_MESSAGE_SIZE 100
#define NOT_FOUND_MESSAGE "Not found"
#define FOUND_MESSAGE "Kontakt(y) nalezen(y)"

typedef struct Arguments {
    char pattern[LINE_SIZE];
    int error_threshold;
    int contiguous_search;
} Arguments;

int min(int a, int b, int c) {
    if (a < b) {
        return a < c ? a : c;
    } 

    return b < c ? b : c;
}

// Calculates edit distance of two strings 
// source: https://en.wikipedia.org/wiki/Levenshtein_distance#Iterative_with_full_matrix
size_t get_lev_distance(char *str1, char *str2) {
    size_t m = strlen(str1) + 1, n = strlen(str2) + 1;

    int matrix[m][n];

    for (size_t i = 0; i < n; i++)
    {
        matrix[0][i] = i;
    }

    for (size_t i = 0; i < m; i++)
    {
        matrix[i][0] = i;
    }

    for (size_t i = 1; i < m; i++)
    {
        for (size_t j = 1; j < n; j++)
        {
            if (str1[i - 1] == str2[j - 1]) {
                matrix[i][j] = matrix[i - 1][j - 1];
            } else {
                matrix[i][j] = 1 + min(
                    matrix[i - 1][j - 1],
                    matrix[i - 1][j],
                    matrix[i][j - 1]
                );
            }
        }
    }

    return matrix[m - 1][n - 1];
}

int is_substring(char *str, char *substring) {
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == substring[0]) {
            int found = 1;

            for (size_t j = 1; substring[j] != '\0'; j++)
            {
                if (substring[j] != str[i + j]) {
                    found = 0;
                    break;
                }
            }

            if (found) {
                return 1;
            }
            
        }
    }
    
    return 0;
}

int is_non_contiguous_substring(char *str, char *substring) {
    int i = 0, j = 0;

    while (str[i] != '\0' && substring[j] != '\0')
    {
        if (str[i] == substring[j]) {
            j++;
        }

        i++;
    }

    return substring[j] == '\0';
}

int match_contact(char *pattern, char *contact_field, int contiguous, int error_threshold) {
    if (*pattern == '\0') {
        return 1;
    }
    
    if (error_threshold != -1) {
        int lev_distance = get_lev_distance(contact_field, pattern);
        
        // Substracting lengths difference of two strings from edit distance, because we dont want to count that in
        return error_threshold >= lev_distance - (int)(strlen(contact_field) - strlen(pattern));
    }

    return contiguous ? is_substring(contact_field, pattern) : is_non_contiguous_substring(contact_field, pattern);
}

void str_to_number(char *str, char *output) {
    size_t idx = 0;

    while (str[idx])
    {
        switch (str[idx]) {
        case 'y':
        case 'z':
            output[idx] = '9';
            break;
        case 's':
            output[idx] = '7';
            break;
        case 'v':
            output[idx] = '8';
            break;
        case '+':
            output[idx] = '0';
            break;
        default:
            if (isdigit(str[idx])) {
                output[idx] = str[idx];
            } else if (isalpha(str[idx])) {
                // Gets position in alphabet, divide by 3 to get groups for 3 chars, added 2 because 'a' is 2 and converting to char
                output[idx] = ((((int)str[idx]) - 97) / 3) + 2 + '0';
            } else {
                output[idx] = '-';
            }
            break;
        }

        idx++;
    }

    output[idx] = '\0';
}

void to_lower(char *str) {
    char *str_ptr = str;
    while (*str_ptr != '\0')
    {
        *str_ptr = tolower(*str_ptr);
        str_ptr++;
    }

    return;
}

int parse_line(char *buf) {
    if (fgets(buf, LINE_SIZE, stdin) == NULL) {
        return EOF;
    }

    if (strchr(buf, '\n') == NULL) {
        fprintf(stderr, "Line is too long.\n");
        return EXIT_FAILURE;
    }

    buf[strcspn(buf, "\n")] = '\0';
    to_lower(buf);

    if (strlen(buf) == 0) {
        fprintf(stderr, "Empty line is not a valid entry.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
    Loads contacts from stdin to contacts array
*/
int process_contacts(char *pattern, int contiguous, int error_threshold) {
    char name_buf[LINE_SIZE];
    char phone_buf[LINE_SIZE];
    char parsed_name[LINE_SIZE];
    char parsed_phone[LINE_SIZE];
    int n = 0;
    int return_code = EXIT_SUCCESS;
    
    for (int i = 0; i < MAX_CONTACTS; i++) {
        int status;

        if ((status = parse_line(name_buf)) != EXIT_SUCCESS) {
            return_code = status == EXIT_FAILURE ? EXIT_FAILURE : EXIT_SUCCESS;
            break;
        }

        if ((status = parse_line(phone_buf)) != EXIT_SUCCESS) {
            return_code = status == EXIT_FAILURE ? EXIT_FAILURE : EXIT_SUCCESS;
            break;
        }

        str_to_number(name_buf, parsed_name);
        str_to_number(phone_buf, parsed_phone);

        if (
            *pattern == '\0' ||
            match_contact(pattern, parsed_name, contiguous, error_threshold) || 
            match_contact(pattern, parsed_phone, contiguous, error_threshold)
            ) {
                n++;
                printf("%s, %s\n", name_buf, phone_buf);
        }
    }

    if (n == 0) {
        printf("%s\n", NOT_FOUND_MESSAGE);
    }
    
    return return_code;
}

long parse_number(char *str, char error_msg[ERROR_MESSAGE_SIZE]) {
    errno = 0;
    char *ptr;
    long error_threshold = strtol(str, &ptr, 10);

    if (*ptr != '\0') {
        strcpy(error_msg, "Value of argument has to be numerical.\n");
        return 0;
    }

    if (errno != 0) {
        strcpy(error_msg, "Error while parsing number\n");
        return 0;
    }

    return error_threshold;
}

int parse_arguments(char *argv[], size_t argc, Arguments *arguments) {
    arguments->error_threshold = -1;
    arguments->contiguous_search = 1;
    arguments->pattern[0] = '\0';

    int pattern_found = 0;
    char error_msg[ERROR_MESSAGE_SIZE] = { 0 };

    for (size_t i = 1; i < argc; i++)
    {
        switch (argv[i][0])
        {
        case '-':
            if (strlen(argv[i]) != 2) {
                fprintf(stderr, "Parameter is longer than expected.\n");
                return EXIT_FAILURE;
            }

            if (argv[i][1] == 's') {
                if (i != 1) {
                    fprintf(stderr, "Parameter -s has to be first.\n");
                    return EXIT_FAILURE;
                }

                arguments->contiguous_search = 0;
                break;

            } else if (argv[i][1] == 'l') {
                if (i + 1 >= argc) {
                    fprintf(stderr, "You have to specify value for -l parameter.\n");
                    return EXIT_FAILURE;
                }
                
                int error_threshold = parse_number(argv[i + 1], error_msg);

                if (strlen(error_msg) > 0) {
                    fprintf(stderr, error_msg);
                    return EXIT_FAILURE;
                }

                arguments->error_threshold = error_threshold;
                i++;
                break;

            } else {
                fprintf(stderr, "Unexptected parameter.\n");
                return EXIT_FAILURE;
            }
        
        default:
            if (pattern_found) {
                fprintf(stderr, "Unexptected parameter.\n");
                return EXIT_FAILURE;
            }

            parse_number(argv[i], error_msg);

            if (strlen(error_msg) > 0) {
                fprintf(stderr, error_msg);
                return EXIT_FAILURE;
            }

            // If pattern is longer than line size, then we create invalid pattern so no contact is matched
            if (strlen(argv[i]) > LINE_SIZE - 1) {
                strcpy(arguments->pattern, "|");
            } else {
                strcpy(arguments->pattern, argv[i]);
            }

            pattern_found = 1;
            break;
        }
    }
    

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    Arguments arguments;

    if (parse_arguments(argv, argc, &arguments) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (process_contacts(arguments.pattern, arguments.contiguous_search, arguments.error_threshold) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}