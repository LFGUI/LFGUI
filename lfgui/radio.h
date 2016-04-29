#ifndef LFGUI_RADIO_H
#define LFGUI_RADIO_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>

#include "label.h"

namespace lfgui
{

/// \brief Simple push button with a text.
class radio : public label
{
    bool checked_;
public:
    image img_unchecked;
    image img_checked;

    radio(int x,int y,int width,int height=25,const std::string& text="",color text_color=color({0,0,0}),bool checked=false)
        : label(x,y,width,height),checked_(checked)
    {
        set_focusable(true);
        set_text_color(text_color);
        set_text(text);
        prepare_images();

        on_paint.functions.clear(); // remove the draw function from the label
        on_paint([this](lfgui::image& img)
        {
            img.draw_image(0,0,checked_?img_checked:img_unchecked);
            img.draw_text(this->height()+5,this->height()/2-_text_size/2,_text,_text_color,_text_size);
        });

        on_mouse_click([this]
        {
            checked_=!checked_;
        });
    }

    radio(int width=100,int height=20,const std::string& text="",color text_color=color({0,0,0}),bool checked=false)
        : radio(0,0,width,height,text,text_color,checked){}
    radio(const std::string& text,color text_color=color({0,0,0}),bool checked=false) : radio(0,0,100,100,text,text_color,checked){}

    void set_checked(bool checked)
    {
        checked_=checked;
        dirty=true;
    }

    bool is_checked()const{return checked_;}

private:
    void prepare_images()
    {
        img_unchecked=image("gui_torus.png").scaled(height(),height()).multiplied(text_color());
        img_checked=image("gui_torus_dot.png").scaled(height(),height()).multiplied(text_color());
    }
};

}   // namespace lfgui

#endif // LFGUI_BUTTON_H
