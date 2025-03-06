#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <fcntl.h>
#include <io.h>
#define white_king   0x2654 // ♔
#define white_queen  0x2655 // ♕
#define white_rook   0x2656 // ♖
#define white_bishop 0x2657 // ♗
#define white_knight 0x2658 // ♘
#define white_pawn   0x2659 // ♙
#define black_king   0x265A // ♚
#define black_queen  0x265B // ♛
#define black_rook   0x265C // ♜
#define black_bishop 0x265D // ♝
#define black_knight 0x265E // ♞
#define black_pawn   0x265F // ♟

wchar_t board[8][8];

void createBoard() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
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
                if (j == 0 || j == 7) board[i][j] = black_rook;
                if (j == 1 || j == 6) board[i][j] = black_knight;
                if (j == 2 || j == 5) board[i][j] = black_bishop;
                if (j == 3) board[i][j] = black_queen;
                if (j == 4) board[i][j] = black_king;
            }
        }
    }
}

void printBoard() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            wprintf(L"%lc ", board[i][j]);
        }
        wprintf(L"\n");
    }
}

int main(void) {
    setlocale(LC_ALL, "");
    _setmode(_fileno(stdout), _O_U16TEXT);  // Enable Unicode output in Windows

createBoard();
    printBoard();
    return 0;
}
