#include "header.h"
#include "ipcs.h"
/*
  Osaid Hamza - Team Leader
  Razan Abdelrahman
  Maisam Alaa
  Ansam Rihan
*/
//***********************************************************************************
void checkArguments(int argc, char **argv, char *file_name, char *file_liquid_medicines, char *file_pill_medicines);
void signal_handler_SIGALRM(int sig);
void signal_handler_SIGCLD(int sig);
void init_signals_handlers();
void getArguments(int *numberArray);
void initializeIPCResources();
void exitProgram();
void createGUI();
void createLiquidProductionLines();
void createPillProductionLines();
void thresholds_monitoring();
void speed_monitoring();
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
pid_t pid_gui[1];

// arrays of structs for all the processes
Liquid_Production_Line *liquid_production_lines;
Pill_Production_Line *pill_production_lines;

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
char str_num_pill_lines[10];
char str_range_expected_level[20];
char str_range_expected_color[20];
char str_range_expected_size_pill[20];
char str_range_expected_color_pill[20];
char str_num_liq_med[10];
char str_num_pill_med[10];

// IPCs resources

// message queues

// shared memories for struct of processes
char *shmptr_liquid_production_lines;
char *shmptr_pill_production_lines;
char *shmptr_num_liquid_medicines_produced;
char *shmptr_num_pill_medicines_produced;
char *shmptr_num_liquid_medicines_failed;
char *shmptr_num_pill_medicines_failed;
char *shmptr_num_liquid_medicines_packaged;
char *shmptr_num_pill_medicines_packaged;

char *shmptr_liquid_medicines;
char *shmptr_pill_medicines;

// shared memories for thresholds

// semaphores
int sem_liquid_production_lines;
int sem_pill_production_lines;
int sem_num_liquid_medicines_produced;
int sem_num_pill_medicines_produced;
int sem_num_liquid_medicines_failed;
int sem_num_pill_medicines_failed;
int sem_num_liquid_medicines_packaged;
int sem_num_pill_medicines_packaged;

pthread_t thresholds_monitoring_thread;
pthread_t speed_monitoring_thread;

Liq_Med liquid_medicines[MAX_NUM_BOTTLES];
Pill_Med pill_medicines[MAX_NUM_PILL_MEDICINES];

int num_liq_meds = 0;
int num_pill_meds = 0;

int main(int argc, char **argv)
{
    char *file_arguments = (char *)malloc(50 * sizeof(char));
    char *file_liquid_medicines = (char *)malloc(50 * sizeof(char));
    char *file_pill_medicines = (char *)malloc(50 * sizeof(char));

    printf("*******************************************\nStart the program, My process ID is %d\n\n", getpid());

    // check a number of arguments,and read a file names
    checkArguments(argc, argv, file_arguments, file_liquid_medicines, file_pill_medicines);

    // to read from User defined numbers file (filename.txt)
    readFromFile(file_arguments, arr_of_arguments);
    num_liq_meds = readLiquidMedicinesFromFile(file_liquid_medicines, liquid_medicines);
    num_pill_meds = readPillMedicinesFromFile(file_pill_medicines, pill_medicines);

    // get the arguments from the file
    getArguments(arr_of_arguments);

    // initialize IPCs resources (shared memory, semaphores, message queues)
    initializeIPCResources();
    init_signals_handlers();

    // craete a monitoring thread to keep track of the thresholds
    pthread_create(&thresholds_monitoring_thread, NULL, (void *)thresholds_monitoring, NULL);

    // custom attributes for the thread to increase the stack size
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 10000 * 10000);

    if (pthread_create(&speed_monitoring_thread, &attr, (void *)speed_monitoring, NULL) != 0)
    {
        perror("Error: pthread_create failed\n");
        exit(1);
    }

    // set an alarm for the simulation threshold time
    alarm(simulation_threshold_time);

    // create the processes (liquid production lines, pill production lines, GUI)
    createGUI();
    createLiquidProductionLines();
    createPillProductionLines();

    pthread_join(thresholds_monitoring_thread, NULL);
    pthread_cancel(speed_monitoring_thread);

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
}

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
            perror("Error:Fork Liquid Production Line Failed.\n");
            exit(1);
            break;

        case 0: // liquid production line
            sprintf(production_line_num, "%d", i + 1);
            sprintf(num_of_liquid_production_lines, "%d", num_liquid_production_lines);
            sprintf(str_num_employees, "%d %d", num_employees[0], num_employees[1]);
            sprintf(str_range_of_speeds, "%d %d", range_speed_lines[0], range_speed_lines[1]);

            sprintf(str_range_level_liquid_medicine, "%d %d", range_level_liquid_medicine[0], range_level_liquid_medicine[1]);
            sprintf(str_range_color_liquid_medicine, "%d %d", range_color_liquid_medicine[0], range_color_liquid_medicine[1]);

            sprintf(str_range_expected_level, "%d %d", range_expected_liquid_medicine_level[0], range_expected_liquid_medicine_level[1]);
            sprintf(str_range_expected_color, "%d %d", range_expected_liquid_medicine_color[0], range_expected_liquid_medicine_color[1]);
            sprintf(str_num_liq_med, "%d", num_liq_meds);

            execlp("./liquid_production_line", "liquid_production_line", production_line_num, num_of_liquid_production_lines, str_num_employees, str_range_of_speeds, str_range_level_liquid_medicine, str_range_color_liquid_medicine, str_range_expected_level, str_range_expected_color, str_num_liq_med, NULL);
            perror("Error:Execute Liquid Production Line Failed.\n");
            exit(1);
            break;

        default: // I'm parent
            pids_liquid_production_lines[i] = pid;
            break;
        }
        sleep(1);
    }
}

void createPillProductionLines()
{
    for (i = 0; i < num_pill_production_lines; i++)
    {
        switch (pid = fork())
        {

        case -1: // Fork Failed
            perror("Error:Fork Pill Production Line Failed.\n");
            exit(1);
            break;

        case 0: // I'm Pill Production Line
            sprintf(production_line_num, "%d", i + 1);
            sprintf(str_num_pill_lines, "%d", num_pill_production_lines);
            sprintf(str_num_employees, "%d %d", num_employees[0], num_employees[1]);
            sprintf(str_range_num_midicines, "%d %d", range_of_pill_medicines[0], range_of_pill_medicines[1]);
            sprintf(str_range_of_speeds, "%d %d", range_speed_lines[0], range_speed_lines[1]);

            sprintf(str_num_plastic_containers, "%d %d", range_of_plastic_containers[0], range_of_plastic_containers[1]);
            sprintf(str_num_pills, "%d %d", range_of_pills[0], range_of_pills[1]);

            sprintf(str_range_size_pill, "%d %d", range_size_pill[0], range_size_pill[1]);
            sprintf(str_range_color_pill, "%d %d", range_color_pill[0], range_color_pill[1]);

            sprintf(str_range_expected_size_pill, " %d %d", range_expected_pill_medicine_size[0], range_expected_pill_medicine_size[1]);
            sprintf(str_range_expected_color_pill, " %d %d", range_expected_pill_medicine_color[0], range_expected_pill_medicine_color[1]);
            sprintf(str_num_pill_med, "%d", num_pill_meds);

            execlp("./pill_production_line", "pill_production_line", production_line_num, str_num_pill_lines, str_num_employees, str_range_num_midicines, str_range_of_speeds, str_num_plastic_containers, str_num_pills, str_range_size_pill, str_range_color_pill, str_num_pill_med, NULL);
            perror("Error:Execute Pill Production Line Failed.\n");
            exit(1);
            break;

        default: // I'm parent
            pids_pill_production_lines[i] = pid;
            break;
        }
    }
}

void createGUI()
{
    switch (pid = fork())
    {
    case -1: // Fork Failed
        perror("Error:Fork GUI Failed.\n");
        exit(1);
        break;

    case 0: // I'm GUI

        sprintf(num_of_liquid_production_lines, "%d", num_liquid_production_lines);
        sprintf(str_num_pill_lines, "%d", num_pill_production_lines);

        execlp("./GUI", "GUI", num_of_liquid_production_lines, str_num_pill_lines, NULL);
        perror("Error:Execute GUI Failed.\n");
        exit(1);
        break;

    default: // I'm parent
        pid_gui[0] = pid;
        break;
    }
}

/*
function to initialize IPCs resources (shared memory, semaphores, message queues)
*/
void initializeIPCResources()
{
    // Create a Shared Memories for struct of processes (4 shared memories done)
    shmptr_liquid_production_lines = createSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, num_liquid_production_lines * sizeof(struct Liquid_Production_Line), "parent.c");
    shmptr_pill_production_lines = createSharedMemory(SHKEY_PILL_PRODUCTION_LINES, num_pill_production_lines * sizeof(struct Pill_Production_Line), "parent.c");
    shmptr_liquid_medicines = createSharedMemory(SHKEY_LIQUID_MEDICINES, num_liq_meds * sizeof(Liq_Med), "parent.c");
    shmptr_pill_medicines = createSharedMemory(SHKEY_PILL_MEDICINES, num_pill_meds * sizeof(Pill_Med), "parent.c");

    shmptr_num_liquid_medicines_produced = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PRODUCED, sizeof(int), "parent.c");
    shmptr_num_liquid_medicines_failed = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_FAILED, sizeof(int), "parent.c");
    shmptr_num_liquid_medicines_packaged = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PACKAGED, sizeof(int), "parent.c");

    shmptr_num_pill_medicines_produced = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_PRODUCED, sizeof(int), "parent.c");
    shmptr_num_pill_medicines_failed = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_FAILED, sizeof(int), "parent.c");
    shmptr_num_pill_medicines_packaged = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_PACKAGED, sizeof(int), "parent.c");

    int x = 0;

    memcpy(shmptr_num_pill_medicines_produced, &x, sizeof(int));
    memcpy(shmptr_num_liquid_medicines_produced, &x, sizeof(int));
    memcpy(shmptr_num_pill_medicines_failed, &x, sizeof(int));
    memcpy(shmptr_num_liquid_medicines_failed, &x, sizeof(int));
    memcpy(shmptr_num_liquid_medicines_packaged, &x, sizeof(int));
    memcpy(shmptr_num_pill_medicines_packaged, &x, sizeof(int));
    memcpy(shmptr_liquid_medicines, liquid_medicines, num_liq_meds * sizeof(Liq_Med));
    memcpy(shmptr_pill_medicines, pill_medicines, num_pill_meds * sizeof(Pill_Med));
    liquid_production_lines = (struct Liquid_Production_Line *)shmptr_liquid_production_lines;
    pill_production_lines = (struct Pill_Production_Line *)shmptr_pill_production_lines;

    // Create a Semaphores
    sem_liquid_production_lines = createSemaphore(SEMKEY_LIQUID_PRODUCTION_LINES, 1, 1, "parent.c");
    sem_pill_production_lines = createSemaphore(SEMKEY_PILL_PRODUCTION_LINES, 1, 1, "parent.c");

    sem_num_liquid_medicines_produced = createSemaphore(SEMKEY_NUM_LIQUID_MEDICINES_PRODUCED, 1, 1, "parent.c");
    sem_num_pill_medicines_produced = createSemaphore(SEMKEY_NUM_PILL_MEDICINES_PRODUCED, 1, 1, "parent.c");

    sem_num_liquid_medicines_failed = createSemaphore(SEMKEY_NUM_LIQUID_MEDICINES_FAILED, 1, 1, "parent.c");
    sem_num_pill_medicines_failed = createSemaphore(SEMKEY_NUM_PILL_MEDICINES_FAILED, 1, 1, "parent.c");

    sem_num_liquid_medicines_packaged = createSemaphore(SEMKEY_NUM_LIQUID_MEDICINES_PACKAGED, 1, 1, "parent.c");
    sem_num_pill_medicines_packaged = createSemaphore(SEMKEY_NUM_PILL_MEDICINES_PACKAGED, 1, 1, "parent.c");
}

/*
function to monitor the thresholds of the simulation
*/
void thresholds_monitoring()
{
    while (!is_end)
    {
        // check all thresholds
        if (is_alarmed)
        {
            printf("The simulation threshold time is reached, the program is finished.\n\n");
            is_end = 1;
        }
        // check the threshold of number of liquid medicines produced
        acquireSem(sem_num_liquid_medicines_produced, 0, "parent.c");
        if (*(int *)shmptr_num_liquid_medicines_produced >= threshold_of_num_liquid_medicines_produced)
        {
            printf("The threshold of number of liquid medicines produced is reached, the program is finished.\n\n");
            is_end = 1;
        }
        releaseSem(sem_num_liquid_medicines_produced, 0, "parent.c");

        // check the threshold of number of pill medicines produced
        acquireSem(sem_num_pill_medicines_produced, 0, "parent.c");
        if (*(int *)shmptr_num_pill_medicines_produced >= threshold_of_num_pill_medicines_produced)
        {
            printf("The threshold of number of pill medicines produced is reached, the program is finished.\n\n");
            is_end = 1;
        }
        releaseSem(sem_num_pill_medicines_produced, 0, "parent.c");

        // check the threshold of number of liquid medicines failed
        acquireSem(sem_num_liquid_medicines_failed, 0, "parent.c");
        if (*(int *)shmptr_num_liquid_medicines_failed >= threshold_of_num_liquid_medicines_failed)
        {
            printf("The threshold of number of liquid medicines failed is reached, the program is finished.\n\n");
            is_end = 1;
        }
        releaseSem(sem_num_liquid_medicines_failed, 0, "parent.c");

        // check the threshold of number of pill medicines failed
        acquireSem(sem_num_pill_medicines_failed, 0, "parent.c");
        if (*(int *)shmptr_num_pill_medicines_failed >= threshold_of_num_pill_medicines_failed)
        {
            printf("The threshold of number of pill medicines failed is reached, the program is finished.\n\n");
            is_end = 1;
        }
        releaseSem(sem_num_pill_medicines_failed, 0, "parent.c");

        sleep(1);
    }

    pthread_exit(0);
}

/*
function to monitor the speed of the production lines to check if a production line is faster
than the other so send an employee from the faster one to the slower one
*/
void speed_monitoring()
{
    // check the speed according to the num produced medicines
    // sort all the production lines(liquid and pill) according to the num of produced medicines
    // send an employee from the faster ones to the slower ones if the difference in the num of produced medicines is greater than 2
    sleep(5);
    while (!is_end)
    {

        Liquid_Production_Line temp_liquid[4];
        Pill_Production_Line temp_pill[4];

        for (int i = 0; i < num_pill_production_lines; i++)
        {

            temp_pill[i].production_line.id = pill_production_lines[i].production_line.id;
            temp_pill[i].production_line.num_produced_medicines = pill_production_lines[i].production_line.num_produced_medicines;
            temp_pill[i].production_line.num_employes = pill_production_lines[i].production_line.num_employes;
            temp_pill[i].production_line.original_num_employes = pill_production_lines[i].production_line.original_num_employes;
            temp_pill[i].production_line.speed = pill_production_lines[i].production_line.speed;

            for (int j = 0; j < pill_production_lines[i].production_line.num_produced_medicines; j++)
            {
                temp_pill[i].pill_medicines[j] = pill_production_lines[i].pill_medicines[j];

                for (int k = 0; k < pill_production_lines[i].pill_medicines[j].num_plastic_containers; k++)
                {
                    temp_pill[i].pill_medicines[j].plastic_containers[k] = pill_production_lines[i].pill_medicines[j].plastic_containers[k];

                    for (int l = 0; l < pill_production_lines[i].pill_medicines[j].plastic_containers[k].num_pills; l++)
                    {
                        temp_pill[i].pill_medicines[j].plastic_containers[k].pills[l] = pill_production_lines[i].pill_medicines[j].plastic_containers[k].pills[l];
                    }
                }
            }
        }

        // copy the production lines to the temp arrays
        for (int i = 0; i < num_liquid_production_lines; i++)
        {
            temp_liquid[i].production_line.id = liquid_production_lines[i].production_line.id;
            temp_liquid[i].production_line.num_produced_medicines = liquid_production_lines[i].production_line.num_produced_medicines;
            temp_liquid[i].production_line.num_employes = liquid_production_lines[i].production_line.num_employes;
            temp_liquid[i].production_line.original_num_employes = liquid_production_lines[i].production_line.original_num_employes;
            temp_liquid[i].production_line.speed = liquid_production_lines[i].production_line.speed;
            for (int j = 0; j < liquid_production_lines[i].production_line.num_produced_medicines; j++)
            {
                temp_liquid[i].bottles[j] = liquid_production_lines[i].bottles[j];
            }
        }

        // sort the liquid production lines
        for (int i = 0; i < num_liquid_production_lines; i++)
        {
            for (int j = i + 1; j < num_liquid_production_lines; j++)
            {
                if (temp_liquid[i].production_line.num_produced_medicines < temp_liquid[j].production_line.num_produced_medicines)
                {
                    Liquid_Production_Line temp = temp_liquid[i];
                    temp_liquid[i] = temp_liquid[j];
                    temp_liquid[j] = temp;
                }
            }
        }

        // sort the pill production lines
        for (int i = 0; i < num_pill_production_lines; i++)
        {
            for (int j = i + 1; j < num_pill_production_lines; j++)
            {
                if (temp_pill[i].production_line.num_produced_medicines < temp_pill[j].production_line.num_produced_medicines)
                {
                    Pill_Production_Line temp = temp_pill[i];
                    temp_pill[i] = temp_pill[j];
                    temp_pill[j] = temp;
                }
            }
        }

        // check the speed of the liquid production lines the first with last and second with second last and so on
        for (int i = 0; i < num_liquid_production_lines / 2; i++)
        {

            if (temp_liquid[num_liquid_production_lines - i - 1].production_line.num_produced_medicines < 5 && temp_liquid[i].production_line.num_produced_medicines - temp_liquid[num_liquid_production_lines - i - 1].production_line.num_produced_medicines > 2)
            {
                printf("liquid from %d to %d\n", temp_liquid[i].production_line.id, temp_liquid[num_liquid_production_lines - i - 1].production_line.id);

                // send an employee from the faster one to the slower one
                acquireSem(sem_liquid_production_lines, 0, "parent.c");
                liquid_production_lines[temp_liquid[i].production_line.id - 1].production_line.num_employes--;
                liquid_production_lines[temp_liquid[i].production_line.id - 1].production_line.original_num_employes--;

                liquid_production_lines[temp_liquid[num_liquid_production_lines - i - 1].production_line.id - 1].production_line.num_employes++;
                liquid_production_lines[temp_liquid[num_liquid_production_lines - i - 1].production_line.id - 1].production_line.from_line = temp_liquid[i].production_line.id;

                // reduce the speed of production line of the faster one by a ratio of one employee to all original employees
                liquid_production_lines[temp_liquid[i].production_line.id].production_line.speed = liquid_production_lines[temp_liquid[i].production_line.id].production_line.speed - (liquid_production_lines[temp_liquid[i].production_line.id].production_line.original_num_employes / 2);
                // increase the speed of production line of the slower one by a ratio of one employee to all original employees
                liquid_production_lines[temp_liquid[num_liquid_production_lines - i - 1].production_line.id].production_line.speed = liquid_production_lines[temp_liquid[num_liquid_production_lines - i - 1].production_line.id].production_line.speed + (liquid_production_lines[temp_liquid[num_liquid_production_lines - i - 1].production_line.id].production_line.original_num_employes / 2);

                releaseSem(sem_liquid_production_lines, 0, "parent.c");
            }
        }

        // check the speed of the pill production lines the first with last and second with second last and so on
        for (int i = 0; i < num_pill_production_lines / 2; i++)
        {
            if (temp_pill[num_pill_production_lines - i - 1].production_line.num_produced_medicines < 5 && temp_pill[i].production_line.num_produced_medicines - temp_pill[num_pill_production_lines - i - 1].production_line.num_produced_medicines > 2)
            {
                printf("pill from %d to %d\n", temp_pill[i].production_line.id, temp_pill[num_pill_production_lines - i - 1].production_line.id);
                // send an employee from the faster one to the slower one
                acquireSem(sem_pill_production_lines, 0, "parent.c");
                pill_production_lines[temp_pill[i].production_line.id - 1].production_line.num_employes--;
                pill_production_lines[temp_pill[i].production_line.id - 1].production_line.original_num_employes--;

                pill_production_lines[temp_pill[num_pill_production_lines - i - 1].production_line.id - 1].production_line.num_employes++;
                pill_production_lines[temp_pill[num_pill_production_lines - i - 1].production_line.id - 1].production_line.from_line = temp_pill[i].production_line.id;

                // printf("hi1 %d\n", pill_production_lines[temp_pill[num_pill_production_lines - i - 1].production_line.id - 1].production_line.from_line);
                // printf("hi2 %d\n", pill_production_lines[temp_pill[i].production_line.id - 1].production_line.id);

                // send a signal to the production line to increase employee thread
                // kill(pids_pill_production_lines[temp_pill[num_pill_production_lines - i - 1].production_line.id - 1], SIGUSR1);
                // send a signal to the production line to decrease employee thread
                // kill(pids_pill_production_lines[temp_pill[i].production_line.id - 1], SIGUSR2);

                // reduce the speed of production line of the faster one by a ratio of one employee to all original employees
                pill_production_lines[temp_pill[i].production_line.id].production_line.speed = pill_production_lines[temp_pill[i].production_line.id].production_line.speed - (pill_production_lines[temp_pill[i].production_line.id].production_line.original_num_employes / 2);
                // increase the speed of production line of the slower one by a ratio of one employee to all original employees
                pill_production_lines[temp_pill[num_pill_production_lines - i - 1].production_line.id].production_line.speed = pill_production_lines[temp_pill[num_pill_production_lines - i - 1].production_line.id].production_line.speed + (pill_production_lines[temp_pill[num_pill_production_lines - i - 1].production_line.id].production_line.original_num_employes / 2);

                releaseSem(sem_pill_production_lines, 0, "parent.c");
            }
        }

        sleep(10);

        // return the employees to their original production lines
        for (int i = 0; i < num_liquid_production_lines; i++)
        {
            acquireSem(sem_liquid_production_lines, 0, "parent.c");

            // check if the number of employees is large than the original number of employees
            if (liquid_production_lines[i].production_line.num_employes > liquid_production_lines[i].production_line.original_num_employes)
            {
                liquid_production_lines[i].production_line.num_employes--;
                liquid_production_lines[liquid_production_lines[i].production_line.from_line - 1].production_line.original_num_employes++;

                // send a signal to the production line to increase employee thread
                // kill(pids_liquid_production_lines[i], SIGUSR1);
            }
            /*else if (liquid_production_lines[i].production_line.num_employes > liquid_production_lines[i].production_line.original_num_employes)
            {
                liquid_production_lines[i].production_line.num_employes--;
                // send a signal to the production line to decrease employee thread
                // kill(pids_liquid_production_lines[i], SIGUSR2);
            }*/

            releaseSem(sem_liquid_production_lines, 0, "parent.c");
        }

        for (int i = 0; i < num_pill_production_lines; i++)
        {
            acquireSem(sem_pill_production_lines, 0, "parent.c");

            // check if the number of employees is less than the original number of employees
            if (pill_production_lines[i].production_line.num_employes > pill_production_lines[i].production_line.original_num_employes)
            {
                pill_production_lines[i].production_line.num_employes--;
                pill_production_lines[pill_production_lines[i].production_line.from_line - 1].production_line.original_num_employes++;

                // send a signal to the production line to increase employee thread
                // kill(pids_pill_production_lines[i], SIGUSR1);
            }
            /*else if (pill_production_lines[i].production_line.num_employes > pill_production_lines[i].production_line.original_num_employes)
            {
                pill_production_lines[i].production_line.num_employes--;
                // send a signal to the production line to decrease employee thread
                // kill(pids_pill_production_lines[i], SIGUSR2);
            }*/

            releaseSem(sem_pill_production_lines, 0, "parent.c");
        }

        sleep(2);
    }

    pthread_exit(0);
}

void getArguments(int *numberArray)
{
    num_liquid_production_lines = numberArray[0];
    num_pill_production_lines = numberArray[1];
    threshold_of_num_liquid_medicines_produced = numberArray[2];
    threshold_of_num_pill_medicines_produced = numberArray[3];
    threshold_of_num_liquid_medicines_failed = numberArray[4];
    threshold_of_num_pill_medicines_failed = numberArray[5];
    simulation_threshold_time = numberArray[6];
}

void checkArguments(int argc, char **argv, char *file_name, char *file_liquid_medicines, char *file_pill_medicines)
{
    if (argc != 4) // check if the user passed the correct arguments
    {
        printf("Usage: Invalid arguments.\n"); // Use the default file names
        printf("Using default file names: arguments.txt\n");
        strcpy(file_name, FILE_NAME);
        strcpy(file_liquid_medicines, LIQUID_MEDICINES_FILE);
        strcpy(file_pill_medicines, PILL_MEDICINES_FILE);
    }
    else
    {
        strcpy(file_name, argv[1]); // Use the file names provided by the user
        strcpy(file_liquid_medicines, argv[2]);
        strcpy(file_pill_medicines, argv[3]);
    }
}

void exitProgram()
{
    printf("\nKilling all processes...\n");
    fflush(stdout);

    // kill all the child processes
    killAllProcesses(pids_liquid_production_lines, num_liquid_production_lines);
    killAllProcesses(pids_pill_production_lines, num_pill_production_lines);
    killAllProcesses(pid_gui, 1);
    printf("All child processes killed\n");

    printf("Cleaning up IPC resources...\n");
    fflush(stdout);

    // delete all the shared memories
    deleteSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, num_liquid_production_lines * sizeof(struct Liquid_Production_Line), shmptr_liquid_production_lines);
    deleteSharedMemory(SHKEY_PILL_PRODUCTION_LINES, num_pill_production_lines * sizeof(struct Pill_Production_Line), shmptr_pill_production_lines);
    deleteSharedMemory(SHKEY_LIQUID_MEDICINES, num_liq_meds * sizeof(Liq_Med), shmptr_liquid_medicines);
    deleteSharedMemory(SHKEY_PILL_MEDICINES, num_pill_meds * sizeof(Pill_Med), shmptr_pill_medicines);
    deleteSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PRODUCED, sizeof(int), shmptr_num_liquid_medicines_produced);
    deleteSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_FAILED, sizeof(int), shmptr_num_liquid_medicines_failed);
    deleteSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PACKAGED, sizeof(int), shmptr_num_liquid_medicines_packaged);
    deleteSharedMemory(SHKEY_NUM_PILL_MEDICINES_PRODUCED, sizeof(int), shmptr_num_pill_medicines_produced);
    deleteSharedMemory(SHKEY_NUM_PILL_MEDICINES_FAILED, sizeof(int), shmptr_num_pill_medicines_failed);

    // delete all the semaphores
    deleteSemaphore(sem_liquid_production_lines);
    deleteSemaphore(sem_pill_production_lines);
    deleteSemaphore(sem_num_liquid_medicines_produced);
    deleteSemaphore(sem_num_pill_medicines_produced);
    deleteSemaphore(sem_num_liquid_medicines_failed);
    deleteSemaphore(sem_num_pill_medicines_failed);

    printf("IPC resources cleaned up successfully\n");
    printf("Exiting...\n");
    fflush(stdout);
    exit(0);
}