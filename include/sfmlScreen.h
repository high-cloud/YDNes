#ifndef __SFMLSCREEN_H__
#define __SFMLSCREEN_H__

#include "screen.h"
#include<SFML/Graphics.hpp>
#include<SFML/Window.hpp>

namespace yn
{
    class SFMLScreen : public Screen
    {
    public:
        void create(unsigned int width, unsigned int height, float pixel_size, uint32_t color) override;
        void setPixel(std::size_t x, std::size_t y, uint32_t color) override;

        sf::VertexArray & vertices();
    private:
        sf::VertexArray m_vertices;
        float m_pixelSize;
        unsigned int m_width;
        unsigned int m_height;
    };
}
#endif // __SFMLSCREEN_H__