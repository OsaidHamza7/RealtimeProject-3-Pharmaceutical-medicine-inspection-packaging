#include "header.h"
#include "ipcs.h"
//***********************************************************************************
// Function Prototypes
void getInformation(char **argv);
void init_signals_handlers();
void employee(void *args);
//***********************************************************************************
int production_line_num;
int num_of_liquid_production_lines;

int range_of_speed[2];
int range_number_of_medicines[2];
int range_level_liq_medicine[2];
int range_color_liq_medicine[2];

Liquid_Production_Line *liquid_production_line;
Liquid_Production_Line *temp;
Liquid_Production_Line *liquid_production_lines;

char *shmptr_liquid_production_lines;
int sem_liquid_production_lines;
sem_t mutex;

int main(int argc, char **argv)
{
    // check the number of arguments
    if (argc < 8)
    {
        perror("The user should pass an argument like:production_line_num,num_of_liquid_production_lines, str_num_employees, str_range_num_midicines, str_range_of_speeds, str_range_level_liquid_medicine, str_range_color_liquid_medicine\n");
        exit(-1);
    }

    srand((unsigned)getpid()); // seed for the random function with the ID of the current process
    production_line_num = atoi(argv[1]);
    num_of_liquid_production_lines = atoi(argv[2]);

    // // Open a shared memories
    shmptr_liquid_production_lines = createSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, num_of_liquid_production_lines * sizeof(struct Liquid_Production_Line), "liquid_production_line.c");
    liquid_production_lines = (struct Liquid_Production_Line *)shmptr_liquid_production_lines;

    // // Open the semaphores
    sem_liquid_production_lines = createSemaphore(SEMKEY_LIQUID_PRODUCTION_LINES, 1, 1, "liquid_production_line.c");
    // 0 :shared between threads 1 :shared between process

    // get information from the arguments
    getInformation(argv);

    // Todo1: create the liquid medicines

    sem_init(&mutex, 0, 1);
    // creats threads for employees
    pthread_t employees[num_of_liquid_production_lines];

    int employee_id[num_employees];
    // create thread for each employee
    for (int i = 0; i < num_of_liquid_production_lines; i++)
    {
        employee_id[i] = i;
        pthread_create(&employees[i], NULL, (void *)employee, (void *)&employee_id[i]);
    }

    // wait for the threads to finish
    for (int i = 0; i < num_of_liquid_production_lines; i++)
    {
        pthread_join(employees[i], NULL);
    }

    // while (1)
    // {
    // }
    sem_destroy(&mutex);
    return 0;
}

void getInformation(char **argv)
{
    // get the information of the liquid production line from the arguments
    liquid_production_line = &liquid_production_lines[production_line_num - 1];
    temp = liquid_production_line;

    split_string(argv[4], range_number_of_medicines);
    split_string(argv[5], range_of_speed);
    split_string(argv[6], range_level_liq_medicine);
    split_string(argv[7], range_color_liq_medicine);

    printf("=====================================================================\n");
    // print all arguments
    printf("production_line_num: %d\n", production_line_num);
    printf("num_of_liquid_production_lines: %d\n", num_of_liquid_production_lines);
    printf("num_employes: %d\n", atoi(argv[3]));
    printf("range_of_medicines: %d - %d\n", range_number_of_medicines[0], range_number_of_medicines[1]);
    printf("range_speed_lines: %d - %d\n", range_of_speed[0], range_of_speed[1]);

    printf("range_level_liquid_medicine: %d - %d\n", range_level_liq_medicine[0], range_level_liq_medicine[1]);
    printf("range_color_liquid_medicine: %d - %d\n\n", range_color_liq_medicine[0], range_color_liq_medicine[1]);

    acquireSem(sem_liquid_production_lines, 0, "liquid_production_line.c");

    liquid_production_line->pid = getpid();
    liquid_production_line->num = production_line_num;
    liquid_production_line->num_employes = atoi(argv[3]);
    liquid_production_line->num_medicines = get_random_number(range_number_of_medicines[0], range_number_of_medicines[1]);
    liquid_production_line->speed = get_random_number(range_of_speed[0], range_of_speed[1]);

    releaseSem(sem_liquid_production_lines, 0, "liquid_production_line.c");

    printf("Liquid Production Line %d is created with %d employees, %d medicines, and speed %d\n\n", liquid_production_line->num, liquid_production_line->num_employes, liquid_production_line->num_medicines, liquid_production_line->speed);
    fflush(stdout);
    printf("=====================================================================\n");
    fflush(stdout);
}

void init_signals_handlers()
{

    /*if (sigset(SIGCLD, signal_handler) == -1)
    { // set the signal handler for SIGINT
        perror("Signal Error\n");
        exit(-1);
    }*/
}

void employee(void *args)
{
    int *emp_id = (int *)args;

    printf("Employee %d: started working\n", *emp_id);

    while (1)
    {
        sem_wait(&mutex);
        // Todo2: code the inspects the medicine

        printf("i'm %d doing the task one\n", *emp_id);
        sleep(3);
        sem_wait(&mutex);
        // Todo3: code the packaging the medicine

        printf("i'm %d done task 1\n", *emp_id);
        sleep(2);
    }
}
