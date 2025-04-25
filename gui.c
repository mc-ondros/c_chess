#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include "chess.h" // Assuming chess.h contains board definition and move logic
#include "gui.h"   // Assuming gui.h contains function prototypes for gui
#include "api.h"   // Added to support one-player AI move
#include "saveload.h"  // Added for saveGame and loadGame functions

#define BOARD_SIZE 8
#define MAX_AI_RETRIES 3  // Maximum number of retry attempts

// Global array of button widgets for board squares
static GtkWidget *buttons[BOARD_SIZE][BOARD_SIZE];
// Flag to track if a piece is selected
static int selected = 0;
// Coordinates of the selected piece
static int sel_row = -1, sel_col = -1;
// Current player (0 for white, 1 for black)
static int currentPlayer = 0;
// Game mode (e.g., 2 for two-player)
static int gameMode = 2; // Default to two-player mode
// Label to display whose turn it is
static GtkWidget *turnLabel;
// Flag to prevent player from making moves during AI turn
static int aiThinking = 0;
// AI retry counter
static int aiRetryCount = 0;

// Function to display check message
static void show_check_message(GtkWindow *parent, int currentPlayerInCheck) {
    const char *playerColor = currentPlayerInCheck ? "Black" : "White";
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Check! %s's king is under attack.",
        playerColor
    );
    gtk_window_set_title(GTK_WINDOW(dialog), "Check");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Function to display checkmate message
static void show_checkmate_message(GtkWindow *parent) {
    const char *winner = currentPlayer == 0 ? "Black" : "White";
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Checkmate! %s wins the game!",
        winner
    );
    gtk_window_set_title(GTK_WINDOW(dialog), "Checkmate");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Function to display stalemate message
static void show_stalemate_message(GtkWindow *parent) {
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "Stalemate! The game is a draw."
    );
    gtk_window_set_title(GTK_WINDOW(dialog), "Stalemate");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Update the board UI based on the global board array
static void refresh_board() {
    wchar_t piece_str[2] = {0, 0}; // Buffer for single wide character + null terminator
    gchar *utf8_str;               // Buffer for UTF-8 string representation

    // Iterate through each square on the board
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            // Ensure the button is clickable
            gtk_widget_set_sensitive(buttons[row][col], TRUE);
            // Get the piece from the board array (inverted row for display)
            wchar_t piece = board[7 - row][col];

            // Get the style context for the button
            GtkStyleContext *context = gtk_widget_get_style_context(buttons[row][col]);

            // Remove existing piece color classes to reset styling
            gtk_style_context_remove_class(context, "white-piece");
            gtk_style_context_remove_class(context, "black-piece");

            if (piece != 0) { // If there's a piece on the square
                // Convert the wide character piece representation to UTF-8 for GTK label
                piece_str[0] = piece;
                utf8_str = g_ucs4_to_utf8((gunichar*)piece_str, 1, NULL, NULL, NULL);
                gtk_button_set_label(GTK_BUTTON(buttons[row][col]), utf8_str);
                g_free(utf8_str); // Free the allocated UTF-8 string

                // Add the appropriate CSS class based on piece color
                if (piece >= 0x2654 && piece <= 0x2659) { // White pieces
                    gtk_style_context_add_class(context, "white-piece");
                } else if (piece >= 0x265A && piece <= 0x265F) { // Black pieces
                    gtk_style_context_add_class(context, "black-piece");
                }
            } else {
                // If the square is empty, set an empty label
                gtk_button_set_label(GTK_BUTTON(buttons[row][col]), " ");
            }
        }
    }

    // Update the turn label for game mode
    if (gameMode == 2) {
        gchar *turnText = g_strdup_printf("Current turn: %s", currentPlayer == 0 ? "White" : "Black");
        gtk_label_set_text(GTK_LABEL(turnLabel), turnText);
        g_free(turnText); // Free the allocated turn text string
    } else if (gameMode == 1) {
        if (aiThinking) {
            gtk_label_set_text(GTK_LABEL(turnLabel), "AI is thinking...");
        } else {
            gtk_label_set_text(GTK_LABEL(turnLabel), "Your turn: White");
        }
    }

    // Check for special game states and display relevant messages
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(buttons[0][0]));

    if (checkMateFlag) {
        // Disable all buttons when game is over
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                gtk_widget_set_sensitive(buttons[r][c], FALSE);
            }
        }
        show_checkmate_message(parent);
        checkMateFlag = 0; // Reset flag after showing message
    }
    else if (stalemateFlag) {
        // Disable all buttons when game is over
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                gtk_widget_set_sensitive(buttons[r][c], FALSE);
            }
        }
        show_stalemate_message(parent);
        stalemateFlag = 0; // Reset flag after showing message
    }
    else if (checkFlag) {
        // Current player's king is in check
        show_check_message(parent, currentPlayer);
        checkFlag = 0; // Reset flag after showing message
    }
}

// Process AI move in a separate function to ensure UI responsiveness
static gboolean process_ai_move(gpointer data) {
    int aiFromRow, aiFromCol, aiToRow, aiToCol;
    int success = getBlackMove(moveHistory, &aiFromRow, &aiFromCol, &aiToRow, &aiToCol);
    
    if (success) {
        // Verify that AI is attempting to move a black piece
        wchar_t selectedPiece = board[aiFromRow][aiFromCol];
        
        if (selectedPiece == 0) {
            // AI trying to move from an empty square
            printf("AI Error: Tried to move from empty square [%d,%d]\n", aiFromRow, aiFromCol);
            aiRetryCount++;
        }
        else if (!isPieceBlack(selectedPiece)) {
            // AI trying to move a white piece or invalid piece
            printf("AI Error: Tried to move a non-black piece: %d at [%d,%d]\n", 
                   (int)selectedPiece, aiFromRow, aiFromCol);
            aiRetryCount++;
        }
        else if (!isValidMove(aiFromRow, aiFromCol, aiToRow, aiToCol)) {
            // AI trying an invalid move
            printf("AI Error: Illegal move from [%d,%d] to [%d,%d]\n", 
                   aiFromRow, aiFromCol, aiToRow, aiToCol);
            aiRetryCount++;
        }
        else if (moveWouldExposeCheck(aiFromRow, aiFromCol, aiToRow, aiToCol, 0)) {
            // AI trying a move that would put its king in check
            printf("AI Error: Move would expose black king to check\n");
            aiRetryCount++;
        }
        else {
            // Valid move, execute it
            printf("AI moving black %lc from [%d,%d] to [%d,%d]\n", 
                   selectedPiece, aiFromRow, aiFromCol, aiToRow, aiToCol);
            aiRetryCount = 0;
            executeMove(aiFromRow, aiFromCol, aiToRow, aiToCol);
            aiThinking = 0;
            refresh_board();
            return FALSE;
        }
        
        // If we get here, the move was invalid but AI reported success
        if (aiRetryCount < MAX_AI_RETRIES) {
            // Update label to show retry attempt
            gchar *retryText = g_strdup_printf("AI made invalid move. Retrying in 10s (%d/%d)...", 
                                              aiRetryCount, MAX_AI_RETRIES);
            gtk_label_set_text(GTK_LABEL(turnLabel), retryText);
            g_free(retryText);
            
            // Schedule another attempt after 10 seconds
            g_timeout_add_seconds(10, process_ai_move, NULL);
            return FALSE;  // Remove this timeout
        }
    } else {
        // AI failed to generate a move
        aiRetryCount++;
    }
    
    // Handle retry or failure cases
    if (aiRetryCount < MAX_AI_RETRIES) {
        // Update label to show retry attempt
        gchar *retryText = g_strdup_printf("AI failed. Retrying in 10s (%d/%d)...", 
                                          aiRetryCount, MAX_AI_RETRIES);
        gtk_label_set_text(GTK_LABEL(turnLabel), retryText);
        g_free(retryText);
        
        // Schedule another attempt after 10 seconds
        g_timeout_add_seconds(10, process_ai_move, NULL);
    } else {
        // All retry attempts exhausted, show error and give up
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                 "AI failed to make a move after %d attempts.", MAX_AI_RETRIES);
        gtk_window_set_title(GTK_WINDOW(dialog), "AI Move Error");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        // Reset for next turn
        aiRetryCount = 0;
        aiThinking = 0;
        refresh_board();
    }
    
    return FALSE; // Return FALSE to remove the timeout
}

// Callback function for when a board square button is clicked
static void on_button_clicked(GtkWidget *widget, gpointer data) {
    // Retrieve the row and column from the passed data
    int *coords = (int *)data;
    int row = coords[0]; // Visual row (0-7, top to bottom)
    int col = coords[1]; // Visual column (0-7, left to right)
    
    // Prevent player from making moves during AI's turn
    if (gameMode == 1 && aiThinking) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                                                 "Please wait for the AI to complete its move.");
        gtk_window_set_title(GTK_WINDOW(dialog), "Please Wait");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // --- First Click: Selecting a piece ---
    if (!selected) {
        wchar_t piece = board[7 - row][col];
        if (piece != 0) {
            if (gameMode == 2) {
                int isPieceWhite = (piece >= 0x2654 && piece <= 0x2659);
                if ((currentPlayer == 0 && !isPieceWhite) || (currentPlayer == 1 && isPieceWhite)) {
                    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                                                             "It's %s's turn!", currentPlayer == 0 ? "White" : "Black");
                    gtk_window_set_title(GTK_WINDOW(dialog), "Wrong Turn");
                    gtk_dialog_run(GTK_DIALOG(dialog));
                    gtk_widget_destroy(dialog);
                    return;
                }
            } else { // One-Player mode: ensure only white pieces are selected
                int isPieceWhite = (piece >= 0x2654 && piece <= 0x2659);
                if (!isPieceWhite) {
                    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                                                             "In one-player mode, you can only move white pieces.");
                    gtk_window_set_title(GTK_WINDOW(dialog), "Invalid Selection");
                    gtk_dialog_run(GTK_DIALOG(dialog));
                    gtk_widget_destroy(dialog);
                    return;
                }
            }
            selected = 1;
            sel_row = 7 - row;
            sel_col = col;
            GtkStyleContext *context = gtk_widget_get_style_context(widget);
            gtk_style_context_add_class(context, "selected-square");
        }
    }
    // --- Second Click: Moving the selected piece or deselecting ---
    else {
        int fromRow = sel_row;
        int fromCol = sel_col;
        int toRow = 7 - row;
        int toCol = col;

        GtkStyleContext *context = gtk_widget_get_style_context(buttons[7 - fromRow][fromCol]);
        gtk_style_context_remove_class(context, "selected-square");
        selected = 0;

        if (fromRow == toRow && fromCol == toCol) {
            return;
        }

        if (isValidMove(fromRow, fromCol, toRow, toCol)) {
            // Check if this move would put the moving player's own king in check
            wchar_t piece = board[fromRow][fromCol];
            int playerIsWhite = isPieceWhite(piece);
            
            if (moveWouldExposeCheck(fromRow, fromCol, toRow, toCol, playerIsWhite)) {
                GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
                                                         "Invalid move: would leave your king in check.");
                gtk_window_set_title(GTK_WINDOW(dialog), "Invalid Move");
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
                return;
            }
            
            executeMove(fromRow, fromCol, toRow, toCol);
            
            if (gameMode == 2) {
                currentPlayer = 1 - currentPlayer;
                refresh_board();
            } else if (gameMode == 1) {
                // One-player mode: update turn label and call AI move after a short delay
                aiThinking = 1;
                aiRetryCount = 0;  // Reset retry counter for new AI turn
                refresh_board();
                
                // Use a timeout to allow the UI to update before processing the AI move
                g_timeout_add(500, process_ai_move, NULL);
            }
        } else {
            GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Invalid move.");
            gtk_window_set_title(GTK_WINDOW(dialog), "Invalid Move");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    }
}

// Modified callback: parses the last move from moveHistory, calls rateMoveWithAI, and displays the rating result.
static void on_rate_move_clicked(GtkWidget *widget, gpointer data) {
    // Create a local copy to avoid modifying global moveHistory
    char copyHistory[4096];
    strcpy(copyHistory, moveHistory);
    char *token = NULL, *lastToken = NULL;
    token = strtok(copyHistory, " ");
    while(token) {
        lastToken = token;
        token = strtok(NULL, " ");
    }
    if (!lastToken) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                                                    GTK_BUTTONS_OK, "No move to rate.");
        gtk_window_set_title(GTK_WINDOW(dialog), "Rate Move");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    // Call API functionality to rate the move
    int rating = rateMoveWithAI(lastToken, moveHistory);
    char message[256];
    sprintf(message, "Rating for move %s: %d", lastToken, rating);
    GtkWidget *resultDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
                                                     GTK_BUTTONS_OK, "%s", message);
    gtk_window_set_title(GTK_WINDOW(resultDialog), "Move Rating");
    gtk_dialog_run(GTK_DIALOG(resultDialog));
    gtk_widget_destroy(resultDialog);
}

// New callback for saving the game.
static void on_save_game_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Game", GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                    GTK_FILE_CHOOSER_ACTION_SAVE,
                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                    "_Save", GTK_RESPONSE_ACCEPT,
                                    NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (saveGame(filename)) {
            GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                     GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                                     "Game saved successfully to %s", filename);
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
        } else {
            GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                     GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                     "Error saving game to %s", filename);
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// New callback for loading the game.
static void on_load_game_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Load Game", GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                    "_Cancel", GTK_RESPONSE_CANCEL,
                                    "_Open", GTK_RESPONSE_ACCEPT,
                                    NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (loadGame(filename)) {
            refresh_board();
            GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                     GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                                     "Game loaded successfully from %s", filename);
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
        } else {
            GtkWidget *msg = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                     GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                     "Error loading game from %s", filename);
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// Creates and returns the GTK Grid widget containing the chess board buttons and labels
GtkWidget* create_board_grid() {
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 0);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 0);

    GtkWidget *label;
    char label_text[2];

    for (int row = 0; row < BOARD_SIZE; row++) {
        sprintf(label_text, "%d", 8 - row);
        label = gtk_label_new(label_text);
        gtk_widget_set_margin_start(label, 5);
        gtk_widget_set_margin_end(label, 5);
        gtk_grid_attach(GTK_GRID(grid), label, 0, row + 1, 1, 1);
        label = gtk_label_new(label_text);
        gtk_widget_set_margin_start(label, 5);
        gtk_widget_set_margin_end(label, 5);
        gtk_grid_attach(GTK_GRID(grid), label, BOARD_SIZE + 1, row + 1, 1, 1);

        for (int col = 0; col < BOARD_SIZE; col++) {
            if (row == 0) {
                sprintf(label_text, "%c", 'a' + col);
                label = gtk_label_new(label_text);
                gtk_widget_set_margin_top(label, 5);
                gtk_widget_set_margin_bottom(label, 5);
                gtk_grid_attach(GTK_GRID(grid), label, col + 1, 0, 1, 1);
                label = gtk_label_new(label_text);
                gtk_widget_set_margin_top(label, 5);
                gtk_widget_set_margin_bottom(label, 5);
                gtk_grid_attach(GTK_GRID(grid), label, col + 1, BOARD_SIZE + 1, 1, 1);
            }

            GtkWidget *button = gtk_button_new_with_label(" ");
            gtk_widget_set_size_request(button, 60, 60);
            GtkStyleContext *context = gtk_widget_get_style_context(button);
            if ((row + col) % 2 == 0) {
                gtk_style_context_add_class(context, "white-square");
            } else {
                gtk_style_context_add_class(context, "black-square");
            }
            gtk_style_context_add_class(context, "board-button");

            int *data = g_new(int, 2);
            data[0] = row;
            data[1] = col;
            g_signal_connect_data(button, "clicked", G_CALLBACK(on_button_clicked), data, (GClosureNotify)g_free, 0);

            buttons[row][col] = button;
            gtk_grid_attach(GTK_GRID(grid), button, col + 1, row + 1, 1, 1);
        }
    }

    // --- Apply CSS Styling from External File ---
    GtkCssProvider *provider = gtk_css_provider_new();
    GError *error = NULL; // For error reporting

    // Load CSS from the file "style.css" located in the same directory as the executable
    if (!gtk_css_provider_load_from_path(provider, "style.css", &error)) {
        // Print an error message if the file couldn't be loaded
        g_warning("Error loading CSS file: %s", error->message);
        g_clear_error(&error); // Clear the error
    }

    // Apply the CSS provider to the entire screen with USER priority
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                            GTK_STYLE_PROVIDER(provider),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider); // Unreference the provider

    return grid;
}

// Initializes and starts the GTK GUI application
void startGui(int mode) {
    gameMode = mode;
    currentPlayer = 0;
    gtk_init(NULL, NULL);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // Set window title and icon
    gtk_window_set_title(GTK_WINDOW(window), "C Chess");
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "chess_icon.png", NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    turnLabel = gtk_label_new("Current turn: White");
    gtk_widget_set_halign(turnLabel, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), turnLabel, FALSE, FALSE, 5);

    GtkWidget *grid = create_board_grid();
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);

    // Create a toolbar for Save, Load, and Rate Move buttons
    GtkWidget *toolBar = gtk_toolbar_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(toolBar), GTK_ORIENTATION_HORIZONTAL);

    // Add Save Game tool button
    GtkToolItem *saveToolItem = gtk_tool_button_new(NULL, "Save Game");
    g_signal_connect(saveToolItem, "clicked", G_CALLBACK(on_save_game_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolBar), saveToolItem, -1);

    // Add Load Game tool button
    GtkToolItem *loadToolItem = gtk_tool_button_new(NULL, "Load Game");
    g_signal_connect(loadToolItem, "clicked", G_CALLBACK(on_load_game_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolBar), loadToolItem, -1);

    // Add Rate Last Move tool button
    GtkToolItem *rateToolItem = gtk_tool_button_new(NULL, "Rate Last Move With Gemini");
    g_signal_connect(rateToolItem, "clicked", G_CALLBACK(on_rate_move_clicked), NULL);
    gtk_toolbar_insert(GTK_TOOLBAR(toolBar), rateToolItem, -1);

    // Pack the toolbar into the vbox
    gtk_box_pack_start(GTK_BOX(vbox), toolBar, FALSE, FALSE, 5);

    createBoard();
    refresh_board();

    gtk_widget_show_all(window);
    gtk_main();
}

