#pragma once
#include "ShipDynamicsAPI.h"

// calculateFuelConsumption 함수 선언
// (구현은 dll_loader.cpp에 있음)
ShipOutput calculateFuelConsumption(const ShipInput& inputData);