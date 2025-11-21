#pragma once

#include <string>
#include <vector>
#include <map>

struct WeatherDataInput {
    // This is already set in binary files.
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
    double windDir = 0.0;
    double windSpd = 0.0;
    double currDir = 0.0;
    double currSpd = 0.0;
    double waveDir = 0.0;
    double waveHgt = 0.0;
    double wavePrd = 0.0;
};

struct VoyageInfo {
    double heading = 0.0;
    double shipSpeed = 0.0;
    double draft = 0.0;
    double trim = 0.0;
};