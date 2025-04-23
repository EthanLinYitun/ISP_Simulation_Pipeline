#pragma once

#include <vector>
#include <array>
#include <cstdint>

std::vector<std::vector<std::array<uint8_t, 3>>> demosaic_bilinear(const std::vector<std::vector<uint8_t>>& bayer);
void run_demosaic();