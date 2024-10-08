#include "rtw_stb_image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

// 可以在这里包含其他必要的头文件或定义

rtw_image::rtw_image() {}

rtw_image::rtw_image(const char *image_filename)
{
    auto filename = std::string(image_filename);
    auto imagedir = getenv("RTW_IMAGES");

    // Hunt for the image file in some likely locations.
    if (imagedir && load(std::string(imagedir) + "/" + image_filename))
        return;
    if (load(filename))
        return;
    if (load("images/" + filename))
        return;
    if (load("../images/" + filename))
        return;
    if (load("../../images/" + filename))
        return;
    if (load("../../../images/" + filename))
        return;
    if (load("../../../../images/" + filename))
        return;
    if (load("../../../../../images/" + filename))
        return;
    if (load("../../../../../../images/" + filename))
        return;

    std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
}

rtw_image::~rtw_image()
{
    delete[] bdata;
    STBI_FREE(fdata);
}
const unsigned char *rtw_image::pixel_data(int x, int y) const
{
    // Return the address of the three RGB bytes of the pixel at x,y. If there is no image
    // data, returns magenta.
    static unsigned char magenta[] = {255, 0, 255};
    if (bdata == nullptr)
        return magenta;

    x = clamp(x, 0, image_width);
    y = clamp(y, 0, image_height);

    return bdata + y * bytes_per_scanline + x * bytes_per_pixel;
}
bool rtw_image::load(const std::string &filename)
{
    auto n = bytes_per_pixel; // Dummy out parameter: original components per pixel
    fdata = stbi_loadf(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
    if (fdata == nullptr)
        return false;

    bytes_per_scanline = image_width * bytes_per_pixel;
    convert_to_bytes();
    return true;
}
void rtw_image::convert_to_bytes()
{
    // Convert the linear floating point pixel data to bytes, storing the resulting byte
    // data in the `bdata` member.

    int total_bytes = image_width * image_height * bytes_per_pixel;
    bdata = new unsigned char[total_bytes];

    // Iterate through all pixel components, converting from [0.0, 1.0] float values to
    // unsigned [0, 255] byte values.

    auto *bptr = bdata;
    auto *fptr = fdata;
    for (auto i = 0; i < total_bytes; i++, fptr++, bptr++)
        *bptr = float_to_byte(*fptr);
}