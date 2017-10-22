#ifndef LFGUI_FONT_H
#define LFGUI_FONT_H

#include <vector>
#include <fstream>
#include <iostream>
#include <memory>
#include <exception>
#include <functional>
#include <intrin.h>

struct stbtt_fontinfo;

namespace lfgui
{

struct memory_wrapper
{
    uint8_t* ptr_=0;
    int size_=0;
    bool foreign_data=false;

    memory_wrapper(size_t size=0):size_(size)
    {
        if(size)
            ptr_=(uint8_t*)malloc(size);
    }

    memory_wrapper(void* data,size_t size):size_(size),ptr_((uint8_t*)data),foreign_data(true)
    {
    }

    ~memory_wrapper()
    {
        if(ptr_&&!foreign_data)
            free(ptr_);
    }

    uint8_t* get()const{return ptr_;}
    int size()const{return size_;}

    void reset(size_t size)
    {
        if(ptr_&&!foreign_data)
            free(ptr_);
        foreign_data=false;
        if(size)
            ptr_=(uint8_t*)malloc(size);
        size_=size;
    }

    void reset(void* data,size_t size)
    {
std::cout<<"resetting to"<<(size_t)data<<std::endl;
        if(ptr_&&!foreign_data)
            free(ptr_);
        foreign_data=true;
        ptr_=(uint8_t*)data;
        size_=size;
    }

    memory_wrapper(memory_wrapper&& o)
    {
        ptr_=o.ptr_;
        size_=o.size_;
        foreign_data=o.foreign_data;
        o.ptr_=0;
        o.size_=0;
    }
    memory_wrapper& operator=(memory_wrapper&& o)
    {
        ptr_=o.ptr_;
        size_=o.size_;
        foreign_data=o.foreign_data;
        o.ptr_=0;
        o.size_=0;
        return *this;
    }

/*    uint8_t& operator[](int i)
    {
        if(i<0||i>size_)
            __builtin_trap();
        return ptr_[i];
    }*/

    memory_wrapper(const memory_wrapper&)=delete;
    bool operator=(const memory_wrapper&)=delete;
};

/// Set this path if the needed ressources (images&fonts) are not in the execution path. For example "data/". The "/" at the end is required.
extern std::string ressource_path;

// based on https://en.wikipedia.org/wiki/UTF-8
extern uint32_t utf8_to_unicode(char*& data,size_t len);

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

    // ascent is the coordinate above the baseline the font extends; descent
    // is the coordinate below the baseline the font extends (i.e. it is typically negative)
    // lineGap is the spacing between one row's descent and the next row's ascent...
    // so you should advance the vertical position by "*ascent - *descent + *lineGap"
    //   these are expressed in unscaled coordinates, so you must multiply by
    //   the scale factor for a given size
    int ascend_;
    int descend_;
    int line_gap_;
    int ascend(int font_size)const;
    int descend(int font_size)const;
    int line_gap(int font_size)const;
    int line_height(int font_size)const;

    std::shared_ptr<stbtt_fontinfo> stbtt_font;
    std::shared_ptr<memory_wrapper> ttf_buffer;    ///< \brief The font file is loaded into this buffer. stb_truetype needs that.
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

    int character_width(uint32_t codepoint,int font_size);

    /// \brief Returns the default font "FreeSans.ttf". Can also be used to set a different default.
    static font& default_font()
    {
        static font f(ressource_path+"FreeSans.ttf");
        return f;
    }
};

}   // namespace lfgui

#endif // LFGUI_FONT_H
