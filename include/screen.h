#ifndef __SCREEN_H__
#define __SCREEN_H__

#include<cstddef>

namespace yn{
    class Screen{
        public:
            virtual void create(unsigned int width,unsigned int height, float pixel_size,int color)=0;
            virtual void setPixel(std::size_t x,std::size_t y, uint32_t color)=0;
    };
}
#endif // __SCREEN_H__