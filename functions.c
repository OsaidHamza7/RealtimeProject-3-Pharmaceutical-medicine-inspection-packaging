#include "header.h"

int num_liquid_production_lines = 5;
int num_pill_production_lines = 5;
int num_employees[2] = {1, 5};
int range_speed_lines[2] = {1, 5};
int range_of_liquid_medicines[2] = {1, 5};
int range_of_pill_medicines[2] = {1, 5};
int range_of_plastic_containers[2] = {1, 5};
int range_of_pills[2] = {1, 5};
int range_level_liquid_medicine[2] = {1, 5};
int range_color_liquid_medicine[2] = {1, 5};
int range_size_pill[2] = {1, 5};
int range_color_pill[2] = {1, 5};
int range_expected_liquid_medicine_level[2] = {1, 5};
int range_expected_liquid_medicine_color[2] = {1, 5};
int range_expected_pill_medicine_size[2] = {1, 5};
int range_expected_pill_medicine_color[2] = {1, 5};

int threshold_of_num_liquid_medicines_produced = 100;
int threshold_of_num_pill_medicines_produced = 100;
int threshold_of_num_liquid_medicines_failed = 10;
int threshold_of_num_pill_medicines_failed = 10;
int simulation_threshold_time = 60;

char tempLine[MAX_LINE_LENGTH];
char varName[MAX_LINE_LENGTH];
char valueStr[MAX_LINE_LENGTH];

void readFromFile(const char *filename, int *array)
{

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int numLines = 0;

    while (fgets(tempLine, sizeof(tempLine), file) != NULL)
    {
        // Split the line into variable name and value
        sscanf(tempLine, "%s %s", varName, valueStr);

        // Remove leading and trailing whitespaces from the variable name and value
        trim(varName);
        trim(valueStr);

        int min = 0, max = 0;
        // If the value is a multiple numbers, split them into min and max
        if (strstr(valueStr, ",") != NULL)
        {
            // If the value is a range, convert it to two integers
            sscanf(valueStr, "%d,%d", &min, &max);
        }
        else // If the value is a single number, convert it to an integer
        {
            array[numLines] = atoi(valueStr);
            numLines++;
        }

        // Assign values based on variable name
        if (strcmp(varName, "range_of_liquid_medicines") == 0)
        {
            range_of_liquid_medicines[0] = min;
            range_of_liquid_medicines[1] = max;
        }
        else if (strcmp(varName, "num_employees") == 0)
        {
            num_employees[0] = min;
            num_employees[1] = max;
        }
        else if (strcmp(varName, "range_of_pill_medicines") == 0)
        {
            range_of_pill_medicines[0] = min;
            range_of_pill_medicines[1] = max;
        }
        else if (strcmp(varName, "range_of_plastic_containers") == 0)
        {
            range_of_plastic_containers[0] = min;
            range_of_plastic_containers[1] = max;
        }
        else if (strcmp(varName, "range_of_pills") == 0)
        {
            range_of_pills[0] = min;
            range_of_pills[1] = max;
        }
        else if (strcmp(varName, "range_speed_lines") == 0)
        {
            range_speed_lines[0] = min;
            range_speed_lines[1] = max;
        }
        else if (strcmp(varName, "range_level_liquid_medicine") == 0)
        {
            range_level_liquid_medicine[0] = min;
            range_level_liquid_medicine[1] = max;
        }
        else if (strcmp(varName, "range_color_liquid_medicine") == 0)
        {
            range_color_liquid_medicine[0] = min;
            range_color_liquid_medicine[1] = max;
        }
        else if (strcmp(varName, "range_size_pill") == 0)
        {
            range_size_pill[0] = min;
            range_size_pill[1] = max;
        }
        else if (strcmp(varName, "range_color_pill") == 0)
        {
            range_color_pill[0] = min;
            range_color_pill[1] = max;
        }
        else if (strcmp(varName, "range_expected_liquid_medicine_level") == 0)
        {
            range_expected_liquid_medicine_level[0] = min;
            range_expected_liquid_medicine_level[1] = max;
        }
        else if (strcmp(varName, "range_expected_liquid_medicine_color") == 0)
        {
            range_expected_liquid_medicine_color[0] = min;
            range_expected_liquid_medicine_color[1] = max;
        }
        else if (strcmp(varName, "range_expected_pill_medicine_size") == 0)
        {
            range_expected_pill_medicine_size[0] = min;
            range_expected_pill_medicine_size[1] = max;
        }
        else if (strcmp(varName, "range_expected_pill_medicine_color") == 0)
        {
            range_expected_pill_medicine_color[0] = min;
            range_expected_pill_medicine_color[1] = max;
        }
    }
    fclose(file); // closing the file
}

void killAllProcesses(int *arr_pid, int size)
{
    printf("\nStart kill all processes\n");
    fflush(stdout);
    for (int i = 0; i < size; i++)
    {
        kill(arr_pid[i], SIGQUIT);
    }
}

void split_string(char *argv, int arr[])
{
    char *token = strtok(argv, " ");
    int i = 0;
    while (token != NULL)
    {
        arr[i] = atoi(token);
        token = strtok(NULL, " ");
        i++;
    }
}

int get_random_number(int min, int max)
{
    if (min > max)
    {
        printf("Error: min should be less than or equal to max.\n");
        return -1; // Return an error code
    }
    // Calculate the range and generate a random number within that range
    int range = max - min + 1;
    int randomNumber = rand() % range + min;

    return randomNumber;
}

// trim function
// this function removes the white spaces from the beginning and the end of a string
char *trim(char *str)
{
    while (*str && (*str == ' ' || *str == '\t' || *str == '\n'))
    {
        str++;
    }
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\n'))
    {
        len--;
    }
    str[len] = '\0';

    return str;
}

Date generate_random_date()
{
    // Seed the random number generator
    srand(time(NULL));

    // Generate a random year between START_YEAR and END_YEAR
    int year = START_YEAR + rand() % (END_YEAR - START_YEAR + 1);

    // Generate a random month between 1 and 12
    int month = 1 + rand() % 12;

    // Generate a random day based on the month and whether it's a leap year
    int day;
    switch (month)
    {
    case 2: // February
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        {
            day = 1 + rand() % 29; // Leap year
        }
        else
        {
            day = 1 + rand() % 28; // Non-leap year
        }
        break;
    case 4:
    case 6:
    case 9:
    case 11:
        day = 1 + rand() % 30; // April, June, September, November
        break;
    default:
        day = 1 + rand() % 31; // January, March, May, July, August, October, December
    }

    Date random_date = {year, month, day};
    return random_date;
}

int readLiquidMedicinesFromFile(char *liquid_medicine_filename, Liq_Med *liquid_medicines)
{
    // Open the products file
    FILE *file = fopen(liquid_medicine_filename, "r");
    if (file == NULL)
    {
        perror("Error opening the liquid medicine file");
        exit(1);
    }

    int i = 0;

    while (fgets(tempLine, sizeof(tempLine), file) != NULL && i < MAX_NUM_BOTTLES)
    {
        // Split the line into the product ID and the product name
        sscanf(tempLine, "%[^,], %d, %d, %d, %d", liquid_medicines[i].label.str, &liquid_medicines[i].min_level, &liquid_medicines[i].max_level, &liquid_medicines[i].min_color, &liquid_medicines[i].max_color);
       // printf("Liquid Medicine %d: %s, %d, %d, %d, %d\n", i + 1, liquid_medicines[i].label.str, liquid_medicines[i].min_level, liquid_medicines[i].max_level, liquid_medicines[i].min_color, liquid_medicines[i].max_color);
        i++;
    }
    fclose(file); // closing the file

    return i;
}

int readPillMedicinesFromFile(char *pill_medicine_filename, Pill_Med *pill_medicines)
{
    // Open the products file
    FILE *file = fopen(pill_medicine_filename, "r");
    if (file == NULL)
    {
        perror("Error opening the liquid medicine file");
        exit(1);
    }

    int i = 0;

    while (fgets(tempLine, sizeof(tempLine), file) != NULL && i < MAX_NUM_PILL_MEDICINES)
    {
        // Split the line into the product ID and the product name
        sscanf(tempLine, "%[^,], %d, %d, %d, %d, %d, %d", pill_medicines[i].label.str, &pill_medicines[i].num_containers, &pill_medicines[i].num_pills, &pill_medicines[i].min_size, &pill_medicines[i].max_size, &pill_medicines[i].min_color, &pill_medicines[i].max_color);
        //printf("Pill Medicine %d: %s, %d, %d, %d, %d, %d, %d\n", i + 1, pill_medicines[i].label.str, pill_medicines[i].num_containers, pill_medicines[i].num_pills, pill_medicines[i].min_size, pill_medicines[i].max_size, pill_medicines[i].min_color, pill_medicines[i].max_color);
        i++;
    }
    fclose(file); // closing the file

    return i;
}