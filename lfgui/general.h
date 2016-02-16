#ifndef LFGUI_GENERAL_H
#define LFGUI_GENERAL_H

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
};

/// \brief Represent a coordinate with its two x and y integers. Sometimes also a size.
struct point
{
    int x;
    int y;

    point(int x=0,int y=0) : x(x),y(y){}

    bool operator<(const point& o)const
    {
        if(y<o.y)
            return true;
        if(y==o.y&&x<o.x)
            return true;
        return false;
    }
    bool operator<=(const point& o)const
    {
        if(y<=o.y)
            return true;
        if(y==o.y&&x<=o.x)
            return true;
        return false;
    }

    bool operator!=(const point& o)const
    {
        if(y!=o.y||x!=o.x)
            return true;
        return false;
    }

    void operator+=(const point& o)
    {
        x+=o.x;
        y+=o.y;
    }
    void operator-=(const point& o)
    {
        x-=o.x;
        y-=o.y;
    }
    void operator*=(const point& o)
    {
        x*=o.x;
        y*=o.y;
    }
    void operator/=(const point& o)
    {
        x/=o.x;
        y/=o.y;
    }

    point operator/(int div)const{return point(x/div,y/div);}
    point operator*(int v)const{return point(x*v,y*v);}
    point operator+(int v)const{return point(x+v,y+v);}
    point operator-(int v)const{return point(x-v,y-v);}
    point operator-()const{return point(-x,-y);}
};

struct rect
{
    int x;
    int y;
    int width;
    int height;

    rect(int x=0,int y=0,int width=0,int height=0) : x(x),y(y),width(width),height(height){}

    /// \brief Checks if the given point is inside this rectangle.
    bool contains(point p) const
    {
        if(x<=p.x&&p.x<=right()&&y<=p.y&&p.y<=bottom())
            return true;
        return false;
    }

    int top()const{return y;}
    int left()const{return x;}
    int bottom()const{return y+height;}
    int right()const{return x+width;}
};

}

inline std::ostream& operator<<(std::ostream& os,const lfgui::point& p)
{
    os<<p.x<<":"<<p.y;
    return os;
}

inline std::ostream& operator<<(std::ostream& os,const lfgui::rect& r)
{
    os<<r.x<<":"<<r.y<<' '<<r.width<<"x"<<r.height;
    return os;
}

inline std::ostream& operator<<(std::ostream& os,const lfgui::color& c)
{
    os<<(int)c.r<<","<<(int)c.g<<','<<(int)c.b<<","<<(int)c.a;
    return os;
}

#endif  // LFGUI_GENERAL_H