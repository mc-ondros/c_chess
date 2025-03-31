#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <fcntl.h>
#include <io.h>
#include "chess.h"

int main(void) {
    setlocale(LC_ALL, "");
    _setmode(_fileno(stdout), _O_U16TEXT);  // Enable Unicode output in Windows

    createBoard();
    printBoard();
    
    // Convert back to normal output for input handling
    _setmode(_fileno(stdout), _O_TEXT);
    
    int fromRow, fromCol, toRow, toCol;
    int playerTurn = 1; // 1 for white, 0 for black
    
    while (1) {
        printf("\n%s's turn\n", playerTurn ? "White" : "Black");
        
        if (!readMove(&fromRow, &fromCol, &toRow, &toCol)) {
            printf("Invalid input format. Use format like e2e4.\n");
            continue;
        }
        
        // Check if player is moving their own piece
        wchar_t piece = board[fromRow][fromCol];
        if ((playerTurn && !isPieceWhite(piece)) || (!playerTurn && !isPieceBlack(piece))) {
            printf("You can only move your own pieces.\n");
            continue;
        }
        
        if (!isValidMove(fromRow, fromCol, toRow, toCol)) {
            printf("Invalid move for this piece.\n");
            continue;
        }
        
        executeMove(fromRow, fromCol, toRow, toCol);
        playerTurn = !playerTurn; // Switch turns
        
        // Switch back to Unicode for board display
        _setmode(_fileno(stdout), _O_U16TEXT);
        printBoard();
        _setmode(_fileno(stdout), _O_TEXT);
    }
    
    return 0;
}
