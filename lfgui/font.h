#ifndef LFGUI_FONT_H
#define LFGUI_FONT_H

#include <vector>
#include <fstream>
#include <memory>
#include <exception>

struct stbtt_fontinfo;

namespace lfgui
{

// based on https://en.wikipedia.org/wiki/UTF-8
extern uint32_t utf8_to_unicode(const char*& data,size_t len);

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
    std::unique_ptr<stbtt_fontinfo> stbtt_font;
    std::vector<uint8_t> ttf_buffer;    ///< \brief The font file is loaded into this buffer. stb_truetype needs that.
    /// \brief When using the get_glyph_cached function all characters are cached in this structure. It is glyph_cache[font_size][character].
    std::vector<std::vector<bitmap>> glyph_cache;
public:
    /// \brief Loads a TrueType font from a file.
    font(const std::string& filename);

    ~font();

    /// \brief Returns a single drawn character.
    bitmap get_glyph(unsigned int character,int font_size);

    /// \brief Returns a single drawn character cached version. If the character has been drawn before the cached
    /// version is returned otherwise one is created and returned.
    bitmap& get_glyph_cached(unsigned int character,size_t font_size);

    /// \brief Returns the length of the given text in font size font_size in pixels.
    int text_length(const std::string& text,int font_size);

    /// \brief Returns the length of the given text starting at character start_character until end_character in font size font_size in pixels.
    int text_length(const std::string& text,int font_size,size_t start_character,size_t end_character);

    /// \brief Returns the default font "FreeSans.ttf". Can also be used to set a different default.
    static font& default_font()
    {
        static font f("FreeSans.ttf");
        return f;
    }
};

}   // namespace lfgui

#endif // LFGUI_FONT_H
