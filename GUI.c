#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define LINES 8
#define LINE_SPACING 120 // Adjust the spacing between lines
#define WINDOW_WIDTH 1000 // Adjusted window width

// Product types
enum ProductType { BOTTLE, PILL };

// Production line structure
typedef struct {
    enum ProductType type;
    int isChecked;
    float liquidLevel; // Liquid level for bottles
    int numberOfPills;
} ProductionLine;

ProductionLine lines[LINES];

// Randomly assign products to lines and check status
void initializeLines() {
    srand(time(NULL));
    for (int i = 0; i < LINES; ++i) {
        lines[i].type = (i < 4) ? BOTTLE : PILL;
        lines[i].isChecked = rand() % 2; // Randomly set check status
        if (lines[i].type == BOTTLE) {
            lines[i].liquidLevel = (rand() % 60) + 20; // Random liquid level between 20 and 80
        } else {
            // Randomly assign number of pills between 1 and 5
            lines[i].numberOfPills = (rand() % 5) + 1;
        }
    }
}

void drawLiquidBottle(float x, float y, float liquidLevel) {
    // Draw the bottle
    glColor3f(0.0, 0.0, 1.0); // Blue color for the bottle
    glBegin(GL_QUADS);
        glVertex2f(x - 20.0, y - 50.0); // Bottom left corner
        glVertex2f(x + 20.0, y - 50.0); // Bottom right corner
        glVertex2f(x + 20.0, y + 50.0); // Top right corner
        glVertex2f(x - 20.0, y + 50.0); // Top left corner
    glEnd();

    // Draw the bottle's cap
    glColor3f(0.7, 0.7, 0.7); // Grey color for the cap
    glBegin(GL_QUADS);
        glVertex2f(x - 20.0, y + 50.0); // Bottom left corner of cap
        glVertex2f(x + 20.0, y + 50.0); // Bottom right corner of cap
        glVertex2f(x + 20.0, y + 60.0); // Top right corner of cap
        glVertex2f(x - 20.0, y + 60.0); // Top left corner of cap
    glEnd();

    // Draw the liquid inside the bottle
    glColor3f(0.0, 1.0, 0.0); // Green color for the liquid
    glBegin(GL_QUADS);
        glVertex2f(x - 18.0, y - 48.0); // Bottom left corner of liquid
        glVertex2f(x + 18.0, y - 48.0); // Bottom right corner of liquid
        glVertex2f(x + 18.0, y - 48.0 + liquidLevel); // Top right corner of liquid
        glVertex2f(x - 18.0, y - 48.0 + liquidLevel); // Top left corner of liquid
    glEnd();
}

void drawPillContainer(float x, float y) {
    glColor3f(0.5, 0.5, 0.5); // Gray color for the container
    glBegin(GL_QUADS);
        glVertex2f(x - 20.0, y - 25.0); // Bottom left corner
        glVertex2f(x + 20.0, y - 25.0); // Bottom right corner
        glVertex2f(x + 20.0, y + 25.0); // Top right corner
        glVertex2f(x - 20.0, y + 25.0); // Top left corner
    glEnd();
}

void drawPill(float x, float y, int numPills) {
    drawPillContainer(x, y); // Draw the pill container first

    glColor3f(1.0, 1.0, 0.0); // Yellow color for pills
    for (int i = 0; i < numPills; ++i) {
        float angle = i * (360.0 / numPills);
        float px = x + 15.0 * cos(angle * M_PI / 180.0);
        float py = y + 15.0 * sin(angle * M_PI / 180.0);
        glBegin(GL_TRIANGLE_FAN);
            glVertex2f(px, py);
            for (int j = 0; j <= 360; j += 10) {
                glVertex2f(px + 3.0 * cos(j * M_PI / 180.0), py + 3.0 * sin(j * M_PI / 180.0));
            }
        glEnd();
    }
}

void drawCheckMark(float x, float y) {
    glColor3f(0.0, 1.0, 0.0); // Green color for check mark
    glLineWidth(5.0); // Adjust line width

    glBegin(GL_LINES);
        glVertex2f(x - 20.0, y - 20.0);
        glVertex2f(x, y - 40.0);
        glVertex2f(x, y - 40.0);
        glVertex2f(x + 40.0, y + 20.0);
    glEnd();
}


void drawTextLabel(const char *text, int x, int y) {
    glColor3f(1.0, 1.0, 1.0); // White color for text
    glRasterPos2i(x, y); // Set the raster position
    while (*text != '\0') {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *text); // Render each character
        ++text;
    }
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0); // Set background color to black
    glMatrixMode(GL_PROJECTION); // Set the matrix mode to projection
    glLoadIdentity(); // Load the identity matrix
    gluOrtho2D(0.0, WINDOW_WIDTH, 0.0, 600.0); // Set up an orthographic projection with wider window
    initializeLines(); // Initialize production lines
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen

    glLineWidth(10.0); // Increase line width

    for (int i = 0; i < LINES; ++i) {
        float x = 100 + i * LINE_SPACING; // Calculate the x position for each line with increased space
        glColor3f(1.0, 1.0, 1.0); // Set the line color to white
        glBegin(GL_LINES);
            glVertex2f(x, 100.0);
            glVertex2f(x, 500.0);
        glEnd();

        float productX = x;
        float productY = 300.0;

        if (lines[i].type == BOTTLE) {
            drawLiquidBottle(productX, productY, lines[i].liquidLevel);
        } else {
            drawPill(productX, productY, lines[i].numberOfPills); // Pass number of pills
        }

        // Draw the label for the production line
        char label[20];
        if (lines[i].type == BOTTLE) {
            snprintf(label, sizeof(label), "Bottle Line %d", i + 1);
        } else {
            snprintf(label, sizeof(label), "Pill Line %d", i + 1);
        }
        drawTextLabel(label, x - 50, 520); // Draw label for the production line

        if (lines[i].isChecked) {
            drawCheckMark(productX, productY - 90.0); // Draw check mark below the drawings
        }else {
            // Draw red X
            glColor3f(1.0, 0.0, 0.0); // Red color for X
            glBegin(GL_LINES);
                glVertex2f(productX - 20.0, productY - 110.0 - 20.0);
                glVertex2f(productX + 20.0, productY - 110.0 + 20.0);
                glVertex2f(productX - 20.0, productY - 110.0 + 20.0);
                glVertex2f(productX + 20.0, productY - 110.0 - 20.0);
            glEnd();
        }
    }

    glFlush(); // Flush the OpenGL buffers to the window
}


int main(int argc, char** argv) {
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // Set display mode
    glutInitWindowSize(WINDOW_WIDTH, 600); // Set window size
    glutInitWindowPosition(100, 100); // Set window position
    glutCreateWindow("Medicine Factory with 8 Production Lines"); // Create window with title

    init(); // Call the initialization function
    glutDisplayFunc(display); // Set the display callback function

    glutMainLoop(); // Enter the GLUT main loop

    return 0;
}

