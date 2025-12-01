//
// Created by Jack Collins on 11/25/25.
//
#include "ComplexPlane.h"
#include <array>
#include <cmath>
#include <complex>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
using namespace std;
constexpr std::array<sf::Color, 16> rColors2 = {
    sf::Color(66, 30, 15), // Brown
    sf::Color(25, 7, 26), // Dark Violet
    sf::Color(9, 1, 47), // Dark Blue
    sf::Color(4, 4, 73), // Navy
    sf::Color(0, 7, 100), // Deep Blue
    sf::Color(12, 44, 138), // Blue
    sf::Color(24, 82, 177), // Lighter Blue
    sf::Color(57, 125, 209), // Sky Blue
    sf::Color(134, 181, 229), // Pale Blue
    sf::Color(211, 236, 248), // White-ish
    sf::Color(241, 233, 191), // Pale Yellow
    sf::Color(248, 201, 95), // Gold
    sf::Color(255, 170, 0), // Orange
    sf::Color(204, 128, 0), // Dark Orange
    sf::Color(153, 87, 0), // Rust
    sf::Color(106, 52, 3) // Dark Brown
};


ComplexPlane::ComplexPlane(int pixelWidth, int pixelHeight) : m_pixel_size(pixelWidth, pixelHeight), offset(0.5, 0.5, 0.5), amp(0.5, 0.5, 0.5),freq(1.0 , 1.0, 0.5), phase(0.80, 0.9, 0.30) {
    loadPalette(rColors2);


    m_aspectRatio = static_cast<float>(pixelHeight) / static_cast<float>(pixelWidth);
    m_plane_center = {0, 0};
    m_plane_size = {
        static_cast<float>(BASE_WIDTH), static_cast<float>(static_cast<float>(BASE_HEIGHT) * m_aspectRatio)
    };
    m_zoomCount = 1.0;
    m_State = State::CALCULATING;
    if (!sf::Shader::isAvailable()) {
        cout << "Error gpu drivers too old\n";
    }
    if (!mandelbrotShader.loadFromFile("mandelbrot.frag", sf::Shader::Type::Fragment)) {
        std::cerr << "Failed to load mandelbrot.frag" << std::endl;
    }    workToDo = true;
    cv.notify_one();
    //m_vArray.setPrimitiveType(sf::PrimitiveType::Points);
    //m_vArray.resize(static_cast<std::size_t>(pixelHeight) * static_cast<std::size_t>(pixelWidth));
};


void ComplexPlane::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    //target.draw(m_vArray);
}
void ComplexPlane::loadPalette(const std::array<sf::Color, 16>& colors) {
    // Create 16x1 texture
    m_paletteTexture.resize({16,1});
    //upload color data
    // cast data to sfUint8* to treat as raw bytes
    m_paletteTexture.update(reinterpret_cast<const uint8_t*>(colors.data()));
    // enable smoothing
    m_paletteTexture.setSmooth(true);
    // prevent color wrap around
    m_paletteTexture.setRepeated(false);
}



void ComplexPlane::updateRender() {
    if (m_State == CALCULATING) {
        // for (int i = 0; i < m_pixel_size.y; ++i) {
        //     for (int j = 0; j < m_pixel_size.x; ++j) {
        //         const std::size_t idx = j + i * m_pixel_size.x; // width stride
        //
        //         m_vArray[idx].position = {static_cast<float>(j), static_cast<float>(i)};
        //         sf::Vector2f ComCoord = mapPixelToCoords({j, i});
        //         int iterationCount = countIterations(ComCoord);
        //
        //         std::uint8_t r = 0, g = 0, b = 0;
        //         iterationsToRGB(iterationCount, r, g, b);
        //
        //         m_vArray[idx].color = {r, g, b};
        //     }
        // }
        mandelbrotShader.setUniform("m_plane_center", sf::Glsl::Vec2(m_plane_center));
        mandelbrotShader.setUniform("m_zoomCount", static_cast<float>(m_zoomCount));
        mandelbrotShader.setUniform(
            "m_pixel_size",
            sf::Glsl::Vec2(static_cast<float>(m_pixel_size.x), static_cast<float>(m_pixel_size.y))
        );        mandelbrotShader.setUniform("MAX_ITER", static_cast<int>(MAX_ITER));
        mandelbrotShader.setUniform("m_aspectRatio", static_cast<float>(m_aspectRatio));
        //mandelbrotShader.setUniform("paletteTexture", m_paletteTexture);
        mandelbrotShader.setUniform("m_plane_size",m_plane_size);
        mandelbrotShader.setUniform("offset", offset);
        mandelbrotShader.setUniform("amp", amp);
        mandelbrotShader.setUniform("freq", freq);
        mandelbrotShader.setUniform("phase", phase);

        m_State = DISPLAYING;
    }
}

void ComplexPlane::zoomIn() {

    m_zoomCount += 1.0;
    cout << "Zoom count Increased: " << m_zoomCount << "\n";
    float x_size = BASE_WIDTH * std::pow(BASE_ZOOM, m_zoomCount);
    float y_size = BASE_HEIGHT * m_aspectRatio * (std::pow(BASE_ZOOM, m_zoomCount));
    m_plane_size = {x_size, y_size};
    m_State = CALCULATING;
    cv.notify_one();
}

void ComplexPlane::zoomOut() {
    m_zoomCount -= 1.0;
    cout << "Zoom count Decreased: " << m_zoomCount << "\n";
    float x_size = BASE_WIDTH * (std::pow(BASE_ZOOM, m_zoomCount));
    float y_size = BASE_HEIGHT * m_aspectRatio * (std::pow(BASE_ZOOM, m_zoomCount));
    m_plane_size = {x_size, y_size};
    m_State = CALCULATING;
    cv.notify_one();
}

void ComplexPlane::setCenter(sf::Vector2i mousePixel) {
    m_plane_center = mapPixelToCoords(mousePixel);
    m_State = CALCULATING;
}

void ComplexPlane::setMouseLocation(sf::Vector2i mousePixel) {
    m_mouseLocation = mapPixelToCoords(mousePixel);
}

void ComplexPlane::loadText(sf::Text &text) {
    std::stringstream s;
    s << "Mandelbrot Set\n";
    s << "Center:(" << m_plane_center.x << "," << m_plane_center.y << ")\n";
    s << "Cursor:(" << m_mouseLocation.x << "," << m_mouseLocation.y << ")\n";
    s << "Left-click to Zoom in\n";
    s << "Right-click to Zoom out\n";
    text.setString(s.str());
}


int ComplexPlane::countIterations(sf::Vector2f coord) {
    const std::complex<double> c = {coord.x, coord.y};
    std::complex<double> z = {0.0, 0.0};
    int i = 0;
    while ((z.real() * z.real() + z.imag() * z.imag()) < 4.0 && i < static_cast<int>(MAX_ITER))
    // z*z executes faster because abs(z) uses sqrt
    {
        z = z * z + c;
        i++;
    }
    return i;
}

void ComplexPlane::iterationsToRGB(unsigned int count, std::uint8_t &r, std::uint8_t &g, std::uint8_t &b) {
    constexpr unsigned int MAX_CUTOFF = MAX_ITER;
    if (count >= MAX_CUTOFF) {
        r = g = b = 0;
    } else {
        std::size_t colorIndex = count % rColors2.size();
        sf::Color C = rColors2[colorIndex];
        r = C.r;
        g = C.g;
        b = C.b;
    }
}


sf::Vector2f ComplexPlane::mapPixelToCoords(sf::Vector2i mousePixel) {
    auto x = static_cast<float>(mousePixel.x);
    auto y = static_cast<float>(mousePixel.y);

    auto sx = static_cast<float>(m_pixel_size.x);
    auto sy = static_cast<float>(m_pixel_size.y);
    float minX = m_plane_center.x - m_plane_size.x / 2.0f;
    float maxX = m_plane_center.x + m_plane_size.x / 2.0f;
    float minY = m_plane_center.y - m_plane_size.y / 2.0f;
    float maxY = m_plane_center.y + m_plane_size.y / 2.0f;

    sf::Vector2f v;
    v.x = minX + (x / (sx - 1.0f)) * (maxX - minX);
    v.y = minY + (y / (sy - 1.0f)) * (maxY - minY);

    return v;
}
