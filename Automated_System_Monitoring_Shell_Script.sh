#!/bin/bash

################################################################################
# Automated System Monitoring Shell Script
# Description: Monitors CPU, memory, disk usage with threshold alerts
# Author: Student Submission
# Date: November 2025
################################################################################

# Configuration file
CONFIG_FILE="monitor_config.txt"
LOG_FILE="system_monitor.log"

# Default thresholds
CPU_THRESHOLD=80
MEMORY_THRESHOLD=80
DISK_THRESHOLD=80

# Monitoring interval (seconds)
MONITOR_INTERVAL=60

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

################################################################################
# Function: Load Configuration
################################################################################
load_config() {
    if [ -f "$CONFIG_FILE" ]; then
        source "$CONFIG_FILE"
        echo -e "${GREEN}Configuration loaded successfully.${NC}"
    else
        # Create default config
        echo "CPU_THRESHOLD=$CPU_THRESHOLD" > "$CONFIG_FILE"
        echo "MEMORY_THRESHOLD=$MEMORY_THRESHOLD" >> "$CONFIG_FILE"
        echo "DISK_THRESHOLD=$DISK_THRESHOLD" >> "$CONFIG_FILE"
        echo -e "${YELLOW}Default configuration created.${NC}"
    fi
}

################################################################################
# Function: Save Configuration
################################################################################
save_config() {
    echo "CPU_THRESHOLD=$CPU_THRESHOLD" > "$CONFIG_FILE"
    echo "MEMORY_THRESHOLD=$MEMORY_THRESHOLD" >> "$CONFIG_FILE"
    echo "DISK_THRESHOLD=$DISK_THRESHOLD" >> "$CONFIG_FILE"
    echo -e "${GREEN}Configuration saved successfully.${NC}"
}

################################################################################
# Function: Get CPU Usage
################################################################################
get_cpu_usage() {
    # Using top command to get CPU usage
    if command -v top &> /dev/null; then
        cpu_usage=$(top -bn1 | grep "Cpu(s)" | sed "s/.*, *\([0-9.]*\)%* id.*/\1/" | awk '{print 100 - $1}')
    else
        # Fallback using mpstat if available
        if command -v mpstat &> /dev/null; then
            cpu_usage=$(mpstat 1 1 | awk '/Average:/ {print 100-$NF}')
        else
            echo "0"
            return
        fi
    fi
    printf "%.2f" "$cpu_usage"
}

################################################################################
# Function: Get Memory Usage
################################################################################
get_memory_usage() {
    if command -v free &> /dev/null; then
        memory_usage=$(free | grep Mem | awk '{printf "%.2f", $3/$2 * 100.0}')
    else
        echo "0"
        return
    fi
    echo "$memory_usage"
}

################################################################################
# Function: Get Disk Usage
################################################################################
get_disk_usage() {
    if command -v df &> /dev/null; then
        disk_usage=$(df -h / | awk 'NR==2 {print $5}' | sed 's/%//')
    else
        echo "0"
        return
    fi
    echo "$disk_usage"
}

################################################################################
# Function: Get System Uptime
################################################################################
get_uptime() {
    if command -v uptime &> /dev/null; then
        uptime | awk -F'( |,|:)+' '{print $6,$7",",$8,"hours,",$9,"minutes"}'
    else
        echo "Unknown"
    fi
}

################################################################################
# Function: Get Top Processes
################################################################################
get_top_processes() {
    if command -v ps &> /dev/null; then
        echo -e "\n${BLUE}Top 5 CPU-consuming processes:${NC}"
        ps aux --sort=-%cpu | head -6 | awk 'NR>1 {printf "  %-20s %5s%%  %10s\n", $11, $3, $4}'
    else
        echo "ps command not available"
    fi
}

################################################################################
# Function: Log Message
################################################################################
log_message() {
    local message="$1"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] $message" >> "$LOG_FILE"
}

################################################################################
# Function: Check Thresholds and Alert
################################################################################
check_thresholds() {
    local cpu=$1
    local memory=$2
    local disk=$3
    local alert_triggered=0
    
    # Check CPU threshold
    if (( $(echo "$cpu > $CPU_THRESHOLD" | bc -l) )); then
        echo -e "${RED}ALERT: CPU usage is ${cpu}% (Threshold: ${CPU_THRESHOLD}%)${NC}"
        log_message "ALERT: CPU usage is ${cpu}% (Threshold: ${CPU_THRESHOLD}%)"
        alert_triggered=1
    fi
    
    # Check Memory threshold
    if (( $(echo "$memory > $MEMORY_THRESHOLD" | bc -l) )); then
        echo -e "${RED}ALERT: Memory usage is ${memory}% (Threshold: ${MEMORY_THRESHOLD}%)${NC}"
        log_message "ALERT: Memory usage is ${memory}% (Threshold: ${MEMORY_THRESHOLD}%)"
        alert_triggered=1
    fi
    
    # Check Disk threshold
    if [ "$disk" -gt "$DISK_THRESHOLD" ]; then
        echo -e "${RED}ALERT: Disk usage is ${disk}% (Threshold: ${DISK_THRESHOLD}%)${NC}"
        log_message "ALERT: Disk usage is ${disk}% (Threshold: ${DISK_THRESHOLD}%)"
        alert_triggered=1
    fi
    
    if [ $alert_triggered -eq 0 ]; then
        echo -e "${GREEN}All systems within normal parameters.${NC}"
    fi
}

################################################################################
# Function: Display System Status
################################################################################
display_status() {
    clear
    echo "================================================================================"
    echo "                    SYSTEM MONITORING DASHBOARD"
    echo "================================================================================"
    echo "Timestamp: $(date '+%Y-%m-%d %H:%M:%S')"
    echo "Hostname: $(hostname)"
    echo "Uptime: $(get_uptime)"
    echo "--------------------------------------------------------------------------------"
    
    # Get metrics
    cpu_usage=$(get_cpu_usage)
    memory_usage=$(get_memory_usage)
    disk_usage=$(get_disk_usage)
    
    # Display CPU
    echo -e "${BLUE}CPU Usage:${NC} ${cpu_usage}%"
    if (( $(echo "$cpu_usage > $CPU_THRESHOLD" | bc -l) )); then
        echo -e "  Status: ${RED}WARNING - Above threshold (${CPU_THRESHOLD}%)${NC}"
    else
        echo -e "  Status: ${GREEN}OK${NC}"
    fi
    
    # Display Memory
    echo -e "${BLUE}Memory Usage:${NC} ${memory_usage}%"
    if (( $(echo "$memory_usage > $MEMORY_THRESHOLD" | bc -l) )); then
        echo -e "  Status: ${RED}WARNING - Above threshold (${MEMORY_THRESHOLD}%)${NC}"
    else
        echo -e "  Status: ${GREEN}OK${NC}"
    fi
    
    # Display Disk
    echo -e "${BLUE}Disk Usage:${NC} ${disk_usage}%"
    if [ "$disk_usage" -gt "$DISK_THRESHOLD" ]; then
        echo -e "  Status: ${RED}WARNING - Above threshold (${DISK_THRESHOLD}%)${NC}"
    else
        echo -e "  Status: ${GREEN}OK${NC}"
    fi
    
    echo "--------------------------------------------------------------------------------"
    
    # Get detailed memory info
    if command -v free &> /dev/null; then
        echo -e "${BLUE}Memory Details:${NC}"
        free -h | awk 'NR==1 {print "  "$0} NR==2 {print "  "$0}'
    fi
    
    echo "--------------------------------------------------------------------------------"
    
    # Get disk info
    if command -v df &> /dev/null; then
        echo -e "${BLUE}Disk Details:${NC}"
        df -h / | awk 'NR==1 {print "  "$0} NR==2 {print "  "$0}'
    fi
    
    # Display top processes
    get_top_processes
    
    echo "================================================================================"
    
    # Log the status
    log_message "Status Check - CPU: ${cpu_usage}%, Memory: ${memory_usage}%, Disk: ${disk_usage}%"
    
    # Check thresholds
    check_thresholds "$cpu_usage" "$memory_usage" "$disk_usage"
}

################################################################################
# Function: Set Thresholds
################################################################################
set_thresholds() {
    echo "================================================================================"
    echo "                        SET ALERT THRESHOLDS"
    echo "================================================================================"
    echo "Current thresholds:"
    echo "  CPU: ${CPU_THRESHOLD}%"
    echo "  Memory: ${MEMORY_THRESHOLD}%"
    echo "  Disk: ${DISK_THRESHOLD}%"
    echo "--------------------------------------------------------------------------------"
    
    # Get CPU threshold
    while true; do
        read -p "Enter CPU threshold (0-100) [current: $CPU_THRESHOLD]: " new_cpu
        if [ -z "$new_cpu" ]; then
            break
        elif [[ "$new_cpu" =~ ^[0-9]+$ ]] && [ "$new_cpu" -ge 0 ] && [ "$new_cpu" -le 100 ]; then
            CPU_THRESHOLD=$new_cpu
            break
        else
            echo -e "${RED}Invalid input. Please enter a number between 0 and 100.${NC}"
        fi
    done
    
    # Get Memory threshold
    while true; do
        read -p "Enter Memory threshold (0-100) [current: $MEMORY_THRESHOLD]: " new_mem
        if [ -z "$new_mem" ]; then
            break
        elif [[ "$new_mem" =~ ^[0-9]+$ ]] && [ "$new_mem" -ge 0 ] && [ "$new_mem" -le 100 ]; then
            MEMORY_THRESHOLD=$new_mem
            break
        else
            echo -e "${RED}Invalid input. Please enter a number between 0 and 100.${NC}"
        fi
    done
    
    # Get Disk threshold
    while true; do
        read -p "Enter Disk threshold (0-100) [current: $DISK_THRESHOLD]: " new_disk
        if [ -z "$new_disk" ]; then
            break
        elif [[ "$new_disk" =~ ^[0-9]+$ ]] && [ "$new_disk" -ge 0 ] && [ "$new_disk" -le 100 ]; then
            DISK_THRESHOLD=$new_disk
            break
        else
            echo -e "${RED}Invalid input. Please enter a number between 0 and 100.${NC}"
        fi
    done
    
    save_config
    echo -e "${GREEN}Thresholds updated successfully!${NC}"
    log_message "Thresholds updated - CPU: ${CPU_THRESHOLD}%, Memory: ${MEMORY_THRESHOLD}%, Disk: ${DISK_THRESHOLD}%"
}

################################################################################
# Function: View Logs
################################################################################
view_logs() {
    echo "================================================================================"
    echo "                           SYSTEM LOGS"
    echo "================================================================================"
    
    if [ ! -f "$LOG_FILE" ]; then
        echo "No logs found. The log file will be created when monitoring starts."
        return
    fi
    
    echo "How many recent entries would you like to view?"
    read -p "Enter number (or 'all' for complete log): " num_lines
    
    if [ "$num_lines" = "all" ]; then
        cat "$LOG_FILE"
    elif [[ "$num_lines" =~ ^[0-9]+$ ]]; then
        tail -n "$num_lines" "$LOG_FILE"
    else
        echo -e "${RED}Invalid input. Showing last 20 entries.${NC}"
        tail -n 20 "$LOG_FILE"
    fi
    
    echo "================================================================================"
}

################################################################################
# Function: Clear Logs
################################################################################
clear_logs() {
    read -p "Are you sure you want to clear all logs? (yes/no): " confirm
    if [ "$confirm" = "yes" ] || [ "$confirm" = "y" ]; then
        > "$LOG_FILE"
        echo -e "${GREEN}Logs cleared successfully.${NC}"
        log_message "Log file cleared by user"
    else
        echo "Operation cancelled."
    fi
}

################################################################################
# Function: Continuous Monitoring Mode
################################################################################
continuous_monitoring() {
    echo "================================================================================"
    echo "Starting continuous monitoring (Press Ctrl+C to stop)..."
    echo "Monitoring interval: ${MONITOR_INTERVAL} seconds"
    echo "================================================================================"
    
    log_message "Continuous monitoring started"
    
    while true; do
        display_status
        echo ""
        echo "Next check in ${MONITOR_INTERVAL} seconds... (Press Ctrl+C to return to menu)"
        sleep "$MONITOR_INTERVAL"
    done
}

################################################################################
# Function: Display Menu
################################################################################
display_menu() {
    echo ""
    echo "================================================================================"
    echo "                    SYSTEM MONITORING MENU"
    echo "================================================================================"
    echo "1. View system status (one-time check)"
    echo "2. Set alert thresholds"
    echo "3. View logs"
    echo "4. Clear logs"
    echo "5. Start continuous monitoring"
    echo "6. Exit"
    echo "================================================================================"
}

################################################################################
# Function: Main Program
################################################################################
main() {
    # Check if required commands are available
    local missing_commands=0
    
    if ! command -v bc &> /dev/null; then
        echo -e "${YELLOW}Warning: 'bc' not found. Some calculations may be limited.${NC}"
        missing_commands=1
    fi
    
    # Load configuration
    load_config
    
    # Log script start
    log_message "System monitoring script started"
    
    # Main menu loop
    while true; do
        display_menu
        read -p "Enter your choice (1-6): " choice
        
        case $choice in
            1)
                display_status
                read -p "Press Enter to continue..."
                ;;
            2)
                set_thresholds
                read -p "Press Enter to continue..."
                ;;
            3)
                view_logs
                read -p "Press Enter to continue..."
                ;;
            4)
                clear_logs
                read -p "Press Enter to continue..."
                ;;
            5)
                continuous_monitoring
                ;;
            6)
                echo "Exiting system monitor..."
                log_message "System monitoring script exited"
                exit 0
                ;;
            *)
                echo -e "${RED}Invalid choice. Please enter a number between 1 and 6.${NC}"
                read -p "Press Enter to continue..."
                ;;
        esac
    done
}

################################################################################
# Script Entry Point
################################################################################

# Check if running with bash
if [ -z "$BASH_VERSION" ]; then
    echo "This script requires bash. Please run with: bash $0"
    exit 1
fi

# Start main program
main