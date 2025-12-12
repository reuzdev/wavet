#pragma once
#include <vector>
#include <cstdint>

#define IMG_BUFFER_CHANNELS 4

struct Color {
    uint8_t r, g, b;
    bool a;

    Color();
    Color(uint8_t r, uint8_t g, uint8_t b, bool a = true);
    Color operator*(float factor) const;
    bool operator==(const Color& other) const;
};

class Image {
public:
    Image() = default;
    Image(size_t p_width, size_t p_height, Color fill = Color());
    Image(uint8_t* stdiBuffer, size_t width, size_t height);

    void resize(size_t p_width, size_t p_height, Color fill = Color());
    void clear(Color fill);
    void setPixel(size_t x, size_t y, Color value);
    Color getPixel(size_t x, size_t y) const;
    size_t getWidth() const;
    size_t getHeight() const;
    std::pair<size_t, size_t> getSize() const;

private:
    std::vector<Color> m_pixels;
    size_t m_width, m_height;
};
