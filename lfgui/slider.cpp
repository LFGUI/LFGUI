#include "slider.h"

using namespace std;

namespace lfgui
{

slider::slider(int x,int y,int width,int height,float min_value,float max_value,float value,bool vertical,float handle_thickness)
        : widget(x,y,width,height),value_min_(min_value),value_max_(max_value),img_handle(&img_handle_normal),vertical_(vertical),handle_thickness_(handle_thickness)
{
    // the drawing is currently a bit weird. The height is used weirdly.
    int not_handle_size;
    if(vertical_)
    {
        handle_size_=width;
        not_handle_size=height;
    }
    else
    {
        handle_size_=height;
        not_handle_size=width;
    }

    img_background.resize_linear(handle_size_,handle_size_);
    image temp(not_handle_size,handle_size_);
    temp.clear();
    temp.draw_image(0,0,img_background.cropped(0,0,handle_size_/2,handle_size_));
    temp.draw_image(handle_size_/2+1,0,img_background.cropped(handle_size_/2,0,0,handle_size_).resize_linear(not_handle_size-handle_size_-(handle_size_%2?0:1),handle_size_+1));
    temp.draw_image(not_handle_size-handle_size_/2,0,img_background.cropped(handle_size_/2,0,handle_size_/2,handle_size_));
    img_handle_normal.resize_linear(handle_size_,handle_size_);
    img_handle_hover.resize_linear(handle_size_,handle_size_);
    img_handle_pressed.resize_linear(handle_size_,handle_size_);

    //temp.draw_image_corners_stretched(border_width,img_normal);

    img_background=temp;

    if(vertical_)
        img_background.rotate90();

    handle=add_child(new widget(0,0,handle_size_,handle_size_));

    on_paint([this](lfgui::event_paint e)
    {
        e.img.draw_image(e.depth_buffer,e.depth,e.offset_x,e.offset_y,img_background);
    });

    handle->on_paint([this](lfgui::event_paint e)
    {
        e.img.draw_image(e.depth_buffer,e.depth,e.offset_x,e.offset_y,*img_handle);
    });

    handle->on_mouse_drag([this](event_mouse e)
    {
        img_handle=&img_handle_pressed;
        //handle->pos.x=from_global(pos.x);
        if(vertical_)
        {
            handle->translate(0,e.movement.y);
            handle->geometry.pos_absolute.y=std::max(handle->geometry.pos_absolute.y,0);
            handle->geometry.pos_absolute.y=std::min(handle->geometry.pos_absolute.y,this->height()-this->width());
        }
        else
        {
            handle->translate(e.movement.x,0);
            handle->geometry.pos_absolute.x=std::max(handle->geometry.pos_absolute.x,0);
            handle->geometry.pos_absolute.x=std::min(handle->geometry.pos_absolute.x,this->width()-this->height());
        }
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

}
