#ifndef LFGUI_GEOMETRY_H
#define LFGUI_GEOMETRY_H

#include <algorithm>
#include <cmath>

namespace lfgui
{

struct exception : public std::exception
{
    std::string text;
    static std::function<void(const std::string&)>& custom_handler()
    {
        static std::function<void(const std::string&)> ch;
        return ch;
    }

    exception(std::string text)
    {
        std::cerr<<"EXCEPTION: "<<text<<std::endl;

        if(custom_handler())
            custom_handler()(text);
        else
        {
            __debugbreak();
            exit(-1);
        }
    }

    virtual const char* what() const noexcept
    {
        return text.c_str();
    }
};

/// \brief Represent a coordinate with its two x and y integers. Sometimes also a size.
template<typename T>
struct point_general
{
    T x;
    T y;

    explicit point_general(T x=0,T y=0) : x(x),y(y){}
    point_general(std::initializer_list<T> l)
    {
        if(l.size()!=2)
            throw lfgui::exception("point_general(std::initializer_list<T> l) not used with two elements");
        auto it=l.begin();
        x=*it;
        it++;
        y=*it;
    }

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
    point_general<T> operator+(const point_general<T>& o)const{return point_general<T>(x+o.x,y+o.y);}
    point_general<T> operator-(const point_general<T>& o)const{return point_general<T>(x-o.x,y-o.y);}
    point_general<T> operator*(const point_general<T>& o)const{return point_general<T>(x*o.x,y*o.y);}
    point_general<T> operator/(const point_general<T>& o)const{return point_general<T>(x/o.x,y/o.y);}

    point_general<T> operator/(int   v)const{return point_general<T>(x/v,y/v);}
    point_general<T> operator*(int   v)const{return point_general<T>(x*v,y*v);}
    point_general<T> operator+(int   v)const{return point_general<T>(x+v,y+v);}
    point_general<T> operator-(int   v)const{return point_general<T>(x-v,y-v);}
    point_general<T> operator/(float v)const{return point_general<T>(x/v,y/v);}
    point_general<T> operator*(float v)const{return point_general<T>(x*v,y*v);}
    point_general<T> operator+(float v)const{return point_general<T>(x+v,y+v);}
    point_general<T> operator-(float v)const{return point_general<T>(x-v,y-v);}
    point_general<T> operator-()const{return point_general<T>(-x,-y);}

    T distance(const point_general<T>& o)
    {
        T dx=x-o.x;
        T dy=y-o.y;
        return sqrt(dx*dx+dy*dy);
    }
    T distance_squared(const point_general<T>& o)
    {
        T dx=x-o.x;
        T dy=y-o.y;
        return dx*dx+dy*dy;
    }
    T distance_squared(T x_,T y_)
    {
        T dx=x-x_;
        T dy=y-y_;
        return dx*dx+dy*dy;
    }
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

/// \brief Used to position and size widgets.
/// The position is: size_absolute+pos_percent*parent_size+offset_percent*widget_size.
/// The size is: size_absolute+size_percent*parent_size.
struct widget_geometry
{
    point pos_absolute;
    point_float pos_percent;
    point size_absolute=point(100,100);
    point_float size_percent;
    point_float offset_percent;

    point pos_absolute_min;
    point_float pos_percent_min;
    point size_absolute_min=point(0,0);
    point_float size_percent_min;

    point pos_absolute_max;
    point_float pos_percent_max;
    point size_absolute_max=point(0,0);
    point_float size_percent_max;

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
        int w=size_absolute.x+size_percent.x*parent_width;
        int h=size_absolute.y+size_percent.y*parent_height;
        w=std::max<int>(size_absolute_min.x+size_percent_min.x*parent_width,w);
        if(size_absolute_max.x>0||size_percent_max.x>0)
            w=std::min<int>(size_absolute_max.x+size_percent_max.x*parent_width,w);
        h=std::max<int>(size_absolute_min.y+size_percent_min.y*parent_height,h);
        if(size_absolute_max.y>0||size_percent_max.y>0)
            h=std::min<int>(size_absolute_max.y+size_percent_max.y*parent_height,h);
        return point(w,h);
    }

    widget_geometry& set_pos(int x,int y,float x_percent=0,float y_percent=0)
    {
        pos_absolute.x=x;
        pos_absolute.y=y;
        pos_percent.x=x_percent;
        pos_percent.y=y_percent;
        return *this;
    }

    widget_geometry& set_pos_min(int x,int y,float x_percent=0,float y_percent=0)
    {
        pos_absolute_min.x=x;
        pos_absolute_min.y=y;
        pos_percent_min.x=x_percent;
        pos_percent_min.y=y_percent;
        return *this;
    }

    widget_geometry& set_pos_max(int x,int y,float x_percent=0,float y_percent=0)
    {
        pos_absolute_max.x=x;
        pos_absolute_max.y=y;
        pos_percent_max.x=x_percent;
        pos_percent_max.y=y_percent;
        return *this;
    }

    widget_geometry& set_size(int x,int y,float x_percent=0,float y_percent=0)
    {
        size_absolute.x=x;
        size_absolute.y=y;
        size_percent.x=x_percent;
        size_percent.y=y_percent;
        return *this;
    }

    widget_geometry& set_size_min(int x,int y,float x_percent=0,float y_percent=0)
    {
        size_absolute_min.x=x;
        size_absolute_min.y=y;
        size_percent_min.x=x_percent;
        size_percent_min.y=y_percent;
        return *this;
    }

    widget_geometry& set_size_max(int x,int y,float x_percent=0,float y_percent=0)
    {
        size_absolute_max.x=x;
        size_absolute_max.y=y;
        size_percent_max.x=x_percent;
        size_percent_max.y=y_percent;
        return *this;
    }

    widget_geometry& set_offset(float x_percent,float y_percent=0)
    {
        offset_percent.x=x_percent;
        offset_percent.y=y_percent;
        return *this;
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

#endif  // LFGUI_GEOMETRY_H
