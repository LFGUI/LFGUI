#ifndef LFGUI_SLIDER_H
#define LFGUI_SLIDER_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>

#include "lfgui.h"

namespace lfgui
{

/// \brief A horizontal slider.
class slider : public widget
{
    float _value_min=0;
    float _value_max=1;
public:
    image img_background=image("gui_slider_background.png");
    image* img_handle=0;
    image* img_handle_old=0;
    image img_handle_normal=image("gui_ball.png");
    image img_handle_hover=image("gui_ball_dent_half.png");
    image img_handle_pressed=image("gui_ball_dent.png");
    widget* handle;
    signal<float> on_value_change;

    slider(int x,int y,int width,int height=20,float min=0,float max=1,float value=0)
        : widget(x,y,width,height),_value_min(min),_value_max(max),img_handle(&img_handle_normal)
    {
        // the drawing is currently a bit weird. The height is used weirdly.
        img_background.resize_linear(height,height);
        image temp(width,height);
        temp.clear();
        temp.draw_image(0,0,img_background.cropped(0,0,height/2,height));
        temp.draw_image(height/2+1,0,img_background.cropped(height/2,0,0,height).resize_linear(width-height-(height%2?0:1),height+1));
        temp.draw_image(width-height/2,0,img_background.cropped(height/2,0,height/2,height));
        img_handle_normal.resize_linear(height,height);
        img_handle_hover.resize_linear(height,height);
        img_handle_pressed.resize_linear(height,height);
        img_background=temp;

        handle=add_child(new widget(0,0,height,height));

        on_paint([this](lfgui::image& img)
        {
            img.draw_image(0,0,img_background);
        });

        handle->on_paint([this](lfgui::image& img)
        {
            img.draw_image(0,0,*img_handle);
        });

        handle->on_mouse_drag([this](event_mouse e)
        {
            img_handle=&img_handle_pressed;
            //handle->pos.x=from_global(pos.x);
            handle->translate(e.movement.x,0);
            handle->geometry.pos_absolute.x=std::max(handle->geometry.pos_absolute.x,0);
            handle->geometry.pos_absolute.x=std::min(handle->geometry.pos_absolute.x,this->width()-this->height());
            on_value_change.call(this->value());
        });

        handle->on_mouse_press([this]
        {
            _gui->set_cursor(mouse_cursor::hand_closed);
            img_handle=&img_handle_pressed;
        });

        handle->on_mouse_click_somewhere([this]
        {
            if(_gui->mouse_hovering_over(handle))
            {
                img_handle=&img_handle_hover;
                _gui->set_cursor(mouse_cursor::hand_open);
            }
            else
            {
                img_handle=&img_handle_normal;
                _gui->set_cursor(mouse_cursor::arrow);
            }
        });

        handle->on_mouse_enter([this]
        {
            if(_gui->held_widget()!=handle)       // don't change the displayed status if this widget is currently held down
            {
                img_handle=&img_handle_hover;
                _gui->set_cursor(mouse_cursor::hand_open);
            }
        });
        handle->on_mouse_leave([this]
        {
            if(_gui->held_widget()!=handle)       // don't change the displayed status if this widget is currently held down
            {
                img_handle=&img_handle_normal;
                _gui->set_cursor(mouse_cursor::arrow);
            }
        });

        set_value(value);
    }

    float value() const
    {
        return(handle->geometry.pos_absolute.x/float(width()-height()))*(value_max()-value_min())+value_min();
    }

    /// \brief Sets the current value to v. If emit_event is set on_value_change will be emitted.
    void set_value(float v,bool emit_event=true)
    {
        v=std::max(value_min(),v);
        v=std::min(value_max(),v);
        handle->geometry.pos_absolute.x=v/(value_max()-value_min())*(width()-height());
        if(emit_event)
            on_value_change.call(this->value());
    }

    float value_min()const{return _value_min;}
    float value_max()const{return _value_max;}
};

}   // namespace lfgui

#endif // LFGUI_SLIDER_H
