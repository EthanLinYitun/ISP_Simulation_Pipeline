#pragma once

#include <vector>
#include <string>

std::vector<std::vector<uint8_t>> load_grayscale_image(const std::string& filename);
void save_rgb_image(
    const std::vector<std::vector<std::array<uint8_t, 3>>>& rgb_image,
    const std::string& filename);

void load_png_save_pgm(const std::string& filename);