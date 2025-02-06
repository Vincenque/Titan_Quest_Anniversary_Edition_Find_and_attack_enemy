#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main() {
	printf("Press and hold 'A' to keep pressing the left mouse button.\n");
	printf("Release 'A' to stop. Press ESC to exit.\n");

	int toggle = 0;
	while (1) {
		if (GetAsyncKeyState(VK_ESCAPE)) {
			break; // Exit the loop if ESC is pressed
		}

		if (GetAsyncKeyState('A')) {
			if (toggle) {
				// Move the cursor to the first position
				SetCursorPos(1920 / 2 + 100, 1080 / 2 + 80);
			} else {
				// Move the cursor to the second position
				SetCursorPos(1920 / 2 - 100, 1080 / 2 + 80);
			}

			// Simulate left mouse button press
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

			toggle = !toggle; // Toggle between positions
		}

		Sleep(10); // Small delay to avoid excessive CPU usage
	}

	return 0;
}