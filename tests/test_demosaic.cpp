#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "demosaic.hpp"
#include "imageio.hpp"
#include <filesystem>

using ImageGray = std::vector<std::vector<uint8_t>>;
using ImageRGB = std::vector<std::vector<std::array<uint8_t, 3>>>;

TEST_CASE("Preserves red pixel at (0,0)", "[core][rggb]") {
    ImageGray bayer = {
        {255, 0},
        {0, 0}
    };

    auto rgb = demosaic_bilinear(bayer);
    REQUIRE(rgb[0][0][0] == 255);  // R
}

TEST_CASE("Preserves green pixel at (0,1)", "[core][rggb]") {
    ImageGray bayer = {
        {0, 128},
        {0, 0}
    };

    auto rgb = demosaic_bilinear(bayer);
    REQUIRE(rgb[0][1][1] == 128);  // G
}

TEST_CASE("Preserves green pixel at (1,0)", "[core][rggb]") {
    ImageGray bayer = {
        {0, 0},
        {88, 0}
    };

    auto rgb = demosaic_bilinear(bayer);
    REQUIRE(rgb[1][0][1] == 88);  // G
}

TEST_CASE("Preserves blue pixel at (1,1)", "[core][rggb]") {
    ImageGray bayer = {
        {0, 0},
        {0, 32}
    };

    auto rgb = demosaic_bilinear(bayer);
    REQUIRE(rgb[1][1][2] == 32);  // B
}

TEST_CASE("Interpolates green at red pixel", "[core][interpolation]") {
    ImageGray bayer = {
        {200, 100},
        {50, 0}
    };

    auto rgb = demosaic_bilinear(bayer);
    // R = 200 at (0,0)
    // G interpolated from 100 (right) and 50 (down), so G = (100 + 50) / 2 = 75
    REQUIRE(rgb[0][0][0] == 200); // R
    REQUIRE(rgb[0][0][1] == 75);  // G
}

TEST_CASE("Interpolates red at green pixel", "[core][interpolation]") {
    ImageGray bayer = {
        {200, 128},
        {0, 0}
    };

    auto rgb = demosaic_bilinear(bayer);
    // Green pixel at (0,1), red on the left (200)
    REQUIRE(rgb[0][1][1] == 128);  // G
    REQUIRE(rgb[0][1][0] == 200);  // R
    REQUIRE(rgb[0][1][0] == rgb[0][0][0]); // Mirror red on the left
}

TEST_CASE("Interpolates blue at green pixel (vertical)", "[core][interpolation]") {
    ImageGray bayer = {
        {0, 128},
        {0, 20}
    };

    auto rgb = demosaic_bilinear(bayer);
    // Green pixel at (0,1), blue expected from below at (1,1)
    REQUIRE(rgb[0][1][1] == 128);      // G
    REQUIRE(rgb[0][1][2] == 20);       // B
    REQUIRE(rgb[0][1][2] == rgb[1][1][2]);  // Mirror blue below
}

TEST_CASE("Interpolates both G and B at red pixel", "[core][interpolation]") {
    ImageGray bayer = {
        {200, 128, 100, 30},
        {0,   1,   12,   20},
        {0,   64,  0,   50}
    };

    auto rgb = demosaic_bilinear(bayer);
    // Red pixel at (0,2): interpolate G from (0,1) & (0,3) & twice (1,2), B from (1,1) & (1,3)
    REQUIRE(rgb[0][2][0] == 100);  // R
    REQUIRE(rgb[0][2][1] == 45);  // G = (128 + 30 + 12*2) / 4
    REQUIRE(rgb[0][2][2] == 10);   // B = (1 + 20)/2
}

TEST_CASE("Mirror fallback at top-left corner (0,0)", "[fallback][corner]") {
    ImageGray bayer = {
        {100, 50},
        {25, 10}
    };

    auto rgb = demosaic_bilinear(bayer);

    // R = 100 at (0,0)
    // G interpolated from 50 (right) and 25 (down), so G = (50 + 25) / 2 = 37
    // B mirrors (1,1)
    REQUIRE(rgb[0][0][0] == 100);         // R pixel at (0,0)
    REQUIRE(rgb[0][0][1] == 37);          // G interpolated
    REQUIRE(rgb[0][0][2] == 10);          // B mirror
}

TEST_CASE("Mirror fallback at bottom-right corner", "[fallback][corner]") {
    ImageGray bayer = {
        {39,   42,   33},
        {97,   157,  0},
        {99,   83,  255}
    };

    auto rgb = demosaic_bilinear(bayer);

    REQUIRE(rgb[2][2][0] == 255);     // R pixel
    REQUIRE(rgb[2][2][1] == 41);      // G = (83 + 0) / 2 = 41
    REQUIRE(rgb[2][2][2] == 157);     // B fallback
}

TEST_CASE("Mirror fallback on top row center", "[fallback][edge]") {
    ImageGray bayer = {
        {0, 100, 20},
        {0,  0,  30}
    };

    auto rgb = demosaic_bilinear(bayer);

    REQUIRE(rgb[0][1][1] == 100);         // Green pixel at (0,1)
    REQUIRE(rgb[0][1][0] == 10);          // R = (20 + 0) / 2 = 10
    REQUIRE(rgb[0][1][2] == 0);           // B below
}

TEST_CASE("Handles 1x1 Bayer image", "[edge][small]") {
    ImageGray bayer = {
        {128}
    };

    auto rgb = demosaic_bilinear(bayer);

    REQUIRE(rgb.size() == 1);
    REQUIRE(rgb[0].size() == 1);
    REQUIRE(rgb[0][0][0] == 128);   // Assume R pixel (0,0)
    REQUIRE(rgb[0][0][1] == 0); // G
    REQUIRE(rgb[0][0][2] == 0); // B
}

TEST_CASE("Handles 2x2 Bayer image (RGGB)", "[edge][small]") {
    ImageGray bayer = {
        {255, 128},
        {128, 64}
    };

    auto rgb = demosaic_bilinear(bayer);

    REQUIRE(rgb[0][0][0] == 255);   // R
    REQUIRE(rgb[0][1][1] == 128);   // G
    REQUIRE(rgb[1][0][1] == 128);   // G
    REQUIRE(rgb[1][1][2] == 64);    // B
}

TEST_CASE("Interpolates smoothly across horizontal R-G-B gradient", "[gradient]") {
    ImageGray bayer = {
        {255, 128, 0},   // R G R
        {128, 64, 128},  // G B G
        {0, 128, 255}    // R G R
    };

    auto rgb = demosaic_bilinear(bayer);

    // Top-left (0,0) should be R
    REQUIRE(rgb[0][0][0] == 255);
    // Center (1,1) should be B
    REQUIRE(rgb[1][1][2] == 64);
    // Bottom-right (2,2) should be R again
    REQUIRE(rgb[2][2][0] == 255);
}

TEST_CASE("Symmetric R-G-R pattern yields symmetric RGB", "[consistency]") {
    ImageGray bayer = {
        {255, 128, 255},
        {128, 64, 128},
        {255, 128, 255}
    };

    auto rgb = demosaic_bilinear(bayer);

    REQUIRE(rgb[0][0] == rgb[0][2]); // top-left == top-right
    REQUIRE(rgb[2][0] == rgb[2][2]); // bottom-left == bottom-right
}

TEST_CASE("Constant R-only input yields uniform R channel", "[consistency]") {
    ImageGray bayer(4, std::vector<uint8_t>(4, 255));  // All pixels 255

    auto rgb = demosaic_bilinear(bayer);

    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            REQUIRE(rgb[y][x][0] == 255);  // All R values should be high
}

TEST_CASE("Demosaicing is deterministic on same input", "[consistency]") {
    ImageGray bayer = {
        {255, 0, 255},
        {0, 128, 0},
        {255, 0, 255}
    };

    auto rgb1 = demosaic_bilinear(bayer);
    auto rgb2 = demosaic_bilinear(bayer);

    REQUIRE(rgb1 == rgb2);  // Test equality of entire output
}

TEST_CASE("Randomized stress testing", "[stress]") {
    for (int i = 0; i < 1000; ++i) {
        int w = rand() % 100 + 2; // Random width
        int h = rand() % 100 + 2; // Random height
        ImageGray bayer(h, std::vector<uint8_t>(w));

        for (auto& row : bayer)
            for (auto& px : row)
                px = rand() % 256;

        auto rgb = demosaic_bilinear(bayer);
        REQUIRE(rgb.size() == h);
        REQUIRE(rgb[0].size() == w);
    }
}

// Visualized test with a real-world image
TEST_CASE("Visualize real-world image", "[visualize]") {
    std::string data_path = "C:/Users/ethan/OneDrive/Documents/GitHub/ISP_Simulation_Pipeline/data/";
    std::string filename = "real_sample.png";
    load_png_save_pgm(data_path + filename);

    filename = "real_sample.pgm";
    auto bayer = load_grayscale_image(data_path + filename);
    auto rgb = demosaic_bilinear(bayer);
    save_rgb_image(rgb, data_path + "real_sample_demosaiced.png");
}