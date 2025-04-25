#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "chess.h"

// Function to find the position of the king
int findKing(int playerIsWhite, int *kingRow, int *kingCol) {
    wchar_t kingPiece = playerIsWhite ? white_king : black_king;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board[row][col] == kingPiece) {
                *kingRow = row;
                *kingCol = col;
                return 1;
            }
        }
    }
    return 0; // King not found (should never happen in a valid game)
}

// Check if a king is in check
int isKingInCheck(int playerIsWhite) {
    int kingRow, kingCol;
    if (!findKing(playerIsWhite, &kingRow, &kingCol)) {
        return 0; // No king found
    }

    // Check if any opponent piece can legally move to the king's position
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            wchar_t piece = board[row][col];
            if (piece == 0) continue; // Empty square
            
            // Only check opponent pieces
            if ((playerIsWhite && isPieceBlack(piece)) || (!playerIsWhite && isPieceWhite(piece))) {
                // Temporarily ignore the presence of the king for move validation
                wchar_t savedKing = board[kingRow][kingCol];
                board[kingRow][kingCol] = 0;
                int canAttack = 0;
                
                // Check move validity based on piece type
                if (piece == white_pawn || piece == black_pawn) {
                    canAttack = isPawnMove(piece, row, col, kingRow, kingCol);
                } else if (piece == white_rook || piece == black_rook) {
                    canAttack = isRookMove(piece, row, col, kingRow, kingCol);
                } else if (piece == white_knight || piece == black_knight) {
                    canAttack = isKnightMove(piece, row, col, kingRow, kingCol);
                } else if (piece == white_bishop || piece == black_bishop) {
                    canAttack = isBishopMove(piece, row, col, kingRow, kingCol);
                } else if (piece == white_queen || piece == black_queen) {
                    canAttack = isQueenMove(piece, row, col, kingRow, kingCol);
                } else if (piece == white_king || piece == black_king) {
                    canAttack = isKingMove(piece, row, col, kingRow, kingCol);
                }
                
                // Restore the king
                board[kingRow][kingCol] = savedKing;
                
                // Special case for pawns which can only capture diagonally
                if ((piece == white_pawn || piece == black_pawn) && col == kingCol) {
                    canAttack = 0; // Pawns can't attack forward
                }
                
                if (canAttack) {
                    return 1; // King is in check
                }
            }
        }
    }
    
    return 0; // King is not in check
}

// Check if a move would leave own king in check
int moveWouldExposeCheck(int fromRow, int fromCol, int toRow, int toCol, int playerIsWhite) {
    // Save current board state
    wchar_t savedPiece = board[toRow][toCol];
    wchar_t movedPiece = board[fromRow][fromCol];
    
    // Make the move temporarily
    board[toRow][toCol] = movedPiece;
    board[fromRow][fromCol] = 0;
    
    // Check if king is in check after this move
    int inCheck = isKingInCheck(playerIsWhite);
    
    // Restore board state
    board[fromRow][fromCol] = movedPiece;
    board[toRow][toCol] = savedPiece;
    
    return inCheck;
}

// Check if a player has any legal moves
int hasLegalMoves(int playerIsWhite) {
    // Check every possible move for the player
    for (int fromRow = 0; fromRow < 8; fromRow++) {
        for (int fromCol = 0; fromCol < 8; fromCol++) {
            wchar_t piece = board[fromRow][fromCol];
            
            // Skip empty squares and opponent pieces
            if (piece == 0 || (playerIsWhite && isPieceBlack(piece)) || (!playerIsWhite && isPieceWhite(piece))) {
                continue;
            }
            
            // Try every destination square
            for (int toRow = 0; toRow < 8; toRow++) {
                for (int toCol = 0; toCol < 8; toCol++) {
                    // Skip if move is not valid according to piece rules
                    if (!isValidMove(fromRow, fromCol, toRow, toCol)) {
                        continue;
                    }
                    
                    // Check if move would leave king in check
                    if (!moveWouldExposeCheck(fromRow, fromCol, toRow, toCol, playerIsWhite)) {
                        return 1; // Found at least one legal move
                    }
                }
            }
        }
    }
    
    return 0; // No legal moves found
}

// Check if a player is in checkmate
int isCheckMate(int playerIsWhite) {
    // If the king is not in check, it's not checkmate
    if (!isKingInCheck(playerIsWhite)) {
        return 0;
    }
    
    // If there are legal moves, it's not checkmate
    return !hasLegalMoves(playerIsWhite);
}

// Check if a player is in stalemate
int isStaleMate(int playerIsWhite) {
    // If the king is in check, it's not stalemate
    if (isKingInCheck(playerIsWhite)) {
        return 0;
    }
    
    // If there are legal moves, it's not stalemate
    return !hasLegalMoves(playerIsWhite);
}
