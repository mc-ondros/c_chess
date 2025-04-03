#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "chess.h"

// Helper functions for piece identification
int isPieceWhite(wchar_t piece) {
    return piece >= white_king && piece <= white_pawn;
}

int isPieceBlack(wchar_t piece) {
    return piece >= black_king && piece <= black_pawn;
}

// Function to record a move in chess notation
void recordMove(int fromRow, int fromCol, int toRow, int toCol) {
    char moveStr[16];
    wchar_t piece = board[fromRow][fromCol];
    
    // Get piece character for notation
    char pieceChar = ' ';
    if (piece == white_rook || piece == black_rook) pieceChar = 'R';
    else if (piece == white_knight || piece == black_knight) pieceChar = 'N';
    else if (piece == white_bishop || piece == black_bishop) pieceChar = 'B';
    else if (piece == white_queen || piece == black_queen) pieceChar = 'Q';
    else if (piece == white_king || piece == black_king) pieceChar = 'K';
    
    // For pawns, we don't use a piece character in notation
    if (pieceChar == ' ') {
        sprintf(moveStr, "%c%d%c%d ", 
                'a' + fromCol, fromRow + 1, 
                'a' + toCol, toRow + 1);
    } else {
        sprintf(moveStr, "%c%c%d%c%d ", 
                pieceChar, 'a' + fromCol, fromRow + 1, 
                'a' + toCol, toRow + 1);
    }
    
    // Append the move to the history
    strcat(moveHistory, moveStr);
}

// Print the move history
void printMoveHistory() {
    printf("Move History: %s\n", moveHistory);
}

// Read a move from the command line (format: e2e4)
int readMove(int *fromRow, int *fromCol, int *toRow, int *toCol) {
    char input[5];
    printf("Enter move (e.g., e2e4): ");
    if (scanf("%4s", input) != 1) {
        return 0;
    }
    if(strcmp(input, "exit")==0) {
        printf("Exiting...\n");
        exit(0);
    }
    if (strcmp(input, "save") == 0) {
            char filename[100];
            printf("Enter filename to save: ");
            scanf("%99s", filename);
            saveGame(filename);
        exit(0);
        }
    if (strcmp(input, "load") == 0) {
        char filename[100];
        printf("Enter filename to load: ");
        scanf("%99s", filename);
        if (loadGame(filename)) {
            printf("Game loaded\n");
        }
    }
    // Check if input is in the correct format
    if (strlen(input) != 4) {
        return 0;
    }
    
    // Convert chess notation to array indices
    *fromCol = tolower(input[0]) - 'a';
    *fromRow = input[1] - '1';
    *toCol = tolower(input[2]) - 'a';
    *toRow = input[3] - '1';
    
    // Check if indices are valid
    if (*fromCol < 0 || *fromCol > 7 || *fromRow < 0 || *fromRow > 7 ||
        *toCol < 0 || *toCol > 7 || *toRow < 0 || *toRow > 7) {
        return 0;
    }
    return 1;
}

// Check if path is clear for pieces that move in straight lines
int isPathClear(int fromRow, int fromCol, int toRow, int toCol) {
    int rowStep = 0, colStep = 0;
    
    if (fromRow < toRow) rowStep = 1;
    else if (fromRow > toRow) rowStep = -1;
    
    if (fromCol < toCol) colStep = 1;
    else if (fromCol > toCol) colStep = -1;
    
    int row = fromRow + rowStep;
    int col = fromCol + colStep;
    
    while (row != toRow || col != toCol) {
        if (board[row][col] != 0) {
            return 0; // Path is blocked
        }
        row += rowStep;
        col += colStep;
    }
    
    return 1; // Path is clear
}

// Check pawn move validity
int isPawnMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol) {
    int direction = (piece == white_pawn) ? 1 : -1;
    int startRow = (piece == white_pawn) ? 1 : 6;
    
    // Forward move (1 square)
    if (fromCol == toCol && toRow == fromRow + direction && board[toRow][toCol] == 0) {
        return 1;
    }
    
    // Forward move (2 squares from starting position)
    if (fromCol == toCol && fromRow == startRow && toRow == fromRow + 2 * direction && 
        board[fromRow + direction][fromCol] == 0 && board[toRow][toCol] == 0) {
        return 1;
    }
    
    // Capture move
    if (abs(fromCol - toCol) == 1 && toRow == fromRow + direction) {
        wchar_t targetPiece = board[toRow][toCol];
        if ((piece == white_pawn && isPieceBlack(targetPiece)) || 
            (piece == black_pawn && isPieceWhite(targetPiece))) {
            return 1;
        }
    }
    
    return 0;
}

// Check rook move validity
int isRookMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol) {
    // Rook moves in straight lines (horizontally or vertically)
    if (fromRow != toRow && fromCol != toCol) {
        return 0;
    }
    
    return isPathClear(fromRow, fromCol, toRow, toCol);
}

// Check knight move validity
int isKnightMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol) {
    // Knight moves in L shape (2 in one direction, 1 in perpendicular direction)
    int rowDiff = abs(fromRow - toRow);
    int colDiff = abs(fromCol - toCol);
    
    return (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);
}

// Check bishop move validity
int isBishopMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol) {
    // Bishop moves diagonally
    if (abs(fromRow - toRow) != abs(fromCol - toCol)) {
        return 0;
    }
    
    return isPathClear(fromRow, fromCol, toRow, toCol);
}

// Check queen move validity
int isQueenMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol) {
    // Queen can move like a rook or bishop
    return isRookMove(piece, fromRow, fromCol, toRow, toCol) || 
           isBishopMove(piece, fromRow, fromCol, toRow, toCol);
}

// Check king move validity
int isKingMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol) {
    // King moves one square in any direction
    return abs(fromRow - toRow) <= 1 && abs(fromCol - toCol) <= 1;
}

// Validate if a move is legal
int isValidMove(int fromRow, int fromCol, int toRow, int toCol) {
    wchar_t piece = board[fromRow][fromCol];
    wchar_t targetPiece = board[toRow][toCol];
    
    // Check if there is a piece to move
    if (piece == 0) {
        printf("No piece at starting position.\n");
        return 0;
    }
    
    // Check if destination has a piece of the same color
    if ((isPieceWhite(piece) && isPieceWhite(targetPiece)) || 
        (isPieceBlack(piece) && isPieceBlack(targetPiece))) {
        printf("Cannot capture own piece.\n");
        return 0;
    }
    
    // Check if move is valid based on piece type
    if (piece == white_pawn || piece == black_pawn) {
        return isPawnMove(piece, fromRow, fromCol, toRow, toCol);
    } else if (piece == white_rook || piece == black_rook) {
        return isRookMove(piece, fromRow, fromCol, toRow, toCol);
    } else if (piece == white_knight || piece == black_knight) {
        return isKnightMove(piece, fromRow, fromCol, toRow, toCol);
    } else if (piece == white_bishop || piece == black_bishop) {
        return isBishopMove(piece, fromRow, fromCol, toRow, toCol);
    } else if (piece == white_queen || piece == black_queen) {
        return isQueenMove(piece, fromRow, fromCol, toRow, toCol);
    } else if (piece == white_king || piece == black_king) {
        return isKingMove(piece, fromRow, fromCol, toRow, toCol);
    }
    
    return 0;
}

// Execute the move on the board
void executeMove(int fromRow, int fromCol, int toRow, int toCol) {
    // Record the move before execution
    recordMove(fromRow, fromCol, toRow, toCol);
    
    board[toRow][toCol] = board[fromRow][fromCol];
    board[fromRow][fromCol] = 0;
    
    // Check for pawn promotion (simplified version)
    if (board[toRow][toCol] == white_pawn && toRow == 7) {
        board[toRow][toCol] = white_queen;  // Automatically promote to queen
        printf("Pawn promoted to Queen!\n");
    } else if (board[toRow][toCol] == black_pawn && toRow == 0) {
        board[toRow][toCol] = black_queen;  // Automatically promote to queen
        printf("Pawn promoted to Queen!\n");
    }
}

