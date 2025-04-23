#include <iostream>
#include "demosaic.hpp"

using ImageGray = std::vector<std::vector<uint8_t>>;
using ImageRGB = std::vector<std::vector<std::array<uint8_t, 3>>>;

ImageRGB demosaic_bilinear(const ImageGray& bayer) {
	if (bayer.empty() || bayer[0].empty()) {
		throw std::runtime_error("Input image is empty. Demosaicing aborted.");
	}

	int height = bayer.size();
	int width = bayer[0].size();

	ImageRGB rgb(height, std::vector<std::array<uint8_t, 3>>(width, { 0, 0, 0 }));

	if (height == 1 && width == 1) {
		uint8_t R = 0, G = 0, B = 0;

		// RGGB: (0,0) = Red pixel
		R = bayer[0][0];

		rgb[0][0] = { R, G, B };
		return rgb;
	}

	// Interior Pixels
	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {
			uint8_t R = 0, G = 0, B = 0;

			// Determine Bayer pattern position (RGGB)
			bool evenRow = (y % 2 == 0);
			bool evenCol = (x % 2 == 0);

			if (evenRow && evenCol) {
				// Red pixel
				R = bayer[y][x];
				G = (bayer[y - 1][x] + bayer[y][x - 1] + bayer[y + 1][x] + bayer[y][x + 1]) / 4;
				B = (bayer[y - 1][x - 1] + bayer[y - 1][x + 1] + bayer[y + 1][x - 1] + bayer[y + 1][x + 1]) / 4;
			}
			else if (evenRow && !evenCol) {
				// Green pixel (on red row)
				G = bayer[y][x];
				R = (bayer[y][x - 1] + bayer[y][x + 1]) / 2;
				B = (bayer[y - 1][x] + bayer[y + 1][x]) / 2;
			}
			else if (!evenRow && evenCol) {
				// Green pixel (on blue row)
				G = bayer[y][x];
				R = (bayer[y - 1][x] + bayer[y + 1][x]) / 2;
				B = (bayer[y][x - 1] + bayer[y][x + 1]) / 2;
			}
			else {
				// Blue pixel
				B = bayer[y][x];
				G = (bayer[y - 1][x] + bayer[y][x - 1] + bayer[y + 1][x] + bayer[y][x + 1]) / 4;
				R = (bayer[y - 1][x - 1] + bayer[y - 1][x + 1] + bayer[y + 1][x - 1] + bayer[y + 1][x + 1]) / 4;

			}

			rgb[y][x] = { R, G, B };
		}
	}

	// Boundary (Mirror fallbacks): Top and bottom rows
	for (int x = 0; x < width; ++x) {
		// top row: y = 0
		// bottom row: y = height - 1
		int y_top = 0, y_bot = height - 1;
		for (int y : { y_top, y_bot }) {
			bool evenRow = (y % 2 == 0);
			bool evenCol = (x % 2 == 0);
			uint8_t R = 0, G = 0, B = 0;

			if (evenRow && evenCol) {
				// Red pixel
				R = bayer[y][x];
				G = (
					(x < width - 1 ? bayer[y][x + 1] : bayer[y][x - 1]) +
					(x > 0 ? bayer[y][x - 1] : bayer[y][x + 1]) +
					2 * (y == 0 ? bayer[y + 1][x] : bayer[y - 1][x])
					) / 4;
				B = (
					(x < width - 1 ? bayer[y + (y == 0 ? 1 : -1)][x + 1] : bayer[y + (y == 0 ? 1 : -1)][x - 1]) +
					(x > 0 ? bayer[y + (y == 0 ? 1 : -1)][x - 1] : bayer[y + (y == 0 ? 1 : -1)][x + 1])
					) / 2;
			}
			else if (evenRow && !evenCol) {
				// Green on red row
				G = bayer[y][x];
				R = (x > 0 && x < width - 1) ? (bayer[y][x - 1] + bayer[y][x + 1]) / 2 :
					(x > 0 ? bayer[y][x - 1] : bayer[y][x + 1]);
				B = (y == 0 ? bayer[y + 1][x] : bayer[y - 1][x]);
			}
			else if (!evenRow && evenCol) {
				// Green on blue row
				G = bayer[y][x];
				R = (y == 0 ? bayer[y + 1][x] : bayer[y - 1][x]);
				B = (x > 0 && x < width - 1) ? (bayer[y][x - 1] + bayer[y][x + 1]) / 2 :
					(x > 0 ? bayer[y][x - 1] : bayer[y][x + 1]);
			}
			else {
				// Blue pixel
				B = bayer[y][x];
				G = (
					(x < width - 1 ? bayer[y][x + 1] : bayer[y][x - 1]) +
					(x > 0 ? bayer[y][x - 1] : bayer[y][x + 1]) +
					2 * (y == 0 ? bayer[y + 1][x] : bayer[y - 1][x])
					) / 4;
				R = (
					(x < width - 1 ? bayer[y + (y == 0 ? 1 : -1)][x + 1] : bayer[y + (y == 0 ? 1 : -1)][x - 1]) +
					(x > 0 ? bayer[y + (y == 0 ? 1 : -1)][x - 1] : bayer[y + (y == 0 ? 1 : -1)][x + 1])
					) / 2;
			}
			
			rgb[y][x] = { R, G, B };
		}
	}

	// Boundary (Mirror fallbacks): Left and right columns
	for (int y = 1; y < height - 1; ++y) {
		for (int x : { 0, width - 1 }) {
			bool evenRow = (y % 2 == 0);
			bool evenCol = (x % 2 == 0);
			uint8_t R = 0, G = 0, B = 0;

			if (evenRow && evenCol) {
				// Red pixel
				R = bayer[y][x];
				G = (
					(y < height - 1 ? bayer[y + 1][x] : bayer[y - 1][x]) +
					(y > 0 ? bayer[y - 1][x] : bayer[y + 1][x]) +
					2 * (x == 0 ? bayer[y][x + 1] : bayer[y][x - 1])
					) / 4;
				B = (
					(y < height - 1 ? bayer[y + 1][x + (x == 0 ? 1 : -1)] : bayer[y - 1][x + (x == 0 ? 1 : -1)]) +
					(y > 0 ? bayer[y - 1][x + (x == 0 ? 1 : -1)] : bayer[y + 1][x + (x == 0 ? 1 : -1)])
					) / 2;
			}
			else if (evenRow && !evenCol) {
				// Green on red row
				G = bayer[y][x];
				R = (x == 0 ? bayer[y][x + 1] : bayer[y][x - 1]);
				B = (y > 0 && y < height - 1) ? (bayer[y - 1][x] + bayer[y + 1][x]) / 2 :
					(y > 0 ? bayer[y - 1][x] : bayer[y + 1][x]);
			}
			else if (!evenRow && evenCol) {
				// Green on blue row
				G = bayer[y][x];
				B = (x == 0 ? bayer[y][x + 1] : bayer[y][x - 1]);
				R = (y > 0 && y < height - 1) ? (bayer[y - 1][x] + bayer[y + 1][x]) / 2 :
					(y > 0 ? bayer[y - 1][x] : bayer[y + 1][x]);
			}
			else {
				// Blue pixel
				B = bayer[y][x];
				G = (
					(y < height - 1 ? bayer[y + 1][x] : bayer[y - 1][x]) +
					(y > 0 ? bayer[y - 1][x] : bayer[y + 1][x]) +
					2 * (x == 0 ? bayer[y][x + 1] : bayer[y][x - 1])
					) / 4;
				R = (
					(y < height - 1 ? bayer[y + 1][x + (x == 0 ? 1 : -1)] : bayer[y - 1][x + (x == 0 ? 1 : -1)]) +
					(y > 0 ? bayer[y - 1][x + (x == 0 ? 1 : -1)] : bayer[y + 1][x + (x == 0 ? 1 : -1)])
					) / 2;
			}

			rgb[y][x] = { R, G, B };
		}
	}

	return rgb;
}

void run_demosaic() {
	std::cout << "Demosaicing module ready (not yet integrated with image input)." << std::endl;
}