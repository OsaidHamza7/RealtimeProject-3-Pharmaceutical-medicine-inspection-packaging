#include "header.h"

int num_cargo_planes = 5;

void readFromFile(const char *filename, int *array)
{
    char tempLine[MAX_LINE_LENGTH];
    char varName[MAX_LINE_LENGTH];
    char valueStr[MAX_LINE_LENGTH];

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
        if (strcmp(varName, "range_num_wheat_flour_containers") == 0)
        {
            range_num_wheat_flour_containers[0] = min;
            range_num_wheat_flour_containers[1] = max;
        }
        else if (strcmp(varName, "range_num_bages") == 0)
        {
            range_num_bages[0] = min;
            range_num_bages[1] = max;
        }
        else if (strcmp(varName, "period_dropping_wheat_flour_container") == 0)
        {
            period_dropping_wheat_flour_container[0] = min;
            period_dropping_wheat_flour_container[1] = max;
        }
        else if (strcmp(varName, "period_refill_planes") == 0)
        {
            period_refill_planes[0] = min;
            period_refill_planes[1] = max;
        }
        else if (strcmp(varName, "range_bags_per_distrib_worker") == 0)
        {
            range_bags_per_distrib_worker[0] = min;
            range_bags_per_distrib_worker[1] = max;
        }
        else if (strcmp(varName, "period_trip_collecting_committees") == 0)
        {
            period_trip_collecting_committees[0] = min;
            period_trip_collecting_committees[1] = max;
        }
        else if (strcmp(varName, "range_energy_of_workers") == 0)
        {
            range_energy_of_workers[0] = min;
            range_energy_of_workers[1] = max;
        }
        else if (strcmp(varName, "energy_loss_range") == 0)
        {
            energy_loss_range[0] = min;
            energy_loss_range[1] = max;
        }
        else if (strcmp(varName, "range_starvation_increase") == 0)
        {
            range_starvation_increase[0] = min;
            range_starvation_increase[1] = max;
        }
        else if (strcmp(varName, "range_starvation_decrease") == 0)
        {
            range_starvation_decrease[0] = min;
            range_starvation_decrease[1] = max;
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