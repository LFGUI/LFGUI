#ifndef LFGUI_H
#define LFGUI_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <memory>
#include <algorithm>

#include "image.h"
#include "key.h"
#include "signal.h"
#include "../external/stk_timer.h"

namespace lfgui
{

union mouse_button
{
    uint32_t all;
    struct
    {
        int left:1;
        int right:1;
        int middle:1;
        // Qt defines up to 24 extra buttons in Qt::MouseButton
        int extra_1:1;int extra_2:1;int extra_3:1;int extra_4:1;
        int extra_5:1;int extra_6:1;int extra_7:1;int extra_8:1;int extra_9:1;
        int extra_10:1;int extra_11:1;int extra_12:1;int extra_13:1;int extra_14:1;
        int extra_15:1;int extra_16:1;int extra_17:1;int extra_18:1;int extra_19:1;
        int extra_20:1;int extra_21:1;int extra_22:1;int extra_23:1;int extra_24:1;
    };
};

/// \brief Represents a mouse event like a button press, mouse movement or mouse wheel move.
class event_mouse
{
public:
    point pos;      ///< Mouse cursor position in local coordinates of the widget which received this event
    point old_pos;  ///< The position in the last event
    point movement; ///< The difference between the current position and the last position, pos-old_pos
    point wheel_movement;   ///< Mouse wheel movement. Qt also supports horizontal movement, may not be available in all wrapper.
    mouse_button button;        ///< Has the button set that triggered this event.
    mouse_button button_state;  ///< Contains the status of all buttons.

    event_mouse(point old_pos=point(),point new_pos=point(),uint32_t event_button=0,
                uint32_t buttons_status=0,int delta_wheel_x=0,int delta_wheel_y=0)
        : pos(new_pos),old_pos(old_pos),movement(new_pos.x-old_pos.x,new_pos.y-old_pos.y),
          wheel_movement(delta_wheel_x,delta_wheel_y)
    {
        this->button.all=event_button;
        this->button_state.all=buttons_status;
    }

    /// \brief Return a translated (moved) version of this event. The given point is added to the coordinates (pos and old_pos).
    event_mouse translated(point p) const
    {
        event_mouse ret(*this);
        ret.pos.x+=p.x;
        ret.pos.y+=p.y;
        ret.old_pos.x+=p.x;
        ret.old_pos.y+=p.y;
        return ret;
    }
};

class event_key
{
public:
    int key;    /// \brief Identical to Qt::Key.
    std::string character_unicode;  /// \brief Contains the character entered in UTF-8.

    event_key(int key,const std::string& character_unicode) : key(key),character_unicode(character_unicode){}
};

class gui;

/// \brief Represents a widget.
class widget
{
protected:
    std::string uid;
    std::vector<std::unique_ptr<widget>> children;
    widget* parent=0;
    gui* _gui=0;
    point size_old;
public:
    /// \brief Determines if this widget and all its children are fully redrawn the next time redraw() gets called.
    bool dirty=true;
    /// \brief Can be set to a time amount in seconds to (at least) redraw this widget and all its children every N seconds.
    float redraw_every_n_seconds=0;
    /// \brief Used to measure the time since the last redraw.
    stk::timer redraw_timer=stk::timer("",false);
    widget_geometry geometry;   ///< The geometry used to position and size this widget.
    image img;  ///< The cIMG image that is used to display this widget.

    signal<event_mouse> on_mouse_press;             ///< called when a mouse button is pressed on this widget
    signal<event_mouse> on_mouse_release;           ///< called when a mouse button is release on this widget
    signal<event_mouse> on_mouse_click;             ///< called when a mouse button has been pressed on this widget and is now released on this widget
    signal<event_mouse> on_mouse_click_somewhere;    ///< called when a mouse button had been pressed on this widget and is now released on this widget or somewhere else
    signal<event_mouse> on_mouse_move;              ///< called when the mouse cursor is moving over this widget
    signal<event_mouse> on_mouse_drag;              ///< called when the mouse cursor has been pressed on this widget and is held while moving over this widget
    signal<event_mouse> on_mouse_enter;             ///< called when the mouse cursor moves from outside this widget onto this widget
    signal<event_mouse> on_mouse_leave;             ///< called when the mouse was over this widget and just left this widget
    signal<event_mouse> on_mouse_wheel;             ///< called when the mouse wheel is used while the cursor is over this widget
    signal<point> on_resize;                        ///< called when this widget changes its size. The parameter is the new size.
    signal<image&> on_paint;                        ///< called when this widget is being drawn.
    signal<event_key> on_key_press;                 ///< called on keyboard key press.
    signal<event_key> on_key_release;               ///< called on keyboard key press.
    signal<void> on_focus_in;                       ///< called when this widgets gets keyboard focus.
    signal<void> on_focus_out;                      ///< called when this widgets loses keyboard focus.

    virtual ~widget();

    /// \brief Inserts a mouse press event with local coordinates. Normally only called from the parent widget (or the
    ///  gui class if that is the parent). Calls on_mouse_press.
    bool _insert_event_mouse_press(const event_mouse&);
    /// \brief Inserts a mouse release event with local coordinates. Normally only called from the parent widget (or
    /// the gui class if that is the parent). Calls on_mouse_release and on_mouse_click.
    bool _insert_event_mouse_release(const event_mouse&);
    /// \brief Inserts a mouse move event with local coordinates. Normally only called from the parent widget (or the
    /// gui class if that is the parent). Calls on_mouse_move and on_mouse_drag.
    bool _insert_event_mouse_move(const event_mouse&);
    /// \brief Inserts a mouse wheel event with local coordinates. Normally only called from the parent widget (or the
    /// gui class if that is the parent). Calls on_mouse_wheel.
    bool _insert_event_mouse_wheel(const event_mouse&);
    /// \brief Inserts a key press event. Normally only called from the parent widget (or the
    /// gui class if that is the parent). Calls on_key_press.
    void _insert_event_key_press(const event_key&);
    /// \brief Inserts a key release event. Normally only called from the parent widget (or the
    /// gui class if that is the parent). Calls on_key_release.
    void _insert_event_key_release(const event_key&);

    /// \brief Call on_paint().
    virtual void redraw();

    /// \brief Returns true if the given point is over this widget. For detecting if a mouse click hit. Uses a
    /// bounding box check per default.
    virtual bool is_over(point p) const
    {
        return rect(0,0,width(),height()).contains(p);
    }

    int width()const{return img.width();}
    int height()const{return img.height();}
    point size()const{return point(width(),height());}
    /// \brief Resizes this widget to the given width and height. Calls on_resize();
    void resize(int width,int height);
    /// \brief Resizes this widget to the given size. Calls on_resize();
    void resize(point size){resize(size.x,size.y);}
    widget* set_pos(int x,int y,float x_percent=0,float y_percent=0){geometry.set_pos(x,y,x_percent,y_percent);return this;}
    widget* set_size(int x,int y,float x_percent=0,float y_percent=0){geometry.set_size(x,y,x_percent,y_percent);resize(geometry.calc_size(parent?parent->width():0,parent?parent->height():0));return this;}
    widget* set_offset(float x_percent,float y_percent){geometry.set_offset(x_percent,y_percent);return this;}
    bool need_redraw()
    {
        if(redraw_every_n_seconds!=0&&redraw_every_n_seconds<redraw_timer.until_now())
            dirty=true;
        if(dirty)
            return true;
        for(auto& e:children)
            if(e->need_redraw())
                return true;
        return false;
    }

    /// \brief Constructs and adds a child widget. Returns a pointer to the widget (uses a std::unique_ptr internally,
    /// no need for delete).
    ///
    /// Example:
    /// \code
    /// auto button=widget->add_child<lfgui::button>(50,155,100,25);    // add a button at 50,155 with the size 100,25
    /// auto slider=widget->add_child<lfgui::slider>(50,95,180,50);     // add a slider
    /// \endcode
    template<typename T,typename... Args>
    T* add_child(Args... args)
    {
        return (T*)_add_child(std::unique_ptr<widget>(new T(args...)));
    }

    /// \brief Removes the given child widget.
    void remove_child(widget* w)
    {
        for(size_t i=0;i<children.size();i++)
            if(children[i].get()==w)
            {
                children.erase(children.begin()+i);
                return;
            }
        dirty=true;
    }

    /// \brief Moves this widget.
    void translate(int x,int y){geometry.pos_absolute.x+=x;geometry.pos_absolute.y+=y;}
    /// \brief Moves this widget.
    void translate(point p){geometry.pos_absolute+=p;}

    /// \brief Transforms the given point with local coordinates of this widget into global coordinates (global as in
    /// relative to the gui class managing this widget).
    point to_global(point p) const
    {
        const widget* w=this;
        while(w)
        {
            if(w->parent)
                p+=w->geometry.calc_pos(w->parent->geometry.size_absolute.x,w->parent->geometry.size_absolute.y);
            else
                p+=w->geometry.calc_pos(0,0);
            w=w->parent;
        }
        return p;
    }

    /// \brief Transforms the given point with global coordinates into local coordinates of this widget (global as in
    /// relative to the gui class managing this widget).
    point to_local(point p) const
    {
        const widget* w=this;
        while(w)
        {
            if(w->parent)
                p-=w->geometry.calc_pos(w->parent->geometry.size_absolute.x,w->parent->geometry.size_absolute.y);
            else
                p-=w->geometry.calc_pos(0,0);
            w=w->parent;
        }
        return p;
    }

    /// \brief Moves this widget to the end of the parents child list. This means that it is drawn as the last (on top)
    /// and receives events first.
    void raise() const
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

    /// \brief Gives this widget keyboard focus.
    void focus();

    /// \brief Returns true if this widget has keyboard focus.
    bool has_focus()const;

protected:
    widget* _add_child(std::unique_ptr<widget>&& w);

    static std::string generate_uid()
    {
        std::string ret("lfgui_widget_");
        static int id=0;
        ret.append(std::to_string(id));
        id++;
        return ret;
    }

    friend class gui;

    widget(int width=1,int height=1);

    widget(int x,int y,int width,int height) : widget(width,height)
    {
        geometry.pos_absolute=point(x,y);
    }
};

/// \brief Used as a manager class and a LFGUI instance.
class gui : public widget
{
    friend class widget;
    widget* _held_widget=0;                 ///< \brief The widget currently held by the mouse.
    widget* _hovering_over_widget=0;        ///< \brief The widget currently under the mouse.
    widget* _hovering_over_widget_old=0;    ///< \brief The widget currently under the mouse during the last check.
    widget* _focus_widget=0;                ///< \brief The widget that has keyboard focus.
    static gui* instance;                   ///< \brief Used by the load functions.
public:
    point mouse_old_pos=0;  // for mouse movement
    uint32_t button_state_last=0;
    uint32_t event_button_last=0;

    gui(int width,int height) : widget(width,height)
    {
        _gui=this;
        gui::instance=this;
    }

    /// \brief Used by a wrapper to inject a mouse press event.
    void insert_event_mouse_press(int mouse_x,int mouse_y,uint32_t event_button,uint32_t button_state)
    {
        event_mouse em(mouse_old_pos,point(mouse_x,mouse_y),event_button,button_state);
        _insert_event_mouse_press(em);
        mouse_old_pos=point(mouse_x,mouse_y);
        button_state_last=button_state;
        event_button_last=event_button;
    }

    /// \brief Used by a wrapper to inject a mouse release event.
    void insert_event_mouse_release(int mouse_x,int mouse_y,uint32_t event_button,uint32_t button_state)
    {
        event_mouse em(mouse_old_pos,point(mouse_x,mouse_y),event_button,button_state);
        _insert_event_mouse_release(em);
        mouse_old_pos=point(mouse_x,mouse_y);
        button_state_last=button_state;
        event_button_last=event_button;
    }

    /// \brief Used by a wrapper to inject a mouse move event.
    void insert_event_mouse_move(int mouse_x,int mouse_y)
    {
        event_mouse em(mouse_old_pos,point(mouse_x,mouse_y),event_button_last,button_state_last);
        _insert_event_mouse_move(em);
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

    /// \brief Used by a wrapper to inject a mouse wheel event.
    void insert_event_mouse_wheel(int delta_wheel_x,int delta_wheel_y)
    {
        event_mouse em(mouse_old_pos,mouse_old_pos,event_button_last,button_state_last,delta_wheel_x,delta_wheel_y);
        _insert_event_mouse_wheel(em);
    }

    void insert_event_key_press(lfgui::key key,std::string character_unicode)
    {
        event_key ek(key,character_unicode);
        _insert_event_key_press(ek);
    }

    void insert_event_key_release(lfgui::key key,std::string character_unicode)
    {
        event_key ek(key,character_unicode);
        _insert_event_key_release(ek);
    }

    /// \brief Returns bool if the mouse is hovering over the given widget.
    bool mouse_hovering_over(const widget* w)const{return w==_hovering_over_widget;}
    /// \brief Returns the widget that is currently being held (down) by the mouse or 0 if none is held.
    widget* held_widget()const{return _held_widget;}

    /// \brief Gives the given widget keyboard focus and calls on_focus_out and on_focus_in. Does nothing if the given
    /// widget has already focus.
    void set_focus(widget* w)
    {
        if(_focus_widget==w)
            return;
        if(_focus_widget&&_focus_widget->on_focus_out)
        {
            _focus_widget->on_focus_out.call();
            _focus_widget->dirty=true;
        }
        _focus_widget=w;
        if(_focus_widget&&_focus_widget->on_focus_in)
        {
            _focus_widget->on_focus_in.call();
            _focus_widget->dirty=true;
        }
    }
};

}   // namespace lfgui

#endif // LFGUI_H
