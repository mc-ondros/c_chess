#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include "chess.h" // Assuming chess.h contains board definition and move logic
#include "gui.h"   // Assuming gui.h contains function prototypes for gui

#define BOARD_SIZE 8

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

// --- Function Prototypes (assuming these are in chess.h or defined elsewhere) ---
// extern wchar_t board[BOARD_SIZE][BOARD_SIZE]; // The game board state
// extern void createBoard();                     // Initializes the board
// extern int isValidMove(int fromRow, int fromCol, int toRow, int toCol); // Checks move validity
// extern void executeMove(int fromRow, int fromCol, int toRow, int toCol); // Makes the move
// extern void recordMove(int fromRow, int fromCol, int toRow, int toCol); // Records the move (optional)
// -----------------------------------------------------------------------------

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

    // Update the turn label (only relevant in two-player mode)
    if (gameMode == 2) {
        gchar *turnText = g_strdup_printf("Current turn: %s", currentPlayer == 0 ? "White" : "Black");
        gtk_label_set_text(GTK_LABEL(turnLabel), turnText);
        g_free(turnText); // Free the allocated turn text string
    }
}

// Callback function for when a board square button is clicked
static void on_button_clicked(GtkWidget *widget, gpointer data) {
    // Retrieve the row and column from the passed data
    int *coords = (int *)data;
    int row = coords[0]; // Visual row (0-7, top to bottom)
    int col = coords[1]; // Visual column (0-7, left to right)

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
            executeMove(fromRow, fromCol, toRow, toCol);
            recordMove(fromRow, fromCol, toRow, toCol);
            if (gameMode == 2) {
                currentPlayer = 1 - currentPlayer;
            }
            refresh_board();
        } else {
            GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Invalid move.");
            gtk_window_set_title(GTK_WINDOW(dialog), "Invalid Move");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    }
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
    gtk_window_set_title(GTK_WINDOW(window), "C Chess");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    turnLabel = gtk_label_new("Current turn: White");
    gtk_widget_set_halign(turnLabel, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), turnLabel, FALSE, FALSE, 5);

    GtkWidget *grid = create_board_grid();
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);

    createBoard();
    refresh_board();

    gtk_widget_show_all(window);
    gtk_main();
}

/*
int main(int argc, char *argv[]) {
    startGui(2);
    return 0;
}
*/
