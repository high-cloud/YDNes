#include "sfmlScreen.h"

namespace yn
{
    void SFMLScreen::create(unsigned int width, unsigned int height, float pixel_size, uint32_t color)
    {
        m_vertices.resize(width * height * 4); //each quad have 4 vertices
        m_width = width;
        m_height = height;
        m_vertices.setPrimitiveType(sf::Triangles);
        m_pixelSize = pixel_size;
        sf::Color sfcolor(color);

        for (std::size_t x = 0; x < width; ++x)
            for (auto y = 0; y < height; ++y)
            {
                auto index = (x * height + y) * 4;
                sf::Vector2f coord2d(x * m_pixelSize, y * m_pixelSize);

                // top-left
                m_vertices[index].position = coord2d;
                m_vertices[index].color = sfcolor;

                //top-right
                m_vertices[index + 1].position = coord2d + sf::Vector2f{m_pixelSize, 0};
                m_vertices[index + 1].color = sfcolor;

                //bottom-right
                m_vertices[index + 2].position = coord2d + sf::Vector2f{m_pixelSize, m_pixelSize};
                m_vertices[index + 2].color = sfcolor;

                //bottom-left
                m_vertices[index + 3].position = coord2d + sf::Vector2f{0, m_pixelSize};
                m_vertices[index + 3].color = sfcolor;
            }
    }

    void SFMLScreen::setPixel(std::size_t x, std::size_t y, uint32_t color)
    {
        auto index = (x * m_height + y) * 4;
        if (index >= m_vertices.getVertexCount())
            return;

        sf::Vector2f coord2d(x * m_pixelSize, y * m_pixelSize);

        sf::Color sfcolor(color);
        m_vertices[index].color = sfcolor;
        m_vertices[index + 1].color = sfcolor;
        m_vertices[index + 2].color = sfcolor;
        m_vertices[index + 3].color = sfcolor;
    }

    sf::VertexArray &SFMLScreen::vertices()
    {
        return m_vertices;
    }
}