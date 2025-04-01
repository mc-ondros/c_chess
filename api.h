#ifndef C_CHESS_API_H
#define C_CHESS_API_H

// Set the API key to be used for API calls
void loadApiKey();
// Get the current API key
const char *getApiKey();

void setAiPersonality(const char* personality);
// Set the AI personality for the API

// Disable SSL certificate verification (less secure)
void disableSSLVerification();

// Enable SSL certificate verification (more secure)
void enableSSLVerification();

// Call the API to get a move for the black player
int getBlackMove(const char *moveHistory, int *fromRow, int *fromCol, int *toRow, int *toCol);

#endif //C_CHESS_API_H
