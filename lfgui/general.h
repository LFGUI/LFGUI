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

    /// \brief Returns a copy of this color with its alpha multiplied by f.
    color alpha_multiplied(int f) const
    {
        return color(r,g,b,a*f/255);
    }
};

/// \brief Represent a coordinate with its two x and y integers. Sometimes also a size.
template<typename T>
struct point_general
{
    T x;
    T y;

    point_general(T x=0,T y=0) : x(x),y(y){}

    bool operator<(const point_general<T>& o)const
    {
        if(y<o.y)
            return true;
        if(y==o.y&&x<o.x)
            return true;
        return false;
    }
    bool operator<=(const point_general<T>& o)const
    {
        if(y<=o.y)
            return true;
        if(y==o.y&&x<=o.x)
            return true;
        return false;
    }

    bool operator!=(const point_general<T>& o)const
    {
        if(y!=o.y||x!=o.x)
            return true;
        return false;
    }

    void operator+=(const point_general<T>& o)
    {
        x+=o.x;
        y+=o.y;
    }
    void operator-=(const point_general<T>& o)
    {
        x-=o.x;
        y-=o.y;
    }
    void operator*=(const point_general<T>& o)
    {
        x*=o.x;
        y*=o.y;
    }
    void operator/=(const point_general<T>& o)
    {
        x/=o.x;
        y/=o.y;
    }

    point_general<T> operator/(int div)const{return point_general<T>(x/div,y/div);}
    point_general<T> operator*(int v)const{return point_general<T>(x*v,y*v);}
    point_general<T> operator+(int v)const{return point_general<T>(x+v,y+v);}
    point_general<T> operator-(int v)const{return point_general<T>(x-v,y-v);}
    point_general<T> operator-()const{return point_general<T>(-x,-y);}
};

using point=point_general<int>;
using point_float=point_general<float>;

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

/// \brief
struct widget_geometry
{
    point pos_absolute;
    point_float pos_percent;
    point size_absolute=point(100,100);
    point_float size_percent;
    point_float offset_percent;

    rect calc_geometry(int parent_width,int parent_height) const
    {
        point p(pos_absolute.x+pos_percent.x*parent_width,
                pos_absolute.y+pos_percent.y*parent_height);
        point s=calc_size(parent_width,parent_height);
        p+=point(offset_percent.x*s.x,
                 offset_percent.y*s.y);
        return rect(p.x,p.y,s.x,s.y);
    }

    point calc_pos(int parent_width,int parent_height) const
    {
        rect r=calc_geometry(parent_width,parent_height);
        return point(r.x,r.y);
    }

    point calc_size(int parent_width,int parent_height) const
    {
        return point(size_absolute.x+size_percent.x*parent_width,
                     size_absolute.y+size_percent.y*parent_height);
    }

    void set_pos(int x,int y,float x_percent,float y_percent)
    {
        pos_absolute.x=x;
        pos_absolute.y=y;
        pos_percent.x=x_percent;
        pos_percent.y=y_percent;
    }

    void set_size(int x,int y,float x_percent,float y_percent)
    {
        size_absolute.x=x;
        size_absolute.y=y;
        size_percent.x=x_percent;
        size_percent.y=y_percent;
    }

    void set_offset(float x_percent,float y_percent)
    {
        offset_percent.x=x_percent;
        offset_percent.y=y_percent;
    }
};

}   // namespace lfgui

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
