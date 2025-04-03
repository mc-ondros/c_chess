#ifndef C_CHESS_SAVELOAD_H
#define C_CHESS_SAVELOAD_H

/**
 * Calculate a simple checksum for data integrity verification
 * 
 * @param data Pointer to the data to calculate checksum for
 * @param size Size of the data in bytes
 * @return Calculated checksum value
 */
unsigned int calculateChecksum(const void* data, size_t size);

/**
 * Saves the current game state to a file
 * 
 * @param filename The path to save the game
 * @return 1 on success, 0 on failure
 */
int saveGame(const char* filename);

/**
 * Loads a game state from a file
 * 
 * @param filename The path to the saved game
 * @return 1 on success, 0 on failure
 */
int loadGame(const char* filename);

#endif // C_CHESS_SAVELOAD_H
