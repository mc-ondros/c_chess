#include <stdio.h>
#include "chess.h"

// Find the king's position for the specified player
static void findKingPosition(int playerIsWhite, int *kingRow, int *kingCol) {
    wchar_t kingPiece = playerIsWhite ? white_king : black_king;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board[row][col] == kingPiece) {
                *kingRow = row;
                *kingCol = col;
                return;
            }
        }
    }
}

// Check if the king is in check
int isKingInCheck(int playerIsWhite) {
    int kingRow, kingCol;
    findKingPosition(playerIsWhite, &kingRow, &kingCol);
    
    // Check if any opponent's piece can attack the king
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            wchar_t piece = board[row][col];
            
            // Skip empty squares and own pieces
            if (piece == 0 || (playerIsWhite && isPieceWhite(piece)) || 
                (!playerIsWhite && isPieceBlack(piece))) {
                continue;
            }
            
            // Check if this opponent piece can capture the king
            if (isValidMove(row, col, kingRow, kingCol)) {
                return 1; // King is in check
            }
        }
    }
    
    return 0; // King is not in check
}

// Check if the player is in checkmate
int isCheckMate(int playerIsWhite) {
    // First, check if the king is in check
    if (!isKingInCheck(playerIsWhite)) {
        return 0; // Not in check, so not in checkmate
    }
    
    // Try all possible moves for all the player's pieces
    for (int fromRow = 0; fromRow < 8; fromRow++) {
        for (int fromCol = 0; fromCol < 8; fromCol++) {
            wchar_t piece = board[fromRow][fromCol];
            
            // Skip empty squares and opponent's pieces
            if (piece == 0 || (playerIsWhite && !isPieceWhite(piece)) || 
                (!playerIsWhite && !isPieceBlack(piece))) {
                continue;
            }
            
            // Try moving this piece to every square on the board
            for (int toRow = 0; toRow < 8; toRow++) {
                for (int toCol = 0; toCol < 8; toCol++) {
                    // Skip invalid moves
                    if (!isValidMove(fromRow, fromCol, toRow, toCol)) {
                        continue;
                    }
                    
                    // Make the move temporarily
                    wchar_t originalFrom = board[fromRow][fromCol];
                    wchar_t originalTo = board[toRow][toCol];
                    board[toRow][toCol] = board[fromRow][fromCol];
                    board[fromRow][fromCol] = 0;
                    
                    // Check if the king is still in check after this move
                    int stillInCheck = isKingInCheck(playerIsWhite);
                    
                    // Undo the move
                    board[fromRow][fromCol] = originalFrom;
                    board[toRow][toCol] = originalTo;
                    
                    // If there's at least one move that gets the king out of check, it's not checkmate
                    if (!stillInCheck) {
                        return 0;
                    }
                }
            }
        }
    }
    
    // If we've tried all moves and none get the king out of check, it's checkmate
    return 1;
}
