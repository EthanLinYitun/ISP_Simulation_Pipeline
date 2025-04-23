#include <iostream>
#include <filesystem>
#include "demosaic.hpp"
#include "imageio.hpp"

namespace fs = std::filesystem;

int main() {
    // Attempt to auto-detect and set project root as working directory
    fs::path exec_path = fs::current_path();
    fs::path project_root = exec_path;

    // Go up until we find the data/ folder
    while (!fs::exists(project_root / "data") && project_root.has_parent_path()) {
        project_root = project_root.parent_path();
    }

    if (fs::exists(project_root / "data")) {
        std::cout << "Setting working directory to: " << project_root << "\n";
        fs::current_path(project_root);
    }
    else {
        std::cerr << "Could not locate 'data/' folder.\n";
    }
	std::cout << "ISP Simulation Pipeline starting..." << std::endl;
	
	auto bayer = load_grayscale_image("data/sample.pgm");
	auto rgb = demosaic_bilinear(bayer);
	save_rgb_image(rgb, "out/demosaiced_img/output.png");
	std::cout << "Demosaicing complete!" << std::endl;
	std::cout << "Image saved as output.png!" << std::endl;
	
	return 0;
}