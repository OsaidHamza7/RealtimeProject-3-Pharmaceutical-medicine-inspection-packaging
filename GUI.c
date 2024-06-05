#include "header.h"
#include "ipcs.h"

#define LINES 8
#define MEDICINES_PER_LINE 5
#define LINE_SPACING 120
#define BOX_SPACING 240
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define TIMER_INTERVAL 2000

enum ProductType
{
    BOTTLE,
    PILL
};

// ProductionLine lines[LINES][MEDICINES_PER_LINE];
int ProductID[8] = {0};

int liquidMedicinesCount[LINES] = {0}; // Keeps track of the number of medicines in each liquid line
int pillmedicinesCount[LINES] = {0};   // Keeps track of the number of medicines in each pill line
int numFailedBottleMedicine = 0;       // Number of failed bottle medicine
int numSuccessfulBottleMedicine = 0;   // Number of successful bottle medicine
int numFailedPillMedicine = 0;         // Number of failed pill medicine
int numSuccessfulPillMedicine = 0;     // Number of successful pill medicine

char *shmptr_liquid_production_lines;
char *shmptr_num_liquid_medicines_produced;
char *shmptr_num_liquid_medicines_failed;

char *shmptr_pill_production_lines;
char *shmptr_num_pill_medicines_produced;
char *shmptr_num_pill_medicines_failed;

int sem_liquid_production_lines;
int sem_num_liquid_medicines_produced;
int sem_num_pill_medicines_failed;

int sem_pill_production_lines;
int sem_num_pill_medicines_produced;
int sem_num_pill_medicines_failed;

Liquid_Production_Line *liquid_lines;
Pill_Production_Line *pill_lines;

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

void drawLiquidBottle(float x, float y, float liquidLevel, int numColor, int medicine_num)
{
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    glVertex2f(x - 15.0, y - 25.0);
    glVertex2f(x + 15.0, y - 25.0);
    glVertex2f(x + 15.0, y + 25.0);
    glVertex2f(x - 15.0, y + 25.0);
    glEnd();

    glColor3f(0.7, 0.7, 0.7);
    glBegin(GL_QUADS);
    glVertex2f(x - 15.0, y + 25.0);
    glVertex2f(x + 15.0, y + 25.0);
    glVertex2f(x + 15.0, y + 30.0);
    glVertex2f(x - 15.0, y + 30.0);
    glEnd();

    switch (numColor)
    {
    case 1:
        glColor3f(0.0f, 0.75f, 1.0f); // RGB: (0, 191, 255)
        break;
    case 2:
        glColor3f(0.86f, 0.08f, 0.24f); // RGB: (220, 20, 60)
        break;
    case 3:
        glColor3f(0.13f, 0.55f, 0.13f); // RGB: (34, 139, 34)
        break;
    case 4:
        glColor3f(0.85f, 0.65f, 0.13f); // RGB: (218, 165, 32)
        break;
    case 5:
        glColor3f(0.58f, 0.44f, 0.86f); // RGB: (148, 112, 219)
        break;
    case 6:
        glColor3f(1.0f, 0.39f, 0.28f); // RGB: (255, 99, 71)
        break;
    case 7:
        glColor3f(0.25f, 0.88f, 0.82f); // RGB: (64, 224, 208)
        break;
    default:
        glColor3f(0.44f, 0.5f, 0.56f); // RGB: (112, 128, 144)
    }
    // glColor3f(0.0, 1.0, 0.0);
    float liquidHeight = (liquidLevel / 50.0) * 50.0;
    glBegin(GL_QUADS);
    glVertex2f(x - 13.0, y - 25.0);
    glVertex2f(x + 13.0, y - 25.0);
    glVertex2f(x + 13.0, y - 25.0 + liquidHeight);
    glVertex2f(x - 13.0, y - 25.0 + liquidHeight);
    glEnd();

    // Draw the label in the middle of the bottle
    glColor3f(1.0, 1.0, 1.0);  // White color for the label
    glRasterPos2f(x - 5.0, y); // Set the raster position for the text
    char labelID[4];
    snprintf(labelID, sizeof(labelID), "%d", medicine_num);
    for (int i = 0; labelID[i] != '\0'; ++i)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, labelID[i]);
    }
}

void drawPillContainer(float x, float y, int medicine_num)
{
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(x - 23.0, y - 23.0);
    glVertex2f(x + 23.0, y - 23.0);
    glVertex2f(x + 23.0, y + 23.0);
    glVertex2f(x - 23.0, y + 23.0);
    glEnd();

    // Draw the label in the middle of the bottle
    glColor3f(0.0, 0.0, 0.0);      // White color for the label
    glRasterPos2f(x - 5.0, y - 7); // Set the raster position for the text
    char labelID[4];
    snprintf(labelID, sizeof(labelID), "%d", medicine_num);
    for (int i = 0; labelID[i] != '\0'; ++i)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, labelID[i]);
    }
}

void drawPill(float x, float y, int numPills, int numColor, int medicine_num)
{
    drawPillContainer(x, y, medicine_num);

    switch (numColor)
    {
    case 1:
        glColor3f(0.0f, 0.75f, 1.0f); // RGB: (0, 191, 255)
        break;
    case 2:
        glColor3f(0.86f, 0.08f, 0.24f); // RGB: (220, 20, 60)
        break;
    case 3:
        glColor3f(0.13f, 0.55f, 0.13f); // RGB: (34, 139, 34)
        break;
    case 4:
        glColor3f(0.85f, 0.65f, 0.13f); // RGB: (218, 165, 32)
        break;
    case 5:
        glColor3f(0.58f, 0.44f, 0.86f); // RGB: (148, 112, 219)
        break;
    case 6:
        glColor3f(1.0f, 0.39f, 0.28f); // RGB: (255, 99, 71)
        break;
    case 7:
        glColor3f(0.25f, 0.88f, 0.82f); // RGB: (64, 224, 208)
        break;
    default:
        glColor3f(0.44f, 0.5f, 0.56f); // RGB: (112, 128, 144)
    }

    // print pills
    for (int i = 0; i < numPills; ++i)
    {
        float angle = i * (360.0 / numPills);
        float px = x + 15.0 * cos(angle * M_PI / 180.0);
        float py = y + 15.0 * sin(angle * M_PI / 180.0);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(px, py);
        for (int j = 0; j <= 360; j += 10)
        {
            glVertex2f(px + 5.0 * cos(j * M_PI / 180.0), py + 5.0 * sin(j * M_PI / 180.0));
        }
        glEnd();
    }
}
void drawTextLabel(const char *text, float x, float y, void *font)
{   
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        ++text;
    }
}

void drawTextLabelWithColor(const char *text, int x, int y, int numColor)
{
    switch (numColor)
    {
    case 1:
        glColor3f(0.0f, 0.75f, 1.0f); // RGB: (0, 191, 255)
        break;
    case 2:
        glColor3f(0.86f, 0.08f, 0.24f); // RGB: (220, 20, 60)
        break;
    case 3:
        glColor3f(0.13f, 0.55f, 0.13f); // RGB: (34, 139, 34)
        break;
    case 4:
        glColor3f(0.85f, 0.65f, 0.13f); // RGB: (218, 165, 32)
        break;
    case 5:
        glColor3f(0.58f, 0.44f, 0.86f); // RGB: (148, 112, 219)
        break;
    case 6:
        glColor3f(1.0f, 0.39f, 0.28f); // RGB: (255, 99, 71)
        break;
    case 7:
        glColor3f(0.25f, 0.88f, 0.82f); // RGB: (64, 224, 208)
        break;
    default:
        glColor3f(0.44f, 0.5f, 0.56f); // RGB: (112, 128, 144)
    }
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

    drawTextLabel(label, x + 10, y -3, GLUT_BITMAP_HELVETICA_12); // Adjusted label position

    char numText[20];
    snprintf(numText, sizeof(numText), "%d", numMedicine);
    drawTextLabel(numText, x + 170, y - 8, GLUT_BITMAP_TIMES_ROMAN_24); // Adjusted number position

    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(3.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y - 30.0);
    glVertex2f(x + 200.0, y - 30.0);
    glVertex2f(x + 200.0, y + 30.0);
    glVertex2f(x, y + 30.0);
    glEnd();
}

void drawBoxProduced(float x, float y, int numMedicine, const char *label)
{
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2f(x, y - 30.0);
    glVertex2f(x + 440.0, y - 30.0); // Adjusted width
    glVertex2f(x + 440.0, y + 30.0); // Adjusted size
    glVertex2f(x, y + 30.0);
    glEnd();

    drawTextLabel(label, x + 10, y - 8, GLUT_BITMAP_TIMES_ROMAN_24); // Adjusted label position

    char numText[20];
    snprintf(numText, sizeof(numText), "%d", numMedicine);
    drawTextLabel(numText, x + 410, y - 8, GLUT_BITMAP_TIMES_ROMAN_24); // Adjusted number position

    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(3.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y - 30.0);
    glVertex2f(x + 440.0, y - 30.0);
    glVertex2f(x + 440.0, y + 30.0);
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

void drawHuman(float x, float y, int numColor)
{
    switch (numColor)
    {
    case 1:
        glColor3f(0.0f, 0.75f, 1.0f); // RGB: (0, 191, 255)
        break;
    case 2:
        glColor3f(0.86f, 0.08f, 0.24f); // RGB: (220, 20, 60)
        break;
    case 3:
        glColor3f(0.13f, 0.55f, 0.13f); // RGB: (34, 139, 34)
        break;
    case 4:
        glColor3f(0.85f, 0.65f, 0.13f); // RGB: (218, 165, 32)
        break;
    case 5:
        glColor3f(0.58f, 0.44f, 0.86f); // RGB: (148, 112, 219)
        break;
    case 6:
        glColor3f(1.0f, 0.39f, 0.28f); // RGB: (255, 99, 71)
        break;
    case 7:
        glColor3f(0.25f, 0.88f, 0.82f); // RGB: (64, 224, 208)
        break;
    default:
        glColor3f(0.44f, 0.5f, 0.56f); // RGB: (112, 128, 144)
    }
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x, y - 40);
    glEnd();

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

/*void drawOnlyOneHuman(float x, float j, int numColor)
{
    switch (numColor)
    {
    case 1:
        glColor3f(0.0f, 0.75f, 1.0f); // RGB: (0, 191, 255)
        break;
    case 2:
        glColor3f(0.86f, 0.08f, 0.24f); // RGB: (220, 20, 60)
        break;
    case 3:
        glColor3f(0.13f, 0.55f, 0.13f); // RGB: (34, 139, 34)
        break;
    case 4:
        glColor3f(0.85f, 0.65f, 0.13f); // RGB: (218, 165, 32)
        break;
    case 5:
        glColor3f(0.58f, 0.44f, 0.86f); // RGB: (148, 112, 219)
        break;
    case 6:
        glColor3f(1.0f, 0.39f, 0.28f); // RGB: (255, 99, 71)
        break;
    case 7:
        glColor3f(0.25f, 0.88f, 0.82f); // RGB: (64, 224, 208)
        break;
    default:
        glColor3f(0.44f, 0.5f, 0.56f); // RGB: (112, 128, 144)
    }
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x, y - 40);
    glEnd();

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
}*/

void init()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT);
    // initializeLines();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glLineWidth(2.0);

    float boxX = 20;
    float boxY = 140;
    drawBox(boxX, boxY, (int)(*shmptr_num_liquid_medicines_failed), "Failed Bottle Medicine");
    boxX += BOX_SPACING;
    drawBox(boxX, boxY, (int)(*shmptr_num_liquid_medicines_produced), "Packaged Bottle Medicine");
    boxX += BOX_SPACING;
    drawBox(boxX, boxY, numFailedPillMedicine, "Failed Pill Medicine");
    boxX += BOX_SPACING;
    drawBox(boxX, boxY, numSuccessfulPillMedicine, "Packaged Pill Medicine");

    boxX = 20;
    boxY = 60;
    drawBoxProduced(boxX, boxY, numSuccessfulPillMedicine, "Produced Bottle Medicine");
    boxX += 480;
    drawBoxProduced(boxX, boxY, numSuccessfulPillMedicine, "Produced Pill Medicine");

    // check the shared memory for the liquid production lines,and if there is a new medicine, update the GUI

    for (int i = 0; i < LINES; ++i)
    {
        float x = 50 + i * LINE_SPACING;
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2f(x, 180.0);
        glVertex2f(x, 650.0);
        glEnd();

        int numColor = i +1;
        // Draw human figure next to the production line
        
        int numPeople = 5; // Number of people, you can modify this based on your logic
        for (int j = 0; j < numPeople; ++j)
        {
            float personX = 50 + i * LINE_SPACING;
            float personY = 640.0 - j * 88; // Adjusted spacing between people
            drawHuman(personX + 50, personY, i + 1);
        }

        int k = 0;

        char label[20];
        if (i < 4)
        {
            snprintf(label, sizeof(label), "Bottle Line %d", i + 1);
            k = 0;
            for (int j = 0; j < liquid_lines[i].production_line.num_produced_medicines; j++)
            {
                if (liquid_lines[i].bottles[j].is_failed == 1)
                {
                    continue;
                }

                float productX = x;
                float productY = 600.0 - k * 80;
                if (k > MEDICINES_PER_LINE - 1)
                {
                    break;
                }
                k++;
                drawLiquidBottle(productX, productY, (liquid_lines[i].bottles[j].liquid_medicine.level % 41) + 10, liquid_lines[i].bottles[j].liquid_medicine.color, j + 1);
            }
            drawTextLabelWithColor(label, x - 27, 670, i+1);
        }

        else
        {
            snprintf(label, sizeof(label), "Pill Line %d", i - 3);
            k = 0;
            for (int j = 0; j < pill_lines[i - 4].production_line.num_produced_medicines; j++)
            {
                if (pill_lines[i - 4].pill_medicines[j].is_failed == 1)
                {
                    continue;
                }

                float productX = x;
                float productY = 600.0 - k * 80;
                if (k > MEDICINES_PER_LINE - 1)
                {
                    break;
                }
                k++;
                drawPill(productX, productY, pill_lines[i - 4].pill_medicines[j].plastic_containers[0].num_pills, pill_lines[i - 4].pill_medicines[j].plastic_containers[0].pills[0].color, j + 1);
            }
            drawTextLabelWithColor(label, x - 27, 670, i+1);
        }
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

    int num_liquid_production_lines = atoi(argv[1]);
    int num_pill_production_lines = atoi(argv[2]);

    // Open a shared memories
    shmptr_liquid_production_lines = createSharedMemory(SHKEY_LIQUID_PRODUCTION_LINES, num_liquid_production_lines * sizeof(struct Liquid_Production_Line), "GUI.c");
    liquid_lines = (struct Liquid_Production_Line *)shmptr_liquid_production_lines;

    shmptr_pill_production_lines = createSharedMemory(SHKEY_PILL_PRODUCTION_LINES, num_pill_production_lines * sizeof(struct Pill_Production_Line), "GUI.c");
    pill_lines = (struct Pill_Production_Line *)shmptr_pill_production_lines;

    shmptr_num_liquid_medicines_produced = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PRODUCED, sizeof(int), "GUI.c");
    shmptr_num_liquid_medicines_failed = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_FAILED, sizeof(int), "GUI.c");

    shmptr_num_pill_medicines_produced = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_PRODUCED, sizeof(int), "GUI.c");
    shmptr_num_pill_medicines_failed = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_FAILED, sizeof(int), "GUI.c");

    // shmptr_num_liquid_medicines_produced = createSharedMemory(SHKEY_NUM_LIQUID_MEDICINES_PRODUCED, sizeof(int), "liquid_production_line.c");
    // shmptr_num_pill_medicines_failed = createSharedMemory(SHKEY_NUM_PILL_MEDICINES_FAILED, sizeof(int), "liquid_production_line.c");

    // Open the semaphores
    sem_liquid_production_lines = createSemaphore(SEMKEY_LIQUID_PRODUCTION_LINES, 1, 1, "GUI.c");
    sem_pill_production_lines = createSemaphore(SEMKEY_PILL_PRODUCTION_LINES, 1, 1, "GUI.c");

    init();
    glutDisplayFunc(display); // Register display callback function
    glutIdleFunc(display);
    // glutTimerFunc(TIMER_INTERVAL, addNewMedicine, 0);
    glutMainLoop();
    return 0;
}
