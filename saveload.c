#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "chess.h"

// File signature to identify valid chess save files
#define CHESS_FILE_SIGNATURE "CCHESS01"
#define SIGNATURE_SIZE 8

// Calculate a simple checksum for data integrity verification
unsigned int calculateChecksum(const void* data, size_t size) {
    // Loop through each byte to compute checksum
    unsigned int checksum = 0;
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        checksum = (checksum + bytes[i]) % 65536;
    }
    return checksum;
}

int saveGame(const char* filename) {
    // Create a temporary filename for safe saving
    char tempFilename[256];
    snprintf(tempFilename, sizeof(tempFilename), "%s.tmp", filename);
    
    FILE* file = fopen(tempFilename, "wb");
    if (!file) {
        printf("Error: Could not create save file %s\n", tempFilename);
        return 0;
    }
    
    // Write file signature to validate the save file format later
    if (fwrite(CHESS_FILE_SIGNATURE, 1, SIGNATURE_SIZE, file) != SIGNATURE_SIZE) {
        printf("Error: Failed to write file signature\n");
        fclose(file);
        remove(tempFilename);
        return 0;
    }
    
    // Write board state: save current game board layout
    if (fwrite(board, sizeof(wchar_t), 64, file) != 64) {
        printf("Error: Failed to write board state\n");
        fclose(file);
        remove(tempFilename);
        return 0;
    }
    
    // Write move history length and data: preserve game moves
    size_t historyLen = strlen(moveHistory) + 1; // Include null terminator
    if (fwrite(&historyLen, sizeof(size_t), 1, file) != 1) {
        printf("Error: Failed to write history length\n");
        fclose(file);
        remove(tempFilename);
        return 0;
    }
    
    if (fwrite(moveHistory, 1, historyLen, file) != historyLen) {
        printf("Error: Failed to write move history\n");
        fclose(file);
        remove(tempFilename);
        return 0;
    }
    
    // Calculate and write checksum over board and move history for file validation
    unsigned int boardChecksum = calculateChecksum(board, sizeof(wchar_t) * 64);
    unsigned int historyChecksum = calculateChecksum(moveHistory, historyLen);
    unsigned int totalChecksum = boardChecksum + historyChecksum;
    
    if (fwrite(&totalChecksum, sizeof(unsigned int), 1, file) != 1) {
        printf("Error: Failed to write checksum\n");
        fclose(file);
        remove(tempFilename);
        return 0;
    }
    
    fclose(file);
    
    // Safely rename the temporary file to the target filename
    if (remove(filename) != 0 && errno != ENOENT) {
        printf("Warning: Could not remove existing save file %s\n", filename);
        remove(tempFilename);
        return 0;
    }
    
    if (rename(tempFilename, filename) != 0) {
        printf("Error: Failed to rename temporary file to %s\n", filename);
        remove(tempFilename);
        return 0;
    }
    
    printf("Game saved successfully to %s\n", filename);
    return 1;
}

int loadGame(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open save file %s\n", filename);
        return 0;
    }
    
    // Read and verify file signature to ensure valid save format
    char signature[SIGNATURE_SIZE];
    if (fread(signature, 1, SIGNATURE_SIZE, file) != SIGNATURE_SIZE || 
        memcmp(signature, CHESS_FILE_SIGNATURE, SIGNATURE_SIZE) != 0) {
        printf("Error: Invalid save file format\n");
        fclose(file);
        return 0;
    }
    
    // Read board state into a temporary buffer for validation
    wchar_t tempBoard[8][8];
    if (fread(tempBoard, sizeof(wchar_t), 64, file) != 64) {
        printf("Error: Failed to read board state\n");
        fclose(file);
        return 0;
    }
    
    // Read move history length and validate it to prevent buffer overflow
    size_t historyLen;
    if (fread(&historyLen, sizeof(size_t), 1, file) != 1) {
        printf("Error: Failed to read history length\n");
        fclose(file);
        return 0;
    }
    
    if (historyLen == 0 || historyLen > sizeof(moveHistory)) {
        printf("Error: Invalid move history length\n");
        fclose(file);
        return 0;
    }
    
    // Read move history into a temporary buffer
    char tempHistory[4096] = {0};
    if (fread(tempHistory, 1, historyLen, file) != historyLen) {
        printf("Error: Failed to read move history\n");
        fclose(file);
        return 0;
    }
    tempHistory[sizeof(tempHistory) - 1] = '\0'; // Ensure null-termination
    
    // Read saved checksum and verify data integrity
    unsigned int savedChecksum;
    if (fread(&savedChecksum, sizeof(unsigned int), 1, file) != 1) {
        printf("Error: Failed to read checksum\n");
        fclose(file);
        return 0;
    }
    
    unsigned int boardChecksum = calculateChecksum(tempBoard, sizeof(wchar_t) * 64);
    unsigned int historyChecksum = calculateChecksum(tempHistory, historyLen);
    unsigned int calculatedChecksum = boardChecksum + historyChecksum;
    
    if (savedChecksum != calculatedChecksum) {
        printf("Error: Save file is corrupted (checksum mismatch)\n");
        fclose(file);
        return 0;
    }
    
    fclose(file);
    
    // Update game state only after successful validation
    memcpy(board, tempBoard, sizeof(wchar_t) * 64);
    strcpy(moveHistory, tempHistory);
    
    printf("Game loaded successfully from %s\n", filename);
    return 1;
}

