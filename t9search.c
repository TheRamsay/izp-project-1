#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_SIZE 42
#define MAX_CONTACTS 100
#define CONTIUGOUS_SEARCH 0
#define NOT_FOUND_MESSAGE "Not found"
#define FOUND_MESSAGE "Kontakt(y) nalezen(y)"

int out_i = 0 ;

typedef struct Contact
{
    char name[LINE_SIZE];
    char phone_number[LINE_SIZE];
} Contact;

char *NUMBERS_TRANSLATION[] = {
    "+",
    "",
    "abc",
    "def",
    "ghi",
    "jkl",
    "mno",
    "pqrs",
    "tuv",
    "wxyz"
};

size_t get_lev_distance(char *str1, char *str2);
int is_substring(char *str, char *substr, int contiguous, size_t error_threshold);

void to_lower(char *str) {
    char *str_ptr = str;
    while (*str_ptr != '\0')
    {
        *str_ptr = tolower(*str_ptr);
        str_ptr++;
    }

    return;
}

int parse_line(char *str) {
    char *ptr;

    if ((ptr = strchr(str, '\n')) == NULL) {
        fprintf(stderr, "Line is too long");
        return EXIT_FAILURE;
    }

    str[strcspn(str, "\n")] = 0;
    to_lower(str);

    return 0;
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
void load_contacts(Contact arr[]) {
    char name_buf[LINE_SIZE];
    char phone_buf[LINE_SIZE];

    for (int i = 0; i < MAX_CONTACTS; i++) {
        Contact contact = {0};

        if (fgets(name_buf, LINE_SIZE, stdin) == NULL || fgets(phone_buf, LINE_SIZE, stdin) == NULL) {
            return;
        }

        parse_line(name_buf);
        parse_line(phone_buf);
        
        strcpy(contact.name, name_buf);
        strcpy(contact.phone_number, phone_buf);

        arr[i] = contact;
    }
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

size_t find_contacts(char *pattern, Contact input[], Contact output[], int contiguous, size_t error_threshold) {
    size_t idx = 0;
    int printed[MAX_CONTACTS] = { 0 };

    for (size_t i = 0; i < MAX_CONTACTS; i++)
    {
        if (strcmp(input[i].name, "\0") == 0) {
            break;
        }

        if (is_substring(input[i].phone_number, pattern, contiguous, error_threshold)) {
            output[idx++] = input[i];
        }

    }

    char buffer[LINE_SIZE] = {0};
    char combinations[1000][LINE_SIZE] = {0};

    generate_substrings(pattern, 0, buffer, combinations);

    for (size_t i = 0; i < MAX_CONTACTS; i++)
    {
        if (strcmp(input[i].name, "\0") == 0) {
            break;
        }

        for (size_t j = 0; j < 1000; j++)
        {

            if (strcmp(combinations[j], "\0") == 0) {
                break;
            }

            char *str1 = input[i].name, *str2 = combinations[j];

            if (is_substring(str1, str2, contiguous, error_threshold)) {
                if (printed[i]++ == 0) {
                    output[idx++] = input[i];
                }
            }
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

int is_substring(char *str, char *substr, int contiguous, size_t error_threshold) {
    if (*substr == '\0') {
        return 1;
    }

    if (error_threshold == 0) {

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
    

    size_t lev_distance = get_lev_distance(substr, str);
    
    // Substracting lengths difference of two strings from edit distance, because we dont want to count that in
    return error_threshold >= lev_distance - (strlen(str) - strlen(substr));
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

int main(int argc, char *argv[]) {
    Contact contacts[MAX_CONTACTS] = { 0 };

    char *pattern = argc < 2 ? "" : argv[1];

    size_t error_threshold = 0;

    // printf("Pattern '%s'\n", pattern);
    Contact results[MAX_CONTACTS] = { 0 };

    load_contacts(contacts);
    size_t n = find_contacts(pattern, contacts, results, CONTIUGOUS_SEARCH, error_threshold);

    if (n == 0) {
        printf("%s\n", NOT_FOUND_MESSAGE);
        exit(0);
    } else  {
        printf("%s\n", FOUND_MESSAGE);
    }

    print_contacts(results);
}