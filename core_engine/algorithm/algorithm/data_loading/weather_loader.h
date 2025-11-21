#pragma once
#include "../types/weather_types.h"
#include <map>
#include <string>

/**
 * @class WeatherLoader
 * @brief 날씨 데이터 로딩 클래스
 * 
 * DSME 포맷 바이너리 파일(.bin)을 읽어서 날씨 데이터를 로드합니다.
 * 환경변수 WEATHER_DATA_PATH를 사용하거나 직접 경로를 지정할 수 있습니다.
 */
class WeatherLoader {
public:
    WeatherLoader();
    ~WeatherLoader();
    
    /**
     * @brief 날씨 데이터 로드 (환경변수 사용)
     * 
     * WEATHER_DATA_PATH 환경변수에서 경로를 읽어옵니다.
     * 환경변수가 없으면 ./weather_data를 사용합니다.
     * 
     * @return 날씨 데이터 맵 (파일명 -> WeatherDataInput)
     *         빈 맵 = zero weather 조건 사용
     */
    std::map<std::string, WeatherDataInput> LoadWeatherData();
    
    /**
     * @brief 날씨 데이터 로드 (경로 직접 지정)
     * 
     * @param directory 날씨 데이터 디렉토리 경로
     *                  빈 문자열이면 환경변수 사용
     * @return 날씨 데이터 맵 (파일명 -> WeatherDataInput)
     *         빈 맵 = zero weather 조건 사용
     */
    std::map<std::string, WeatherDataInput> LoadWeatherData(const std::string& directory);
    
private:
    /**
     * @brief 단일 바이너리 파일 로드
     * 
     * DSME 포맷 바이너리 파일을 읽어서 WeatherDataInput으로 변환합니다.
     * 
     * @param filepath 바이너리 파일 경로
     * @return WeatherDataInput 구조체 (실패 시 빈 data)
     */
    WeatherDataInput LoadBinaryFile(const std::string& filepath);
};