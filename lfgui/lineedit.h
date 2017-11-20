#ifndef LFGUI_LINEEDIT_H
#define LFGUI_LINEEDIT_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>

#include "lfgui.h"
#include "../stk_timer.h"

namespace lfgui
{

/// \brief .
class lineedit : public widget
{
protected:
    std::string _text;
    size_t cursor_position=0;
    color _text_color={255,255,255};
    int _text_size=16;
    image img_background;
    image img_background_focused;
    stk::timer cursor_timer=stk::timer("",false);
public:
    lineedit(int x,int y,int _width,int _height=20,const std::string& text="",color text_color={0,0,0},int text_size=14);

    lineedit(int width=100,int height=20) : lineedit(0,0,width,height){}

    std::string text(){return _text;}
    void set_text(const std::string& t){_text=t;}
    color text_color(){return _text_color;}
    void set_text_color(color c){_text_color=c;}
    int text_size(){return _text_size;}
    void set_text_size(int s){_text_size=s;}
};

}   // namespace lfgui

#endif // LFGUI_LABEL_H
