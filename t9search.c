#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define LINE_SIZE 102
#define MAX_CONTACTS 42
#define ERROR_MESSAGE_SIZE 100
#define CONTIGUOUS_SEARCH 0
#define NOT_FOUND_MESSAGE "Not found"
#define FOUND_MESSAGE "Kontakt(y) nalezen(y)"

int out_i = 0 ;

typedef struct Contact {
    char name[LINE_SIZE];
    char phone_number[LINE_SIZE];
} Contact;

typedef struct ProcessingResult {
    int exit_code;
    size_t length;
    Contact arr[MAX_CONTACTS];
} ProcessingResult;

typedef struct Arguments
{
    char pattern[LINE_SIZE];
    int error_threshold;
} Arguments;


char *NUMBERS_TRANSLATION[] = {
    "0+",
    "1",
    "2abc",
    "3def",
    "4ghi",
    "5jkl",
    "6mno",
    "7pqrs",
    "8tuv",
    "9wxyz"
};

size_t get_lev_distance(char *str1, char *str2);
int match_contact(char *str, char *substr, int contiguous, int error_threshold);
int match_string(char *str, char combinations[][LINE_SIZE], int contiguous, int error_threshold);

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

int min(int a, int b, int c) {
    if (a < b) {
        return a < c ? a : c;
    } 

    return b < c ? b : c;
}

/*
    Loads contacts from stdin to contacts array
*/
void process_contacts(char *pattern, ProcessingResult *result, int contiguous, int error_threshold) {
    char name_buf[LINE_SIZE];
    char phone_buf[LINE_SIZE];

    result->exit_code = EXIT_SUCCESS;
    result->length = 0;

    Contact arr[MAX_CONTACTS] = { 0 };
    memcpy(result->arr, arr, MAX_CONTACTS);

    for (int i = 0; i < MAX_CONTACTS; i++) {
        int status;

        if ((status = parse_line(name_buf)) != EXIT_SUCCESS) {
            if (status == EXIT_FAILURE) {
                result->exit_code = EXIT_FAILURE;
            }

            return;
        }

        if ((status = parse_line(phone_buf)) != EXIT_SUCCESS) {
            if (status == EXIT_FAILURE) {
                result->exit_code = EXIT_FAILURE;
            }

            return;
        }

        if (
            *pattern == '\0' ||
            match_contact(pattern, name_buf, contiguous, error_threshold) || 
            match_contact(pattern, phone_buf, contiguous, error_threshold)
            ) {
                Contact contact;

                strcpy(contact.name, name_buf);
                strcpy(contact.phone_number, phone_buf);

                result->arr[result->length++] = contact;
        }
    }
    return;
}

void print_result(ProcessingResult *result) {
    for (size_t i = 0; i < result->length; i++)
    {
        printf("%s, %s\n", result->arr[i].name, result->arr[i].phone_number);
    }
}

char *get_possible_chars(char char_digit) {
    int digit = char_digit - '0';
    return NUMBERS_TRANSLATION[digit];
}

int is_substring(char *str, char *substring) {
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        if (strchr(get_possible_chars(substring[0]), str[i])) {
            int found = 1;

            for (size_t j = 1; substring[j] != '\0'; j++)
            {
                if (!strchr(get_possible_chars(substring[j]), str[j + i])) {
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

void str_to_number(char *str, char *output) {
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '+') {
            output[i] = '0';
        } else if (str[i] == 'z') {
            output[i] = '9';
        } else if (isdigit(str[i])) {
            output[i] = str[i];
        } else if (isalpha(str[i])) {
            // Gets position in alphabet, divide by 3 to get groups for 3 chars, added 2 because 'a' is 2 and converting to char
            output[i] = ((((int)str[i]) - 97) / 3) + 2 + '0';
        } else {
            output[i] = '-';
        }
    }
}

int is_non_contiguous_substring(char *str, char *substring) {
    int i = 0, j = 0;

    while (str[i] != '\0' && substring[j] != '\0')
    {
        if (strchr(get_possible_chars(substring[j]), str[i])) {
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

// Calculates edit distance of two strings 
// source: https://en.wikipedia.org/wiki/Levenshtein_distance#Iterative_with_full_matrix
size_t get_lev_distance(char *str1, char *str2) {
    size_t m = strlen(str1) + 1, n = strlen(str2) + 1;

    int matrix[m][n];

    // Adding sequence of number for first row and column, representing position in strings
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
            // If position in str1 and str2 has same characters, then we add value of previous result
            // strchr(get_possible_chars(substring[j]), )
            if (str1[i - 1] == str2[j - 1]) {
                matrix[i][j] = matrix[i - 1][j - 1];
            } else {
                // We find minimum of three adjacent cells and adding 1
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

int parse_arguments(char *argv[], size_t n, Arguments *arguments) {
    arguments->error_threshold = -1;
    arguments->pattern[0] = '\0';
    char error_msg[ERROR_MESSAGE_SIZE] = { 0 };

    if (n > 1) {
        parse_number(argv[1], error_msg);

        if (strlen(error_msg) > 0) {
            fprintf(stderr, error_msg);
            return EXIT_FAILURE;
        }

        strcpy(arguments->pattern, argv[1]);
    }

    for (size_t i = 2; i < (size_t) n; i++)
    {
        if (strlen(argv[i]) < 2) {
            continue;
        }

        if (argv[i][0] == '-' && argv[i][1] == 'l') {
            if (i + 1 >= n) {
                fprintf(stderr, "You have to specify value for -l parameter.");
                return EXIT_FAILURE;
            }
            
            int error_threshold = parse_number(argv[i + 1], error_msg);

            if (strlen(error_msg) > 0) {
                fprintf(stderr, error_msg);
                return EXIT_FAILURE;
            }

            arguments->error_threshold = error_threshold;
            
            return EXIT_SUCCESS;
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    // Arguments arguments;

    // ProcessingResult result;

    // if (parse_arguments(argv, argc, &arguments) == EXIT_FAILURE) {
    //     return EXIT_FAILURE;
    // }

    // process_contacts(arguments.pattern, &result, CONTIGUOUS_SEARCH, arguments.error_threshold);
    // if (result.exit_code == EXIT_FAILURE) {
    //     return EXIT_FAILURE;
    // }

    // if (result.length == 0) {
    //     printf("%s\n", NOT_FOUND_MESSAGE);
    //     return EXIT_SUCCESS;
    // } else  {
    //     printf("%s\n", FOUND_MESSAGE);
    // }

    // print_result(&result);

    // return EXIT_SUCCESS;

    // TODO prvni a ctvrty test na druhe rozsireni bad
    // TODO checknout jak realne funguje process contact s eofem

    char *x = "fewwef";
    printf("%s\n",x);
    x = "rrwrewr";
    printf("%s\n",x);
    (void)x;

}


int main(int argc, char *argv[]) {
    int x = 10;

    printf("Hello world \n");

    return EXIT_SUCCESS;
}

