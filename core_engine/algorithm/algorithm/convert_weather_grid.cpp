//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <string>
//#include <cstring>
//#include <direct.h>
//
//// Weather data header structure
//struct WeatherHeader {
//    unsigned int iStartTime;
//    unsigned int iNumTime;
//    unsigned int iTimeBin;
//    float StartLon;
//    unsigned int iNumLon;
//    float LonBin;
//    float StartLat;
//    unsigned int iNumLat;
//    float LatBin;
//};
//
//bool convertWeatherFile(const std::string& inputPath, const std::string& outputPath) {
//    std::string fileName = inputPath.substr(inputPath.find_last_of("/\\") + 1);
//    std::cout << "\n=== Processing: " << fileName << " ===" << std::endl;
//
//    // Open input file
//    FILE* fpIn = nullptr;
//#ifdef _WIN32
//    errno_t err = fopen_s(&fpIn, inputPath.c_str(), "rb");
//    if (err || !fpIn) {
//#else
//    fpIn = fopen(inputPath.c_str(), "rb");
//    if (!fpIn) {
//#endif
//        std::cerr << "ERROR: Cannot open input file: " << inputPath << std::endl;
//        return false;
//    }
//
//    // Read header
//    WeatherHeader header;
//    fread(&header.iStartTime, sizeof(unsigned int), 1, fpIn);
//    fread(&header.iNumTime, sizeof(unsigned int), 1, fpIn);
//    fread(&header.iTimeBin, sizeof(unsigned int), 1, fpIn);
//    fread(&header.StartLon, sizeof(float), 1, fpIn);
//    fread(&header.iNumLon, sizeof(unsigned int), 1, fpIn);
//    fread(&header.LonBin, sizeof(float), 1, fpIn);
//    fread(&header.StartLat, sizeof(float), 1, fpIn);
//    fread(&header.iNumLat, sizeof(unsigned int), 1, fpIn);
//    fread(&header.LatBin, sizeof(float), 1, fpIn);
//
//    std::cout << "Original Grid: [" << header.iNumTime << " ¡¿ "
//        << header.iNumLon << " ¡¿ " << header.iNumLat << "]" << std::endl;
//    std::cout << "Latitude: " << header.StartLat << "¡Æ to "
//        << (header.StartLat + (header.iNumLat - 1) * header.LatBin) << "¡Æ" << std::endl;
//
//    // Check if conversion is needed
//    if (header.iNumLat == 341) {
//        std::cout << "INFO: Already 341-grid. Skipping conversion." << std::endl;
//        fclose(fpIn);
//        return true;  // Not an error, just skip
//    }
//
//    if (header.iNumLat != 361) {
//        std::cerr << "ERROR: Expected NumLat=361 for conversion, got " << header.iNumLat << std::endl;
//        fclose(fpIn);
//        return false;
//    }
//
//    // Read all data
//    size_t totalElements = static_cast<size_t>(header.iNumTime) * header.iNumLon * header.iNumLat;
//    std::vector<float> inputData(totalElements);
//    size_t readCount = fread(inputData.data(), sizeof(float), totalElements, fpIn);
//    fclose(fpIn);
//
//    if (readCount != totalElements) {
//        std::cerr << "ERROR: Read " << readCount << " elements, expected " << totalElements << std::endl;
//        return false;
//    }
//
//    std::cout << "Read " << totalElements << " data points successfully." << std::endl;
//
//    // Prepare output data (341 lat instead of 361)
//    const unsigned int NEW_NUM_LAT = 341;
//    const unsigned int OLD_NUM_LAT = 361;
//    const unsigned int REMOVE_COUNT = 20;  // Remove last 20 latitude points
//
//    size_t newTotalElements = static_cast<size_t>(header.iNumTime) * header.iNumLon * NEW_NUM_LAT;
//    std::vector<float> outputData;
//    outputData.reserve(newTotalElements);
//
//    std::cout << "Converting: Removing last " << REMOVE_COUNT << " latitude points..." << std::endl;
//    std::cout << "New Latitude range: " << header.StartLat << "¡Æ to "
//        << (header.StartLat + (NEW_NUM_LAT - 1) * header.LatBin) << "¡Æ" << std::endl;
//
//    // Reshape data: [iT][iLon][361] -> [iT][iLon][341]
//    // Keep first 341 lat points, discard last 20
//    for (unsigned int t = 0; t < header.iNumTime; ++t) {
//        for (unsigned int lon = 0; lon < header.iNumLon; ++lon) {
//            // Calculate starting index for this [t][lon] slice
//            size_t inputSliceStart = (t * header.iNumLon + lon) * OLD_NUM_LAT;
//
//            // Copy first 341 lat points (discard last 20)
//            for (unsigned int lat = 0; lat < NEW_NUM_LAT; ++lat) {
//                outputData.push_back(inputData[inputSliceStart + lat]);
//            }
//        }
//
//        // Progress indicator
//        if ((t + 1) % 50 == 0 || t == header.iNumTime - 1) {
//            std::cout << "  Progress: " << (t + 1) << "/" << header.iNumTime
//                << " time steps (" << ((t + 1) * 100 / header.iNumTime) << "%)" << std::endl;
//        }
//    }
//
//    std::cout << "Conversion complete. Output data size: " << outputData.size() << std::endl;
//
//    // Update header
//    header.iNumLat = NEW_NUM_LAT;
//
//    // Write output file
//    FILE* fpOut = nullptr;
//#ifdef _WIN32
//    err = fopen_s(&fpOut, outputPath.c_str(), "wb");
//    if (err || !fpOut) {
//#else
//    fpOut = fopen(outputPath.c_str(), "wb");
//    if (!fpOut) {
//#endif
//        std::cerr << "ERROR: Cannot create output file: " << outputPath << std::endl;
//        return false;
//    }
//
//    // Write updated header
//    fwrite(&header.iStartTime, sizeof(unsigned int), 1, fpOut);
//    fwrite(&header.iNumTime, sizeof(unsigned int), 1, fpOut);
//    fwrite(&header.iTimeBin, sizeof(unsigned int), 1, fpOut);
//    fwrite(&header.StartLon, sizeof(float), 1, fpOut);
//    fwrite(&header.iNumLon, sizeof(unsigned int), 1, fpOut);
//    fwrite(&header.LonBin, sizeof(float), 1, fpOut);
//    fwrite(&header.StartLat, sizeof(float), 1, fpOut);
//    fwrite(&header.iNumLat, sizeof(unsigned int), 1, fpOut);
//    fwrite(&header.LatBin, sizeof(float), 1, fpOut);
//
//    // Write data
//    size_t writeCount = fwrite(outputData.data(), sizeof(float), outputData.size(), fpOut);
//    fclose(fpOut);
//
//    if (writeCount != outputData.size()) {
//        std::cerr << "ERROR: Wrote " << writeCount << " elements, expected " << outputData.size() << std::endl;
//        return false;
//    }
//
//    std::cout << "Successfully wrote: " << outputPath << std::endl;
//    std::cout << "New Grid: [" << header.iNumTime << " ¡¿ "
//        << header.iNumLon << " ¡¿ " << header.iNumLat << "]" << std::endl;
//
//    return true;
//    }
//
//int main(int argc, char* argv[]) {
//    std::cout << "===Weather Data Grid Converter: 361-grid ¡æ 341-grid ===" << std::endl;
//    std::cout << "===Removes last 20 latitude points (South Pole region) ===" << std::endl;
//
//    // Get base path
//    std::string base_path;
//
//#ifdef _WIN32
//    char* weather_data_path_env = nullptr;
//    size_t len = 0;
//    if (_dupenv_s(&weather_data_path_env, &len, "WEATHER_DATA_PATH") == 0 && weather_data_path_env != nullptr) {
//        base_path = weather_data_path_env;
//        free(weather_data_path_env);
//    }
//#else
//    char* weather_data_path_env = getenv("WEATHER_DATA_PATH");
//    if (weather_data_path_env != nullptr) {
//        base_path = weather_data_path_env;
//    }
//#endif
//
//    if (base_path.empty()) {
//        std::cerr << "\nERROR: WEATHER_DATA_PATH environment variable not set!" << std::endl;
//        std::cerr << "Please set it to the directory containing weather data files." << std::endl;
//        return -1;
//    }
//
//    std::cout << "\nWeather data directory: " << base_path << std::endl;
//
//    // Create backup directory
//    std::string backup_path = base_path + "backup_361grid/";
//
//#ifdef _WIN32
//    _mkdir(backup_path.c_str());
//#else
//    mkdir(backup_path.c_str(), 0755);
//#endif
//
//    std::cout << "Backup directory: " << backup_path << std::endl;
//
//    // Files to convert (Wind and Wave data)
//    std::vector<std::string> filesToConvert = {
//        "WindDir.bin",
//        "WindSpd.bin",
//        "WaveDir.bin",
//        "WaveHgt.bin",
//        "WavePrd.bin"
//    };
//
//    std::cout << "===Step 1: Backup original files ===" << std::endl;
//
//    for (const auto& fileName : filesToConvert) {
//        std::string srcPath = base_path + fileName;
//        std::string backupFilePath = backup_path + fileName;
//
//        // Check if file exists
//        std::ifstream checkFile(srcPath, std::ios::binary);
//        if (!checkFile.good()) {
//            std::cerr << "WARNING: File not found: " << srcPath << std::endl;
//            continue;
//        }
//        checkFile.close();
//
//        // Copy to backup
//        std::ifstream src(srcPath, std::ios::binary);
//        std::ofstream dst(backupFilePath, std::ios::binary);
//
//        if (!src || !dst) {
//            std::cerr << "ERROR: Cannot create backup for " << fileName << std::endl;
//            return -1;
//        }
//
//        dst << src.rdbuf();
//        src.close();
//        dst.close();
//
//        std::cout << "Backed up: " << fileName << std::endl;
//    }
//
//    std::cout << "==Step 2: Convert files to 341-grid ===" << std::endl;
//
//    int successCount = 0;
//    int skipCount = 0;
//
//    for (const auto& fileName : filesToConvert) {
//        std::string inputPath = base_path + fileName;
//        std::string tempOutputPath = base_path + fileName + ".tmp";
//
//        if (convertWeatherFile(inputPath, tempOutputPath)) {
//            // Check if it was skipped (already 341)
//            std::ifstream check(tempOutputPath, std::ios::binary);
//            if (!check.good()) {
//                skipCount++;
//            }
//            else {
//                check.close();
//                // Replace original with converted file
//#ifdef _WIN32
//                _unlink(inputPath.c_str());
//#else
//                unlink(inputPath.c_str());
//#endif
//                rename(tempOutputPath.c_str(), inputPath.c_str());
//                successCount++;
//            }
//        }
//        else {
//            std::cerr << "FAILED: " << fileName << std::endl;
//        }
//    }
//
//    std::cout << "===Conversion Complete===" << std::endl;
//    std::cout << "\nSummary:" << std::endl;
//    std::cout << "  - Files converted: " << successCount << std::endl;
//    std::cout << "  - Files skipped (already 341-grid): " << skipCount << std::endl;
//    std::cout << "  - Backup location: " << backup_path << std::endl;
//
//    std::cout << "\nAll Wind/Wave files now have 341-grid (matching Current data)" << std::endl;
//    std::cout << "Latitude range: 90¡Æ to -80¡Æ (341 points, 0.5¡Æ spacing)" << std::endl;
//    std::cout << "\nNext steps:" << std::endl;
//    std::cout << "  1. Run check_weather_grid to verify consistency" << std::endl;
//    std::cout << "  2. Remove workaround code (use original parse_weather_data.cpp)" << std::endl;
//    std::cout << "  3. Recompile and test your navigation system" << std::endl;
//
//    return 0;
//}