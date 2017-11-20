#ifndef LFGUI_CHECKBOX_H
#define LFGUI_CHECKBOX_H

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
class checkbox : public label
{
    bool checked_;
public:
    image img_unchecked;
    image img_checked;

    checkbox(int x,int y,int width,int height=25,const std::string& text="",color text_color=color({0,0,0}),bool checked=false)
        : label(x,y,width,height),checked_(checked)
    {
        set_focusable(true);
        set_text_color(text_color);
        set_text(text);
        prepare_images();

        on_paint.functions.clear(); // remove the draw function from the label
        on_paint([this](lfgui::event_paint e)
        {
            e.img.draw_image(e.offset_x,e.offset_y,checked_?img_checked:img_unchecked);
            e.img.draw_text(e.offset_x+this->height()+5,e.offset_y+this->height()/2-_text_size/2,_text,_text_color,_text_size);
        });

        on_mouse_click([this]
        {
            checked_=!checked_;
        });
    }

    checkbox(int width=100,int height=20,const std::string& text="",color text_color=color({0,0,0}),bool checked=false)
        : checkbox(0,0,width,height,text,text_color,checked){}
    checkbox(const std::string& text,color text_color=color({0,0,0}),bool checked=false) : checkbox(0,0,100,100,text,text_color,checked){}

    void set_checked(bool checked)
    {
        checked_=checked;
        dirty=true;
    }

    bool is_checked()const{return checked_;}

private:
    void prepare_images()
    {
        img_unchecked=image(ressource_path::get()+"gui_checkbox_unchecked.png").scaled(height(),height()).multiplied(text_color());
        img_checked  =image(ressource_path::get()+"gui_checkbox_checked.png").scaled(height(),height()).multiplied(text_color());
    }
};

}   // namespace lfgui

#endif // LFGUI_BUTTON_H
