#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
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

// Threefold repetition tracking
unsigned long long positionHashes[MAX_REPETITIONS];
int repetitionCount = 0;

// --- Bitboard representation for speed optimization ---
Bitboard bitboards[12] = {0}; // 0-5: white, 6-11: black (K,Q,R,B,N,P)

static int pieceToBitboardIndex(wchar_t piece) {
    switch (piece) {
        case white_king:   return 0;
        case white_queen:  return 1;
        case white_rook:   return 2;
        case white_bishop: return 3;
        case white_knight: return 4;
        case white_pawn:   return 5;
        case black_king:   return 6;
        case black_queen:  return 7;
        case black_rook:   return 8;
        case black_bishop: return 9;
        case black_knight: return 10;
        case black_pawn:   return 11;
        default: return -1;
    }
}

void initBitboards() {
    memset(bitboards, 0, sizeof(bitboards));
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            wchar_t piece = board[row][col];
            int idx = pieceToBitboardIndex(piece);
            if (idx >= 0) {
                bitboards[idx] |= (1ULL << (row * 8 + col));
            }
        }
    }
}

void updateBitboards() {
    initBitboards();
}

int isSquareOccupied(int row, int col) {
    for (int i = 0; i < 12; ++i) {
        if (bitboards[i] & (1ULL << (row * 8 + col)))
            return 1;
    }
    return 0;
}

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

// Fast attack check using bitboards for knights and pawns (partial optimization)
int isSquareAttackedBB(int row, int col, int defenderIsWhite) {
    // Knight moves
    static const int knightMoves[8][2] = {
        {2,1},{1,2},{-1,2},{-2,1},{-2,-1},{-1,-2},{1,-2},{2,-1}
    };
    int attackerIdx = defenderIsWhite ? 10 : 4; // black_knight or white_knight
    for (int i = 0; i < 8; ++i) {
        int r = row + knightMoves[i][0];
        int c = col + knightMoves[i][1];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            if (bitboards[attackerIdx] & (1ULL << (r * 8 + c)))
                return 1;
        }
    }
    // Pawn attacks
    if (defenderIsWhite) {
        // black pawn attacks down
        if (row > 0 && col > 0 && (bitboards[11] & (1ULL << ((row-1)*8 + (col-1))))) return 1;
        if (row > 0 && col < 7 && (bitboards[11] & (1ULL << ((row-1)*8 + (col+1))))) return 1;
    } else {
        // white pawn attacks up
        if (row < 7 && col > 0 && (bitboards[5] & (1ULL << ((row+1)*8 + (col-1))))) return 1;
        if (row < 7 && col < 7 && (bitboards[5] & (1ULL << ((row+1)*8 + (col+1))))) return 1;
    }
    // For other pieces, fallback to slow path (could be optimized further)
    return isSquareAttacked(row, col, defenderIsWhite);
}

// Function to check for 50-move rule draw
int isFiftyMoveRuleDraw() {
    return fiftyMoveCounter >= 100;
}

// Simple board hash: XOR all pieces and castling/en passant state
unsigned long long computeBoardHash() {
    unsigned long long hash = 0xcbf29ce484222325ULL; // FNV offset basis
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            hash ^= (unsigned long long)board[row][col] + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
    }
    // Include castling rights and en passant
    hash ^= (whiteKingMoved << 1) | (whiteKingRookMoved << 2) | (whiteQueenRookMoved << 3);
    hash ^= (blackKingMoved << 4) | (blackKingRookMoved << 5) | (blackQueenRookMoved << 6);
    hash ^= ((enPassantTargetRow & 0xF) << 8) | ((enPassantTargetCol & 0xF) << 12);
    return hash;
}

void recordPositionHash() {
    if (repetitionCount < MAX_REPETITIONS) {
        positionHashes[repetitionCount++] = computeBoardHash();
    }
}

int isThreefoldRepetition() {
    unsigned long long current = computeBoardHash();
    int count = 0;
    for (int i = 0; i < repetitionCount; ++i) {
        if (positionHashes[i] == current) {
            count++;
        }
    }
    return count >= 3;
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
                     if(isSquareAttackedBB(0, 5, 1) || isSquareAttackedBB(0, 6, 1))
                         return 0;
                     return 1;
              }
              else if(toCol == 2 && !whiteKingMoved && !whiteQueenRookMoved &&
                      board[0][3] == 0 && board[0][2] == 0 && board[0][1] == 0 && board[0][0] == white_rook) {
                     // For queenside, ensure squares d1 and c1 are not attacked.
                     if(isSquareAttackedBB(0, 3, 1) || isSquareAttackedBB(0, 2, 1))
                         return 0;
                     return 1;
              }
         } else if(piece == black_king && fromRow == 7) {
              if(toCol == 6 && !blackKingMoved && !blackKingRookMoved &&
                 board[7][5] == 0 && board[7][6] == 0 && board[7][7] == black_rook) {
                     // For kingside, ensure squares f8 and g8 are not attacked.
                     if(isSquareAttackedBB(7, 5, 0) || isSquareAttackedBB(7, 6, 0))
                         return 0;
                     return 1;
              }
              else if(toCol == 2 && !blackKingMoved && !blackQueenRookMoved &&
                      board[7][3] == 0 && board[7][2] == 0 && board[7][1] == 0 && board[7][0] == black_rook) {
                     // For queenside, ensure squares d8 and c8 are not attacked.
                     if(isSquareAttackedBB(7, 3, 0) || isSquareAttackedBB(7, 2, 0))
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

    // Record position hash for threefold repetition
    recordPositionHash();

    // Check for 50-move rule draw
    if (isFiftyMoveRuleDraw()) {
        stalemateFlag = 1;
        return;
    }
    // Check for threefold repetition draw
    if (isThreefoldRepetition()) {
        stalemateFlag = 1;
        return;
    }

    // Update bitboards after move
    updateBitboards();
}

// --- Local CPU (minimax with alpha-beta pruning and iterative deepening) ---

// Piece-square tables (simplified, midgame, white's perspective; black is mirrored)
static const int pawn_table[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};
static const int knight_table[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};
static const int bishop_table[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};
static const int rook_table[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
};
static const int queen_table[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};
static const int king_table[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

// Helper for mirroring tables for black
static inline int mirror_row(int row) { return 7 - row; }

// Refined evaluation function using Shannon's formula and piece-square tables
int evaluateBoard() {
    // Piece values
    const int PAWN_VALUE = 100;
    const int KNIGHT_VALUE = 320;
    const int BISHOP_VALUE = 330;
    const int ROOK_VALUE = 500;
    const int QUEEN_VALUE = 900;
    const int KING_VALUE = 20000;

    int score = 0;

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            wchar_t piece = board[row][col];
            if (piece == 0) continue;
            int value = 0, psq = 0;
            switch (piece) {
                case white_pawn:
                    value = PAWN_VALUE;
                    psq = pawn_table[row][col];
                    break;
                case black_pawn:
                    value = -PAWN_VALUE;
                    psq = -pawn_table[mirror_row(row)][col];
                    break;
                case white_knight:
                    value = KNIGHT_VALUE;
                    psq = knight_table[row][col];
                    break;
                case black_knight:
                    value = -KNIGHT_VALUE;
                    psq = -knight_table[mirror_row(row)][col];
                    break;
                case white_bishop:
                    value = BISHOP_VALUE;
                    psq = bishop_table[row][col];
                    break;
                case black_bishop:
                    value = -BISHOP_VALUE;
                    psq = -bishop_table[mirror_row(row)][col];
                    break;
                case white_rook:
                    value = ROOK_VALUE;
                    psq = rook_table[row][col];
                    break;
                case black_rook:
                    value = -ROOK_VALUE;
                    psq = -rook_table[mirror_row(row)][col];
                    break;
                case white_queen:
                    value = QUEEN_VALUE;
                    psq = queen_table[row][col];
                    break;
                case black_queen:
                    value = -QUEEN_VALUE;
                    psq = -queen_table[mirror_row(row)][col];
                    break;
                case white_king:
                    value = KING_VALUE;
                    psq = king_table[row][col];
                    break;
                case black_king:
                    value = -KING_VALUE;
                    psq = -king_table[mirror_row(row)][col];
                    break;
            }
            score += value + psq;
        }
    }

    // Optionally, add simple bonuses/penalties for castling rights, doubled pawns, etc.

    // Checkmate/stalemate detection for terminal positions
    if (isCheckMate(0)) return -100000; // White is checkmated
    if (isCheckMate(1)) return 100000;  // Black is checkmated
    if (isStaleMate(0) || isStaleMate(1)) return 0; // Draw

    return score;
}

// Move representation for fast move generation
typedef struct {
    int fromRow, fromCol, toRow, toCol;
    int captureValue; // For move ordering: higher is better
} Move;

// Generate all legal moves for a player (0=white, 1=black), returns count
static int generateLegalMoves(int playerIsWhite, Move moves[], int maxMoves) {
    int count = 0;
    for (int fr = 0; fr < 8; ++fr) {
        for (int fc = 0; fc < 8; ++fc) {
            wchar_t piece = board[fr][fc];
            if (piece == 0) continue;
            if (playerIsWhite ? !isPieceWhite(piece) : !isPieceBlack(piece)) continue;
            for (int tr = 0; tr < 8; ++tr) {
                for (int tc = 0; tc < 8; ++tc) {
                    if (fr == tr && fc == tc) continue;
                    wchar_t target = board[tr][tc];
                    if ((playerIsWhite && isPieceWhite(target)) || (!playerIsWhite && isPieceBlack(target)))
                        continue;
                    if (!isValidMove(fr, fc, tr, tc)) continue;
                    if (moveWouldExposeCheck(fr, fc, tr, tc, playerIsWhite)) continue;
                    // Don't allow capturing the king (illegal in chess)
                    if (target == white_king || target == black_king) continue;
                    if (count < maxMoves) {
                        moves[count].fromRow = fr;
                        moves[count].fromCol = fc;
                        moves[count].toRow = tr;
                        moves[count].toCol = tc;
                        // Prefer captures for move ordering
                        moves[count].captureValue = 0;
                        if (target != 0) {
                            // Assign higher value for capturing more valuable pieces
                            if (target == white_queen || target == black_queen) moves[count].captureValue = 9;
                            else if (target == white_rook || target == black_rook) moves[count].captureValue = 5;
                            else if (target == white_bishop || target == black_bishop ||
                                     target == white_knight || target == black_knight) moves[count].captureValue = 3;
                            else if (target == white_pawn || target == black_pawn) moves[count].captureValue = 1;
                        }
                        count++;
                    }
                }
            }
        }
    }
    return count;
}

// Sort moves by capture value (descending) for better alpha-beta pruning
static void sortMoves(Move moves[], int count) {
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (moves[j].captureValue > moves[i].captureValue) {
                Move tmp = moves[i];
                moves[i] = moves[j];
                moves[j] = tmp;
            }
        }
    }
}

// Minimax with alpha-beta pruning, depth-limited, using fast move generation
static int minimax(int depth, int maximizingPlayer, int alpha, int beta) {
    // Terminal state: checkmate, stalemate, or depth limit
    if (depth == 0 || isCheckMate(0) || isCheckMate(1) || isStaleMate(0) || isStaleMate(1)) {
        return evaluateBoard();
    }

    Move moves[128];
    int moveCount = generateLegalMoves(maximizingPlayer ? 1 : 0, moves, 128);
    if (moveCount == 0) return evaluateBoard();
    sortMoves(moves, moveCount);

    int bestScore = maximizingPlayer ? -10000 : 10000;
    for (int i = 0; i < moveCount; ++i) {
        int fr = moves[i].fromRow, fc = moves[i].fromCol, tr = moves[i].toRow, tc = moves[i].toCol;
        wchar_t savedFrom = board[fr][fc];
        wchar_t savedTo = board[tr][tc];
        board[tr][tc] = savedFrom;
        board[fr][fc] = 0;
        int score = minimax(depth - 1, !maximizingPlayer, alpha, beta);
        board[fr][fc] = savedFrom;
        board[tr][tc] = savedTo;
        if (maximizingPlayer) {
            if (score > bestScore) bestScore = score;
            if (score > alpha) alpha = score;
            if (beta <= alpha) break;
        } else {
            if (score < bestScore) bestScore = score;
            if (score < beta) beta = score;
            if (beta <= alpha) break;
        }
    }
    return bestScore;
}

// Iterative deepening wrapper for minimax, optimized for speed
void getLocalCPUMove(int *fromRow, int *fromCol, int *toRow, int *toCol) {
    Move moves[128];
    int bestScore = 10000;
    int found = 0;
    int bestIdx = -1;
    int maxDepth = 3; // Lowered for faster response (increase for stronger play)
    int moveCount = generateLegalMoves(0, moves, 128);
    if (moveCount == 0) return;
    sortMoves(moves, moveCount);

    // Search at increasing depth, always keep best move found so far
    for (int depth = 1; depth <= maxDepth; ++depth) {
        found = 0;
        bestScore = 10000;
        for (int i = 0; i < moveCount; ++i) {
            int fr = moves[i].fromRow, fc = moves[i].fromCol, tr = moves[i].toRow, tc = moves[i].toCol;
            wchar_t savedFrom = board[fr][fc];
            wchar_t savedTo = board[tr][tc];
            board[tr][tc] = savedFrom;
            board[fr][fc] = 0;
            int score = minimax(depth - 1, 1, -10000, 10000);
            board[fr][fc] = savedFrom;
            board[tr][tc] = savedTo;
            if (!found || score < bestScore) {
                bestScore = score;
                bestIdx = i;
                found = 1;
            }
        }
        if (!found) break;
    }
    if (found && bestIdx >= 0) {
        *fromRow = moves[bestIdx].fromRow;
        *fromCol = moves[bestIdx].fromCol;
        *toRow = moves[bestIdx].toRow;
        *toCol = moves[bestIdx].toCol;
        return;
    }
    // Fallback: pick first legal move
    *fromRow = moves[0].fromRow;
    *fromCol = moves[0].fromCol;
    *toRow = moves[0].toRow;
    *toCol = moves[0].toCol;
}

