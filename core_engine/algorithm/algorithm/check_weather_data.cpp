//#include <iostream>
//#include <map>
//#include "../../include/read_weather_data.h"
//#include "../../include/parse_weather_data.h"
//
//int main() {
//    std::map<std::string, WeatherDataInput> all_weather_data = storeWeatherData();
//
//    if (all_weather_data.empty()) {
//        std::cerr << "Error: No weather data loaded!" << std::endl;
//        return -1;
//    }
//
//    std::cout << "\n=== Weather Data Grid Information ===" << std::endl;
//    std::cout << std::string(80, '=') << std::endl;
//
//    bool gridConsistent = true;
//    unsigned int refNumLon = 0, refNumLat = 0, refNumTime = 0;
//    std::string refFile = "";
//
//    for (const auto& [fileName, data] : all_weather_data) {
//        std::cout << "\nFile: " << fileName << std::endl;
//        std::cout << "  Time Info:" << std::endl;
//        std::cout << "    - StartTime: " << data.iStartTime << std::endl;
//        std::cout << "    - NumTime:   " << data.iNumTime << std::endl;
//        std::cout << "    - TimeBin:   " << data.iTimeBin << " hours" << std::endl;
//
//        std::cout << "  Longitude Info:" << std::endl;
//        std::cout << "    - StartLon:  " << data.StartLon << "°" << std::endl;
//        std::cout << "    - NumLon:    " << data.iNumLon << std::endl;
//        std::cout << "    - LonBin:    " << data.LonBin << "°" << std::endl;
//
//        std::cout << "  Latitude Info:" << std::endl;
//        std::cout << "    - StartLat:  " << data.StartLat << "°" << std::endl;
//        std::cout << "    - NumLat:    " << data.iNumLat << std::endl;
//        std::cout << "    - LatBin:    " << data.LatBin << "°" << std::endl;
//
//        size_t expectedSize = static_cast<size_t>(data.iNumTime) * data.iNumLon * data.iNumLat;
//        std::cout << "  Data Array Size: " << data.data.size()
//            << " (expected: " << expectedSize << ")" << std::endl;
//
//        if (data.data.size() != expectedSize) {
//            std::cerr << "    WARNING: Data size mismatch!" << std::endl;
//        }
//
//        // Check consistency
//        if (refFile.empty()) {
//            refFile = fileName;
//            refNumLon = data.iNumLon;
//            refNumLat = data.iNumLat;
//            refNumTime = data.iNumTime;
//        }
//        else {
//            if (data.iNumLon != refNumLon || data.iNumLat != refNumLat || data.iNumTime != refNumTime) {
//                std::cerr << "    ERROR: Grid size differs from " << refFile << "!" << std::endl;
//                std::cerr << "      Reference: [" << refNumTime << " x " << refNumLon << " x " << refNumLat << "]" << std::endl;
//                std::cerr << "      This file: [" << data.iNumTime << " x " << data.iNumLon << " x " << data.iNumLat << "]" << std::endl;
//                gridConsistent = false;
//            }
//        }
//    }
//
//    std::cout << "\n" << std::string(80, '=') << std::endl;
//
//    if (gridConsistent) {
//        std::cout << "All weather files have consistent grid dimensions." << std::endl;
//    }
//    else {
//        std::cout << "ERROR: Weather files have INCONSISTENT grid dimensions!" << std::endl;
//        std::cout << "  This will cause the FinalIndex calculation to be WRONG!" << std::endl;
//        std::cout << "  Each weather variable must use the same grid structure." << std::endl;
//    }
//
//    std::cout << std::string(80, '=') << std::endl;
//
//    return 0;
//}