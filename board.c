#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include "chess.h"

// The board array
wchar_t board[8][8];

// Move history
char moveHistory[4096] = {0};

// Function to initialize the chess board with pieces
void createBoard() {
    // Initialize the entire board to empty
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            board[row][col] = 0;
        }
    }

    // Set up pawns
    for (int col = 0; col < 8; col++) {
        board[1][col] = white_pawn;
        board[6][col] = black_pawn;
    }

    // Set up other pieces
    board[0][0] = board[0][7] = white_rook;
    board[0][1] = board[0][6] = white_knight;
    board[0][2] = board[0][5] = white_bishop;
    board[0][3] = white_queen;
    board[0][4] = white_king;

    board[7][0] = board[7][7] = black_rook;
    board[7][1] = board[7][6] = black_knight;
    board[7][2] = board[7][5] = black_bishop;
    board[7][3] = black_queen;
    board[7][4] = black_king;
}

// Function to print the chess board with borders and aligned rows/columns
void printBoard() {
    // Print the column labels on top
    wprintf(L"     a   b   c   d   e   f   g   h  \n");
    wprintf(L"   ┌───┬───┬───┬───┬───┬───┬───┬───┐\n");

    // Print each row
    for (int row = 7; row >= 0; row--) {
        wprintf(L" %d │", row + 1); // Row number and left border

        for (int col = 0; col < 8; col++) {
            wchar_t piece = board[row][col];
            if (piece) {
                wprintf(L" %lc ", piece);
            } else {
                wprintf(L" %lc ");
            }
            if (col < 7) {
                wprintf(L"│");
            } else {
                wprintf(L"│\n");
            }
        }

        if (row > 0) {
            wprintf(L"   ├───┼───┼───┼───┼───┼───┼───┼───┤\n");
        } else {
            wprintf(L"   └───┴───┴───┴───┴───┴───┴───┴───┘\n");
        }
    }
    wprintf(L"     a   b   c   d   e   f   g   h  \n");
}

