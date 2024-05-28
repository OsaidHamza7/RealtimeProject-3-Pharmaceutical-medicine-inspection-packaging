#include "header.h"
#include "ipcs.h"

#define LINES 4
#define MEDICINES_PER_LINE 5
#define LINE_SPACING 120
#define BOX_SPACING 240
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 600
#define TIMER_INTERVAL 2000

enum ProductType
{
    BOTTLE,
    PILL
};

// ProductionLine lines[LINES][MEDICINES_PER_LINE];
// Liquid_Production_Line liquid_lines[LINES];

int liquidMedicinesCount[LINES] = {0}; // Keeps track of the number of medicines in each liquid line
int pillmedicinesCount[LINES] = {0};   // Keeps track of the number of medicines in each pill line
int numFailedBottleMedicine = 0;       // Number of failed bottle medicine
int numSuccessfulBottleMedicine = 0;   // Number of successful bottle medicine
int numFailedPillMedicine = 0;         // Number of failed pill medicine
int numSuccessfulPillMedicine = 0;     // Number of successful pill medicine

char *shmptr_liquid_production_lines;
char *shmptr_num_liquid_medicines_produced;
char *shmptr_num_pill_medicines_failed;

int sem_liquid_production_lines;
int sem_num_liquid_medicines_produced;
int sem_num_pill_medicines_failed;

Liquid_Production_Line *liquid_lines;

void initializeLines()
{
    srand(time(NULL));
    /*for (int i = 0; i < LINES; ++i)
    {
        for (int j = 0; j < MEDICINES_PER_LINE; ++j)
        {
            lines[i][j].type = (i < 4) ? BOTTLE : PILL;
            lines[i][j].failed = rand() % 2; // Randomly determine if the medicine failed or not
            if (lines[i][j].type == BOTTLE)
            {
                lines[i][j].liquidLevel = (rand() % 41) + 10; // Random liquid level between 10 and 50
            }
            else
            {
                lines[i][j].numberOfPills = (rand() % 5) + 1;
            }
        }
    }*/
}

void drawLiquidBottle(float x, float y, float liquidLevel)
{
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    glVertex2f(x - 10.0, y - 25.0);
    glVertex2f(x + 10.0, y - 25.0);
    glVertex2f(x + 10.0, y + 25.0);
    glVertex2f(x - 10.0, y + 25.0);
    glEnd();

    glColor3f(0.7, 0.7, 0.7);
    glBegin(GL_QUADS);
    glVertex2f(x - 10.0, y + 25.0);
    glVertex2f(x + 10.0, y + 25.0);
    glVertex2f(x + 10.0, y + 30.0);
    glVertex2f(x - 10.0, y + 30.0);
    glEnd();

    glColor3f(0.0, 1.0, 0.0);
    float liquidHeight = (liquidLevel / 50.0) * 50.0;
    glBegin(GL_QUADS);
    glVertex2f(x - 8.0, y - 25.0);
    glVertex2f(x + 8.0, y - 25.0);
    glVertex2f(x + 8.0, y - 25.0 + liquidHeight);
    glVertex2f(x - 8.0, y - 25.0 + liquidHeight);
    glEnd();
}

void drawPillContainer(float x, float y)
{
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_QUADS);
    glVertex2f(x - 20.0, y - 20.0);
    glVertex2f(x + 20.0, y - 20.0);
    glVertex2f(x + 20.0, y + 20.0);
    glVertex2f(x - 20.0, y + 20.0);
    glEnd();
}

void drawPill(float x, float y, int numPills)
{
    drawPillContainer(x, y);

    glColor3f(1.0, 1.0, 0.0);
    for (int i = 0; i < numPills; ++i)
    {
        float angle = i * (360.0 / numPills);
        float px = x + 15.0 * cos(angle * M_PI / 180.0);
        float py = y + 15.0 * sin(angle * M_PI / 180.0);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(px, py);
        for (int j = 0; j <= 360; j += 10)
        {
            glVertex2f(px + 3.0 * cos(j * M_PI / 180.0), py + 3.0 * sin(j * M_PI / 180.0));
        }
        glEnd();
    }
}

void drawTextLabel(const char *text, int x, int y)
{
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(x, y);
    while (*text != '\0')
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *text);
        ++text;
    }
}

void drawBox(float x, float y, int numMedicine, const char *label)
{
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2f(x, y - 30.0);
    glVertex2f(x + 200.0, y - 30.0); // Adjusted width
    glVertex2f(x + 200.0, y + 30.0); // Adjusted size
    glVertex2f(x, y + 30.0);
    glEnd();

    drawTextLabel(label, x + 10, y + 10); // Adjusted label position

    char numText[20];
    snprintf(numText, sizeof(numText), "%d", numMedicine);
    drawTextLabel(numText, x + 170, y + 10); // Adjusted number position

    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(3.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y - 30.0);
    glVertex2f(x + 200.0, y - 30.0);
    glVertex2f(x + 200.0, y + 30.0);
    glVertex2f(x, y + 30.0);
    glEnd();
}

void addNewMedicine(enum ProductType type, int line_num)
{
    // Add a new medicine to the specified production line
    /*if (liquidMedicinesCount[line_num - 1] < MEDICINES_PER_LINE)
    {
        if (type == BOTTLE)
        {
            liquid_lines.lines[i][medicineCount[i]].liquidLevel = (% 41) + 10;
            liquidMedicinesCount[line_num - 1]++;
        }
    }
*/
    // Update the count of failed or successful medicine
    numFailedBottleMedicine = 0;
    numSuccessfulBottleMedicine = 0;
    numFailedPillMedicine = 0;
    numSuccessfulPillMedicine = 0;
    /*for (int i = 0; i < LINES; ++i)
    {
        for (int j = 0; j < medicineCount[i]; ++j)
        {
            if (lines[i][j].failed)
            {
                if (lines[i][j].type == BOTTLE)
                {
                    numFailedBottleMedicine++;
                }
                else
                {
                    numFailedPillMedicine++;
                }
            }
            else
            {
                if (lines[i][j].type == BOTTLE)
                {
                    numSuccessfulBottleMedicine++;
                }
                else
                {
                    numSuccessfulPillMedicine++;
                }
            }
        }
    }*/

    glutPostRedisplay();
    // glutTimerFunc(TIMER_INTERVAL, addNewMedicine, 0);
}

void drawHuman(float x, float y)
{
    // Body
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x, y - 40);
    glEnd();

    // Head
    glColor3f(1.0, 1.0, 1.0);
    glPointSize(7.0);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();

    // Arms
    glBegin(GL_LINES);
    glVertex2f(x, y - 20);
    glVertex2f(x - 15, y - 30);

    glVertex2f(x, y - 20);
    glVertex2f(x + 15, y - 30);
    glEnd();

    // Legs
    glBegin(GL_LINES);
    glVertex2f(x, y - 40);
    glVertex2f(x - 10, y - 55);

    glVertex2f(x, y - 40);
    glVertex2f(x + 10, y - 55);
    glEnd();
}

void init()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
    // initializeLines();

    // Open a shared memories
    /*shmptr_liquid_production_lines = createSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, 4 * sizeof(struct Liquid_Production_Line), "GUI.c");
    liquid_production_lines = (struct Liquid_Production_Line *)shmptr_liquid_production_lines;

    shmptr_num_liquid_medicines_produced = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PRODUCED, sizeof(int), "liquid_production_line.c");
    shmptr_num_pill_medicines_failed = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_FAILED, sizeof(int), "liquid_production_line.c");

    // Open the semaphores
    sem_liquid_production_lines = createSemaphore(SEMKEY_LIQUID_PRODUCTION_LINES, 1, 1, "liquid_production_line.c");*/
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glLineWidth(2.0);

    float boxX = 20;
    float boxY = 50;
    drawBox(boxX, boxY, numFailedBottleMedicine, "Failed Bottle Medicine");
    boxX += BOX_SPACING;
    drawBox(boxX, boxY, numSuccessfulBottleMedicine, "Successful Bottle Medicine");
    boxX += BOX_SPACING;
    drawBox(boxX, boxY, numFailedPillMedicine, "Failed Pill Medicine");
    boxX += BOX_SPACING;
    drawBox(boxX, boxY, numSuccessfulPillMedicine, "Successful Pill Medicine");

    // check the shared memory for the liquid production lines,and if there is a new medicine, update the GUI

    for (int i = 0; i < LINES; ++i)
    {
        float x = 50 + i * LINE_SPACING;
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2f(x, 50.0);
        glVertex2f(x, 550.0);
        glEnd();

        // Draw human figure next to the production line
        drawHuman(x + 50, 300); // Adjusted position
        int k = 0;
        for (int j = 0; j < liquid_lines[i].num_medicines; j++)
        {
            // float productX = 50 + i * LINE_SPACING;
            // float productY = 500.0 - j * 80;

            if (liquid_lines[i].liquid_medicines[j].is_failed == 1)
            {
                continue;
            }

            float productX = x;
            float productY = 500.0 - k * 80;
            if (k > MEDICINES_PER_LINE - 1)
            {
                break;
            }
            k++;
            // printf("Liquid Medicine %d level %d in line %d ProductX: %f, ProductY: %f\n", j, liquid_lines[i].liquid_medicines[j].level, i, productX, productY);
            drawLiquidBottle(productX, productY, (liquid_lines[i].liquid_medicines[j].level % 41) + 10);
        }

        char label[20];
        if (i < 4)
        {
            snprintf(label, sizeof(label), "Bottle Line %d", i + 1);
        }
        else
        {
            snprintf(label, sizeof(label), "Pill Line %d", i - 3);
        }
        drawTextLabel(label, x - 40, 570);
    }
    glFlush();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(160, 120);
    glutCreateWindow("Medicine Factory with 8 Production Lines");

    // Open a shared memories
    shmptr_liquid_production_lines = createSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, 2 * sizeof(struct Liquid_Production_Line), "GUI.c");
    liquid_lines = (struct Liquid_Production_Line *)shmptr_liquid_production_lines;

    // shmptr_num_liquid_medicines_produced = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PRODUCED, sizeof(int), "liquid_production_line.c");
    // shmptr_num_pill_medicines_failed = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_FAILED, sizeof(int), "liquid_production_line.c");

    // Open the semaphores
    sem_liquid_production_lines = createSemaphore(SEMKEY_LIQUID_PRODUCTION_LINES, 1, 1, "liquid_production_line.c");

    init();
    glutDisplayFunc(display); // Register display callback function
    glutIdleFunc(display);
    // glutTimerFunc(TIMER_INTERVAL, addNewMedicine, 0);
    glutMainLoop();
    return 0;
}
