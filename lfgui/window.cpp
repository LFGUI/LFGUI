#include "window.h"

namespace lfgui
{

window::window(int x,int y,int width,int height,const std::string& title,bool closable,bool resizeable)
    : widget(x,y,width,height),title(title),_closable(closable),_resizeable(resizeable),widget_this(this)
{
    prepare_images();
    set_size_min(100,100);

    on_paint([this](lfgui::event_paint e)
    {
        if(has_focus())
            e.img.draw_image(e.offset_x,e.offset_y,img_highlighted);
        else
            e.img.draw_image(e.offset_x,e.offset_y,img_normal);

        if(this->_closable)
            e.img.draw_text(e.offset_x+e.widget.width()/2-15,e.offset_y+7,this->title,_title_color,18,alignment::center);
        else
            e.img.draw_text(e.offset_x+e.widget.width()/2,e.offset_y+7,this->title,_title_color,18,alignment::center);
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

void window::prepare_images()
{
    {
        static image img(ressource_path+"gui_window.png");
        static image img_upper=img.cropped(0,0,img.width(),42);
        static image img_lower=img.cropped(0,42,img.width(),90-42);

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
        static image img(ressource_path+"gui_window_highlighted.png");
        static image img_upper=img.cropped(0,0,img.width(),42);
        static image img_lower=img.cropped(0,42,img.width(),90-42);

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

}
