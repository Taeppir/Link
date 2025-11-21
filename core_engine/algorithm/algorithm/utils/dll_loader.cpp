#include "dll_loader.h"
#include <windows.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

// ===== 기존 fetch_dll.cpp 코드 100% 보존 =====

class ShipDynamicsDLL {
private:
    HINSTANCE hDll;
    typedef void (*CalculateShipDynamicsFunc)(const ShipInput*, ShipOutput*);
    CalculateShipDynamicsFunc calculateFunc;

    void LoadDLL() {
        hDll = LoadLibraryW(L"ShipDynamics.dll");
        if (hDll == NULL) {
            throw std::runtime_error("ERROR: Cannot load ShipDynamics.dll");
        }

        calculateFunc = (CalculateShipDynamicsFunc)GetProcAddress(hDll, "CalculateShipDynamics");
        if (calculateFunc == NULL) {
            FreeLibrary(hDll);
            hDll = nullptr;
            throw std::runtime_error("ERROR: Cannot find CalculateShipDynamics function in DLL");
        }
    }

    ShipDynamicsDLL() : hDll(nullptr), calculateFunc(nullptr) {
        LoadDLL();
    }

    ~ShipDynamicsDLL() {
        if (hDll != nullptr) {
            FreeLibrary(hDll);
        }
    }

    ShipDynamicsDLL(const ShipDynamicsDLL&) = delete;
    ShipDynamicsDLL& operator=(const ShipDynamicsDLL&) = delete;

public:
    static ShipDynamicsDLL& GetInstance() {
        static ShipDynamicsDLL instance;
        return instance;
    }

    void Calculate(const ShipInput* input, ShipOutput* output) {
        if (calculateFunc == nullptr) {
            throw std::runtime_error("DLL function not loaded");
        }

        std::memset(output, 0, sizeof(ShipOutput));
        calculateFunc(input, output);
    }
};

ShipOutput calculateFuelConsumption(const ShipInput& inputData) {
    ShipOutput outputData;
    std::memset(&outputData, 0, sizeof(ShipOutput));

    ShipDynamicsDLL::GetInstance().Calculate(&inputData, &outputData);

    return outputData;
}