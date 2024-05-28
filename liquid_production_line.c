#include "header.h"
#include "ipcs.h"
//***********************************************************************************
// Function Prototypes
void getInformation(char **argv);
void init_signals_handlers();
void employee(void *args);
void createLiquidMedicines();
int get_index_of_uninspected_medicine(Liquid_Medicine *liquid_medicines);
int inspect_medicine(Liquid_Medicine *liquid_medicines, int index_medicine);
void package_medcine(Liquid_Medicine *liquid_medicines, int num_medicines);
//***********************************************************************************
int production_line_num;
int num_of_liquid_production_lines;

int range_of_speed[2];
int range_level_liq_medicine[2];
int range_color_liq_medicine[2];
int range_expected_liq_medicine_level[2];
int range_expected_liq_medicine_color[2];
int number_of_employees;
int is_there_sucess_inspected_medicine = 0;
int max_produce_time = 10;
Liquid_Production_Line *liquid_production_line;
Liquid_Production_Line *temp;
Liquid_Production_Line *liquid_production_lines;

char *shmptr_liquid_production_lines;
char *shmptr_num_liquid_medicines_produced;
char *shmptr_num_pill_medicines_failed;

int sem_liquid_production_lines;
int sem_num_liquid_medicines_produced;
int sem_num_pill_medicines_failed;

sem_t mutex_liquid_midicines;

int main(int argc, char **argv)
{
    // check the number of arguments
    if (argc < 9)
    {
        perror("The user should pass an argument like:production_line_num,num_of_liquid_production_lines, str_num_employees, str_range_of_speeds, str_range_level_liquid_medicine, str_range_color_liquid_medicine\n");
        exit(-1);
    }

    srand((unsigned)getpid()); // seed for the random function with the ID of the current process
    production_line_num = atoi(argv[1]);
    num_of_liquid_production_lines = atoi(argv[2]);

    // Open a shared memories
    shmptr_liquid_production_lines = createSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, num_of_liquid_production_lines * sizeof(struct Liquid_Production_Line), "liquid_production_line.c");
    liquid_production_lines = (struct Liquid_Production_Line *)shmptr_liquid_production_lines;

    shmptr_num_liquid_medicines_produced = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PRODUCED, sizeof(int), "liquid_production_line.c");
    shmptr_num_pill_medicines_failed = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_FAILED, sizeof(int), "liquid_production_line.c");

    // Open the semaphores
    sem_liquid_production_lines = createSemaphore(SEMKEY_LIQUID_PRODUCTION_LINES, 1, 1, "liquid_production_line.c");
    // 0 :shared between threads 1 :shared between process

    // get information from the arguments
    getInformation(argv);

    // Thread to create the liquid medicines
    pthread_t create_liquid_medicine_thread;
    pthread_create(&create_liquid_medicine_thread, NULL, (void *)createLiquidMedicines, NULL);

    sem_init(&mutex_liquid_midicines, 0, 1);

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

    pthread_join(create_liquid_medicine_thread, NULL);
    // while (1)
    // {
    // }
    sem_destroy(&mutex_liquid_midicines);
    return 0;
}

void getInformation(char **argv)
{
    // get the information of the liquid production line from the arguments
    liquid_production_line = &liquid_production_lines[production_line_num - 1];
    temp = liquid_production_line;
    number_of_employees = atoi(argv[3]);
    split_string(argv[4], range_of_speed);
    split_string(argv[5], range_level_liq_medicine);
    split_string(argv[6], range_color_liq_medicine);
    split_string(argv[7], range_expected_liq_medicine_level);
    split_string(argv[8], range_expected_liq_medicine_color);

    // printf("=====================================================================\n");
    // fflush(stdout);

    // print all arguments
    /*printf("production_line_num: %d\n", production_line_num);
    printf("num_of_liquid_production_lines: %d\n", num_of_liquid_production_lines);
    printf("num_employes: %d\n", number_of_employees);
    printf("range_speed_lines: %d - %d\n", range_of_speed[0], range_of_speed[1]);

    printf("range_level_liquid_medicine: %d - %d\n", range_level_liq_medicine[0], range_level_liq_medicine[1]);
    printf("range_color_liquid_medicine: %d - %d\n\n", range_color_liq_medicine[0], range_color_liq_medicine[1]);

    printf("range_expected_liquid_medicine_level: %d - %d\n", range_expected_liq_medicine_level[0], range_expected_liq_medicine_level[1]);
    printf("range_expected_liquid_medicine_color: %d - %d\n", range_expected_liq_medicine_color[0], range_expected_liq_medicine_color[1]);
*/
    acquireSem(sem_liquid_production_lines, 0, "liquid_production_line.c");

    liquid_production_line->pid = getpid();
    liquid_production_line->num = production_line_num;
    liquid_production_line->num_employes = number_of_employees;
    liquid_production_line->speed = get_random_number(range_of_speed[0], range_of_speed[1]);
    liquid_production_line->num_medicines = 0;
    releaseSem(sem_liquid_production_lines, 0, "liquid_production_line.c");

    printf("Liquid Production Line %d is created with %d employees, and speed %d\n\n", liquid_production_line->num, liquid_production_line->num_employes, liquid_production_line->speed);
    // printf("=====================================================================\n");
    // fflush(stdout);
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
        // inspect the uninspected medicines
        sem_wait(&mutex_liquid_midicines);
        printf("Employee %d in liquid line %d go to take a new medicine\n", *emp_id, liquid_production_line->num);
        int index_medicine = get_index_of_uninspected_medicine(liquid_production_line->liquid_medicines);
        sem_post(&mutex_liquid_midicines);

        // there is no medicines to inspect
        if (index_medicine == -1)
        {
            printf("Liquid line %d has no medicines\n", liquid_production_line->num);
            sleep(1);
            continue;
        }

        printf("Employee %d in liquid line %d inspects the medicine %d\n", *emp_id, liquid_production_line->num, index_medicine + 1);
        int j = inspect_medicine(liquid_production_line->liquid_medicines, index_medicine);
        sleep(5); // sleep for 3 seconds to simulate the inspection process

        // the inspection for this medicine is failed
        if (j == 0)
        {
            printf("Liquid Medicine %d in line %d is failed\n", liquid_production_line->liquid_medicines[index_medicine].id, liquid_production_line->liquid_medicines[index_medicine].production_line_num);
            liquid_production_line->liquid_medicines[index_medicine].is_failed = 1;
            continue;
        }
        printf("Liquid Medicine %d in line %d is passed the inspection successfully\n", liquid_production_line->liquid_medicines[index_medicine].id, liquid_production_line->liquid_medicines[index_medicine].production_line_num);

        // the inspection is successful go to the packaging
        // packaging the medicines

        /*printf("Employee %d in liquid line %d doing package task\n", *emp_id, liquid_production_line->num);
        package_medcine(liquid_production_line->liquid_medicines, index_medicine);
        sleep(3); // sleep for 3 seconds to simulate the packaging process
        printf("Liquid Medicine %d in line %d is packaged successfully\n", liquid_production_line->liquid_medicines[index_medicine].id, liquid_production_line->num);
        */
    }
}

void createLiquidMedicines()
{
    int j = 0;
    while (1)
    {
        int time = max_produce_time - (liquid_production_line->speed * max_produce_time) / 100;
        time = (time < 2) ? 2 : time;
        sleep(time); // sleep for some seconds to simulate the production of the medicines
        acquireSem(sem_liquid_production_lines, 0, "liquid_production_line.c");

        liquid_production_line->liquid_medicines[j].id = j + 1;
        liquid_production_line->liquid_medicines[j].production_line_num = liquid_production_line->num;
        liquid_production_line->liquid_medicines[j].level = get_random_number(range_level_liq_medicine[0], range_level_liq_medicine[1]);
        liquid_production_line->liquid_medicines[j].color = get_random_number(range_color_liq_medicine[0], range_color_liq_medicine[1]);
        liquid_production_line->liquid_medicines[j].is_sealed = get_random_number(0, 1);
        liquid_production_line->liquid_medicines[j].is_label_placed = get_random_number(0, 1);
        liquid_production_line->liquid_medicines[j].is_inspected = 0;
        liquid_production_line->liquid_medicines[j].is_failed = 0;
        liquid_production_line->num_medicines++;

        releaseSem(sem_liquid_production_lines, 0, "liquid_production_line.c");

        // create the medicine successfully
        printf("Created Liquid Medicine %d in line %d with level %d, color %d, is_sealed %d, is_label_placed %d\n", liquid_production_line->liquid_medicines[j].id, liquid_production_line->num, liquid_production_line->liquid_medicines[j].level, liquid_production_line->liquid_medicines[j].color, liquid_production_line->liquid_medicines[j].is_sealed, liquid_production_line->liquid_medicines[j].is_label_placed);
        fflush(stdout);
        j++;
    }
}

int get_index_of_uninspected_medicine(Liquid_Medicine *liquid_medicines)
{
    for (int i = 0; i < liquid_production_line->num_medicines; i++)
    {
        if (liquid_medicines[i].is_inspected == 0)
        {
            printf("Liquid Medicine %d in line %d is inspecting\n", liquid_medicines[i].id, liquid_medicines[i].production_line_num);
            liquid_medicines[i].is_inspected = 1; // set the medicine as inspected
            return i;                             // return the index of the uninspected medicine to inspect it
        }
    }
    return -1;
}

int inspect_medicine(Liquid_Medicine *liquid_medicines, int index_medicine)
{

    if (liquid_medicines[index_medicine].level >= range_expected_liq_medicine_level[0] && liquid_medicines[index_medicine].level <= range_expected_liq_medicine_level[1])
    {
        // printf("Liquid Medicine %d in line %d is in expected range of level\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    }
    else
    {
        // printf("Liquid Medicine %d in line %d is NOT in expected range of level\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
        return 0;
    }

    if (liquid_medicines[index_medicine].color >= range_expected_liq_medicine_color[0] && liquid_medicines[index_medicine].color <= range_expected_liq_medicine_color[1])
    {
        // printf("Liquid Medicine %d in line %d is in expected range of color\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    }
    else
    {
        // printf("Liquid Medicine %d in line %d is NOT in expected range of color\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
        return 0;
    }

    if (liquid_medicines[index_medicine].is_sealed == 1)
    {
        // printf("Liquid Medicine %d in line %d is sealed\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    }
    else
    {
        // printf("Liquid Medicine %d in line %d is NOT sealed\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
        return 0;
    }

    if (liquid_medicines[index_medicine].is_label_placed == 1)
    {
        // printf("Liquid Medicine %d in line %d is labeled\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    }
    else
    {
        // printf("Liquid Medicine %d in line %d is NOT labeled\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
        return 0;
    }
    // printf("Liquid Medicine %d in line %d is passed the inspected successfully\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    return 1;
}

void package_medcine(Liquid_Medicine *liquid_medicines, int index_medicine)
{
    printf("Liquid Medicine %d in line %d is packaging\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    liquid_medicines[index_medicine].is_prescription_placed = 1;
    liquid_medicines[index_medicine].is_medicine_placed = 1;
}
