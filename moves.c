#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "chess.h"

// Add global flags for GUI notifications of special moves and states
int enPassantCaptureExecuted = 0;
int castlingExecuted = 0;
int promotionExecuted = 0;
int checkFlag = 0;
int checkMateFlag = 0;
int stalemateFlag = 0;

// 50-move rule counter
int fiftyMoveCounter = 0;

// Helper functions for piece identification
int isPieceWhite(wchar_t piece) {
    return piece >= white_king && piece <= white_pawn;
}

int isPieceBlack(wchar_t piece) {
    return piece >= black_king && piece <= black_pawn;
}

// New helper: check if a square is attacked by opponent pieces.
// defenderIsWhite indicates the color of the king that would occupy the square.
static int isSquareAttacked(int row, int col, int defenderIsWhite) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            wchar_t piece = board[i][j];
            if (piece == 0) continue;
            // Only consider opponent pieces.
            if (defenderIsWhite && isPieceBlack(piece)) {
                // Use isValidMove but skip castling (assume king moves only one square).
                if ( (abs(i - row) <= 1 && abs(j - col) <= 1 && piece == black_king) ||
                     (piece != black_king && isValidMove(i, j, row, col)) ) {
                    return 1;
                }
            } else if (!defenderIsWhite && isPieceWhite(piece)) {
                if ( (abs(i - row) <= 1 && abs(j - col) <= 1 && piece == white_king) ||
                     (piece != white_king && isValidMove(i, j, row, col)) ) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Function to check for 50-move rule draw
int isFiftyMoveRuleDraw() {
    return fiftyMoveCounter >= 100;
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

// Modified pawn move validity function to support en passant capture
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

    // Capture move (including en passant)
    if (abs(fromCol - toCol) == 1 && toRow == fromRow + direction) {
        wchar_t targetPiece = board[toRow][toCol];
        // Normal capture
        if ((piece == white_pawn && isPieceBlack(targetPiece)) ||
            (piece == black_pawn && isPieceWhite(targetPiece))) {
            return 1;
        }
        // En passant capture: if target square is empty and matches enPassant target
        if (targetPiece == 0 &&
            toRow == enPassantTargetRow && toCol == enPassantTargetCol) {
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
    int rowDiff = abs(fromRow - toRow);
    int colDiff = abs(fromCol - toCol);
    // Normal one-square king move
    if(rowDiff <= 1 && colDiff <= 1)
        return 1;
    
    // Castling move (king moves two squares horizontally on its starting row)
    if(rowDiff == 0 && colDiff == 2) {
         if(piece == white_king && fromRow == 0) {
              if(toCol == 6 && !whiteKingMoved && !whiteKingRookMoved &&
                 board[0][5] == 0 && board[0][6] == 0 && board[0][7] == white_rook) {
                     // For kingside, ensure squares f1 and g1 are not attacked.
                     if(isSquareAttacked(0, 5, 1) || isSquareAttacked(0, 6, 1))
                         return 0;
                     return 1;
              }
              else if(toCol == 2 && !whiteKingMoved && !whiteQueenRookMoved &&
                      board[0][3] == 0 && board[0][2] == 0 && board[0][1] == 0 && board[0][0] == white_rook) {
                     // For queenside, ensure squares d1 and c1 are not attacked.
                     if(isSquareAttacked(0, 3, 1) || isSquareAttacked(0, 2, 1))
                         return 0;
                     return 1;
              }
         } else if(piece == black_king && fromRow == 7) {
              if(toCol == 6 && !blackKingMoved && !blackKingRookMoved &&
                 board[7][5] == 0 && board[7][6] == 0 && board[7][7] == black_rook) {
                     // For kingside, ensure squares f8 and g8 are not attacked.
                     if(isSquareAttacked(7, 5, 0) || isSquareAttacked(7, 6, 0))
                         return 0;
                     return 1;
              }
              else if(toCol == 2 && !blackKingMoved && !blackQueenRookMoved &&
                      board[7][3] == 0 && board[7][2] == 0 && board[7][1] == 0 && board[7][0] == black_rook) {
                     // For queenside, ensure squares d8 and c8 are not attacked.
                     if(isSquareAttacked(7, 3, 0) || isSquareAttacked(7, 2, 0))
                         return 0;
                     return 1;
              }
         }
    }
    return 0;
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

    // Store the piece being moved and the original destination
    wchar_t movedPiece = board[fromRow][fromCol];
    wchar_t targetPiece = board[toRow][toCol];

    // --- 50-move rule logic ---
    // Reset counter if pawn move or capture, else increment
    if ((movedPiece == white_pawn || movedPiece == black_pawn) || (targetPiece != 0)) {
        fiftyMoveCounter = 0;
    } else {
        fiftyMoveCounter++;
    }

    // For pawns: set en passant target if moving two squares, else reset targets
    if((movedPiece == white_pawn && (toRow - fromRow) == 2) ||
       (movedPiece == black_pawn && (fromRow - toRow) == 2)) {
         enPassantTargetRow = (fromRow + toRow) / 2;
         enPassantTargetCol = fromCol;
    } else {
         enPassantTargetRow = -1;
         enPassantTargetCol = -1;
    }

    // Execute the move
    board[toRow][toCol] = movedPiece;
    board[fromRow][fromCol] = 0;

    // Handle en passant capture:
    // If a pawn moves diagonally into an empty square (i.e. targetPiece was 0),
    // capture the opponent's pawn located just behind the destination.
    if((movedPiece == white_pawn || movedPiece == black_pawn) &&
       (abs(fromCol - toCol) == 1) && targetPiece == 0) {
         if(movedPiece == white_pawn) {
              board[toRow - 1][toCol] = 0;
         } else if(movedPiece == black_pawn) {
              board[toRow + 1][toCol] = 0;
         }
         printf("En passant capture executed!\n");
         enPassantCaptureExecuted = 1;
    }

    // Handle castling: if the king moves two squares horizontally, move the rook accordingly.
    if(movedPiece == white_king && abs(fromCol - toCol) == 2 && fromRow == 0) {
         if(toCol == 6) {  // White kingside castling
              board[0][5] = board[0][7];
              board[0][7] = 0;
              whiteKingRookMoved = 1;
         } else if(toCol == 2) {  // White queenside castling
              board[0][3] = board[0][0];
              board[0][0] = 0;
              whiteQueenRookMoved = 1;
         }
         whiteKingMoved = 1;
         castlingExecuted = 1;
    } else if(movedPiece == black_king && abs(fromCol - toCol) == 2 && fromRow == 7) {
         if(toCol == 6) {  // Black kingside castling
              board[7][5] = board[7][7];
              board[7][7] = 0;
              blackKingRookMoved = 1;
         } else if(toCol == 2) {  // Black queenside castling
              board[7][3] = board[7][0];
              board[7][0] = 0;
              blackQueenRookMoved = 1;
         }
         blackKingMoved = 1;
         castlingExecuted = 1;
    }
    // Pawn promotion
    if(board[toRow][toCol] == white_pawn && toRow == 7) {
         board[toRow][toCol] = white_queen;  // Automatically promote to queen
         printf("Pawn promoted to Queen!\n");
         promotionExecuted = 1;
    } else if(board[toRow][toCol] == black_pawn && toRow == 0) {
         board[toRow][toCol] = black_queen;  // Automatically promote to queen
         printf("Pawn promoted to Queen!\n");
         promotionExecuted = 1;
    }

    // Get the color of the player who just moved
    int whiteMove = isPieceWhite(movedPiece);
    
    // Clear previous state flags
    checkFlag = 0;
    checkMateFlag = 0;
    stalemateFlag = 0;
    
    // Check if opponent's king is in check after this move
    int opponentIsWhite = !whiteMove;
    if (isKingInCheck(opponentIsWhite)) {
        if (isCheckMate(opponentIsWhite)) {
            checkMateFlag = 1;
        } else {
            checkFlag = 1;
        }
    } else if (isStaleMate(opponentIsWhite)) {
        stalemateFlag = 1;
    }

    // Check for 50-move rule draw
    if (isFiftyMoveRuleDraw()) {
        stalemateFlag = 1;
        return;
    }
}
