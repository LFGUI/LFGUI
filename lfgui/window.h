#ifndef LFGUI_WINDOW_H
#define LFGUI_WINDOW_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>

#include "label.h"
#include "button.h"

namespace lfgui
{

/// \brief A window with a titlebar and a close button.
class window : public widget
{
    int border_width=14;
    color _title_color=color({0,0,0});
public:
    image img_normal;
    image img_highlighted;
    std::string title;
private:
    bool _closable;
    bool _resizeable;
public:
    widget* widget_content;
    widget* widget_this;

    window(int x,int y,int width,int height,const std::string& title="",bool closable=false,bool resizeable=false)
        : widget(x,y,width,height),title(title),_closable(closable),_resizeable(resizeable),widget_this(this)
    {
        prepare_images();
        set_size_min(100,100);

        on_paint([this](lfgui::image& img)
        {
            if(has_focus())
                img.draw_image(0,0,img_highlighted);
            else
                img.draw_image(0,0,img_normal);

            if(this->_closable)
                img.draw_text(img.width()/2-15,7,this->title,_title_color,18,alignment::center);
            else
                img.draw_text(img.width()/2,7,this->title,_title_color,18,alignment::center);
        });

        on_focus_out([]{}); // widgets get redrawn when they have signals connected and that's all that should be done here (to remove the highlight effect

        on_mouse_drag([this](lfgui::event_mouse e){translate(e.movement);});

        if(closable)
        {
            auto button=add_child(new lfgui::button(width-30,4,24,24,"X",lfgui::color({0,0,0}),6));
            button->img_normal.multiply({255,192,192});
            button->img_hover.multiply({255,192,192});
            button->img_pressed.multiply({255,192,192});
            button->on_mouse_click([this](lfgui::event_mouse,bool& b){b=true;close();});    // close the window and abort event handling
        }
        if(resizeable)
        {
            widget* w_resize_top=add_child(new widget())->set_pos(10,0)->set_size(-19,5,1,0);
            w_resize_top->on_mouse_press([this]{this->focus();});
            w_resize_top->on_mouse_drag([this](lfgui::event_mouse e){this->translate(0,e.movement.y);this->adjust_size(0,-e.movement.y);prepare_images();this->focus();});
            w_resize_top->set_hover_cursor(lfgui::mouse_cursor::size_vertical);

            widget* w_resize_right=add_child(new widget())->set_pos(-5,10,1,0)->set_size(5,-19,0,1);
            w_resize_right->on_mouse_press([this]{this->focus();});
            w_resize_right->on_mouse_drag([this](lfgui::event_mouse e){this->adjust_size(e.movement.x,0);prepare_images();this->focus();});
            w_resize_right->set_hover_cursor(lfgui::mouse_cursor::size_horizontal);

            widget* w_resize_bottom=add_child(new widget())->set_pos(10,-5,0,1)->set_size(-19,5,1,0);
            w_resize_bottom->on_mouse_press([this]{this->focus();});
            w_resize_bottom->on_mouse_drag([this](lfgui::event_mouse e){this->adjust_size(0,e.movement.y);prepare_images();this->focus();});
            w_resize_bottom->set_hover_cursor(lfgui::mouse_cursor::size_vertical);

            widget* w_resize_left=add_child(new widget())->set_pos(0,10)->set_size(5,-19,0,1);
            w_resize_left->on_mouse_press([this]{this->focus();});
            w_resize_left->on_mouse_drag([this](lfgui::event_mouse e){this->translate(e.movement.x,0);this->adjust_size(-e.movement.x,0);prepare_images();this->focus();});
            w_resize_left->set_hover_cursor(lfgui::mouse_cursor::size_horizontal);


            widget* w_resize_topleft=add_child(new widget())->set_pos(0,0)->set_size(10,10);
            w_resize_topleft->on_mouse_press([this]{this->focus();});
            w_resize_topleft->on_mouse_drag([this](lfgui::event_mouse e){this->translate(e.movement);this->adjust_size(-e.movement);prepare_images();this->focus();});
            w_resize_topleft->set_hover_cursor(lfgui::mouse_cursor::size_topleft_bottomright);

            widget* w_resize_bottomright=add_child(new widget())->set_pos(0,0,1,1)->set_size(10,10)->set_offset(-1,-1);
            w_resize_bottomright->on_mouse_press([this]{this->focus();});
            w_resize_bottomright->on_mouse_drag([this](lfgui::event_mouse e){this->adjust_size(e.movement);prepare_images();this->focus();});
            w_resize_bottomright->set_hover_cursor(lfgui::mouse_cursor::size_topleft_bottomright);

            widget* w_resize_topright=add_child(new widget())->set_pos(0,0,1,0)->set_size(10,10)->set_offset(-1,0);
            w_resize_topright->on_mouse_press([this]{this->focus();});
            w_resize_topright->on_mouse_drag([this](lfgui::event_mouse e){this->translate(0,e.movement.y);this->adjust_size(e.movement.x,-e.movement.y);prepare_images();this->focus();});
            w_resize_topright->set_hover_cursor(lfgui::mouse_cursor::size_topright_bottomleft);

            widget* w_resize_bottomleft=add_child(new widget())->set_pos(0,0,0,1)->set_size(10,10)->set_offset(0,-1);
            w_resize_bottomleft->on_mouse_press([this]{this->focus();});
            w_resize_bottomleft->on_mouse_drag([this](lfgui::event_mouse e){this->translate(e.movement.x,0);this->adjust_size(-e.movement.x,e.movement.y);prepare_images();this->focus();});
            w_resize_bottomleft->set_hover_cursor(lfgui::mouse_cursor::size_topright_bottomleft);
        }

        widget_content=add_child(new widget())->set_pos(6,30)->set_size(-13,-37,1,1);
        widget_content->set_focusable(false);
    }

    window(int width=100,int height=20,const std::string& text="")
        : window(0,0,width,height,text){}

    /// \brief Closes this window. Removes this window from the parent and destroys it.
    void close()
    {
        if(!parent)
            throw std::logic_error("LFGUI Error: close() called without having a parent.");
        parent->remove_child(this);
    }

    /// \brief Same as widget::create_child but adds directly to the widget_content which represents the
    /// content area of this window.
    template<typename T,typename... Args>
    T* create_child_in_content_widget(Args... args)
    {
        return widget_content->create_child<T>(args...);
    }

    /// \brief Same as widget::add_child but adds directly to the widget_content which represents the
    /// content area of this window.
    template<typename T>
    T* add_child_to_content_widget(T* w)
    {
        return widget_content->add_child<T>(w);
    }

private:
    // called by the constructor to create the button images that are draw when drawing the widget
    void prepare_images()
    {
        {
            image img("gui_window.png");
            image img_upper=img.cropped(0,0,img.width(),42);
            image img_lower=img.cropped(0,42,img.width(),90-42);

            image img_titlebar(width(),25);
            img_titlebar.clear();
            img_titlebar.draw_image_corners_stretched(border_width,img_upper);

            image img_content(width(),height()-25);
            img_content.clear();
            img_content.draw_image_corners_stretched(border_width,img_lower);

            img_normal=image(width(),height());
            img_normal.clear();
            img_normal.draw_image(0,0,img_titlebar);
            img_normal.draw_image(0,25,img_content);
        }
        {
            image img("gui_window_highlighted.png");
            image img_upper=img.cropped(0,0,img.width(),42);
            image img_lower=img.cropped(0,42,img.width(),90-42);

            image img_titlebar(width(),25);
            img_titlebar.clear();
            img_titlebar.draw_image_corners_stretched(border_width,img_upper);

            image img_content(width(),height()-25);
            img_content.clear();
            img_content.draw_image_corners_stretched(border_width,img_lower);

            img_highlighted=image(width(),height());
            img_highlighted.clear();
            img_highlighted.draw_image(0,0,img_titlebar);
            img_highlighted.draw_image(0,25,img_content);
        }
    }
};

}   // namespace lfgui

#endif // LFGUI_BUTTON_H
