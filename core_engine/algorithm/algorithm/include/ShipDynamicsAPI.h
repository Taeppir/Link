#pragma once

#ifdef SHIPDYNAMICS_EXPORTS
#define SHIPDYNAMICS_API __declspec(dllexport)
#else
#define SHIPDYNAMICS_API __declspec(dllimport)
#endif

extern "C" {

    struct SHIPDYNAMICS_API ShipInput {

        // weather info
        double windDirectionDeg;	//(deg)
        double windSpeed;		//(m/s)
        double currentDirectionDeg;	//(deg)
        double currentSpeed;		//(m/s)
        double waveHeight;		//(m)
        double waveDirectionDeg;	//(deg)
        double wavePeriod;		//(sec)

        // ship info
        double heading;     		//(deg)
        double shipSpeed;   		//(m/s)
        double draft;       		//(m)
        double trim;        		//(m)

        // waps info
        int waps_type;      		//0,1,2 - default:1
    };


    struct SHIPDYNAMICS_API ShipOutput {
        double resistance;
        double fuelConsumption;		//(kg/h)
    };

    SHIPDYNAMICS_API void CalculateShipDynamics(const ShipInput* input, ShipOutput* output);

}