#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

const float PI_VALUE = 3.14159265f;
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const int TIMER_INTERVAL_MS = 16;
const int RAIN_DROP_COUNT = 500;

const float homeGarageDoorBottomY = 300.0f;
const float homeGarageDoorHeight = 110.0f;

int currentScene = 1;
int FIRST_SCENE_INDEX = 1;
int LAST_SCENE_INDEX = 9;
bool isRainEnabled = false;
bool isPaused = false;
float animationSpeed = 1.0f;

float rainDropX[RAIN_DROP_COUNT];
float rainDropY[RAIN_DROP_COUNT];

float grassSwayTimer = 0.0f;

float sunHorizontalOffset = 0.0f;
float cloudOffsetX_layerA = 0.0f;
float cloudOffsetX_layerB = 0.0f;
float cloudOffsetX_layerC = 0.0f;
float cloudDriftSpeedA = 0.0f;
float cloudDriftSpeedB = 0.0f;
float cloudDriftSpeedC = 0.0f;

float wheelRotationAngle = 0.0f;
float garageDoorOpenAmount = 0.0f;

float sunGlowPulse = 0.0f;
float sunGlowDirection = 1.0f;

float scene1_carPosX = 400.0f;
float scene1_carPosY = 50.0f;
int carState_scene1 = 0;
bool scene1HasCarExitedScreen = false;
float scene1_carAngle = 0.0f;

float scene9_carPosX = 500.0f;
float scene9_carPosY = 50.0f;
int carState_scene9 = 1;
bool scene9ParkingCompleted = false;
bool isHouseLightOn = true;
float scene9_carAngle = 0.0f;

int sceneFrameCounter = 0;
int scene9ParkedFrameCounter = 0;

float windowFlickerTimer = 0.0f;
float windowFlickerAmount[10];

// BASIC SHAPES
void rect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void circle(float cx, float cy, float r, int seg) {
    if (seg < 12) seg = 12;
    glBegin(GL_POLYGON);
    for (int i = 0; i < seg; i++) {
        float a = 2.0f * PI_VALUE * i / seg;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }
    glEnd();
}

void ellipse(float cx, float cy, float rx, float ry, int seg) {
    if (seg < 12) seg = 12;
    glBegin(GL_POLYGON);
    for (int i = 0; i < seg; i++) {
        float a = 2.0f * PI_VALUE * i / seg;
        glVertex2f(cx + cos(a) * rx, cy + sin(a) * ry);
    }
    glEnd();
}

// ENVIRONMENT ELEMENTS
void gradSky(float br, float bg, float bb, float tr, float tg, float tb) {
    glBegin(GL_QUADS);
    glColor3f(br, bg, bb); glVertex2f(0, 0);
    glColor3f(br, bg, bb); glVertex2f(1280, 0);
    glColor3f(tr, tg, tb); glVertex2f(1280, 720);
    glColor3f(tr, tg, tb); glVertex2f(0, 720);
    glEnd();
}

void cloud(float cx, float cy, float alpha) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (isRainEnabled) {
        glColor4f(0.45f, 0.49f, 0.53f, alpha);
    } else {
        glColor4f(0.96f, 0.96f, 0.98f, alpha);
    }

    circle(cx, cy, 32, 32);
    circle(cx - 35, cy - 6, 26, 28);
    circle(cx + 35, cy - 4, 28, 28);
    circle(cx + 5, cy + 20, 24, 28);
    circle(cx - 20, cy + 12, 20, 24);
    circle(cx + 22, cy + 14, 22, 24);

    if (!isRainEnabled) {
        glColor4f(1.0f, 1.0f, 1.0f, alpha * 0.6f);
        circle(cx - 5, cy + 16, 16, 20);
        circle(cx + 8, cy + 22, 12, 18);
    }

    glDisable(GL_BLEND);
}

void sun(float cx, float cy) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float glow = 0.5f + sunGlowPulse * 0.5f;

    glColor4f(1.0f, 0.95f, 0.5f, 0.1f * glow);
    circle(cx, cy, 80, 48);

    glColor4f(1.0f, 0.9f, 0.4f, 0.2f * glow);
    circle(cx, cy, 60, 48);

    glColor4f(1.0f, 0.85f, 0.3f, 0.35f * glow);
    circle(cx, cy, 48, 48);

    glColor3f(1.0f, 0.92f, 0.32f);
    circle(cx, cy, 36, 48);

    glColor4f(1.0f, 0.98f, 0.7f, 0.6f);
    circle(cx, cy, 20, 36);

    glColor4f(1.0f, 0.84f, 0.22f, 0.4f * glow);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    for (int i = 0; i < 12; i++) {
        float angle = i * PI_VALUE * 2.0f / 12.0f;
        float startR = 44.0f;
        float endR = 58.0f + glow * 10.0f;
        glVertex2f(cx + cos(angle) * startR, cy + sin(angle) * startR);
        glVertex2f(cx + cos(angle) * endR, cy + sin(angle) * endR);
    }
    glEnd();
    glLineWidth(1.0f);

    glDisable(GL_BLEND);
}

void moon(float cx, float cy) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.85f, 0.88f, 0.95f, 0.15f);
    circle(cx, cy, 50, 48);

    glColor4f(0.88f, 0.90f, 0.94f, 0.3f);
    circle(cx, cy, 40, 48);

    glColor3f(0.90f, 0.92f, 0.96f);
    circle(cx, cy, 30, 48);

    glColor3f(0.05f, 0.07f, 0.16f);
    circle(cx + 10, cy + 4, 26, 48);

    glColor4f(0.7f, 0.72f, 0.78f, 0.2f);
    circle(cx - 8, cy + 6, 8, 24);
    circle(cx + 2, cy - 10, 6, 20);

    glDisable(GL_BLEND);
}

void stars() {
    const int STAR_COUNT = 150;
    static float starX[STAR_COUNT];
    static float starY[STAR_COUNT];
    static float starSize[STAR_COUNT];
    static bool initialized = false;

    if (!initialized) {
        for (int i = 0; i < STAR_COUNT; i++) {
            starX[i] = rand() % WINDOW_WIDTH;
            starY[i] = 350 + (rand() % (WINDOW_HEIGHT - 350));
            starSize[i] = 1.0f + (rand() % 20) / 10.0f;
        }
        initialized = true;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (int i = 0; i < STAR_COUNT; i++) {
        float twinkle = 0.5f + 0.5f * sin(scene9ParkedFrameCounter * 0.05f + i);

        glPointSize(starSize[i]);
        glColor4f(1.0f, 1.0f, 0.9f, twinkle);
        glBegin(GL_POINTS);
            glVertex2f(starX[i], starY[i]);
        glEnd();

        if (i % 10 == 0) {
            glColor4f(1.0f, 1.0f, 1.0f, 0.4f * twinkle);
            glBegin(GL_LINES);
                glVertex2f(starX[i] - 6, starY[i]); glVertex2f(starX[i] + 6, starY[i]);
                glVertex2f(starX[i], starY[i] - 6); glVertex2f(starX[i], starY[i] + 6);
            glEnd();
        }
    }

    glDisable(GL_BLEND);
}

void drawFireflies() {
    struct Firefly {
        float x, y, phase, brightness;

        Firefly() {
            x = 100.0f + (rand() % 1080);
            y = 120.0f + (rand() % 500);
            phase = (rand() % 100) / 100.0f * 2.0f * PI_VALUE;
            brightness = 0.3f + (rand() % 70) / 100.0f;
        }
    };

    static Firefly fireflies[30];

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (int i = 0; i < 30; i++) {
        Firefly& f = fireflies[i];

        float glow = f.brightness * (0.5f + 0.5f * sin(grassSwayTimer * 0.04f + f.phase));

        f.x += sin(grassSwayTimer * 0.01f + f.phase) * 0.3f;
        f.y += cos(grassSwayTimer * 0.015f + f.phase) * 0.2f;

        if (f.x < 0) f.x = 1280;
        if (f.x > 1280) f.x = 0;
        if (f.y < 110) f.y = 600;
        if (f.y > 720) f.y = 110;

        glPointSize(8.0f);
        glColor4f(1.0f, 0.95f, 0.5f, 0.08f * glow);
        glBegin(GL_POINTS); glVertex2f(f.x, f.y); glEnd();

        glPointSize(4.0f);
        glColor4f(1.0f, 0.9f, 0.4f, 0.25f * glow);
        glBegin(GL_POINTS); glVertex2f(f.x, f.y); glEnd();

        glPointSize(2.0f);
        glColor4f(1.0f, 1.0f, 0.7f, 0.6f * glow);
        glBegin(GL_POINTS); glVertex2f(f.x, f.y); glEnd();
    }

    glDisable(GL_BLEND);
}

void initRain() {
    for (int i = 0; i < RAIN_DROP_COUNT; i++) {
        rainDropX[i] = rand() % WINDOW_WIDTH;
        rainDropY[i] = rand() % WINDOW_HEIGHT;
    }
}

void drawRain() {
    glColor3f(0.70f, 0.74f, 0.80f);
    glLineWidth(1.4f);

    glBegin(GL_LINES);
    for (int i = 0; i < RAIN_DROP_COUNT; i++) {
        float x = rainDropX[i];
        float y = rainDropY[i];

        glVertex2f(x, y);
        glVertex2f(x + 3.0f, y - 14.0f);
    }
    glEnd();

    glLineWidth(1.0f);
}

void drawGrassField(bool night) {
    const int GRASS_BLADE_COUNT = 2000;
    const int PATCH_COUNT = 30;

    static float grassBladeX[GRASS_BLADE_COUNT];
    static float grassBladeY[GRASS_BLADE_COUNT];
    static float grassBladeSway[GRASS_BLADE_COUNT];

    static float patchX[PATCH_COUNT];
    static float patchY[PATCH_COUNT];
    static float patchR[PATCH_COUNT];

    static bool initialized = false;

    if (!initialized) {
        for (int i = 0; i < GRASS_BLADE_COUNT; i++) {
            grassBladeX[i] = rand() % 1280;
            grassBladeY[i] = 110.0f + (rand() % 190);
            grassBladeSway[i] = (rand() % 100) / 100.0f;
        }
        for (int i = 0; i < PATCH_COUNT; i++) {
            patchX[i] = rand() % 1280;

            patchR[i] = 10.0f + (rand() % 20);

            int minY = 110 + (int)patchR[i];
            int maxY = 300 - (int)patchR[i];

            patchY[i] = minY + (rand() % (maxY - minY));
        }
        initialized = true;
    }

    float baseR = night ? 0.08f : 0.22f;
    float baseG = night ? 0.18f : 0.48f;
    float baseB = night ? 0.10f : 0.22f;

    glColor3f(baseR, baseG, baseB);
    rect(0, 110, 1280, 190);

    float sway = sin(grassSwayTimer * 0.02f) * 3.0f;

    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < GRASS_BLADE_COUNT; i++) {
        float x = grassBladeX[i];
        float y = grassBladeY[i];
        float height = 4.0f + grassBladeSway[i] * 8.0f;
        float shade = 0.6f + grassBladeSway[i] * 0.4f;

        if (night) {
            glColor3f(baseR * shade, baseG * shade * 1.2f, baseB * shade);
        } else {
            glColor3f(baseR * shade, baseG * shade * 1.3f, baseB * shade * 1.2f);
        }

        float swayX = sway * grassBladeSway[i];
        glVertex2f(x, y);
        glVertex2f(x + swayX, y + height);
    }
    glEnd();
    glLineWidth(1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (night) {
        glColor4f(0.05f, 0.12f, 0.06f, 0.3f);
    } else {
        glColor4f(0.15f, 0.35f, 0.15f, 0.25f);
    }

    for (int i = 0; i < PATCH_COUNT; i++) {
        circle(patchX[i], patchY[i], patchR[i], 16);
    }

    glDisable(GL_BLEND);
}

void tree(float bx, float by) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.15f);
    ellipse(bx, by + 2.0f, 30.0f, 6.0f, 24);
    glDisable(GL_BLEND);

    glColor3f(0.36f, 0.22f, 0.12f);
    rect(bx - 8.0f, by, 16.0f, 50.0f);

    glColor3f(0.28f, 0.16f, 0.08f);
    glBegin(GL_LINES);
        glVertex2f(bx - 4.0f, by + 15.0f); glVertex2f(bx + 4.0f, by + 18.0f);
        glVertex2f(bx - 4.0f, by + 25.0f); glVertex2f(bx + 4.0f, by + 22.0f);
        glVertex2f(bx - 4.0f, by + 35.0f); glVertex2f(bx + 4.0f, by + 38.0f);
    glEnd();

    glColor3f(0.12f, 0.38f, 0.14f);
    glBegin(GL_TRIANGLES);
        glVertex2f(bx - 42.0f, by + 25.0f);
        glVertex2f(bx + 42.0f, by + 25.0f);
        glVertex2f(bx, by + 90.0f);
    glEnd();

    glColor3f(0.10f, 0.32f, 0.12f);
    glBegin(GL_TRIANGLES);
        glVertex2f(bx - 38.0f, by + 35.0f);
        glVertex2f(bx + 38.0f, by + 35.0f);
        glVertex2f(bx - 3.0f, by + 85.0f);
    glEnd();

    glColor3f(0.16f, 0.46f, 0.18f);
    glBegin(GL_TRIANGLES);
        glVertex2f(bx - 34.0f, by + 60.0f);
        glVertex2f(bx + 34.0f, by + 60.0f);
        glVertex2f(bx, by + 120.0f);
    glEnd();

    glColor3f(0.24f, 0.64f, 0.26f);
    glBegin(GL_TRIANGLES);
        glVertex2f(bx - 26.0f, by + 90.0f);
        glVertex2f(bx + 26.0f, by + 90.0f);
        glVertex2f(bx, by + 148.0f);
    glEnd();

    glColor3f(0.40f, 0.75f, 0.40f);
    glBegin(GL_TRIANGLES);
        glVertex2f(bx - 12.0f, by + 110.0f);
        glVertex2f(bx + 12.0f, by + 110.0f);
        glVertex2f(bx, by + 140.0f);
    glEnd();
}

void road() {
    bool roadNightMode_current = false;
    float roadBottomY_current = 0.0f;
    float roadHeight_current = 150.0f;
    int roadLaneCount_current = 3;

    float roadR, roadG, roadB;
    float dashR, dashG, dashB;

    if (roadNightMode_current) {
        roadR = 0.10f; roadG = 0.10f; roadB = 0.12f;
        dashR = 0.92f; dashG = 0.92f; dashB = 0.64f;
    } else {
        roadR = 0.23f; roadG = 0.23f; roadB = 0.25f;
        dashR = 0.95f; dashG = 0.95f; dashB = 0.72f;
    }

    float roadWidth = 1280.0f;

    glColor3f(roadR, roadG, roadB);
    rect(0, roadBottomY_current, roadWidth, roadHeight_current);

    glColor3f(0.85f, 0.85f, 0.88f);
    rect(0, roadBottomY_current, roadWidth, 5);
    rect(0, (roadBottomY_current + roadHeight_current) - 5, roadWidth, 5);

    float laneHeight = roadHeight_current / roadLaneCount_current;

    glColor3f(dashR, dashG, dashB);

    for (int i = 1; i < roadLaneCount_current; i++) {
        float dividerY = roadBottomY_current + (laneHeight * i);

        for (float dashX = 20; dashX < roadWidth; dashX += 76) {
            rect(dashX, dividerY - 2.0f, 48, 4);
        }
    }
}

// VEHICLE
void wheel(float centerX, float centerY, float angle) {
    glPushMatrix();

    glTranslatef(centerX, centerY, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);

    glColor3f(0.05f, 0.05f, 0.06f); circle(0, 0, 16, 36);
    glColor3f(0.15f, 0.15f, 0.17f); circle(0, 0, 12, 30);
    glColor3f(0.74f, 0.74f, 0.78f); circle(0, 0,  7, 28);

    glColor3f(0.95f, 0.95f, 0.96f);
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex2f(-8.0f,  0.0f); glVertex2f( 8.0f,  0.0f);
    glVertex2f( 0.0f, -8.0f); glVertex2f( 0.0f,  8.0f);
    glVertex2f(-5.5f, -5.5f); glVertex2f( 5.5f,  5.5f);
    glVertex2f(-5.5f,  5.5f); glVertex2f( 5.5f, -5.5f);
    glEnd();
    glLineWidth(1);

    glPopMatrix();
}

void drawCar(float posX, float posY, float colorR, float colorG, float colorB, float wheelAngle, bool isHeadlightOn, bool isTaillightOn, bool isFacingRight) {
    glPushMatrix();

    glTranslatef(posX, posY, 0.0f);
    if (!isFacingRight) {
        glScalef(-1.0f, 1.0f, 1.0f);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.24f);
    ellipse(0, -10, 74, 11, 36);

    glColor3f(colorR, colorG, colorB);
    rect(-71, 0, 142, 32);

    glBegin(GL_POLYGON);
    glVertex2f(-38, 32);
    glVertex2f( 42, 32);
    glVertex2f( 20, 58);
    glVertex2f(-20, 58);
    glEnd();

    glColor3f(0.64f, 0.84f, 0.96f);
    glBegin(GL_POLYGON);
    glVertex2f(-30, 35);
    glVertex2f( 34, 35);
    glVertex2f( 16, 54);
    glVertex2f(-14, 54);
    glEnd();

    glColor3f(0.18f, 0.18f, 0.20f);
    rect( 64, 8, 10, 14);
    rect(-74, 8, 10, 14);

    wheel(-42, -2, wheelAngle);
    wheel( 42, -2, wheelAngle);

    if (isHeadlightOn) {
        glColor4f(1.0f, 0.96f, 0.42f, 0.30f);
        glBegin(GL_TRIANGLES);
        glVertex2f( 70,  20);
        glVertex2f(186,  58);
        glVertex2f(186, -10);
        glEnd();

        glColor3f(1.0f, 1.0f, 0.72f);
        rect(66, 14, 8, 8);
    }

    if (isTaillightOn) {
        glColor3f(0.95f, 0.08f, 0.08f);
        rect(-74, 16, 8, 8);
        rect(-74,  6, 8, 8);
    }

    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawHeadlightCone(float offsetX, float offsetY, bool isOn) {
    if (!isOn) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.94f, 0.46f, 0.26f);

    glBegin(GL_TRIANGLES);
    glVertex2f(offsetX +  70.0f, offsetY + 20.0f);
    glVertex2f(offsetX + 252.0f, offsetY + 58.0f);
    glVertex2f(offsetX + 252.0f, offsetY - 14.0f);
    glEnd();

    glDisable(GL_BLEND);
}

// ARCHITECTURE
void homeGround(bool night) {
    const float homeRoadBottomY = 0.0f;
    const float homeRoadHeight = 110.0f;

    float roadR = night ? 0.10f : 0.20f;
    float roadG = night ? 0.10f : 0.20f;
    float roadB = night ? 0.12f : 0.22f;

    float pathR = night ? 0.30f : 0.52f;
    float pathG = night ? 0.31f : 0.52f;
    float pathB = night ? 0.34f : 0.56f;

    glColor3f(roadR, roadG, roadB);
    rect(0, homeRoadBottomY, 1280, homeRoadHeight);

    glColor3f(0.66f, 0.66f, 0.70f);
    rect(0, (homeRoadBottomY + homeRoadHeight) - 3.0f, 1280.0f, 3.0f);

    drawGrassField(night);

    glColor3f(pathR, pathG, pathB);
    glBegin(GL_POLYGON);
        glVertex2f(216.0f, 110.0f);
        glVertex2f(376.0f, 110.0f);
        glVertex2f(386.0f, 300.0f);
        glVertex2f(206.0f, 300.0f);
    glEnd();
}

void houseWindow(float lx, float by, float w, float h, bool morning, bool lit) {
    glColor3f(0.20f, 0.18f, 0.16f);
    rect(lx, by, w, h);

    float flicker = 1.0f;
    if (lit && !morning) {
        int windowIdx = (int)(lx + by) % 10;
        flicker = windowFlickerAmount[windowIdx];
    }

    if (morning) {
        glColor3f(0.60f, 0.74f, 0.84f);
    } else if (lit) {
        glColor3f(0.98f * flicker, 0.88f * flicker, 0.54f * flicker);
    } else {
        glColor3f(0.10f, 0.12f, 0.16f);
    }

    rect(lx + 3.0f, by + 3.0f, w - 6.0f, h - 6.0f);

    if (morning) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 0.15f);
        glBegin(GL_TRIANGLES);
        glVertex2f(lx + 6, by + h - 6);
        glVertex2f(lx + w - 6, by + h - 6);
        glVertex2f(lx + 6, by + h - 30);
        glEnd();
        glDisable(GL_BLEND);
    }

    glColor3f(0.18f, 0.16f, 0.14f);
    rect(lx + (w / 2.0f) - 1.0f, by + 3.0f, 2.0f, h - 6.0f);
    rect(lx + 3.0f, by + (h / 2.0f) - 1.0f, w - 6.0f, 2.0f);
}

void house(bool isLit) {
    const float homeHouseLeftX = 450.0f;
    const float homeHouseBottomY = 300.0f;
    const float homeGarageLeftX = 200.0f;
    const float homeGarageBottomY = 300.0f;
    const float homeGarageDoorLeftX = 206.0f;
    const float homeGarageDoorWidth = 180.0f;

    const float hBaseX = homeHouseLeftX + 14.0f;
    const float hBaseY = homeHouseBottomY;
    const float secW = 300.0f;
    const float secH = 270.0f;
    const float gStartX = homeGarageLeftX;
    const float gBaseY = homeGarageBottomY;

    bool morning = (currentScene == 1);

    glColor3f(0.12f, 0.13f, 0.15f);
    rect(hBaseX, hBaseY, secW, 126.0f);

    glColor3f(0.18f, 0.20f, 0.22f);
    rect(hBaseX, hBaseY + 126.0f, secW, secH - 126.0f);

    glColor3f(0.14f, 0.15f, 0.17f);
    rect(gStartX, gBaseY, 240.0f, 170.0f);

    glColor3f(0.04f, 0.04f, 0.05f);
    glBegin(GL_TRIANGLES);
    glVertex2f(hBaseX - 24.0f, hBaseY + secH + 8.0f);
    glVertex2f(hBaseX + secW + 24.0f, hBaseY + secH + 8.0f);
    glVertex2f(hBaseX + 150.0f, hBaseY + secH + 100.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(gStartX - 24.0f, gBaseY + 176.0f);
    glVertex2f(gStartX + 264.0f, gBaseY + 176.0f);
    glVertex2f(gStartX + 120.0f, gBaseY + 230.0f);
    glEnd();

    glColor3f(0.40f, 0.18f, 0.10f);
    rect(hBaseX - 6.0f, hBaseY + 122.0f, secW + 12.0f, 6.0f);
    rect(gStartX - 6.0f, gBaseY + 168.0f, 252.0f, 6.0f);

    glColor3f(0.35f, 0.16f, 0.08f);
    rect(hBaseX + 94.0f, hBaseY, 56.0f, 120.0f);

    glColor3f(0.75f, 0.75f, 0.80f);
    rect(hBaseX + 140.0f, hBaseY + 50.0f, 4.0f, 32.0f);

    houseWindow(hBaseX + 16.0f, hBaseY + 30.0f, 28.0f, 82.0f, morning, isLit);
    houseWindow(hBaseX + 50.0f, hBaseY + 30.0f, 28.0f, 82.0f, morning, isLit);

    houseWindow(hBaseX + 170.0f, hBaseY + 58.0f, 110.0f, 54.0f, morning, false);

    houseWindow(hBaseX + 16.0f, hBaseY + 150.0f, 80.0f, 80.0f, morning, isLit);
    houseWindow(hBaseX + 104.0f, hBaseY + 150.0f, 80.0f, 80.0f, morning, false);
    houseWindow(hBaseX + 192.0f, hBaseY + 150.0f, 80.0f, 80.0f, morning, isLit);

    houseWindow(gStartX + 16.0f, gBaseY + 110.0f, 60.0f, 24.0f, morning, isLit);
    houseWindow(gStartX + 164.0f, gBaseY + 110.0f, 60.0f, 24.0f, morning, isLit);

    glColor3f(0.02f, 0.02f, 0.02f);
    rect(homeGarageDoorLeftX, homeGarageDoorBottomY, homeGarageDoorWidth, homeGarageDoorHeight);

    glColor3f(0.30f, 0.15f, 0.08f);
    rect(hBaseX + 230.0f, hBaseY + secH + 30.0f, 30.0f, 80.0f);

    glColor3f(0.20f, 0.10f, 0.05f);
    rect(hBaseX + 226.0f, hBaseY + secH + 106.0f, 38.0f, 8.0f);
}

void drawGarageDoor() {
    const float homeGarageDoorLeftX = 206.0f;
    const float homeGarageDoorWidth = 180.0f;

    float currentH = homeGarageDoorHeight - garageDoorOpenAmount;
    if (currentH <= 0.5f) return;

    float doorY = homeGarageDoorBottomY + garageDoorOpenAmount;

    glColor3f(0.06f, 0.07f, 0.08f);
    rect(homeGarageDoorLeftX, doorY, homeGarageDoorWidth, currentH);

    glColor3f(0.28f, 0.30f, 0.35f);
    for (int i = 1; i < 8; i++) {
        float panelY = doorY + (currentH / 8.0f) * i;
        if (panelY < homeGarageDoorBottomY + homeGarageDoorHeight) {
            rect(homeGarageDoorLeftX + 6.0f, panelY, homeGarageDoorWidth - 12.0f, 3.0f);
        }
    }

    glColor3f(0.55f, 0.55f, 0.58f);
    float handleY = doorY + currentH * 0.35f;
    rect(homeGarageDoorLeftX + homeGarageDoorWidth - 30.0f, handleY - 2.0f, 20.0f, 4.0f);
}

// SCENES
void scene1() {
    if (!isRainEnabled) {
        gradSky(0.68f, 0.88f, 0.99f, 0.88f, 0.96f, 1.00f);
        sun(1020.0f + sunHorizontalOffset * 0.20f, 620.0f);

        // FIX: Added 1.0f for the alpha parameter here
        cloud(210.0f + cloudOffsetX_layerA * 0.70f, 620.0f, 1.0f);
        cloud(520.0f + cloudOffsetX_layerB * 0.60f, 650.0f, 1.0f);
    } else {
        gradSky(0.24f, 0.26f, 0.29f, 0.36f, 0.38f, 0.41f);
        cloud(220.0f + cloudOffsetX_layerA * 0.5f, 632.0f, 0.8f);
        cloud(620.0f + cloudOffsetX_layerB * 0.5f, 654.0f, 0.75f);
        cloud(850.0f + cloudOffsetX_layerC * 0.5f, 600.0f, 0.7f);
    }

    homeGround(false);

    tree(60.0f, 120.0f);
    tree(1120.0f, 125.0f);
    tree(1050.0f, 118.0f);
    tree(980.0f, 128.0f);

    house(false);

    drawCar(scene1_carPosX, scene1_carPosY, 0.84f, 0.20f, 0.18f, wheelRotationAngle, false, false, true);
    drawGarageDoor();
}

void scene9() {
    gradSky(0.05f, 0.08f, 0.18f, 0.11f, 0.16f, 0.28f);
    stars();
    moon(1050.0f, 620.0f);

    homeGround(true);
    drawFireflies();

    tree(60.0f, 120.0f);
    tree(1120.0f, 125.0f);
    tree(1050.0f, 118.0f);
    tree(980.0f, 128.0f);

    house(isHouseLightOn);

    bool headOn = (carState_scene9 != 2);
    drawCar(scene9_carPosX, scene9_carPosY, 0.84f, 0.20f, 0.18f, wheelRotationAngle, headOn, true, true);
    drawHeadlightCone(scene9_carPosX, scene9_carPosY, headOn);

    drawGarageDoor();
}


// ANIMATION
void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);

    initRain();

    cloudDriftSpeedA = 0.3f + (rand() % 20) / 100.0f;
    cloudDriftSpeedB = 0.25f + (rand() % 20) / 100.0f;
    cloudDriftSpeedC = 0.35f + (rand() % 20) / 100.0f;

    for (int i = 0; i < 10; i++) {
        windowFlickerAmount[i] = 0.85f + (rand() % 30) / 100.0f;
    }
}

void resetScene(int idx) {
    if (idx == 1) {
        const float scene1GarageStartX = 296.0f;
        const float scene1GarageStartY = 330.0f;

        scene1_carPosX = scene1GarageStartX;
        scene1_carPosY = scene1GarageStartY;
        scene1_carAngle = 0.0f;
        wheelRotationAngle = 0;
        garageDoorOpenAmount = 0.0f;
        carState_scene1 = 0;
        scene1HasCarExitedScreen = false;
        sunGlowPulse = 0.0f;
        sunGlowDirection = 1.0f;
    }
    if (idx == 9) {
        const float scene9GarageStartX = -100.0f;
        const float scene9GarageStartY = 50.0f;

        scene9_carPosX = scene9GarageStartX;
        scene9_carPosY = scene9GarageStartY;
        scene9_carAngle = 0.0f;
        wheelRotationAngle = 0;
        carState_scene9 = 0;
        scene9ParkingCompleted = false;
        isHouseLightOn = false;

        garageDoorOpenAmount = 0.0f;
    }
}

void nextScene() {
    currentScene++;
    if (currentScene == 2) currentScene = 9;
    if (currentScene > LAST_SCENE_INDEX) currentScene = FIRST_SCENE_INDEX;
    sceneFrameCounter = 0;
    resetScene(currentScene);
}

void anim1() {
    const float scene1RoadTravelY = 50.0f;
    const float scene1ExitCheckX = 1350.0f;

    if (carState_scene1 == 0) {
        carState_scene1 = 1;
    }
    else if (carState_scene1 == 1) {
        garageDoorOpenAmount += 1.5f * animationSpeed;
        if (garageDoorOpenAmount >= homeGarageDoorHeight - 5.0f) {
            garageDoorOpenAmount = homeGarageDoorHeight - 5.0f;
            carState_scene1 = 2;
        }
    }
    else if (carState_scene1 == 2) {
        float speedY = 2.0f * animationSpeed;
        scene1_carPosY -= speedY;
        wheelRotationAngle -= speedY * 2.0f;
        if (scene1_carPosY <= scene1RoadTravelY) {
            scene1_carPosY = scene1RoadTravelY;
            carState_scene1 = 3;
        }
    }
    else if (carState_scene1 == 3) {
        float speed = 3.0f * animationSpeed;
        scene1_carPosX += speed;
        wheelRotationAngle -= speed * 2.0f;

        if (garageDoorOpenAmount > 0) {
            garageDoorOpenAmount -= 1.0f * animationSpeed;
            if (garageDoorOpenAmount < 0) garageDoorOpenAmount = 0;
        }

        if (scene1_carPosX > scene1ExitCheckX) {
            scene1HasCarExitedScreen = true;
            nextScene();
        }
    }
}

void anim9() {
    if (carState_scene9 == 0) {
        float step = 3.0f * animationSpeed;
        scene9_carPosX += step;
        wheelRotationAngle -= step * 2.0f;

        if (scene9_carPosX >= 296.0f) {
            scene9_carPosX = 296.0f;
            carState_scene9 = 1;
        }
    }
    else if (carState_scene9 == 1) {
        garageDoorOpenAmount += 1.5f * animationSpeed;
        if (garageDoorOpenAmount >= homeGarageDoorHeight - 5.0f) {
            garageDoorOpenAmount = homeGarageDoorHeight - 5.0f;
            carState_scene9 = 2;
        }
    }
    else if (carState_scene9 == 2) {
        float up = 2.0f * animationSpeed;
        scene9_carPosY += up;
        wheelRotationAngle -= up * 1.8f;

        if (scene9_carPosY >= 330.0f) {
            scene9_carPosY = 330.0f;
            carState_scene9 = 3;
            scene9ParkingCompleted = true;
            isHouseLightOn = true;
        }
    }
    else if (carState_scene9 == 3) {
        scene9ParkedFrameCounter++;
        if (garageDoorOpenAmount > 0) {
            garageDoorOpenAmount -= 1.0f * animationSpeed;

            if (garageDoorOpenAmount <= 0) {
                garageDoorOpenAmount = 0;
                isPaused = true;
                currentScene = 1;
                resetScene(1);
            }
        }
    }
}

// DISPLAY
void display() {
    if (currentScene == 1) {
        glClearColor(0.62f, 0.84f, 0.97f, 1.0f);
    } else {
        glClearColor(0.05f, 0.07f, 0.15f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (currentScene == 1) {
        scene1();
    } else if (currentScene == 9) {
        scene9();
    }

    if (isRainEnabled) {
        drawRain();
    }

    glutSwapBuffers();
}

void update(int val) {
    if (!isPaused) {
        sunGlowPulse += 0.01f * sunGlowDirection * animationSpeed;
        if (sunGlowPulse >= 1.0f) { sunGlowPulse = 1.0f; sunGlowDirection = -1.0f; }
        if (sunGlowPulse <= 0.0f) { sunGlowPulse = 0.0f; sunGlowDirection = 1.0f; }

        grassSwayTimer += 1.0f * animationSpeed;

        cloudOffsetX_layerA += cloudDriftSpeedA * animationSpeed;
        cloudOffsetX_layerB += cloudDriftSpeedB * animationSpeed;
        cloudOffsetX_layerC += cloudDriftSpeedC * animationSpeed;
        if (cloudOffsetX_layerA > 1400) cloudOffsetX_layerA = -300;
        if (cloudOffsetX_layerB > 1400) cloudOffsetX_layerB = -300;
        if (cloudOffsetX_layerC > 1400) cloudOffsetX_layerC = -300;

        windowFlickerTimer += 1.0f * animationSpeed;
        for (int i = 0; i < 10; i++) {
            windowFlickerAmount[i] = 0.85f + 0.15f * sin(windowFlickerTimer * 0.03f + i * 1.2f);
        }

        if (currentScene == 1) anim1();
        else if (currentScene == 9) anim9();

        if (isRainEnabled) {
            for (int i = 0; i < RAIN_DROP_COUNT; i++) {
                rainDropY[i] -= 6.0f * animationSpeed;

                rainDropX[i] -= 1.0f * animationSpeed;

                if (rainDropY[i] < 0) {
                    rainDropY[i] = WINDOW_HEIGHT;
                    rainDropX[i] = rand() % WINDOW_WIDTH;
                }
            }
        }
    }
    glutPostRedisplay();
    glutTimerFunc(TIMER_INTERVAL_MS, update, 0);
}

// INPUT
void keyboard(unsigned char key, int x, int y) {
    if (key == 27) exit(0);
    if (key == 'r' || key == 'R') isRainEnabled = true;
    if (key == 'v' || key == 'V') isRainEnabled = false;
    if (key == ' ') isPaused = !isPaused;
    if (key == 'n' || key == 'N') {
        currentScene = 1;
        resetScene(1);
        resetScene(9);
        isPaused = false;
    }
}

void specialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT) {
        if (currentScene == 9) currentScene = 1;
        else if (currentScene == 1) currentScene = 9;
        resetScene(currentScene);
    }
    if (key == GLUT_KEY_RIGHT) {
        nextScene();
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Modern Urban Area - Clean Build");

    init();
    resetScene(currentScene);

    glutDisplayFunc(display);
    glutTimerFunc(TIMER_INTERVAL_MS, update, 0);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}
