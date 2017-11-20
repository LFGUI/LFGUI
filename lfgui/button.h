#ifndef LFGUI_BUTTON_H
#define LFGUI_BUTTON_H

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
class button : public label
{
    int border_width;
public:
    image* img_ptr=0;           ///< Points to the currently used image to draw the button. The button has three stages with three different images.
    image* img_ptr_old=0;
    image img_normal;
    image img_hover;
    image img_pressed;

    button(int x,int y,int width,int height=25,const std::string& text="",color text_color=color({0,0,0}),int border_width=10)
        : label(x,y,width,height),border_width(border_width),img_ptr(&img_normal)
    {
        set_focusable(true);
        prepare_images();
        set_text_color(text_color);
        set_text(text);

        on_paint.functions.clear(); // remove the draw function from the label
        on_paint([this](lfgui::event_paint e)
        {
            e.img.draw_image(e.offset_x,e.offset_y,*img_ptr);
            e.img.draw_text(e.offset_x+this->width()/2,e.offset_y+this->height()/2-_text_size/2,_text,_text_color,_text_size,alignment::center);
        });

        on_mouse_press([this]
        {
            img_ptr=&img_pressed;
        });
        on_mouse_click_somewhere([this]
        {
            if(_gui->mouse_hovering_over(this))
                img_ptr=&img_hover;
            else
                img_ptr=&img_normal;
        });
        on_mouse_enter([this]
        {
            if(_gui->held_widget()!=this)       // don't change the displayed status if this widget is currently held down
                img_ptr=&img_hover;
        });
        on_mouse_leave([this]
        {
            if(_gui->held_widget()!=this)       // don't change the displayed status if this widget is currently held down
                img_ptr=&img_normal;
        });
    }

    button(int width=100,int height=20,const std::string& text="",color text_color=color({0,0,0}),int border_width=10)
        : button(0,0,width,height,text,text_color,border_width){}
    button(const std::string& text,color text_color=color({0,0,0}),int border_width=10) : button(0,0,100,100,text,text_color,border_width){}

private:
    // called by the constructor to create the button images that are draw when drawing the widget
    // A rendered image of a 3D ball is used to draw a rectangular button with three states, rounded corners and borders.
    void prepare_images()
    {
        img_normal =image(ressource_path::get().append("gui_ball.png"));
        img_hover  =image(ressource_path::get().append("gui_ball_dent_half.png"));
        img_pressed=image(ressource_path::get().append("gui_ball_dent.png"));
        image temp(width(),height());

        // image for the button in normal state
        {
            temp.clear();
            temp.draw_image_corners_stretched(border_width,img_normal);
            img_normal=temp.copy();
        }

        // image for hovered button
        {
            temp.clear();
            temp.draw_image_corners_stretched(border_width,img_hover);
            img_hover=temp.copy();
        }

        // image for pressed button
        {
            temp.clear();
            temp.draw_image_corners_stretched(border_width,img_pressed);
            img_pressed=std::move(temp);
        }
    }
};

}   // namespace lfgui

#endif // LFGUI_BUTTON_H
