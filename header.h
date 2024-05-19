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

typedef struct Plastic_Container
{
    int id;
    int num_pills;
    int pills[MAX_NUM_PILLS];
} Plastic_Container;

typedef struct Pill
{
    int id;
    int color;
    int size;
} Pill;

typedef struct Liquid_Medicine
{
    int id;
    int production_line_id;
    int level;
    int color;
    bool is_sealed;
    bool is_label_placed;
    int expiry_date;
} Liquid_Medicine;

typedef struct Pill_Medicine
{
    int id;
    int production_line_num;
    int num_plastic_containers;
    Plastic_Container plastic_containers[MAX_NUM_PLASTIC_CONTAINERS];
    int Expiry_date;
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
    Liquid_Medicine liquid_medicines[MAX_NUM_LIQUID_MEDICINES];
    int speed;
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

#endif