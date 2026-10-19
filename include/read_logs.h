#pragma once
#include<stdio.h>
#include<vector>
#include<cstdio>

typedef struct
{
    int hours;
    int minutes;
    int seconds;
    int day;
    int month;
    int year;
} Timestamp;

typedef struct
{
    int* x; //array
    int *y; //array
    int count;
} PairDataWithCount;

struct PlotSeries {
    std::vector<double> x;
    std::vector<double> y;
    std::vector<int> power;
};


class SingleInstantBatteryState {

    private:
    int batteryStatus;
    int powerStatus; //charging, discharging, fully charged
    int remainingTime; //In seconds?
    Timestamp timestamp;


    public:
    SingleInstantBatteryState(int batteryStatus, int powerStatus, int remainingTime, int hours, int minutes, int secconds, int day, int month, int year);
    int getBatteryStatus() const;
    int getPowerStatus() const;
    int getRemainingTime() const;
    Timestamp getTimeStamp() const;

    void setBatteryStatus(int arg);
    void setPowerStatus(int arg);
    void setRemainingTime(int arg);
    void setTimeStamp(Timestamp arg);

    void print() const;
    void print_simple() const;
    

    ~SingleInstantBatteryState();
};

SingleInstantBatteryState* read_from_log_file(char *filename);
std::vector<SingleInstantBatteryState> read_from_log_file_vector(char *filename);
int getSecondsFromTimeStamp(Timestamp t);
PairDataWithCount getXYandCount(std::vector<SingleInstantBatteryState> s);

PlotSeries getXY(const std::vector<SingleInstantBatteryState>& data);