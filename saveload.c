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

    // Write standard PGN headers
    fprintf(file, "[Event \"C Chess Game\"]\n");
    fprintf(file, "[Site \"Local\"]\n");
    fprintf(file, "[Date \"????.??.??\"]\n");
    fprintf(file, "[Round \"1\"]\n");
    fprintf(file, "[White \"Player1\"]\n");
    fprintf(file, "[Black \"Player2\"]\n");
    fprintf(file, "[Result \"*\"]\n");
    fprintf(file, "\n");

    // Write move history as PGN main line with move numbers and line breaks every 10 moves
    int moveNum = 1;
    char copy[4096];
    strcpy(copy, moveHistory);
    char* token = strtok(copy, " ");
    int isWhite = 1;
    int movesOnLine = 0;
    while (token) {
        if (isWhite) {
            fprintf(file, "%d. %s ", moveNum, token);
        } else {
            fprintf(file, "%s ", token);
            moveNum++;
        }
        isWhite = !isWhite;
        movesOnLine++;
        if (movesOnLine >= 10) {
            fprintf(file, "\n");
            movesOnLine = 0;
        }
        token = strtok(NULL, " ");
    }
    // Write result marker at the end
    fprintf(file, "*\n");
    fclose(file);
    printf("Game saved as PGN to %s\n", filename);
    return 1;
}

// Helper: Convert SAN move to coordinates (returns 1 if valid, 0 if not)
static int sanToCoords(const char *san, int *fromRow, int *fromCol, int *toRow, int *toCol) {
    // Handle castling
    if (strcmp(san, "O-O") == 0 || strcmp(san, "0-0") == 0) {
        // Kingside castling
        // White or black: find king and rook
        for (int row = 0; row < 8; ++row) {
            for (int col = 4; col <= 4; ++col) {
                wchar_t piece = board[row][col];
                if (piece == white_king && row == 0) {
                    *fromRow = 0; *fromCol = 4; *toRow = 0; *toCol = 6;
                    return 1;
                }
                if (piece == black_king && row == 7) {
                    *fromRow = 7; *fromCol = 4; *toRow = 7; *toCol = 6;
                    return 1;
                }
            }
        }
    }
    if (strcmp(san, "O-O-O") == 0 || strcmp(san, "0-0-0") == 0) {
        // Queenside castling
        for (int row = 0; row < 8; ++row) {
            for (int col = 4; col <= 4; ++col) {
                wchar_t piece = board[row][col];
                if (piece == white_king && row == 0) {
                    *fromRow = 0; *fromCol = 4; *toRow = 0; *toCol = 2;
                    return 1;
                }
                if (piece == black_king && row == 7) {
                    *fromRow = 7; *fromCol = 4; *toRow = 7; *toCol = 2;
                    return 1;
                }
            }
        }
    }

    // Remove check/mate symbols and promotion (e.g., e8=Q+)
    char move[16];
    strncpy(move, san, sizeof(move) - 1);
    move[sizeof(move) - 1] = 0;
    char *plus = strchr(move, '+'); if (plus) *plus = 0;
    char *hash = strchr(move, '#'); if (hash) *hash = 0;
    char *eq = strchr(move, '='); if (eq) *eq = 0;

    // Remove trailing punctuation (e.g., "e4!", "e4?")
    int len = strlen(move);
    while (len > 0 && (move[len-1] == '!' || move[len-1] == '?')) move[--len] = 0;

    // Handle pawn moves like "e4", "exd5"
    if ((len == 2 && move[0] >= 'a' && move[0] <= 'h' && move[1] >= '1' && move[1] <= '8') ||
        (len >= 4 && move[1] == 'x')) {
        int destCol = move[len-2] - 'a';
        int destRow = move[len-1] - '1';
        // Find pawn that can move to dest
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                wchar_t piece = board[row][col];
                if (piece == white_pawn || piece == black_pawn) {
                    if (isValidMove(row, col, destRow, destCol)) {
                        *fromRow = row; *fromCol = col; *toRow = destRow; *toCol = destCol;
                        return 1;
                    }
                }
            }
        }
        return 0;
    }

    // Handle piece moves: Nf3, Rxa8, Qh5, etc.
    char pieceChar = 0;
    int srcCol = -1, srcRow = -1;
    int destCol = -1, destRow = -1;
    int idx = 0;
    if (move[0] >= 'A' && move[0] <= 'Z' && move[0] != 'O') {
        pieceChar = move[0];
        idx = 1;
    }
    // Disambiguation (e.g., Nbd2, R1e1)
    if (move[idx] >= 'a' && move[idx] <= 'h') {
        srcCol = move[idx] - 'a';
        idx++;
    } else if (move[idx] >= '1' && move[idx] <= '8') {
        srcRow = move[idx] - '1';
        idx++;
    }
    if (move[idx] == 'x') idx++; // capture
    if (move[idx] >= 'a' && move[idx] <= 'h' && move[idx+1] >= '1' && move[idx+1] <= '8') {
        destCol = move[idx] - 'a';
        destRow = move[idx+1] - '1';
    } else {
        return 0;
    }
    // Find matching piece
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            wchar_t piece = board[row][col];
            if (pieceChar) {
                if ((pieceChar == 'N' && (piece == white_knight || piece == black_knight)) ||
                    (pieceChar == 'B' && (piece == white_bishop || piece == black_bishop)) ||
                    (pieceChar == 'R' && (piece == white_rook || piece == black_rook)) ||
                    (pieceChar == 'Q' && (piece == white_queen || piece == black_queen)) ||
                    (pieceChar == 'K' && (piece == white_king || piece == black_king))) {
                    if ((srcCol == -1 || col == srcCol) && (srcRow == -1 || row == srcRow)) {
                        if (isValidMove(row, col, destRow, destCol)) {
                            *fromRow = row; *fromCol = col; *toRow = destRow; *toCol = destCol;
                            return 1;
                        }
                    }
                }
            }
        }
    }
    // Fallback: try all pieces for this destination
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            wchar_t piece = board[row][col];
            if (!pieceChar || (pieceChar == 'N' && (piece == white_knight || piece == black_knight)) ||
                (pieceChar == 'B' && (piece == white_bishop || piece == black_bishop)) ||
                (pieceChar == 'R' && (piece == white_rook || piece == black_rook)) ||
                (pieceChar == 'Q' && (piece == white_queen || piece == black_queen)) ||
                (pieceChar == 'K' && (piece == white_king || piece == black_king))) {
                if ((srcCol == -1 || col == srcCol) && (srcRow == -1 || row == srcRow)) {
                    if (isValidMove(row, col, destRow, destCol)) {
                        *fromRow = row; *fromCol = col; *toRow = destRow; *toCol = destCol;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

// Helper: Reset board and replay moves from moveHistory or PGN SAN moves
static void replayMoves(const char* moves) {
    createBoard();
    // Reset all state variables
    moveHistory[0] = '\0';
    whiteKingMoved = whiteKingRookMoved = whiteQueenRookMoved = 0;
    blackKingMoved = blackKingRookMoved = blackQueenRookMoved = 0;
    enPassantTargetRow = enPassantTargetCol = -1;
    fiftyMoveCounter = 0;
    repetitionCount = 0;

    char copy[4096];
    strncpy(copy, moves, sizeof(copy)-1);
    copy[sizeof(copy)-1] = '\0';
    char* token = strtok(copy, " \t\r\n");
    while (token) {
        // Skip move numbers and results
        if (isdigit(token[0]) && (strchr(token, '.') || strlen(token) <= 3)) {
            token = strtok(NULL, " \t\r\n");
            continue;
        }
        if (strcmp(token, "1-0") == 0 || strcmp(token, "0-1") == 0 || strcmp(token, "1/2-1/2") == 0 || strcmp(token, "*") == 0) {
            break;
        }
        // Skip comments and empty tokens
        if (token[0] == '{' || token[0] == '(' || token[0] == '[') {
            token = strtok(NULL, " \t\r\n");
            continue;
        }
        // Try to parse SAN move
        int fromRow, fromCol, toRow, toCol;
        if (sanToCoords(token, &fromRow, &fromCol, &toRow, &toCol)) {
            executeMove(fromRow, fromCol, toRow, toCol);
        }
        token = strtok(NULL, " \t\r\n");
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
    // Skip headers and collect move text
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '[') continue;
        // Remove comments in curly braces or parentheses
        char *p = line;
        while (*p) {
            if (*p == '{' || *p == '(') {
                while (*p && *p != '}' && *p != ')') ++p;
                if (*p) ++p;
            } else {
                strncat(moves, p, 1);
                ++p;
            }
        }
    }
    fclose(file);

    // Remove newlines and extra whitespace
    char cleanMoves[4096] = "";
    int j = 0;
    for (int i = 0; moves[i]; ++i) {
        if (moves[i] == '\n' || moves[i] == '\r') continue;
        cleanMoves[j++] = moves[i];
    }
    cleanMoves[j] = '\0';

    // Replay moves to reconstruct board and moveHistory
    replayMoves(cleanMoves);

    printf("Game loaded from PGN %s\n", filename);
    return 1;
}
