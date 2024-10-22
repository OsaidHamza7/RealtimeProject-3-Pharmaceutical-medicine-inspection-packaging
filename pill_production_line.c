#include "header.h"
#include "ipcs.h"

//***********************************************************************************
// Function Prototypes
void getInformation(char **argv);
void init_signals_handlers();
void employee(void *args);
void createPillMedicines();
int make_inspection(int index_medicine, int index_expected_medicine);
void packaging(Pill_Medicine *pill_medicines, int index_medicine);
int get_index_of_uninspected_medicine(Pill_Medicine *pill_medicines);
int get_index_expected_medicine(int indx);
void printPillMedicine(Pill_Med *pill_medicines);
void increase_employees();
void decrease_employees();
//***********************************************************************************

int production_line_num;
int num_of_pill_production_lines;
int number_of_employees[2];
int range_of_speed[2];
int range_number_of_medicines[2];

int range_plastic_containers[2];
int range_pills[2];

int range_of_size_pills[2];
int range_of_color_pills[2];

Pill_Production_Line *pill_production_line;
Pill_Production_Line *temp;
Pill_Production_Line *pill_production_lines;

pthread_t additional_employees[max_additional_employees];
int additional_employees_head = 0;
int additional_employees_tail = 0;

char *shmptr_pill_production_lines;
char *shmptr_num_pill_medicines_produced;
char *shmptr_num_pill_medicines_failed;
char *shmptr_num_pill_medicines_packaged;
char *shmptr_pill_medicines;

int sem_pill_production_lines;
int sem_num_pill_medicines_produced;
int sem_num_pill_medicines_failed;
int sem_num_pill_medicines_packaged;

sem_t mutex_pill_medcines;
int max_produce_time = 10;
int num_pill_meds;

Pill_Med *pill_medicines;

int main(int argc, char **argv)
{
    // check the number of arguments
    if (argc < 11)
    {
        perror("The user should pass an argument like:production_line_num,num_pill_production_lines,num_employees,range_num_midicines,range_of_speeds,range_of_plastic_containers,range_of_pills,range_size_pill,range_color_pill,num_pill_meds\n");
        exit(-1);
    }

    srand((unsigned)getpid()); // seed for the random function with the ID of the current process
    production_line_num = atoi(argv[1]);
    num_of_pill_production_lines = atoi(argv[2]);
    num_pill_meds = atoi(argv[10]);

    // Open a shared memories
    shmptr_pill_production_lines = createSharedMemory(SHKEY_PILL_PRODUCTION_LINES, num_of_pill_production_lines * sizeof(struct Pill_Production_Line), "pill_production_line.c");
    pill_production_lines = (struct Pill_Production_Line *)shmptr_pill_production_lines;

    shmptr_pill_medicines = createSharedMemory(SHKEY_PILL_MEDICINES, num_pill_meds * sizeof(Pill_Med), "pill_production_line.c");
    pill_medicines = (struct Pill_Med *)shmptr_pill_medicines;

    shmptr_num_pill_medicines_produced = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_PRODUCED, sizeof(int), "pill_production_line.c");
    shmptr_num_pill_medicines_failed = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_FAILED, sizeof(int), "pill_production_line.c");
    shmptr_num_pill_medicines_packaged = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_PACKAGED, sizeof(int), "pill_production_line.c");

    // Open the semaphores
    sem_pill_production_lines = createSemaphore(SEMKEY_PILL_PRODUCTION_LINES, 1, 1, "pill_production_line.c");
    sem_num_pill_medicines_produced = createSemaphore(SEMKEY_NUM_PILL_MEDICINES_PRODUCED, 1, 1, "pill_production_line.c");
    sem_num_pill_medicines_failed = createSemaphore(SEMKEY_NUM_PILL_MEDICINES_FAILED, 1, 1, "pill_production_line.c");
    sem_num_pill_medicines_packaged = createSemaphore(SEMKEY_NUM_PILL_MEDICINES_PACKAGED, 1, 1, "pill_production_line.c");

    // get information from the arguments
    getInformation(argv);

    // Thread to create the pill medicines
    pthread_t create_pill_medicine_thread;
    pthread_create(&create_pill_medicine_thread, NULL, (void *)createPillMedicines, NULL);

    sem_init(&mutex_pill_medcines, 0, 1);
    // creats threads for employees
    pthread_t employees[pill_production_line->production_line.num_employes];

    int employee_id[pill_production_line->production_line.num_employes];
    // create thread for each employee

    for (int i = 0; i < pill_production_line->production_line.num_employes; i++)
    {
        employee_id[i] = i + 1;
        pthread_create(&employees[i], NULL, (void *)employee, (void *)&employee_id[i]);
    }

    // wait for the threads to finish
    for (int i = 0; i < pill_production_line->production_line.num_employes; i++)
    {
        pthread_join(employees[i], NULL);
    }

    while (1)
    {
        pause();
    }

    pthread_join(create_pill_medicine_thread, NULL);
    sem_destroy(&mutex_pill_medcines);

    return 0;
}

void getInformation(char **argv)
{
    // get the information of the pill production line from the arguments
    pill_production_line = &pill_production_lines[production_line_num - 1];
    temp = pill_production_line;
    split_string(argv[3], number_of_employees);
    split_string(argv[4], range_number_of_medicines);
    split_string(argv[5], range_of_speed);
    split_string(argv[6], range_plastic_containers);
    split_string(argv[7], range_pills);
    split_string(argv[8], range_of_size_pills);
    split_string(argv[9], range_of_color_pills);

    acquireSem(sem_pill_production_lines, 0, "pill_production_line.c");

    pill_production_line->pid = getpid();
    pill_production_line->production_line.id = production_line_num;
    pill_production_line->production_line.num_employes = get_random_number(number_of_employees[0], number_of_employees[1]);
    pill_production_line->production_line.original_num_employes = pill_production_line->production_line.num_employes;
    pill_production_line->production_line.speed = get_random_number(range_of_speed[0], range_of_speed[1]);

    releaseSem(sem_pill_production_lines, 0, "pill_production_line.c");

    printf("Pill Production Line %d is created with %d employees,and speed %d\n\n", production_line_num, pill_production_line->production_line.num_employes, pill_production_line->production_line.speed);
    fflush(stdout);
}

void init_signals_handlers()
{
    // signal handler for SIGUSR1 to increase the number of employees by 1
    if (signal(SIGUSR1, increase_employees) == SIG_ERR)
    {
        perror("Signal Error\n");
        exit(-1);
    }

    // signal handler for SIGUSR2 to decrease the number of employees by 1
    if (signal(SIGUSR2, decrease_employees) == SIG_ERR)
    {
        perror("Signal Error\n");
        exit(-1);
    }
}

void increase_employees()
{
    acquireSem(sem_pill_production_lines, 0, "pill_production_line.c");

    // create thread for the new employee
    int emp_id = pill_production_line->production_line.num_employes;
    pthread_create(&additional_employees[additional_employees_tail], NULL, (void *)employee, (void *)&emp_id);
    additional_employees_tail = (additional_employees_tail + 1) % max_additional_employees;

    releaseSem(sem_pill_production_lines, 0, "pill_production_line.c");
    printf("Pill Production Line %d: Number of employees increased to %d\n", pill_production_line->production_line.id, pill_production_line->production_line.num_employes);
    fflush(stdout);
}

void decrease_employees()
{
    acquireSem(sem_pill_production_lines, 0, "pill_production_line.c");

    // kill first additional employee
    pthread_cancel(additional_employees[additional_employees_head]);
    additional_employees_head = (additional_employees_head + 1) % max_additional_employees;

    releaseSem(sem_pill_production_lines, 0, "pill_production_line.c");
    printf("Pill Production Line %d: Number of employees decreased to %d\n", pill_production_line->production_line.id, pill_production_line->production_line.num_employes);
    fflush(stdout);
}

void employee(void *args)
{
    int *emp_id = (int *)args;
    while (1)
    {
        // inspect the uninspected medicines
        sem_wait(&mutex_pill_medcines);
        // printf("Employee %d in pill line %d go to take a new medicine\n", *emp_id, pill_production_line->production_line.id);
        int index_uninspected_medicine = get_index_of_uninspected_medicine(pill_production_line->pill_medicines);
        sem_post(&mutex_pill_medcines);

        if (index_uninspected_medicine == -1) // there is no uninspected medicines
        {
            // printf("pill line %d has no uninspected medicines\n", pill_production_line->production_line.id);
            sleep(1);
            continue;
        }

        int index_expected_medicine = get_index_expected_medicine(index_uninspected_medicine);
        // printf("index_expected_medicine %d\n", index_expected_medicine);

        printf("Employee %d in pill line %d inspects the medicine %d\n", *emp_id, pill_production_line->production_line.id, index_uninspected_medicine + 1);
        int j = make_inspection(index_uninspected_medicine, index_expected_medicine);
        sleep(5); // sleep for 5 seconds to simulate the inspection process
        printf("Employee %d in pill line %d finish inspects the medicine %d\n", *emp_id, pill_production_line->production_line.id, index_uninspected_medicine + 1);

        if (j == 0) // the inspection is failed
        {
            printf("pill Medicine %d in line %d is failed\n", pill_production_line->pill_medicines[index_uninspected_medicine].id, pill_production_line->production_line.id);
            pill_production_line->pill_medicines[index_uninspected_medicine].is_failed = 1;
            *shmptr_num_pill_medicines_failed += 1;
            continue;
        }

        printf("Pill Medicine %d in line %d is passed the inspection successfully\n", pill_production_line->pill_medicines[index_uninspected_medicine].id, pill_production_line->production_line.id);
        // packaging the medicines
        printf("Employee %d in pill line %d packages the medicine\n", *emp_id, pill_production_line->production_line.id);
        packaging(pill_production_line->pill_medicines, index_uninspected_medicine);
        sleep(1); // sleep for 1 seconds to simulate the packaging process
        printf("pill Medicine %d in line %d is packaged successfully\n", pill_production_line->pill_medicines[index_uninspected_medicine].id, pill_production_line->production_line.id);
        fflush(stdout);
    }
}

void createPillMedicines()
{
    int i = 0;
    while (1)
    {
        int time = max_produce_time - (pill_production_line->production_line.speed * max_produce_time) / 100;
        time = (time < 2) ? 2 : time;
        sleep(time);
        acquireSem(sem_pill_production_lines, 0, "pill_production_line.c");

        pill_production_line->pill_medicines[i].id = i + 1;
        pill_production_line->pill_medicines[i].num_plastic_containers = get_random_number(range_plastic_containers[0], range_plastic_containers[1]);
        pill_production_line->pill_medicines[i].is_inspected = 0;
        pill_production_line->pill_medicines[i].plastic_containers[0].num_pills = get_random_number(range_pills[0], range_pills[1]);
        pill_production_line->pill_medicines[i].plastic_containers[0].pills[0].color = get_random_number(range_of_color_pills[0], range_of_color_pills[1]);
        pill_production_line->pill_medicines[i].plastic_containers[0].pills[0].size = get_random_number(range_of_size_pills[0], range_of_size_pills[1]);
        pill_production_line->pill_medicines[i].plastic_containers[0].is_date_printed = rand() % 20 < 18 ? 1 : 0;

        for (int j = 0; j < pill_production_line->pill_medicines[i].num_plastic_containers; j++)
        {
            pill_production_line->pill_medicines[i].plastic_containers[j].id = j + 1;
            pill_production_line->pill_medicines[i].plastic_containers[j].num_pills = pill_production_line->pill_medicines[i].plastic_containers[0].num_pills;
            pill_production_line->pill_medicines[i].plastic_containers[j].is_date_printed = pill_production_line->pill_medicines[i].plastic_containers[0].is_date_printed;
            for (int k = 0; k < pill_production_line->pill_medicines[i].plastic_containers[j].num_pills; k++)
            {
                pill_production_line->pill_medicines[i].plastic_containers[j].pills[k].id = k + 1;
                pill_production_line->pill_medicines[i].plastic_containers[j].pills[k].color = pill_production_line->pill_medicines[i].plastic_containers[0].pills[0].color;
                pill_production_line->pill_medicines[i].plastic_containers[j].pills[k].size = pill_production_line->pill_medicines[i].plastic_containers[0].pills[0].size;
            }
        }

        pill_production_line->pill_medicines[i].expiry_date = generate_random_date();

        int indx = rand() % num_pill_meds;
        pill_production_line->pill_medicines[i].label = pill_medicines[indx].label;

        pill_production_line->production_line.num_produced_medicines += 1;
        *shmptr_num_pill_medicines_produced += 1;

        releaseSem(sem_pill_production_lines, 0, "pill_production_line.c");
        // print the pill based medicine information

        printf("Created Pill Medicine %d Label %s in line %d,with num plastic containers %d \n", pill_production_line->pill_medicines[i].id, pill_production_line->pill_medicines[i].label.str, pill_production_line->production_line.id, pill_production_line->pill_medicines[i].num_plastic_containers);

        /*for (int j = 0; j < pill_production_line->pill_medicines[i].num_plastic_containers; j++)
        {
            printf("Plastic container %d, number of bills %d\n", pill_production_line->pill_medicines[i].plastic_containers[j].id, pill_production_line->pill_medicines[i].plastic_containers[j].num_pills);
            for (int k = 0; k < pill_production_line->pill_medicines[i].plastic_containers[j].num_pills; k++)
            {
                printf("Pill %d, color %d, size %d\n", pill_production_line->pill_medicines[i].plastic_containers[j].pills[k].id, pill_production_line->pill_medicines[i].plastic_containers[j].pills[k].color, pill_production_line->pill_medicines[i].plastic_containers[j].pills[k].size);
            }
        }

        printf("=====================================================================\n");
        fflush(stdout);*/

        i++;
    }
}

// Make an inspection function for the pill production line to check No plastic container is missing any pill, Pills in the plastic containers have the correct color and size and Medicine expiry date is clearly printed on the plastic container label
int make_inspection(int index_medicine, int index_expected_medicine)
{

    // check for each medicine in the production line
    // printf("Expected Pill Medicine %s with num of plastic containers %d, num of pills %d, range of size %d - %d, range of color %d - %d\n", pill_medicines[index_expected_medicine].label.str, pill_medicines[index_expected_medicine].num_containers, pill_medicines[index_expected_medicine].num_pills, pill_medicines[index_expected_medicine].min_size, pill_medicines[index_expected_medicine].max_size, pill_medicines[index_expected_medicine].min_color, pill_medicines[index_expected_medicine].max_color);
    printf("num of plastic containers %d\n", pill_production_line->pill_medicines[index_medicine].num_plastic_containers);

    if (pill_production_line->pill_medicines[index_medicine].num_plastic_containers == pill_medicines[index_expected_medicine].num_containers)
    {
        // printf("Pill Medicine %d in the production line %d is in expected range of the number for plastic containers\n\n", pill_medicines[index_medicine].id, pill_medicines[index_medicine].production_line_num);
    }
    else
    {
        printf("NOT in expected range of number of plastic containers\n");
        // printf("Pill Medicine %d in line %d is NOT in expected range of number of plastic containers\n", pill_medicines[index_medicine].id, pill_medicines[index_medicine].production_line_num);
        return 0;
    }

    for (int j = 0; j < pill_production_line->pill_medicines[index_medicine].num_plastic_containers; j++)
    {
        if (pill_production_line->pill_medicines[index_medicine].plastic_containers[j].num_pills == pill_medicines[index_expected_medicine].num_pills)
        {
            // printf("Pill Medicine %d in line %d is in expected range of number of pills\n", pill_medicines[index_medicine].id, pill_medicines[index_medicine].production_line_num);
        }
        else
        {
            printf("NOT in expected range of number of pills\n");
            // printf("Pill Medicine %d in line %d is NOT in expected range of number of pills\n", pill_medicines[index_medicine].id, pill_medicines[index_medicine].production_line_num);
            return 0;
        }

        if (pill_production_line->pill_medicines[index_medicine].plastic_containers[j].is_date_printed == 0)
        {
            // printf("Pill Medicine %d in line %d does NOT have an expiry date in thE back of the container\n", pill_medicines[index_medicine].id, pill_medicines[index_medicine].production_line_num);
            printf("NOT have an expiry date\n");
            return 0;
        }

        // Pills in the plastic containers have the correct color and size
        for (int k = 0; k < pill_production_line->pill_medicines[index_medicine].plastic_containers[j].num_pills; k++)
        {
            if (pill_production_line->pill_medicines[index_medicine].plastic_containers[j].pills[k].color >= pill_medicines[index_expected_medicine].min_color && pill_production_line->pill_medicines[index_medicine].plastic_containers[j].pills[k].color <= pill_medicines[index_expected_medicine].max_color)
            {
                // printf("Pill Medicine %d in line %d is in expected range of color of pills\n", pill_medicines[index_medicine].id, pill_medicines[index_medicine].production_line_num);
            }
            else
            {
                printf("NOT in expected range of color of pills\n");
                // printf("Pill Medicine %d in line %d is NOT in expected range of color of pills\n", pill_medicines[index_medicine].id, pill_medicines[index_medicine].production_line_num);
                return 0;
            }

            if (pill_production_line->pill_medicines[index_medicine].plastic_containers[j].pills[k].size >= pill_medicines[index_expected_medicine].min_size && pill_production_line->pill_medicines[index_medicine].plastic_containers[j].pills[k].size <= pill_medicines[index_expected_medicine].max_size)
            {
                // printf("Pill Medicine %d in line %d is in expected range of size of pills\n", pill_medicines[index_medicine].id, pill_medicines[index_medicine].production_line_num);
            }
            else
            {
                printf("NOT in expected range of size of pills\n");
                // printf("Pill Medicine %d in line %d is NOT in expected range of size of pills\n", pill_medicines[index_medicine].id, pill_medicines[index_medicine].production_line_num);
                return 0;
            }
        }
    }

    return 1;
}

void packaging(Pill_Medicine *pill_medicines, int index_medicine)
{
    printf("Pill Medicine %d in line %d is packaging\n", pill_medicines[index_medicine].id, pill_production_line->production_line.id);
    // pill_medicines[index_medicine].is_prescription_placed = rand() % 20 < 18 ? 1 : 0;
    pill_medicines[index_medicine].is_prescription_placed = 1;

    if (pill_medicines[index_medicine].is_prescription_placed == 1)
    {
        pill_medicines[index_medicine].is_packaged = 1;
        *shmptr_num_pill_medicines_packaged += 1;
    }
    else
    {
        pill_medicines[index_medicine].is_failed = 1;
        *shmptr_num_pill_medicines_failed += 1;
    }
}

int get_index_of_uninspected_medicine(Pill_Medicine *pill_medicines)
{
    for (int i = 0; i < pill_production_line->production_line.num_produced_medicines; i++)
    {
        if (pill_medicines[i].is_inspected == 0)
        {
            sleep(1);
            pill_medicines[i].is_inspected = 1; // set the medicine as inspected
            return i;                           // return the index of the uninspected medicine to inspect it
        }
    }
    return -1;
}

int get_index_expected_medicine(int indx)
{
    for (int i = 0; i < num_pill_meds; i++)
    {
        if (strcmp(pill_medicines[i].label.str, pill_production_line->pill_medicines[indx].label.str) == 0)
        {
            return i;
        }
    }
    return -1;
}