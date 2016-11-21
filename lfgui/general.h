#ifndef LFGUI_GENERAL_H
#define LFGUI_GENERAL_H

#include <algorithm>

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

#endif  // LFGUI_GENERAL_H
