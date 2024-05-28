#ifndef LIBRARIES
#define LIBRARIES

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdbool.h>
#include <GL/glut.h>
#include <math.h>

#define FILE_NAME "arguments.txt"

#define MSGKEY_GUI 1234
#define MAX_LINES 100
#define MAX_LINE_LENGTH 255
#define NUM_SEMAPHORES 2

#define MAX_NUM_LIQUID_PRODUCTION_LINES 100
#define MAX_NUM_PILL_PRODUCTION_LINES 100
#define MAX_NUM_LIQUID_MEDICINES 100
#define MAX_NUM_PILL_MEDICINES 100
#define MAX_NUM_PLASTIC_CONTAINERS 100
#define MAX_NUM_PILLS 100

#define SHKEY_LIQUID_PRODUCTION_LINES 1111
#define SHKEY_PILL_PRODUCTION_LINES 2222
#define SHKEY_NUM_LIQUID_MEDICINES_PRODUCED 1234
#define SHKEY_NUM_PILL_MEDICINES_PRODUCED 2345
#define SHKEY_NUM_LIQUID_MEDICINES_FAILED 3456
#define SHKEY_NUM_PILL_MEDICINES_FAILED 4567
#define SHKEY_NUM_LIQUID_MEDICINES_PACKAGED 5678
#define SHKEY_NUM_PILL_MEDICINES_PACKAGED 6789

#define SEMKEY_LIQUID_PRODUCTION_LINES 7777
#define SEMKEY_PILL_PRODUCTION_LINES 8888
#define SEMKEY_NUM_LIQUID_MEDICINES_PRODUCED 5678
#define SEMKEY_NUM_PILL_MEDICINES_PRODUCED 6789
#define SEMKEY_NUM_LIQUID_MEDICINES_FAILED 7890
#define SEMKEY_NUM_PILL_MEDICINES_FAILED 8901

#define START_YEAR 2025
#define END_YEAR 2030

typedef struct Date
{
    int year;
    int month;
    int day;
} Date;

struct msgbuf
{
    long mtype;
    int x; // x coordinate
    int y; // y coordinate
    int z; // z coordinate
    int r; // red color
    int g; // green color
    int b; // blue color
};

struct String
{
    char str[MAX_LINE_LENGTH];
};

typedef struct Pill
{
    int id;
    int color;
    int size;
} Pill;

typedef struct Plastic_Container
{
    int id;
    int num_pills;
    bool date_is_printed;
    Pill pills[MAX_NUM_PILLS];
} Plastic_Container;

typedef struct Liquid_Medicine
{
    int id;
    int production_line_num;
    int level;
    int color;
    bool is_sealed;
    bool is_label_placed;
    bool is_inspected;
    bool is_failed;
    bool is_medicine_placed;
    bool is_prescription_placed;
    bool date_is_printed;
    Date expiry_date;
} Liquid_Medicine;

typedef struct Pill_Medicine
{
    int id;
    int production_line_num;
    int num_plastic_containers;
    Plastic_Container plastic_containers[MAX_NUM_PLASTIC_CONTAINERS];
    bool is_inspected;
    bool is_failed;
    bool prescription_is_added;
    bool is_packaged;
    Date Expiry_date;
} Pill_Medicine;

typedef struct Pill_Production_Line
{
    pid_t pid;
    int num;
    int num_employes;
    int num_medicines;
    Pill_Medicine pill_medicines[MAX_NUM_PILL_MEDICINES];
    int speed;
} Pill_Production_Line;

typedef struct Liquid_Production_Line
{
    pid_t pid;
    int num;
    int num_employes;
    int num_medicines;
    int speed;
    Liquid_Medicine liquid_medicines[MAX_NUM_LIQUID_MEDICINES];
} Liquid_Production_Line;

// ===================================================================================
extern int num_liquid_production_lines;
extern int num_pill_production_lines;
extern int num_employees;
extern int range_speed_lines[2];
extern int range_of_liquid_medicines[2];
extern int range_of_pill_medicines[2];
extern int range_of_plastic_containers[2];
extern int range_of_pills[2];
extern int range_level_liquid_medicine[2];
extern int range_color_liquid_medicine[2];
extern int range_size_pill[2];
extern int range_color_pill[2];
extern int range_expected_liquid_medicine_level[2];
extern int range_expected_liquid_medicine_color[2];
extern int range_expected_pill_medicine_color[2];
extern int range_expected_pill_medicine_size[2];
// thresholds
extern int threshold_of_num_liquid_medicines_produced;
extern int threshold_of_num_pill_medicines_produced;
extern int threshold_of_num_liquid_medicines_failed;
extern int threshold_of_num_pill_medicines_failed;
extern int simulation_threshold_time;
// ====================================================================================
void readArgumentsFromFile(char *filename);
void readFromFile(const char *filename, int *array);
char *trim(char *str);
void killAllProcesses(int *arr_pid, int size);
void split_string(char *argv, int arr[]);
int get_random_number(int min, int max);
Date generate_random_date();
#endif