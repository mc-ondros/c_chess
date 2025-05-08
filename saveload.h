#ifndef C_CHESS_SAVELOAD_H
#define C_CHESS_SAVELOAD_H

/**
 * Saves the current game state to a PGN file.
 * 
 * @param filename The path to save the PGN game
 * @return 1 on success, 0 on failure
 */
int saveGame(const char* filename);

/**
 * Loads a game state from a PGN file.
 * 
 * @param filename The path to the PGN game
 * @return 1 on success, 0 on failure
 */
int loadGame(const char* filename);

#endif // C_CHESS_SAVELOAD_H

