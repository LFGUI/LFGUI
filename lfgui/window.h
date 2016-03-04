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
    bool closable;

    window(int x,int y,int width,int height,const std::string& title="",bool closable=false)
        : widget(x,y,width,height),title(title),closable(closable)
    {
        prepare_images();

        on_paint([this](lfgui::image& img)
        {
            if(has_focus())
                img.draw_image(0,0,img_highlighted);
            else
                img.draw_image(0,0,img_normal);

            if(this->closable)
                img.draw_text(img.width()/2-15,7,this->title,_title_color,18,alignment::center);
            else
                img.draw_text(img.width()/2,7,this->title,_title_color,18,alignment::center);
        });

        on_focus_out([]{}); // widgets get redrawn when they have signals connected and that's all that should be done here (to remove the highlight effect

        on_mouse_drag([this](lfgui::event_mouse e){translate(e.movement);});

        if(closable)
        {
            auto button=add_child<lfgui::button>(width-30,4,24,24,"X",lfgui::color({0,0,0}),6);
            button->img_normal.multiply({255,192,192});
            button->img_hover.multiply({255,192,192});
            button->img_pressed.multiply({255,192,192});
            button->on_mouse_click([this](lfgui::event_mouse,bool& b){b=true;close();});    // close the window and abort event handling
        }
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
