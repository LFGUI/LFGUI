#include "lfgui.h"
//#include "../external/cimg/CImg.h"

using namespace std;

namespace lfgui
{

widget::widget(int width,int height) : uid(generate_uid()),size_old(width,height),img(width,height)
{
    geometry.size_absolute.x=width;
    geometry.size_absolute.y=height;
    if(_gui==0)
        _gui=lfgui::gui::instance;
}

bool widget::_insert_event_mouse_press(const event_mouse& event)
{
    // check if any children accepts this event
    for(auto it=children.rbegin();it!=children.rend();it++) // Reverse iteration to start to start with the topmost drawn one.
    {
        if((*it)->_insert_event_mouse_press(event.translated(-(*it)->geometry.calc_pos(width(),height()))))
            return true;
    }

    bool ret=true;

    // check this widget
    if(is_over(event.pos))
    {
        _gui->_focus_widget=this;
        if(on_mouse_press)
            ret=on_mouse_press.call(event);
        _gui->_held_widget=this;
    }
    else
        ret=false;

    return ret;
}

bool widget::_insert_event_mouse_release(const event_mouse& event)
{
    bool ret=false;

    if(_gui->_held_widget&&_gui->_held_widget!=this)
    {
        ret=_gui->_held_widget->_insert_event_mouse_release(event.translated(to_global(point(0,0))).translated(_gui->_held_widget->to_local(point(0,0))));
        _gui->_held_widget=0;
    }
    else if(is_over(event.pos)) // check this widget
    {
        if(on_mouse_release)
            ret=on_mouse_release.call(event);
        ret=true;
    }

    if(_gui->_held_widget&&_gui->_held_widget==_gui->_hovering_over_widget)
        _gui->_held_widget->on_mouse_click.call(event.translated(to_global(point(0,0))).translated(_gui->_held_widget->to_local(point(0,0))));

    if(_gui->_held_widget)
        _gui->_held_widget->on_mouse_click_somewhere.call(event.translated(to_global(point(0,0))).translated(_gui->_held_widget->to_local(point(0,0))));

    return ret;
}

bool widget::_insert_event_mouse_move(const event_mouse& event)
{

    if(_gui->_held_widget)
        if(_gui->_held_widget->on_mouse_drag)
        {
            _gui->_held_widget->on_mouse_drag.call(event.translated(to_global(point(0,0))).translated(_gui->_held_widget->to_local(point(0,0))));
            return true;
        }

    // Check if any children accepts this event.
    for(auto it=children.rbegin();it!=children.rend();it++) // Reverse iteration to start to start with the topmost drawn one.
    {
        if((*it)->_insert_event_mouse_move(event.translated(-(*it)->geometry.calc_pos(width(),height()))))
            return true;
    }

    // check this
    if(is_over(event.pos))
    {
        _gui->_hovering_over_widget=this;
        if(on_mouse_move)
            on_mouse_move.call(event);
        return true;
    }

    return false;
}

bool widget::_insert_event_mouse_wheel(const event_mouse& event)
{
    // check if any children accepts this event
    for(auto it=children.rbegin();it!=children.rend();it++) // Reverse iteration to start to start with the topmost drawn one.
    {
        if((*it)->_insert_event_mouse_wheel(event.translated(-(*it)->geometry.calc_pos(width(),height()))))
            return true;
    }

    // check this
    if(is_over(event.pos))
    {
        if(on_mouse_wheel)
            on_mouse_wheel.call(event);
        return true;
    }
    return false;
}

void widget::_insert_event_key_press(const event_key& event)
{
    if(_gui->_focus_widget)
        _gui->_focus_widget->on_key_press.call(event);
}

void widget::_insert_event_key_release(const event_key& event)
{
    if(_gui->_focus_widget)
        _gui->_focus_widget->on_key_release.call(event);
}

void widget::resize(int width,int height)
{
    img=img.resize_nearest(width,height);   // is supposed to be redrawn anyway
}

void widget::redraw()
{
    // clear image before drawing anything
    img.clear();

    if(size()!=size_old&&on_resize)
        on_resize.call(size());
    size_old=size();

    // draw this
    if(on_paint)
        on_paint.call(img);

    // draw children
    for(auto& e:children)
    {
        e->redraw();
        img.draw_image(e->geometry.calc_pos(width(),height()),e->img);
    }
}

widget* widget::_add_child(std::unique_ptr<widget>&& w)
{
    if(_gui)
        gui::instance=_gui;
    children.emplace_back(std::move(w));
    auto& ret=children.back();
    ret->parent=this;
    ret->_gui=_gui;
    return ret.get();
}

void widget::focus()
{
    _gui->_focus_widget=this;
}

bool widget::has_focus()const
{
    return _gui->_focus_widget==this;
}

}

std::function<lfgui::image(std::string)> lfgui::image::load=[](std::string){throw std::logic_error("LFGUI Error: Image loading not supported. No wrapper set this std::function.");return lfgui::image();};
lfgui::gui* lfgui::gui::instance=0;
