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
        prepare_images();
        set_text_color(text_color);
        set_text(text);

        on_paint.functions.clear(); // remove the draw function from the label
        on_paint([this](lfgui::image& img)
        {
            img.draw_image(0,0,*img_ptr);
            img.draw_text(this->width()/2,this->height()/2-_text_size/2,_text,_text_color,_text_size,1,alignment::center);
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
    button(const std::string& text="",color text_color=color({0,0,0}),int border_width=10) : button(0,0,100,100,text,text_color,border_width){}

private:
    // called by the constructor to create the button images that are draw when drawing the widget
    // A rendered image of a 3D ball is used to draw a rectangular button with three states, rounded corners and borders.
    void prepare_images()
    {
        img_normal=image("gui_ball.png");
        img_hover=image("gui_ball_dent_half.png");
        img_pressed=image("gui_ball_dent.png");
        image temp(width(),height());

        // image for the button in normal state
        {
            temp.clear();
            auto w=img_normal.width();
            auto h=img_normal.height();

            // draw corners
            temp.draw_image(0,0,img_normal.cropped(0,0,w/2,h/2).scale(border_width,border_width));                                               // top left
            temp.draw_image(width()-border_width,0,img_normal.cropped(w/2,0,w/2,h/2).scale(border_width,border_width));                          // top right
            temp.draw_image(0,height()-border_width,img_normal.cropped(0,h/2,w/2,h/2).scale(border_width,border_width));                         // bottom left
            temp.draw_image(width()-border_width,height()-border_width,img_normal.cropped(w/2,h/2,w/2,h/2).scale(border_width,border_width));    // bottom right

            // draw borders
            temp.draw_image(border_width,0,img_normal.cropped(w/2,0,0,h/2).scale(width()-border_width*2,border_width));                          // top
            temp.draw_image(border_width,height()-border_width,img_normal.cropped(w/2,h/2,0,h/2).scale(width()-border_width*2,border_width));    // bottom
            temp.draw_image(0,border_width,img_normal.cropped(0,h/2,w/2,0).scale(border_width,height()-border_width*2));                         // left
            temp.draw_image(width()-border_width,border_width,img_normal.cropped(w/2,h/2,w/2,0).scale(border_width,height()-border_width*2));    // right

            // draw center
            temp.draw_image(border_width,border_width,img_normal.cropped(w/2,h/2,0,0).scale(width()-border_width*2,height()-border_width*2));

            img_normal=temp;
        }

        // image for hovered button
        {
            temp.clear();
            auto w=img_hover.width();
            auto h=img_hover.height();

            // draw corners
            temp.draw_image(0,0,img_hover.cropped(0,0,w/2,h/2).scale(border_width,border_width));                                                // top left
            temp.draw_image(width()-border_width,0,img_hover.cropped(w/2,0,w/2,h/2).scale(border_width,border_width));                           // top right
            temp.draw_image(0,height()-border_width,img_hover.cropped(0,h/2,w/2,h/2).scale(border_width,border_width));                          // bottom left
            temp.draw_image(width()-border_width,height()-border_width,img_hover.cropped(w/2,h/2,w/2,h/2).scale(border_width,border_width));     // bottom right

            // draw borders
            temp.draw_image(border_width,0,img_hover.cropped(w/2,0,0,h/2).scale(width()-border_width*2,border_width));                           // top
            temp.draw_image(border_width,height()-border_width,img_hover.cropped(w/2,h/2,0,h/2).scale(width()-border_width*2,border_width));     // bottom
            temp.draw_image(0,border_width,img_hover.cropped(0,h/2,w/2,0).scale(border_width,height()-border_width*2));                          // left
            temp.draw_image(width()-border_width,border_width,img_hover.cropped(w/2,h/2,w/2,0).scale(border_width,height()-border_width*2));     // right

            // draw center
            temp.draw_image(border_width,border_width,img_hover.cropped(w/2,h/2,0,0).scale(width()-border_width*2,height()-border_width*2));

            img_hover=temp;
        }

        // image for pressed button
        {
            temp.clear();
            auto w=img_pressed.width();
            auto h=img_pressed.height();

            // draw corners
            temp.draw_image(0,0,img_pressed.cropped(0,0,w/2,h/2).scale(border_width,border_width));                                              // top left
            temp.draw_image(width()-border_width,0,img_pressed.cropped(w/2,0,w/2,h/2).scale(border_width,border_width));                         // top right
            temp.draw_image(0,height()-border_width,img_pressed.cropped(0,h/2,w/2,h/2).scale(border_width,border_width));                        // bottom left
            temp.draw_image(width()-border_width,height()-border_width,img_pressed.cropped(w/2,h/2,w/2,h/2).scale(border_width,border_width));   // bottom right

            // draw borders
            temp.draw_image(border_width,0,img_pressed.cropped(w/2,0,0,h/2).scale(width()-border_width*2,border_width));                         // top
            temp.draw_image(border_width,height()-border_width,img_pressed.cropped(w/2,h/2,0,h/2).scale(width()-border_width*2,border_width));   // bottom
            temp.draw_image(0,border_width,img_pressed.cropped(0,h/2,w/2,0).scale(border_width,height()-border_width*2));                        // left
            temp.draw_image(width()-border_width,border_width,img_pressed.cropped(w/2,h/2,w/2,0).scale(border_width,height()-border_width*2));   // right

            // draw center
            temp.draw_image(border_width,border_width,img_pressed.cropped(w/2,h/2,0,0).scale(width()-border_width*2,height()-border_width*2));

            img_pressed=temp;
        }
    }
};

}   // namespace lfgui

#endif // LFGUI_BUTTON_H
