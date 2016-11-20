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
    int border_width=12;
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
    signal<> on_accept; ///< Called when the accept() function is used to close this window.
    signal<> on_reject; ///< Called when this widget is closed with the X or if the close() function is called otherwise.
    signal<> on_close;  ///< Called when this widget is closed.

    window(int x,int y,int width,int height,const std::string& title="",bool closable=false,bool resizeable=false);

    window(int width=100,int height=20,const std::string& text="")
        : window(0,0,width,height,text){}

    /// \brief Closes this window. Removes this window from the parent and destroys it. Calls on_accept and on_close.
    /// This should be called by an "Ok" or "Yes" button to provide a standardized dialog interface (like Qt).
    void accept()
    {
        on_accept.call();
        on_close.call();
        if(!parent)
            throw std::logic_error("LFGUI Error: close() called without having a parent.");
        parent->remove_child(this);
    }

    /// \brief Closes this window. Removes this window from the parent and destroys it. Calls on_reject and on_close.
    void close()
    {
        on_reject.call();
        on_close.call();
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
    void prepare_images();
};

}   // namespace lfgui

#endif // LFGUI_BUTTON_H
