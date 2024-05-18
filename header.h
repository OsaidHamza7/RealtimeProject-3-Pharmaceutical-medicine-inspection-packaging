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

// ===================================================================================
// ====================================================================================
void readArgumentsFromFile(char *filename);
void readFromFile(const char *filename, int *array);
char *trim(char *str);
void killAllProcesses(int *arr_pid, int size);
void split_string(char *argv, int arr[]);
int get_random_number(int min, int max);

#endif