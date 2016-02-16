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

namespace lfgui
{

/// \brief Represents a mouse event like a button press, mouse movement or mouse wheel move.
class event_mouse
{
public:
    point pos;      ///< Mouse cursor position in local coordinates of the widget which received this event
    point old_pos;  ///< The position in the last event
    point movement; ///< The difference between the current position and the last position, pos-old_pos
    point wheel_movement;   ///< Mouse wheel movement. Qt also supports horizontal movement, may not be available in all wrapper.

    /// Has the button set that triggered this event.
    union
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
    } button;

    /// Contains the status of all buttons.
    union
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
    } button_state;

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

/// \brief Used by signal to store std::functions.
template <typename T>
struct event_function
{
    std::function<void()> void_void;
    std::function<void(T)> void_em;
    std::function<void(T,bool&)> bool_em;

    event_function(){}
    event_function(std::function<void()> f) : void_void(f) {}
    event_function(std::function<void(T)> f) : void_em(f) {}
    event_function(std::function<void(T,bool&)> f) : bool_em(f) {}
    /// \brief Returns true if this even_function has a std::function set.
    operator bool()const{return void_void||void_em||bool_em;}
};

/// \brief This class is the signal slot system of LFGUI. Functions can be set to be called by using the = or () operator.
/// The functions are called by using the call() command. Functions can have an optional argument that is given by the
/// T template parameter. Functions have a priority, lower priority numbers are called first. (default priority is 0)
template <typename T>
class signal
{
public:
    std::multimap<int,event_function<T>> functions;

    signal(){}
    signal(const event_function<T>& f)
    {
        functions.emplace(0,f);
    }
    signal(int priority,const event_function<T>& f)
    {
        functions.emplace(priority,f);
    }
    template<typename T2>
    signal(const T2& f)
    {
        functions.emplace(0,event_function<T>(f));
    }
    template<typename T2>
    signal(int priority,const T2& f)
    {
        functions.emplace(priority,event_function<T>(f));
    }

    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(const event_function<T>& f){functions.emplace(0,f);}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(int priority,const event_function<T>& f){functions.emplace(priority,f);}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(std::function<void()> f){functions.emplace(0,event_function<T>(f));}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(std::function<void(T)> f){functions.emplace(0,event_function<T>(f));}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(std::function<void(T,bool&)> f){functions.emplace(0,event_function<T>(f));}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(int priority,std::function<void()> f){functions.emplace(priority,event_function<T>(f));}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(int priority,std::function<void(T)> f){functions.emplace(priority,event_function<T>(f));}
    /// \brief Adds a callback that is called when this signal is activated via call().
    void operator()(int priority,std::function<void(T,bool&)> f){functions.emplace(priority,event_function<T>(f));}
    template <typename T2>
    void operator=(const T2& o){(*this)(o);}

    /// \brief Returns true if the signal has any callbacks set.
    operator bool() const
    {
        return !functions.empty();
    }

    /// \brief Calls each appended function/lambda.
    bool call(const T& me)
    {
        for(auto& e:functions)
        {
            event_function<T>& s=e.second;
            bool stop;
            if(s.bool_em)
                s.bool_em(me,stop);
            else if(s.void_em)
                s.void_em(me);
            else if(s.void_void)
                s.void_void();
            if(stop)
                return true;
        }
        return !functions.empty();
    }
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
    point pos;
    image img;

    signal<event_mouse> on_mouse_press;             ///< called when a mouse button is pressed on this widget
    signal<event_mouse> on_mouse_release;           ///< called when a mouse button is release on this widget
    signal<event_mouse> on_mouse_click;             ///< called when a mouse button has been pressed on this widget and is now released on this widget
    signal<event_mouse> on_mouse_click_somewhere;    ///< called when a mouse button had been pressed on this widget and is now released on this widget or somewhere else
    signal<event_mouse> on_mouse_move;              ///< called when the mouse cursor is moving over this widget
    signal<event_mouse> on_mouse_drag;              ///< called when the mouse cursor has been pressed on this widget and is held while moving over this widget
    signal<event_mouse> on_mouse_enter;             ///< called when the mouse cursor moves from outside this widget onto this widget
    signal<event_mouse> on_mouse_leave;             ///< called when the mouse was over this widget and just left this widget
    signal<event_mouse> on_mouse_wheel;             ///< called when the mouse wheel is used while the cursor is over this widget
    signal<point> on_resize;                        ///< called when this widget changes its size
    signal<image&> on_paint;                        ///< called when this widget is being painted

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

    /// \brief Call on_paint().
    virtual void redraw();

    /// \brief Returns true if the given point is over this widget. For detecting if a mouse click hit. Uses a
    /// bounding box check per default.
    virtual bool is_over(point p) const
    {
        return rect(0,0,width(),height()).contains(p);
    }

    int x()const{return pos.x;}
    int y()const{return pos.y;}
    int width()const{return img.width();}
    int height()const{return img.height();}
    point size()const{return point(width(),height());}
    /// \brief Resizes this widget to the given width and height. Calls on_resize();
    void resize(int width,int height);

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

    /// \brief Moves this widget.
    void translate(int x,int y){pos.x+=x;pos.y+=y;}
    /// \brief Moves this widget.
    void translate(point p){pos+=p;}

    /// \brief Transforms the given point with local coordinates of this widget into global coordinates (global as in
    /// relative to the gui class managing this widget).
    point to_global(point p) const
    {
        const widget* w=this;
        while(w)
        {
            p+=w->pos;
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
            p-=w->pos;
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
        pos=point(x,y);
    }
};

/// \brief Used as a manager class and a LFGUI instance.
class gui : public widget
{
    friend class widget;
    widget* _held_widget=0;                 /// \brief The widget currently held by the mouse.
    widget* _hovering_over_widget=0;        /// \brief The widget currently under the mouse.
    widget* _hovering_over_widget_old=0;    /// \brief The widget currently under the mouse during the last check.
    static gui* instance;                   /// \brief Used by the load functions.
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
                _hovering_over_widget_old->on_mouse_leave.call(em.translated(_hovering_over_widget_old->to_local(point(0,0))));
            if(_hovering_over_widget&&_hovering_over_widget->on_mouse_enter)
                _hovering_over_widget->on_mouse_enter.call(em.translated(_hovering_over_widget->to_local(point(0,0))));
        }
        _hovering_over_widget_old=_hovering_over_widget;
    }

    /// \brief Used by a wrapper to inject a mouse wheel event.
    void insert_event_mouse_wheel(int delta_wheel_x,int delta_wheel_y)
    {
        event_mouse em(mouse_old_pos,mouse_old_pos,event_button_last,button_state_last,delta_wheel_x,delta_wheel_y);
        _insert_event_mouse_wheel(em);
    }

    /// \brief Returns bool if the mouse is hovering over the given widget.
    bool mouse_hovering_over(const widget* w)const{return w==_hovering_over_widget;}
    /// \brief Returns the widget that is currently being held (down) by the mouse or 0 if none is held.
    widget* held_widget()const{return _held_widget;}
};

}   // namespace lfgui

#endif // LFGUI_H
