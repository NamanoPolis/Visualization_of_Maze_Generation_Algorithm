//
//Defining Libraries
#include <GL/glut.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <freeglut.h>

//Dimensions of display window
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 580
#define ENEMY_DELTA_TIME 120
#define ENEMY_ATTRACT_RADIUS 50

//Speed of Maze Generation
int DELTA_TIME = 0;

#define MAZE_POS_X 350
#define MAZE_POS_Y 70

//Details of each room
//#define MAZESIZE 20
//#define ROOMSIZE 20

#define TEXT_POSITION_X 100
#define TEXT_POSITION_Y 80

bool isStartScreenOver = false;

float currentTime, startTime;

char timeString[100];

int MAZESIZE = 25;
int ROOMSIZE = 20;

int currentRoomX, currentRoomY, currentDirection;
int currentColumn = 0;

int playerRoomX, playerRoomY;
int lastPlayerRoomX, lastPlayerRoomY;
float playerMouthPosition = 0;
bool playerMode = false;

int destRoomX, destRoomY;

//Initially maze in incomplete
bool mazeComplete = false;
int matchState = 0;
bool randomWallsBroken = false;
bool walking = false;
bool steppingMode = true;
bool doStep = false;

enum {
    LEFT, RIGHT, TOP, BOTTOM, CLIFF
};

//Directions of movement
int oppositeDirection[] = {RIGHT, LEFT, BOTTOM, TOP};

//Sign change when moves
int dx[] = {-1, 1, 0, 0};
int dy[] = {0, 0, -1, 1};

typedef struct COLOR {
    int r, g, b;
} Color;

void SetColor(int r, int g, int b) {
    glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);
}


//Components of a room
struct Room {
    bool wall[4];                               //For each wall
    bool visited;                               //If that room is visited of not
    bool scanning;                              //If that room is under scanning or not
    bool playerVisited;
};

//Components of a Maze
struct Maze {
    struct Room room[1000][1000];
} mazePrimary;

struct Enemy {
    int xRoom, yRoom;
    int randRadius[4];
    int randPosX[4];
    int randPosY[4];
    int lastDir;
    Color color[4];
} enemies[100];
int nEnemies = 0;
int enemyMoveCountdown = ENEMY_DELTA_TIME;


void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void RandomizeDirections(int *arr, int n) {          //Randomizing variables of the array
    int i, j;
    for (i = n - 1; i > 0; i--) {
        j = rand() % (i + 1);
        swap(&arr[i], &arr[j]);
    }
}

void DrawRoom(float x, float y, float size, int roomNoX, int roomNoY) {
    //Draw filled rectangle if not visited
    SetColor(255, 255, 255);
    if (mazePrimary.room[roomNoX][roomNoY].visited == false) {
        glBegin(GL_POLYGON);
        glVertex2f(x - size / 2, y - size / 2);
        glVertex2f(x + size / 2, y - size / 2);
        glVertex2f(x + size / 2, y + size / 2);
        glVertex2f(x - size / 2, y + size / 2);
        glEnd();
    }

    //Draw filled rectangle if visited
    SetColor(180, 210, 250);
    if (mazePrimary.room[roomNoX][roomNoY].visited) {
        glBegin(GL_POLYGON);
        glVertex2f(x - size / 2, y - size / 2);
        glVertex2f(x + size / 2, y - size / 2);
        glVertex2f(x + size / 2, y + size / 2);
        glVertex2f(x - size / 2, y + size / 2);
        glEnd();
    }

    //Draw filled rectangle if current room
    SetColor(90, 200, 120);
    if (roomNoX == currentRoomX && roomNoY == currentRoomY) {
        glBegin(GL_POLYGON);
        glVertex2f(x - size / 2, y - size / 2);
        glVertex2f(x + size / 2, y - size / 2);
        glVertex2f(x + size / 2, y + size / 2);
        glVertex2f(x - size / 2, y + size / 2);
        glEnd();
    }

    //Draw filled rectangle if under scanning
    SetColor(190, 100, 120);
    if (mazePrimary.room[roomNoX][roomNoY].scanning) {
        glBegin(GL_POLYGON);
        glVertex2f(x - size / 2, y - size / 2);
        glVertex2f(x + size / 2, y - size / 2);
        glVertex2f(x + size / 2, y + size / 2);
        glVertex2f(x - size / 2, y + size / 2);
        glEnd();
    }

    //Draw filled rectangle if player visited
    SetColor(140, 180, 220);
    if (mazePrimary.room[roomNoX][roomNoY].playerVisited) {
        glBegin(GL_POLYGON);
        glVertex2f(x - size / 2, y - size / 2);
        glVertex2f(x + size / 2, y - size / 2);
        glVertex2f(x + size / 2, y + size / 2);
        glVertex2f(x - size / 2, y + size / 2);
        glEnd();
    }

    if (mazePrimary.room[roomNoX][roomNoY].visited) {
        SetColor(10, 10, 10);
    } else {
        SetColor(150, 150, 150);
    }
    //Draw Left Wall
    if (mazePrimary.room[roomNoX][roomNoY].wall[LEFT]) {
        glBegin(GL_LINES);
        glVertex2f(x - size / 2, y - size / 2);
        glVertex2f(x - size / 2, y + size / 2);
        glEnd();
    }

    //Draw Right Wall
    if (mazePrimary.room[roomNoX][roomNoY].wall[RIGHT]) {
        glBegin(GL_LINES);
        glVertex2f(x + size / 2, y - size / 2);
        glVertex2f(x + size / 2, y + size / 2);
        glEnd();
    }

    //Draw Top Wall
    if (mazePrimary.room[roomNoX][roomNoY].wall[TOP]) {
        glBegin(GL_LINES);
        glVertex2f(x - size / 2, y - size / 2);
        glVertex2f(x + size / 2, y - size / 2);
        glEnd();
    }
    //Draw Bottom Wall
    if (mazePrimary.room[roomNoX][roomNoY].wall[BOTTOM]) {
        glBegin(GL_LINES);
        glVertex2f(x - size / 2, y + size / 2);
        glVertex2f(x + size / 2, y + size / 2);
        glEnd();
    }
}

void DrawPlayer(GLfloat x, GLfloat y, GLfloat radius) {
    int i;
    int triangleAmount = 60;

    GLfloat twicePi = (2 - (int) playerMouthPosition * (0.125 / 5)) * 3.141;

//    SetColor(255,255,255);
    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(x, y);                               // Center of Circle

    for (i = (int) playerMouthPosition; i <= triangleAmount; i++) {
        glVertex2f(
                x + (radius * cos(i * twicePi / triangleAmount)),
                y + (radius * sin(i * twicePi / triangleAmount))
        );
    }
    glEnd();
}

void DrawEnemy(int enemyNo) {
    int i, j;
    int triangleAmount = 30;

    GLfloat twicePi = 2 * 3.141;

//    SetColor(255,255,255);
    glBegin(GL_TRIANGLE_FAN);

    for (j = 0; j < 4; j++) {
        SetColor(enemies[enemyNo].color[j].r, enemies[enemyNo].color[j].g, enemies[enemyNo].color[j].b);
        for (i = 0; i <= triangleAmount; i++) {
            glVertex2f(
                    MAZE_POS_X + enemies[enemyNo].xRoom * ROOMSIZE + enemies[enemyNo].randPosX[j] +
                    (enemies[enemyNo].randRadius[j] * cos(i * twicePi / triangleAmount)),
                    MAZE_POS_Y + enemies[enemyNo].yRoom * ROOMSIZE + enemies[enemyNo].randPosY[j] +
                    (enemies[enemyNo].randRadius[j] * sin(i * twicePi / triangleAmount))
            );
        }
    }
    glEnd();
}

void DrawFilledCircle(GLfloat x, GLfloat y, GLfloat radius) {
    int i;
    int triangleAmount = 60;

    GLfloat twicePi = 2 * 3.141;

//    SetColor(255,255,255);
    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(x, y);                               // Center of Circle

    for (i = 0; i <= triangleAmount; i++) {
        glVertex2f(
                x + (radius * cos(i * twicePi / triangleAmount)),
                y + (radius * sin(i * twicePi / triangleAmount))
        );
    }
    glEnd();
}

void DrawPlayerInRoom(int roomX, int roomY) {
    SetColor(250, 250, 0);
    DrawPlayer(MAZE_POS_X + roomX * ROOMSIZE, MAZE_POS_Y + roomY * ROOMSIZE, (ROOMSIZE - ROOMSIZE / 5) / 2);
    if (playerMode == false) {
        playerMouthPosition += 0.02 * (DELTA_TIME + 1);
    } else {
        playerMouthPosition -= 0.02 * (DELTA_TIME + 1);
    }
    if (playerMouthPosition > 7 || playerMouthPosition < 0) {
        playerMode = !playerMode;
        if (playerMouthPosition > 7)
            playerMouthPosition = 7;
        if (playerMouthPosition < 0)
            playerMouthPosition = 0;
    }
//    printf("%f + %d\n", playerMouthPosition, playerMode);
}


//void SortPlayerEnemyDistances(int *distanceArr, int *directionArr, int n) {
//    int i, key, j;
//    for (i = 1; i < n; i++) {
//        key = i;
//        j = i - 1;
//
//        while (j >= 0 && distanceArr[j] > distanceArr[key]) {
//            distanceArr[j + 1] = distanceArr[j];
//            directionArr[j + 1] = directionArr[j];
//            j = j - 1;
//        }
//        distanceArr[j + 1] = distanceArr[key];
//        directionArr[j + 1] = directionArr[key];
//    }
//}

void SortPlayerEnemyDistances(int *distanceArr, int *directionArr, int n) {
    int i, j;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (distanceArr[j] > distanceArr[j + 1]) {
                swap(&distanceArr[j], &distanceArr[j + 1]);
                swap(&directionArr[j], &directionArr[j + 1]);
            }
        }
    }
}

void MoveEnemyTowardPlayer(int enemyNo) {
    int directionArray[] = {LEFT, RIGHT, TOP, BOTTOM};
    RandomizeDirections(directionArray, 4);

    int i, tempRoomX, tempRoomY;

    int localDistanceArr[4];
    int localDirectionArr[4];
    printf("MOVING TOWARDS\n");
    //Picking up any random Direction
    for (i = 0; i < 4; i++) {
        currentDirection = directionArray[i];
        //Moving in that direction
        tempRoomX = enemies[enemyNo].xRoom + dx[currentDirection];
        tempRoomY = enemies[enemyNo].yRoom + dy[currentDirection];

        //If that room is not visited
        if (tempRoomX >= 0 && tempRoomX < MAZESIZE && tempRoomY >= 0 && tempRoomY < MAZESIZE &&
            mazePrimary.room[enemies[enemyNo].xRoom][enemies[enemyNo].yRoom].wall[currentDirection] == false) {
            localDistanceArr[i] =
                    pow(playerRoomX - enemies[enemyNo].xRoom, 2) + pow(playerRoomY - enemies[enemyNo].yRoom, 2);
            localDirectionArr[i] = currentDirection;
//            printf("DIR : %d DIST : %d\n", currentDirection, localDistanceArr[i]);
        } else {
            localDistanceArr[i] = 30000;
            localDirectionArr[i] = -1;
        }
    }
//    printf("direction : %d distance : %d\n", localDirectionArr[0], localDistanceArr[0]);
//    printf("direction : %d distance : %d\n", localDirectionArr[1], localDistanceArr[1]);
//    printf("direction : %d distance : %d\n", localDirectionArr[2], localDistanceArr[2]);
//    printf("direction : %d distance : %d\n\n", localDirectionArr[3], localDistanceArr[3]);
    SortPlayerEnemyDistances(localDistanceArr, localDirectionArr, 4);
//    printf("direction : %d distance : %d\n", localDirectionArr[0], localDistanceArr[0]);
//    printf("direction : %d distance : %d\n", localDirectionArr[1], localDistanceArr[1]);
//    printf("direction : %d distance : %d\n", localDirectionArr[2], localDistanceArr[2]);
//    printf("direction : %d distance : %d\n\n", localDirectionArr[3], localDistanceArr[3]);
    enemies[enemyNo].xRoom = enemies[enemyNo].xRoom + dx[localDirectionArr[0]];
    enemies[enemyNo].yRoom = enemies[enemyNo].yRoom + dy[localDirectionArr[0]];
}

void MoveEnemyRandomly(int enemyNo) {
    int directionArray[] = {LEFT, RIGHT, TOP, BOTTOM};
    RandomizeDirections(directionArray, 4);

    int i, tempRoomX, tempRoomY;

    printf("MOVING RANDOMLY\n\n");
    //Picking up any random Direction
    for (i = 0; i < 4; i++) {
        currentDirection = directionArray[i];
        //Moving in that direction
        tempRoomX = enemies[enemyNo].xRoom + dx[currentDirection];
        tempRoomY = enemies[enemyNo].yRoom + dy[currentDirection];
        printf("new Room : %d\n", tempRoomX);

        //If that room is not visited
        if (tempRoomX >= 0 && tempRoomX < MAZESIZE && tempRoomY >= 0 && tempRoomY < MAZESIZE &&
            //            currentDirection != enemies[enemyNo].lastDir &&
            mazePrimary.room[enemies[enemyNo].xRoom][enemies[enemyNo].yRoom].wall[currentDirection] == false) {
            enemies[enemyNo].xRoom = tempRoomX;
            enemies[enemyNo].yRoom = tempRoomY;
            enemies[enemyNo].lastDir = currentDirection;
            return;
        }
    }
}

void MoveEnemiesInMaze() {
    if (enemyMoveCountdown < 0) {
        enemyMoveCountdown = ENEMY_DELTA_TIME;

        int i;
        for (i = 0; i < nEnemies; i++) {
            if (pow(playerRoomX - enemies[i].xRoom, 2) + pow(playerRoomY - enemies[i].yRoom, 2) <
                ENEMY_ATTRACT_RADIUS * ENEMY_ATTRACT_RADIUS) {
                MoveEnemyTowardPlayer(i);
            } else {
                MoveEnemyRandomly(i);
            }
        }
    } else {
        enemyMoveCountdown--;
    }
}

void DrawEnemies() {
    int i;
    for (i = 0; i < nEnemies; i++) {
        DrawEnemy(i);
    }
}

bool PlayerCollidingWithEnemies() {
    int i;
    for (i = 0; i < nEnemies; i++) {
        if (playerRoomX == enemies[i].xRoom && playerRoomY == enemies[i].yRoom) {
            return true;
        }
    }
    return false;
}

void DrawDestination(int destX, int destY) {
    SetColor(0, 0, 128);
//    DrawFilledCircle(MAZE_POS_X + destX * ROOMSIZE, MAZE_POS_Y + destY * ROOMSIZE, ROOMSIZE);
//    printf("DRAWING DESTINATION\n");
    DrawFilledCircle(MAZE_POS_X + destX * ROOMSIZE, MAZE_POS_Y + destY * ROOMSIZE,
                     (ROOMSIZE - ROOMSIZE / 5) / 2 * (playerMouthPosition / (500 * 0.02 * (DELTA_TIME + 1))));
}

void printArray(int arr[], int n) {
    int i;
    for (i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

//When free to walk
void Walk() {
    walking = true;

    int currentHuntRow;
    for (currentHuntRow = 0; currentHuntRow < MAZESIZE; ++currentHuntRow) {
        mazePrimary.room[currentColumn][currentHuntRow].scanning = false;
    }

    int directionArray[] = {LEFT, RIGHT, TOP, BOTTOM};
    RandomizeDirections(directionArray, 4);

//    printArray(directionArray, 4);
    int i, tempRoomX, tempRoomY;

    //Picking up any random Direction
    for (i = 0; i < 4; i++) {
        currentDirection = directionArray[i];
        //Moving in that direction
        tempRoomX = currentRoomX + dx[currentDirection];
        tempRoomY = currentRoomY + dy[currentDirection];

        //If that room is not visited
        if (tempRoomX >= 0 && tempRoomX < MAZESIZE && tempRoomY >= 0 && tempRoomY < MAZESIZE &&
            mazePrimary.room[tempRoomX][tempRoomY].visited == false) {

            //Break wall of both rooms
            mazePrimary.room[currentRoomX][currentRoomY].wall[currentDirection] = false;
            mazePrimary.room[tempRoomX][tempRoomY].wall[oppositeDirection[currentDirection]] = false;

            //Make that room currentRoom
            currentRoomX = tempRoomX;
            currentRoomY = tempRoomY;
            mazePrimary.room[currentRoomX][currentRoomY].visited = true;

//            printf("%d\n", currentDirection);
            return;
        }
    }
    //When gets traped
    currentDirection = CLIFF;
    currentColumn = 0;

    SetColor(50, 50, 50);
    glBegin(GL_POLYGON);
//    glRecti(MAZE_POS_X + currentRoomX * ROOMSIZE, MAZE_POS_Y + currentRoomY * ROOMSIZE,
//            MAZE_POS_Y + currentRoomY * ROOMSIZE + 4 * ROOMSIZE, MAZE_POS_Y + currentRoomY * ROOMSIZE + ROOMSIZE);
    glVertex2i(MAZE_POS_X + currentRoomX * ROOMSIZE - 30, MAZE_POS_Y + currentRoomY * ROOMSIZE - 20);
    glVertex2i(MAZE_POS_X + currentRoomX * ROOMSIZE - 30, MAZE_POS_Y + currentRoomY * ROOMSIZE - ROOMSIZE - 30);
    glVertex2i(MAZE_POS_X + currentRoomX * ROOMSIZE + 4 * ROOMSIZE - 10,
               MAZE_POS_Y + currentRoomY * ROOMSIZE - ROOMSIZE - 30);
    glVertex2i(MAZE_POS_X + currentRoomX * ROOMSIZE + 4 * ROOMSIZE - 10,
               MAZE_POS_Y + currentRoomY * ROOMSIZE - 20);
    glEnd();

    SetColor(250, 250, 250);
    glRasterPos2f(MAZE_POS_X + currentRoomX * ROOMSIZE - 20, MAZE_POS_Y + currentRoomY * ROOMSIZE - 25);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, "STUCK");
    glFlush();

    Sleep(DELTA_TIME * 10);
}

//When going on a hunt
void Hunt() {
    walking = false;

    int currentHuntRow;
    int directionArray[] = {LEFT, RIGHT, TOP, BOTTOM};
    RandomizeDirections(directionArray, 4);

    if (currentColumn > 0) {
        for (currentHuntRow = 0; currentHuntRow < MAZESIZE; ++currentHuntRow) {
            mazePrimary.room[currentColumn - 1][currentHuntRow].scanning = false;                   //
        }
    }

    for (currentHuntRow = 0; currentHuntRow < MAZESIZE; ++currentHuntRow) {
        mazePrimary.room[currentColumn][currentHuntRow].scanning = true;

        //When scanning room is un-visited
        if (mazePrimary.room[currentColumn][currentHuntRow].visited == false) {
            int k, tempHuntRow, tempHuntColumn, tempDirection;
            for (k = 0; k < 4; k++) {
                tempDirection = directionArray[k];
                //Checking in all directions of that unvisited room
                tempHuntRow = currentColumn + dx[tempDirection];
                tempHuntColumn = currentHuntRow + dy[tempDirection];

                //If  any neighbor room is visited
                if (tempHuntRow >= 0 && tempHuntRow < MAZESIZE && tempHuntColumn >= 0 &&
                    tempHuntColumn < MAZESIZE &&
                    mazePrimary.room[tempHuntRow][tempHuntColumn].visited == true) {

                    //Breaking wall of both rooms
                    mazePrimary.room[currentColumn][currentHuntRow].wall[tempDirection] = false;
                    mazePrimary.room[tempHuntRow][tempHuntColumn].wall[oppositeDirection[tempDirection]] = false;

                    //Moving to that room
                    currentRoomX = currentColumn;
                    currentRoomY = currentHuntRow;
                    currentDirection = tempDirection;

                    //Marking that room as visited
                    mazePrimary.room[currentRoomX][currentRoomY].visited = true;

//                    printf("RETURNING FROM HUNT\n");
                    return;
                }
            }
        }
    }

//    printf("INCREMENTING HUNT ROW : %d\n", currentColumn + 1);

    currentColumn++;
    //When maze is generated
    if (currentColumn == MAZESIZE) {
        for (currentHuntRow = 0; currentHuntRow < MAZESIZE; ++currentHuntRow) {
            mazePrimary.room[currentColumn - 1][currentHuntRow].scanning = false;
        }
        currentRoomX = -1;
        currentRoomY = -1;
        printf("\nMAZE GENERATED");
        mazeComplete = true;
        startTime = ((float) clock()) / 1000;
        mazePrimary.room[playerRoomX][playerRoomY].playerVisited = true;
    }
}

void BreakRandomWalls() {
    int k, randomRoomX, randomRoomY, tempRoomX, tempRoomY, direction;
    for (k = 0; k < MAZESIZE; ++k) {
        direction = rand() % 4;
        randomRoomX = rand() % MAZESIZE;
        randomRoomY = rand() % MAZESIZE;

        tempRoomX = randomRoomX + dx[direction];
        tempRoomY = randomRoomY + dy[direction];

        if (tempRoomX >= 0 && tempRoomX < MAZESIZE && tempRoomY >= 0 && tempRoomY < MAZESIZE) {
            mazePrimary.room[randomRoomX][randomRoomY].wall[direction] = false;
            mazePrimary.room[tempRoomX][tempRoomY].wall[oppositeDirection[direction]] = false;
        }
    }
}

void DisplayFrontPage() {
    int x=20,y=20;
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    SetColor(200, 60, 20);
    glRasterPos2f(SCREEN_WIDTH / 4 - 20, SCREEN_HEIGHT / 2 - 170);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "VISUAL REPRESENTATION OF MAZE GENERATION ALGORITHM");

    SetColor(0,0,200);
    glRasterPos2f(SCREEN_WIDTH -150+x, SCREEN_HEIGHT / 2 - 250+y);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24,"UPES");

    SetColor(20, 20, 20);
    glRasterPos2f(SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 120);
    glutBitmapString(GLUT_BITMAP_9_BY_15,
                     "Minor 1 Project\nFor the degree of\n");
    glRasterPos2f(SCREEN_WIDTH / 4 + 30, SCREEN_HEIGHT / 2 - 70);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18,
                     "BACHELOR OF TECHNOLOGY in COMPUTER SCIENCE & ENGINEERING");
    glRasterPos2f(SCREEN_WIDTH / 4 + 150, SCREEN_HEIGHT / 2 - 40);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18,
                     "With specialization in Graphics and Gaming");
    glRasterPos2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20);
    glutBitmapString(GLUT_BITMAP_9_BY_15,
                     "by");
    glRasterPos2f(SCREEN_WIDTH / 4 + 80, SCREEN_HEIGHT / 2 + 20);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18,
                     "       Name                           Roll No.                          SAP ID");
    glRasterPos2f(SCREEN_WIDTH / 4 + 80, SCREEN_HEIGHT / 2 + 40);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18,
                     "  Lucky Bansal               R110215072                     500045248");
    glRasterPos2f(SCREEN_WIDTH / 4 + 80, SCREEN_HEIGHT / 2 + 60);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18,
                     "   Naman Jain                 R142215024                     500046336");
    glRasterPos2f(SCREEN_WIDTH / 4 + 80, SCREEN_HEIGHT / 2 + 80);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18,
                     "Vaibhav Raj Singh         R142215050                     500045332");
    glRasterPos2f(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 + 120);
    glutBitmapString(GLUT_BITMAP_9_BY_15,
                     "Under the Guidance of");
    glRasterPos2f(SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 2 + 145);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18,
                     "Mr. Pankaj Badoni");
    glRasterPos2f(SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 2 + 160);
    glutBitmapString(GLUT_BITMAP_HELVETICA_12,
                     "Assistant Professor (SS)");
    glRasterPos2f(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 180);
    glutBitmapString(GLUT_BITMAP_HELVETICA_12,
                     "School of Computer Science & Engineering");

    SetColor(20, 20, 220);
    glRasterPos2f(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 + 210);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18,
                     "University of Petroleum and Energy Studies");


    SetColor(200,0,0);
    glBegin(GL_POLYGON);
    glVertex2f(SCREEN_WIDTH-200+x,SCREEN_HEIGHT/2-290+y);
    glVertex2f(SCREEN_WIDTH-200+x,SCREEN_HEIGHT/2-250+y);
    glVertex2f(SCREEN_WIDTH-180+x,SCREEN_HEIGHT/2-230+y);
    glVertex2f(SCREEN_WIDTH-160+x,SCREEN_HEIGHT/2-250+y);
    glVertex2f(SCREEN_WIDTH-160+x,SCREEN_HEIGHT/2-265+y);
    glEnd();


    SetColor(255,255,255);
    glBegin(GL_POLYGON);
    glVertex2f(SCREEN_WIDTH-190+x,SCREEN_HEIGHT/2-290+y);
    glVertex2f(SCREEN_WIDTH-190+x,SCREEN_HEIGHT/2-255+y);
    glVertex2f(SCREEN_WIDTH-180+x,SCREEN_HEIGHT/2-245+y);
    glVertex2f(SCREEN_WIDTH-170+x,SCREEN_HEIGHT/2-255+y);
    glVertex2f(SCREEN_WIDTH-170+x,SCREEN_HEIGHT/2-275+y);
    glEnd();


    SetColor(255,255,0);
    glBegin(GL_POLYGON);
    glVertex2f(SCREEN_WIDTH-165+x,SCREEN_HEIGHT/2-270+y);
    glVertex2f(SCREEN_WIDTH-160+x,SCREEN_HEIGHT/2-275+y);
    glVertex2f(SCREEN_WIDTH-160+x,SCREEN_HEIGHT/2-290+y);
    glVertex2f(SCREEN_WIDTH-180+x,SCREEN_HEIGHT/2-275+y);
    glEnd();

}

void display() {
    int i, j;

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    //Draw Rooms at (300,50) pixel of the display window
    for (i = 0; i < MAZESIZE; ++i) {
        for (j = 0; j < MAZESIZE; ++j) {
            DrawRoom(MAZE_POS_X + i * ROOMSIZE, MAZE_POS_Y + j * ROOMSIZE, ROOMSIZE, i, j);
//            printf("%d \n", i * ROOMSIZE);
        }
    }

    if (matchState == 0) {
        if (mazeComplete) {
            currentTime = ((float) clock()) / 1000 - startTime;
            SetColor(90, 140, 140);

            char tStr[100];
            glRasterPos2f(SCREEN_WIDTH / 3 + 140, MAZE_POS_Y - 50);
            strcpy(timeString, "TIME : ");
            itoa((int) currentTime / 60, tStr, 10);
            strcat(timeString, tStr);
            strcat(timeString, "m ");
            itoa((int) currentTime % 60, tStr, 10);
            strcat(timeString, tStr);
            strcat(timeString, "s");
            glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char *) timeString);
            printf("Time : %dm %ds\n", ((int) currentTime) / 60, ((int) currentTime) % 60);
        } else {
            SetColor(190, 140, 190);
            glRasterPos2f(SCREEN_WIDTH / 3 + 90, MAZE_POS_Y - 50);
            glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "WALK 'N HUNT");
            SetColor(100, 100, 100);
            glRasterPos2f(SCREEN_WIDTH / 3 + 120, MAZE_POS_Y - 35);
            glutBitmapString(GLUT_BITMAP_HELVETICA_10, "Maze Generation Algorithm");
        }
    } else if (matchState == 1) {
        SetColor(220, 60, 60);
        glRasterPos2f(SCREEN_WIDTH / 3 + 20, MAZE_POS_Y - 50);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, "WELL DONE!!! YOU COMPLETED THE MAZE");
        glRasterPos2f(SCREEN_WIDTH / 3 + 40, MAZE_POS_Y - 30);
        char tStr[1000];
        strcpy(tStr, "You get the title of MAZEMASTER. YOUR ");
        strcat(tStr, timeString);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char *) tStr);
    } else if (matchState == -1) {
        SetColor(220, 60, 60);
        glRasterPos2f(SCREEN_WIDTH / 3 + 20, MAZE_POS_Y - 50);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, "AN ENEMY FOUND YOU!!!");
        glRasterPos2f(SCREEN_WIDTH / 3 + 40, MAZE_POS_Y - 30);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, "Press 'R' to replay.");
    }

    if (!mazeComplete && !matchState) {
        if (walking) {
            SetColor(120, 180, 240);
            glRasterPos2f(TEXT_POSITION_X + 100, TEXT_POSITION_Y);
            glutBitmapString(GLUT_BITMAP_HELVETICA_18, "WALKING");

            SetColor(90, 110, 110);
            glRasterPos2f(TEXT_POSITION_X + 100, TEXT_POSITION_Y + 20);
            glutBitmapString(GLUT_BITMAP_HELVETICA_12, "HUNTING");
        } else {
            SetColor(90, 110, 110);
            glRasterPos2f(TEXT_POSITION_X + 100, TEXT_POSITION_Y);
            glutBitmapString(GLUT_BITMAP_HELVETICA_12, "WALKING");

            SetColor(120, 180, 240);
            glRasterPos2f(TEXT_POSITION_X + 100, TEXT_POSITION_Y + 20);
            glutBitmapString(GLUT_BITMAP_HELVETICA_18, "HUNTING");
        }

        SetColor(90, 140, 140);
        glRasterPos2f(TEXT_POSITION_X + 800, TEXT_POSITION_Y);
        glutBitmapString(GLUT_BITMAP_HELVETICA_12, "PRESS 'A' FOR NORMAL SPEED");

        SetColor(90, 140, 140);
        glRasterPos2f(TEXT_POSITION_X + 800, TEXT_POSITION_Y + 20);
        glutBitmapString(GLUT_BITMAP_HELVETICA_12, "PRESS 'S' FOR STEP-BY-STEP");
    }

    if (mazeComplete && matchState == 0) {
        SetColor(90, 140, 140);
        glRasterPos2f(TEXT_POSITION_X - 75, TEXT_POSITION_Y + 100);
        glutBitmapString(GLUT_BITMAP_HELVETICA_12,
                         "MAZE HAS BEEN COMPLETED\nYOU CAN PLAY THE GAME NOW\nTRY TO REACH THE DESTINATION");

        SetColor(90, 140, 140);
        glRasterPos2f(TEXT_POSITION_X + 775, TEXT_POSITION_Y + 100);
        glutBitmapString(GLUT_BITMAP_HELVETICA_12, "PRESS ARROWS KEYS TO MOVE\nINSIDE THE MAZE");
    }

    if (!mazeComplete) {
        if (steppingMode && doStep || !steppingMode) {
            if (currentDirection == CLIFF) {
                Hunt();                     //When gets trapped
            } else {
                Walk();                    //When free to walk
            }
            doStep = false;
        }
    } else if (matchState == 0) {
        if (!randomWallsBroken) {
            BreakRandomWalls();
            randomWallsBroken = true;
        }
        DrawDestination(destRoomX, destRoomY);
        DrawPlayerInRoom(playerRoomX, playerRoomY);

        MoveEnemiesInMaze();
        DrawEnemies();

        if (playerRoomX == destRoomX && playerRoomY == destRoomX) {
            matchState = 1;
        } else if (PlayerCollidingWithEnemies()) {
            matchState = -1;
        }
    }

    if (!isStartScreenOver) {
        DisplayFrontPage();
    } else {

    }

    if (!walking)
        Sleep(DELTA_TIME);

    Sleep(DELTA_TIME);                 //Limiting the speed of program

    glFlush();

    glutPostRedisplay();              //Calling the display function again

}

void InitMaze() {
    int i, j, k;
    for (i = 0; i < MAZESIZE; ++i) {
        for (j = 0; j < MAZESIZE; ++j) {
            //Marking all rooms an un-visited initially
            mazePrimary.room[i][j].visited = false;
            mazePrimary.room[i][j].playerVisited = false;

            //Marking all walls as true
            for (k = 0; k < 4; k++) { mazePrimary.room[i][j].wall[k] = true; }
        }
    }

    //Staring from a random position in maze
    currentRoomX = rand() % MAZESIZE;
    currentRoomY = rand() % MAZESIZE;
    mazePrimary.room[currentRoomX][currentRoomY].visited = true;

    mazeComplete = false;
}

void InitGame() {
    playerRoomX = 0;
    playerRoomY = 0;
    lastPlayerRoomX = playerRoomX;
    lastPlayerRoomY = playerRoomY;

    destRoomX = MAZESIZE - 1;
    destRoomY = MAZESIZE - 1;

    matchState = 0;
}

void InitEnemies() {
    nEnemies = rand() % 8 + 5;
//    nEnemies = 1;
    int i, j;
    int radius = ROOMSIZE * 2 / 3;
    for (i = 0; i < nEnemies; i++) {
        enemies[i].xRoom = rand() % MAZESIZE;
        enemies[i].yRoom = rand() % MAZESIZE;

        for (j = 0; j < 4; j++) {
            enemies[i].randRadius[j] = radius / 4 + rand() % (int) (radius / 4);
            enemies[i].randPosX[j] = rand() % (int) radius / 2 - rand() % (int) radius / 2;
            enemies[i].randPosY[j] = rand() % (int) radius / 2 - rand() % (int) radius / 2;
            enemies[i].color[j].r = 55;
            enemies[i].color[j].g = 100 + rand() % 50;
            enemies[i].color[j].b = 55;
        }
    }
}

void KeyboardFunction(int key) {
    switch (key) {
        case 's':
            if (steppingMode == true) {
                doStep = true;
            } else {
                steppingMode = true;
            }
            break;
        case 'S':
            if (steppingMode == true) {
                doStep = true;
            } else {
                steppingMode = true;
            }
            break;
        case 'a':
            steppingMode = false;
            doStep = false;
            break;
        case 'A':
            steppingMode = false;
            doStep = false;
            break;
        case 'r':
            steppingMode = false;
            DELTA_TIME = 0;
            InitMaze();
            InitGame();
            InitEnemies();
            break;
        case 'R':
            steppingMode = false;
            DELTA_TIME = 0;
            InitMaze();
            InitGame();
            InitEnemies();
            break;
        case 13:
            isStartScreenOver = true;
        default:
            break;
    }
}

void PlayerMovementFunction(int key, int x, int y) {

    int directionArray[] = {LEFT, RIGHT, TOP, BOTTOM};
    int i, tempRoomX, tempRoomY;

    switch (key) {
        case GLUT_KEY_UP:
            currentDirection = directionArray[2];

            tempRoomX = playerRoomX + dx[currentDirection];
            tempRoomY = playerRoomY + dy[currentDirection];

            if (tempRoomX >= 0 && tempRoomX < MAZESIZE && tempRoomY >= 0 && tempRoomY < MAZESIZE &&
                mazePrimary.room[playerRoomX][playerRoomY].wall[TOP] == false) {

                playerRoomX = tempRoomX;
                playerRoomY = tempRoomY;
            }
            break;
        case GLUT_KEY_DOWN:
            currentDirection = directionArray[3];

            tempRoomX = playerRoomX + dx[currentDirection];
            tempRoomY = playerRoomY + dy[currentDirection];

            if (tempRoomX >= 0 && tempRoomX < MAZESIZE && tempRoomY >= 0 && tempRoomY < MAZESIZE &&
                mazePrimary.room[playerRoomX][playerRoomY].wall[BOTTOM] == false) {

                playerRoomX = tempRoomX;
                playerRoomY = tempRoomY;
            }
            break;
        case GLUT_KEY_LEFT:
            currentDirection = directionArray[0];

            tempRoomX = playerRoomX + dx[currentDirection];
            tempRoomY = playerRoomY + dy[currentDirection];

            if (tempRoomX >= 0 && tempRoomX < MAZESIZE && tempRoomY >= 0 && tempRoomY < MAZESIZE &&
                mazePrimary.room[playerRoomX][playerRoomY].wall[LEFT] == false) {

                playerRoomX = tempRoomX;
                playerRoomY = tempRoomY;
            }
            break;
        case GLUT_KEY_RIGHT:
            currentDirection = directionArray[1];

            tempRoomX = playerRoomX + dx[currentDirection];
            tempRoomY = playerRoomY + dy[currentDirection];

            if (tempRoomX >= 0 && tempRoomX < MAZESIZE && tempRoomY >= 0 && tempRoomY < MAZESIZE &&
                mazePrimary.room[playerRoomX][playerRoomY].wall[RIGHT] == false) {

                playerRoomX = tempRoomX;
                playerRoomY = tempRoomY;
            }
            break;
        default:
            currentDirection = CLIFF;
    }

//    if (lastPlayerRoomX != playerRoomX && lastPlayerRoomY != playerRoomY)
//    mazePrimary.room[lastPlayerRoomX][lastPlayerRoomY].playerVisited = !mazePrimary.room[lastPlayerRoomX][lastPlayerRoomY].playerVisited;
    mazePrimary.room[playerRoomX][playerRoomY].playerVisited = true;

    lastPlayerRoomX = playerRoomX;
    lastPlayerRoomY = playerRoomY;
};

/*void ReshapeFunction(GLsizei width, GLsizei height) {
    if (height == 0) height = 1;
    GLfloat aspect = (GLfloat) width / (GLfloat) height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (width >= height) {
        gluOrtho2D(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
    } else {
        gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);
    }
}*/

int main(int argc, char **argv) {
    glutInit(&argc, argv);

//    printf("ENTER THE SIZE OF MAZE YOU WANT (<80): ");
//    scanf("%d", &MAZESIZE);
//    while (MAZESIZE < 1 || MAZESIZE > 80) {
//        if (MAZESIZE < 1)
//            printf("Cannot make a maze with zero rooms!\nEnter maze size >0 and <80: ");
//        else
//            printf("I can make a maze that large...\nif you have a million years!\nEnter maze size >0 and <80: ");
//        scanf("%d", &MAZESIZE);
//    }

    ROOMSIZE = (SCREEN_HEIGHT - SCREEN_HEIGHT / 8) / MAZESIZE;

    // Draw Display window
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    //Position of display window
    glutInitWindowPosition(20, 20);

    int winID = glutCreateWindow("Maze Generation");
//    ShowWindow(winID, SW_SHOW);
//    SetForegroundWindow(winID);
//    SetFocus(winID);

//    glutReshapeFunc(ReshapeFunction);

    //Registering display function
    glutDisplayFunc(display);

    //Registering Special Keyboard (Non ASCII) function
    glutSpecialFunc(PlayerMovementFunction);

    //Registering Keyboard function
    glutKeyboardFunc(KeyboardFunction);

    // New coordinates according to the display window
    gluOrtho2D(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    srand(time(NULL));
    // Width of the wall
    glLineWidth(50 / MAZESIZE);

    InitMaze();
    InitGame();
    InitEnemies();

    glutMainLoop();

    return 0;
}