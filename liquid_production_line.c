#include "header.h"
#include "ipcs.h"
//***********************************************************************************
// Function Prototypes
void getInformation(char **argv);
void init_signals_handlers();
void employee(void *args);
void createLiquidMedicines(Liquid_Production_Line *liquid_production_line);
void inspect_medicine(Liquid_Medicine *liquid_medicines);
void package_medcine(Liquid_Medicine *liquid_medicines, int num_medicines);
//***********************************************************************************
int production_line_num;
int num_of_liquid_production_lines;

int range_of_speed[2];
int range_number_of_medicines[2];
int range_level_liq_medicine[2];
int range_color_liq_medicine[2];
int range_expected_liq_medicine_level[2];
int range_expected_liq_medicine_color[2];
int number_of_employees;

Liquid_Production_Line *liquid_production_line;
Liquid_Production_Line *temp;
Liquid_Production_Line *liquid_production_lines;

char *shmptr_liquid_production_lines;
int sem_liquid_production_lines;
sem_t mutex;

int main(int argc, char **argv)
{
    // check the number of arguments
    if (argc < 10)
    {
        perror("The user should pass an argument like:production_line_num,num_of_liquid_production_lines, str_num_employees, str_range_num_midicines, str_range_of_speeds, str_range_level_liquid_medicine, str_range_color_liquid_medicine\n");
        exit(-1);
    }

    srand((unsigned)getpid()); // seed for the random function with the ID of the current process
    production_line_num = atoi(argv[1]);
    num_of_liquid_production_lines = atoi(argv[2]);

    // Open a shared memories
    shmptr_liquid_production_lines = createSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, num_of_liquid_production_lines * sizeof(struct Liquid_Production_Line), "liquid_production_line.c");
    liquid_production_lines = (struct Liquid_Production_Line *)shmptr_liquid_production_lines;

    // Open the semaphores
    sem_liquid_production_lines = createSemaphore(SEMKEY_LIQUID_PRODUCTION_LINES, 1, 1, "liquid_production_line.c");
    // 0 :shared between threads 1 :shared between process

    // get information from the arguments
    getInformation(argv);

    // create the liquid medicines
    createLiquidMedicines(liquid_production_line);

    sem_init(&mutex, 0, 1);

    // creats threads for employees
    pthread_t employees[number_of_employees];
    int employee_id[number_of_employees];

    // create thread for each employee
    for (int i = 0; i < number_of_employees; i++)
    {
        employee_id[i] = i + 1;
        pthread_create(&employees[i], NULL, (void *)employee, (void *)&employee_id[i]);
    }

    // wait for the threads to finish
    for (int i = 0; i < number_of_employees; i++)
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
    number_of_employees = atoi(argv[3]);
    split_string(argv[4], range_number_of_medicines);
    split_string(argv[5], range_of_speed);
    split_string(argv[6], range_level_liq_medicine);
    split_string(argv[7], range_color_liq_medicine);
    split_string(argv[8], range_expected_liq_medicine_level);
    split_string(argv[9], range_expected_liq_medicine_color);

    printf("=====================================================================\n");
    fflush(stdout);

    // print all arguments
    printf("production_line_num: %d\n", production_line_num);
    printf("num_of_liquid_production_lines: %d\n", num_of_liquid_production_lines);
    printf("num_employes: %d\n", number_of_employees);
    printf("range_of_medicines: %d - %d\n", range_number_of_medicines[0], range_number_of_medicines[1]);
    printf("range_speed_lines: %d - %d\n", range_of_speed[0], range_of_speed[1]);

    printf("range_level_liquid_medicine: %d - %d\n", range_level_liq_medicine[0], range_level_liq_medicine[1]);
    printf("range_color_liquid_medicine: %d - %d\n\n", range_color_liq_medicine[0], range_color_liq_medicine[1]);

    printf("range_expected_liquid_medicine_level: %d - %d\n", range_expected_liq_medicine_level[0], range_expected_liq_medicine_level[1]);
    printf("range_expected_liquid_medicine_color: %d - %d\n", range_expected_liq_medicine_color[0], range_expected_liq_medicine_color[1]);

    acquireSem(sem_liquid_production_lines, 0, "liquid_production_line.c");

    liquid_production_line->pid = getpid();
    liquid_production_line->num = production_line_num;
    liquid_production_line->num_employes = number_of_employees;
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

    while (1)
    {
        sem_wait(&mutex);
        // inspect the medicine
        printf("Employee %d in liquid line %d doing inspect task\n", *emp_id, liquid_production_line->num);
        inspect_medicine(liquid_production_line->liquid_medicines);

        printf("Employee %d in liquid line %d doing package task\n", *emp_id, liquid_production_line->num);
        // packaging the medicines
        // package_medicine(liquid_production_line->liquid_medicines, liquid_production_line->num_medicines);
        sem_post(&mutex);
        break;
    }
}

void createLiquidMedicines(Liquid_Production_Line *liquid_production_line)
{
    for (int i = 0; i < liquid_production_line->num_medicines; i++)
    {
        liquid_production_line->liquid_medicines[i].id = i + 1;
        liquid_production_line->liquid_medicines[i].production_line_num = liquid_production_line->num;
        liquid_production_line->liquid_medicines[i].level = get_random_number(range_level_liq_medicine[0], range_level_liq_medicine[1]);
        liquid_production_line->liquid_medicines[i].color = get_random_number(range_color_liq_medicine[0], range_color_liq_medicine[1]);
        liquid_production_line->liquid_medicines[i].is_sealed = get_random_number(0, 1);
        liquid_production_line->liquid_medicines[i].is_label_placed = get_random_number(0, 1);
        liquid_production_line->liquid_medicines[i].is_inspected = 0;
        liquid_production_line->liquid_medicines[i].is_packaged = 0;
        liquid_production_line->liquid_medicines[i].is_failed = 0;
    }

    // print the liquid medicines
    for (int i = 0; i < liquid_production_line->num_medicines; i++)
    {
        printf("Liquid Medicine %d is created with level %d, color %d, is_sealed %d, is_label_placed %d\n", liquid_production_line->liquid_medicines[i].id, liquid_production_line->liquid_medicines[i].level, liquid_production_line->liquid_medicines[i].color, liquid_production_line->liquid_medicines[i].is_sealed, liquid_production_line->liquid_medicines[i].is_label_placed);
        fflush(stdout);
    }
    printf("=====================================================================\n");
    fflush(stdout);
}

void inspect_medicine(Liquid_Medicine *liquid_medicines)
{
    for (int i = 0; i < liquid_production_line->num_medicines; i++)
    {
        if (liquid_medicines[i].is_inspected == 0)
        {
            printf("Liquid Medicine %d in line %d is inspecting\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
            liquid_medicines[i].is_inspected = 1;
            sleep(3); // sleep for 3 seconds to simulate the inspection process
            if (liquid_medicines[i].level >= range_expected_liq_medicine_level[0] && liquid_medicines[i].level <= range_expected_liq_medicine_level[1])
            {
                printf("Liquid Medicine %d in line %d is in expected range of level\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
            }
            else
            {
                printf("Liquid Medicine %d in line %d is NOT in expected range of level\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
                liquid_medicines[i].is_failed = 1;
                break;
            }

            if (liquid_medicines[i].color >= range_expected_liq_medicine_color[0] && liquid_medicines[i].color <= range_expected_liq_medicine_color[1])
            {
                printf("Liquid Medicine %d in line %d is in expected range of color\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
            }
            else
            {
                printf("Liquid Medicine %d in line %d is NOT in expected range of color\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
                liquid_medicines[i].is_failed = 1;
                break;
            }

            if (liquid_medicines[i].is_sealed == 1)
            {
                printf("Liquid Medicine %d in line %d is sealed\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
            }
            else
            {
                printf("Liquid Medicine %d in line %d is NOT sealed\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
                liquid_medicines[i].is_failed = 1;
                break;
            }

            if (liquid_medicines[i].is_label_placed == 1)
            {
                printf("Liquid Medicine %d in line %d is labeled\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
            }
            else
            {
                printf("Liquid Medicine %d in line %d is NOT labeled\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
                liquid_medicines[i].is_failed = 1;
                break;
            }
            printf("Liquid Medicine %d in line %d is passed the inspected successfully\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
        }
    }
}

void package_medcine(Liquid_Medicine *liquid_medicines, int num_medicines)
{
    for (int i = 0; i < num_medicines; i++)
    {
        if (liquid_medicines[i].is_packaged == 0)
        {
            printf("Liquid Medicine %d in line %d is packaging\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
            liquid_medicines[i].is_packaged = 1;
            sleep(3); // sleep for 3 seconds to simulate the packaging process
            printf("Liquid Medicine %d in line %d is packaged successfully\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
        }
    }
}