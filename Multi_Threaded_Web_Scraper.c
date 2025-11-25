/*
 * Multi-threaded Web Scraper
 * Description: Parallel web scraping using POSIX threads
 * Author: Student Submission
 * Date: November 2025
 * 
 * Compilation: gcc -pthread web_scraper.c -o web_scraper -lcurl
 * Usage: ./web_scraper
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
#include <unistd.h>
#include <time.h>

#define MAX_URLS 20
#define MAX_URL_LENGTH 512
#define MAX_FILENAME 100
#define OUTPUT_DIR "scraped_data"

// Structure to hold thread data
typedef struct {
    int threadID;
    char url[MAX_URL_LENGTH];
    char outputFile[MAX_FILENAME];
    int success;
    size_t dataSize;
    time_t startTime;
    time_t endTime;
} ThreadData;

// Structure for curl write callback
typedef struct {
    char* data;
    size_t size;
} MemoryStruct;

// Global variables
ThreadData* threadDataArray = NULL;
int urlCount = 0;

// Function prototypes
void initializeSystem();
void cleanupSystem();
void createOutputDirectory();
void addURLs();
void startScraping();
void displayResults();
void saveURLsToFile();
void loadURLsFromFile();
void* scrapeURL(void* arg);
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
void displayMenu();
int getValidInteger(const char* prompt);
void clearInputBuffer();
char* getCurrentTimestamp();

int main() {
    int choice;
    
    // Initialize curl globally
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    initializeSystem();
    createOutputDirectory();
    
    printf("\n====================================================\n");
    printf("       MULTI-THREADED WEB SCRAPER\n");
    printf("====================================================\n");
    printf("Output directory: %s/\n", OUTPUT_DIR);
    printf("====================================================\n");
    
    while (1) {
        displayMenu();
        choice = getValidInteger("Enter your choice: ");
        
        switch (choice) {
            case 1:
                addURLs();
                break;
            case 2:
                startScraping();
                break;
            case 3:
                displayResults();
                break;
            case 4:
                saveURLsToFile();
                break;
            case 5:
                loadURLsFromFile();
                break;
            case 6:
                cleanupSystem();
                curl_global_cleanup();
                printf("\nExiting program. Goodbye!\n");
                return 0;
            default:
                printf("\nInvalid choice! Please try again.\n");
        }
        
        printf("\nPress Enter to continue...");
        clearInputBuffer();
        getchar();
    }
    
    return 0;
}

// Initialize the system
void initializeSystem() {
    threadDataArray = (ThreadData*)malloc(MAX_URLS * sizeof(ThreadData));
    if (threadDataArray == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    
    for (int i = 0; i < MAX_URLS; i++) {
        threadDataArray[i].threadID = -1;
        threadDataArray[i].url[0] = '\0';
        threadDataArray[i].outputFile[0] = '\0';
        threadDataArray[i].success = 0;
        threadDataArray[i].dataSize = 0;
    }
    
    urlCount = 0;
}

// Cleanup system resources
void cleanupSystem() {
    if (threadDataArray != NULL) {
        free(threadDataArray);
        threadDataArray = NULL;
    }
    urlCount = 0;
}

// Create output directory if it doesn't exist
void createOutputDirectory() {
    char command[200];
    sprintf(command, "mkdir -p %s", OUTPUT_DIR);
    system(command);
}

// Add URLs to scrape
void addURLs() {
    printf("\n========== ADD URLs ==========\n");
    
    if (urlCount >= MAX_URLS) {
        printf("Maximum URL limit (%d) reached!\n", MAX_URLS);
        return;
    }
    
    printf("Current URLs: %d / %d\n", urlCount, MAX_URLS);
    printf("Enter URLs (one per line, empty line to finish):\n");
    
    clearInputBuffer();
    
    while (urlCount < MAX_URLS) {
        printf("URL %d: ", urlCount + 1);
        
        char url[MAX_URL_LENGTH];
        if (fgets(url, MAX_URL_LENGTH, stdin) == NULL) {
            break;
        }
        
        // Remove newline
        url[strcspn(url, "\n")] = 0;
        
        // Check if empty (user wants to stop)
        if (strlen(url) == 0) {
            break;
        }
        
        // Validate URL format (basic check)
        if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) {
            printf("Warning: URL should start with http:// or https://\n");
            printf("Continue anyway? (y/n): ");
            char confirm = getchar();
            clearInputBuffer();
            if (confirm != 'y' && confirm != 'Y') {
                continue;
            }
        }
        
        // Add URL
        strcpy(threadDataArray[urlCount].url, url);
        threadDataArray[urlCount].threadID = urlCount;
        sprintf(threadDataArray[urlCount].outputFile, "%s/page_%d.html", OUTPUT_DIR, urlCount + 1);
        urlCount++;
        
        printf("URL added successfully!\n");
    }
    
    printf("\nTotal URLs: %d\n", urlCount);
}

// Start multi-threaded scraping
void startScraping() {
    printf("\n========== START SCRAPING ==========\n");
    
    if (urlCount == 0) {
        printf("No URLs to scrape. Please add URLs first.\n");
        return;
    }
    
    printf("Starting scraping of %d URLs using %d threads...\n", urlCount, urlCount);
    printf("This may take a moment...\n\n");
    
    pthread_t* threads = (pthread_t*)malloc(urlCount * sizeof(pthread_t));
    if (threads == NULL) {
        printf("Failed to allocate memory for threads!\n");
        return;
    }
    
    // Create threads
    time_t overallStart = time(NULL);
    
    for (int i = 0; i < urlCount; i++) {
        threadDataArray[i].startTime = time(NULL);
        
        if (pthread_create(&threads[i], NULL, scrapeURL, &threadDataArray[i]) != 0) {
            printf("Error creating thread %d\n", i);
            threadDataArray[i].success = 0;
        } else {
            printf("Thread %d started for: %s\n", i + 1, threadDataArray[i].url);
        }
    }
    
    // Wait for all threads to complete
    printf("\nWaiting for threads to complete...\n");
    
    for (int i = 0; i < urlCount; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %d completed.\n", i + 1);
    }
    
    time_t overallEnd = time(NULL);
    
    free(threads);
    
    // Display summary
    printf("\n========== SCRAPING SUMMARY ==========\n");
    printf("Total time: %ld seconds\n", overallEnd - overallStart);
    
    int successCount = 0;
    size_t totalData = 0;
    
    for (int i = 0; i < urlCount; i++) {
        if (threadDataArray[i].success) {
            successCount++;
            totalData += threadDataArray[i].dataSize;
        }
    }
    
    printf("Successful: %d / %d\n", successCount, urlCount);
    printf("Failed: %d / %d\n", urlCount - successCount, urlCount);
    printf("Total data downloaded: %zu bytes (%.2f KB)\n", totalData, totalData / 1024.0);
    printf("======================================\n");
}

// Thread function to scrape a single URL
void* scrapeURL(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    CURL* curl;
    CURLcode res;
    MemoryStruct chunk;
    
    chunk.data = malloc(1);
    chunk.size = 0;
    
    curl = curl_easy_init();
    
    if (curl) {
        // Set URL
        curl_easy_setopt(curl, CURLOPT_URL, data->url);
        
        // Set callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
        
        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        // Set timeout (30 seconds)
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        
        // Set user agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Web Scraper/1.0)");
        
        // Perform the request
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            printf("Thread %d ERROR: %s\n", data->threadID + 1, curl_easy_strerror(res));
            data->success = 0;
        } else {
            // Save to file
            FILE* file = fopen(data->outputFile, "w");
            if (file == NULL) {
                printf("Thread %d ERROR: Could not create output file\n", data->threadID + 1);
                data->success = 0;
            } else {
                fwrite(chunk.data, 1, chunk.size, file);
                fclose(file);
                
                data->success = 1;
                data->dataSize = chunk.size;
                
                printf("Thread %d SUCCESS: Downloaded %zu bytes\n", 
                       data->threadID + 1, chunk.size);
            }
        }
        
        curl_easy_cleanup(curl);
    } else {
        printf("Thread %d ERROR: Failed to initialize curl\n", data->threadID + 1);
        data->success = 0;
    }
    
    data->endTime = time(NULL);
    
    free(chunk.data);
    
    pthread_exit(NULL);
}

// Callback function for curl to write data
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryStruct* mem = (MemoryStruct*)userp;
    
    char* ptr = realloc(mem->data, mem->size + realsize + 1);
    if (ptr == NULL) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

// Display scraping results
void displayResults() {
    printf("\n========== SCRAPING RESULTS ==========\n");
    
    if (urlCount == 0) {
        printf("No scraping results available.\n");
        return;
    }
    
    printf("%-4s %-10s %-50s %-15s %-10s\n", "ID", "Status", "URL", "Output File", "Size (KB)");
    printf("--------------------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < urlCount; i++) {
        ThreadData* data = &threadDataArray[i];
        
        if (data->threadID == -1) continue;
        
        char statusStr[10];
        if (data->success) {
            strcpy(statusStr, "SUCCESS");
        } else {
            strcpy(statusStr, "FAILED");
        }
        
        // Truncate URL if too long
        char truncatedURL[51];
        if (strlen(data->url) > 47) {
            strncpy(truncatedURL, data->url, 47);
            truncatedURL[47] = '.';
            truncatedURL[48] = '.';
            truncatedURL[49] = '.';
            truncatedURL[50] = '\0';
        } else {
            strcpy(truncatedURL, data->url);
        }
        
        // Truncate filename if too long
        char truncatedFile[16];
        if (strlen(data->outputFile) > 12) {
            strncpy(truncatedFile, data->outputFile, 12);
            truncatedFile[12] = '.';
            truncatedFile[13] = '.';
            truncatedFile[14] = '.';
            truncatedFile[15] = '\0';
        } else {
            strcpy(truncatedFile, data->outputFile);
        }
        
        printf("%-4d %-10s %-50s %-15s %-10.2f\n",
               i + 1,
               statusStr,
               truncatedURL,
               truncatedFile,
               data->dataSize / 1024.0);
    }
    
    printf("==================================================================================================\n");
}

// Save URLs to file
void saveURLsToFile() {
    printf("\n========== SAVE URLs TO FILE ==========\n");
    
    if (urlCount == 0) {
        printf("No URLs to save.\n");
        return;
    }
    
    char filename[MAX_FILENAME];
    printf("Enter filename: ");
    clearInputBuffer();
    fgets(filename, MAX_FILENAME, stdin);
    filename[strcspn(filename, "\n")] = 0;
    
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not create file '%s'.\n", filename);
        return;
    }
    
    for (int i = 0; i < urlCount; i++) {
        if (threadDataArray[i].threadID != -1) {
            fprintf(file, "%s\n", threadDataArray[i].url);
        }
    }
    
    fclose(file);
    
    printf("Successfully saved %d URLs to '%s'.\n", urlCount, filename);
}

// Load URLs from file
void loadURLsFromFile() {
    printf("\n========== LOAD URLs FROM FILE ==========\n");
    
    char filename[MAX_FILENAME];
    printf("Enter filename: ");
    clearInputBuffer();
    fgets(filename, MAX_FILENAME, stdin);
    filename[strcspn(filename, "\n")] = 0;
    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file '%s'.\n", filename);
        return;
    }
    
    // Clear existing URLs
    urlCount = 0;
    
    char line[MAX_URL_LENGTH];
    int count = 0;
    
    while (fgets(line, MAX_URL_LENGTH, file) != NULL && urlCount < MAX_URLS) {
        line[strcspn(line, "\n")] = 0;
        
        if (strlen(line) > 0) {
            strcpy(threadDataArray[urlCount].url, line);
            threadDataArray[urlCount].threadID = urlCount;
            sprintf(threadDataArray[urlCount].outputFile, "%s/page_%d.html", OUTPUT_DIR, urlCount + 1);
            threadDataArray[urlCount].success = 0;
            threadDataArray[urlCount].dataSize = 0;
            urlCount++;
            count++;
        }
    }
    
    fclose(file);
    
    printf("Successfully loaded %d URLs from '%s'.\n", count, filename);
}

// Display menu
void displayMenu() {
    printf("\n====================================================\n");
    printf("                    MAIN MENU\n");
    printf("====================================================\n");
    printf("1. Add URLs to scrape\n");
    printf("2. Start scraping (multi-threaded)\n");
    printf("3. Display results\n");
    printf("4. Save URLs to file\n");
    printf("5. Load URLs from file\n");
    printf("6. Exit\n");
    printf("====================================================\n");
}

// Input validation functions
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int getValidInteger(const char* prompt) {
    int value;
    char buffer[100];
    
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Error reading input.\n");
            continue;
        }
        
        if (sscanf(buffer, "%d", &value) == 1) {
            return value;
        }
        
        printf("Invalid input. Please enter a valid integer.\n");
    }
}

char* getCurrentTimestamp() {
    static char timestamp[20];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    return timestamp;
}