#ifndef LFGUI_IMAGE_H
#define LFGUI_IMAGE_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>
#include <functional>
#include <cstring>
#include <assert.h>

#include "general.h"
#include "font.h"

namespace lfgui
{

/// \brief Contains and offers various image drawing and manipulation functions.
/// The pixel data can be in two different formats:
/// Default (when LFGUI_SEPARATE_COLOR_CHANNELS is not defined):
/// Pixel oriented: The pixel are stored as a whole as BGRA in one contiguous stream (BGRABGRABGRA...).
/// When LFGUI_SEPARATE_COLOR_CHANNELS is defined:
/// The pixel data starts with all the blue channel values of all pixels, then all green, all red and all alpha values
/// (BBB...GGG...RRR...AAA...).
class image
{
public:
    memory_wrapper image_data;
    int width_=0;
    int height_=0;

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
#ifdef LFGUI_SEPARATE_COLOR_CHANNELS
    uint8_t* data() const {return image_data.get();}
#else
    uint32_t* data() const {return (uint32_t*)image_data.get();}
#endif


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
        int i=x+y*width();

#ifdef LFGUI_SEPARATE_COLOR_CHANNELS
        uint8_t* d=data();
        int count=width()*height();
        d[i]=c.b;
        d[i+count]=c.g;
        d[i+count*2]=c.r;
        d[i+count*3]=c.a;
#else
        uint32_t* d=data();
        d[i]=c.value;
#endif
    }

    /// \brief Blends the pixel at position x,y with the given color. Blending means that the given color is drawn on
    /// top using the colors alpha.
    inline void blend_pixel(int index,int channel_size,uint8_t b,uint8_t g,uint8_t r,uint8_t a)
    {
        //if(x<0||y<0||x>=width()||y>=height()) // useful for debugging
        //    throw lfgui::exception("");
        if(a==0)
            return;

#ifdef LFGUI_SEPARATE_COLOR_CHANNELS
        uint8_t* d=data();
        d+=index;
        if(a==255)
        {
            *d=b;
            d+=channel_size;
            *d=g;
            d+=channel_size;
            *d=r;
            d+=channel_size;
            *d=255;
            return;
        }

        *d=(int((*d)*(255-a)+b*a)*(257))>>16;
        d+=channel_size;
        *d=(int((*d)*(255-a)+g*a)*(257))>>16;
        d+=channel_size;
        *d=(int((*d)*(255-a)+r*a)*(257))>>16;
        d+=channel_size;
        auto alpha=(*d)+a;
        *d=alpha>255?255:alpha;
#else
        color* d=((color*)data())+index;
        if(a==255)
        {
            *d=lfgui::color(r,g,b,255);
            return;
        }
        auto alpha=d->a+a;
        *d=lfgui::color(((d->r*int(255-a)+r*a)*(257))>>16,
                        ((d->g*int(255-a)+g*a)*(257))>>16,
                        ((d->b*int(255-a)+b*a)*(257))>>16,
                        alpha>255?255:alpha);
#endif
    }
    /// \brief Blends the pixel at position x,y with the given color. Blending means that the given color is drawn on
    /// top using the colors alpha.
    inline void blend_pixel(int index,color c)
    {
#ifdef LFGUI_SEPARATE_COLOR_CHANNELS
        blend_pixel(index,width()*height(),c.b,c.g,c.r,c.a);
#else
        blend_pixel(index,0,c.b,c.g,c.r,c.a);
#endif
    }
    /// \brief Blends the pixel at position x,y with the given color. Blending means that the given color is drawn on
    /// top using the colors alpha.
    inline void blend_pixel(int x,int y,color c)
    {
        blend_pixel(x+y*width(),c);
    }
    void blend_pixel_safe(int x,int y,color c)
    {
        if(x<0||y<0||x>=width()||y>=height())
            return;
        blend_pixel(x,y,c);
    }

    color get_pixel(int x,int y) const
    {
        int i=x+y*width();
#ifdef LFGUI_SEPARATE_COLOR_CHANNELS
        uint8_t* d=data();
        int count=width()*height();
        return color(*(d+i+count*2),*(d+i+count),*(d+i),*(d+i+count*3));
#else
        color* d=(color*)data();
        return d[i];
#endif
    }
    /// \brief Returns the pixel at the position given by an offset. offset=x+y*width()
    color get_pixel(int offset) const
    {
#ifdef LFGUI_SEPARATE_COLOR_CHANNELS
        int count=width()*height();
        return color(*(data()+offset+count*2),*(data()+offset+count),*(data()+offset),*(data()+offset+count*3));
#else
        color* d=(color*)data();
        return d[offset];
#endif
    }

    /// \brief The alignment specifies if the given coordinate should be left, centered, or right of the text. Multiple lines of text are not aligned correctly.
    void draw_text(int x,int y,const std::string& text,const color& color,int font_size=15,alignment a=alignment::left,font& f=font::default_font());

    void draw_character(int x,int y,unsigned int character,const color& color,int font_size=15,font& f=font::default_font());
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
    /// \brief Draw a path along the given points. The last point is connected with the first if connect_last_point_with_first is set to true.
    void draw_path(const std::vector<point>& vec,color _color,bool connect_last_point_with_first=false);
    void draw_rect(int x,int y,int width,int height,color color);
    void draw_rect(rect rectangle,color color)
    {
        draw_rect(rectangle.x,rectangle.y,rectangle.width,rectangle.height,color);
    }
    void draw_rect_lines(int x,int y,int width,int height,color color,float thickness=1,float fading_start=0.7)
    {
        draw_line(x      ,y       ,x+width,y       ,color,thickness,fading_start);  // top
        draw_line(x      ,y       ,x      ,y+height,color,thickness,fading_start);  // left
        draw_line(x+width,y       ,x+width,y+height,color,thickness,fading_start);  //  right
        draw_line(x      ,y+height,x+width,y+height,color,thickness,fading_start);  // bottom
    }
    /// \brief Draws a filled polygon.
    void draw_polygon(const std::vector<point>& vec,color color);
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
    void draw_image(int x,int y,const image& img,lfgui::rect area=lfgui::rect());
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

    /// \brief Fills the image with the given value.
    void clear(uint8_t value=0)
    {
        memset(data(),value,count()*4);
    }

    /// \brief Returns a rect with the size of this image.
    lfgui::rect rect()const{return lfgui::rect(0,0,width(),height());}

    /// \brief Used to set a function used to load images. This function is set by the wrappers.
    static std::function<image(std::string)> load;
};

}   // namespace lfgui

#endif // LFGUI_IMAGE_H
