#ifndef LFGUI_SLIDER_H
#define LFGUI_SLIDER_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>
#include <algorithm>

#include "lfgui.h"

namespace lfgui
{

/// \brief A horizontal slider.
class slider : public widget
{
    float value_min_=0;
    float value_max_=1;
    int handle_size_;
    bool vertical_=false;
    float handle_thickness_=0;
public:
    image img_background=image(ressource_path+"gui_slider_background.png");
    image* img_handle=0;
    image* img_handle_old=0;
    image img_handle_normal=image(ressource_path+"gui_ball.png");
    image img_handle_hover=image(ressource_path+"gui_ball_dent_half.png");
    image img_handle_pressed=image(ressource_path+"gui_ball_dent.png");
    widget* handle;
    signal<float> on_value_change;

    /// \brief handle_thickness is an additional thickness of the handle in percent of the sliders size. This is often
    /// used to show how much is visible when used as a scrollbar.
    slider(int x,int y,int width,int height=20,float min_value=0,float max_value=1,float value=0,bool vertical=false,float handle_thickness=0);

    float value() const
    {
        if(vertical_)
            return(handle->geometry.pos_absolute.y/float(height()-width()))*(value_max()-value_min())+value_min();
        else
            return(handle->geometry.pos_absolute.x/float(width()-height()))*(value_max()-value_min())+value_min();
    }

    /// \brief Sets the current value to v. If emit_event is set on_value_change will be emitted.
    void set_value(float v,bool emit_event=true)
    {
        v=std::max(value_min(),v);
        v=std::min(value_max(),v);
        if(vertical_)
            handle->geometry.pos_absolute.y=v/(value_max()-value_min())*(height()-width());
        else
            handle->geometry.pos_absolute.x=v/(value_max()-value_min())*(width()-height());
        if(emit_event)
            on_value_change.call(this->value());
    }

    float value_min()const{return value_min_;}
    float value_max()const{return value_max_;}
};

}   // namespace lfgui

#endif // LFGUI_SLIDER_H
