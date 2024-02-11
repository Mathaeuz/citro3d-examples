#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include "exercises.h"

int main()
{
	// Initialize graphics
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);
	const int exerciseCount = getExerciseCount();
	int selection = 0;

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();
		printf("\x1b[1;1H");
		printf("<< or >> Choose\n");
		printf("A to Confirm\n");
		printf("START to quit\n");
		printf("\n%s\n", getExerciseName(selection));

		// Respond to user input
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		if (kDown & KEY_LEFT)
			selection -= 1;

		if (kDown & KEY_RIGHT)
			selection += 1;

		selection = (selection + exerciseCount) % exerciseCount;

		if (kDown & KEY_A)
		{
			consoleClear();
			runExercise(selection);
			gfxInitDefault();
			consoleInit(GFX_BOTTOM, NULL);
		}
	}
	consoleClear();
	gfxExit();
	return 0;
}