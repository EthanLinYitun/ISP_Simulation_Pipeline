#include "imageio.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <filesystem>

std::vector<std::vector<uint8_t>> load_grayscale_image(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filename << std::endl;
		throw std::runtime_error("Cannot open file: " + filename);
	}

	std::string line;
	std::getline(file, line);

	if (line != "P2") {
		std::cerr << "Unsupported file format (not P2): " << filename << std::endl;
		throw std::runtime_error("Unsupported file format (not P2)");
	}

	int width = 0, height = 0;
	file >> width >> height;

	std::cout << "Width: " << width << ", Height: " << height << std::endl;
	
	int max_val = 0;
	file >> max_val;

	if (max_val != 255) {
		std::cerr << "Warning: Unexpected max value (" << max_val << "). This loader assumes 8_bit images." << std::endl;
	}

	std::vector<std::vector<uint8_t>> image(height, std::vector<uint8_t>(width));

	for (int row = 0; row < height; ++row) {
		for (int col = 0; col < width; ++col) {
			int pixel = 0;
			file >> pixel;

			if (pixel < 0 || pixel > 255) {
				throw std::runtime_error("Invalid pixel value in file.");
			}

			image[row][col] = static_cast<uint8_t>(pixel);
		}
	}

	// test start
	/*for (const auto& row : image) {
		for (uint8_t px : row) {
			std::cout << static_cast<int>(px) << " ";
		}
		std::cout << "\n";
	}*/
	// test end

	std::cout << "Image loaded successfully!" << std::endl;
	return image;
}

void save_rgb_image(
	const std::vector<std::vector<std::array<uint8_t, 3>>>& rgb_image,
	const std::string& filename)
{
	int height = static_cast<int>(rgb_image.size());
	int width = static_cast<int>(rgb_image[0].size());

	cv::Mat output(height, width, CV_8UC3);

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			const auto& pixel = rgb_image[y][x];
			// OpenCV uses BGR
			output.at<cv::Vec3b>(y, x) = cv::Vec3b(pixel[2], pixel[1], pixel[0]);
		}
	}

	cv::imwrite(filename, output);
}

void load_png_save_pgm(const std::string& filename) {
	// Load the Bayer .png image as grayscale
	cv::Mat bayer = cv::imread(filename, cv::IMREAD_GRAYSCALE);
	if (bayer.empty()) {
		std::cerr << "Failed to load image!" << std::endl;
	}

	if (bayer.channels() != 1 || bayer.depth() != CV_8U) {
		std::cerr << "Expected 8-bit single-channel (grayscale) image." << std::endl;
	}

	// Construct output .pgm path
	std::filesystem::path pgm_path = filename;
	pgm_path.replace_extension(".pgm");

	std::ofstream out(pgm_path);
	if (!out.is_open()) {
		std::cerr << "Failed to open file for writing: " << pgm_path << std::endl;
	}

	// Write P2 header
	out << "P2\n";
	out << bayer.cols << " " << bayer.rows << "\n";
	out << "255\n";  // Max gray level

	// Write pixel data
	for (int y = 0; y < bayer.rows; ++y) {
		for (int x = 0; x < bayer.cols; ++x) {
			int val = static_cast<int>(bayer.at<uchar>(y, x));
			out << val << " ";
		}
		out << "\n";
	}

	out.close();
	std::cout << "Successfully converted to ASCII PGM: " << pgm_path << std::endl;
}