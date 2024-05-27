#include "header.h"
#include "ipcs.h"

//***********************************************************************************
// Function Prototypes
void getInformation(char **argv);
void init_signals_handlers();
void employee(void *args);
void createPillMedicines(Pill_Production_Line *pill_production_line);


//***********************************************************************************

int production_line_num;
int num_of_pill_production_lines;

int range_of_speed[2];
int range_number_of_medicines[2];

int range_plastic_containers[2];
int range_pills[2];

int range_size_pills[2];
int range_color_pills[2];

Pill_Production_Line *pill_production_line;
Pill_Production_Line *temp;
Pill_Production_Line *pill_production_lines;

char *shmptr_pill_production_lines;
int sem_pill_production_lines;
sem_t mutex;

int main(int argc, char **argv)
{
    // check the number of arguments
    if (argc < 9)
    {
        perror("The user should pass an argument like:production_line_num,num_pill_production_lines,num_employees,range_num_midicines,range_of_speeds,range_of_plastic_containers,range_of_pills,range_size_pill,range_color_pill\n");
        exit(-1);
    }

    srand((unsigned)getpid()); // seed for the random function with the ID of the current process
    production_line_num = atoi(argv[1]);
    num_of_pill_production_lines = atoi(argv[2]);

    // Open a shared memories
    shmptr_pill_production_lines = createSharedMemory(SHKEY_PILL_PRODUCTION_LINES, num_of_pill_production_lines * sizeof(struct Pill_Production_Line), "pill_production_line.c");
    pill_production_lines = (struct Pill_Production_Line *)shmptr_pill_production_lines;

    // Open the semaphores
    sem_pill_production_lines = createSemaphore(SEMKEY_PILL_PRODUCTION_LINES, 1, 1, "pill_production_line.c");

    sleep(2);
    // get information from the arguments
    getInformation(argv);

    // Todo1: create the liquid medicines

    sem_init(&mutex, 0, 1);
    // creats threads for employees
    pthread_t employees[num_of_pill_production_lines];

    int employee_id[num_employees];
    // create thread for each employee
    for (int i = 0; i < num_of_pill_production_lines; i++)
    {
        employee_id[i] = i;
        pthread_create(&employees[i], NULL, (void *)employee, (void *)&employee_id[i]);
    }

    // wait for the threads to finish
    for (int i = 0; i < num_of_pill_production_lines; i++)
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
    // get the information of the pill production line from the arguments
    pill_production_line = &pill_production_lines[production_line_num - 1];
    temp = pill_production_line;

    split_string(argv[4], range_number_of_medicines);
    split_string(argv[5], range_of_speed);
    split_string(argv[6], range_plastic_containers);
    split_string(argv[7], range_pills);
    split_string(argv[8], range_size_pills);
    split_string(argv[9], range_color_pills);

    printf("=====================================================================\n");
    // print the information of the pill production line
    printf("Pill Production Line %d\n", production_line_num);
    printf("Number of employees: %d\n", atoi(argv[3]));
    printf("Number of Pill Medicines: %d - %d\n", range_number_of_medicines[0], range_number_of_medicines[1]);
    printf("Speed: %d - %d\n", range_of_speed[0], range_of_speed[1]);
    printf("Plastic Containers: %d - %d\n", range_plastic_containers[0], range_plastic_containers[1]);
    printf("Pills: %d - %d\n", range_pills[0], range_pills[1]);
    printf("Size of Pills: %d - %d\n", range_size_pills[0], range_size_pills[1]);
    printf("Color of Pills: %d - %d\n\n", range_color_pills[0], range_color_pills[1]);
    fflush(stdout);

    acquireSem(sem_pill_production_lines, 0, "pill_production_line.c");

    pill_production_line->pid = getpid();
    pill_production_line->num = production_line_num;
    pill_production_line->num_employes = atoi(argv[3]);
    pill_production_line->num_medicines = get_random_number(range_number_of_medicines[0], range_number_of_medicines[1]);
    pill_production_line->speed = get_random_number(range_of_speed[0], range_of_speed[1]);

    releaseSem(sem_pill_production_lines, 0, "pill_production_line.c");

    printf("Pill Production Line %d is created with %d employees, %d medicines, and speed %d\n\n", pill_production_line->num, pill_production_line->num_employes, pill_production_line->num_medicines, pill_production_line->speed);
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
void createPillMedicines(Pill_Production_Line *pill_Production_Line)
{
    for (int i = 0; i  < pill_Production_Line->num_medicines; i++)
    {
        pill_Production_Line->pill_medicines[i].id = i + 1;
        pill_Production_Line->pill_medicines[i].production_line_num = pill_Production_Line->num;
        pill_Production_Line->pill_medicines[i].num_plastic_containers = get_random_number(range_of_plastic_containers[0], range_of_plastic_containers[1]);
        for(int j=0; j<  pill_Production_Line->pill_medicines->num_plastic_containers; j++)
        {
            pill_Production_Line->pill_medicines->plastic_containers[j].id= j+1;
            pill_Production_Line->pill_medicines->plastic_containers[j].num_pills= get_random_number(range_of_pills[0],range_of_pills[1]);
            for(int k=0; k<  pill_Production_Line->pill_medicines->plastic_containers->pills; k++)
            {
                pill_Production_Line->pill_medicines->plastic_containers->pills[k].id = k+1;
                pill_Production_Line->pill_medicines->plastic_containers->pills[k].color = get_random_number(range_color_pill[0], range_color_pill[1]);
                pill_Production_Line->pill_medicines->plastic_containers->pills[k].size = get_random_number(range_size_pill[0],range_size_pill[1]);
            }
        }
        //pill_Production_Line->pill_medicines[i].Expiry_date=;
    }
    // print the liquid medicines
    for (int i = 0; i < pill_Production_Line->num_medicines; i++)
    {
        printf("Pill Medicine %d is created with id %d, production line num %d, num plastic containers %d", pill_Production_Line->pill_medicines[i].id, pill_Production_Line->pill_medicines[i].production_line_num, pill_Production_Line->pill_medicines[i].num_plastic_containers);
        fflush(stdout);
        for(int j=0; j<  pill_Production_Line->pill_medicines->num_plastic_containers; j++)
        {
            printf("Plastic container with id %d, number of bills %d", pill_Production_Line->pill_medicines->plastic_containers[j].id,pill_Production_Line->pill_medicines->plastic_containers[j].num_pills);
            for(int k=0; k<  pill_Production_Line->pill_medicines->plastic_containers->pills; k++){
                printf("Pill with id %d, color %d, size %d",pill_Production_Line->pill_medicines->plastic_containers->pills[k].id,pill_Production_Line->pill_medicines->plastic_containers->pills[k].color,pill_Production_Line->pill_medicines->plastic_containers->pills[k].size);
            }
        }
    }
    printf("=====================================================================\n");
    fflush(stdout);
}
