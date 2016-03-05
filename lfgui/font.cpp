#include "../external/stb_truetype.h"

#include "font.h"

namespace lfgui
{

uint32_t utf8_to_unicode(const char*& data,size_t len)
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
    std::streampos size;

    std::ifstream file(filename,std::ios::in|std::ios::binary|std::ios::ate);
    if(!file.is_open())
        throw std::logic_error("LFGUI Error: Can't load font file \""+filename+"\".");

    size=file.tellg();
    ttf_buffer.resize(size);
    file.seekg(0,std::ios::beg);
    file.read((char*)ttf_buffer.data(),size);
    file.close();

    stbtt_font.reset(new stbtt_fontinfo);
    stbtt_InitFont(stbtt_font.get(),ttf_buffer.data(),stbtt_GetFontOffsetForIndex(ttf_buffer.data(),0));
}

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

int font::text_length(const std::string& text,int font_size)
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

int font::text_length(const std::string& text,int font_size,size_t start_character,size_t end_character)
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

font::~font(){}

}   // namespace lfgui
