#include "header.h"
#include "ipcs.h"
/*
  Osaid Hamza - Team Leader
  Razan Abdelrahman
  Maisam Alaa
  Ansam Rihan
*/
//***********************************************************************************
void checkArguments(int argc, char **argv, char *file_name);
void signal_handler_SIGALRM(int sig);
void signal_handler_SIGCLD(int sig);
void init_signals_handlers();
void getArguments(int *numberArray);
void printArguments();
void initializeIPCResources();
void exitProgram();
void createGUI();
void createLiquidProductionLines();
void createPillProductionLines();
//***********************************************************************************
int arr_of_arguments[MAX_LINES];
int is_alarmed = 0;
int x = 0;
int size = 0;
int i = 0;
int pid = 0;
int is_end = 0;
int msg_gui;
struct msgbuf msg;

// arrays of pids for all the processes
pid_t pids_liquid_production_lines[MAX_NUM_LIQUID_PRODUCTION_LINES];
pid_t pids_pill_production_lines[MAX_NUM_PILL_PRODUCTION_LINES];

// arrays of structs for all the processes
Liquid_Production_Line liquid_production_lines[MAX_NUM_LIQUID_PRODUCTION_LINES];
Pill_Production_Line pill_production_lines[MAX_NUM_PILL_PRODUCTION_LINES];

// arguments from the file
char production_line_num[10];
char str_num_employees[10];
char str_range_num_midicines[20];
char str_range_of_speeds[20];
char str_range_level_liquid_medicine[20];
char str_range_color_liquid_medicine[20];
char str_num_plastic_containers[10];
char str_num_pills[10];
char str_range_size_pill[20];
char str_range_color_pill[20];
char num_of_liquid_production_lines[10];

// IPCs resources

// message queues

// shared memories for struct of processes
char *shmptr_liquid_production_lines;
// shared memories for thresholds

// semaphores
int sem_liquid_production_lines;

int main(int argc, char **argv)
{
    char *file_name = (char *)malloc(50 * sizeof(char));

    // Start the program
    printf("*******************************************\nStart the program, My process ID is %d\n\n", getpid());

    // check a number of arguments,and read a file name
    checkArguments(argc, argv, file_name);

    // to read from User defined numbers file (filename.txt)
    readFromFile(file_name, arr_of_arguments);

    // get the arguments from the file
    getArguments(arr_of_arguments);
    printArguments();

    // initialize IPCs resources (shared memory, semaphores, message queues)
    initializeIPCResources();
    init_signals_handlers();
    alarm(simulation_threshold_time);

    //  create the GUI
    // createGUI();
    createLiquidProductionLines();
    // createPillProductionLines();

    while (1)
    {
        pause();
        is_end = 0;
        if (is_alarmed)
        {
            printf("The trishold time is reached, the program is finished.\n\n");
            is_end = 1;
        }
        if (is_end)
        {
            break;
        }
    }

    exitProgram();

    return 0;
}

void init_signals_handlers()
{
    if (sigset(SIGALRM, signal_handler_SIGALRM) == -1)
    { // set the signal handler for SIGALRM
        perror("Signal Error\n");
        exit(-1);
    }
    // if (sigset(SIGCLD, signal_handler_SIGCLD) == -1)
    // { // set the signal handler for SIGALRM
    //     perror("Signal Error\n");
    //     exit(-1);
    // }
}

// function signal_handler_SIGALRM
void signal_handler_SIGALRM(int sig)
{
    is_alarmed = 1;
    printf("The signal %d reached the parent\n\n", sig);
    fflush(stdout);
}

void createLiquidProductionLines()
{

    for (i = 0; i < num_liquid_production_lines; i++)
    {
        switch (pid = fork())
        {

        case -1: // Fork Failed
            perror("Error:Fork liquid production line Failed.\n");
            exit(1);
            break;

        case 0: // liquid production line
            sprintf(production_line_num, "%d", i + 1);
            sprintf(num_of_liquid_production_lines, "%d", num_liquid_production_lines);
            sprintf(str_num_employees, "%d", num_employees);
            sprintf(str_range_num_midicines, "%d %d", range_of_liquid_medicines[0], range_of_liquid_medicines[1]);
            sprintf(str_range_of_speeds, "%d %d", range_speed_lines[0], range_speed_lines[1]);

            sprintf(str_range_level_liquid_medicine, "%d %d", range_level_liquid_medicine[0], range_level_liquid_medicine[1]);
            sprintf(str_range_color_liquid_medicine, "%d %d", range_color_liquid_medicine[0], range_color_liquid_medicine[1]);

            execlp("./liquid_production_line", "liquid_production_line", production_line_num, num_of_liquid_production_lines, str_num_employees, str_range_num_midicines, str_range_of_speeds, str_range_level_liquid_medicine, str_range_color_liquid_medicine, NULL);
            perror("Error:Execute liquid production line Failed.\n");
            exit(1);
            break;

        default: // I'm parent
            pids_liquid_production_lines[i] = pid;
            break;
        }
    }
}

void createPillProductionLines()
{

    for (i = 0; i < num_pill_production_lines; i++)
    {
        switch (pid = fork())
        {

        case -1: // Fork Failed
            perror("Error:Fork Plane Failed.\n");
            exit(1);
            break;

        case 0: // I'm plane
            sprintf(str_range_of_speeds, "%d %d", range_speed_lines[0], range_speed_lines[1]);
            sprintf(str_num_employees, "%d", num_employees);
            sprintf(str_range_num_midicines, "%d %d", range_of_pill_medicines[0], range_of_pill_medicines[1]);
            sprintf(production_line_num, "%d", i + 1);

            sprintf(str_num_plastic_containers, "%d %d", range_of_plastic_containers[0], range_of_plastic_containers[1]);
            sprintf(str_num_pills, "%d %d", range_of_pills[0], range_of_pills[1]);

            sprintf(str_range_size_pill, "%d %d", range_level_liquid_medicine[0], range_level_liquid_medicine[1]);
            sprintf(str_range_color_pill, "%d %d", range_color_liquid_medicine[0], range_color_liquid_medicine[1]);

            execlp("./pill_production_line", "pill_production_line", production_line_num, str_num_employees, str_range_num_midicines, str_range_of_speeds, str_num_plastic_containers, str_num_pills, str_range_size_pill, str_range_color_pill, NULL);
            perror("Error:Execute plane Failed.\n");
            exit(1);
            break;

        default: // I'm parent
            pids_pill_production_lines[i] = pid;
            break;
        }
    }
}
/*
function to create gui process
*/
void createGUI()
{
    switch (pid = fork())
    {
    case -1: // Fork Failed
        perror("Error:Fork GUI Failed.\n");
        exit(1);
        break;

    case 0: // I'm GUI
        execlp("./gui", "gui", NULL);
        perror("Error:Execute GUI Failed.\n");
        exit(1);
        break;

    default: // I'm parent
        break;
    }
}
/*
function to initialize IPCs resources (shared memory, semaphores, message queues)
*/
void initializeIPCResources()
{
    // Create a Massage Queues (2 queues done)
    // msg_ground = createMessageQueue(MSGQKEY_GROUND, "parent.c");       // Create a massage queue for the ground

    // Create a Shared Memories for struct of processes (4 shared memories done)
    shmptr_liquid_production_lines = createSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, num_liquid_production_lines * sizeof(struct Liquid_Production_Line), "parent.c");

    // Create a Shared Memories for thresholds (5 shared memories done)
    // shmptr_threshold_num_cargo_planes_crashed = createSharedMemory(SHKEY_THRESHOLD_NUM_CARGO_PLANES_CRASHED, sizeof(int), "parent.c");

    // Copy the the shared memories
    // memcpy(shmptr_plane, planes, num_cargo_planes * sizeof(struct Plane));                                                               // Copy the struct of all planes to the shared memory

    // Copy the the shared memories for thresholds
    // memcpy(shmptr_threshold_num_cargo_planes_crashed, &x, sizeof(int));

    // Create a Semaphores
    sem_liquid_production_lines = createSemaphore(SEMKEY_LIQUID_PRODUCTION_LINES, 1, 1, "parent.c");
}

void getArguments(int *numberArray)
{
    num_liquid_production_lines = numberArray[0];
    num_pill_production_lines = numberArray[1];
    num_employees = numberArray[2];
    threshold_of_num_liquid_medicines_produced = numberArray[3];
    threshold_of_num_pill_medicines_produced = numberArray[4];
    threshold_of_num_liquid_medicines_failed = numberArray[5];
    threshold_of_num_pill_medicines_failed = numberArray[6];
    simulation_threshold_time = numberArray[7];
}

void printArguments()
{
    // print the arguments read from the file
    printf("The arguments read from the file are:\n");
    printf("Number of liquid production lines: %d\n", num_liquid_production_lines);
    printf("Number of pill production lines: %d\n", num_pill_production_lines);
    printf("Number of employees: %d\n", num_employees);
    printf("Range of speed lins: %d - %d\n", range_speed_lines[0], range_speed_lines[1]);
    printf("Range of liquid medicines: %d - %d\n", range_of_liquid_medicines[0], range_of_liquid_medicines[1]);
    printf("Range of pill medicines: %d - %d\n", range_of_pill_medicines[0], range_of_pill_medicines[1]);
    printf("Range of plastic containers: %d - %d\n", range_of_plastic_containers[0], range_of_plastic_containers[1]);
    printf("Range of pills: %d - %d\n", range_of_pills[0], range_of_pills[1]);
    printf("Range of level liquid medicines: %d - %d\n", range_level_liquid_medicine[0], range_level_liquid_medicine[1]);
    printf("Range of color liquid medicines: %d - %d\n", range_color_liquid_medicine[0], range_color_liquid_medicine[1]);
    printf("Range of size pill: %d - %d\n", range_size_pill[0], range_size_pill[1]);
    printf("Range of color pill: %d - %d\n", range_color_pill[0], range_color_pill[1]);

    printf("Threshold of number of liquid medicines produced: %d\n", threshold_of_num_liquid_medicines_produced);
    printf("Threshold of number of pill medicines produced: %d\n", threshold_of_num_pill_medicines_produced);
    printf("Threshold of number of liquid medicines failed: %d\n", threshold_of_num_liquid_medicines_failed);
    printf("Threshold of number of pill medicines failed: %d\n", threshold_of_num_pill_medicines_failed);
    printf("Simulation threshold time: %d\n", simulation_threshold_time);
    printf("\n");
}

// function checkArguments
void checkArguments(int argc, char **argv, char *file_name)

{
    if (argc != 2) // check if the user passed the correct arguments
    {
        printf("Usage: Invalid arguments.\n"); // Use the default file names
        printf("Using default file names: arguments.txt\n");
        strcpy(file_name, FILE_NAME);
    }
    else
    {
        strcpy(file_name, argv[1]); // Use the file names provided by the user
    }
}

void exitProgram()
{

    printf("\nKilling all processes...\n");
    fflush(stdout);

    // kill all the child processes
    killAllProcesses(pids_liquid_production_lines, num_liquid_production_lines);
    // killAllProcesses(pids_pill_production_lines, num_pill_production_lines);

    printf("All child processes killed\n");

    printf("Cleaning up IPC resources...\n");
    fflush(stdout);

    deleteSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, num_liquid_production_lines * sizeof(struct Liquid_Production_Line), shmptr_liquid_production_lines);

    deleteSemaphore(sem_liquid_production_lines);

    printf("IPC resources cleaned up successfully\n");
    printf("Exiting...\n");
    fflush(stdout);
    exit(0);
}