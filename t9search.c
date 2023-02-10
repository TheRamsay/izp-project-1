/**
 * @name IZP Projekt 1 - Prace s textem
 * @author Dominik Huml - xhumld00
 * 2022
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define LINE_SIZE 102
#define NOT_FOUND_MESSAGE "Not found"
#define UNKNOWN_CHAR '-'
#define INVALID_PATTERN "|"

typedef struct Arguments {
    char pattern[LINE_SIZE];
    int error_threshold;
    bool contiguous_search;
} Arguments;

int min(int a, int b, int c) {
    if (a < b) {
        return a < c ? a : c;
    }

    return b < c ? b : c;
}

int raise_error(char *error_msg) {
    fprintf(stderr, "%s\n", error_msg);
    return EXIT_FAILURE;
}

int get_edit_distance(char *str1, char *str2) {
    int min_err_count = __INT_MAX__;
    for (size_t i = 0; str1[i] != '\0'; i++)
    {
        int err_count = 0;
        for (size_t j = 0; str2[j] != '\0'; j++)
        {
            if (str1[i + j] != str2[j]) {
                err_count += 1;
            }
        }

        if (err_count < min_err_count) {
            min_err_count = err_count;
        }
        
    }
    
    return min_err_count;
}

// Calculates edit distance of two strings
// source: https://en.wikipedia.org/wiki/Levenshtein_distance#Iterative_with_full_matrix
int get_lev_distance(char *str1, char *str2) {
    size_t m = strlen(str1) + 1, n = strlen(str2) + 1;

    int matrix[m][n];

    for (size_t i = 0; i < n; i++) {
        matrix[0][i] = i;
    }

    for (size_t i = 0; i < m; i++) {
        matrix[i][0] = i;
    }

    for (size_t i = 1; i < m; i++) {
        for (size_t j = 1; j < n; j++) {
            if (str1[i - 1] == str2[j - 1]) {
                matrix[i][j] = matrix[i - 1][j - 1];
            }
            else {
                matrix[i][j] = 1 + min(matrix[i - 1][j - 1], matrix[i - 1][j], matrix[i][j - 1]);
            }
        }
    }

    return matrix[m][n];
}

int non_contiguous_strstr(char *str, char *substring) {
    int i = 0, j = 0;

    while (str[i] != '\0' && substring[j] != '\0') {
        if (str[i] == substring[j]) {
            j++;
        }

        i++;
    }

    return substring[j] == '\0';
}

bool match_contact(char *pattern, char *contact_field, bool contiguous, int error_threshold) {
    if (*pattern == '\0') {
        return true;
    }

    if (error_threshold != -1) {
        // Search for contigous substring with some error threshold
        if (contiguous) {
            return error_threshold >= get_edit_distance(contact_field, pattern);
        }

        // Combination of -s and -l
        // Using levenshtein distance for finding uncontiguous substring with allowed erorrs
        // Substracting length difference of strings from edit distance, so we can ignore non pattern symbols 
        int lev_distance = get_lev_distance(contact_field, pattern);

        return error_threshold >= lev_distance - (int)(strlen(contact_field) - strlen(pattern));
    }

    return contiguous ? strstr(contact_field, pattern) != NULL : non_contiguous_strstr(contact_field, pattern);
}

void str_to_num_value(char *str, char *output) {
    size_t idx = 0;

    while (str[idx]) {
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
                }
                else if (isalpha(str[idx])) {
                    // Gets position in alphabet, divide by 3 to get groups for
                    // 3 chars, added 2 because 'a' is 2 and converting to char
                    output[idx] = ((((int)str[idx]) - 97) / 3) + 2 + '0';
                }
                else {
                    output[idx] = UNKNOWN_CHAR;
                }
                break;
        }

        idx++;
    }

    output[idx] = '\0';
}

void str_to_lower(char *str) {
    char *str_ptr = str;

    while (*str_ptr != '\0') {
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
        return raise_error("Line is too long.");
    }

    buf[strcspn(buf, "\n")] = '\0';
    str_to_lower(buf);

    if (strlen(buf) == 0) {
        return raise_error("Empty line is not a valid entry.");
    }

    return EXIT_SUCCESS;
}

int process_contacts(char *pattern, int contiguous, int error_threshold) {
    char name_buf[LINE_SIZE], phone_buf[LINE_SIZE], parsed_name[LINE_SIZE], parsed_phone[LINE_SIZE];
    int found_count = 0;
    int status_code = EXIT_SUCCESS;
    int status;

    while (true) {
        if ((status = parse_line(name_buf)) != EXIT_SUCCESS) {
            status_code = status == EXIT_FAILURE ? EXIT_FAILURE : EXIT_SUCCESS;
            break;
        }

        if ((status = parse_line(phone_buf)) != EXIT_SUCCESS) {
            status_code = status == EXIT_FAILURE ? EXIT_FAILURE : EXIT_SUCCESS;
            break;
        }

        str_to_num_value(name_buf, parsed_name);
        str_to_num_value(phone_buf, parsed_phone);

        if (
            match_contact(pattern, parsed_name, contiguous, error_threshold) ||
            match_contact(pattern, parsed_phone, contiguous, error_threshold)
        ) {
            found_count++;
            fprintf(stdout, "%s, %s\n", name_buf, phone_buf);
        }
    }

    if (status_code == EXIT_SUCCESS && found_count == 0) {
        fprintf(stdout, "%s\n", NOT_FOUND_MESSAGE);
    }

    return status_code;
}

bool is_number(char *str) {
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        if (!isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

int parse_arguments(char *argv[], size_t argc, Arguments *arguments) {
    arguments->error_threshold = -1;
    arguments->contiguous_search = true;
    arguments->pattern[0] = '\0';

    bool pattern_found = false;

    for (size_t i = 1; i < argc; i++) {
        switch (argv[i][0]) {
            case '-':
                if (strlen(argv[i]) != 2) {
                    return raise_error("Parameter is longer than expected.");
                }

                if (argv[i][1] == 's') {
                    if (i != 1) {
                        return raise_error("Parameter -s has to be first.");
                    }

                    arguments->contiguous_search = false;
                    break;
                } else if (argv[i][1] == 'l') {
                    if (i + 1 >= argc) {
                        return raise_error("You have to specify a value for -l parameter.");
                    }

                    if (!is_number(argv[i + 1])) {
                        return raise_error("Value of -l parameter has to be numerical.");
                    }

                    // If value of -l is triple or more digit number, set it to 100 cuz max pattern length is 100
                    if (strlen(argv[i + 1]) >= 3) {
                        arguments->error_threshold = 100;
                    } else {
                        arguments->error_threshold = strtol(argv[i + 1], NULL, 10);
                    }

                    i++;
                    break;
                } else {
                    return raise_error("Unexpected parameter.");
                }

            default:
                // If pattern was already found raise error
                if (pattern_found) {
                    return raise_error("Unexpected parameter.");
                }

                // Handles empty argument ./t9search ""
                if (argv[i][0] == '\0') {
                    return raise_error("Pattern can't be empty sequence.");
                }

                if (!is_number(argv[i])) {
                    return raise_error("Argument has to be numerical.");
                }

                // If pattern is longer than line size, then we create invalid
                // pattern so no contact is matched
                if (strlen(argv[i]) > LINE_SIZE - 1) {
                    strcpy(arguments->pattern, INVALID_PATTERN);
                } else {
                    strcpy(arguments->pattern, argv[i]);
                }

                pattern_found = true;
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