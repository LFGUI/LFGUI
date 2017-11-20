#ifndef LFGUI_GENERAL_H
#define LFGUI_GENERAL_H

#include <algorithm>
#include <thread>
#include <vector>
#include <functional>
#include <atomic>
#include <immintrin.h>

#include "geometry.h"

namespace lfgui
{

/// \brief Currently only used by the draw_text function. Specifies how the text is aligned.
enum class alignment
{
    left,
    center,
    right
};

struct color
{
    union
    {
        uint8_t array[4];
        uint32_t value;
        struct
        {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t a;
        };
    };

    color(uint8_t r=0,uint8_t g=0,uint8_t b=0,uint8_t a=255) : b(b),g(g),r(r),a(a){}
    /// \brief Constructs a color from an array. Order is RGBA.
    //color(uint8_t array[4]) {*((uint32_t*)this->array)=*((uint32_t*)array);}
    /// \brief Constructs a color from one uint32_t. On little-endian systems (x86) the colors are reversed as ABGR.
    //color(uint32_t v) : value(v){}

    /// \brief Returns a copy of this color with its alpha multiplied by f.
    color alpha_multiplied(float f) const
    {
        return color(r,g,b,a*f);
    }

    /// \brief Returns a copy of this color with its alpha multiplied by f.
    color alpha_multiplied(int f) const
    {
        return color(r,g,b,a*f/255);
    }

    color operator*(float f)
    {
        return color(r*f,g*f,b*f,a*f);
    }

    color operator+(const color& o)
    {
        return color(r+o.r,g+o.g,b+o.b,a+o.a);
    }
};

}   // namespace lfgui

inline std::ostream& operator<<(std::ostream& os,const lfgui::color& c)
{
    os<<(int)c.r<<","<<(int)c.g<<','<<(int)c.b<<","<<(int)c.a;
    return os;
}

inline void print(__m128i v)
{
    static char hex[]="0123456789ABCDEF";
    uint32_t vec[4];
    _mm_storeu_si128((__m128i*)vec,v);
    unsigned char* c=(unsigned char*)vec;
    std::cout<<"0x";
    std::cout<<hex[(c[0])>>4]<<hex[(c[0])&0x0F];
    std::cout<<hex[(c[1])>>4]<<hex[(c[1])&0x0F];
    std::cout<<hex[(c[2])>>4]<<hex[(c[2])&0x0F];
    std::cout<<hex[(c[3])>>4]<<hex[(c[3])&0x0F];
    c+=4;
    std::cout<<",0x";
    std::cout<<hex[(c[0])>>4]<<hex[(c[0])&0x0F];
    std::cout<<hex[(c[1])>>4]<<hex[(c[1])&0x0F];
    std::cout<<hex[(c[2])>>4]<<hex[(c[2])&0x0F];
    std::cout<<hex[(c[3])>>4]<<hex[(c[3])&0x0F];
    c+=4;
    std::cout<<",0x";
    std::cout<<hex[(c[0])>>4]<<hex[(c[0])&0x0F];
    std::cout<<hex[(c[1])>>4]<<hex[(c[1])&0x0F];
    std::cout<<hex[(c[2])>>4]<<hex[(c[2])&0x0F];
    std::cout<<hex[(c[3])>>4]<<hex[(c[3])&0x0F];
    c+=4;
    std::cout<<",0x";
    std::cout<<hex[(c[0])>>4]<<hex[(c[0])&0x0F];
    std::cout<<hex[(c[1])>>4]<<hex[(c[1])&0x0F];
    std::cout<<hex[(c[2])>>4]<<hex[(c[2])&0x0F];
    std::cout<<hex[(c[3])>>4]<<hex[(c[3])&0x0F];
    std::cout<<" "<<vec[0]<<','<<vec[1]<<','<<vec[2]<<','<<vec[3]<<std::endl;
}

#endif  // LFGUI_GENERAL_H
