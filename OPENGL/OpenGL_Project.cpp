#define _USE_MATH_DEFINES // Required for M_PI in some compilers (e.g., Visual Studio)
// This line ensures that M_PI, the mathematical constant pi, is defined. 
// It's often needed for compilers like Visual Studio to include it from <cmath>.

#include <glut.h> // Includes the GLUT library for OpenGL utility toolkit functionalities.
#include <cmath> // Includes the C++ standard math library, providing functions like sin, cos, sqrt, fabs.
#include <cstdlib> // Includes the C standard library, providing general utilities like rand and srand for random numbers.
#include <ctime> // Includes the C time library, used for seeding the random number generator with current time.
#include <vector> // Includes the C++ standard library for using dynamic arrays (std::vector).
#include <algorithm> // Includes the C++ standard algorithm library, providing functions like std::min and std::max.
#include <cstdio>    // Includes the C standard input/output library, used here for printf debugging.

// --- World Coordinates ---
const float WORLD_WIDTH = 4.0f; // Defines the width of the simulated world.
const float WORLD_HEIGHT = 4.0f; // Defines the height of the simulated world.

// --- Animation Variables ---
float y_pos = 2.0f; // Parachutist's initial base Y-position in world coordinates.
float velocity = 0.0f; // Parachutist's initial vertical velocity.
float gravity = -0.00005f; // Defines the acceleration due to gravity (negative for downward direction).
const float ground_level = -1.0f; // Defines the Y-coordinate of the lowest green layer (ground).
float sway_phase = 0.0f; // Used to control the phase of the parachute's 'breathing' or sway effect.
float sway_amplitude = 0.2f; // Defines the amplitude of the parachute's sway.
bool landed = false; // Flag to track if the parachutist has landed.
bool jumped = false; // Flag to track if the parachutist has jumped from the plane.
float plane_x = -2.5f; // Initial X-position of the plane.
float drift_x = 0.0f; // Accumulates the parachutist's horizontal movement due to wind.
bool plane_stopped = false; // Flag to indicate if the plane has stopped to allow the jump.

// --- Global wind speed factor ---
float wind_strength = 1.0f; // Multiplier for global wind strength, affecting drift and cloud speed.

// --- Landing Slowdown Parameters ---
const float LANDING_SLOWDOWN_DISTANCE = 0.5f; // Distance above the ground where landing slowdown begins.
const float LANDING_DRAG_FACTOR = 0.0001f; // How much drag is applied to slow down the landing.

// --- Post-Landing Sway and Collapse Parameters ---
float post_land_sway_timer = 0.0f; // Timer for how long the parachute sways after landing.
const float POST_LAND_SWAY_DURATION = 2.0f; // Duration in seconds for the post-landing sway.
const float PARACHUTE_COLLAPSE_SPEED = 0.005f; // Speed at which the parachute collapses.
float parachute_current_ry_scale = 1.0f; // Current vertical radius scale of the parachute (1.0 means full size).

// MODIFIED: Parachute's relative Y offset from human after detach (for its independent fall)
float parachute_relative_y_offset_after_detach; // Stores the parachute's Y offset once it detaches.
bool parachute_detaching_and_falling = false; // Flag to indicate if the parachute is falling independently.

// --- Scenery Variables ---
const float BASE_CLOUD1_SPEED = 0.001f; // Base speed for cloud 1.
const float BASE_CLOUD2_SPEED = 0.0008f; // Base speed for cloud 2.
const float BASE_CLOUD3_SPEED = 0.0012f; // Base speed for cloud 3.
float cloud1_x = -2.2f, cloud2_x = 1.0f, cloud3_x = -1.0f; // Initial X-positions for the clouds.

// --- Butterfly structure and container ---
struct Butterfly { // Defines a structure to hold properties of a single butterfly.
    float x, y; // Current position of the butterfly.
    float initial_spawn_x, initial_spawn_y; // Initial spawn position to create an oscillating path.
    float wing_angle; // Current angle of the butterfly's wings for animation.
    float flight_phase_offset; // Random offset for individual butterfly flight paths.
    float flight_speed_mult_x; // Multiplier for horizontal oscillation speed.
    float flight_speed_mult_y; // Multiplier for vertical oscillation speed.
    float flight_amplitude_x; // Amplitude for horizontal oscillation.
    float flight_amplitude_y; // Amplitude for vertical oscillation.
    float color[3]; // RGB color components for the butterfly.
};
std::vector<Butterfly> butterflies; // A dynamic array (vector) to store multiple butterfly objects.

// --- NEW: Running Animation Variables ---
bool isRunning = false; // Flag to indicate if the human is in a running state.
float run_phase = 0.0f; // Phase variable for the running animation (controls leg/arm movement).
const float RUN_SPEED = 0.008f; // Speed at which the human runs horizontally.

// --- Utility Functions ---
void drawCircle(float cx, float cy, float r, int segments = 50) {
    glBegin(GL_TRIANGLE_FAN); // Starts drawing a filled circle using a triangle fan.
    glVertex2f(cx, cy); // Center of the circle (first vertex of the fan).
    for (int i = 0; i <= segments; ++i) { // Loop to draw segments of the circle.
        float theta = i * 2.0f * M_PI / segments; // Calculates the angle for each segment.
        glVertex2f(cx + r * cos(theta), cy + r * sin(theta)); // Calculates and sets the vertex for the circle's perimeter.
    }
    glEnd(); // Ends drawing the triangle fan.
}

// --- Scenery Drawing Functions ---

void drawSun() {
    float sun_cx = -1.0f; // X-coordinate of the sun's center.
    float sun_cy = 1.5f; // Y-coordinate of the sun's center.
    float sun_radius = 0.15f; // Radius of the sun.

    // Sun body - only the solid circle, no rays or glow
    glColor3f(1.0f, 1.0f, 0.0f); // Sets the drawing color to bright yellow for the sun.
    drawCircle(sun_cx, sun_cy, sun_radius); // Draws the solid yellow circle for the sun.
}

// Function for drawing the dark green, conical trees
void drawDarkGreenTree(float x, float y, float scale = 1.0f) {
    // Trunk
    glColor3f(0.3f, 0.2f, 0.05f); // Sets color for the tree trunk (darker brown).
    glBegin(GL_QUADS); // Starts drawing the rectangular trunk.
    glVertex2f(x - 0.02f * scale, y - 0.1f * scale); // Bottom-left vertex of the trunk.
    glVertex2f(x + 0.02f * scale, y - 0.1f * scale); // Bottom-right vertex of the trunk.
    glVertex2f(x + 0.02f * scale, y); // Top-right vertex of the trunk.
    glVertex2f(x - 0.02f * scale, y); // Top-left vertex of the trunk.
    glEnd(); // Ends drawing the trunk.

    // Foliage (conical shape using triangles)
    glColor3f(0.0f, 0.4f, 0.1f); // Sets color for the foliage (dark green).
    glBegin(GL_TRIANGLES); // Starts drawing the bottom layer of foliage.
    glVertex2f(x, y + 0.2f * scale); // Top point of the cone.
    glVertex2f(x - 0.12f * scale, y); // Bottom-left point.
    glVertex2f(x + 0.12f * scale, y); // Bottom-right point.
    glEnd(); // Ends drawing the bottom foliage.

    glBegin(GL_TRIANGLES); // Starts drawing the middle layer of foliage.
    glVertex2f(x, y + 0.25f * scale); // Top point of the middle cone.
    glVertex2f(x - 0.1f * scale, y + 0.05f * scale); // Bottom-left point.
    glVertex2f(x + 0.1f * scale, y + 0.05f * scale); // Bottom-right point.
    glEnd(); // Ends drawing the middle foliage.

    glBegin(GL_TRIANGLES); // Starts drawing the top layer of foliage.
    glVertex2f(x, y + 0.3f * scale); // Top point of the top cone.
    glVertex2f(x - 0.07f * scale, y + 0.15f * scale); // Bottom-left point.
    glVertex2f(x + 0.07f * scale, y + 0.15f * scale); // Bottom-right point.
    glEnd(); // Ends drawing the top foliage.
}

// --- Main Landscape/Ground Drawing Function ---
void drawGround() {
    // Sky gradient
    glBegin(GL_QUADS); // Starts drawing a rectangle for the sky.
    glColor3f(0.5f, 0.8f, 1.0f); // Sets color for the top of the sky (brighter blue).
    glVertex2f(-WORLD_WIDTH / 2, WORLD_HEIGHT / 2); // Top-left vertex.
    glVertex2f(WORLD_WIDTH / 2, WORLD_HEIGHT / 2); // Top-right vertex.
    glColor3f(0.8f, 0.9f, 1.0f); // Sets color for the horizon (lighter blue).
    glVertex2f(WORLD_WIDTH / 2, -WORLD_HEIGHT / 2); // Bottom-right vertex.
    glVertex2f(-WORLD_WIDTH / 2, -WORLD_HEIGHT / 2); // Bottom-left vertex.
    glEnd(); // Ends drawing the sky.

    // Farthest Green Hill Layer (lighter and more desaturated for depth)
    glColor3f(0.6f, 0.9f, 0.4f); // Sets color for the farthest hill (lighter green).
    glBegin(GL_POLYGON); // Starts drawing a custom-shaped polygon for the hill.
    glVertex2f(-WORLD_WIDTH / 2, ground_level + 0.3f); // Various vertices define the hill's contour.
    glVertex2f(-1.5f, ground_level + 0.5f);
    glVertex2f(0.0f, ground_level + 0.4f);
    glVertex2f(1.8f, ground_level + 0.6f);
    glVertex2f(WORLD_WIDTH / 2, ground_level + 0.3f);
    glVertex2f(WORLD_WIDTH / 2, ground_level); // Connects to the base of the world.
    glVertex2f(-WORLD_WIDTH / 2, ground_level); // Connects to the base of the world.
    glEnd(); // Ends drawing the farthest hill.

    // Trees on the farthest hill (conical, increased quantity)
    // Multiple calls to drawDarkGreenTree with different positions and scales for varied trees.
    drawDarkGreenTree(-2.0f, ground_level + 0.3f, 0.7f);
    drawDarkGreenTree(-1.9f, ground_level + 0.32f, 0.75f);
    drawDarkGreenTree(-1.8f, ground_level + 0.35f, 0.8f);
    drawDarkGreenTree(-1.7f, ground_level + 0.37f, 0.7f);
    drawDarkGreenTree(-1.6f, ground_level + 0.4f, 0.82f);
    drawDarkGreenTree(-1.5f, ground_level + 0.43f, 0.78f);
    drawDarkGreenTree(-1.4f, ground_level + 0.45f, 0.85f);
    drawDarkGreenTree(-1.3f, ground_level + 0.47f, 0.8f);
    drawDarkGreenTree(-1.2f, ground_level + 0.48f, 0.88f);
    drawDarkGreenTree(-1.1f, ground_level + 0.45f, 0.75f);

    drawDarkGreenTree(-0.9f, ground_level + 0.42f, 0.7f);
    drawDarkGreenTree(-0.7f, ground_level + 0.45f, 0.8f);
    drawDarkGreenTree(-0.5f, ground_level + 0.47f, 0.85f);
    drawDarkGreenTree(-0.3f, ground_level + 0.44f, 0.79f);
    drawDarkGreenTree(-0.1f, ground_level + 0.42f, 0.72f);
    drawDarkGreenTree(0.1f, ground_level + 0.40f, 0.75f);

    drawDarkGreenTree(0.5f, ground_level + 0.45f, 0.8f);
    drawDarkGreenTree(0.7f, ground_level + 0.48f, 0.88f);
    drawDarkGreenTree(0.9f, ground_level + 0.50f, 0.9f);
    drawDarkGreenTree(1.1f, ground_level + 0.52f, 0.95f);
    drawDarkGreenTree(1.3f, ground_level + 0.55f, 0.92f);
    drawDarkGreenTree(1.5f, ground_level + 0.53f, 0.88f);
    drawDarkGreenTree(1.7f, ground_level + 0.50f, 0.85f);
    drawDarkGreenTree(1.9f, ground_level + 0.47f, 0.8f);

    // Middle Green Hill Layer (medium green with subtle sun effect on ground)
    glColor3f(0.4f, 0.8f, 0.2f); // Sets color for the middle hill (medium green).
    glBegin(GL_POLYGON); // Starts drawing a custom-shaped polygon for the middle hill.
    glVertex2f(-WORLD_WIDTH / 2, ground_level + 0.1f);
    glVertex2f(-1.0f, ground_level + 0.3f);

    glColor3f(0.6f, 0.9f, 0.3f); // Sets brighter green for a sunlit part of the hill.
    glVertex2f(0.5f, ground_level + 0.2f);
    glVertex2f(WORLD_WIDTH / 2, ground_level + 0.1f);

    glColor3f(0.4f, 0.8f, 0.2f); // Returns to medium green for the base of the hill.
    glVertex2f(WORLD_WIDTH / 2, ground_level);
    glVertex2f(-WORLD_WIDTH / 2, ground_level);
    glEnd(); // Ends drawing the middle hill.

    // Trees on the middle hill (conical, increased quantity)
    // More calls to drawDarkGreenTree for trees on the middle hill.
    drawDarkGreenTree(-1.8f, ground_level + 0.1f, 0.8f);
    drawDarkGreenTree(-1.6f, ground_level + 0.15f, 0.85f);
    drawDarkGreenTree(-1.4f, ground_level + 0.20f, 0.9f);
    drawDarkGreenTree(-1.2f, ground_level + 0.25f, 0.95f);
    drawDarkGreenTree(-1.0f, ground_level + 0.28f, 1.0f);
    drawDarkGreenTree(-0.8f, ground_level + 0.25f, 0.9f);
    drawDarkGreenTree(-0.6f, ground_level + 0.22f, 0.85f);
    drawDarkGreenTree(-0.4f, ground_level + 0.20f, 0.8f);

    drawDarkGreenTree(0.0f, ground_level + 0.18f, 0.88f);
    drawDarkGreenTree(0.2f, ground_level + 0.15f, 0.92f);
    drawDarkGreenTree(0.4f, ground_level + 0.12f, 0.85f);
    drawDarkGreenTree(0.6f, ground_level + 0.09f, 0.8f);
    drawDarkGreenTree(0.8f, ground_level + 0.07f, 0.75f);
    drawDarkGreenTree(1.0f, ground_level + 0.05f, 0.7f);
    drawDarkGreenTree(1.2f, ground_level + 0.03f, 0.65f);
    drawDarkGreenTree(1.4f, ground_level + 0.01f, 0.6f);
    drawDarkGreenTree(1.6f, ground_level - 0.01f, 0.55f);
    drawDarkGreenTree(1.8f, ground_level - 0.03f, 0.5f);

    // Closest Green Hill/Foreground (darker, more vibrant green)
    glBegin(GL_QUADS); // Starts drawing a rectangle for the foreground.
    glColor3f(0.3f, 0.7f, 0.1f); // Sets darker green for the top of the foreground.
    glVertex2f(-WORLD_WIDTH / 2, ground_level);
    glVertex2f(WORLD_WIDTH / 2, ground_level);
    glColor3f(0.25f, 0.6f, 0.08f); // Sets slightly darker green for the bottom.
    glVertex2f(WORLD_WIDTH / 2, -WORLD_HEIGHT / 2);
    glVertex2f(-WORLD_WIDTH / 2, -WORLD_HEIGHT / 2);
    glEnd(); // Ends drawing the foreground.

    // Small foreground details (flowers/dots) - clear landing area remains
    glColor3f(1.0f, 1.0f, 1.0f); // Sets color to white for small dots/flowers.
    drawCircle(-0.3f, ground_level - 0.2f, 0.008f); // Draws a small white circle.
    drawCircle(0.8f, ground_level - 0.1f, 0.008f); // Draws another small white circle.
    drawCircle(1.5f, ground_level - 0.3f, 0.008f); // Draws a third small white circle.
}

void drawCloud(float x, float y, float scale) {
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f); // Sets color to white with 90% opacity (semi-transparent).
    drawCircle(x, y, 0.07f * scale); // Draws a main circle for the cloud.
    drawCircle(x + 0.05f * scale, y + 0.02f * scale, 0.06f * scale); // Draws overlapping circles to form a cloud shape.
    drawCircle(x + 0.10f * scale, y, 0.05f * scale);
    drawCircle(x + 0.02f * scale, y - 0.02f * scale, 0.06f * scale);
}

// --- Redesigned, more detailed plane ---
void drawImprovedPlane(float x) {
    float y = 1.5f; // Y-coordinate of the plane.
    // Fuselage
    glColor3f(0.8f, 0.8f, 0.85f); // Sets color for the plane's body (light gray).
    glBegin(GL_POLYGON); // Starts drawing the fuselage as a polygon.
    glVertex2f(x - 0.25f, y - 0.03f); // Defines the shape of the fuselage.
    glVertex2f(x + 0.2f, y - 0.03f);
    glVertex2f(x + 0.25f, y);
    glVertex2f(x + 0.2f, y + 0.03f);
    glVertex2f(x - 0.25f, y + 0.03f);
    glEnd(); // Ends drawing the fuselage.

    // Cockpit
    glColor3f(0.5f, 0.8f, 1.0f); // Sets color for the cockpit (light blue).
    glBegin(GL_POLYGON); // Starts drawing the cockpit.
    glVertex2f(x + 0.18f, y + 0.03f); // Defines the triangular shape of the cockpit.
    glVertex2f(x + 0.25f, y);
    glVertex2f(x + 0.18f, y);
    glEnd(); // Ends drawing the cockpit.

    // Wings
    glColor3f(0.6f, 0.6f, 0.65f); // Sets color for the wings (medium gray).
    glBegin(GL_POLYGON); // Starts drawing the wing.
    glVertex2f(x - 0.05f, y); // Defines the shape of the main wing.
    glVertex2f(x + 0.05f, y);
    glVertex2f(x - 0.1f, y - 0.2f);
    glVertex2f(x - 0.15f, y - 0.2f);
    glEnd(); // Ends drawing the wing.

    // Tail Fin
    glColor3f(0.9f, 0.1f, 0.1f); // Sets color for the tail fin (red).
    glBegin(GL_POLYGON); // Starts drawing the tail fin.
    glVertex2f(x - 0.25f, y + 0.03f); // Defines the shape of the tail fin.
    glVertex2f(x - 0.20f, y + 0.12f);
    glVertex2f(x - 0.28f, y + 0.03f);
    glEnd(); // Ends drawing the tail fin.
}

// --- Butterfly drawing function (smaller) ---
void drawButterfly(const Butterfly& b) {
    glPushMatrix(); // Saves the current transformation matrix.
    glTranslatef(b.x, b.y, 0); // Translates to the butterfly's current position.
    glColor3fv(b.color); // Sets the color of the butterfly.

    float wing_h_small = 0.02f; // Reduced height of the butterfly wings.
    float wing_w_small = 0.03f; // Reduced width of the butterfly wings.

    // Left Wing
    glPushMatrix(); // Saves the current matrix for the left wing.
    glRotatef(b.wing_angle, 0, 1, 0); // Rotates the wing based on the wing_angle (for flapping effect).
    glBegin(GL_TRIANGLES); // Starts drawing the left wing as a triangle.
    glVertex2f(0, 0); // Origin for the wing.
    glVertex2f(-wing_w_small, wing_h_small); // Top-left vertex.
    glVertex2f(-wing_w_small, -wing_h_small); // Bottom-left vertex.
    glEnd(); // Ends drawing the left wing.
    glPopMatrix(); // Restores the previous transformation matrix.

    // Right Wing
    glPushMatrix(); // Saves the current matrix for the right wing.
    glRotatef(-b.wing_angle, 0, 1, 0); // Rotates the right wing in the opposite direction.
    glBegin(GL_TRIANGLES); // Starts drawing the right wing.
    glVertex2f(0, 0); // Origin for the wing.
    glVertex2f(wing_w_small, wing_h_small); // Top-right vertex.
    glVertex2f(wing_w_small, -wing_h_small); // Bottom-right vertex.
    glEnd(); // Ends drawing the right wing.
    glPopMatrix(); // Restores the previous transformation matrix.

    glPopMatrix(); // Restores the initial transformation matrix before butterfly drawing.
}

// --- Parachutist Drawing Functions ---
void drawParachute(float cx_local_offset, float cy_local_offset, float rx, float ry); // Function prototype for drawing the parachute.
void drawRopes(float cx_local_offset, float cy_local_offset, float rx, float ry, float human_x_local_offset, float human_y_local_offset, float scale); // Function prototype for drawing the parachute ropes.
void drawHuman(float x_local_offset, float base_y_local_offset, float scale, bool is_running, float anim_phase); // Function prototype for drawing the human figure.

// --- Main Display Function ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT); // Clears the color buffer, effectively clearing the screen.

    // Draw scene elements from back to front
    drawGround(); // Draws the background landscape (sky, hills, ground).
    drawCloud(cloud1_x, 1.6f, 1.2f); // Draws cloud 1 with its current position and scale.
    drawCloud(cloud2_x, 1.4f, 1.0f); // Draws cloud 2.
    drawCloud(cloud3_x, 1.7f, 1.5f); // Draws cloud 3.
    drawSun(); // Draws the sun.
    drawImprovedPlane(plane_x); // Draws the plane at its current X-position.

    // Draw the parachutist
    if (jumped || landed) { // Only draw the parachutist if they have jumped or landed.
        glPushMatrix(); // Saves the current transformation matrix.
        glTranslatef(drift_x, y_pos, 0); // Translates the entire parachutist (human + parachute) to its current world position.
        float scale = 0.25f; // Defines the overall scale of the parachutist.
        float human_y_offset_local = -0.05f; // Local Y-offset for the human relative to the group's y_pos.

        float parachute_y_offset_initial = 0.15f; // Initial local Y-offset for the parachute.

        drawHuman(0, human_y_offset_local, scale, isRunning, run_phase); // Draws the human, potentially in running animation.

        if (!parachute_detaching_and_falling) { // If the parachute is still attached.
            drawParachute(0, parachute_y_offset_initial, 0.17f, 0.1f); // Draws the parachute.
            drawRopes(0, parachute_y_offset_initial, 0.17f, 0.1f, 0.0f, human_y_offset_local, scale); // Draws the ropes connecting human and parachute.
        }
        else { // If the parachute has detached and is falling independently.
            drawParachute(0, parachute_relative_y_offset_after_detach, 0.17f, 0.1f); // Draws the parachute at its new relative Y offset.
        }

        glPopMatrix(); // Restores the previous transformation matrix.
    }

    // Draw butterflies in the foreground
    for (const auto& b : butterflies) { // Iterates through each butterfly in the vector.
        drawButterfly(b); // Draws each butterfly.
    }

    glutSwapBuffers(); // Swaps the front and back buffers to display the rendered scene.
}

// --- Animation Update Function ---
void update(int value) {
    // --- Scenery animations - Cloud speed now depends on wind_strength ---
    cloud1_x += BASE_CLOUD1_SPEED * wind_strength; // Updates cloud 1's X-position, scaled by wind strength.
    if (cloud1_x > WORLD_WIDTH / 2 + 0.3f) cloud1_x = -WORLD_WIDTH / 2 - 0.3f; // Resets cloud 1 to the left when it moves off-screen right.

    cloud2_x += BASE_CLOUD2_SPEED * wind_strength; // Updates cloud 2's X-position.
    if (cloud2_x > WORLD_WIDTH / 2 + 0.3f) cloud2_x = -WORLD_WIDTH / 2 - 0.3f; // Resets cloud 2.

    cloud3_x += BASE_CLOUD3_SPEED * wind_strength; // Updates cloud 3's X-position.
    if (cloud3_x > WORLD_WIDTH / 2 + 0.3f) cloud3_x = -WORLD_WIDTH / 2 - 0.3f; // Resets cloud 3.

    // Butterfly animation
    float current_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Gets current time in seconds since GLUT started.
    for (auto& b : butterflies) { // Iterates through each butterfly.
        b.x = b.initial_spawn_x + (b.flight_amplitude_x * sin(current_time * b.flight_speed_mult_x + b.flight_phase_offset)); // Updates butterfly's X-position with sinusoidal motion.
        b.y = b.initial_spawn_y + (b.flight_amplitude_y * cos(current_time * b.flight_speed_mult_y + b.flight_phase_offset * 1.5f)); // Updates butterfly's Y-position with sinusoidal motion.
        b.wing_angle = 45.0f * sin(current_time * 5.0f + b.flight_phase_offset * 2.0f); // Updates wing angle for flapping.
        // Wrap around butterflies if they go off-screen.
        if (b.x > WORLD_WIDTH / 2 + 0.1f) b.x = -WORLD_WIDTH / 2 - 0.1f;
        if (b.x < -WORLD_WIDTH / 2 - 0.1f) b.x = WORLD_WIDTH / 2 + 0.1f;
        if (b.y > WORLD_HEIGHT / 2 + 0.1f) b.y = -WORLD_HEIGHT / 2 - 0.1f;
        if (b.y < -WORLD_HEIGHT / 2 - 0.1f) b.y = WORLD_HEIGHT / 2 + 0.1f;
    }

    // Plane animation before jump
    if (!plane_stopped) { // If the plane hasn't stopped yet.
        plane_x += 0.005f; // Move the plane to the right.
        if (plane_x >= 0.0f && !jumped) { // If the plane reaches the jump point and the parachutist hasn't jumped.
            plane_x = 0.0f; // Stop the plane at the jump point.
            plane_stopped = true; // Set the flag to true.
        }
    }

    // Trigger the jump
    if (plane_stopped && !jumped) { // If the plane has stopped and the parachutist hasn't jumped yet.
        jumped = true; // Set jumped flag to true.
        y_pos = 1.45f; // Set initial Y-position for the parachutist after jumping.
        parachute_relative_y_offset_after_detach = 0.15f; // Set initial relative offset for detached parachute.
        velocity = -0.0005f; // Give an initial downward velocity.
        drift_x = plane_x; // Inherit the plane's X-position for initial drift.
    }

    if (jumped) { // If the parachutist has jumped.
        // STATE 1: FALLING (Not landed yet)
        if (!landed) { // If the parachutist has not yet landed.
            sway_phase += 0.12f; // Increment sway phase for parachute 'breathing' effect.

            // Wind calculation with proper scaling for parachute drift
            float scaled_velocity_factor = fabs(velocity) * 1000.0f; // Scale velocity to influence wind effect.
            float wind_effect = wind_strength * scaled_velocity_factor; // Calculate total wind effect.
            drift_x += ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.005f * wind_effect; // Add random horizontal drift based on wind.

            drift_x = std::max(-WORLD_WIDTH / 2 + 0.2f, std::min(WORLD_WIDTH / 2 - 0.2f, drift_x)); // Clamp drift_x to stay within world boundaries.

            float human_base_y_offset = -0.05f; // Local Y-offset of the human's base.
            float human_scale = 0.25f; // Scale of the human.
            float human_height_approx = (0.22f + 0.15f) * human_scale; // Approximate height of the human.
            float human_feet_relative_y = human_base_y_offset - human_height_approx; // Relative Y-position of the human's feet.
            float current_feet_world_y = y_pos + human_feet_relative_y; // Absolute world Y-position of the human's feet.
            const float desired_landing_world_y = ground_level - 0.35f; // The exact world Y-position where the human's feet should land.

            if (current_feet_world_y > desired_landing_world_y) { // If feet are still above the desired landing height.
                velocity += gravity; // Apply gravity to increase downward velocity.
                float terminal_velocity_limit = -0.002f; // Define terminal velocity.
                velocity = std::max(velocity, terminal_velocity_limit); // Cap velocity at terminal velocity (don't fall too fast).
                float distance_to_ground_feet = current_feet_world_y - desired_landing_world_y; // Calculate distance to landing.
                if (distance_to_ground_feet < LANDING_SLOWDOWN_DISTANCE && distance_to_ground_feet > 0) { // If within slowdown distance.
                    float slowdown_factor = distance_to_ground_feet / LANDING_SLOWDOWN_DISTANCE; // Calculate slowdown factor (closer = more slowdown).
                    velocity += (LANDING_DRAG_FACTOR * (1.0f - slowdown_factor)); // Apply drag force, increasing as they get closer.
                    velocity = std::min(velocity, 0.0f); // Ensure velocity doesn't become positive (no upward bounce from drag).
                }
                y_pos += velocity; // Update the parachutist's Y-position.
            }
            else {
                // LANDING DETECTED!
                y_pos = desired_landing_world_y - human_feet_relative_y; // Snap Y-position to exact landing spot.
                velocity = 0.0f; // Stop vertical movement.
                landed = true; // Set landed flag to true.
                post_land_sway_timer = POST_LAND_SWAY_DURATION; // Start post-landing sway timer.
            }
        }
        // STATE 2: ON THE GROUND (Landed is true)
        else {
            // STATE 2A: PARACHUTE COLLAPSING (Not running yet)
            if (!isRunning) { // If the running animation hasn't started yet.
                if (post_land_sway_timer > 0) { // If the post-landing sway timer is still active.
                    post_land_sway_timer -= (16.0f / 1000.0f); // Decrement timer (16ms per frame).
                    sway_amplitude = 0.2f * (post_land_sway_timer / POST_LAND_SWAY_DURATION); // Decrease sway amplitude over time.
                    if (sway_amplitude < 0) sway_amplitude = 0; // Clamp sway amplitude to non-negative.
                    parachute_current_ry_scale -= (PARACHUTE_COLLAPSE_SPEED * 0.5f); // Slowly collapse the parachute.
                    if (parachute_current_ry_scale < 0.05f) parachute_current_ry_scale = 0.05f; // Prevent scale from going too small.
                }
                else { // After post-landing sway timer expires.
                    parachute_detaching_and_falling = true; // Flag parachute to fall independently.
                    parachute_current_ry_scale -= PARACHUTE_COLLAPSE_SPEED; // Continue collapsing the parachute.
                    if (parachute_current_ry_scale < 0.05f) parachute_current_ry_scale = 0.05f; // Clamp scale.

                    parachute_relative_y_offset_after_detach -= (PARACHUTE_COLLAPSE_SPEED * 1.5f); // Make the detached parachute fall faster.
                    float parachute_bottom_world_y = (y_pos + parachute_relative_y_offset_after_detach) - (0.1f * parachute_current_ry_scale); // Calculate parachute's world Y position.

                    if (parachute_bottom_world_y <= ground_level) { // If the parachute has hit the ground.
                        parachute_relative_y_offset_after_detach = ground_level + (0.1f * parachute_current_ry_scale) - y_pos; // Snap parachute to ground.
                        isRunning = true; // Start the running animation.
                    }
                }
            }
            // STATE 2B: RUNNING
            else {
                drift_x += RUN_SPEED; // Move the human horizontally (running).
                run_phase += 0.3f; // Increment run phase for leg/arm animation.
                float human_width_buffer = 0.2f; // Buffer to prevent human from instantly disappearing off-screen.
                if (drift_x > WORLD_WIDTH / 2 + human_width_buffer) { // If human runs off the right side.
                    drift_x = -WORLD_WIDTH / 2 - human_width_buffer; // Wrap around to the left side.
                }
            }
        }
    }

    // Let the plane fly away after the jump
    if (jumped && plane_stopped && plane_x <= WORLD_WIDTH / 2 + 0.5f) { // If parachutist jumped, plane stopped, and plane is still on screen.
        plane_x += 0.005f; // Move the plane further off-screen.
    }

    glutPostRedisplay(); // Requests a redraw of the window.
    glutTimerFunc(16, update, 0); // Sets a timer for the next update call (approx. 60 FPS).
}

// --- Initialization and Control ---
void initButterflies(int count) {
    butterflies.clear(); // Clears any existing butterflies.
    for (int i = 0; i < count; ++i) { // Loop to create 'count' number of butterflies.
        Butterfly b; // Create a new Butterfly object.
        b.initial_spawn_x = (static_cast<float>(rand()) / RAND_MAX) * WORLD_WIDTH - (WORLD_WIDTH / 2); // Random initial X.
        b.initial_spawn_y = (static_cast<float>(rand()) / RAND_MAX) * 0.8f + (ground_level - 0.5f); // Random initial Y (above ground).
        b.x = b.initial_spawn_x; // Set current X to initial spawn X.
        b.y = b.initial_spawn_y; // Set current Y to initial spawn Y.
        b.flight_phase_offset = (static_cast<float>(rand()) / RAND_MAX) * 100.0f; // Random phase offset for unique flight paths.
        b.flight_speed_mult_x = 0.1f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f; // Random X speed multiplier.
        b.flight_speed_mult_y = 0.1f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f; // Random Y speed multiplier.
        b.flight_amplitude_x = 0.02f + (static_cast<float>(rand()) / RAND_MAX) * 0.05f; // Random X amplitude.
        b.flight_amplitude_y = 0.02f + (static_cast<float>(rand()) / RAND_MAX) * 0.05f; // Random Y amplitude.
        b.color[0] = (static_cast<float>(rand()) / RAND_MAX); // Random red component for color.
        b.color[1] = (static_cast<float>(rand()) / RAND_MAX); // Random green component.
        b.color[2] = (static_cast<float>(rand()) / RAND_MAX); // Random blue component.
        butterflies.push_back(b); // Add the new butterfly to the vector.
    }
}

void resetAnimation() {
    y_pos = 2.0f; // Reset parachutist's Y-position.
    velocity = 0.0f; // Reset velocity.
    sway_phase = 0.0f; // Reset parachute sway.
    sway_amplitude = 0.2f; // Reset sway amplitude.
    landed = false; // Reset landed flag.
    jumped = false; // Reset jumped flag.
    plane_x = -2.5f; // Reset plane position.
    drift_x = 0.0f; // Reset horizontal drift.
    plane_stopped = false; // Reset plane stopped flag.
    post_land_sway_timer = 0.0f; // Reset post-landing timer.
    parachute_current_ry_scale = 1.0f; // Reset parachute scale.
    parachute_detaching_and_falling = false; // Reset parachute detach flag.
    parachute_relative_y_offset_after_detach = 0.15f; // Reset parachute relative offset.
    wind_strength = 1.0f; // Reset wind strength to default.

    isRunning = false; // Stop running animation.
    run_phase = 0.0f; // Reset run phase.

    initButterflies(5); // Reinitialize butterflies.
}

// Keyboard controls with wind adjustment
void keyboard(unsigned char key, int x, int y) {
    if (key == 'r' || key == 'R') { // If 'r' or 'R' is pressed.
        resetAnimation(); // Call the function to reset the animation.
    }
    else if (key == '+') { // If '+' is pressed.
        wind_strength += 0.5f; // Increase wind strength.
        if (wind_strength > 5.0f) wind_strength = 5.0f; // Cap wind strength at 5.0.
        printf("Wind Strength Increased: %.2f\n", wind_strength); // Print current wind strength to console.
    }
    else if (key == '-') { // If '-' is pressed.
        wind_strength -= 0.5f; // Decrease wind strength.
        if (wind_strength < 0.0f) wind_strength = 0.0f; // Cap wind strength at 0.0.
        printf("Wind Strength Decreased: %.2f\n", wind_strength); // Print current wind strength to console.
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h); // Sets the viewport to the entire window.
    glMatrixMode(GL_PROJECTION); // Switches to the projection matrix mode.
    glLoadIdentity(); // Resets the projection matrix.
    glOrtho(-WORLD_WIDTH / 2, WORLD_WIDTH / 2, -WORLD_HEIGHT / 2, WORLD_HEIGHT / 2, -1, 1); // Sets up an orthographic projection.
    glMatrixMode(GL_MODELVIEW); // Switches back to the modelview matrix mode.
}

int main(int argc, char** argv) {
    srand(static_cast<unsigned int>(time(0))); // Seeds the random number generator with the current time.
    glutInit(&argc, argv); // Initializes the GLUT library.
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA); // Sets display mode: double buffering, RGB colors, alpha channel.
    glutInitWindowSize(1200, 900); // Sets the initial window size.
    glutCreateWindow("Parachute Drop - Living World (Wind Control: +/-)"); // Creates the window with a title.

    glEnable(GL_BLEND); // Enables blending for transparency effects (e.g., clouds).
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Sets the blending function for alpha transparency.

    initButterflies(5); // Initializes 5 butterflies at the start.

    glutDisplayFunc(display); // Registers the 'display' function as the display callback.
    glutReshapeFunc(reshape); // Registers the 'reshape' function as the reshape callback.
    glutKeyboardFunc(keyboard); // Registers the 'keyboard' function as the keyboard callback.
    glutTimerFunc(0, update, 0); // Starts the animation timer. 'update' will be called after 0 milliseconds (first call), then every 16ms.

    glutMainLoop(); // Enters the GLUT event processing loop, starts the animation.
    return 0; // Returns 0 when the program exits.
}

// --- Parachutist Functions ---
void drawParachute(float cx_local_offset, float cy_local_offset, float rx, float ry_original) {
    float colors[1][3] = { {1,0,0} }; // Defines a red color for the parachute.
    int panels = 7; // Number of panels to draw the parachute.
    float panel_width = (2 * rx) / panels; // Width of each panel.
    float dynamic_ry = ry_original * parachute_current_ry_scale; // Calculate current vertical radius based on scale.
    if (parachute_current_ry_scale > 0.1f) { // If parachute is not too collapsed.
        dynamic_ry += ry_original * sway_amplitude * sin(sway_phase * 2.0f); // Add sway effect to the vertical radius.
    }
    float effective_parachute_draw_cy = cy_local_offset; // Effective center Y for drawing.

    if (dynamic_ry > 0.001f) { // Only draw if vertical radius is significant.
        for (int i = 0; i < panels; ++i) { // Loop to draw each panel.
            float x1_local = cx_local_offset - rx + i * panel_width; // Left X of the current panel.
            float x2_local = x1_local + panel_width; // Right X of the current panel.
            glColor3fv(colors[0]); // Set panel color to red.
            glBegin(GL_POLYGON); // Begin drawing the panel as a polygon.
            for (int j = 0; j <= 20; ++j) { // Draw curved top part of the panel.
                float t = (float)j / 20; // Parameter for interpolation along the curve.
                float x_draw = x1_local + t * (x2_local - x1_local); // Interpolate X-coordinate.
                float y_draw = effective_parachute_draw_cy + dynamic_ry * sqrt(1.0f - pow((x_draw - cx_local_offset) / rx, 2)); // Calculate Y-coordinate using ellipse equation.
                glVertex2f(x_draw, y_draw); // Set vertex.
            }
            glVertex2f(x2_local, effective_parachute_draw_cy); // Bottom-right vertex of the panel.
            glVertex2f(x1_local, effective_parachute_draw_cy); // Bottom-left vertex of the panel.
            glEnd(); // End drawing the panel.
        }
        glColor3f(0, 0, 0); // Set color to black for the outline.
        glLineWidth(1.5f); // Set line width for the outline.
        glBegin(GL_LINE_STRIP); // Begin drawing the top curved outline.
        for (int i = 0; i <= 100; ++i) { // Loop to draw points along the curve.
            float theta = M_PI * i / 100; // Angle for the semi-circle.
            glVertex2f(cx_local_offset + rx * cos(theta), effective_parachute_draw_cy + dynamic_ry * sin(theta)); // Calculate and set vertex for the outline.
        }
        glEnd(); // End drawing the outline.
    }
}

void drawRopes(float cx_local_offset, float cy_local_offset, float rx, float ry_original, float human_x_local_offset, float human_y_local_offset, float scale) {
    if (parachute_current_ry_scale > 0.1f && !parachute_detaching_and_falling) { // Only draw ropes if parachute is not too collapsed and still attached.
        glColor3f(0.4f, 0.2f, 0.1f); // Set color to brown for ropes.
        glLineWidth(2.0f); // Set line width for ropes.
        glBegin(GL_LINES); // Begin drawing lines for ropes.
        float hand_offset_x = 0.09f * scale; // Horizontal offset for human's hands.
        float hand_y_local = human_y_local_offset + 0.18f * scale; // Local Y-position of human's hands.
        float dynamic_ry = ry_original * parachute_current_ry_scale; // Current vertical radius of parachute.
        if (parachute_current_ry_scale > 0.1f) { // If parachute not too collapsed.
            dynamic_ry += ry_original * sway_amplitude * sin(sway_phase * 2.0f); // Add sway effect to parachute's vertical radius.
        }
        float effective_parachute_draw_cy = cy_local_offset; // Effective center Y for parachute.
        float attachment_x_offset_inner = rx * 0.4f; // Inner attachment point X-offset on parachute.
        float attachment_x_offset_outer = rx * 0.7f; // Outer attachment point X-offset on parachute.
        // Calculate Y-positions of attachment points on the parachute's curve.
        float y_inner_attachment_local = effective_parachute_draw_cy + dynamic_ry * sqrt(1.0f - pow((attachment_x_offset_inner) / rx, 2));
        float y_outer_attachment_local = effective_parachute_draw_cy + dynamic_ry * sqrt(1.0f - pow((attachment_x_offset_outer) / rx, 2));
        // Draw ropes from human's hands to parachute attachment points.
        glVertex2f(human_x_local_offset - hand_offset_x, hand_y_local); // Left hand.
        glVertex2f(cx_local_offset - attachment_x_offset_inner, y_inner_attachment_local); // To inner left attachment.
        glVertex2f(human_x_local_offset - hand_offset_x, hand_y_local); // Left hand again.
        glVertex2f(cx_local_offset - attachment_x_offset_outer, y_outer_attachment_local); // To outer left attachment.
        glVertex2f(human_x_local_offset + hand_offset_x, hand_y_local); // Right hand.
        glVertex2f(cx_local_offset + attachment_x_offset_inner, y_inner_attachment_local); // To inner right attachment.
        glVertex2f(human_x_local_offset + hand_offset_x, hand_y_local); // Right hand again.
        glVertex2f(cx_local_offset + attachment_x_offset_outer, y_outer_attachment_local); // To outer right attachment.
        glEnd(); // End drawing ropes.
    }
}

void drawHuman(float x_local_offset, float base_y_local_offset, float scale, bool is_running, float anim_phase) {
    float torso_w = 0.12f * scale, torso_h = 0.22f * scale, leg_h = 0.15f * scale; // Define dimensions based on scale.
    float hip_y_local = base_y_local_offset - torso_h; // Local Y-position of the hips.
    float shoulder_y_local = base_y_local_offset; // Local Y-position of the shoulders.

    // Torso
    glColor3f(0.2f, 0.2f, 0.8f); // Set color for torso (blue).
    glBegin(GL_QUADS); // Begin drawing torso as a rectangle.
    glVertex2f(x_local_offset - torso_w / 2, shoulder_y_local); // Top-left.
    glVertex2f(x_local_offset + torso_w / 2, shoulder_y_local); // Top-right.
    glVertex2f(x_local_offset + torso_w / 2, hip_y_local); // Bottom-right.
    glVertex2f(x_local_offset - torso_w / 2, hip_y_local); // Bottom-left.
    glEnd(); // End drawing torso.

    // Head
    glColor3f(1.0f, 0.85f, 0.7f); // Set skin color for head.
    drawCircle(x_local_offset, shoulder_y_local + 0.07f * scale, 0.05f * scale); // Draw circular head.

    // Arms
    glColor3f(0.2f, 0.2f, 0.8f); // Set arm color (same as torso).
    glLineWidth(3.0f); // Set line width for arms.
    if (is_running) { // If human is running.
        glPushMatrix(); // Save current matrix.
        glTranslatef(x_local_offset, shoulder_y_local, 0); // Translate to shoulder for rotation.
        glPushMatrix(); // Save matrix for left arm.
        glRotatef(40.0f * sin(anim_phase + M_PI), 0, 0, 1); // Rotate arm based on run phase (opposite of other arm).
        glBegin(GL_LINES); // Draw arm as a line.
        glVertex2f(0, 0); // Start at shoulder.
        glVertex2f(0, -leg_h * 0.9f); // End of arm.
        glEnd(); // End drawing arm.
        glPopMatrix(); // Restore matrix for left arm.
        glPushMatrix(); // Save matrix for right arm.
        glRotatef(40.0f * sin(anim_phase), 0, 0, 1); // Rotate arm based on run phase.
        glBegin(GL_LINES); // Draw arm as a line.
        glVertex2f(0, 0); // Start at shoulder.
        glVertex2f(0, -leg_h * 0.9f); // End of arm.
        glEnd(); // End drawing arm.
        glPopMatrix(); // Restore matrix for right arm.
        glPopMatrix(); // Restore initial matrix.
    }
    else { // If human is not running (falling/standing).
        glBegin(GL_LINES); // Draw arms as simple lines.
        glVertex2f(x_local_offset - torso_w / 2, shoulder_y_local); // Left arm start.
        glVertex2f(x_local_offset - torso_w / 2, shoulder_y_local + 0.1f * scale); // Left arm end (upwards).
        glVertex2f(x_local_offset + torso_w / 2, shoulder_y_local); // Right arm start.
        glVertex2f(x_local_offset + torso_w / 2, shoulder_y_local + 0.1f * scale); // Right arm end (upwards).
        glEnd(); // End drawing arms.
    }

    // Legs
    glColor3f(0.1f, 0.1f, 0.4f); // Set leg color (darker blue).
    glLineWidth(4.0f); // Set line width for legs.
    if (is_running) { // If human is running.
        glPushMatrix(); // Save current matrix.
        glTranslatef(x_local_offset, hip_y_local, 0); // Translate to hips for rotation.
        glPushMatrix(); // Save matrix for left leg.
        glRotatef(40.0f * sin(anim_phase), 0, 0, 1); // Rotate leg based on run phase.
        glBegin(GL_LINES); // Draw leg as a line.
        glVertex2f(0, 0); // Start at hip.
        glVertex2f(0, -leg_h); // End of leg.
        glEnd(); // End drawing leg.
        glPopMatrix(); // Restore matrix for left leg.
        glPushMatrix(); // Save matrix for right leg.
        glRotatef(40.0f * sin(anim_phase + M_PI), 0, 0, 1); // Rotate leg based on run phase (opposite of other leg).
        glBegin(GL_LINES); // Draw leg as a line.
        glVertex2f(0, 0); // Start at hip.
        glVertex2f(0, -leg_h); // End of leg.
        glEnd(); // End drawing leg.
        glPopMatrix(); // Restore matrix for right leg.
        glPopMatrix(); // Restore initial matrix.
    }
    else { // If human is not running.
        glBegin(GL_QUADS); // Draw legs as simple rectangles (standing pose).
        glVertex2f(x_local_offset - 0.03f * scale, hip_y_local); // Left leg top-left.
        glVertex2f(x_local_offset, hip_y_local); // Left leg top-right.
        glVertex2f(x_local_offset, hip_y_local - leg_h); // Left leg bottom-right.
        glVertex2f(x_local_offset - 0.03f * scale, hip_y_local - leg_h); // Left leg bottom-left.
        glVertex2f(x_local_offset, hip_y_local); // Right leg top-left.
        glVertex2f(x_local_offset + 0.03f * scale, hip_y_local); // Right leg top-right.
        glVertex2f(x_local_offset + 0.03f * scale, hip_y_local - leg_h); // Right leg bottom-right.
        glVertex2f(x_local_offset, hip_y_local - leg_h); // Right leg bottom-left.
        glEnd(); // End drawing legs.
    }
}