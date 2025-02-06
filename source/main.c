#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define TARGET_X 906
#define TARGET_Y 62
#define TARGET_R 214
#define TARGET_G 65
#define TARGET_B 37
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define INITIAL_RADIUS 100
#define STEP_INCREMENT 50
#define MAX_RADIUS 500
#define STEPS 5

COLORREF getPixelColor(int x, int y) {
    HDC hdc = GetDC(NULL);
    COLORREF color = GetPixel(hdc, x, y);
    ReleaseDC(NULL, hdc);
    return color;
}

void holdLeftMouseButton() {
    printf("Holding left mouse button...\n");
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    while (1) {
        if (!GetAsyncKeyState('A')) break; // Stop if 'A' is released
        COLORREF color = getPixelColor(TARGET_X, TARGET_Y);
        if (GetRValue(color) != TARGET_R || GetGValue(color) != TARGET_G || GetBValue(color) != TARGET_B) break; // Stop if color changes
        Sleep(10);
    }
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    printf("Released left mouse button.\n");
}

int main() {
    printf("Moving mouse until pixel at (906,62) is RGB=(214,65,37)\n");

    int angle = 0;
    int centerX = SCREEN_WIDTH / 2;
    int centerY = SCREEN_HEIGHT / 2;
    int radius = INITIAL_RADIUS;

    while (1) {
        if (GetAsyncKeyState(VK_ESCAPE)) {
            break; // Exit the loop if ESC is pressed
        }

        if (GetAsyncKeyState('A')) {
            // Get pixel color at the target location
            COLORREF color = getPixelColor(TARGET_X, TARGET_Y);
            int r = GetRValue(color);
            int g = GetGValue(color);
            int b = GetBValue(color);
            printf("Pixel at (%d,%d): RGB=(%d,%d,%d)\n", TARGET_X, TARGET_Y, r, g, b);

            // Check if the color matches the target
            if (r == TARGET_R && g == TARGET_G && b == TARGET_B) {
                printf("Target color reached. Holding left mouse button.\n");
                holdLeftMouseButton();
            }

            // Calculate new mouse position in circular motion
            int x = centerX + (int)(radius * cos(angle * (3.14159265 / 180)));
            int y = centerY + (int)(radius * sin(angle * (3.14159265 / 180)));
            SetCursorPos(x, y);

            // Increment angle for next position
            angle = (angle + 360 / STEPS) % 360;

            // If a full circle is completed and target is not found, increase radius
            if (angle == 0) {
                if (radius < MAX_RADIUS) {
                    printf("Incrementing radius.\n");
                    radius += STEP_INCREMENT;
                } else {
                    printf("Resetting radius.\n");
                    radius = INITIAL_RADIUS;
                }
            }

            Sleep(10); // Small delay to prevent excessive CPU usage
        }

        Sleep(10); // Small delay to avoid excessive CPU usage
    }

    return 0;
}