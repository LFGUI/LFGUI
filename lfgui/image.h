#ifndef LFGUI_IMAGE_H
#define LFGUI_IMAGE_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>

#include "general.h"
#include "font.h"

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

    /// \brief Crops the image to the given size.
    image& crop(int x,int y,int w,int h);
    /// \brief Returns a cropped version of this image.
    image cropped(int x,int y,int w,int h)const{image ret(*this);ret.crop(x,y,w,h);return ret;}

    /// \brief Multiplies the color of every pixel with the given color. Can be used to colorize the image. Alpha is
    /// not affected.
    image& multiply(color c)
    {
        int size=count();
        auto d=data();
        for(int i=0;i<size;i++)
        {
            *d=(*d)*c.b/255;
            d++;
        }
        for(int i=0;i<size;i++)
        {
            *d=(*d)*c.g/255;
            d++;
        }
        for(int i=0;i<size;i++)
        {
            *d=(*d)*c.r/255;
            d++;
        }
        return *this;
    }
    /// \brief Returns an image with the color of every pixel multiplied with the given color. Can be used to get a
    /// colorized image. The alpha is not changed.
    image multiplied(color c)const{image ret(*this);ret.multiply(c);return ret;}

    /// \brief Multiplies the color of every pixel with the given color. Can be used to colorize the image. Alpha is
    /// not affected.
    image& add(color c)
    {
        int size=count();
        auto d=data();
        for(int i=0;i<size;i++)
        {
            *d=std::min(255,(*d)+c.b);
            d++;
        }
        for(int i=0;i<size;i++)
        {
            *d=std::min(255,(*d)+c.g);
            d++;
        }
        for(int i=0;i<size;i++)
        {
            *d=std::min(255,(*d)+c.r);
            d++;
        }
        return *this;
    }
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
    void blend_pixel(int x,int y,color c);
    void blend_pixel_safe(int x,int y,color c)
    {
        if(x<0||y<0||x>=width()||y>=height())
            return;
        blend_pixel(x,y,c);
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
    void draw_text(int x,int y,const std::string& text,const color& color,int font_size=15,alignment a=alignment::left,font& f=font::default_font())
    {
        int x_orig=x;
        int w=f.text_length(text,font_size);
        if(a==alignment::center)
            x-=w/2;
        else if(a==alignment::right)
            x-=w;
        const char* end=text.data()+text.size();
        for(const char* data=text.data();data<end;data++)
        {
            if(*data=='\n')
            {
                x=x_orig;
                y+=font_size;
            }
            else
                draw_character(x,y,lfgui::utf8_to_unicode(data,end-data),color,font_size);
        }
    }

    void draw_character(int& x,int y,unsigned int character,const color& color,int font_size=15,font& f=font::default_font())
    {
        if(character==' ')
        {
            x+=font_size/3;
            return;
        }
        if(character=='\t')
        {
            x+=font_size/3*4;
            return;
        }
        if(character<0x20)
            return;
        const font::bitmap& b=f.get_glyph_cached(character,font_size);
        for(int y2=0;y2<b.height();y2++)
            for(int x2=0;x2<b.width();x2++) // just adding 13 seems weird. Maybe there has to be some other calculation.
                blend_pixel_safe(x+x2+b.x0,y+y2+b.y0+13,color.alpha_multiplied(b.data[x2+y2*b.width()]));
        x+=b.width()+1;
    }
    void draw_line(int x1,int y1,int x2,int y2,color _color);
    /// \brief Draw a path along the given points. The last point is connected with the first if connect_last_point_with_first is set to true.
    void draw_path(const std::vector<point>& vec,color _color,bool connect_last_point_with_first=false)
    {
        if(vec.size()<2)
            return;
        auto first_point=vec.begin();
        auto second_point=first_point;
        second_point++;
        while(second_point!=vec.end())
        {
            draw_line(first_point->x,first_point->y,second_point->x,second_point->y,_color);
            first_point=second_point;
            second_point++;
        }
        if(connect_last_point_with_first)
        {
            second_point=vec.begin();
            draw_line(first_point->x,first_point->y,second_point->x,second_point->y,_color);
        }
    }
    void draw_rect(int x,int y,int width,int height,color color);
    /// \brief Draws a filled polygon.
    void draw_polygon(const std::vector<point>& vec,color color);

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
    void draw_image(int x,int y,const image& img);
    /// \brief Draws another image onto this one.
    void draw_image(int x,int y,const image& img,float opacity);
    /// \brief Draws another image onto this one.
    void draw_image(point p,const image& img){draw_image(p.x,p.y,img);}
    /// \brief Draws another image onto this one.
    void draw_image(point p,const image& img,float opacity){draw_image(p.x,p.y,img,opacity);}

    /// \brief Fills the image with transparent black.
    void clear();

    /// \brief Used to set a function used to load images. This function is set by the wrappers.
    static std::function<image(std::string)> load;
};

}   // namespace lfgui

#endif // LFGUI_IMAGE_H
