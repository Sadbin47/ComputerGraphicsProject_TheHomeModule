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

struct Bird {
    float x, y;
    float vx, vy;
    float wingAngle;
    float wingSpeed;
    bool isFlying;
};

const int BIRD_COUNT = 5;
Bird birds[BIRD_COUNT];

float scene1_carPosX = 400.0f;
float scene1_carPosY = 50.0f;
int carState_scene1 = 0;
bool scene1HasCarExitedScreen = false;
float scene1_carAngle = 0.0f;
float scene1_planeX = 980.0f;
float scene1_planeY = 780.0f;
float scene1_planeTilt = 200.0f;

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

// Small helper to keep a value inside a range.
float clamp(float v, float mn, float mx) {
    if (v < mn) {
        return mn;
    }

    if (v > mx) {
        return mx;
    }

    return v;
}

// Basic shapes used everywhere in the scene.
void rect(float x, float y, float w, float h) {
    // One rectangle = 4 corner points.
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void circle(float cx, float cy, float r, int seg) {
    // Too few segments makes the circle look like a polygon.
    if (seg < 12) seg = 12;
    glBegin(GL_POLYGON);
    for (int i = 0; i < seg; i++) {
        // Angle goes around 360 degree in small steps.
        float a = 2.0f * PI_VALUE * i / seg;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }
    glEnd();
}

void ellipse(float cx, float cy, float rx, float ry, int seg) {
    if (seg < 12) seg = 12;
    glBegin(GL_POLYGON);
    for (int i = 0; i < seg; i++) {
        // Same circle formula, but X and Y radius are different.
        float a = 2.0f * PI_VALUE * i / seg;
        glVertex2f(cx + cos(a) * rx, cy + sin(a) * ry);
    }
    glEnd();
}

// Vertical sky gradient from bottom color to top color.
void gradSky(float br, float bg, float bb, float tr, float tg, float tb) {
    glBegin(GL_QUADS);
    // Bottom two points get one color.
    glColor3f(br, bg, bb); glVertex2f(0, 0);
    glColor3f(br, bg, bb); glVertex2f(1280, 0);
    // Top two points get another color, OpenGL blends between them.
    glColor3f(tr, tg, tb); glVertex2f(1280, 720);
    glColor3f(tr, tg, tb); glVertex2f(0, 720);
    glEnd();
}

void cloud(float cx, float cy, float alpha) {
    // Blend is needed here because cloud color uses alpha.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (isRainEnabled) {
        // Darker cloud color for rainy weather.
        glColor4f(0.45f, 0.49f, 0.53f, alpha);
    } else {
        glColor4f(0.96f, 0.96f, 0.98f, alpha);
    }

    // Many circles overlap to form one soft cloud shape.
    circle(cx, cy, 32, 32);
    circle(cx - 35, cy - 6, 26, 28);
    circle(cx + 35, cy - 4, 28, 28);
    circle(cx + 5, cy + 20, 24, 28);
    circle(cx - 20, cy + 12, 20, 24);
    circle(cx + 22, cy + 14, 22, 24);

    if (!isRainEnabled) {
        // Small white circles add highlight in clear weather.
        glColor4f(1.0f, 1.0f, 1.0f, alpha * 0.6f);
        circle(cx - 5, cy + 16, 16, 20);
        circle(cx + 8, cy + 22, 12, 18);
    }

    glDisable(GL_BLEND);
}

void sun(float cx, float cy) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Pulse value changes the size and brightness of the glow.
    float glow = 0.5f + sunGlowPulse * 0.5f;

    // Bigger transparent circles make the outer glow.
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
        // Every line starts from the sun and goes outward like a ray.
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
// Moon is made by drawing a dark circle over a bright one.
void moon(float cx, float cy) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Light circles first.
    glColor4f(0.85f, 0.88f, 0.95f, 0.15f);
    circle(cx, cy, 50, 48);

    glColor4f(0.88f, 0.90f, 0.94f, 0.3f);
    circle(cx, cy, 40, 48);

    glColor3f(0.90f, 0.92f, 0.96f);
    circle(cx, cy, 30, 48);

    // Dark circle cuts the shape and creates the crescent look.
    glColor3f(0.05f, 0.07f, 0.16f);
    circle(cx + 10, cy + 4, 26, 48);

    glColor4f(0.7f, 0.72f, 0.78f, 0.2f);
    circle(cx - 8, cy + 6, 8, 24);
    circle(cx + 2, cy - 10, 6, 20);

    glDisable(GL_BLEND);
}

// Star positions are random once, then only brightness changes.
void stars() {
    const int STAR_COUNT = 150;
    static float starX[STAR_COUNT];
    static float starY[STAR_COUNT];
    static float starSize[STAR_COUNT];
    static bool initialized = false;

    if (!initialized) {
        for (int i = 0; i < STAR_COUNT; i++) {
            // Random once so star positions stay fixed every frame.
            starX[i] = rand() % WINDOW_WIDTH;
            starY[i] = 350 + (rand() % (WINDOW_HEIGHT - 350));
            starSize[i] = 1.0f + (rand() % 20) / 10.0f;
        }
        initialized = true;
    }

    glEnable(GL_BLEND);
    // Additive blending makes stars feel brighter.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (int i = 0; i < STAR_COUNT; i++) {
        // `sin` gives a smooth twinkle instead of sudden blinking.
        float twinkle = 0.5f + 0.5f * sin(scene9ParkedFrameCounter * 0.05f + i);

        // Each star can have a slightly different size.
        glPointSize(starSize[i]);
        glColor4f(1.0f, 1.0f, 0.9f, twinkle);
        glBegin(GL_POINTS);
            glVertex2f(starX[i], starY[i]);
        glEnd();

        if (i % 10 == 0) {
            // Some stars get a cross sparkle so the sky looks richer.
            glColor4f(1.0f, 1.0f, 1.0f, 0.4f * twinkle);
            glBegin(GL_LINES);
                glVertex2f(starX[i] - 6, starY[i]); glVertex2f(starX[i] + 6, starY[i]);
                glVertex2f(starX[i], starY[i] - 6); glVertex2f(starX[i], starY[i] + 6);
            glEnd();
        }
    }

    glDisable(GL_BLEND);
}

// Fireflies use their own phase so all of them do not move together.
void drawFireflies() {
    struct Firefly {
        float x, y, phase, brightness;

        Firefly() {
            // Keep them inside the grass/yard area.
            x = 100.0f + (rand() % 1080);
            y = 120.0f + (rand() % 500);
            // Each firefly gets a different timing.
            phase = (rand() % 100) / 100.0f * 2.0f * PI_VALUE;
            brightness = 0.3f + (rand() % 70) / 100.0f;
        }
    };

    static Firefly fireflies[30];

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (int i = 0; i < 30; i++) {
        Firefly& f = fireflies[i];

        // Same point is drawn in 3 sizes to make a soft glow.
        float glow = f.brightness * (0.5f + 0.5f * sin(grassSwayTimer * 0.04f + f.phase));

        // Small sin/cos movement keeps them floating around gently.
        f.x += sin(grassSwayTimer * 0.01f + f.phase) * 0.3f;
        f.y += cos(grassSwayTimer * 0.015f + f.phase) * 0.2f;

        // Wrap around so they never disappear for good.
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
// Random starting position for each rain line.
void initRain() {
    for (int i = 0; i < RAIN_DROP_COUNT; i++) {
        // Start every drop at a random place.
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

        // One short slanted line looks like one rain drop.
        glVertex2f(x, y);
        glVertex2f(x + 3.0f, y - 14.0f);
        // Extra copies make rain look continuous when drops wrap around.
        glVertex2f(x - WINDOW_WIDTH, y);
        glVertex2f(x - WINDOW_WIDTH + 3.0f, y - 14.0f);
        glVertex2f(x + WINDOW_WIDTH, y);
        glVertex2f(x + WINDOW_WIDTH + 3.0f, y - 14.0f);
    }
    glEnd();

    glLineWidth(1.0f);
}

// Grass field uses many tiny lines plus soft round patches.
void drawGrassField(bool night) {
    const int GRASS_BLADE_COUNT = 2000;
    const int PATCH_COUNT = 30;

    // Saved once so every frame keeps the same field layout.
    static float grassBladeX[GRASS_BLADE_COUNT];
    static float grassBladeY[GRASS_BLADE_COUNT];
    static float grassBladeSway[GRASS_BLADE_COUNT];

    // Round patches make the ground feel less flat.
    static float patchX[PATCH_COUNT];
    static float patchY[PATCH_COUNT];
    static float patchR[PATCH_COUNT];

    static bool initialized = false;

    if (!initialized) {
        for (int i = 0; i < GRASS_BLADE_COUNT; i++) {
            // Spread blades all across the field area.
            grassBladeX[i] = rand() % 1280;
            grassBladeY[i] = 110.0f + (rand() % 190);
            // One random value controls height, shade, and sway strength.
            grassBladeSway[i] = (rand() % 100) / 100.0f;
        }
        for (int i = 0; i < PATCH_COUNT; i++) {
            patchX[i] = rand() % 1280;
            // Patch sizes are slightly different on purpose.
            patchR[i] = 10.0f + (rand() % 20);

            // Keep the whole circle inside the grass area.
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

    // `sin` gives a smooth left-right wind motion.
    float sway = sin(grassSwayTimer * 0.02f) * 3.0f;

    glLineWidth(1.0f);
    glBegin(GL_LINES);
    // Each blade is just one slanted line.
    for (int i = 0; i < GRASS_BLADE_COUNT; i++) {
        float x = grassBladeX[i];
        float y = grassBladeY[i];
        // Short and tall blades come from the same random sway value.
        float height = 4.0f + grassBladeSway[i] * 8.0f;
        // Shade change stops the field from looking flat.
        float shade = 0.6f + grassBladeSway[i] * 0.4f;

        if (night) {
            glColor3f(
                baseR * shade,
                baseG * shade * 1.2f,
                baseB * shade);
        } else {
            glColor3f(
                baseR * shade,
                baseG * shade * 1.3f,
                baseB * shade * 1.2f
            );
        }
        // Only the top point moves, so the blade looks bent by wind.
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
        // Soft circles break the uniform look of plain grass.
        circle(patchX[i], patchY[i], patchR[i], 16);
    }

    glDisable(GL_BLEND);
}

// Static tree with layered triangles for leaves.
void tree(float bx, float by) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Shadow under the tree gives a bit more depth.
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

    // Leaf layers get smaller toward the top like a stylized pine tree.
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

void treeAnimated(float bx, float by, float sway) {
    // Same tree idea, but the leaf points shift a bit to make wind.
    glColor3f(0.36f, 0.22f, 0.12f);
    rect(bx - 8.0f, by, 16.0f, 50.0f);

    glColor3f(0.12f, 0.38f, 0.14f);
    glBegin(GL_TRIANGLES);
    glVertex2f(bx - 40.0f + sway * 0.6f, by + 30.0f);
    glVertex2f(bx + 40.0f + sway * 0.6f, by + 30.0f);
    glVertex2f(bx + sway * 1.0f, by + 90.0f);
    glEnd();

    glColor3f(0.16f, 0.46f, 0.18f);
    glBegin(GL_TRIANGLES);
    glVertex2f(bx - 32.0f + sway * 0.8f, by + 65.0f);
    glVertex2f(bx + 32.0f + sway * 0.8f, by + 65.0f);
    glVertex2f(bx + sway * 1.2f, by + 120.0f);
    glEnd();

    glColor3f(0.24f, 0.64f, 0.26f);
    glBegin(GL_TRIANGLES);
    glVertex2f(bx - 24.0f + sway, by + 95.0f);
    glVertex2f(bx + 24.0f + sway, by + 95.0f);
    glVertex2f(bx + sway * 1.4f, by + 145.0f);
    glEnd();
}

// Generic road with lane dividers.
// Draws a 3-lane road at the bottom of the screen.
void road() {
    // Basic road settings: starts from bottom, height 150, divided into 3 lanes.
    bool roadNightMode_current = false;
    float roadBottomY_current = 0.0f;
    float roadHeight_current = 150.0f;
    int roadLaneCount_current = 3;

    float roadR, roadG, roadB;
    float dashR, dashG, dashB;

    // Select road and lane-dash colors based on day/night mode.
    if (roadNightMode_current) {
        roadR = 0.10f; roadG = 0.10f; roadB = 0.12f;
        dashR = 0.92f; dashG = 0.92f; dashB = 0.64f;
    } else {
        roadR = 0.23f; roadG = 0.23f; roadB = 0.25f;
        dashR = 0.95f; dashG = 0.95f; dashB = 0.72f;
    }

    float roadWidth = 1280.0f;

    // Draw the main road rectangle.
    glColor3f(roadR, roadG, roadB);
    rect(0, roadBottomY_current, roadWidth, roadHeight_current);

    // Draw top and bottom road border lines.
    glColor3f(0.85f, 0.85f, 0.88f);
    rect(0, roadBottomY_current, roadWidth, 5);
    rect(0, roadBottomY_current + roadHeight_current - 5, roadWidth, 5);

    // Each lane height = total road height / number of lanes.
    float laneHeight = roadHeight_current / roadLaneCount_current;

    // Draw dashed divider lines between lanes.
    glColor3f(dashR, dashG, dashB);

    for (int i = 1; i < roadLaneCount_current; i++) {
        // Y position of each lane divider.
        float dividerY = roadBottomY_current + (laneHeight * i);

        // Repeated small rectangles create dashed lane marks.
        for (float dashX = 20; dashX < roadWidth; dashX += 76) {
            rect(dashX, dividerY - 2.0f, 48, 4);
        }
    }
}

// Wheel rotates around its own center.
void wheel(float centerX, float centerY, float angle) {
    glPushMatrix();

    // Move wheel to its position, then rotate around that center.
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

    // Whole car is drawn relative to this point.
    glTranslatef(posX, posY, 0.0f);
    // Flip the same model
    if (!isFacingRight) {
        glScalef(-1.0f, 1.0f, 1.0f);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Flat ellipse under the car acts like a shadow.
    glColor4f(0.0f, 0.0f, 0.0f, 0.24f);
    ellipse(0, -10, 74, 11, 36);

    // Lower body.
    glColor3f(colorR, colorG, colorB);
    rect(-71, 0, 142, 32);

    // Upper body / roof.
    glBegin(GL_POLYGON);
    glVertex2f(-38, 32);
    glVertex2f( 42, 32);
    glVertex2f( 20, 58);
    glVertex2f(-20, 58);
    glEnd();

    // Window glass.
    glColor3f(0.64f, 0.84f, 0.96f);
    glBegin(GL_POLYGON);
    glVertex2f(-30, 35);
    glVertex2f( 34, 35);
    glVertex2f( 16, 54);
    glVertex2f(-14, 54);
    glEnd();

    // Front and back bumper blocks.
    glColor3f(0.18f, 0.18f, 0.20f);
    rect( 64, 8, 10, 14);
    rect(-74, 8, 10, 14);

    // Wheels use the shared rotation angle so motion feels connected.
    wheel(-42, -2, wheelAngle);
    wheel( 42, -2, wheelAngle);

    if (isHeadlightOn) {
        // Headlight beam is attached to the front of the car.
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
        // Small red blocks are enough for taillights.
        glColor3f(0.95f, 0.08f, 0.08f);
        rect(-74, 16, 8, 8);
        rect(-74,  6, 8, 8);
    }

    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawHeadlightCone(float offsetX, float offsetY, bool isOn) {
    if (!isOn) return;

    // A transparent triangle is enough to make a light beam.
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

// Ground joins road, grass, and the front path.
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

    // Thin line separates road from grass area.
    glColor3f(0.66f, 0.66f, 0.70f);
    rect(0, (homeRoadBottomY + homeRoadHeight) - 3.0f, 1280.0f, 3.0f);

    // Simple dashed center line.
    if (night) glColor3f(0.82f, 0.78f, 0.42f);
    else glColor3f(0.95f, 0.94f, 0.72f);
    for (float x = 20.0f; x < 1280.0f; x += 76.0f) {
        rect(x, 54.0f, 42.0f, 4.0f);
    }

    // Draw grass after road so the yard sits behind it.
    drawGrassField(night);

    glColor3f(pathR, pathG, pathB);
    // Four points make the front path slightly wider at the top.
    glBegin(GL_POLYGON);
        glVertex2f(216.0f, 110.0f);
        glVertex2f(376.0f, 110.0f);
        glVertex2f(386.0f, 300.0f);
        glVertex2f(206.0f, 300.0f);
    glEnd();
}

// Window color changes for morning and for night light flicker.
void houseWindow(float lx, float by, float w, float h, bool morning, bool lit) {
    // Outer frame.
    glColor3f(0.20f, 0.18f, 0.16f);
    rect(lx, by, w, h);

    float flicker = 1.0f;
    if (lit && !morning) {
        // This picks one saved flicker value for each window.
        int windowIdx = (int)(lx + by) % 10;
        flicker = windowFlickerAmount[windowIdx];
    }

    if (morning) {
        glColor3f(0.60f, 0.74f, 0.84f);
    } else if (lit) {
        // Flicker changes only brightness, not window position.
        glColor3f(0.98f * flicker, 0.88f * flicker, 0.54f * flicker);
    } else {
        glColor3f(0.10f, 0.12f, 0.16f);
    }

    // Inner glass area.
    rect(lx + 3.0f, by + 3.0f, w - 6.0f, h - 6.0f);

    if (morning) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Small bright triangle looks like glass reflection.
        glColor4f(1.0f, 1.0f, 1.0f, 0.15f);
        glBegin(GL_TRIANGLES);
        glVertex2f(lx + 6, by + h - 6);
        glVertex2f(lx + w - 6, by + h - 6);
        glVertex2f(lx + 6, by + h - 30);
        glEnd();
        glDisable(GL_BLEND);
    }

    // Cross bars divide the window into four parts.
    glColor3f(0.18f, 0.16f, 0.14f);
    rect(lx + (w / 2.0f) - 1.0f, by + 3.0f, 2.0f, h - 6.0f);
    rect(lx + 3.0f, by + (h / 2.0f) - 1.0f, w - 6.0f, 2.0f);
}

// Main house and garage body.
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

    // Main house block.
    glColor3f(0.12f, 0.13f, 0.15f);
    rect(hBaseX, hBaseY, secW, 126.0f);

    glColor3f(0.18f, 0.20f, 0.22f);
    rect(hBaseX, hBaseY + 126.0f, secW, secH - 126.0f);

    // Garage block.
    glColor3f(0.14f, 0.15f, 0.17f);
    rect(gStartX, gBaseY, 240.0f, 170.0f);

    // Roof triangles.
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

    // Thin strips help separate walls and roof.
    glColor3f(0.40f, 0.18f, 0.10f);
    rect(hBaseX - 6.0f, hBaseY + 122.0f, secW + 12.0f, 6.0f);
    rect(gStartX - 6.0f, gBaseY + 168.0f, 252.0f, 6.0f);

    glColor3f(0.35f, 0.16f, 0.08f);
    rect(hBaseX + 94.0f, hBaseY, 56.0f, 120.0f);

    glColor3f(0.75f, 0.75f, 0.80f);
    rect(hBaseX + 140.0f, hBaseY + 50.0f, 4.0f, 32.0f);

    // Reusing the same window function keeps the style consistent.
    houseWindow(hBaseX + 16.0f, hBaseY + 30.0f, 28.0f, 82.0f, morning, isLit);
    houseWindow(hBaseX + 50.0f, hBaseY + 30.0f, 28.0f, 82.0f, morning, isLit);

    houseWindow(hBaseX + 170.0f, hBaseY + 58.0f, 110.0f, 54.0f, morning, false);

    houseWindow(hBaseX + 16.0f, hBaseY + 150.0f, 80.0f, 80.0f, morning, isLit);
    houseWindow(hBaseX + 104.0f, hBaseY + 150.0f, 80.0f, 80.0f, morning, false);
    houseWindow(hBaseX + 192.0f, hBaseY + 150.0f, 80.0f, 80.0f, morning, isLit);

    houseWindow(gStartX + 16.0f, gBaseY + 110.0f, 60.0f, 24.0f, morning, isLit);
    houseWindow(gStartX + 164.0f, gBaseY + 110.0f, 60.0f, 24.0f, morning, isLit);

    // Dark opening behind the moving door.
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

    //garageDoorOpenAmount is 0 initially, so the full garage door is visible
    // currentH = garage door open.
    float currentH = homeGarageDoorHeight - garageDoorOpenAmount;

    // If the door is almost fully open, no need to draw it.
    if (currentH <= 0.5f) return;

    // So its Y position increases with garageDoorOpenAmount.
    float doorY = homeGarageDoorBottomY + garageDoorOpenAmount;

    // Draw the main visible part of the garage door.
    glColor3f(0.06f, 0.07f, 0.08f);
    rect(homeGarageDoorLeftX, doorY, homeGarageDoorWidth, currentH);

    // Draw horizontal panel lines on the shutter.
    glColor3f(0.28f, 0.30f, 0.35f);
    for (int i = 1; i < 8; i++) {
        float panelY = doorY + (currentH / 8.0f) * i;

        // Keep panel lines inside the original garage door area.
        if (panelY < homeGarageDoorBottomY + homeGarageDoorHeight) {
            rect(homeGarageDoorLeftX + 6.0f, panelY, homeGarageDoorWidth - 12.0f, 3.0f);
        }
    }

    // Draw the small handle of the garage door.
    glColor3f(0.55f, 0.55f, 0.58f);
    float handleY = doorY + currentH * 0.35f;
    rect(homeGarageDoorLeftX + homeGarageDoorWidth - 30.0f, handleY - 2.0f, 20.0f, 4.0f);
}

void initBirds() {
    // Birds start with small random differences so the flock looks natural.
    for (int i = 0; i < BIRD_COUNT; i++) {
        birds[i].x = static_cast<float>(rand() % WINDOW_WIDTH);
        birds[i].y = 480.0f + static_cast<float>(rand() % 200);
        birds[i].vx = 0.8f + static_cast<float>(rand() % 80) / 100.0f;
        birds[i].vy = -0.15f + static_cast<float>(rand() % 30) / 100.0f;
        birds[i].wingAngle = static_cast<float>(rand() % 60 - 30);
        birds[i].wingSpeed = 1.2f + static_cast<float>(rand() % 120) / 100.0f;
        birds[i].isFlying = true;
    }
}

void updateBirds() {
    if (isRainEnabled) return;

    float birdMotionScale = 0.75f;
    for (int i = 0; i < BIRD_COUNT; i++) {
        if (!birds[i].isFlying) continue;

        // Position changes use the bird's own velocity.
        birds[i].x += birds[i].vx * animationSpeed * birdMotionScale;
        birds[i].y += birds[i].vy * animationSpeed * birdMotionScale;
        // Wing angle changes separately so flying feels alive.
        birds[i].wingAngle += birds[i].wingSpeed * animationSpeed * birdMotionScale;

        // Reverse wing direction when the flap reaches its limit.
        if (birds[i].wingAngle > 35.0f || birds[i].wingAngle < -35.0f) {
            birds[i].wingSpeed = -birds[i].wingSpeed;
            birds[i].wingAngle = clamp(birds[i].wingAngle, -35.0f, 35.0f);
        }

        // Wrap and bounce keep birds inside the visible sky area.
        if (birds[i].x > WINDOW_WIDTH + 40.0f) birds[i].x = -40.0f;
        if (birds[i].x < -40.0f) birds[i].x = WINDOW_WIDTH + 40.0f;
        if (birds[i].y > WINDOW_HEIGHT - 40.0f) birds[i].vy = -fabsf(birds[i].vy);
        if (birds[i].y < 420.0f) birds[i].vy = fabsf(birds[i].vy);
    }
}

void drawBirds(bool eveningPalette) {
    if (isRainEnabled) return;

    // Three points are enough to suggest flying wings.
    glLineWidth(2.0f);
    if (eveningPalette) glColor3f(0.18f, 0.12f, 0.14f);
    else glColor3f(0.12f, 0.14f, 0.18f);

    for (int i = 0; i < BIRD_COUNT; i++) {
        if (!birds[i].isFlying) continue;
        float x = birds[i].x;
        float y = birds[i].y;
        // Wing angle controls how open or closed the bird looks.
        float flap = birds[i].wingAngle * 0.30f;

        glBegin(GL_LINE_STRIP);
        glVertex2f(x - 12.0f, y - flap);
        glVertex2f(x, y + flap);
        glVertex2f(x + 12.0f, y - flap);
        glEnd();
    }
    glLineWidth(1.0f);
}

void drawScene1Airplane() {
    glPushMatrix();
    // Move the airplane first, then rotate and scale the whole model.
    glTranslatef(scene1_planeX, scene1_planeY, 0.0f);
    glRotatef(scene1_planeTilt, 0.0f, 0.0f, 1.0f);
    glScalef(1.45f, 1.45f, 1.0f);

    // A few polygons build a simple side-view airplane.
    glColor3f(0.03f, 0.03f, 0.04f);

    glBegin(GL_POLYGON);
    glVertex2f(-8.0f, 2.0f);
    glVertex2f(18.0f, 2.0f);
    glVertex2f(8.0f, -26.0f);
    glVertex2f(-18.0f, -22.0f);
    glEnd();

    glColor3f(0.07f, 0.07f, 0.08f);
    glBegin(GL_POLYGON);
    glVertex2f(-55.0f, -6.0f);
    glVertex2f(30.0f, -7.0f);
    glVertex2f(56.0f, 0.0f);
    glVertex2f(30.0f, 8.0f);
    glVertex2f(-55.0f, 7.0f);
    glEnd();

    glColor3f(0.04f, 0.04f, 0.045f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-48.0f, 6.0f);
    glVertex2f(-28.0f, 6.0f);
    glVertex2f(-44.0f, 26.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-50.0f, -4.0f);
    glVertex2f(-30.0f, -4.0f);
    glVertex2f(-46.0f, -16.0f);
    glEnd();

    glColor3f(0.34f, 0.40f, 0.46f);
    glBegin(GL_TRIANGLES);
    glVertex2f(32.0f, 3.0f);
    glVertex2f(47.0f, 1.0f);
    glVertex2f(36.0f, 7.0f);
    glEnd();

    glColor3f(0.22f, 0.26f, 0.30f);
    for (float wx = -24.0f; wx <= 12.0f; wx += 12.0f) {
        ellipse(wx, 3.5f, 2.0f, 1.4f, 10);
    }

    glPopMatrix();
}

// Morning departure scene.
void scene1() {
    float treeSway = sinf(grassSwayTimer * 0.02f) * 3.0f;
    if (!isRainEnabled) {
        // Sunny 
        gradSky(0.68f, 0.88f, 0.99f, 0.88f, 0.96f, 1.00f);
        sun(1020.0f + sunHorizontalOffset * 0.20f, 620.0f);

        cloud(210.0f + cloudOffsetX_layerA * 0.70f, 620.0f, 1.0f);
        cloud(520.0f + cloudOffsetX_layerB * 0.60f, 650.0f, 1.0f);
        drawScene1Airplane();
    } else {
        //rainy
        gradSky(0.24f, 0.26f, 0.29f, 0.36f, 0.38f, 0.41f);
        cloud(220.0f + cloudOffsetX_layerA * 0.5f, 632.0f, 0.8f);
        cloud(620.0f + cloudOffsetX_layerB * 0.5f, 654.0f, 0.75f);
        cloud(850.0f + cloudOffsetX_layerC * 0.5f, 600.0f, 0.7f);
    }

    homeGround(false);

    treeAnimated(60.0f, 120.0f, treeSway * 1.00f);
    treeAnimated(1120.0f, 125.0f, treeSway * 0.90f);
    treeAnimated(1050.0f, 118.0f, treeSway * 1.10f);
    treeAnimated(980.0f, 128.0f, treeSway * 0.80f);

    house(false);

    drawCar(scene1_carPosX, scene1_carPosY, 0.84f, 0.20f, 0.18f, wheelRotationAngle, false, false, true);
    drawGarageDoor();
    drawBirds(false);
}

// Night return scene.
void scene9() {
    float treeSway = sinf(grassSwayTimer * 0.02f) * 3.0f;
    gradSky(0.05f, 0.08f, 0.18f, 0.11f, 0.16f, 0.28f);
    stars();
    moon(1050.0f, 620.0f);

    homeGround(true);
    if (!isRainEnabled) drawFireflies();

    treeAnimated(60.0f, 120.0f, treeSway * 1.00f);
    treeAnimated(1120.0f, 125.0f, treeSway * 0.90f);
    treeAnimated(1050.0f, 118.0f, treeSway * 1.10f);
    treeAnimated(980.0f, 128.0f, treeSway * 0.80f);

    house(isHouseLightOn);

    bool headOn = (carState_scene9 < 2);
    // Headlight stays on while the car is still outside the garage.
    drawCar(scene9_carPosX, scene9_carPosY, 0.84f, 0.20f, 0.18f, wheelRotationAngle, headOn, true, true);
    drawHeadlightCone(scene9_carPosX, scene9_carPosY, headOn);

    drawGarageDoor();
}


// OpenGL setup
void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 2D coordinate system matching the window size.
    glOrtho(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);

    initRain();

    // Slight speed difference stops clouds from moving as one group.
    cloudDriftSpeedA = 0.3f + (rand() % 20) / 100.0f;
    cloudDriftSpeedB = 0.25f + (rand() % 20) / 100.0f;
    cloudDriftSpeedC = 0.35f + (rand() % 20) / 100.0f;

    for (int i = 0; i < 10; i++) {
        // Each window starts with a slightly different brightness.
        windowFlickerAmount[i] = 0.85f + (rand() % 30) / 100.0f;
    }

    initBirds();
}

// Put each scene back to its starting state.
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
        scene1_planeX = 980.0f;
        scene1_planeY = 780.0f;
        scene1_planeTilt = 200.0f;
        initBirds();
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

// Scene 1 car flow
void anim1() {
    updateBirds();
    if (!isRainEnabled) {
    scene1_planeX -= 0.90f * animationSpeed;
    scene1_planeY -= 0.30f * animationSpeed;

    if (scene1_planeX < -260.0f || scene1_planeY < -120.0f) {
        scene1_planeX = 980.0f + (float)(rand() % 120);
        scene1_planeY = 780.0f + (float)(rand() % 40);
    }
}

    const float scene1RoadTravelY = 50.0f;
    const float scene1ExitCheckX = 1350.0f;

    if (carState_scene1 == 0) {
        // changes the state.
        carState_scene1 = 1;
    }
    else if (carState_scene1 == 1) {
        // Open door
        garageDoorOpenAmount += 1.5f * animationSpeed;
        // When enough space is open, the car can start coming down.
        if (garageDoorOpenAmount >= homeGarageDoorHeight - 5.0f) {
            garageDoorOpenAmount = homeGarageDoorHeight - 5.0f;
            carState_scene1 = 2;
        }
    }
    else if (carState_scene1 == 2) {
        // Move car down
        float speedY = 2.0f * animationSpeed;
        scene1_carPosY -= speedY;
        // Wheel still rotates here so it feels like active motion.
        wheelRotationAngle -= speedY * 2.0f;
        if (scene1_carPosY <= scene1RoadTravelY) {
            scene1_carPosY = scene1RoadTravelY;
            carState_scene1 = 3;
        }
    }
    else if (carState_scene1 == 3) {
        // Drive out
        float speed = 3.0f * animationSpeed;
        scene1_carPosX += speed;
        wheelRotationAngle -= speed * 2.0f;

        if (garageDoorOpenAmount > 0) {
            // Door closes while the car is already leaving.
            garageDoorOpenAmount -= 1.0f * animationSpeed;
            if (garageDoorOpenAmount < 0) garageDoorOpenAmount = 0;
        }

        if (scene1_carPosX > scene1ExitCheckX) {
            scene1HasCarExitedScreen = true;
            // After leaving home, switch to the return scene.
            nextScene();
        }
    }
}

// Scene 9 car flow: arrive -> open door -> park -> finish.
void anim9() {
    if (carState_scene9 == 0) {
        // Come home
        float step = 3.0f * animationSpeed;
        scene9_carPosX += step;
        wheelRotationAngle -= step * 2.0f;

        if (scene9_carPosX >= 296.0f) {
            scene9_carPosX = 296.0f;
            carState_scene9 = 1;
        }
    }
    else if (carState_scene9 == 1) {
        // Open door
        garageDoorOpenAmount += 1.5f * animationSpeed;
        if (garageDoorOpenAmount >= homeGarageDoorHeight - 5.0f) {
            garageDoorOpenAmount = homeGarageDoorHeight - 5.0f;
            carState_scene9 = 2;
        }
    }
    else if (carState_scene9 == 2) {
        // Park car
        float up = 2.0f * animationSpeed;
        scene9_carPosY += up;
        // Upward motion makes it look like the car enters the garage.
        wheelRotationAngle -= up * 1.8f;

        if (scene9_carPosY >= 330.0f) {
            scene9_carPosY = 330.0f;
            carState_scene9 = 3;
            scene9ParkingCompleted = true;
            // House light turns on after the car safely gets home.
            isHouseLightOn = true;
        }
    }
    else if (carState_scene9 == 3) {
        // Finish
        scene9ParkedFrameCounter++;
        if (garageDoorOpenAmount > 0) {
            // Final step is closing the garage.
            garageDoorOpenAmount -= 1.0f * animationSpeed;

            if (garageDoorOpenAmount <= 0) {
                garageDoorOpenAmount = 0;
                // Pause here so the final parked frame stays visible.
                isPaused = true;
                currentScene = 1;
                resetScene(1);
            }
        }
    }
}

// Draw the active scene, then draw rain on top if needed.
void display() {
    if (currentScene == 1) {
        glClearColor(0.62f, 0.84f, 0.97f, 1.0f);
    } else {
        glClearColor(0.05f, 0.07f, 0.15f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT);
    // Reset transform before drawing the current frame.
    glLoadIdentity();

    if (currentScene == 1) {
        scene1();
    } else if (currentScene == 9) {
        scene9();
    }

    if (isRainEnabled) {
        // Rain is last so it stays in front of everything.
        drawRain();
    }

    glutSwapBuffers();
}

// One timer updates all shared animation values.
void update(int val) {
    if (!isPaused) {
        sunGlowPulse += 0.01f * sunGlowDirection * animationSpeed;
        // Keep the pulse between 0 and 1, then reverse it.
        if (sunGlowPulse >= 1.0f) { 
            sunGlowPulse = 1.0f; 
            sunGlowDirection = -1.0f; 
        }
        if (sunGlowPulse <= 0.0f) { 
            sunGlowPulse = 0.0f; 
            sunGlowDirection = 1.0f; 
        }

        grassSwayTimer += 1.0f * animationSpeed;

        // Clouds keep moving and jump back after leaving the screen.
        cloudOffsetX_layerA += cloudDriftSpeedA * animationSpeed;
        cloudOffsetX_layerB += cloudDriftSpeedB * animationSpeed;
        cloudOffsetX_layerC += cloudDriftSpeedC * animationSpeed;
        if (cloudOffsetX_layerA > 1400) cloudOffsetX_layerA = -300;
        if (cloudOffsetX_layerB > 1400) cloudOffsetX_layerB = -300;
        if (cloudOffsetX_layerC > 1400) cloudOffsetX_layerC = -300;

        windowFlickerTimer += 1.0f * animationSpeed;
        for (int i = 0; i < 10; i++) {
            // Different phase per window makes flicker look less robotic.
            windowFlickerAmount[i] = 0.85f + 0.15f * sin(windowFlickerTimer * 0.03f + i * 1.2f);
        }

        if (currentScene == 1) anim1();
        else if (currentScene == 9) anim9();

        if (isRainEnabled) {
            for (int i = 0; i < RAIN_DROP_COUNT; i++) {
                // Downward movement is stronger than sideways drift.
                rainDropY[i] -= 9.0f;
                rainDropX[i] -= 1.4f;

                // Reuse the same drops by sending them back to the top.
                if (rainDropY[i] < 0) {
                    rainDropY[i] = WINDOW_HEIGHT;
                    rainDropX[i] = rand() % WINDOW_WIDTH;
                }
            }
        }
    }
    // Ask GLUT to draw the next frame.
    glutPostRedisplay();
    // Call update again after about 16 ms for smooth animation.
    glutTimerFunc(TIMER_INTERVAL_MS, update, 0);
}

// Keyboard controls for rain, pause, and reset.
void keyboard(unsigned char key, int x, int y) {
    // ESC closes the program quickly.
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
        // Left arrow toggles between the two scenes.
        if (currentScene == 9) currentScene = 1;
        else if (currentScene == 1) currentScene = 9;
        resetScene(currentScene);
    }
    if (key == GLUT_KEY_RIGHT) {
        // Right arrow uses the normal scene change function.
        nextScene();
    }
}

// Program entry point.
int main(int argc, char** argv) {
    // Standard GLUT window setup.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Modern Urban Area - Clean Build");

    // Prepare scene data before the loop starts.
    init();
    resetScene(currentScene);

    // Register the functions GLUT will keep calling.
    glutDisplayFunc(display);
    glutTimerFunc(TIMER_INTERVAL_MS, update, 0);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}
