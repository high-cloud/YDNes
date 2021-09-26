#include "controller.h"
#include "Log.h"

namespace yn
{
    Byte Controller::read()
    {
        // LOG(Debug) << "controller read" << std::endl;
        Byte result;
        if (m_isStrobe)
        {
            result = sf::Keyboard::isKeyPressed(m_keyBindings[static_cast<int>(Button::A)]);
        }
        else
        {
            result = (m_data & 1);
            m_data >>= 1;
        }

        return result | 0x40;
    }

    void Controller::write(Byte value)
    {
        // LOG(Debug) << "controller writre" << std::endl;
        if (value & 1)
        {
            m_isStrobe = true;
        }
        else
        {
            m_isStrobe = false;

            m_data = 0;
            int shift = 0;
            for (int button = 0; button < 8; ++button)
            {
                m_data |= (sf::Keyboard::isKeyPressed(m_keyBindings[button]) << shift);

                ++shift;
            }
        }
    }

    void Controller::setKeyBindings(const std::vector<sf::Keyboard::Key> &keys)
    {
        m_keyBindings = keys;
    }
}