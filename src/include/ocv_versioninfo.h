#include <opencv2/core/core.hpp>
#include <iostream>

inline void print_ocv_version()
{
    std::cout << "Template project using OpenCV " << CV_VERSION_MAJOR << "." << CV_VERSION_MINOR << "." << CV_VERSION_REVISION << std::endl;
}

