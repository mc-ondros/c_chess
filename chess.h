#ifndef C_CHESS_CHESS_H
#define C_CHESS_CHESS_H

#include <wchar.h>

// Piece definitions
#define white_king   0x2654 // ♔
#define white_queen  0x2655 // ♕
#define white_rook   0x2656 // ♖
#define white_bishop 0x2657 // ♗
#define white_knight 0x2658 // ♘
#define white_pawn   0x2659 // ♙
#define black_king   0x265A // ♚
#define black_queen  0x265B // ♛
#define black_rook   0x265C // ♜
#define black_bishop  0x265D // ♝
#define black_knight  0x265E // ♞
#define black_pawn   0x265F // ♟

// The global board
extern wchar_t board[8][8];

// Global move history
extern char moveHistory[4096];

// Add castling and en passant tracking variables
extern int whiteKingMoved, whiteKingRookMoved, whiteQueenRookMoved;
extern int blackKingMoved, blackKingRookMoved, blackQueenRookMoved;
extern int enPassantTargetRow, enPassantTargetCol;

// 50-move rule counter
extern int fiftyMoveCounter;

// Threefold repetition
#define MAX_REPETITIONS 512
extern unsigned long long positionHashes[MAX_REPETITIONS];
extern int repetitionCount;
unsigned long long computeBoardHash();
void recordPositionHash();
int isThreefoldRepetition();

// --- Bitboard representation for speed optimization ---
typedef unsigned long long Bitboard;
extern Bitboard bitboards[12]; // 0-5: white, 6-11: black (K,Q,R,B,N,P)
void updateBitboards();
void initBitboards();
int isSquareOccupied(int row, int col);
int isSquareAttackedBB(int row, int col, int defenderIsWhite);

// Board manipulation and movement functions
void createBoard();
void printBoard();
int readMove(int *fromRow, int *fromCol, int *toRow, int *toCol);
int isValidMove(int fromRow, int fromCol, int toRow, int toCol);
void executeMove(int fromRow, int fromCol, int toRow, int toCol);
int isPawnMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol);
int isRookMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol);
int isKnightMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol);
int isBishopMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol);
int isQueenMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol);
int isKingMove(wchar_t piece, int fromRow, int fromCol, int toRow, int toCol);
int isPathClear(int fromRow, int fromCol, int toRow, int toCol);
int isPieceWhite(wchar_t piece);
int isPieceBlack(wchar_t piece);
void recordMove(int fromRow, int fromCol, int toRow, int toCol);
void printMoveHistory();

// Local CPU (minimax) move function
void getLocalCPUMove(int *fromRow, int *fromCol, int *toRow, int *toCol);

// Static evaluation function
int evaluateBoard();

// Check-related functions (defined in check.c)
int findKing(int playerIsWhite, int *kingRow, int *kingCol);
int isKingInCheck(int playerIsWhite);
int moveWouldExposeCheck(int fromRow, int fromCol, int toRow, int toCol, int playerIsWhite);
int hasLegalMoves(int playerIsWhite);
int isCheckMate(int playerIsWhite);
int isStaleMate(int playerIsWhite);

// Function to check for 50-move rule draw
int isFiftyMoveRuleDraw();

// Save/Load functions
int saveGame(const char* filename);
int loadGame(const char* filename);

#endif //C_CHESS_CHESS_H
