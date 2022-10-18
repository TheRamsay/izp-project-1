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

typedef struct Contact
{
    char name[LINE_SIZE];
    char phone_number[LINE_SIZE];
} Contact;

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
int is_substring(char *str, char *substr, int contiguous, int error_threshold);

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
int process_contacts(Contact arr[]) {
    char name_buf[LINE_SIZE];
    char phone_buf[LINE_SIZE];

    for (int i = 0; i < MAX_CONTACTS; i++) {
        Contact contact = {0};

        int result1 = parse_line(name_buf);
        int result2 = parse_line(phone_buf);

        if (result1 == EXIT_FAILURE || result2 == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }

        if (result1 == EOF || result2 == EOF) {
            return EXIT_SUCCESS;
        }
        
        strcpy(contact.name, name_buf);
        strcpy(contact.phone_number, phone_buf);

        arr[i] = contact;
    }

    return EXIT_SUCCESS;
}

void generate_substrings(char *sequence, int seq_idx, char buf[], char output[][LINE_SIZE]) {
    if (sequence[seq_idx] == '\0') {
        strcpy(output[out_i++], buf);
        return;
    }

    int digit = sequence[seq_idx] - '0';
    char *available_chars = NUMBERS_TRANSLATION[digit];

    for (size_t i = 0; available_chars[i] != '\0'; i++)
    {
        buf[seq_idx] = available_chars[i];
        generate_substrings(sequence, seq_idx + 1, buf, output);
        buf[seq_idx] = '\0';
    }
    
}

int match_string(char *str, char combinations[][LINE_SIZE], int contiguous, int error_threshold) {
    for (size_t i = 0; i < 1000; i++)
    {
        if (strcmp(combinations[i], "\0") == 0) {
            return 0;
        }

        if (is_substring(str, combinations[i], contiguous, error_threshold)) {
            return 1;
        }
    }

    return 0;
}

size_t find_contacts(char *pattern, Contact input[], Contact output[], int contiguous, int error_threshold) {
    size_t idx = 0;

    char buffer[LINE_SIZE] = {0};
    char combinations[1000][LINE_SIZE] = {0};

    generate_substrings(pattern, 0, buffer, combinations);

    for (size_t i = 0; i < MAX_CONTACTS; i++)
    {
        if (strcmp(input[i].name, "\0") == 0) {
            break;
        }

        if (
            match_string(input[i].name, combinations, contiguous, error_threshold) || 
            match_string(input[i].phone_number, combinations, contiguous, error_threshold)
            ) {
                output[idx++] = input[i];
        }

    }
    
    return idx;
}

void print_contacts(Contact arr[]) {
    for (size_t i = 0; i < MAX_CONTACTS; i++)
    {
        if (strcmp(arr[i].name, "\0") == 0) {
            return;
        }

        printf("%s, %s\n", arr[i].name, arr[i].phone_number);
    }
}

int is_substring(char *str, char *substr, int contiguous, int error_threshold) {
    if (*substr == '\0') {
        return 1;
    }

    if (error_threshold == -1) {

        if (contiguous) {
            return strstr(str, substr) != NULL;
        }

        size_t substr_idx = 0;

        for (size_t i = 0; str[i] != '\0'; i++)
        {
            if (substr_idx < strlen(substr) && str[i] == substr[substr_idx]) {
                substr_idx++;
            }
        }

        return substr_idx == strlen(substr);
    }
    

    int lev_distance = get_lev_distance(substr, str);
    
    // Substracting lengths difference of two strings from edit distance, because we dont want to count that in
    return error_threshold >= lev_distance - (int)(strlen(str) - strlen(substr));
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
    (void) argc;
    (void) argv;

    Contact contacts[MAX_CONTACTS] = { 0 };
    Contact results[MAX_CONTACTS] = { 0 };
    Arguments arguments;

    if (parse_arguments(argv, argc, &arguments) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (process_contacts(contacts) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    size_t n = find_contacts(arguments.pattern, contacts, results, CONTIGUOUS_SEARCH, arguments.error_threshold);

    if (n == 0) {
        printf("%s\n", NOT_FOUND_MESSAGE);
        return EXIT_SUCCESS;
    } else  {
        printf("%s\n", FOUND_MESSAGE);
    }

    print_contacts(results);

    return EXIT_SUCCESS;
}