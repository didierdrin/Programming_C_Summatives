/*
 * Student Management System with Data Analytics
 * Description: Advanced student database with CRUD, analytics, and file I/O
 * Author: Student Submission
 * Date: November 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LENGTH 100
#define MAX_COURSE_LENGTH 50
#define MAX_SUBJECTS 5
#define FILENAME "students.dat"

// Structure for Student Record
typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    int age;
    char course[MAX_COURSE_LENGTH];
    float grades[MAX_SUBJECTS];
    int numSubjects;
    float gpa;
} Student;

// Global variables
Student* students = NULL;
int studentCount = 0;
int capacity = 0;

// Function prototypes
void initializeSystem();
void freeMemory();
void addStudent();
void displayAllStudents();
void searchStudent();
void updateStudent();
void deleteStudent();
void sortStudents();
void generateReports();
void saveToFile();
void loadFromFile();
void computeGPA(Student* student);
int isValidID(int id);
int findStudentIndex(int id);
void bubbleSort(Student* arr, int n, int sortBy);
void insertionSort(Student* arr, int n, int sortBy);
int binarySearchByID(int id);
void calculateStatistics();
void topNStudents(int n);
void topStudentPerCourse();
void courseWiseAverage();
void displayMenu();
void clearInputBuffer();
int getValidInteger(const char* prompt);
float getValidFloat(const char* prompt);
void toLowerCase(char* str);

// Main function
int main() {
    int choice;
    
    initializeSystem();
    loadFromFile();
    
    printf("\n========================================\n");
    printf("   STUDENT MANAGEMENT SYSTEM\n");
    printf("========================================\n");
    
    while (1) {
        displayMenu();
        choice = getValidInteger("Enter your choice: ");
        
        switch (choice) {
            case 1:
                addStudent();
                break;
            case 2:
                displayAllStudents();
                break;
            case 3:
                searchStudent();
                break;
            case 4:
                updateStudent();
                break;
            case 5:
                deleteStudent();
                break;
            case 6:
                sortStudents();
                break;
            case 7:
                generateReports();
                break;
            case 8:
                saveToFile();
                break;
            case 9:
                loadFromFile();
                break;
            case 10:
                saveToFile();
                freeMemory();
                printf("\nExiting... Thank you!\n");
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

void initializeSystem() {
    capacity = 10;
    students = (Student*)malloc(capacity * sizeof(Student));
    if (students == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    studentCount = 0;
}

void freeMemory() {
    if (students != NULL) {
        free(students);
        students = NULL;
    }
    studentCount = 0;
    capacity = 0;
}

void addStudent() {
    printf("\n========== ADD NEW STUDENT ==========\n");
    
    // Check if we need to expand the array
    if (studentCount >= capacity) {
        capacity *= 2;
        Student* temp = (Student*)realloc(students, capacity * sizeof(Student));
        if (temp == NULL) {
            printf("Memory reallocation failed!\n");
            return;
        }
        students = temp;
        printf("Memory expanded to accommodate more students.\n");
    }
    
    Student newStudent;
    
    // Get student ID
    while (1) {
        newStudent.id = getValidInteger("Enter Student ID: ");
        if (newStudent.id <= 0) {
            printf("Error: ID must be a positive number.\n");
            continue;
        }
        if (!isValidID(newStudent.id)) {
            printf("Error: Student ID already exists!\n");
            continue;
        }
        break;
    }
    
    // Get student name
    printf("Enter Student Name: ");
    clearInputBuffer();
    fgets(newStudent.name, MAX_NAME_LENGTH, stdin);
    newStudent.name[strcspn(newStudent.name, "\n")] = 0;
    
    // Get age
    while (1) {
        newStudent.age = getValidInteger("Enter Age: ");
        if (newStudent.age < 1 || newStudent.age > 150) {
            printf("Error: Invalid age. Please enter a value between 1 and 150.\n");
            continue;
        }
        break;
    }
    
    // Get course
    printf("Enter Course: ");
    fgets(newStudent.course, MAX_COURSE_LENGTH, stdin);
    newStudent.course[strcspn(newStudent.course, "\n")] = 0;
    
    // Get number of subjects
    while (1) {
        newStudent.numSubjects = getValidInteger("Enter number of subjects (1-5): ");
        if (newStudent.numSubjects < 1 || newStudent.numSubjects > MAX_SUBJECTS) {
            printf("Error: Number of subjects must be between 1 and 5.\n");
            continue;
        }
        break;
    }
    
    // Get grades
    printf("Enter grades for each subject:\n");
    for (int i = 0; i < newStudent.numSubjects; i++) {
        while (1) {
            char prompt[50];
            sprintf(prompt, "  Subject %d: ", i + 1);
            newStudent.grades[i] = getValidFloat(prompt);
            if (newStudent.grades[i] < 0 || newStudent.grades[i] > 100) {
                printf("Error: Grade must be between 0 and 100.\n");
                continue;
            }
            break;
        }
    }
    
    // Calculate GPA
    computeGPA(&newStudent);
    
    // Add to array
    students[studentCount] = newStudent;
    studentCount++;
    
    printf("\nStudent added successfully! GPA: %.2f\n", newStudent.gpa);
}

void displayAllStudents() {
    printf("\n========== ALL STUDENTS ==========\n");
    
    if (studentCount == 0) {
        printf("No students in the database.\n");
        return;
    }
    
    printf("%-6s %-25s %-5s %-20s %-8s\n", "ID", "Name", "Age", "Course", "GPA");
    printf("------------------------------------------------------------------------\n");
    
    for (int i = 0; i < studentCount; i++) {
        printf("%-6d %-25s %-5d %-20s %-8.2f\n",
               students[i].id,
               students[i].name,
               students[i].age,
               students[i].course,
               students[i].gpa);
    }
    
    printf("\nTotal Students: %d\n", studentCount);
}

void searchStudent() {
    printf("\n========== SEARCH STUDENT ==========\n");
    printf("1. Search by ID\n");
    printf("2. Search by Name\n");
    int choice = getValidInteger("Enter choice: ");
    
    if (choice == 1) {
        int id = getValidInteger("Enter Student ID: ");
        int index = findStudentIndex(id);
        
        if (index == -1) {
            printf("Student with ID %d not found.\n", id);
            return;
        }
        
        Student* s = &students[index];
        printf("\n--- Student Details ---\n");
        printf("ID: %d\n", s->id);
        printf("Name: %s\n", s->name);
        printf("Age: %d\n", s->age);
        printf("Course: %s\n", s->course);
        printf("GPA: %.2f\n", s->gpa);
        printf("Grades: ");
        for (int i = 0; i < s->numSubjects; i++) {
            printf("%.2f ", s->grades[i]);
        }
        printf("\n");
    } else if (choice == 2) {
        char searchName[MAX_NAME_LENGTH];
        printf("Enter Student Name: ");
        clearInputBuffer();
        fgets(searchName, MAX_NAME_LENGTH, stdin);
        searchName[strcspn(searchName, "\n")] = 0;
        toLowerCase(searchName);
        
        int found = 0;
        for (int i = 0; i < studentCount; i++) {
            char tempName[MAX_NAME_LENGTH];
            strcpy(tempName, students[i].name);
            toLowerCase(tempName);
            
            if (strstr(tempName, searchName) != NULL) {
                Student* s = &students[i];
                printf("\n--- Student Details ---\n");
                printf("ID: %d\n", s->id);
                printf("Name: %s\n", s->name);
                printf("Age: %d\n", s->age);
                printf("Course: %s\n", s->course);
                printf("GPA: %.2f\n", s->gpa);
                found = 1;
            }
        }
        
        if (!found) {
            printf("No students found with name containing '%s'.\n", searchName);
        }
    } else {
        printf("Invalid choice.\n");
    }
}

void updateStudent() {
    printf("\n========== UPDATE STUDENT ==========\n");
    int id = getValidInteger("Enter Student ID to update: ");
    int index = findStudentIndex(id);
    
    if (index == -1) {
        printf("Student with ID %d not found.\n", id);
        return;
    }
    
    Student* s = &students[index];
    printf("\nCurrent Details:\n");
    printf("Name: %s\n", s->name);
    printf("Age: %d\n", s->age);
    printf("Course: %s\n", s->course);
    
    printf("\nWhat would you like to update?\n");
    printf("1. Name\n");
    printf("2. Age\n");
    printf("3. Course\n");
    printf("4. Grades\n");
    printf("5. All details\n");
    
    int choice = getValidInteger("Enter choice: ");
    
    switch (choice) {
        case 1:
            printf("Enter new name: ");
            clearInputBuffer();
            fgets(s->name, MAX_NAME_LENGTH, stdin);
            s->name[strcspn(s->name, "\n")] = 0;
            break;
        case 2:
            while (1) {
                s->age = getValidInteger("Enter new age: ");
                if (s->age < 1 || s->age > 150) {
                    printf("Error: Invalid age.\n");
                    continue;
                }
                break;
            }
            break;
        case 3:
            printf("Enter new course: ");
            clearInputBuffer();
            fgets(s->course, MAX_COURSE_LENGTH, stdin);
            s->course[strcspn(s->course, "\n")] = 0;
            break;
        case 4:
            printf("Enter new grades:\n");
            for (int i = 0; i < s->numSubjects; i++) {
                while (1) {
                    char prompt[50];
                    sprintf(prompt, "  Subject %d: ", i + 1);
                    s->grades[i] = getValidFloat(prompt);
                    if (s->grades[i] < 0 || s->grades[i] > 100) {
                        printf("Error: Grade must be between 0 and 100.\n");
                        continue;
                    }
                    break;
                }
            }
            computeGPA(s);
            break;
        case 5:
            printf("Enter new name: ");
            clearInputBuffer();
            fgets(s->name, MAX_NAME_LENGTH, stdin);
            s->name[strcspn(s->name, "\n")] = 0;
            
            while (1) {
                s->age = getValidInteger("Enter new age: ");
                if (s->age < 1 || s->age > 150) {
                    printf("Error: Invalid age.\n");
                    continue;
                }
                break;
            }
            
            printf("Enter new course: ");
            fgets(s->course, MAX_COURSE_LENGTH, stdin);
            s->course[strcspn(s->course, "\n")] = 0;
            
            printf("Enter new grades:\n");
            for (int i = 0; i < s->numSubjects; i++) {
                while (1) {
                    char prompt[50];
                    sprintf(prompt, "  Subject %d: ", i + 1);
                    s->grades[i] = getValidFloat(prompt);
                    if (s->grades[i] < 0 || s->grades[i] > 100) {
                        printf("Error: Grade must be between 0 and 100.\n");
                        continue;
                    }
                    break;
                }
            }
            computeGPA(s);
            break;
        default:
            printf("Invalid choice.\n");
            return;
    }
    
    printf("\nStudent updated successfully!\n");
}

void deleteStudent() {
    printf("\n========== DELETE STUDENT ==========\n");
    int id = getValidInteger("Enter Student ID to delete: ");
    int index = findStudentIndex(id);
    
    if (index == -1) {
        printf("Student with ID %d not found.\n", id);
        return;
    }
    
    printf("Are you sure you want to delete %s? (y/n): ", students[index].name);
    clearInputBuffer();
    char confirm = getchar();
    
    if (confirm == 'y' || confirm == 'Y') {
        // Shift all students after the deleted one
        for (int i = index; i < studentCount - 1; i++) {
            students[i] = students[i + 1];
        }
        studentCount--;
        printf("Student deleted successfully!\n");
    } else {
        printf("Deletion cancelled.\n");
    }
}

void sortStudents() {
    if (studentCount == 0) {
        printf("No students to sort.\n");
        return;
    }
    
    printf("\n========== SORT STUDENTS ==========\n");
    printf("1. Sort by GPA (Bubble Sort)\n");
    printf("2. Sort by Name (Insertion Sort)\n");
    printf("3. Sort by ID (Insertion Sort)\n");
    
    int choice = getValidInteger("Enter choice: ");
    
    switch (choice) {
        case 1:
            bubbleSort(students, studentCount, 1);
            printf("Students sorted by GPA (descending).\n");
            break;
        case 2:
            insertionSort(students, studentCount, 2);
            printf("Students sorted by Name (alphabetically).\n");
            break;
        case 3:
            insertionSort(students, studentCount, 3);
            printf("Students sorted by ID (ascending).\n");
            break;
        default:
            printf("Invalid choice.\n");
            return;
    }
    
    displayAllStudents();
}

void generateReports() {
    printf("\n========== ANALYTICS & REPORTS ==========\n");
    printf("1. Calculate Statistics (Average, Median, Highest/Lowest GPA)\n");
    printf("2. Top N Students\n");
    printf("3. Top Student Per Course\n");
    printf("4. Course-wise Average GPA\n");
    
    int choice = getValidInteger("Enter choice: ");
    
    switch (choice) {
        case 1:
            calculateStatistics();
            break;
        case 2: {
            int n = getValidInteger("Enter N (number of top students): ");
            topNStudents(n);
            break;
        }
        case 3:
            topStudentPerCourse();
            break;
        case 4:
            courseWiseAverage();
            break;
        default:
            printf("Invalid choice.\n");
    }
}

void saveToFile() {
    FILE* file = fopen(FILENAME, "wb");
    if (file == NULL) {
        printf("Error opening file for writing!\n");
        return;
    }
    
    fwrite(&studentCount, sizeof(int), 1, file);
    fwrite(students, sizeof(Student), studentCount, file);
    
    fclose(file);
    printf("Data saved successfully to %s\n", FILENAME);
}

void loadFromFile() {
    FILE* file = fopen(FILENAME, "rb");
    if (file == NULL) {
        printf("No existing data file found. Starting fresh.\n");
        return;
    }
    
    int count;
    fread(&count, sizeof(int), 1, file);
    
    if (count > capacity) {
        capacity = count + 10;
        Student* temp = (Student*)realloc(students, capacity * sizeof(Student));
        if (temp == NULL) {
            printf("Memory allocation failed!\n");
            fclose(file);
            return;
        }
        students = temp;
    }
    
    fread(students, sizeof(Student), count, file);
    studentCount = count;
    
    fclose(file);
    printf("Data loaded successfully from %s (%d students)\n", FILENAME, studentCount);
}

void computeGPA(Student* student) {
    float sum = 0;
    for (int i = 0; i < student->numSubjects; i++) {
        sum += student->grades[i];
    }
    student->gpa = sum / student->numSubjects;
}

int isValidID(int id) {
    for (int i = 0; i < studentCount; i++) {
        if (students[i].id == id) {
            return 0;
        }
    }
    return 1;
}

int findStudentIndex(int id) {
    for (int i = 0; i < studentCount; i++) {
        if (students[i].id == id) {
            return i;
        }
    }
    return -1;
}

void bubbleSort(Student* arr, int n, int sortBy) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            int swap = 0;
            if (sortBy == 1) { // Sort by GPA (descending)
                if (arr[j].gpa < arr[j + 1].gpa) swap = 1;
            }
            
            if (swap) {
                Student temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void insertionSort(Student* arr, int n, int sortBy) {
    for (int i = 1; i < n; i++) {
        Student key = arr[i];
        int j = i - 1;
        
        while (j >= 0) {
            int shouldMove = 0;
            
            if (sortBy == 2) { // Sort by name
                if (strcmp(arr[j].name, key.name) > 0) shouldMove = 1;
            } else if (sortBy == 3) { // Sort by ID
                if (arr[j].id > key.id) shouldMove = 1;
            }
            
            if (!shouldMove) break;
            
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void calculateStatistics() {
    if (studentCount == 0) {
        printf("No students in the database.\n");
        return;
    }
    
    float sum = 0, highest = students[0].gpa, lowest = students[0].gpa;
    
    for (int i = 0; i < studentCount; i++) {
        sum += students[i].gpa;
        if (students[i].gpa > highest) highest = students[i].gpa;
        if (students[i].gpa < lowest) lowest = students[i].gpa;
    }
    
    float average = sum / studentCount;
    
    // Calculate median
    Student* tempArr = (Student*)malloc(studentCount * sizeof(Student));
    memcpy(tempArr, students, studentCount * sizeof(Student));
    bubbleSort(tempArr, studentCount, 1);
    
    float median;
    if (studentCount % 2 == 0) {
        median = (tempArr[studentCount/2 - 1].gpa + tempArr[studentCount/2].gpa) / 2.0;
    } else {
        median = tempArr[studentCount/2].gpa;
    }
    
    free(tempArr);
    
    printf("\n--- Class Statistics ---\n");
    printf("Average GPA: %.2f\n", average);
    printf("Median GPA: %.2f\n", median);
    printf("Highest GPA: %.2f\n", highest);
    printf("Lowest GPA: %.2f\n", lowest);
    printf("Total Students: %d\n", studentCount);
}

void topNStudents(int n) {
    if (n > studentCount) n = studentCount;
    if (studentCount == 0) {
        printf("No students in the database.\n");
        return;
    }
    
    Student* tempArr = (Student*)malloc(studentCount * sizeof(Student));
    memcpy(tempArr, students, studentCount * sizeof(Student));
    bubbleSort(tempArr, studentCount, 1);
    
    printf("\n--- Top %d Students ---\n", n);
    printf("%-6s %-25s %-20s %-8s\n", "Rank", "Name", "Course", "GPA");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < n && i < studentCount; i++) {
        printf("%-6d %-25s %-20s %-8.2f\n", 
               i + 1, tempArr[i].name, tempArr[i].course, tempArr[i].gpa);
    }
    
    free(tempArr);
}

void topStudentPerCourse() {
    if (studentCount == 0) {
        printf("No students in the database.\n");
        return;
    }
    
    printf("\n--- Top Student Per Course ---\n");
    
    // Find unique courses
    char courses[100][MAX_COURSE_LENGTH];
    int courseCount = 0;
    
    for (int i = 0; i < studentCount; i++) {
        int found = 0;
        for (int j = 0; j < courseCount; j++) {
            if (strcmp(courses[j], students[i].course) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(courses[courseCount], students[i].course);
            courseCount++;
        }
    }
    
    // For each course, find top student
    for (int i = 0; i < courseCount; i++) {
        float maxGPA = -1;
        int topIndex = -1;
        
        for (int j = 0; j < studentCount; j++) {
            if (strcmp(students[j].course, courses[i]) == 0) {
                if (students[j].gpa > maxGPA) {
                    maxGPA = students[j].gpa;
                    topIndex = j;
                }
            }
        }
        
        if (topIndex != -1) {
            printf("Course: %s\n", courses[i]);
            printf("  Top Student: %s (GPA: %.2f)\n\n", 
                   students[topIndex].name, students[topIndex].gpa);
        }
    }
}

void courseWiseAverage() {
    if (studentCount == 0) {
        printf("No students in the database.\n");
        return;
    }
    
    printf("\n--- Course-wise Average GPA ---\n");
    
    char courses[100][MAX_COURSE_LENGTH];
    int courseCount = 0;
    
    for (int i = 0; i < studentCount; i++) {
        int found = 0;
        for (int j = 0; j < courseCount; j++) {
            if (strcmp(courses[j], students[i].course) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(courses[courseCount], students[i].course);
            courseCount++;
        }
    }
    
    for (int i = 0; i < courseCount; i++) {
        float sum = 0;
        int count = 0;
        
        for (int j = 0; j < studentCount; j++) {
            if (strcmp(students[j].course, courses[i]) == 0) {
                sum += students[j].gpa;
                count++;
            }
        }
        
        printf("Course: %s\n", courses[i]);
        printf("  Students: %d\n", count);
        printf("  Average GPA: %.2f\n\n", sum / count);
    }
}

void displayMenu() {
    printf("\n========================================\n");
    printf("           MAIN MENU\n");
    printf("========================================\n");
    printf("1.  Add Student\n");
    printf("2.  Display All Students\n");
    printf("3.  Search Student\n");
    printf("4.  Update Student\n");
    printf("5.  Delete Student\n");
    printf("6.  Sort Students\n");
    printf("7.  Generate Reports\n");
    printf("8.  Save to File\n");
    printf("9.  Load from File\n");
    printf("10. Exit\n");
    printf("========================================\n");
}

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

void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}