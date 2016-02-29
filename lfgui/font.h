#ifndef LFGUI_FONT_H
#define LFGUI_FONT_H

#include <vector>
#include <fstream>
#include <memory>
#include <exception>

#include "../external/stb_truetype.h"

namespace lfgui
{

// based on https://en.wikipedia.org/wiki/UTF-8
inline uint32_t utf8_to_unicode(const char*& data,size_t len)
{
    if((data[0]&0x80)==0)     // looks like 0xxx xxxx -> no UTF-8
        return data[0];
    uint32_t code=0;
    if((data[0]&0xE0)==0xC0)  // looks like 110x xxxx -> 2 byte UTF-8
    {
        if(len<2)
            return 0;
        code=data[0]&0x1F;
        code=code<<6;
        code+=data[1]&0x7F;
        data+=1;
    }
    else if((data[0]&0xF0)==0xE0)  // looks like 1110 xxxx -> 3 byte UTF-8
    {
        if(len<3)
            return 0;
        code=data[0]&0x0F;
        code=code<<6;
        code+=data[1]&0x7F;
        code=code<<6;
        code+=data[2]&0x7F;
        data+=2;
    }
    else if((data[0]&0xF8)==0xF0)  // looks like 1111 0xxx -> 4 byte UTF-8
    {
        if(len<4)
            return 0;
        code=data[0]&0x0F;
        code=code<<6;
        code+=data[1]&0x7F;
        code=code<<6;
        code+=data[2]&0x7F;
        code=code<<6;
        code+=data[3]&0x7F;
        data+=3;
    }
    return code;
}

/// \brief Wrapper class for stb_truetype.h.
class font
{
public:
    /// \brief A bitmap represents a single drawn character. It's a greyscale image.
    struct bitmap
    {
        int x0=0;
        int x1=0;
        int y0=0;
        int y1=0;
        unsigned char* data=0;
        int width()const{return x1-x0;}
        int height()const{return y1-y0;}

        bitmap(){}

        bitmap(const bitmap& o)
        {
            if(!o.valid())
            {
                x0=0;
                x1=0;
                y0=0;
                y1=0;
                data=0;
                return;
            }
            data=(unsigned char*)malloc(o.width()*o.height());
            x0=o.x0;
            x1=o.x1;
            y0=o.y0;
            y1=o.y1;
            memcpy(data,o.data,o.width()*o.height());
        }

        bitmap& operator=(const bitmap& o)
        {
            if(!o.valid())
            {
                x0=0;
                x1=0;
                y0=0;
                y1=0;
                data=0;
                return *this;
            }
            data=(unsigned char*)malloc(o.width()*o.height());
            x0=o.x0;
            x1=o.x1;
            y0=o.y0;
            y1=o.y1;
            memcpy(data,o.data,o.width()*o.height());
            return *this;
        }

        bitmap(bitmap&& o)
        {
            if(!o.valid())
            {
                x0=0;
                x1=0;
                y0=0;
                y1=0;
                data=0;
                return;
            }
            data=o.data;
            x0=o.x0;
            x1=o.x1;
            y0=o.y0;
            y1=o.y1;
            o.data=0;
        }

        bitmap& operator=(bitmap&& o)
        {
            if(!o.valid())
            {
                x0=0;
                x1=0;
                y0=0;
                y1=0;
                data=0;
                return *this;
            }
            data=o.data;
            x0=o.x0;
            x1=o.x1;
            y0=o.y0;
            y1=o.y1;
            o.data=0;
            return *this;
        }

        ~bitmap()
        {
            if(data)
                free(data);
            data=0;
        }

        bool valid()const{return width();}
    };
private:
    stbtt_fontinfo stbtt_font;
    std::vector<uint8_t> ttf_buffer;    ///< \brief The font file is loaded into this buffer. stb_truetype needs that.
    /// \brief When using the get_glyph_cached function all characters are cached in this structure. It is glyph_cache[font_size][character].
    std::vector<std::vector<bitmap>> glyph_cache;
public:
    /// \brief Loads a TrueType font from a file.
    font(const std::string& filename)
    {
        std::streampos size;

        std::ifstream file(filename,std::ios::in|std::ios::binary|std::ios::ate);
        if(!file.is_open())
            throw std::logic_error("LFGUI Error: Can't load font file \""+filename+"\".");

        size=file.tellg();
        ttf_buffer.resize(size);
        file.seekg(0,std::ios::beg);
        file.read((char*)ttf_buffer.data(),size);
        file.close();

        stbtt_InitFont(&stbtt_font,ttf_buffer.data(),stbtt_GetFontOffsetForIndex(ttf_buffer.data(),0));
    }

    /// \brief Returns a single drawn character.
    bitmap get_glyph(unsigned int character,int font_size)
    {
        bitmap b;
        int width;
        int height;
        float s=stbtt_ScaleForPixelHeight(&stbtt_font,font_size);
        stbtt_GetCodepointBitmapBox(&stbtt_font,character,s,s,&b.x0,&b.y0,&b.x1,&b.y1);
//std::cout<<b.x0<<' '<<b.y0<<' '<<b.x1<<' '<<b.y1<<std::endl;
        b.data=stbtt_GetCodepointBitmap(&stbtt_font,s,s,character,&width,&height,0,0);
        return b;
    }

    /// \brief Returns a single drawn character cached version. If the character has been drawn before the cached
    /// version is returned otherwise one is created and returned.
    bitmap& get_glyph_cached(unsigned int character,size_t font_size)
    {
        if(glyph_cache.size()<font_size)
            glyph_cache.resize(font_size+1);

        if(character<=glyph_cache[font_size].size())
        {
            if(glyph_cache[font_size][character].width()==0)
                goto create_glyph;
            return glyph_cache[font_size][character];
        }
        glyph_cache[font_size].resize(character+1);
create_glyph:
        glyph_cache[font_size][character]=get_glyph(character,font_size);
        return glyph_cache[font_size][character];
    }

    /// \brief Returns the length of the given text in font size font_size in pixels.
    int text_length(const std::string& text,int font_size)
    {
        int w=0;
        const char* end=text.data()+text.size();
        for(const char* data=text.data();data<end;data++)
        {
            if(*data==' ')
                w+=font_size/3;
            else
                w+=get_glyph_cached(utf8_to_unicode(data,end-data),font_size).width()+1;
        }
        return w;
    }

    /// \brief Returns the length of the given text starting at character start_character until end_character in font size font_size in pixels.
    int text_length(const std::string& text,int font_size,size_t start_character,size_t end_character)
    {
        int w=0;
        const char* end=text.data()+end_character;
        for(const char* data=text.data()+start_character;data<end;data++)
        {
            if(*data==' ')
                w+=font_size/3;
            else
                w+=get_glyph_cached(utf8_to_unicode(data,end-data),font_size).width()+1;    // an additional pixel space between each character
        }
        return w;
    }

    /// \brief Returns the default font "FreeSans.ttf". Can also be used to set a different default.
    static font& default_font()
    {
        static font f("FreeSans.ttf");
        return f;
    }
};

}   // namespace lfgui

#endif // LFGUI_FONT_H
