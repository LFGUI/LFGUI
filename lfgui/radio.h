#ifndef LFGUI_RADIO_H
#define LFGUI_RADIO_H

#include <iostream>
#include <vector>
#include <map>
#include <set>
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
    std::shared_ptr<std::set<radio*>> group;

    radio(int x,int y,int width,int height=25,const std::string& text="",color text_color=color({0,0,0}),bool checked=false)
        : label(x,y,width,height),checked_(checked),group(new std::set<radio*>())
    {
        set_focusable(true);
        set_text_color(text_color);
        set_text(text);
        prepare_images();
        group->insert(this);

        on_paint.functions.clear(); // remove the draw function from the label
        on_paint([this](lfgui::event_paint e)
        {
            e.img.draw_image(e.depth_buffer,e.depth,e.offset_x,e.offset_y,checked_?img_checked:img_unchecked);
            e.img.draw_text(e.depth_buffer,e.depth,e.offset_x+this->height()+5,e.offset_y+this->height()/2-_text_size/2,_text,_text_color,_text_size);
        });

        on_mouse_click([this]
        {
            set_checked();
        });
    }

    radio(int width=100,int height=20,const std::string& text="",color text_color=color({0,0,0}),bool checked=false)
        : radio(0,0,width,height,text,text_color,checked){}
    radio(const std::string& text,color text_color=color({0,0,0}),bool checked=false)
        : radio(0,0,100,100,text,text_color,checked){}

    // would these be useful for anything? Disabled them due to the grouping feature.
    radio(const radio&)=delete;
    radio(radio&&)=delete;
    radio& operator=(const radio&)=delete;
    radio& operator=(radio&&)=delete;

    ~radio()
    {
        group->erase(group->find(this));
    }

    void set_checked()
    {
        checked_=true;
        dirty=true;
        for(auto&& e:*group)
            if(e!=this)
            {
                e->checked_=false;
                e->dirty=true;
            }
    }

    bool is_checked()const{return checked_;}

    void group_with(radio* r)
    {
        if(group)
            group->erase(group->find(this));
        group=r->group;

        bool one_checked=false;
        for(auto&& e:*group)
        {
            e->dirty=true;
            if(e->checked_)
            {
                one_checked=true;
                if(checked_)
                {
                    checked_=false;
                    break;
                }
            }
        }
        if(!one_checked)
            checked_=true;

        group->insert(this);
        dirty=true;
    }

private:
    void prepare_images()
    {
        img_unchecked=image("gui_torus.png").scaled(height(),height()).multiplied(text_color());
        img_checked=image("gui_torus_dot.png").scaled(height(),height()).multiplied(text_color());
    }
};

}   // namespace lfgui

#endif // LFGUI_BUTTON_H
