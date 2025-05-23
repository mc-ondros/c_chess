#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <fcntl.h>
#include <io.h>
#include <stdbool.h>
#include "chess.h"
#include "saveload.h"
#include "api.h"
#include "gui.h"

// ...existing declarations and includes...

int main(void) {
    setlocale(LC_ALL, "");
    _setmode(_fileno(stdout), _O_U16TEXT); // Enable Unicode output in Windows

    // Set the API key - replace this with your actual API key
    setApiKey("AIzaSyCVA-8dRIBWBXtu2EenqWVpqEVSjDkaNAU");

    // Game mode selection: now with three options
    int gameMode = 0;
    _setmode(_fileno(stdout), _O_TEXT);
    printf("Select game mode:\n");
    printf("1. One-Player (vs Gemini AI)\n");
    printf("2. Two-Player\n");
    printf("3. One-Player (vs Local CPU)\n");
    printf("Your choice: ");
    scanf("%d", &gameMode);
    while (gameMode < 1 || gameMode > 3) {
        printf("Invalid option. Please enter 1, 2, or 3: ");
        scanf("%d", &gameMode);
    }
    while(getchar() != '\n'); // Clear input buffer

#ifdef NO_CURL_SUPPORT
    _setmode(_fileno(stdout), _O_TEXT);
    printf("NOTE: Building without API support. Black moves will use a simple fallback AI.\n");
    printf("To enable API support, install libcurl development files and rebuild.\n");
    _setmode(_fileno(stdout), _O_U16TEXT);
#else
    _setmode(_fileno(stdout), _O_TEXT);
    if (gameMode == 1) {
        printf("API support enabled. Black will use the Gemini API for moves.\n");
        printf("If you see errors, check your API key in main.c\n\n");

        char personality[512];
        printf("Enter Gemini personality for the black player (e.g., 'aggressive', 'cautious'):\n");
        fgets(personality, sizeof(personality), stdin);
        size_t len = strlen(personality);
        if (len > 0 && personality[len - 1] == '\n') {
            personality[len - 1] = '\0';
        }
        setAiPersonality(personality);
        printf("Black player set to '%s' personality.\n", personality);
    } else if (gameMode == 2) {
        printf("Two-Player mode selected. Both moves will be input by players.\n\n");
        printf("Players must alternate turns - White goes first.\n");
    } else if (gameMode == 3) {
        printf("Local CPU mode selected. Black will use a minimax-based local engine.\n\n");
    }
    _setmode(_fileno(stdout), _O_U16TEXT);
#endif

    // Initialize game state and launch the GUI with the selected game mode
    createBoard();
    initBitboards();
    startGui(gameMode);

    // Removed console-based game loop and board printing.
    return 0;
}
