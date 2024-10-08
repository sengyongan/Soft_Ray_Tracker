#ifndef RTW_STB_IMAGE_H
#define RTW_STB_IMAGE_H

#include <string>
#include <cstdlib>
#include <iostream>

class rtw_image
{
public:
    rtw_image();
    rtw_image(const char *image_filename);
    ~rtw_image();

    bool load(const std::string &filename);
    int width() const { return (fdata == nullptr) ? 0 : image_width; }
    int height() const { return (fdata == nullptr) ? 0 : image_height; }
    const unsigned char *pixel_data(int x, int y) const;

    static int clamp(int x, int low, int high)
    {
        // Return the value clamped to the range [low, high).
        if (x < low)
            return low;
        if (x < high)
            return x;
        return high - 1;
    }

    static unsigned char float_to_byte(float value)
    {
        if (value <= 0.0)
            return 0;
        if (1.0 <= value)
            return 255;
        return static_cast<unsigned char>(256.0 * value);
    }
private:
    static const int bytes_per_pixel = 3;
    float *fdata = nullptr;
    unsigned char *bdata = nullptr;
    int image_width = 0;
    int image_height = 0;
    int bytes_per_scanline = 0;


    void convert_to_bytes();
};
// Restore MSVC compiler warnings
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // RTW_STB_IMAGE_H