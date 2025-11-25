/*
 * Dynamic Math and Data Processing Engine
 * Description: Function pointer-based math operations with dynamic memory
 * Author: Student Submission
 * Date: November 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_FILENAME 100

// Function pointer type definitions
typedef float (*MathOperation)(float*, int);
typedef void (*SortOperation)(float*, int, int);

// Structure to hold operation information
typedef struct {
    char name[50];
    MathOperation operation;
} Operation;

// Global dataset
float* dataset = NULL;
int dataSize = 0;
int dataCapacity = 0;

// Function prototypes - Math Operations
float computeSum(float* data, int size);
float computeAverage(float* data, int size);
float findMaximum(float* data, int size);
float findMinimum(float* data, int size);
float computeMedian(float* data, int size);
float computeStdDev(float* data, int size);

// Function prototypes - Data Operations
void sortAscending(float* data, int size, int dummy);
void sortDescending(float* data, int size, int dummy);
int searchValue(float* data, int size, float target);

// Function prototypes - Memory Management
void initializeDataset();
void expandDataset();
void addElement();
void removeElement();
void modifyElement();
void clearDataset();
void freeDataset();

// Function prototypes - File Operations
void loadFromFile();
void saveToFile();

// Function prototypes - Menu & Display
void displayMenu();
void displayDataset();
void executeOperation();
void performSearch();
void performSort();
void displayStatistics();
int getValidInteger(const char* prompt);
float getValidFloat(const char* prompt);
void clearInputBuffer();

// Operation registry
Operation operations[] = {
    {"Sum of Elements", computeSum},
    {"Average of Elements", computeAverage},
    {"Maximum Value", findMaximum},
    {"Minimum Value", findMinimum},
    {"Median Value", computeMedian},
    {"Standard Deviation", computeStdDev}
};

int operationCount = 6;

// Main function
int main() {
    int choice;
    
    initializeDataset();
    
    printf("\n================================================\n");
    printf("   DYNAMIC MATH AND DATA PROCESSING ENGINE\n");
    printf("================================================\n");
    
    while (1) {
        displayMenu();
        choice = getValidInteger("Enter your choice: ");
        
        switch (choice) {
            case 1:
                addElement();
                break;
            case 2:
                modifyElement();
                break;
            case 3:
                removeElement();
                break;
            case 4:
                displayDataset();
                break;
            case 5:
                executeOperation();
                break;
            case 6:
                performSort();
                break;
            case 7:
                performSearch();
                break;
            case 8:
                displayStatistics();
                break;
            case 9:
                loadFromFile();
                break;
            case 10:
                saveToFile();
                break;
            case 11:
                clearDataset();
                break;
            case 12:
                freeDataset();
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

// Initialize dataset with initial capacity
void initializeDataset() {
    dataCapacity = 10;
    dataset = (float*)malloc(dataCapacity * sizeof(float));
    if (dataset == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    dataSize = 0;
    printf("Dataset initialized with capacity %d.\n", dataCapacity);
}

// Expand dataset capacity
void expandDataset() {
    dataCapacity *= 2;
    float* temp = (float*)realloc(dataset, dataCapacity * sizeof(float));
    if (temp == NULL) {
        printf("Memory reallocation failed!\n");
        return;
    }
    dataset = temp;
    printf("Dataset capacity expanded to %d.\n", dataCapacity);
}

// Add element to dataset
void addElement() {
    printf("\n========== ADD ELEMENT ==========\n");
    
    if (dataSize >= dataCapacity) {
        expandDataset();
    }
    
    float value = getValidFloat("Enter value to add: ");
    dataset[dataSize] = value;
    dataSize++;
    
    printf("Value %.2f added successfully. Current size: %d\n", value, dataSize);
}

// Remove element from dataset
void removeElement() {
    printf("\n========== REMOVE ELEMENT ==========\n");
    
    if (dataSize == 0) {
        printf("Dataset is empty. Nothing to remove.\n");
        return;
    }
    
    displayDataset();
    
    int index = getValidInteger("Enter index to remove (0-%d): ", dataSize - 1);
    
    if (index < 0 || index >= dataSize) {
        printf("Invalid index!\n");
        return;
    }
    
    float removedValue = dataset[index];
    
    // Shift elements left
    for (int i = index; i < dataSize - 1; i++) {
        dataset[i] = dataset[i + 1];
    }
    
    dataSize--;
    printf("Value %.2f removed successfully. Current size: %d\n", removedValue, dataSize);
}

// Modify element in dataset
void modifyElement() {
    printf("\n========== MODIFY ELEMENT ==========\n");
    
    if (dataSize == 0) {
        printf("Dataset is empty. Nothing to modify.\n");
        return;
    }
    
    displayDataset();
    
    int index = getValidInteger("Enter index to modify (0-%d): ", dataSize - 1);
    
    if (index < 0 || index >= dataSize) {
        printf("Invalid index!\n");
        return;
    }
    
    printf("Current value: %.2f\n", dataset[index]);
    float newValue = getValidFloat("Enter new value: ");
    
    dataset[index] = newValue;
    printf("Value updated successfully!\n");
}

// Display current dataset
void displayDataset() {
    printf("\n========== CURRENT DATASET ==========\n");
    
    if (dataSize == 0) {
        printf("Dataset is empty.\n");
        return;
    }
    
    printf("Size: %d / Capacity: %d\n", dataSize, dataCapacity);
    printf("Index | Value\n");
    printf("------|--------\n");
    
    for (int i = 0; i < dataSize; i++) {
        printf("%-6d| %.2f\n", i, dataset[i]);
    }
    
    printf("=====================================\n");
}

// Execute a math operation using function pointer
void executeOperation() {
    printf("\n========== EXECUTE OPERATION ==========\n");
    
    if (dataSize == 0) {
        printf("Dataset is empty. Please add data first.\n");
        return;
    }
    
    printf("Available operations:\n");
    for (int i = 0; i < operationCount; i++) {
        printf("%d. %s\n", i + 1, operations[i].name);
    }
    
    int choice = getValidInteger("Select operation: ");
    
    if (choice < 1 || choice > operationCount) {
        printf("Invalid choice!\n");
        return;
    }
    
    // Use function pointer to execute selected operation
    MathOperation selectedOp = operations[choice - 1].operation;
    float result = selectedOp(dataset, dataSize);
    
    printf("\n%s: %.2f\n", operations[choice - 1].name, result);
}

// Perform sorting
void performSort() {
    printf("\n========== SORT DATASET ==========\n");
    
    if (dataSize == 0) {
        printf("Dataset is empty. Nothing to sort.\n");
        return;
    }
    
    printf("1. Sort Ascending\n");
    printf("2. Sort Descending\n");
    
    int choice = getValidInteger("Select sorting order: ");
    
    SortOperation sortFunc;
    
    if (choice == 1) {
        sortFunc = sortAscending;
        sortFunc(dataset, dataSize, 0);
        printf("Dataset sorted in ascending order.\n");
    } else if (choice == 2) {
        sortFunc = sortDescending;
        sortFunc(dataset, dataSize, 0);
        printf("Dataset sorted in descending order.\n");
    } else {
        printf("Invalid choice!\n");
        return;
    }
    
    displayDataset();
}

// Perform search
void performSearch() {
    printf("\n========== SEARCH VALUE ==========\n");
    
    if (dataSize == 0) {
        printf("Dataset is empty. Nothing to search.\n");
        return;
    }
    
    float target = getValidFloat("Enter value to search: ");
    
    int index = searchValue(dataset, dataSize, target);
    
    if (index != -1) {
        printf("Value %.2f found at index %d.\n", target, index);
    } else {
        printf("Value %.2f not found in dataset.\n", target);
    }
}

// Display all statistics
void displayStatistics() {
    printf("\n========== DATASET STATISTICS ==========\n");
    
    if (dataSize == 0) {
        printf("Dataset is empty.\n");
        return;
    }
    
    printf("Count:              %d\n", dataSize);
    printf("Sum:                %.2f\n", computeSum(dataset, dataSize));
    printf("Average:            %.2f\n", computeAverage(dataset, dataSize));
    printf("Minimum:            %.2f\n", findMinimum(dataset, dataSize));
    printf("Maximum:            %.2f\n", findMaximum(dataset, dataSize));
    printf("Median:             %.2f\n", computeMedian(dataset, dataSize));
    printf("Standard Deviation: %.2f\n", computeStdDev(dataset, dataSize));
    
    printf("=========================================\n");
}

// Math operation: Sum
float computeSum(float* data, int size) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

// Math operation: Average
float computeAverage(float* data, int size) {
    if (size == 0) return 0;
    return computeSum(data, size) / size;
}

// Math operation: Maximum
float findMaximum(float* data, int size) {
    if (size == 0) return 0;
    
    float max = data[0];
    for (int i = 1; i < size; i++) {
        if (data[i] > max) {
            max = data[i];
        }
    }
    return max;
}

// Math operation: Minimum
float findMinimum(float* data, int size) {
    if (size == 0) return 0;
    
    float min = data[0];
    for (int i = 1; i < size; i++) {
        if (data[i] < min) {
            min = data[i];
        }
    }
    return min;
}

// Math operation: Median
float computeMedian(float* data, int size) {
    if (size == 0) return 0;
    
    // Create a copy and sort it
    float* temp = (float*)malloc(size * sizeof(float));
    if (temp == NULL) {
        printf("Memory allocation failed for median calculation!\n");
        return 0;
    }
    
    memcpy(temp, data, size * sizeof(float));
    sortAscending(temp, size, 0);
    
    float median;
    if (size % 2 == 0) {
        median = (temp[size/2 - 1] + temp[size/2]) / 2.0;
    } else {
        median = temp[size/2];
    }
    
    free(temp);
    return median;
}

// Math operation: Standard Deviation
float computeStdDev(float* data, int size) {
    if (size <= 1) return 0;
    
    float mean = computeAverage(data, size);
    float sumSquaredDiff = 0;
    
    for (int i = 0; i < size; i++) {
        float diff = data[i] - mean;
        sumSquaredDiff += diff * diff;
    }
    
    return sqrt(sumSquaredDiff / size);
}

// Sort ascending (Bubble Sort)
void sortAscending(float* data, int size, int dummy) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (data[j] > data[j + 1]) {
                float temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
}

// Sort descending
void sortDescending(float* data, int size, int dummy) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (data[j] < data[j + 1]) {
                float temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
}

// Search for a value (Linear Search)
int searchValue(float* data, int size, float target) {
    for (int i = 0; i < size; i++) {
        if (data[i] == target) {
            return i;
        }
    }
    return -1;
}

// Load dataset from file
void loadFromFile() {
    printf("\n========== LOAD FROM FILE ==========\n");
    
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
    
    // Clear existing data
    dataSize = 0;
    
    float value;
    int count = 0;
    
    while (fscanf(file, "%f", &value) == 1) {
        if (dataSize >= dataCapacity) {
            expandDataset();
        }
        dataset[dataSize] = value;
        dataSize++;
        count++;
    }
    
    fclose(file);
    
    printf("Successfully loaded %d values from '%s'.\n", count, filename);
}

// Save dataset to file
void saveToFile() {
    printf("\n========== SAVE TO FILE ==========\n");
    
    if (dataSize == 0) {
        printf("Dataset is empty. Nothing to save.\n");
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
    
    for (int i = 0; i < dataSize; i++) {
        fprintf(file, "%.2f\n", dataset[i]);
    }
    
    fclose(file);
    
    printf("Successfully saved %d values to '%s'.\n", dataSize, filename);
}

// Clear all data
void clearDataset() {
    printf("\n========== CLEAR DATASET ==========\n");
    
    if (dataSize == 0) {
        printf("Dataset is already empty.\n");
        return;
    }
    
    char confirm;
    printf("Are you sure you want to clear all data? (y/n): ");
    clearInputBuffer();
    confirm = getchar();
    
    if (confirm == 'y' || confirm == 'Y') {
        dataSize = 0;
        printf("Dataset cleared successfully.\n");
    } else {
        printf("Operation cancelled.\n");
    }
}

// Free memory and exit
void freeDataset() {
    if (dataset != NULL) {
        free(dataset);
        dataset = NULL;
    }
    dataSize = 0;
    dataCapacity = 0;
}

// Display menu
void displayMenu() {
    printf("\n================================================\n");
    printf("                  MAIN MENU\n");
    printf("================================================\n");
    printf("  DATA MANAGEMENT:\n");
    printf("  1.  Add Element\n");
    printf("  2.  Modify Element\n");
    printf("  3.  Remove Element\n");
    printf("  4.  Display Dataset\n");
    printf("\n  OPERATIONS:\n");
    printf("  5.  Execute Math Operation\n");
    printf("  6.  Sort Dataset\n");
    printf("  7.  Search Value\n");
    printf("  8.  Display Statistics\n");
    printf("\n  FILE OPERATIONS:\n");
    printf("  9.  Load from File\n");
    printf("  10. Save to File\n");
    printf("\n  SYSTEM:\n");
    printf("  11. Clear Dataset\n");
    printf("  12. Exit\n");
    printf("================================================\n");
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

float getValidFloat(const char* prompt) {
    float value;
    char buffer[100];
    
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Error reading input.\n");
            continue;
        }
        
        if (sscanf(buffer, "%f", &value) == 1) {
            return value;
        }
        
        printf("Invalid input. Please enter a valid number.\n");
    }
}