#pragma once

// ===== 기존 ShipDynamicsAPI.h 내용 100% 그대로 =====

#ifdef SHIPDYNAMICS_EXPORTS
#define SHIPDYNAMICS_API __declspec(dllexport)
#else
#define SHIPDYNAMICS_API __declspec(dllimport)
#endif

extern "C" {

    struct SHIPDYNAMICS_API ShipInput {
        // weather info
        double windDirectionDeg;
        double windSpeed;
        double currentDirectionDeg;
        double currentSpeed;
        double waveHeight;
        double waveDirectionDeg;
        double wavePeriod;

        // ship info
        double heading;
        double shipSpeed;
        double draft;
        double trim;

        // waps info
        int waps_type;
    };

    struct SHIPDYNAMICS_API ShipOutput {
        double resistance;
        double fuelConsumption;
    };

    SHIPDYNAMICS_API void CalculateShipDynamics(const ShipInput* input, ShipOutput* output);
}