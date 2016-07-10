#ifndef LFGUI_IMAGE_H
#define LFGUI_IMAGE_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>
#include <functional>

#include "general.h"
#include "font.h"
#include "../external/stk_memory_plain.h"

namespace cimg_library
{
template<typename T>
class CImg;
}

namespace lfgui
{

/// \brief Contains a cimg and offers various drawing and manipulation functions. The cIMG library trores its pixel
/// data in seperate channels. The pixel data start with all the blue channel value of all pixels, then all green, all
/// red and all alpha values.
class image
{
public:
    typedef cimg_library::CImg<unsigned char> cimg;
    std::unique_ptr<cimg> cimage;

    /// \brief Tries to load an image from the given filename.
    image(std::string filename);
    /// \brief Constructs an image with the given width and height.
    image(int width=0,int height=0);
    image(const image& o);
    image& operator=(const image& o);
    ~image();

    int width()const;
    int height()const;
    /// \brief Returns the pixel count which is width()*height().
    int count()const{return width()*height();}
    /// \brief Returns a pointer to the pixel data.
    uint8_t* data()const;

    /// \brief Scales this image. Uses "nearest" scaling.
    image& resize_nearest(int w,int h);
    /// \brief Scales this image. Uses linear scaling.
    image& resize_linear(int w,int h);
    /// \brief Scales this image. Uses cubic scaling.
    image& resize_cubic(int w,int h);
    /// \brief Scales this image. Same as resize_linear().
    image& scale(int w,int h){return resize_linear(w,h);}

    /// \brief Returns a scaled version of this image. Uses "nearest" scaling.
    image resized_nearest(int w,int h)const{image ret(*this);ret.resize_nearest(w,h);return ret;}
    /// \brief Returns a scaled version of this image. Uses linear scaling.
    image resized_linear(int w,int h)const{image ret(*this);ret.resize_linear(w,h);return ret;}
    /// \brief Returns a scaled version of this image. Uses cubic scaling.
    image resized_cubic(int w,int h)const{image ret(*this);ret.resize_cubic(w,h);return ret;}
    /// \brief Returns a scaled version of this image. Same as resize_linear().
    image scaled(int w,int h)const{return resized_linear(w,h);}

    /// \brief Rotates the image by 90 degrees clockwise.
    image& rotate90(){*this=rotated90();return *this;}
    /// \brief Rotates the image by 180 degrees clockwise.
    image& rotate180();
    /// \brief Rotates the image by 270 degrees clockwise.
    image& rotate270(){*this=rotated270();return *this;}
    /// \brief Returns a by 180 degrees clockwise rotated image.
    image rotated90() const;
    /// \brief Returns a by 180 degrees clockwise rotated image.
    image rotated180() const {auto ret=*this;ret.rotate180();return ret;}
    /// \brief Returns a by 180 degrees clockwise rotated image.
    image rotated270() const;

    /// \brief Crops the image to the given size.
    image& crop(int x,int y,int w,int h);
    /// \brief Returns a cropped version of this image.
    image cropped(int x,int y,int w,int h)const{image ret(*this);ret.crop(x,y,w,h);return ret;}

    /// \brief Multiplies the color of every pixel with the given color. Can be used to colorize the image. Alpha is
    /// not affected.
    image& multiply(color c);
    /// \brief Returns an image with the color of every pixel multiplied with the given color. Can be used to get a
    /// colorized image. The alpha is not changed.
    image multiplied(color c)const{image ret(*this);ret.multiply(c);return ret;}

    /// \brief Multiplies the color of every pixel with the given color. Can be used to colorize the image. Alpha is
    /// not affected.
    image& add(color c);
    /// \brief Returns an image with the color of every pixel multiplied with the given color. Can be used to get a
    /// colorized image. The alpha is not changed.
    image added(color c)const{image ret(*this);ret.add(c);return ret;}

    void set_pixel(int x,int y,color c)
    {
        int count=width()*height();
        int i=x+y*width();
        auto d=data();
        d[i]=c.b;
        d[i+count]=c.g;
        d[i+count*2]=c.r;
        d[i+count*3]=c.a;
    }

    /// \brief Blends the pixel at position x,y with the given color. Blending means that the given color is drawn on
    /// top using the colors alpha.
    inline void blend_pixel(int x,int y,color c)
    {
        //if(x<0||y<0||x>=width()||y>=height()) // useful for debugging
        //    throw std::logic_error("");
        if(c.a==0)
            return;

        int count=width()*height();
        int i=x+y*width();
        uint8_t* d=data();
        d+=i;
        if(c.a==255)
        {
            *d=c.b;
            d+=count;
            *d=c.g;
            d+=count;
            *d=c.r;
            d+=count;
            *d=255;
            return;
        }

        *d=((*d)*(255-c.a)+c.b*c.a)/255;
        d+=count;
        *d=((*d)*(255-c.a)+c.g*c.a)/255;
        d+=count;
        *d=((*d)*(255-c.a)+c.r*c.a)/255;
        d+=count;
        auto a=(*d)+c.a;
        *d=a>255?255:a;
    }
    void blend_pixel_safe(int x,int y,color c)
    {
        if(x<0||y<0||x>=width()||y>=height())
            return;
        blend_pixel(x,y,c);
    }

    /// \brief Blends the pixel at position x,y with the given color. Blending means that the given color is drawn on
    /// top using the colors alpha.
    inline void blend_pixel(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int index,uint8_t b,uint8_t g,uint8_t r,uint8_t a)
    {
        //if(x<0||y<0||x>=width()||y>=height()) // useful for debugging
        //    throw std::logic_error("");
        if(a==0)
            return;

        int count=width()*height();
        if(depth<depth_buffer[index])
            return;
        uint8_t* d=data();
        d+=index;
        if(a==255)
        {
            *d=b;
            d+=count;
            *d=g;
            d+=count;
            *d=r;
            d+=count;
            *d=255;
            depth_buffer[index]=depth;
            return;
        }

        *d=((*d)*(255-a)+b*a)/255;
        d+=count;
        *d=((*d)*(255-a)+g*a)/255;
        d+=count;
        *d=((*d)*(255-a)+r*a)/255;
        d+=count;
        auto alpha=(*d)+a;
        *d=alpha>255?255:alpha;
    }
    /// \brief Blends the pixel at position x,y with the given color. Blending means that the given color is drawn on
    /// top using the colors alpha.
    inline void blend_pixel(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int index,color c)
    {
        blend_pixel(depth_buffer,depth,index,c.b,c.g,c.r,c.a);
    }
    /// \brief Blends the pixel at position x,y with the given color. Blending means that the given color is drawn on
    /// top using the colors alpha.
    inline void blend_pixel(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int x,int y,color c)
    {
        blend_pixel(depth_buffer,depth,x+y*width(),c);
    }
    void blend_pixel_safe(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int x,int y,color c)
    {
        if(x<0||y<0||x>=width()||y>=height())
            return;
        blend_pixel(depth_buffer,depth,x,y,c);
    }

    color get_pixel(int x,int y) const
    {
        int count=width()*height();
        int i=x+y*width();
        auto d=data();
        return color(*(d+i+count*2),*(d+i+count),*(d+i),*(d+i+count*3));
    }
    /// \brief Returns the pixel at the position given by an offset. offset=x+y*width()
    color get_pixel(int offset) const
    {
        int count=width()*height();
        return color(*(data()+offset+count*2),*(data()+offset+count),*(data()+offset),*(data()+offset+count*3));
    }

    /// \brief The alignment specifies if the given coordinate should be left, centered, or right of the text. Multiple lines of text are not aligned correctly.
    void draw_text(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int x,int y,const std::string& text,const color& color,int font_size=15,alignment a=alignment::left,font& f=font::default_font());

    void draw_character(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int& x,int y,unsigned int character,const color& color,int font_size=15,font& f=font::default_font());
    void draw_line(int x1,int y1,int x2,int y2,color _color);
    /// \brief Draws a line with the given thickness. The drawn color gets more transparent when further away from the
    /// center of the line. This can be adjusted with the fading parameter where 1 is no fading and 0 fading starting in the center.
    void draw_line(int x1,int y1,int x2,int y2,color _color,float thickness,float fading=0.7);
    void draw_line(point start,point end,color _color)
    {
        draw_line(start.x,start.y,end.x,end.y,_color);
    }
    /// \brief Draws a line with the given thickness. The drawn color gets more transparent when further away from the
    /// center of the line. This can be adjusted with the fading parameter where 1 is no fading and 0 fading starting in the center.
    void draw_line(point start,point end,color _color,float width,float fading_start=0.7)
    {
        draw_line(start.x,start.y,end.x,end.y,_color,width,fading_start);
    }
    void draw_line(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int x1,int y1,int x2,int y2,color _color);
    /// \brief Draws a line with the given thickness. The drawn color gets more transparent when further away from the
    /// center of the line. This can be adjusted with the fading parameter where 1 is no fading and 0 fading starting in the center.
    void draw_line(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int x1,int y1,int x2,int y2,color _color,float thickness,float fading=0.7);
    void draw_line(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,point start,point end,color _color)
    {
        draw_line(depth_buffer,depth,start.x,start.y,end.x,end.y,_color);
    }
    /// \brief Draws a line with the given thickness. The drawn color gets more transparent when further away from the
    /// center of the line. This can be adjusted with the fading parameter where 1 is no fading and 0 fading starting in the center.
    void draw_line(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,point start,point end,color _color,float width,float fading_start=0.7)
    {
        draw_line(depth_buffer,depth,start.x,start.y,end.x,end.y,_color,width,fading_start);
    }
    /// \brief Draw a path along the given points. The last point is connected with the first if connect_last_point_with_first is set to true.
    void draw_path(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,const std::vector<point>& vec,color _color,bool connect_last_point_with_first=false);
    /*void draw_rect(int x,int y,int width,int height,color color);
    void draw_rect(rect rectangle,color color)
    {
        draw_rect(depth_buffer,depth,rectangle.x,rectangle.y,rectangle.width,rectangle.height,color);
    }*/
    void draw_rect(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int x,int y,int width,int height,color color);
    void draw_rect(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,rect rectangle,color color)
    {
        draw_rect(depth_buffer,depth,rectangle.x,rectangle.y,rectangle.width,rectangle.height,color);
    }
    /// \brief Draws a filled polygon.
    void draw_polygon(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,const std::vector<point>& vec,color color);
    /// \brief Fills the whole image with one color.
    void fill(color color);

    /// \brief Returns the pixel length of the given text from start_character to end_character.
    int text_length(const std::string& str,int font_size,size_t start_character,size_t end_character)const
    {
        return font::default_font().text_length(str,font_size,start_character,end_character);
    }
    /// \brief Returns the pixel length of the given text to end_character.
    int text_length(const std::string& str,int font_size,size_t end_character=UINT_MAX)const
    {
        if(end_character>str.size())
            end_character=str.size();
        return text_length(str,font_size,0,end_character);
    }

    /// \brief Draws another image onto this one.
    void draw_image(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int x,int y,const image& img);
    /// \brief Draws another image onto this one.
    void draw_image(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int x,int y,const image& img,float opacity);
    /// \brief Draws another image onto this one.
    void draw_image(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,point p,const image& img){draw_image(depth_buffer,depth,p.x,p.y,img);}
    /// \brief Draws another image onto this one.
    void draw_image(stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,point p,const image& img,float opacity){draw_image(depth_buffer,depth,p.x,p.y,img,opacity);}

    /// \brief Draws another image onto this one.
    void draw_image(int x,int y,const image& img);
    /// \brief Draws another image onto this one.
    void draw_image(int x,int y,const image& img,float opacity);
    /// \brief Draws another image onto this one.
    void draw_image(point p,const image& img){draw_image(p.x,p.y,img);}
    /// \brief Draws another image onto this one.
    void draw_image(point p,const image& img,float opacity){draw_image(p.x,p.y,img,opacity);}

    /// \brief Draws another image onto this one.
    void draw_image_solid(int x,int y,const image& img);
    /// \brief Draws another image onto this one.
    void draw_image_solid(point p,const image& img){draw_image_solid(p.x,p.y,img);}

    /// \brief Fills this image with the given image, it is stretched to act as a border with "stretched filling".
    void draw_image_corners_stretched(int border_width,const image& img);

    /// \brief Fills the image with transparent black.
    void clear();

    /// \brief Returns a rect with the size of this image.
    lfgui::rect rect()const{return lfgui::rect(0,0,width(),height());}

    /// \brief Used to set a function used to load images. This function is set by the wrappers.
    static std::function<image(std::string)> load;
};

}   // namespace lfgui

#endif // LFGUI_IMAGE_H
