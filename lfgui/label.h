#ifndef LFGUI_LABEL_H
#define LFGUI_LABEL_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>

#include "lfgui.h"

namespace lfgui
{

/// \brief Simple text label.
class label : public widget
{
protected:
    std::string _text;
    color _text_color={255,255,255};
    int _text_size=16;
public:
    label(int x,int y,int width,int height=20,const std::string& text="",color text_color={255,255,255},int text_size=16)
        : widget(x,y,width,height),_text(text),_text_color(text_color),_text_size(text_size)
    {
        on_paint([this](lfgui::image& img)
        {
            img.draw_text(0,0,_text,_text_color,_text_size);
        });
        set_focusable(false);
    }

    label(int width=100,int height=20) : label(0,0,width,height){}
    label(const std::string& text,color text_color={0,0,0},int text_size=14) : label(0,0,100,20,text,text_color,text_size){}
    label(const std::string& text,int text_size) : label(0,0,100,20,text,{0,0,0},text_size){}

    std::string text(){return _text;}
    void set_text(const std::string& t){_text=t;}
    color text_color(){return _text_color;}
    void set_text_color(color c){_text_color=c;}
    int text_size(){return _text_size;}
    void set_text_size(int s){_text_size=s;}
};

}   // namespace lfgui

#endif // LFGUI_LABEL_H
