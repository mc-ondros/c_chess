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

// Add this near your api_key declaration
static char ai_personality[512] = ""; // Store AI personality

// Add this function to set the personality
void setAiPersonality(const char* personality) {
    strncpy(ai_personality, personality, sizeof(ai_personality) - 1);
    ai_personality[sizeof(ai_personality) - 1] = '\0';
}
// Set the API key
void setApiKey(const char* key) {
    strncpy(api_key, key, sizeof(api_key) - 1);
    api_key[sizeof(api_key) - 1] = '\0';
}

char* find_pattern(const char *text) {
    while (*text && *(text + 1) && *(text + 2) && *(text + 3)) {
        if (isalpha(text[0]) && isdigit(text[1]) && isalpha(text[2]) && isdigit(text[3])) {
            // Allocate memory for the result (4 characters + null terminator)
            char *result = (char *)malloc(5 * sizeof(char));
            if (!result) {
                return NULL;  // Memory allocation failed
            }
            result[0] = text[0];
            result[1] = text[1];
            result[2] = text[2];
            result[3] = text[3];
            result[4] = '\0';  // Null-terminate the string
            return result;
        }
        text++;  // Move to the next character
    }
    return NULL;  // No match found
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

    // Convert chess notation to indices
    *fromCol = response[0] - 'a';
    *fromRow = response[1] - '1';
    *toCol = response[2] - 'a';
    *toRow = response[3] - '1';

    return 1;
}

int extractRating(const char *str) {
    if (!str) {
        return -1;
    }

    const char *p = str;
    while (*p) {
        if (isdigit(*p)) {
            char *end_ptr;
            long value = strtol(p, &end_ptr, 10);
            if (value >= 0 && value <= 10) {
                return (int)value;
            }
            // Move pointer past this number and continue searching.
            p = end_ptr;
        } else {
            p++;
        }
    }
    return -1; // No valid rating found
}

int rateMoveWithAI(const char *move,const char *moveHistory) {
    int rating = 0;
    CURL *curl;
    CURLcode res;
    struct ResponseData response;
    response.data = malloc(1);
    response.size = 0;
    if (!response.data) {
        printf("Error: Memory allocation failed!\n");
        return 0;
    }
    // Prepare the prompt for Gemini API
    char prompt[1024];
    snprintf(prompt, sizeof(prompt),
        "Considering this move history: %s "
        "Rate the following chess move: %s. "
        "Provide a score from 0 to 10, where 0 is a bad move and 10 is an excellent move.",
        moveHistory,move);

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
    sprintf(url, "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-pro-exp-03-25:generateContent?key=%s", api_key);

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

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
            rating = extractRating(response.data);
        }
        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    free(response.data);
    free(prompt);
    free(payload);
    return rating;
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
    char base_prompt[1024];
    snprintf(base_prompt, sizeof(base_prompt),
        "Let's play a game of chess. I will play as white. "
        "You will play as black with the following personality: %s. "
        "Start with the standard initial chess position. "
        "After each of my moves, you will: "
        "Validate the move according to standard chess rules. "
        "I will tell you only legal moves. "
        "Then, you will make your move as black, telling me the piece position "
        "and move position (e.g. e2e4). "
        "Every response you give must be composed of exactly 4 letters, representing your move."
        "Provide the board state in standard FEN notation after your move. "
        "Maintain the game state throughout our interaction. "
        "Inform me if a checkmate, stalemate, or draw occurs, and explain the outcome. "
        "Please do not provide any commentary or analysis beyond the requested information, "
        "unless I ask for it. Respond only with the move.",
        ai_personality[0] ? ai_personality : "a skilled chess player");

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
    sprintf(url, "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-pro-exp-03-25:generateContent?key=%s", api_key);

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
            success = extractMove(find_pattern(response.data), fromRow, fromCol, toRow, toCol);
            if (!success) {
                printf("Error: Failed to extract move from API response\n");
                printf("API response: %s\n", find_pattern(response.data));
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
