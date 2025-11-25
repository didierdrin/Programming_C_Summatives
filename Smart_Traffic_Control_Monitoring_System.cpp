/*
 * Smart Traffic Control and Monitoring System
 * Description: Intelligent traffic intersection with dynamic timing
 * Author: Student Submission
 * Date: November 2025
 */

#include <stdlib.h>
#include <string.h>

// Pin Definitions
#define NS_RED 2
#define NS_YELLOW 3
#define NS_GREEN 4
#define EW_RED 5
#define EW_YELLOW 6
#define EW_GREEN 7
#define NS_SENSOR 8
#define EW_SENSOR 9

// Timing Constants (milliseconds)
#define MIN_GREEN_TIME 3000
#define MAX_GREEN_TIME 10000
#define YELLOW_TIME 2000
#define RED_CLEARANCE 1000
#define EMERGENCY_OVERRIDE_TIME 15000

// Traffic States
typedef enum {
    STATE_NS_GREEN,
    STATE_NS_YELLOW,
    STATE_EW_GREEN,
    STATE_EW_YELLOW,
    STATE_EMERGENCY
} TrafficState;

// Structure for Traffic Lane
typedef struct {
    char laneID[10];
    int redPin;
    int yellowPin;
    int greenPin;
    int sensorPin;
    int vehicleCount;
    unsigned long totalWaitTime;
    unsigned long lastDetectionTime;
    bool vehiclePresent;
} TrafficLane;

// Structure for Traffic Log Entry
typedef struct {
    char laneID[10];
    unsigned long timestamp;
    int vehicleCount;
    unsigned long waitTime;
    char state[20];
} LogEntry;

// Global Variables
TrafficLane* nsLane = NULL;
TrafficLane* ewLane = NULL;
LogEntry* logBuffer = NULL;
int logBufferSize = 0;
int logCount = 0;

TrafficState currentState = STATE_NS_GREEN;
unsigned long stateStartTime = 0;
unsigned long currentGreenTime = MIN_GREEN_TIME;
bool emergencyMode = false;
unsigned long emergencyStartTime = 0;

// Function Prototypes
void initializeSystem();
void createLane(TrafficLane** lane, const char* id, int r, int y, int g, int s);
void freeLane(TrafficLane* lane);
void detectVehicles();
void updateTrafficSignals();
void dynamicTimingAdjustment();
void logTrafficData(const char* laneID, const char* state);
void addLogEntry(const char* laneID, unsigned long ts, int count, unsigned long wait, const char* state);
void setLights(TrafficLane* lane, bool red, bool yellow, bool green);
void checkErrorStates();
void printMenu();
void handleSerialCommands();
void printStatistics();
void printLogs();
void enterEmergencyMode();

void setup() {
    Serial.begin(9600);
    initializeSystem();
    Serial.println("\n=== Smart Traffic Control System Started ===");
    Serial.println("Type 'help' for available commands\n");
    printMenu();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Handle emergency mode timeout
    if (emergencyMode && (currentTime - emergencyStartTime >= EMERGENCY_OVERRIDE_TIME)) {
        emergencyMode = false;
        currentState = STATE_NS_GREEN;
        stateStartTime = currentTime;
        Serial.println("Emergency mode ended. Returning to normal operation.");
    }
    
    // Handle serial commands
    handleSerialCommands();
    
    // Detect vehicles (non-blocking)
    detectVehicles();
    
    // Update traffic signals based on state
    updateTrafficSignals();
    
    // Check for error states
    checkErrorStates();
    
    delay(50); // Small delay for stability
}

void initializeSystem() {
    // Initialize lanes with dynamic memory
    createLane(&nsLane, "NS", NS_RED, NS_YELLOW, NS_GREEN, NS_SENSOR);
    createLane(&ewLane, "EW", EW_RED, EW_YELLOW, EW_GREEN, EW_SENSOR);
    
    // Initialize log buffer
    logBufferSize = 10;
    logBuffer = (LogEntry*)malloc(logBufferSize * sizeof(LogEntry));
    if (logBuffer == NULL) {
        Serial.println("ERROR: Failed to allocate memory for log buffer!");
        return;
    }
    
    // Set initial state
    stateStartTime = millis();
    setLights(nsLane, false, false, true);  // NS Green
    setLights(ewLane, true, false, false);  // EW Red
}

void createLane(TrafficLane** lane, const char* id, int r, int y, int g, int s) {
    *lane = (TrafficLane*)malloc(sizeof(TrafficLane));
    if (*lane == NULL) {
        Serial.println("ERROR: Failed to allocate memory for lane!");
        return;
    }
    
    strcpy((*lane)->laneID, id);
    (*lane)->redPin = r;
    (*lane)->yellowPin = y;
    (*lane)->greenPin = g;
    (*lane)->sensorPin = s;
    (*lane)->vehicleCount = 0;
    (*lane)->totalWaitTime = 0;
    (*lane)->lastDetectionTime = 0;
    (*lane)->vehiclePresent = false;
    
    // Configure pins
    pinMode(r, OUTPUT);
    pinMode(y, OUTPUT);
    pinMode(g, OUTPUT);
    pinMode(s, INPUT);
    
    // Initialize all lights off
    digitalWrite(r, LOW);
    digitalWrite(y, LOW);
    digitalWrite(g, LOW);
}

void freeLane(TrafficLane* lane) {
    if (lane != NULL) {
        free(lane);
    }
}

void detectVehicles() {
    unsigned long currentTime = millis();
    
    // Check NS sensor
    bool nsDetection = digitalRead(NS_SENSOR);
    if (nsDetection && !nsLane->vehiclePresent) {
        nsLane->vehicleCount++;
        nsLane->lastDetectionTime = currentTime;
        nsLane->vehiclePresent = true;
        Serial.print("Vehicle detected at NS intersection. Count: ");
        Serial.println(nsLane->vehicleCount);
    } else if (!nsDetection) {
        nsLane->vehiclePresent = false;
    }
    
    // Check EW sensor
    bool ewDetection = digitalRead(EW_SENSOR);
    if (ewDetection && !ewLane->vehiclePresent) {
        ewLane->vehicleCount++;
        ewLane->lastDetectionTime = currentTime;
        ewLane->vehiclePresent = true;
        Serial.print("Vehicle detected at EW intersection. Count: ");
        Serial.println(ewLane->vehicleCount);
    } else if (!ewDetection) {
        ewLane->vehiclePresent = false;
    }
}

void updateTrafficSignals() {
    if (emergencyMode) {
        // All red in emergency mode
        setLights(nsLane, true, false, false);
        setLights(ewLane, true, false, false);
        return;
    }
    
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - stateStartTime;
    
    switch (currentState) {
        case STATE_NS_GREEN:
            if (elapsedTime >= currentGreenTime) {
                currentState = STATE_NS_YELLOW;
                stateStartTime = currentTime;
                setLights(nsLane, false, true, false);
                logTrafficData("NS", "YELLOW");
                Serial.println("NS: GREEN -> YELLOW");
            }
            break;
            
        case STATE_NS_YELLOW:
            if (elapsedTime >= YELLOW_TIME) {
                currentState = STATE_EW_GREEN;
                stateStartTime = currentTime;
                setLights(nsLane, true, false, false);
                setLights(ewLane, false, false, true);
                dynamicTimingAdjustment();
                logTrafficData("NS", "RED");
                logTrafficData("EW", "GREEN");
                Serial.println("NS: YELLOW -> RED | EW: RED -> GREEN");
            }
            break;
            
        case STATE_EW_GREEN:
            if (elapsedTime >= currentGreenTime) {
                currentState = STATE_EW_YELLOW;
                stateStartTime = currentTime;
                setLights(ewLane, false, true, false);
                logTrafficData("EW", "YELLOW");
                Serial.println("EW: GREEN -> YELLOW");
            }
            break;
            
        case STATE_EW_YELLOW:
            if (elapsedTime >= YELLOW_TIME) {
                currentState = STATE_NS_GREEN;
                stateStartTime = currentTime;
                setLights(ewLane, true, false, false);
                setLights(nsLane, false, false, true);
                dynamicTimingAdjustment();
                logTrafficData("EW", "RED");
                logTrafficData("NS", "GREEN");
                Serial.println("EW: YELLOW -> RED | NS: RED -> GREEN");
            }
            break;
    }
}

void dynamicTimingAdjustment() {
    // Adjust green time based on vehicle density
    int nsCount = nsLane->vehicleCount;
    int ewCount = ewLane->vehicleCount;
    
    if (currentState == STATE_NS_GREEN || currentState == STATE_NS_YELLOW) {
        // Calculate EW green time based on EW vehicle count
        if (ewCount > 10) {
            currentGreenTime = MAX_GREEN_TIME;
        } else if (ewCount > 5) {
            currentGreenTime = 7000;
        } else if (ewCount > 2) {
            currentGreenTime = 5000;
        } else {
            currentGreenTime = MIN_GREEN_TIME;
        }
    } else {
        // Calculate NS green time based on NS vehicle count
        if (nsCount > 10) {
            currentGreenTime = MAX_GREEN_TIME;
        } else if (nsCount > 5) {
            currentGreenTime = 7000;
        } else if (nsCount > 2) {
            currentGreenTime = 5000;
        } else {
            currentGreenTime = MIN_GREEN_TIME;
        }
    }
    
    Serial.print("Dynamic timing adjusted: ");
    Serial.print(currentGreenTime / 1000);
    Serial.println(" seconds");
}

void setLights(TrafficLane* lane, bool red, bool yellow, bool green) {
    digitalWrite(lane->redPin, red ? HIGH : LOW);
    digitalWrite(lane->yellowPin, yellow ? HIGH : LOW);
    digitalWrite(lane->greenPin, green ? HIGH : LOW);
}

void checkErrorStates() {
    // Check for simultaneous green lights (error condition)
    bool nsGreen = digitalRead(NS_GREEN);
    bool ewGreen = digitalRead(EW_GREEN);
    
    if (nsGreen && ewGreen) {
        Serial.println("ERROR: Both intersections showing GREEN! Safety violation!");
        // Force both to red
        setLights(nsLane, true, false, false);
        setLights(ewLane, true, false, false);
        emergencyMode = true;
        emergencyStartTime = millis();
    }
}

void logTrafficData(const char* laneID, const char* state) {
    unsigned long timestamp = millis() / 1000; // Convert to seconds
    TrafficLane* lane = (strcmp(laneID, "NS") == 0) ? nsLane : ewLane;
    
    unsigned long waitTime = 0;
    if (lane->lastDetectionTime > 0) {
        waitTime = (millis() - lane->lastDetectionTime) / 1000;
    }
    
    addLogEntry(laneID, timestamp, lane->vehicleCount, waitTime, state);
}

void addLogEntry(const char* laneID, unsigned long ts, int count, unsigned long wait, const char* state) {
    // Expand buffer if needed
    if (logCount >= logBufferSize) {
        logBufferSize *= 2;
        LogEntry* newBuffer = (LogEntry*)realloc(logBuffer, logBufferSize * sizeof(LogEntry));
        if (newBuffer == NULL) {
            Serial.println("ERROR: Failed to reallocate log buffer!");
            return;
        }
        logBuffer = newBuffer;
    }
    
    // Add new entry
    strcpy(logBuffer[logCount].laneID, laneID);
    logBuffer[logCount].timestamp = ts;
    logBuffer[logCount].vehicleCount = count;
    logBuffer[logCount].waitTime = wait;
    strcpy(logBuffer[logCount].state, state);
    logCount++;
}

void handleSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();
        
        if (command == "help" || command == "menu") {
            printMenu();
        } else if (command == "status") {
            printStatistics();
        } else if (command == "logs") {
            printLogs();
        } else if (command == "emergency") {
            enterEmergencyMode();
        } else if (command == "reset") {
            nsLane->vehicleCount = 0;
            ewLane->vehicleCount = 0;
            logCount = 0;
            Serial.println("System reset. All counters cleared.");
        } else {
            Serial.println("Unknown command. Type 'help' for available commands.");
        }
    }
}

void printMenu() {
    Serial.println("\n========== MENU ==========");
    Serial.println("status    - Show current signal states and statistics");
    Serial.println("logs      - Display traffic logs");
    Serial.println("emergency - Activate emergency mode (all red)");
    Serial.println("reset     - Reset vehicle counters and logs");
    Serial.println("help      - Show this menu");
    Serial.println("==========================\n");
}

void printStatistics() {
    Serial.println("\n===== CURRENT STATUS =====");
    Serial.print("System Time: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    
    Serial.print("Current State: ");
    switch (currentState) {
        case STATE_NS_GREEN: Serial.println("NS GREEN / EW RED"); break;
        case STATE_NS_YELLOW: Serial.println("NS YELLOW / EW RED"); break;
        case STATE_EW_GREEN: Serial.println("EW GREEN / NS RED"); break;
        case STATE_EW_YELLOW: Serial.println("EW YELLOW / NS RED"); break;
        case STATE_EMERGENCY: Serial.println("EMERGENCY MODE"); break;
    }
    
    Serial.println("\nNorth-South Intersection:");
    Serial.print("  Total Vehicles: ");
    Serial.println(nsLane->vehicleCount);
    Serial.print("  Red Light: ");
    Serial.println(digitalRead(NS_RED) ? "ON" : "OFF");
    Serial.print("  Yellow Light: ");
    Serial.println(digitalRead(NS_YELLOW) ? "ON" : "OFF");
    Serial.print("  Green Light: ");
    Serial.println(digitalRead(NS_GREEN) ? "ON" : "OFF");
    
    Serial.println("\nEast-West Intersection:");
    Serial.print("  Total Vehicles: ");
    Serial.println(ewLane->vehicleCount);
    Serial.print("  Red Light: ");
    Serial.println(digitalRead(EW_RED) ? "ON" : "OFF");
    Serial.print("  Yellow Light: ");
    Serial.println(digitalRead(EW_YELLOW) ? "ON" : "OFF");
    Serial.print("  Green Light: ");
    Serial.println(digitalRead(EW_GREEN) ? "ON" : "OFF");
    
    Serial.println("==========================\n");
}

void printLogs() {
    Serial.println("\n===== TRAFFIC LOGS =====");
    Serial.println("Time(s) | Lane | Vehicles | Wait(s) | State");
    Serial.println("--------|------|----------|---------|--------");
    
    int displayCount = (logCount > 20) ? 20 : logCount;
    int startIndex = (logCount > 20) ? (logCount - 20) : 0;
    
    for (int i = startIndex; i < logCount; i++) {
        Serial.print(logBuffer[i].timestamp);
        Serial.print("     | ");
        Serial.print(logBuffer[i].laneID);
        Serial.print("   | ");
        Serial.print(logBuffer[i].vehicleCount);
        Serial.print("        | ");
        Serial.print(logBuffer[i].waitTime);
        Serial.print("       | ");
        Serial.println(logBuffer[i].state);
    }
    
    Serial.println("========================\n");
}

void enterEmergencyMode() {
    emergencyMode = true;
    emergencyStartTime = millis();
    currentState = STATE_EMERGENCY;
    setLights(nsLane, true, false, false);
    setLights(ewLane, true, false, false);
    Serial.println("EMERGENCY MODE ACTIVATED - All lights RED");
    Serial.println("Will auto-resume in 15 seconds");
}