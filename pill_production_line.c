#include "header.h"
#include "ipcs.h"

//***********************************************************************************
// Function Prototypes
void getInformation(char **argv);
void init_signals_handlers();
void employee(void *args);
void createPillMedicines();
int make_inspection(Pill_Production_Line *pill_Production_Line, int index_medicine);
void packaging(Pill_Production_Line *pill_Production_Line, int index_medicine);
int get_index_of_uninspected_medicine(Pill_Medicine *pill_medicines);
//***********************************************************************************

int production_line_num;
int num_of_pill_production_lines;
int number_of_employees;
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
char *shmptr_num_pill_medicines_produced;
char *shmptr_num_pill_medicines_failed;
char *shmptr_num_pill_medicines_packaged;

int sem_pill_production_lines;
int sem_num_pill_medicines_produced;
int sem_num_pill_medicines_failed;

sem_t mutex_pill_medcines;

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

    shmptr_num_pill_medicines_produced = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_PRODUCED, sizeof(int), "pill_production_line.c");
    shmptr_num_pill_medicines_failed = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_FAILED, sizeof(int), "pill_production_line.c");
    shmptr_num_pill_medicines_packaged = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_PACKAGED, sizeof(int), "liquid_production_line.c");

    // Open the semaphores
    sem_pill_production_lines = createSemaphore(SEMKEY_PILL_PRODUCTION_LINES, 1, 1, "pill_production_line.c");

    // get information from the arguments
    getInformation(argv);

    // Thread to create the pill medicines
    pthread_t create_pill_medicine_thread;
    pthread_create(&create_pill_medicine_thread, NULL, (void *)createPillMedicines, NULL);

    sem_init(&mutex_pill_medcines, 0, 1);
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

    sem_destroy(&mutex_pill_medcines);

    return 0;
}

void getInformation(char **argv)
{
    // get the information of the pill production line from the arguments
    pill_production_line = &pill_production_lines[production_line_num - 1];
    temp = pill_production_line;
    number_of_employees = atoi(argv[3]);
    split_string(argv[4], range_number_of_medicines);
    split_string(argv[5], range_of_speed);
    split_string(argv[6], range_plastic_containers);
    split_string(argv[7], range_pills);
    split_string(argv[8], range_size_pills);
    split_string(argv[9], range_color_pills);

    printf("=====================================================================\n");
    // print the information of the pill production line
    printf("Pill Production Line %d\n", production_line_num);
    printf("Number of employees: %d\n", number_of_employees);
    printf("Speed: %d - %d\n", range_of_speed[0], range_of_speed[1]);
    printf("Plastic Containers: %d - %d\n", range_plastic_containers[0], range_plastic_containers[1]);
    printf("Pills: %d - %d\n", range_pills[0], range_pills[1]);
    printf("Size of Pills: %d - %d\n", range_size_pills[0], range_size_pills[1]);
    printf("Color of Pills: %d - %d\n\n", range_color_pills[0], range_color_pills[1]);
    fflush(stdout);

    acquireSem(sem_pill_production_lines, 0, "pill_production_line.c");

    pill_production_line->pid = getpid();
    pill_production_line->num = production_line_num;
    pill_production_line->num_employes = number_of_employees;
    pill_production_line->num_medicines = 0;
    pill_production_line->speed = get_random_number(range_of_speed[0], range_of_speed[1]);

    releaseSem(sem_pill_production_lines, 0, "pill_production_line.c");

    printf("Pill Production Line %d is created with %d employees, %d medicines, and speed %d\n\n", pill_production_line->num, pill_production_line->num_employes, pill_production_line->num_medicines, pill_production_line->speed);
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
        sem_wait(&mutex_pill_medcines);
        printf("Employee %d in pill line %d go to take a new medicine\n", *emp_id, pill_production_line->num);
        int index_medicine = get_index_of_uninspected_medicine(pill_production_line->pill_medicines);
        sem_post(&mutex_pill_medcines);

        if (index_medicine == -1) // there is no uninspected medicines
        {
            printf("pill line %d has no uninspected medicines\n", pill_production_line->num);
            sleep(1);
            continue;
        }

        printf("Employee %d in liquid line %d inspects the medicine %d\n", *emp_id, pill_production_line->num, index_medicine + 1);
        int j = make_inspection(pill_production_line->pill_medicines, index_medicine);
        sleep(3); // sleep for 3 seconds to simulate the inspection process

        if (j == 0) // the inspection is failed
        {
            printf("pill Medicine %d in line %d is failed\n", pill_production_line->pill_medicines[index_medicine].id, pill_production_line->pill_medicines[index_medicine].production_line_num);
            pill_production_line->pill_medicines[index_medicine].is_failed = 1;
            *shmptr_num_pill_medicines_failed += 1;
            continue;
        }
        printf("Pill Medicine %d in line %d is passed the inspection successfully\n", pill_production_line->pill_medicines[index_medicine].id, pill_production_line->pill_medicines[index_medicine].production_line_num);

        // the inspection is successful go to the packaging
        // packaging the medicines

        printf("Employee %d in pill line %d packages the medicine\n", *emp_id, pill_production_line->num);
        packaging(pill_production_line->pill_medicines, index_medicine);
        sleep(3); // sleep for 3 seconds to simulate the packaging process
        *shmptr_num_pill_medicines_packaged += 1;
        printf("pill Medicine %d in line %d is packaged successfully\n", pill_production_line->pill_medicines[index_medicine].id, pill_production_line->num);
        fflush(stdout);
    }
}

void createPillMedicines()
{
    int i = 0;
    while (1)
    {
        acquireSem(sem_pill_production_lines, 0, "pill_production_line.c");

        pill_production_line->pill_medicines[i].id = i + 1;
        pill_production_line->pill_medicines[i].production_line_num = pill_production_line->num;
        pill_production_line->pill_medicines[i].num_plastic_containers = get_random_number(range_of_plastic_containers[0], range_of_plastic_containers[1]);
        for (int j = 0; j < pill_production_line->pill_medicines->num_plastic_containers; j++)
        {
            pill_production_line->pill_medicines->plastic_containers[j].id = j + 1;
            pill_production_line->pill_medicines->plastic_containers[j].num_pills = get_random_number(range_of_pills[0], range_of_pills[1]);
            for (int k = 0; k < pill_production_line->pill_medicines->plastic_containers->num_pills; k++)
            {
                pill_production_line->pill_medicines->plastic_containers->pills[k].id = k + 1;
                pill_production_line->pill_medicines->plastic_containers->pills[k].color = get_random_number(range_color_pill[0], range_color_pill[1]);
                pill_production_line->pill_medicines->plastic_containers->pills[k].size = get_random_number(range_size_pill[0], range_size_pill[1]);
            }
        }
        pill_production_line->pill_medicines[i].Expiry_date = generate_random_date();
        pill_production_line->pill_medicines[i].plastic_containers->date_is_printed = get_random_number(0, 1);
        pill_production_line->pill_medicines[i].is_failed = 0;
        pill_production_line->pill_medicines[i].is_inspected = 0;
        pill_production_line->pill_medicines[i].is_packaged = 0;

        releaseSem(sem_pill_production_lines, 0, "pill_production_line.c");
        // print the pill based medicines

        printf("Pill Medicine %d in production line num %d,with num plastic containers %d, with expiry date %d\n", pill_production_line->pill_medicines[i].id, pill_production_line->pill_medicines[i].production_line_num, pill_production_line->pill_medicines[i].num_plastic_containers, pill_production_line->pill_medicines[i].Expiry_date);
        for (int j = 0; j < pill_production_line->pill_medicines->num_plastic_containers; j++)
        {
            printf("Plastic container with id %d, number of bills %d\n", pill_production_line->pill_medicines->plastic_containers[j].id, pill_production_line->pill_medicines->plastic_containers[j].num_pills);
            for (int k = 0; k < pill_production_line->pill_medicines->plastic_containers->num_pills; k++)
            {
                printf("Pill with id %d, color %d, size %d\n", pill_production_line->pill_medicines->plastic_containers->pills[k].id, pill_production_line->pill_medicines->plastic_containers->pills[k].color, pill_production_line->pill_medicines->plastic_containers->pills[k].size);
            }
        }
        printf("=====================================================================\n");
        fflush(stdout);
        sleep(5);
        i++;
    }
}

// Make an inspection function for the pill production line to check No plastic container is missing any pill, Pills in the plastic containers have the correct color and size and Medicine expiry date is clearly printed on the plastic container label
int make_inspection(Pill_Production_Line *pill_Production_Line, int index_medicine)
{
    // check for each medicine in the production line
    printf("Pill Medicine %d in line %d is inspecting\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
    pill_Production_Line->pill_medicines[index_medicine].is_inspected = 1;
    sleep(2); // sleep for 2 seconds to simulate the inspection process
    if (pill_Production_Line->pill_medicines[index_medicine].num_plastic_containers >= range_of_plastic_containers[0] && pill_Production_Line->pill_medicines[index_medicine].num_plastic_containers <= range_of_plastic_containers[1])
    {
        printf("Pill Medicine %d in the production line %d is in expected range of the number for plastic containers\n\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
        fflush(stdout);
    }
    else
    {
        printf("Pill Medicine %d in line %d is NOT in expected range of number of plastic containers\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
        return 0;
    }

    for (int j = 0; j < pill_Production_Line->pill_medicines->num_plastic_containers; j++)
    {
        if (pill_Production_Line->pill_medicines->plastic_containers[index_medicine].num_pills >= range_of_pills[0] && pill_Production_Line->pill_medicines->plastic_containers[index_medicine].num_pills <= range_of_pills[1])
        {
            printf("Pill Medicine %d in line %d is in expected range of number of pills\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
            fflush(stdout);
        }
        else
        {
            printf("Pill Medicine %d in line %d is NOT in expected range of number of pills\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
            return 0;
        }

        // Pills in the plastic containers have the correct color and size
        for (int k = 0; k < pill_Production_Line->pill_medicines->plastic_containers->num_pills; k++)
        {
            if (pill_Production_Line->pill_medicines->plastic_containers->pills[k].color >= range_color_pill[0] && pill_Production_Line->pill_medicines->plastic_containers->pills[k].color <= range_color_pill[1])
            {
                printf("Pill Medicine %d in line %d is in expected range of color of pills\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
                fflush(stdout);
            }
            else
            {
                printf("Pill Medicine %d in line %d is NOT in expected range of color of pills\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
                return 0;
            }

            if (pill_Production_Line->pill_medicines->plastic_containers->pills[k].size >= range_size_pill[0] && pill_Production_Line->pill_medicines->plastic_containers->pills[k].size <= range_size_pill[1])
            {
                printf("Pill Medicine %d in line %d is in expected range of size of pills\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
                fflush(stdout);
            }
            else
            {
                printf("Pill Medicine %d in line %d is NOT in expected range of size of pills\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
                return 0;
            }

            if (pill_Production_Line->pill_medicines[index_medicine].plastic_containers[j].date_is_printed == 0)
            {
                printf("Pill Medicine %d in line %d does NOT have an expiry date in thE back of the container\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
                return 0;
            }
            else
            {
                printf("Pill Medicine %d in line %d has an expiry date in the back of the container\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
                fflush(stdout);
            }
        }
    }

    printf("=====================================================================\n");
    fflush(stdout);

    // print the inspection results
    printf("Inspection results:\n");
    // if the inspection is successful , print that is good , if not create a varaible to add the number of mdedicines that are missing that missed
    printf("Pill production line %d inspection is successful.\n", pill_Production_Line->num);
    fflush(stdout);
    return 1;
}

void packaging(Pill_Production_Line *pill_Production_Line, int index_medicine)
{

    printf("Pill Medicine %d in line %d is packaging\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
    pill_Production_Line->pill_medicines[index_medicine].is_packaged = 1;

    printf("The folded prescriptions is added to Pill Medicine %d in line %d.\n", pill_Production_Line->pill_medicines[index_medicine].id, pill_Production_Line->pill_medicines[index_medicine].production_line_num);
    pill_Production_Line->pill_medicines[index_medicine].prescription_is_added = 1;
}

int get_index_of_uninspected_medicine(Pill_Medicine *pill_medicines)
{
    for (int i = 0; i < pill_production_line->num_medicines; i++)
    {
        if (pill_medicines[i].is_inspected == 0)
        {
            printf("pill Medicine %d in line %d is inspecting\n", pill_medicines[i].id, pill_medicines[i].production_line_num);
            pill_medicines[i].is_inspected = 1; // set the medicine as inspected
            return i;                           // return the index of the uninspected medicine to inspect it
        }
    }
    return -1;
}