#include <wchar.h>
#include <stdio.h>
#include "chess.h"

// Global board declaration (defined in chess.h as extern)
wchar_t board[8][8];

void createBoard() {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if (i == 0) {
                if (j == 0 || j == 7) board[i][j] = white_rook;
                if (j == 1 || j == 6) board[i][j] = white_knight;
                if (j == 2 || j == 5) board[i][j] = white_bishop;
                if (j == 3) board[i][j] = white_queen;
                if (j == 4) board[i][j] = white_king;
            }

            if (i == 1) board[i][j] = white_pawn;

            if (i > 1 && i < 6) board[i][j] = 0;

            if (i == 6) board[i][j] = black_pawn;
            if (i == 7) {
                if(j == 0 || j == 7) board[i][j] = black_rook;
                if(j == 1 || j == 6) board[i][j] = black_knight;
                if(j == 2 || j == 5) board[i][j] = black_bishop;
                if(j == 3) board[i][j] = black_queen;
                if(j == 4) board[i][j] = black_king;
            }
        }
    }
}

void printBoard() {
    wprintf(L"  a b c d e f g h\n");
    wprintf(L"  ---------------\n");
    for (int i = 7; i >= 0; i--) {
        wprintf(L"%d|", i + 1);
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == 0) {
                wprintf(L"  ");
            } else {
                wprintf(L"%lc ", board[i][j]);
            }
        }
        wprintf(L"\n");
    }
}
