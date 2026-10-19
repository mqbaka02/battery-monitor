#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "read_logs.h"
#include <ctime>

int SingleInstantBatteryState::getBatteryStatus() const{
    return batteryStatus;
}
int SingleInstantBatteryState::getPowerStatus() const{
    return powerStatus;
}
int SingleInstantBatteryState::getRemainingTime() const{
    return remainingTime;
}

Timestamp SingleInstantBatteryState::getTimeStamp() const{
    return timestamp;
}

void SingleInstantBatteryState::setBatteryStatus(int arg){
    batteryStatus= arg;
}
void SingleInstantBatteryState::setPowerStatus(int arg){
    powerStatus= arg;
}
void SingleInstantBatteryState::setRemainingTime(int arg){
    remainingTime= arg;
}

void SingleInstantBatteryState::setTimeStamp(Timestamp arg){
    timestamp= arg;
}

SingleInstantBatteryState::SingleInstantBatteryState(int bStatus, int pStatus, int rTime, int hours, int minutes, int seconds, int day, int month, int year){
    Timestamp the_timestamp= {
        hours, minutes, seconds, day, month, year
    };
    timestamp= the_timestamp;
    batteryStatus= bStatus;
    powerStatus= pStatus;
    remainingTime= rTime;
    //
}

void SingleInstantBatteryState::print() const{
    printf("Battery Status: %d\n", batteryStatus);
    printf("Power Status: %d\n", powerStatus);
    printf("Remaining Time: %d\n", remainingTime);
    printf("Date: %d:%d:%d|%d-%d-%d\n", timestamp.hours, timestamp.minutes, timestamp.seconds, timestamp.day, timestamp.month, timestamp.year);
}
void SingleInstantBatteryState::print_simple() const{
    printf("%d, ", batteryStatus);
    printf("%d, ", powerStatus);
    printf("%d, ", remainingTime);
    printf("%d:%d:%d|%d-%d-%d\n", timestamp.hours, timestamp.minutes, timestamp.seconds, timestamp.day, timestamp.month, timestamp.year);
}

SingleInstantBatteryState::~SingleInstantBatteryState(){
    //
}


SingleInstantBatteryState* read_from_log_file(char *filename){

    SingleInstantBatteryState* the_data;
    //Open the file
    //Count the total number of lines
    //Allocate the_data according to how many lines there are
    //Read values line by line
    //Each line is one item,
    //Line is like this batteryStatus, powerStatus, remainingTime, hours, minutes, secconds, day, month, year
    //Append each line to the_data

    return the_data;
}


std::vector<SingleInstantBatteryState> read_from_log_file_vector(char* filename)
{
    std::vector<SingleInstantBatteryState> the_data;

    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << '\n';
        return the_data;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        int batteryStatus, powerStatus, remainingTime;
        int hours, minutes, seconds, day, month, year;
        char comma;

        std::istringstream stream(line);

        if (!(stream >> batteryStatus >> comma
                     >> powerStatus >> comma
                     >> remainingTime >> comma
                     >> hours >> comma
                     >> minutes >> comma
                     >> seconds >> comma
                     >> day >> comma
                     >> month >> comma
                     >> year)) {
            std::cerr << "Warning: Invalid line: " << line << '\n';
            continue;
        }

        Timestamp timestamp{
            hours, minutes, seconds, day, month, year
        };

        the_data.emplace_back(
            batteryStatus,
            powerStatus,
            remainingTime,
            hours,
            minutes,
            seconds,
            day,
            month,
            year
        );
    }

    return the_data;
}

int getSecondsFromTimeStamp(Timestamp t)
{
    return t.seconds + t.minutes * 60 + t.hours * 3600;
}

PairDataWithCount getXYandCount(std::vector<SingleInstantBatteryState> data)
{
    int *x;
    int *y;
    int count= data.size();
    printf("Total count: %d", count);
    PairDataWithCount result= {x, y, count};
    int counter= 0;
    for (const auto& item: data)
    {
        printf("Counter: %d", counter);
        y[counter]= item.getBatteryStatus();
        x[counter]= getSecondsFromTimeStamp(item.getTimeStamp());

        counter++;
    }

    return result;
}

double getEpochFromTimeStamp(Timestamp t)
{
    std::tm tm{};
    tm.tm_sec  = t.seconds;
    tm.tm_min  = t.minutes;
    tm.tm_hour = t.hours;
    tm.tm_mday = t.day;
    tm.tm_mon  = t.month - 1;     // tm_mon is 0-11
    tm.tm_year = t.year - 1900;   // tm_year is years since 1900
    return (double)timegm(&tm);   // treat the log's wall-clock time as UTC
}

PlotSeries getXY(const std::vector<SingleInstantBatteryState>& data)
{
    PlotSeries result;
    result.x.reserve(data.size());
    result.y.reserve(data.size());
    result.power.reserve(data.size());

    for (const auto& item : data) {
        result.x.push_back(getEpochFromTimeStamp(item.getTimeStamp()));
        result.y.push_back(item.getBatteryStatus());
        result.power.push_back(item.getPowerStatus());
    }
    return result;
}