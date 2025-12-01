//
// Created by Jack Collins on 11/25/25.
//
#pragma once

#include <cstdint>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

namespace sf {
    class Text;
}

constexpr unsigned int MAX_ITER = 512;
const float BASE_WIDTH = 4.0;
const float BASE_HEIGHT = 4.0;
const float BASE_ZOOM = 1.1;

enum State {
    CALCULATING,
    DISPLAYING
};

class ComplexPlane : public sf::Drawable {
public:
    std::mutex mutex;

    ComplexPlane(int pixelWidth, int pixelHeight);



    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    void loadPalette(const std::array<sf::Color, 16>& colors);
    void zoomIn();

    void zoomOut();

    void setCenter(sf::Vector2i mousePixel);

    void setMouseLocation(sf::Vector2i mousePixel);

    void loadText(sf::Text &text);

    void updateRender();

    int countIterations(sf::Vector2f coord);

    static void iterationsToRGB(unsigned int count, std::uint8_t &r, std::uint8_t &g, std::uint8_t &b);

    sf::Vector2f mapPixelToCoords(sf::Vector2i mousePixel);

    State getState() const { return m_State; };
    double getZoom() const { return m_zoomCount; };
    bool pendingRecalc = false;
    std::condition_variable cv;
    bool workToDo = false;
    sf::Shader mandelbrotShader;
    sf::Vector3f offset;
    sf::Vector3f amp;
    sf::Vector3f freq;
    sf::Vector3f phase;
    State m_State;
private:

    ///sf::VertexArray m_vArray;
    sf::Texture m_paletteTexture;

    sf::Vector2f m_mouseLocation;
    sf::Vector2i m_pixel_size;
    sf::Vector2f m_plane_center;
    sf::Vector2f m_plane_size;
    double m_zoomCount;
    double m_aspectRatio;
    double screenH{};
};
