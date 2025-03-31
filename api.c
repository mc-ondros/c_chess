#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ctype.h>
#include "api.h"
#include "chess.h"

// Global variable to store the API key
static char api_key[256] = "";

// Structure to store the response data
struct ResponseData {
    char *data;
    size_t size;
};

// Set the API key
void setApiKey(const char* key) {
    strncpy(api_key, key, sizeof(api_key) - 1);
    api_key[sizeof(api_key) - 1] = '\0';
}

// Get the API key
const char* getApiKey() {
    return api_key;
}

// Callback function for handling the API response
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct ResponseData *mem = (struct ResponseData *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        printf("Error: Not enough memory!\n");
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

// Function to extract the move from the API response
static int extractMove(const char *response, int *fromRow, int *fromCol, int *toRow, int *toCol) {
    // Look for patterns like e2e4, a7a5, etc.
    char movePattern[5] = {0};

    // Search for a move pattern like "e2e4" in the response
    // This is simplified and may need improvement for real-world use
    char *ptr = strstr(response, "move as black");
    if (!ptr) return 0;

    // Look for a simple 4-character pattern that could be a chess move
    while (*ptr) {
        if (isalpha(*ptr) && isdigit(*(ptr+1)) && isalpha(*(ptr+2)) && isdigit(*(ptr+3)) &&
            *ptr >= 'a' && *ptr <= 'h' && *(ptr+2) >= 'a' && *(ptr+2) <= 'h' &&
            *(ptr+1) >= '1' && *(ptr+1) <= '8' && *(ptr+3) >= '1' && *(ptr+3) <= '8') {

            strncpy(movePattern, ptr, 4);
            movePattern[4] = '\0';
            break;
        }
        ptr++;
    }

    if (movePattern[0] == 0) return 0;

    // Convert chess notation to indices
    *fromCol = movePattern[0] - 'a';
    *fromRow = movePattern[1] - '1';
    *toCol = movePattern[2] - 'a';
    *toRow = movePattern[3] - '1';

    return 1;
}

// Function to call the Gemini API and get black's move
int getBlackMove(const char* moveHistory, int* fromRow, int* fromCol, int* toRow, int* toCol) {
    CURL *curl;
    CURLcode res;

    // Initialize the response structure
    struct ResponseData response;
    response.data = malloc(1);
    response.size = 0;

    if (!response.data) {
        printf("Error: Memory allocation failed!\n");
        return 0;
    }

    // Prepare the prompt for Gemini API
    char base_prompt[] = "Let's play a game of chess. I will play as white. "
                         "Start with the standard initial chess position. "
                         "After each of my moves, you will: "
                         "Validate the move according to standard chess rules. "
                         "I will tell you only legal moves. "
                         "Then, you will make your move as black, telling me the piece position "
                         "and move position (e.g. e2e4). "
                         "Provide the board state in standard FEN notation after your move. "
                         "Maintain the game state throughout our interaction. "
                         "Inform me if a checkmate, stalemate, or draw occurs, and explain the outcome. "
                         "Please do not provide any commentary or analysis beyond the requested information, "
                         "unless I ask for it. Respond only with the move.";

    // Create the complete prompt with the move history
    char *prompt = malloc(strlen(base_prompt) + strlen(moveHistory) + 100);
    if (!prompt) {
        printf("Error: Memory allocation failed!\n");
        free(response.data);
        return 0;
    }

    sprintf(prompt, "%s\n\nMove history: %s", base_prompt, moveHistory);

    // Create the JSON payload
    char *payload = malloc(strlen(prompt) + 256);
    if (!payload) {
        printf("Error: Memory allocation failed!\n");
        free(response.data);
        free(prompt);
        return 0;
    }

    sprintf(payload, "{\"contents\":[{\"parts\":[{\"text\":\"%s\"}]}]}", prompt);

    // Create the URL with the API key
    char url[512];
    sprintf(url, "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=%s", api_key);

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    int success = 0;

    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Set the POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);

        // Set headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        // Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Extract the black's move from the response
            success = extractMove(response.data, fromRow, fromCol, toRow, toCol);
            if (!success) {
                printf("Error: Failed to extract move from API response\n");
                printf("API response: %s\n", response.data);
            }
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    // Clean up
    curl_global_cleanup();
    free(response.data);
    free(prompt);
    free(payload);

    return success;
}
