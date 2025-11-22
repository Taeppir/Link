#pragma once

#include <string>
#include <vector>
#include <map>

struct WeatherDataInput {
    // Binary file metadata
    unsigned int iStartTime = 0;
    unsigned int iNumTime = 0;
    unsigned int iTimeBin = 0;
    float StartLon = 0.0f;
    unsigned int iNumLon = 0;
    float LonBin = 0.0f;
    float StartLat = 0.0f;
    unsigned int iNumLat = 0;
    float LatBin = 0.0f;
    std::vector<float> data;
};

struct Weather {
    double windDir = 0.0;   // degrees
    double windSpd = 0.0;   // m/s
    double currDir = 0.0;   // degrees
    double currSpd = 0.0;   // m/s
    double waveDir = 0.0;   // degrees
    double waveHgt = 0.0;   // meters
    double wavePrd = 0.0;   // seconds
};