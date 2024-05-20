#include "header.h"
#include "ipcs.h"

//***********************************************************************************
// Function Prototypes
void getInformation(char **argv);
void init_signals_handlers();
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

    // while (1)
    // {
    // }
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
