# ISP Simulation Pipeline (C++)

This project is a lightweight ISP pipeline implemented in C++. 

## Completed Modules
### Demosaicing
- Bilinear interpolation for RGGB Bayer pattern
- Using mirror-based fallback
- Includes unit tests and randomized stress tests ```./demosaic_tests```
- Visual output tool
#### Example Input/Output
![real_sample](https://github.com/user-attachments/assets/b65fffe5-5ba1-4258-a540-94f7676ac067)
![real_sample_demosaiced](https://github.com/user-attachments/assets/b9375bf9-842d-47e2-a549-ff22da4fa8d8)


## Planned Modules
Denosing, AWB, Color Correction, Tone Mapping

## Project structure
```bash
ISP_Simulation_Pipeline/
├── include/            # Public headers
├── modules/            # Module implementations
├── tests/              # Unit tests (Catch2)
├── data/               # (Ignored) Test images and samples
├── external/           # Catch2 header
├── CMakeLists.txt      # Build configuration
└── README.md
```

## Prerequisites
- CMake 3.8+
- OpenCV 4.x
- Visual Studio

```bash
mkdir build && cd build
cmake ..
cmake --build .
```
