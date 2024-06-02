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
#define LIQUID_MEDICINES_FILE "liquid_medicines.txt"
#define PILL_MEDICINES_FILE "pill_based_medicines.txt"

#define MSGKEY_GUI 1234
#define MAX_LINES 100
#define MAX_LINE_LENGTH 255
#define NUM_SEMAPHORES 2

#define MAX_NUM_LIQUID_PRODUCTION_LINES 100
#define MAX_NUM_PILL_PRODUCTION_LINES 100
#define MAX_NUM_BOTTLES 100
#define MAX_NUM_PILL_MEDICINES 100
#define MAX_NUM_PLASTIC_CONTAINERS 100
#define MAX_NUM_PILLS 100

#define SHKEY_LIQUID_PRODUCTION_LINES 1111
#define SHKEY_PILL_PRODUCTION_LINES 2222
#define SHKEY_LIQUID_MEDICINES 3333

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

typedef struct String
{
    char str[MAX_LINE_LENGTH];
} String;

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
    bool is_date_printed;
    Pill pills[MAX_NUM_PILLS];
} Plastic_Container;

typedef struct Liquid_Medicine
{
    int level;
    int color;

} Liquid_Medicine;

typedef struct Bottle_Liquid_Medicine
{
    int id;
    String label;
    bool is_sealed;
    bool is_label_placed;
    bool is_date_printed;
    bool is_liquid_medicine_placed;
    bool is_prescription_placed;
    bool is_inspected;
    bool is_packaged;
    bool is_failed;
    Liquid_Medicine liquid_medicine;
    Date expiry_date;

} Bottle_Liquid_Medicine;
typedef struct Production_Line
{
    int id;
    int num_employes;
    int num_produced_medicines;
    int num_successful_medicines;
    int num_falied_medicines;
    int speed;

} Production_Line;

typedef struct Liquid_Production_Line
{
    pid_t pid;
    Production_Line production_line;
    Bottle_Liquid_Medicine bottles[MAX_NUM_BOTTLES];

} Liquid_Production_Line;

typedef struct Pill_Based_Medicine
{
    int id;
    int num_plastic_containers;
    bool is_inspected;
    bool is_failed;
    bool prescription_is_added;
    bool is_packaged;
    Plastic_Container plastic_containers[MAX_NUM_PLASTIC_CONTAINERS];
    Date Expiry_date;

} Pill_Medicine;

typedef struct Pill_Production_Line
{
    pid_t pid;
    Production_Line production_line;
    Pill_Medicine pill_medicines[MAX_NUM_PILL_MEDICINES];

} Pill_Production_Line;

typedef struct Liq_Med
{
    String label;
    int min_level;
    int max_level;
    int min_color;
    int max_color;

} Liq_Med;

// ===================================================================================
extern int num_liquid_production_lines;
extern int num_pill_production_lines;
extern int num_employees[2];
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
void readFromFile(const char *filename, int *array);
int readLiquidMedicinesFromFile(char *liquid_medicine_filename, Liq_Med *liquid_medicines);
char *trim(char *str);
void killAllProcesses(int *arr_pid, int size);
void split_string(char *argv, int arr[]);
int get_random_number(int min, int max);
Date generate_random_date();
#endif