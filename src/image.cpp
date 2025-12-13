#include "image.hpp"
#include <cassert>
#include <vector>
#include <cstdint>
#include "stb_image.h"

Color::Color()
    : r(0), g(0), b(0), a(false) {}

Color::Color(uint8_t r, uint8_t g, uint8_t b, bool a)
    : r(r), g(g), b(b), a(a) {}

Color Color::operator*(float factor) const {
    return Color(uint8_t(r * factor), uint8_t(g * factor), uint8_t(b * factor), a);
}

bool Color::operator==(const Color& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
}

Image::Image(size_t width, size_t height, Color fill)
    : m_width(width), m_height(height) {
    m_pixels.resize(m_width * m_height, fill);
}

Image::Image(uint8_t* stdiBuffer, size_t width, size_t height)
    : m_width(width), m_height(height) {
    assert(stdiBuffer != nullptr && "Buffer must be verified before calling this function!\n");

    m_pixels.resize(m_width * m_height);
    for (size_t i = 0; i < m_width * m_height; i++) {
        size_t byteIdx = i * IMG_BUFFER_CHANNELS;
        Color color(
            stdiBuffer[byteIdx],
            stdiBuffer[byteIdx + 1],
            stdiBuffer[byteIdx + 2],
            stdiBuffer[byteIdx + 3] > UINT8_MAX / 2
        );
        m_pixels[i] = color;
    }
    stbi_image_free(stdiBuffer);
}

void Image::resize(size_t width, size_t height, Color fill) {
    m_pixels.resize(width * height, fill);
    m_width = width;
    m_height = height;
}

void Image::clear(Color fill) {
    std::fill(m_pixels.begin(), m_pixels.end(), fill);
}

void Image::setPixel(size_t x, size_t y, Color value) {
    m_pixels.at(x + y * m_width) = value;
}

Color Image::getPixel(size_t x, size_t y) const {
    return m_pixels.at(x + y * m_width);
}

size_t Image::getWidth() const {
    return m_width;
}

size_t Image::getHeight() const {
    return m_height;
}

std::pair<size_t, size_t> Image::getSize() const {
    return std::pair<size_t, size_t>(m_width, m_height);
}
