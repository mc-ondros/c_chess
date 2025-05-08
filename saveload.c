#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "chess.h"

// Save the game in PGN format
int saveGame(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not create PGN file %s\n", filename);
        return 0;
    }

    // Write minimal PGN headers
    fprintf(file, "[Event \"C Chess Game\"]\n");
    fprintf(file, "[Site \"Local\"]\n");
    fprintf(file, "[Date \"????.??.??\"]\n");
    fprintf(file, "[Round \"-\"]\n");
    fprintf(file, "[White \"Player1\"]\n");
    fprintf(file, "[Black \"Player2\"]\n");
    fprintf(file, "\n");

    // Write move history as a single line (PGN main line)
    // Assume moveHistory is already in SAN or coordinate notation separated by spaces
    // Optionally, add move numbers
    int moveNum = 1;
    char copy[4096];
    strcpy(copy, moveHistory);
    char* token = strtok(copy, " ");
    int isWhite = 1;
    while (token) {
        if (isWhite) {
            fprintf(file, "%d. %s ", moveNum, token);
        } else {
            fprintf(file, "%s ", token);
            moveNum++;
        }
        isWhite = !isWhite;
        token = strtok(NULL, " ");
    }
    fprintf(file, "\n");
    fclose(file);
    printf("Game saved as PGN to %s\n", filename);
    return 1;
}

// Helper: Reset board and replay moves from moveHistory
static void replayMoves(const char* moves) {
    createBoard();
    // moveHistory will be rebuilt
    moveHistory[0] = '\0';
    char copy[4096];
    strcpy(copy, moves);
    char* token = strtok(copy, " ");
    while (token) {
        // Skip move numbers and dots
        if (isdigit(token[0]) && token[strlen(token)-1] == '.') {
            token = strtok(NULL, " ");
            continue;
        }
        // Parse move in coordinate notation (e.g., e2e4 or Ne2e4)
        int len = strlen(token);
        int fromCol, fromRow, toCol, toRow;
        if (len == 4 && isalpha(token[0]) && isdigit(token[1]) && isalpha(token[2]) && isdigit(token[3])) {
            fromCol = tolower(token[0]) - 'a';
            fromRow = token[1] - '1';
            toCol = tolower(token[2]) - 'a';
            toRow = token[3] - '1';
        } else if (len == 5 && isupper(token[0]) && isalpha(token[1]) && isdigit(token[2]) && isalpha(token[3]) && isdigit(token[4])) {
            // Piece letter present (e.g., Ne2e4)
            fromCol = tolower(token[1]) - 'a';
            fromRow = token[2] - '1';
            toCol = tolower(token[3]) - 'a';
            toRow = token[4] - '1';
        } else {
            // Not a move, skip
            token = strtok(NULL, " ");
            continue;
        }
        if (isValidMove(fromRow, fromCol, toRow, toCol)) {
            executeMove(fromRow, fromCol, toRow, toCol);
        }
        token = strtok(NULL, " ");
    }
}

int loadGame(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open PGN file %s\n", filename);
        return 0;
    }
    char line[4096];
    char moves[4096] = "";
    // Skip headers
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '[') continue;
        // Concatenate move lines
        strcat(moves, line);
    }
    fclose(file);

    // Remove comments, result, and newlines
    char cleanMoves[4096] = "";
    int j = 0;
    for (int i = 0; moves[i]; ++i) {
        if (moves[i] == '{' || moves[i] == '(') {
            // Skip comments/variations
            while (moves[i] && moves[i] != '}' && moves[i] != ')') ++i;
            continue;
        }
        if (moves[i] == '\n' || moves[i] == '\r') continue;
        if (moves[i] == '1' && strncmp(&moves[i], "1-0", 3) == 0) break;
        if (moves[i] == '0' && strncmp(&moves[i], "0-1", 3) == 0) break;
        if (moves[i] == '1' && strncmp(&moves[i], "1/2-1/2", 7) == 0) break;
        cleanMoves[j++] = moves[i];
    }
    cleanMoves[j] = '\0';

    // Replay moves to reconstruct board and moveHistory
    replayMoves(cleanMoves);

    printf("Game loaded from PGN %s\n", filename);
    return 1;
}
