#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <fcntl.h>
#include <io.h>
#include "chess.h"
#include "api.h"

int main(void) {
    setlocale(LC_ALL, "");
    _setmode(_fileno(stdout), _O_U16TEXT);  // Enable Unicode output in Windows

    // Set the API key - replace this with your actual API key
    setApiKey("AIzaSyCVA-8dRIBWBXtu2EenqWVpqEVSjDkaNAU");

    #ifdef NO_CURL_SUPPORT
    _setmode(_fileno(stdout), _O_TEXT);  // Temporarily switch back for this message
    printf("NOTE: Building without API support. Black moves will use a simple fallback AI.\n");
    printf("To enable API support, install libcurl development files and rebuild.\n");
    _setmode(_fileno(stdout), _O_U16TEXT);  // Switch back to Unicode
    #else
    _setmode(_fileno(stdout), _O_TEXT);
    printf("API support enabled. Black will use the Gemini API for moves.\n");
    printf("If you see errors, check your API key in main.c\n\n");

    char personality[512];
    printf("Enter AI personality for the black player (e.g., 'aggressive', 'cautious', 'beginner'):\n");
    fgets(personality, sizeof(personality), stdin);
    // Remove newline if present
    size_t len = strlen(personality);
    if (len > 0 && personality[len-1] == '\n') {
        personality[len-1] = '\0';
    }
    setAiPersonality(personality);
    printf("Black player set to play with '%s' personality.\n", personality);
    _setmode(_fileno(stdout), _O_U16TEXT);
    #endif


    createBoard();
    printBoard();

    // Convert back to normal output for input handling
    _setmode(_fileno(stdout), _O_TEXT);

    int fromRow, fromCol, toRow, toCol;
    int playerTurn = 1; // 1 for white, 0 for black
    int gameOver = 0;

    while (!gameOver) {
        printf("\n%s's turn\n", playerTurn ? "White" : "Black");
        printMoveHistory(); // Display the move history

        // Check for check
        if (isKingInCheck(playerTurn)) {
            printf("%s is in CHECK!\n", playerTurn ? "White" : "Black");

            // Check for checkmate
            if (isCheckMate(playerTurn)) {
                printf("CHECKMATE! %s loses the game.\n", playerTurn ? "White" : "Black");
                gameOver = 1;
                break;
            }
        }

        if (playerTurn) {
            // Human player (White)
            if (!readMove(&fromRow, &fromCol, &toRow, &toCol)) {
                printf("Invalid input format. Use format like e2e4.\n");
                continue;
            }

            // Check if player is moving their own piece
            wchar_t piece = board[fromRow][fromCol];
            if (!isPieceWhite(piece)) {
                printf("You can only move your own pieces.\n");
                continue;
            }

            if (!isValidMove(fromRow, fromCol, toRow, toCol)) {
                printf("Invalid move for this piece.\n");
                continue;
            }

            // Check if the move would leave/put the king in check
            wchar_t originalFrom = board[fromRow][fromCol];
            wchar_t originalTo = board[toRow][toCol];

            // Temporarily make the move
            board[toRow][toCol] = board[fromRow][fromCol];
            board[fromRow][fromCol] = 0;

            // Check if the king is in check after this move
            if (isKingInCheck(1)) {
                // Undo the move
                board[fromRow][fromCol] = originalFrom;
                board[toRow][toCol] = originalTo;

                printf("Invalid move: Your king would be in check.\n");
                continue;
            }

            // Undo the temporary move (we'll execute it properly later)
            board[fromRow][fromCol] = originalFrom;
            board[toRow][toCol] = originalTo;

        } else {
            // AI player (Black) - Use Gemini API
            printf("Waiting for AI to make a move...\n");

            if (!getBlackMove(moveHistory, &fromRow, &fromCol, &toRow, &toCol)) {
                printf("Error: AI failed to make a valid move. Retrying...\n");
                continue;
            }

            printf("AI moves: %c%d%c%d\n",
                   'a' + fromCol, fromRow + 1,
                   'a' + toCol, toRow + 1);

            // Validate the AI's move
            wchar_t piece = board[fromRow][fromCol];
            if (!isPieceBlack(piece)) {
                printf("Error: AI attempted to move an invalid piece. Retrying...\n");
                continue;
            }

            if (!isValidMove(fromRow, fromCol, toRow, toCol)) {
                printf("Error: AI made an invalid move. Retrying...\n");
                continue;
            }

            // Check if the move would leave/put the black king in check
            wchar_t originalFrom = board[fromRow][fromCol];
            wchar_t originalTo = board[toRow][toCol];

            // Temporarily make the move
            board[toRow][toCol] = board[fromRow][fromCol];
            board[fromRow][fromCol] = 0;

            // Check if the king is in check after this move
            if (isKingInCheck(0)) {
                // Undo the move
                board[fromRow][fromCol] = originalFrom;
                board[toRow][toCol] = originalTo;

                printf("Error: AI attempted a move that would leave its king in check. Retrying...\n");
                continue;
            }

            // Undo the temporary move (we'll execute it properly later)
            board[fromRow][fromCol] = originalFrom;
            board[toRow][toCol] = originalTo;
        }

        executeMove(fromRow, fromCol, toRow, toCol);

        // Check if this move results in checkmate for the opponent
        if (isKingInCheck(!playerTurn) && isCheckMate(!playerTurn)) {
            // Switch back to Unicode for board display
            _setmode(_fileno(stdout), _O_U16TEXT);
            printBoard();
            _setmode(_fileno(stdout), _O_TEXT);

            printf("CHECKMATE! %s wins the game.\n", playerTurn ? "White" : "Black");
            gameOver = 1;
            break;
        }

        playerTurn = !playerTurn; // Switch turns

        // Switch back to Unicode for board display
        _setmode(_fileno(stdout), _O_U16TEXT);
        printBoard();
        _setmode(_fileno(stdout), _O_TEXT);
    }

    printf("\nGame over. Final move history: %s\n", moveHistory);
    return 0;
}
