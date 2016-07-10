#include "lfgui.h"
//#include "../external/cimg/CImg.h"

using namespace std;

namespace lfgui
{

widget::widget(int width,int height) : uid(generate_uid()),size_old(width,height),width_(width),height_(height)
{
    geometry.size_absolute.x=width;
    geometry.size_absolute.y=height;
    if(_gui==0)
        _gui=lfgui::gui::instance;
}

bool widget::_insert_event_mouse_press(const event_mouse& event)
{
    if(!rect().contains(event.pos))
        return false;

    // check if any children accepts this event
    for(auto&& it=children.rbegin();it!=children.rend();it++) // Reverse iteration to start with the topmost drawn one.
    {
        if((*it)->_insert_event_mouse_press(event.translated(-(*it)->geometry.calc_pos(width(),height()))))
            return true;
    }

    bool ret=false;

    // check this widget
    if(is_over(event.pos))
    {
        if(focusable())
        {
            _gui->set_focus(this);
            _gui->_held_widget=this;
            ret=true;
        }
        if(on_mouse_press)
        {
            dirty=true;
            ret=on_mouse_press.call(event);
        }
    }

    return ret;
}

bool widget::_insert_event_mouse_release(const event_mouse& event)
{
    bool ret=false;
    auto _gui=this->_gui;

    if(_gui->_held_widget&&_gui->_held_widget!=this)
    {
        ret=_gui->_held_widget->_insert_event_mouse_release(event.translated(to_global(point(0,0))).translated(_gui->_held_widget->to_local(point(0,0))));
        _gui->_held_widget=0;
    }
    else if(is_over(event.pos)) // check this widget
    {
        if(on_mouse_release)
        {
            dirty=true;
            ret=on_mouse_release.call(event);
        }
        ret=true;
    }

    if(_gui->_held_widget&&_gui->_held_widget==_gui->_hovering_over_widget)
    {
        _gui->_held_widget->dirty=true;
        _gui->_held_widget->on_mouse_click.call(event.translated(to_global(point(0,0))).translated(_gui->_held_widget->to_local(point(0,0))));
    }

    if(_gui->_held_widget)
    {
        _gui->_held_widget->dirty=true;
        _gui->_held_widget->on_mouse_click_somewhere.call(event.translated(to_global(point(0,0))).translated(_gui->_held_widget->to_local(point(0,0))));
    }

    return ret;
}

bool widget::_insert_event_mouse_move(const event_mouse& event)
{
    if(!rect().contains(event.pos))
        return false;

    if(_gui->_held_widget)
        if(_gui->_held_widget->on_mouse_drag)
        {
            _gui->_held_widget->dirty=true;
            _gui->_held_widget->on_mouse_drag.call(event.translated(to_global(point(0,0))).translated(_gui->_held_widget->to_local(point(0,0))));
            return true;
        }

    // Check if any child accepts this event.
    for(auto it=children.rbegin();it!=children.rend();it++) // Reverse iteration to start with the topmost drawn one.
    {
        if((*it)->_insert_event_mouse_move(event.translated(-(*it)->geometry.calc_pos(width(),height()))))
            return true;
    }

    // check this
    if(is_over(event.pos))
    {
        if(on_mouse_move)
        {
            dirty=true;
            on_mouse_move.call(event);
        }
        return true;
    }

    return false;
}

bool widget::_insert_event_mouse_wheel(const event_mouse& event)
{
    if(!rect().contains(event.pos))
        return false;

    // check if any children accepts this event
    for(auto it=children.rbegin();it!=children.rend();it++) // Reverse iteration to start with the topmost drawn one.
    {
        if((*it)->_insert_event_mouse_wheel(event.translated(-(*it)->geometry.calc_pos(width(),height()))))
            return true;
    }

    // check this
    if(is_over(event.pos))
    {
        if(on_mouse_wheel)
        {
            dirty=true;
            on_mouse_wheel.call(event);
        }
        return true;
    }
    return false;
}

void widget::_insert_event_key_press(const event_key& event)
{
    if(_gui->_focus_widget)
    {
        _gui->_focus_widget->dirty=true;
        _gui->_focus_widget->on_key_press.call(event);
    }
}

void widget::_insert_event_key_release(const event_key& event)
{
    if(_gui->_focus_widget)
    {
        _gui->_focus_widget->dirty=true;
        _gui->_focus_widget->on_key_release.call(event);
    }
}

void widget::resize(int width,int height)
{
    width_=width;
    height_=height;
    for(auto& e:children)
        e->update_geometry();
    dirty=true;
}

void widget::redraw(image& img,stk::memory_plain<uint16_t>& depth_buffer,uint16_t depth,int offset_x,int offset_y)
{
    /*if(dirty)
        goto redraw;
    for(auto& e:children)
        if(e->need_redraw())
            goto redraw;
    return;
redraw:*/
    if(!visible())
        return;

    if(size()!=size_old&&on_resize)
        on_resize.call(size());
    size_old=size();

    // draw this
    if(on_paint)
        on_paint.call(event_paint(img,depth_buffer,depth,offset_x,offset_y,*this));

    // draw children
    for(std::unique_ptr<widget>& e:children)
    {
        point p=e->geometry.calc_pos(width(),height())+point(offset_x,offset_y);
        e->redraw(img,depth_buffer,depth,p.x,p.y);
    }

    dirty=false;
    if(redraw_every_n_seconds)
        redraw_timer.reset();
}

widget* widget::_add_child(std::unique_ptr<widget>&& w)
{
    if(_gui)
        gui::instance=_gui;
    children.emplace_back(std::move(w));
    auto& ret=children.back();
    ret->parent=this;
    ret->_gui=_gui;
    dirty=true;
    return ret.get();
}

void widget::focus()
{
    _gui->set_focus(this);
}

bool widget::has_focus()const
{
    return _gui->_focus_widget==this;
}

widget::~widget()
{
    if(_gui->_held_widget==this)
        _gui->_held_widget=0;
    if(_gui->_focus_widget==this)
        _gui->_focus_widget=0;
    if(_gui->_hovering_over_widget==this)
        _gui->_hovering_over_widget=0;
    if(_gui->_hovering_over_widget_old==this)
        _gui->_hovering_over_widget_old=0;
}

void widget::raise() const
{
    if(!parent)
        return;
    auto it=parent->children.begin();
    for(;it->get()!=this;it++)
        if(it==parent->children.end())  // should never happen
            return;

    auto next=it;
    next++;

    for(;next!=parent->children.end();it++,next++)
        it->swap(*next);
}

bool widget::_check_mouse_hover(point p) const
{
    if(!rect().contains(p))
        return false;
    for(auto it=children.rbegin();it!=children.rend();it++) // Reverse iteration to start with the topmost drawn one.
        if((*it)->_check_mouse_hover(p-(*it)->geometry.calc_pos(width(),height())))
            return true;
    if(!is_over(p))
        return false;
    _gui->_hovering_over_widget=(widget*)this;
    return true;
}

void widget::set_hover_cursor(mouse_cursor c)
{
    on_mouse_enter([this,c]{_gui->set_cursor(c);});
    on_mouse_leave([this,c]{_gui->set_cursor(lfgui::mouse_cursor::arrow);});
}

// //////////////////////////////////// gui

void gui::insert_event_mouse_move(int mouse_x,int mouse_y)
{
    event_mouse em(mouse_old_pos,point(mouse_x,mouse_y),event_button_last,button_state_last);
    _insert_event_mouse_move(em);
    _check_mouse_hover(point(mouse_x,mouse_y));
    mouse_old_pos=point(mouse_x,mouse_y);
    if(_hovering_over_widget_old!=_hovering_over_widget)
    {
        if(_hovering_over_widget_old&&_hovering_over_widget_old->on_mouse_leave)
        {
            _hovering_over_widget_old->on_mouse_leave.call(em.translated(_hovering_over_widget_old->to_local(point(0,0))));
            _hovering_over_widget_old->dirty=true;
        }
        if(_hovering_over_widget&&_hovering_over_widget->on_mouse_enter)
        {
            _hovering_over_widget->on_mouse_enter.call(em.translated(_hovering_over_widget->to_local(point(0,0))));
            _hovering_over_widget->dirty=true;
        }
    }
    _hovering_over_widget_old=_hovering_over_widget;
}

void gui::set_focus(widget* w)
{
    if(_focus_widget==w)
        return;
    if(_focus_widget&&_focus_widget->on_focus_out)
    {
        _focus_widget->dirty=true;
        _focus_widget->on_focus_out.call();
    }
    _focus_widget=w;
    if(_focus_widget&&_focus_widget->on_focus_in)
    {
        _focus_widget->dirty=true;
        _focus_widget->on_focus_in.call();
    }
}

}

std::function<lfgui::image(std::string)> lfgui::image::load=[](std::string)
{
    throw std::logic_error("LFGUI Error: Image loading not supported. No wrapper set this std::function.");return lfgui::image();
};

lfgui::gui* lfgui::gui::instance=0;
