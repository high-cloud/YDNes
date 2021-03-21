#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <SFML/Window.hpp>
#include <vector>
#include "constant.h"

namespace yn
{
    class Controller
    {
    public:
        enum class Button
        {
            A,
            B,
            Select,
            Start,
            Up,
            Down,
            Left,
            Right,
            TotalButtons
        };

        Byte read();
        void write(Byte value);
        void setKeyBindings(const std::vector<sf::Keyboard::Key> & keys);

    private:
        bool m_isStrobe;
        Byte m_data;
        std::vector<sf::Keyboard::Key> m_keyBindings;
    };
}
#endif // __CONTROLLER_H__