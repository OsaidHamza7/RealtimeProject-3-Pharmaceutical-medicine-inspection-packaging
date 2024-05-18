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
void createPlanes();
void getArguments(int *numberArray);
void printArguments();
void initializeIPCResources();
void exitProgram();
void createGUI();
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

char production_line_num[2];
// arrays of pids for all the processes
pid_t pids_production_lines_liquid[MAX_NUM_PRODUCTION_LINES_LIQUID];
pid_t pids_production_lines_pill[MAX_NUM_PRODUCTION_LINES_PILL];

// arrays of structs for all the processes
Liquid_Production_Line production_lines_liquid[MAX_NUM_PRODUCTION_LINES_LIQUID];
Pill_Production_Line production_lines_pill[MAX_NUM_PRODUCTION_LINES_PILL];

// arguments from the file
int num_liquid_production_lines = 0;
int num_pill_production_lines = 0;
int num_employees;
int threshold_of_num_liquid_medicines_produced;
int threshold_of_num_pill_medicines_produced;
int threshold_of_num_liquid_medicines_failed;
int threshold_of_num_pill_medicines_failed;
int simulation_threshold_time;

int range_of_liquid_medicines[2];
int range_of_pill_medicines[2];
int range_of_plastic_containers[2];
int range_of_pills[2];
int range_speed_lines[2];
// IPCs resources

// message queues

// shared memories for struct of processes

// shared memories for thresholds

// semaphores

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
    // initializeIPCResources();
    init_signals_handlers();
    alarm(simulation_threshold_time);

    //  create the GUI
    // createGUI();
    createLiquidProductionLines();
    createPillProductionLines();

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

    // exitProgram();

    return 0;
}

void init_signals_handlers()
{
    if (sigset(SIGALRM, signal_handler_SIGALRM) == -1)
    { // set the signal handler for SIGALRM
        perror("Signal Error\n");
        exit(-1);
    }
    if (sigset(SIGCLD, signal_handler_SIGCLD) == -1)
    { // set the signal handler for SIGALRM
        perror("Signal Error\n");
        exit(-1);
    }
}

// function signal_handler_SIGALRM
void signal_handler_SIGALRM(int sig)
{
    is_alarmed = 1;
    printf("The signal %d reached the parent\n\n", sig);
    fflush(stdout);
}

void signal_handler_SIGCLD(int sig)
{
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
            perror("Error:Fork Plane Failed.\n");
            exit(1);
            break;

        case 0: // I'm plane
            sprintf(str_range_of_speeds, "%d %d", range_of_speed[0], range_of_speed[1]);
            sprintf(str_num_employees, "%d", num_employees);
            sprintf(str_range_num_midicines, "%d %d", range_of_liquid_medicines);

            sprintf(production_line_num, "%d", i + 1);

            execlp("./liquid_production_line", "liquid_production_line", production_line_num, str_num_employees, str_range_num_midicines, str_range_of_speed, NULL);
            perror("Error:Execute plane Failed.\n");
            exit(1);
            break;

        default: // I'm parent
            pids_production_lines_liquid = pid;
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
    msg_ground = createMessageQueue(MSGQKEY_GROUND, "parent.c");       // Create a massage queue for the ground
    msg_safe_area = createMessageQueue(MSGQKEY_SAFE_AREA, "parent.c"); // Create a massage queue for the safe storage area

    // Create a Shared Memories for struct of processes (4 shared memories done)
    shmptr_plane = createSharedMemory(SHKEY_PLANES, num_cargo_planes * sizeof(struct Plane), "parent.c");                                                        // Create a shared memory for all struct planes
    shmptr_collecting_committees = createSharedMemory(SHKEY_COLLECTION_COMMITTEES, num_collecting_relief_committees * sizeof(Collecting_Committee), "parent.c"); // Create a shared memory for all struct of the collecting committees
    shmptr_splitting_workers = createSharedMemory(SHKEY_SPLITTING_WORKERS, num_splitting_relief_workers * sizeof(Splitting_Worker), "parent.c");                 // Create a shared memory for all struct of the splitting workers
    shmptr_splitted_bages = createSharedMemory(SHKEY_SPLITTED_BAGS, sizeof(Container), "parent.c");                                                              // Create a shared memory for splitted bages
    shmptr_distributing_workers = createSharedMemory(SHKEY_DISTRIBUTING_WORKERS, num_distributing_relief_workers * sizeof(Distributing_Worker), "parent.c");
    shmptr_families = createSharedMemory(SHKEY_FAMILIES, num_families * sizeof(struct Family), "parent.c");

    // Create a Shared Memories for thresholds (5 shared memories done)
    shmptr_threshold_num_cargo_planes_crashed = createSharedMemory(SHKEY_THRESHOLD_NUM_CARGO_PLANES_CRASHED, sizeof(int), "parent.c");
    shmptr_threshold_wheat_flour_containers_shoted = createSharedMemory(SHKEY_THRESHOLD_WHEAT_FLOUR_CONTAINERS_SHOTED, sizeof(int), "parent.c");
    shmptr_threshold_martyred_collecting_committee_workers = createSharedMemory(SHKEY_THRESHOLD_MARTYRED_COLLECTING_COMMITTEE, sizeof(int), "parent.c");
    shmptr_threshold_martyred_distributing_workers = createSharedMemory(SHKEY_THRESHOLD_MARTYRED_DISTRIBUTING_WORKERS, sizeof(int), "parent.c");
    shmptr_threshold_num_deceased_families = createSharedMemory(SHKEY_THRESHOLD_NUM_DECEASED_FAMILIES, sizeof(int), "parent.c");

    // Copy the the shared memories
    memcpy(shmptr_plane, planes, num_cargo_planes * sizeof(struct Plane));                                                               // Copy the struct of all planes to the shared memory
    memcpy(shmptr_collecting_committees, collecting_committees, num_collecting_relief_committees * sizeof(struct Collecting_Committee)); // Copy the struct of all planes to the shared memory
    memcpy(shmptr_splitting_workers, splitting_workers, num_splitting_relief_workers * sizeof(struct Splitting_Worker));                 // Copy the struct of all planes to the shared memory
    memcpy(shmptr_splitted_bages, &x, sizeof(Container));                                                                                // Copy the struct of all planes to the shared memory
    memcpy(shmptr_distributing_workers, distributing_workers, num_distributing_relief_workers * sizeof(struct Distributing_Worker));     // Copy the struct of all planes to the shared memory

    // Copy the the shared memories for thresholds
    memcpy(shmptr_threshold_num_cargo_planes_crashed, &x, sizeof(int));
    memcpy(shmptr_threshold_wheat_flour_containers_shoted, &x, sizeof(int));
    memcpy(shmptr_threshold_martyred_collecting_committee_workers, &x, sizeof(int));
    memcpy(shmptr_threshold_martyred_distributing_workers, &x, sizeof(int));
    memcpy(shmptr_threshold_num_deceased_families, &x, sizeof(int));

    // Create a Semaphores
    sem_planes = createSemaphore(SEMKEY_PLANES, 1, 1, "parent.c");
    sem_collecting_committees = createSemaphore(SEMKEY_COLLECTING_COMMITTEES, 1, 1, "parent.c");
    sem_splitted_bags = createSemaphore(SEMKEY_SPLITTED_BAGS, 1, 0, "parent.c");
    sem_spaces_available = createSemaphore(SEMKEY_SPACES_AVAILABLE, 1, 1, "parent.c");
    sem_distributing_workers = createSemaphore(SEMKEY_DISTRIBUTING_WORKERS, 1, 1, "parent.c");
    sem_starviation_familes = createSemaphore(SEMKEY_STARVATION_FAMILIES, 1, 1, "parent.c");
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
    printf("Range of liquid medicines: %d - %d\n", range_of_liquid_medicines[0], range_of_liquid_medicines[1]);
    printf("Range of pill medicines: %d - %d\n", range_of_pill_medicines[0], range_of_pill_medicines[1]);
    printf("Range of plastic containers: %d - %d\n", range_of_plastic_containers[0], range_of_plastic_containers[1]);
    printf("Range of pills: %d - %d\n", range_of_pills[0], range_of_pills[1]);

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
    killAllProcesses(arr_pids_planes, num_cargo_planes);
    killAllProcesses(arr_pids_occupation, number_of_occupations);
    killAllProcesses(arr_pids_collecting_committees, num_collecting_relief_committees);
    killAllProcesses(arr_pids_distributing_workers, num_distributing_relief_workers);
    killAllProcesses(arr_pids_splitting_workers, num_splitting_relief_workers);
    killAllProcesses(arr_pids_families, 1);
    printf("All child processes killed\n");

    printf("Cleaning up IPC resources...\n");
    fflush(stdout);

    deleteMessageQueue(msg_ground);
    deleteMessageQueue(msg_safe_area);

    deleteSharedMemory(SHKEY_PLANES, num_cargo_planes * sizeof(struct Plane), shmptr_plane);
    deleteSharedMemory(SHKEY_COLLECTION_COMMITTEES, num_collecting_relief_committees * sizeof(struct Collecting_Committee), shmptr_collecting_committees);
    deleteSharedMemory(SHKEY_SPLITTED_BAGS, sizeof(Container), shmptr_splitted_bages);
    deleteSharedMemory(SHKEY_DISTRIBUTING_WORKERS, num_distributing_relief_workers * sizeof(struct Distributing_Worker), shmptr_distributing_workers);
    deleteSharedMemory(SHKEY_FAMILIES, num_families * sizeof(struct Family), shmptr_families);

    deleteSemaphore(sem_splitted_bags);
    deleteSemaphore(sem_spaces_available);
    deleteSemaphore(sem_planes);
    deleteSemaphore(sem_collecting_committees);
    deleteSemaphore(sem_distributing_workers);
    deleteSemaphore(sem_starviation_familes);

    printf("IPC resources cleaned up successfully\n");
    printf("Exiting...\n");
    fflush(stdout);
    exit(0);
}