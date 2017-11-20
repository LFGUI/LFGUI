#define STB_TRUETYPE_IMPLEMENTATION
#include "../stb_truetype.h"
#undef STB_TRUETYPE_IMPLEMENTATION

#include "font.h"
#include "geometry.h"

#include <iostream>

namespace lfgui
{

uint32_t utf8_to_unicode(char*& data,size_t len)
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

font::font(const std::string& filename)
{
    STK_STACKTRACE
    std::streampos size;

    std::ifstream file(filename,std::ios::in|std::ios::binary|std::ios::ate);
    if(!file.is_open())
        throw lfgui::exception("LFGUI Error: Can't load font file \""+filename+"\".");

    size=file.tellg();
    ttf_buffer.reset(new memory_wrapper(size));
    file.seekg(0,std::ios::beg);
    file.read((char*)ttf_buffer.get()->get(),size);
    file.close();

    stbtt_font.reset(new stbtt_fontinfo);
    stbtt_InitFont(stbtt_font.get(),ttf_buffer.get()->get(),stbtt_GetFontOffsetForIndex(ttf_buffer.get()->get(),0));

    stbtt_GetFontVMetrics(stbtt_font.get(),&ascend_,&descend_,&line_gap_);
}

font::font(const char* data,size_t)
{
    STK_STACKTRACE
    stbtt_font.reset(new stbtt_fontinfo);
    stbtt_InitFont(stbtt_font.get(),(const unsigned char*)data,stbtt_GetFontOffsetForIndex((const unsigned char*)data,0));

    stbtt_GetFontVMetrics(stbtt_font.get(),&ascend_,&descend_,&line_gap_);
}

int font::ascend(int font_size)const{return ascend_*stbtt_ScaleForPixelHeight(stbtt_font.get(),font_size);}
int font::descend(int font_size)const{return descend_*stbtt_ScaleForPixelHeight(stbtt_font.get(),font_size);}
int font::line_gap(int font_size)const{return line_gap_*stbtt_ScaleForPixelHeight(stbtt_font.get(),font_size);}
int font::line_height(int font_size)const{return (line_gap_+ascend_-descend_)*stbtt_ScaleForPixelHeight(stbtt_font.get(),font_size);;}

font::bitmap font::get_glyph(unsigned int character,int font_size)
{
    bitmap b;
    int width;
    int height;
    float s=stbtt_ScaleForPixelHeight(stbtt_font.get(),font_size);
    stbtt_GetCodepointBitmapBox(stbtt_font.get(),character,s,s,&b.x0,&b.y0,&b.x1,&b.y1);
//std::cout<<b.x0<<' '<<b.y0<<' '<<b.x1<<' '<<b.y1<<std::endl;
    b.data=stbtt_GetCodepointBitmap(stbtt_font.get(),s,s,character,&width,&height,0,0);
    return b;
}

font::bitmap& font::get_glyph_cached(unsigned int character,size_t font_size)
{
    if(glyph_cache.size()<=font_size)
        glyph_cache.resize(font_size+1);

    if(character<glyph_cache[font_size].size())
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

int font::text_length(const std::string& text,int font_size)
{
    int w=0;
    char* end=(char*)text.data()+text.size();
    for(char* data=(char*)text.data();data<end;data++)
        w+=character_width(utf8_to_unicode(data,end-data),font_size);
    return w;
}

int font::text_length(const std::string& text,int font_size,size_t start_character,size_t end_character)
{
    int w=0;
    char* end=(char*)text.data()+end_character;
    for(char* data=(char*)text.data()+start_character;data<end;data++)
        w+=character_width(utf8_to_unicode(data,end-data),font_size);
    return w;
}

int font::character_width(uint32_t codepoint,int font_size)
{
    int w1;
    int w2;
    stbtt_GetCodepointHMetrics(stbtt_font.get(),codepoint,&w1,&w2);
    float scale=stbtt_ScaleForPixelHeight(stbtt_font.get(),font_size);
    w1*=scale;
    w2*=scale;
    return w1;
}

font::~font(){}

}   // namespace lfgui
