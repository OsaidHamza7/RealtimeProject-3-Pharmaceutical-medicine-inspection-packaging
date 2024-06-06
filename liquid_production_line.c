#include "header.h"
#include "ipcs.h"
//***********************************************************************************
// Function Prototypes
void getInformation(char **argv);
void init_signals_handlers();
void employee(void *args);
void createLiquidMedicines();
int get_index_of_uninspected_medicine(Bottle_Liquid_Medicine *bottles);
int inspect_medicine(Bottle_Liquid_Medicine *bottles, int index_medicine, int index_expected_medicine);
void package_medcine(Bottle_Liquid_Medicine *bottles, int index_medicine);
void printLiquidMedicines();
int get_index_expected_medicine(int indx);
//***********************************************************************************
int production_line_num;
int num_of_liquid_production_lines;

int range_of_speed[2];
int range_level_liq_medicine[2];
int range_color_liq_medicine[2];
int range_expected_liq_medicine_level[2];
int range_expected_liq_medicine_color[2];
int number_of_employees[2];
int is_there_sucess_inspected_medicine = 0;
int max_produce_time = 10;

Liquid_Production_Line *liquid_production_line;
Liquid_Production_Line *temp;
Liquid_Production_Line *liquid_production_lines;

char *shmptr_liquid_production_lines;
char *shmptr_num_liquid_medicines_produced;
char *shmptr_num_liquid_medicines_failed;
char *shmptr_num_liquid_medicines_packaged;
char *shmptr_liquid_medicines;

int sem_liquid_production_lines;
int sem_num_liquid_medicines_produced;
int sem_num_pill_medicines_failed;
int num_liq_meds = 0;

sem_t mutex_liquid_midicines;
Liq_Med *liq_medicines;

int main(int argc, char **argv)
{
    // check the number of arguments
    if (argc < 10)
    {
        perror("The user should pass an argument like:production_line_num,num_of_liquid_production_lines, str_num_employees, str_range_of_speeds, str_range_level_liquid_medicine, str_range_color_liquid_medicine,num_liq_meds\n");
        exit(-1);
    }

    srand((unsigned)getpid()); // seed for the random function with the ID of the current process
    production_line_num = atoi(argv[1]);
    num_of_liquid_production_lines = atoi(argv[2]);
    num_liq_meds = atoi(argv[9]);

    // Open a shared memories
    shmptr_liquid_production_lines = createSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, num_of_liquid_production_lines * sizeof(struct Liquid_Production_Line), "liquid_production_line.c");
    liquid_production_lines = (struct Liquid_Production_Line *)shmptr_liquid_production_lines;

    shmptr_liquid_medicines = createSharedMemory(SHKEY_LIQUID_MEDICINES, num_liq_meds * sizeof(Liq_Med), "liquid_production_line.c");
    liq_medicines = (struct Liq_Med *)shmptr_liquid_medicines;

    shmptr_num_liquid_medicines_produced = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PRODUCED, sizeof(int), "liquid_production_line.c");
    shmptr_num_liquid_medicines_failed = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_FAILED, sizeof(int), "liquid_production_line.c");
    shmptr_num_liquid_medicines_packaged = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PACKAGED, sizeof(int), "liquid_production_line.c");

    // Open the semaphores
    sem_liquid_production_lines = createSemaphore(SEMKEY_LIQUID_PRODUCTION_LINES, 1, 1, "liquid_production_line.c");
    // 0 :shared between threads 1 :shared between process

    // get information from the arguments
    getInformation(argv);
    // printLiquidMedicines();

    // Thread to create the liquid medicines
    pthread_t create_liquid_medicine_thread;
    pthread_create(&create_liquid_medicine_thread, NULL, (void *)createLiquidMedicines, NULL);

    sem_init(&mutex_liquid_midicines, 0, 1);

    // creats threads for employees
    pthread_t employees[liquid_production_line->production_line.num_employes];
    int employee_id[liquid_production_line->production_line.num_employes];

    // create thread for each employee
    for (int i = 0; i < liquid_production_line->production_line.num_employes; i++)
    {
        employee_id[i] = i + 1;
        pthread_create(&employees[i], NULL, (void *)employee, (void *)&employee_id[i]);
    }

    // wait for the threads to finish
    for (int i = 0; i < liquid_production_line->production_line.num_employes; i++)
    {
        pthread_join(employees[i], NULL);
    }

    pthread_join(create_liquid_medicine_thread, NULL);

    sem_destroy(&mutex_liquid_midicines);
    return 0;
}

void getInformation(char **argv)
{
    // get the information of the liquid production line from the arguments
    liquid_production_line = &liquid_production_lines[production_line_num - 1];
    temp = liquid_production_line;
    split_string(argv[3], number_of_employees);
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
    liquid_production_line->production_line.id = production_line_num;
    liquid_production_line->production_line.num_employes = get_random_number(number_of_employees[0], number_of_employees[1]);
    liquid_production_line->production_line.speed = get_random_number(range_of_speed[0], range_of_speed[1]);

    releaseSem(sem_liquid_production_lines, 0, "liquid_production_line.c");

    printf("Liquid Production Line %d is created with %d employees, and speed %d\n\n", liquid_production_line->production_line.id, liquid_production_line->production_line.num_employes, liquid_production_line->production_line.speed);
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
        // inspect the uninspected medicines
        sem_wait(&mutex_liquid_midicines);
        printf("Employee %d in liquid line %d go to take a new medicine\n", *emp_id, liquid_production_line->production_line.id);
        int index_uninspected_medicine = get_index_of_uninspected_medicine(liquid_production_line->bottles);
        sem_post(&mutex_liquid_midicines);

        // there is no medicines to inspect
        if (index_uninspected_medicine == -1)
        {
            printf("Liquid line %d has no medicines\n", liquid_production_line->production_line.id);
            sleep(1);
            continue;
        }

        int index_expected_medicine = get_index_expected_medicine(index_uninspected_medicine);

        printf("index_expected_medicine %d\n", index_expected_medicine);
        printf("Employee %d in liquid line %d inspects the medicine %d\n", *emp_id, liquid_production_line->production_line.id, index_uninspected_medicine + 1);
        int j = inspect_medicine(liquid_production_line->bottles, index_uninspected_medicine, index_expected_medicine);
        sleep(20); // sleep for 3 seconds to simulate the inspection process

        // the inspection for this medicine is failed
        if (j == 0)
        {
            printf("Bottle Liquid Medicine %d in line %d is failed\n", liquid_production_line->bottles[index_uninspected_medicine].id, liquid_production_line->production_line.id);
            liquid_production_line->bottles[index_uninspected_medicine].is_failed = 1;
            *shmptr_num_liquid_medicines_failed += 1;
            continue;
        }
        printf("Liquid Medicine %d in line %d is passed the inspection successfully\n", liquid_production_line->bottles[index_uninspected_medicine].id, liquid_production_line->production_line.id);

        // the inspection is successful go to the packaging
        // packaging the medicines

        printf("Employee %d in liquid line %d packages the medicine \n", *emp_id, liquid_production_line->production_line.id);
        package_medcine(liquid_production_line->bottles, index_uninspected_medicine);
        sleep(3); // sleep for 3 seconds to simulate the packaging process
        *shmptr_num_liquid_medicines_packaged += 1;
        printf("Liquid Medicine %d in line %d is packaged successfully\n", liquid_production_line->bottles[index_uninspected_medicine].id, liquid_production_line->production_line.id);
        fflush(stdout);
    }
}

void createLiquidMedicines()
{
    int j = 0;
    while (1)
    {
        int time = max_produce_time - (liquid_production_line->production_line.speed * max_produce_time) / 100;
        time = (time < 2) ? 2 : time;
        sleep(time); // sleep for some seconds to simulate the production of the medicines
        acquireSem(sem_liquid_production_lines, 0, "liquid_production_line.c");

        liquid_production_line->bottles[j].id = j + 1;
        liquid_production_line->bottles[j].liquid_medicine.level = get_random_number(range_level_liq_medicine[0], range_level_liq_medicine[1]);
        liquid_production_line->bottles[j].liquid_medicine.color = get_random_number(range_color_liq_medicine[0], range_color_liq_medicine[1]);
        liquid_production_line->bottles[j].expiry_date = generate_random_date();
        liquid_production_line->bottles[j].is_sealed = rand() % 10 < 8 ? 1 : 0;
        liquid_production_line->bottles[j].is_label_placed = rand() % 10 < 8 ? 1 : 0;
        liquid_production_line->bottles[j].is_date_printed = rand() % 10 < 8 ? 1 : 0;

        int indx = rand() % num_liq_meds;
        liquid_production_line->bottles[j].label = liq_medicines[indx].label;

        liquid_production_line->production_line.num_produced_medicines += 1;
        *shmptr_num_liquid_medicines_produced += 1;

        releaseSem(sem_liquid_production_lines, 0, "liquid_production_line.c");
        // create the medicine successfully
        printf("Created Bottle %d Label %s in line %d,is_sealed %d, is_label_placed %d,is_date_printed %d with liquid medicine level %d and color %d.\n", liquid_production_line->bottles[j].id, liquid_production_line->bottles[j].label.str, liquid_production_line->production_line.id, liquid_production_line->bottles[j].is_sealed, liquid_production_line->bottles[j].is_label_placed, liquid_production_line->bottles[j].is_date_printed, liquid_production_line->bottles[j].liquid_medicine.level, liquid_production_line->bottles[j].liquid_medicine.color);
        fflush(stdout);
        j++;
    }
}

int get_index_of_uninspected_medicine(Bottle_Liquid_Medicine *bottles)
{
    for (int i = 0; i < liquid_production_line->production_line.num_produced_medicines; i++)
    {
        if (bottles[i].is_inspected == 0)
        {
            printf("Liquid Medicine %d label %s in line %d is inspecting\n", bottles[i].id, bottles[i].label.str, liquid_production_line->production_line.id);
            sleep(1);
            bottles[i].is_inspected = 1;
            return i; // return the index of the uninspected medicine to inspect it
        }
    }
    return -1;
}

int inspect_medicine(Bottle_Liquid_Medicine *bottles, int index_medicine, int index_expected_medicine)
{

    // print expected medicine info
    printf("Expected Liquid Medicine %s with level %d-%d and color %d-%d\n", liq_medicines[index_expected_medicine].label.str, liq_medicines[index_expected_medicine].min_level, liq_medicines[index_expected_medicine].max_level, liq_medicines[index_expected_medicine].min_color, liq_medicines[index_expected_medicine].max_color);

    if (bottles[index_medicine].liquid_medicine.level >= liq_medicines[index_expected_medicine].min_level && bottles[index_medicine].liquid_medicine.level <= liq_medicines[index_expected_medicine].max_level)
    {
        // printf("Liquid Medicine %d in line %d is in expected range of level\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    }
    else
    {
        // printf("Liquid Medicine %d in line %d is NOT in expected range of level\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
        return 0;
    }

    if (bottles[index_medicine].liquid_medicine.color >= liq_medicines[index_expected_medicine].min_color && bottles[index_medicine].liquid_medicine.color <= liq_medicines[index_expected_medicine].max_color)
    {
        // printf("Liquid Medicine %d in line %d is in expected range of color\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    }
    else
    {
        // printf("Liquid Medicine %d in line %d is NOT in expected range of color\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
        return 0;
    }

    if (bottles[index_medicine].is_sealed == 1)
    {
        // printf("Liquid Medicine %d in line %d is sealed\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    }
    else
    {
        // printf("Liquid Medicine %d in line %d is NOT sealed\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
        return 0;
    }

    if (bottles[index_medicine].is_label_placed == 1)
    {
        // printf("Liquid Medicine %d in line %d is labeled\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    }
    else
    {
        // printf("Liquid Medicine %d in line %d is NOT labeled\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
        return 0;
    }

    if (bottles[index_medicine].is_date_printed == 1)
    {
        // printf("Liquid Medicine %d in line %d is printed with date\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    }
    else
    {
        // printf("Liquid Medicine %d in line %d is NOT printed with date\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
        return 0;
    }

    // printf("Liquid Medicine %d in line %d is passed the inspected successfully\n", liquid_medicines[index_medicine].id, liquid_medicines[index_medicine].production_line_num);
    return 1;
}

void package_medcine(Bottle_Liquid_Medicine *bottles, int index_medicine)
{
    printf("Bottle Liquid Medicine %d in line %d is packaging\n", bottles[index_medicine].id, liquid_production_line->production_line.id);
    bottles[index_medicine].is_liquid_medicine_placed = 1;
    bottles[index_medicine].is_prescription_placed = 1;
}

void printLiquidMedicines()
{
    printf("**********Liquid Medicines*********\n");
    for (int i = 0; i < num_liq_meds; i++)
    {
        printf("Liquid Medicine %d:\n", i + 1);
        printf("Label: %s\n", liq_medicines[i].label.str);
        printf("Min Level: %d\n", liq_medicines[i].min_level);
        printf("Max Level: %d\n", liq_medicines[i].max_level);
        printf("Min Color: %d\n", liq_medicines[i].min_color);
        printf("Max Color: %d\n", liq_medicines[i].max_color);
    }
}

int get_index_expected_medicine(int indx)
{
    for (int i = 0; i < num_liq_meds; i++)
    {
        if (strcmp(liq_medicines[i].label.str, liquid_production_line->bottles[indx].label.str) == 0)
        {
            return i;
        }
    }
    return -1;
}