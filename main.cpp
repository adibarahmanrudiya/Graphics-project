#include <windows.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>

const float PI = 3.14159265f;


float birdX      = -50.0f;
float flagAngle  = 0.0f;
int   frameCount = 0;
float walkPhase  = 0.0f;

// Keyboard

float gPanX = 0.0f, gPanY = 0.0f;
float gZoom = 1.0f;
float gRotateAngle = 0.0f;


bool isNight   = false;
bool isRaining = false;

const int RAIN_DROPS = 80;
float rainX[RAIN_DROPS];
float rainY[RAIN_DROPS];


const int PEOPLE_COUNT = 10;
float peopleX[PEOPLE_COUNT] = { 150, 450, 468, 250, 400, 320, 230, 360, -30, 630 };
float peopleSpeed[PEOPLE_COUNT] = { 1.1f, -1.3f, 1.2f, -0.9f, 1.4f, -1.1f, 1.0f, -1.2f, 1.4f, -1.4f };

void bezierPoint(float t, float x0, float y0, float x1, float y1, float x2, float y2,
                    float &outX, float &outY)
{
    float u = 1.0f - t;
    outX = u * u * x0 + 2.0f * u * t * x1 + t * t * x2;
    outY = u * u * y0 + 2.0f * u * t * y1 + t * t * y2;
}


const int BEZIER_STEPS = 48;
const int OUTLINE_PTS  = 2 * (BEZIER_STEPS + 1);

int buildShardOutline(float baseCenterX, float apexX, float baseY, float apexY, float halfWidth,
                        float outline[][2])
{
    float leftBaseX  = baseCenterX - halfWidth;
    float rightBaseX = baseCenterX + halfWidth;
    float ctrlX_L    = apexX - halfWidth * 0.15f;
    float ctrlX_R    = apexX + halfWidth * 0.15f;
    float ctrlY      = baseY + (apexY - baseY) * 0.22f;

    int n = 0;
    for (int i = 0; i <= BEZIER_STEPS; i++)
    {
        float t = (float)i / BEZIER_STEPS;
        bezierPoint(t, leftBaseX, baseY, ctrlX_L, ctrlY, apexX, apexY, outline[n][0], outline[n][1]);
        n++;
    }
    for (int i = 0; i <= BEZIER_STEPS; i++)
    {
        float t = (float)i / BEZIER_STEPS;
        bezierPoint(t, apexX, apexY, ctrlX_R, ctrlY, rightBaseX, baseY, outline[n][0], outline[n][1]);
        n++;
    }
    return n;
}

void drawShardFill(float outline[][2], int n, float r, float g, float b)
{
    glBegin(GL_POLYGON);
        glColor3f(r, g, b);
        for (int i = 0; i < n; i++)
            glVertex2f(outline[i][0], outline[i][1]);
    glEnd();
}


// Monument
void drawMonument()
{
    const int HALF_PAIRS = 6;
    const int SHARDS     = 2 * HALF_PAIRS + 1;

    float centerX     = 300.0f;
    float baseY       = 150.0f;
    float maxH        = 290.0f;
    float minH        = 65.0f;
    float baseSpacing = 31.0f;
    float halfWidth   = 19.0f;

    float baseCx[SHARDS], apexCx[SHARDS], apexYArr[SHARDS], shade[SHARDS];
    int count = 0;

    for (int ad = HALF_PAIRS; ad >= 0; ad--)
    {
        float h  = maxH - ad * ((maxH - minH) / (float)HALF_PAIRS);
        float sd = (ad % 2 == 0) ? 0.58f : 0.68f;

        if (ad == 0)
        {
            baseCx[count] = centerX;  apexCx[count] = centerX;
            apexYArr[count] = baseY + h;  shade[count] = sd;
            count++;
        }
        else
        {

            baseCx[count] = centerX - ad * baseSpacing;  apexCx[count] = centerX;
            apexYArr[count] = baseY + h;  shade[count] = sd;
            count++;

            baseCx[count] = centerX + ad * baseSpacing;  apexCx[count] = centerX;
            apexYArr[count] = baseY + h;  shade[count] = sd;
            count++;
        }
    }

    float outline[OUTLINE_PTS][2];


    for (int i = 0; i < count; i++)
    {
        int n = buildShardOutline(baseCx[i], apexCx[i], baseY, apexYArr[i], halfWidth, outline);
        drawShardFill(outline, n, shade[i], shade[i], shade[i]);
    }
}

// Sky
void drawSky()
{
    glBegin(GL_QUADS);
        if (isNight) glColor3f(0.06f, 0.08f, 0.20f);
        else         glColor3f(0.75f, 0.85f, 0.95f);
        glVertex2f(0, 150);
        glVertex2f(600, 150);
        glVertex2f(600, 600);
        glVertex2f(0, 600);
    glEnd();
}

//Sun
void drawSun()
{
    if (isNight) return;

    float cx = 520.0f, cy = 500.0f, r = 32.0f;

    // Rays
    glLineWidth(2.5f);
    glBegin(GL_LINES);
        glColor3f(1.0f, 0.75f, 0.25f);
        for (int a = 0; a < 360; a += 30)
        {
            float angle = a * PI / 180.0f;
            glVertex2f(cx + (r + 6.0f) * cos(angle), cy + (r + 6.0f) * sin(angle));
            glVertex2f(cx + (r + 24.0f) * cos(angle), cy + (r + 24.0f) * sin(angle));
        }
    glEnd();

    // Sun
    glBegin(GL_POLYGON);
        glColor3f(1.0f, 0.82f, 0.30f);
        for (int a = 0; a <= 360; a += 20)
        {
            float angle = a * PI / 180.0f;
            glVertex2f(cx + r * cos(angle), cy + r * sin(angle));
        }
    glEnd();
}

//Moon and stars
void drawMoon()
{
    if (!isNight) return;

    float cx = 520.0f, cy = 500.0f, r = 26.0f;
    glBegin(GL_POLYGON);
        glColor3f(0.92f, 0.92f, 0.85f);
        for (int a = 0; a <= 360; a += 20)
        {
            float angle = a * PI / 180.0f;
            glVertex2f(cx + r * cos(angle), cy + r * sin(angle));
        }
    glEnd();

    //  shadow
    glBegin(GL_POLYGON);
        if (isNight) glColor3f(0.06f, 0.08f, 0.20f);
        for (int a = 0; a <= 360; a += 20)
        {
            float angle = a * PI / 180.0f;
            glVertex2f(cx + 10.0f + r * 0.85f * cos(angle), cy + r * 0.85f * sin(angle));
        }
    glEnd();
}

void drawStars()
{
    if (!isNight) return;

    glColor3f(1.0f, 1.0f, 0.95f);
    float starsX[] = { 40, 90, 150, 210, 270, 330, 560, 20, 480, 130, 350, 590 };
    float starsY[] = { 560, 520, 580, 540, 570, 510, 430, 400, 380, 460, 560, 350 };
    int n = sizeof(starsX) / sizeof(starsX[0]);

    glPointSize(2.5f);
    glBegin(GL_POINTS);
        for (int i = 0; i < n; i++)
            glVertex2f(starsX[i], starsY[i]);
    glEnd();
}

//  Rain
void drawRain()
{
    if (!isRaining) return;

    glLineWidth(1.5f);
    glColor3f(0.7f, 0.8f, 0.95f);
    glBegin(GL_LINES);
        for (int i = 0; i < RAIN_DROPS; i++)
        {
            glVertex2f(rainX[i], rainY[i]);
            glVertex2f(rainX[i] - 4.0f, rainY[i] - 14.0f);
        }
    glEnd();
}


//  Ground
void drawGround()
{
    glBegin(GL_QUADS);
        glColor3f(0.55f, 0.27f, 0.19f);
        glVertex2f(0, 0);
        glVertex2f(600, 0);
        glVertex2f(600, 150);
        glVertex2f(0, 150);
    glEnd();
}

//Low brick walls
void drawWall(float xStart, float xEnd, float yTop)
{
    // Wall body
    glBegin(GL_QUADS);
        glColor3f(0.40f, 0.18f, 0.12f);
        glVertex2f(xStart, 0);
        glVertex2f(xEnd, 0);
        glVertex2f(xEnd, yTop);
        glVertex2f(xStart, yTop);
    glEnd();


    glBegin(GL_QUADS);
        glColor3f(0.62f, 0.42f, 0.32f);
        glVertex2f(xStart, yTop - 3.0f);
        glVertex2f(xEnd, yTop - 3.0f);
        glVertex2f(xEnd, yTop);
        glVertex2f(xStart, yTop);
    glEnd();
}

void drawWalls()
{

    drawWall(10.0f, 95.0f, 24.0f);
    drawWall(505.0f, 590.0f, 24.0f);


    drawWall(84.0f, 100.0f, 45.0f);
    drawWall(75.0f, 108.0f, 10.0f);

    drawWall(500.0f, 516.0f, 45.0f);
    drawWall(492.0f, 525.0f, 10.0f);


    drawWall(178.0f, 194.0f, 45.0f);
    drawWall(170.0f, 202.0f, 10.0f);

    drawWall(406.0f, 422.0f, 45.0f);
    drawWall(398.0f, 430.0f, 10.0f);


    drawWall(0.0f, 22.0f, 100.0f);
    drawWall(0.0f, 22.0f, 30.0f);

    drawWall(578.0f, 600.0f, 100.0f);
    drawWall(578.0f, 600.0f, 30.0f);
}

//person
void drawRect(float xLeft, float yBottom, float w, float h, float r, float g, float b)
{
    glBegin(GL_QUADS);
        glColor3f(r, g, b);
        glVertex2f(xLeft,     yBottom);
        glVertex2f(xLeft + w, yBottom);
        glVertex2f(xLeft + w, yBottom + h);
        glVertex2f(xLeft,     yBottom + h);
    glEnd();
}

// walking person

void drawLimb(float length, float width, float r, float g, float b)
{
    glBegin(GL_QUADS);
        glColor3f(r, g, b);
        glVertex2f(-width * 0.5f, 0);
        glVertex2f(width * 0.5f, 0);
        glVertex2f(width * 0.5f, -length);
        glVertex2f(-width * 0.5f, -length);
    glEnd();
}

void drawPerson(float x, float y, float scale, float r, float g, float b, float phase)
{
    float hipY      = y + scale * 4.4f;
    float shoulderY = y + scale * 7.0f;
    float headY     = y + scale * 8.3f;

    float thigh = scale * 2.0f, shin = scale * 1.8f;
    float upperArm = scale * 1.8f, forearm = scale * 1.6f;

    float legSwing = sin(phase) * 10.0f;
    float lSwing =  legSwing, rSwing = -legSwing;
    float lKnee  = (lSwing < 0.0f) ? -lSwing * 0.5f : 0.0f;
    float rKnee  = (rSwing < 0.0f) ? -rSwing * 0.5f : 0.0f;
    float armSwing = -legSwing * 0.7f;


    glBegin(GL_POLYGON);
        glColor3f(0.35f, 0.32f, 0.29f);
        for (int a = 0; a <= 360; a += 30)
        {
            float angle = a * PI / 180.0f;
            glVertex2f(x + scale * 1.1f * cos(angle), y + scale * 0.3f * sin(angle));
        }
    glEnd();

    // Left leg
    glPushMatrix();
        glTranslatef(x - scale * 0.55f, hipY, 0);
        glRotatef(lSwing, 0, 0, 1);
        drawLimb(thigh, scale * 0.75f, 0.16f, 0.16f, 0.21f);
        glTranslatef(0, -thigh, 0);
        glRotatef(lKnee, 0, 0, 1);
        drawLimb(shin, scale * 0.62f, 0.13f, 0.13f, 0.18f);
        glTranslatef(0, -shin, 0);
        glBegin(GL_QUADS);
            glColor3f(0.06f, 0.06f, 0.06f);
            glVertex2f(-scale * 0.5f, 0);
            glVertex2f(scale * 0.65f, 0);
            glVertex2f(scale * 0.65f, -scale * 0.45f);
            glVertex2f(-scale * 0.5f, -scale * 0.45f);
        glEnd();
    glPopMatrix();

    //Right leg
    glPushMatrix();
        glTranslatef(x + scale * 0.55f, hipY, 0);
        glRotatef(rSwing, 0, 0, 1);
        drawLimb(thigh, scale * 0.75f, 0.16f, 0.16f, 0.21f);
        glTranslatef(0, -thigh, 0);
        glRotatef(rKnee, 0, 0, 1);
        drawLimb(shin, scale * 0.62f, 0.13f, 0.13f, 0.18f);
        glTranslatef(0, -shin, 0);
        glBegin(GL_QUADS);
            glColor3f(0.06f, 0.06f, 0.06f);
            glVertex2f(-scale * 0.5f, 0);
            glVertex2f(scale * 0.65f, 0);
            glVertex2f(scale * 0.65f, -scale * 0.45f);
            glVertex2f(-scale * 0.5f, -scale * 0.45f);
        glEnd();
    glPopMatrix();


    glBegin(GL_QUADS);
        glColor3f(r, g, b);
        glVertex2f(x - scale * 1.05f, hipY);
        glVertex2f(x + scale * 1.05f, hipY);
        glVertex2f(x + scale * 0.95f, shoulderY);
        glVertex2f(x - scale * 0.95f, shoulderY);
    glEnd();

    //Left arm
    glPushMatrix();
        glTranslatef(x - scale * 0.95f, shoulderY, 0);
        glRotatef(-armSwing, 0, 0, 1);
        drawLimb(upperArm, scale * 0.55f, r * 0.9f, g * 0.9f, b * 0.9f);
        glTranslatef(0, -upperArm, 0);
        drawLimb(forearm, scale * 0.45f, 0.92f, 0.78f, 0.62f);
    glPopMatrix();

    //Right arm
    glPushMatrix();
        glTranslatef(x + scale * 0.95f, shoulderY, 0);
        glRotatef(armSwing, 0, 0, 1);
        drawLimb(upperArm, scale * 0.55f, r * 0.9f, g * 0.9f, b * 0.9f);
        glTranslatef(0, -upperArm, 0);
        drawLimb(forearm, scale * 0.45f, 0.92f, 0.78f, 0.62f);
    glPopMatrix();

    // Head
    glBegin(GL_POLYGON);
        glColor3f(0.92f, 0.78f, 0.62f);
        for (int a = 0; a <= 360; a += 30)
        {
            float angle = a * PI / 180.0f;
            glVertex2f(x + scale * 0.85f * cos(angle), headY + scale * 0.85f * sin(angle));
        }
    glEnd();
}

void drawPeople()
{
    float laneY[PEOPLE_COUNT]    = { 50, 45, 42, 62, 100, 55, 45, 95, 55, 70 };
    float laneScale[PEOPLE_COUNT] = { 5.0f, 5.0f, 4.5f, 4.0f, 4.0f, 4.2f, 4.0f, 3.8f, 4.5f, 4.5f };
    float laneR[PEOPLE_COUNT] = { 0.15f, 0.35f, 0.15f, 0.2f, 0.15f, 0.2f, 0.15f, 0.25f, 0.15f, 0.2f };
    float laneG[PEOPLE_COUNT] = { 0.15f, 0.1f,  0.15f, 0.2f, 0.15f, 0.15f,0.15f, 0.15f, 0.15f, 0.1f };
    float laneB[PEOPLE_COUNT] = { 0.15f, 0.1f,  0.15f, 0.2f, 0.15f, 0.15f,0.2f,  0.1f,  0.15f, 0.1f };

    for (int i = 0; i < PEOPLE_COUNT; i++)
    {
        float phase = walkPhase * (peopleSpeed[i] > 0 ? 1.0f : -1.0f) + i * 0.7f;
        drawPerson(peopleX[i], laneY[i], laneScale[i], laneR[i], laneG[i], laneB[i], phase);
    }
}

//  pool
void drawPool()
{
    float cx = 300.0f, cy = 90.0f;
    float rx = 110.0f, ry = 16.0f;


    glBegin(GL_POLYGON);
        glColor3f(0.55f, 0.75f, 0.85f);
        for (int i = 0; i <= 360; i += 10)
        {
            float angle = i * PI / 180.0f;
            glVertex2f(cx + rx * cos(angle), cy + ry * sin(angle));
        }
    glEnd();


    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glColor3f(0.25f, 0.35f, 0.4f);
        for (int i = 0; i < 360; i += 10)
        {
            float angle = i * PI / 180.0f;
            glVertex2f(cx + rx * cos(angle), cy + ry * sin(angle));
        }
    glEnd();
}


// Flag
void drawFlag()
{
    float poleX = 290.0f;


    glLineWidth(4.0f);
    glBegin(GL_LINES);
        glColor3f(0.2f, 0.2f, 0.2f);
        glVertex2f(poleX, 30);
        glVertex2f(poleX, 200);
    glEnd();


    glBegin(GL_TRIANGLES);
        glColor3f(0.2f, 0.2f, 0.2f);
        glVertex2f(poleX - 11, 30);
        glVertex2f(poleX + 11, 30);
        glVertex2f(poleX, 44);
    glEnd();

    glPushMatrix();
        glTranslated(poleX, 180, 0);
        glRotated(flagAngle, 0, 0, 1);


        glBegin(GL_QUADS);
            glColor3f(0.0f, 0.4f, 0.2f);
            glVertex2f(0, 0);
            glVertex2f(42, 0);
            glVertex2f(42, -21);
            glVertex2f(0, -21);
        glEnd();


        glBegin(GL_POLYGON);
            glColor3f(0.8f, 0.0f, 0.0f);
            for (int i = 0; i <= 360; i += 20)
            {
                float angle = i * PI / 180.0f;
                float x = 17 + 7 * cos(angle);
                float y = -10 + 7 * sin(angle);
                glVertex2f(x, y);
            }
        glEnd();
    glPopMatrix();
}

//small bird
void drawBird(float x, float y)
{
    glPushMatrix();
        glTranslated(x, y, 0);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
            glColor3f(0.1f, 0.1f, 0.1f);
            glVertex2f(-8, 0);
            glVertex2f(-3, 4);
            glVertex2f(0, 0);
            glVertex2f(3, 4);
            glVertex2f(8, 0);
        glEnd();
    glPopMatrix();
}

void drawBirds()
{
    drawBird(birdX, 480);
    drawBird(birdX - 120, 520);
}

// Simple tree
void drawTree(float cx, float cy, float r)
{
    glBegin(GL_QUADS);
        glColor3f(0.4f, 0.25f, 0.1f);
        glVertex2f(cx - 3, cy - 20);
        glVertex2f(cx + 3, cy - 20);
        glVertex2f(cx + 3, cy);
        glVertex2f(cx - 3, cy);
    glEnd();

    glBegin(GL_POLYGON);
        glColor3f(0.13f, 0.45f, 0.15f);
        for (int i = 0; i <= 360; i += 20)
        {
            float angle = i * PI / 180.0f;
            glVertex2f(cx + r * cos(angle), cy + r * sin(angle));
        }
    glEnd();
}

void drawTrees()
{
    drawTree(60, 170, 25);
    drawTree(110, 165, 20);
    drawTree(500, 165, 20);
    drawTree(550, 170, 25);
}

//A more realistic tree
void drawFoliageClump(float cx, float cy, float r, float shadeMul)
{
    glBegin(GL_POLYGON);
        glColor3f(0.10f * shadeMul, 0.42f * shadeMul, 0.14f * shadeMul);
        for (int a = 0; a <= 360; a += 24)
        {
            float angle = a * PI / 180.0f;
            glVertex2f(cx + r * cos(angle), cy + r * sin(angle));
        }
    glEnd();
}

void drawRealisticTree(float baseX, float baseY, float scale)
{

    glBegin(GL_QUADS);
        glColor3f(0.32f, 0.20f, 0.10f);
        glVertex2f(baseX - 3.0f * scale, baseY);
        glVertex2f(baseX + 3.0f * scale, baseY);
        glVertex2f(baseX + 1.6f * scale, baseY + 16.0f * scale);
        glVertex2f(baseX - 1.6f * scale, baseY + 16.0f * scale);
    glEnd();

    float topY = baseY + 15.0f * scale;
    drawFoliageClump(baseX - 6.0f * scale, topY + 2.0f * scale, 9.0f * scale, 0.85f);
    drawFoliageClump(baseX + 6.0f * scale, topY + 1.0f * scale, 8.5f * scale, 1.05f);
    drawFoliageClump(baseX,               topY + 9.0f * scale, 10.0f * scale, 1.0f);
    drawFoliageClump(baseX - 3.0f * scale, topY + 13.0f * scale, 7.0f * scale, 1.15f);
    drawFoliageClump(baseX + 4.0f * scale, topY + 12.0f * scale, 7.5f * scale, 0.95f);
}

void drawBigTrees()
{
    drawRealisticTree(75.0f,  150.0f, 2.7f);
    drawRealisticTree(150.0f, 150.0f, 3.3f);
    drawRealisticTree(450.0f, 150.0f, 3.3f);
    drawRealisticTree(525.0f, 150.0f, 2.7f);
    drawRealisticTree(50.0f,  150.0f, 2.3f);  // extra tree, far left edge
}
//jungle
void drawJungleBackdrop()
{
    float positions[] = { 0, 40, 80, 120, 160, 200, 240, 280, 320, 360, 400, 440, 480, 520, 560, 600 };
    int   count = sizeof(positions) / sizeof(positions[0]);

    for (int i = 0; i < count; i++)
    {
        float cx = positions[i];
        float cy = 195.0f + (i % 2) * 5.0f;
        float r  = 20.0f + (i % 3) * 3.0f;
        float m  = 0.55f + (i % 3) * 0.08f;

        glBegin(GL_POLYGON);
            glColor3f(0.12f * m, 0.30f * m, 0.15f * m);
            for (int a = 0; a <= 360; a += 30)
            {
                float angle = a * PI / 180.0f;
                glVertex2f(cx + r * cos(angle), cy + r * sin(angle));
            }
        glEnd();
    }
}

//Dense

void drawBackgroundTrees()
{
    float positions[] = { 20, 70, 120, 170, 220, 270, 330, 380, 430, 480, 530, 580 };
    int   count = sizeof(positions) / sizeof(positions[0]);

    for (int i = 0; i < count; i++)
    {
        float cx = positions[i];
        float cy = 180.0f + (i % 3) * 4.0f;
        float r  = 18.0f + (i % 3) * 3.0f;
        float shade = (i % 2 == 0) ? 0.16f : 0.20f;

        glBegin(GL_POLYGON);
            glColor3f(shade * 0.5f, shade + 0.16f, shade * 0.6f);
            for (int a = 0; a <= 360; a += 30)
            {
                float angle = a * PI / 180.0f;
                glVertex2f(cx + r * cos(angle), cy + r * sin(angle));
            }
        glEnd();
    }
}


void applyProjection();
void drawNightOverlay();

void display()
{
    applyProjection();
    glClear(GL_COLOR_BUFFER_BIT);

    drawSky();
    drawSun();
    drawMoon();
    drawStars();
    drawTrees();
    drawGround();
    drawWalls();
    drawJungleBackdrop();
    drawBackgroundTrees();
    drawBigTrees();
    drawMonument();
    drawPool();
    drawFlag();
    drawPeople();
    drawBirds();
    drawNightOverlay();
    drawRain();

    glFlush();
}


void timer(int value)
{
    frameCount++;

    birdX += 3.0f;
    if (birdX > 650.0f) {
        birdX = -50.0f;
    }

    flagAngle = 12.0f * sin(frameCount * 0.1f);

    walkPhase += 0.35f;

    for (int i = 0; i < PEOPLE_COUNT; i++)
    {
        peopleX[i] += peopleSpeed[i];
        if (peopleX[i] > 630.0f) peopleX[i] = -30.0f;
        if (peopleX[i] < -30.0f) peopleX[i] = 630.0f;
    }

    if (isRaining)
    {
        for (int i = 0; i < RAIN_DROPS; i++)
        {
            rainY[i] -= 14.0f;
            if (rainY[i] < 0.0f)
            {
                rainY[i] = 600.0f;
                rainX[i] = (float)(rand() % 600);
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(30, timer, 0);
}

void applyProjection()
{
    float hw = 300.0f / gZoom;
    float hh = 300.0f / gZoom;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(300.0f - hw + gPanX, 300.0f + hw + gPanX,
               300.0f - hh + gPanY, 300.0f + hh + gPanY);
    glRotatef(gRotateAngle, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
}

void init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 600, 0, 600);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < RAIN_DROPS; i++)
    {
        rainX[i] = (float)(rand() % 600);
        rainY[i] = (float)(rand() % 600);
    }
}

void drawNightOverlay()
{
    if (!isNight) return;

    glBegin(GL_QUADS);
        glColor4f(0.04f, 0.05f, 0.15f, 0.45f);
        glVertex2f(0, 0);
        glVertex2f(600, 0);
        glVertex2f(600, 600);
        glVertex2f(0, 600);
    glEnd();
}

void reshape(int w, int h)
{
    int side = (w < h) ? w : h;
    glViewport((w - side) / 2, (h - side) / 2, side, side);
    applyProjection();
}

//Keyboard
void keyboard(unsigned char key, int x, int y)
{
    if (key == 'd' || key == 'D') { isNight = false; }
    else if (key == 'n' || key == 'N') { isNight = true; }
    else if (key == 'r' || key == 'R') { isRaining = !isRaining; }
    else if (key == '+' || key == '=') { gZoom *= 1.1f; }
    else if (key == '-' || key == '_') { gZoom /= 1.1f; if (gZoom < 0.1f) gZoom = 0.1f; }
    else if (key == 'v' || key == 'V')
    {
        gPanX = 0.0f; gPanY = 0.0f; gZoom = 1.0f; gRotateAngle = 0.0f;
    }
    glutPostRedisplay();
}

//Arrow keys
void specialKeys(int key, int x, int y)
{
    float step = 12.0f;
    if (key == GLUT_KEY_LEFT)  gPanX -= step;
    if (key == GLUT_KEY_RIGHT) gPanX += step;
    if (key == GLUT_KEY_UP)    gPanY += step;
    if (key == GLUT_KEY_DOWN)  gPanY -= step;
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(150, 50);
    glutCreateWindow("Jatiyo Sriti Shoudho");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(20, timer, 0);   // animation loop
    glutMainLoop();
    return 0;
}
