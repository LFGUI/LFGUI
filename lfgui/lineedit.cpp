#include "lineedit.h"

namespace lfgui
{

lineedit::lineedit(int x,int y,int _width,int _height,const std::string& text,color text_color,int text_size)
        : widget(x,y,_width,_height),_text(text),_text_color(text_color),_text_size(text_size)
{
    STK_STACKTRACE
    cursor_position=_text.size();
    redraw_every_n_seconds=0.5;

    int border_width=8;

    {
        image img(ressource_path::get()+"gui_torus_filled.png");
        img_background=image(width(),height());
        img_background.clear();
        img_background.draw_image_corners_stretched(border_width,img);
    }
    {
        image img(ressource_path::get().append("gui_torus_filled_highlighted.png"));
        img_background_focused=image(width(),height());
        img_background_focused.clear();
        img_background_focused.draw_image_corners_stretched(border_width,img);
    }

    on_paint([this](lfgui::event_paint e)
    {
        if(has_focus())
            e.img.draw_image(e.offset_x,e.offset_y,img_background_focused);
        else
            e.img.draw_image(e.offset_x,e.offset_y,img_background);

        int space_for_n_characters=0;   // calculate how many characters we can display
        int available_space=width()-8;
        int needed_space=0;
        while(needed_space<available_space)
        {
            if(space_for_n_characters>(int)_text.size())
                break;
            needed_space=e.img.text_length(_text,_text_size,0,space_for_n_characters);
            space_for_n_characters++;
        }
        space_for_n_characters--;

        if(cursor_timer.until_now()>1.0)
            cursor_timer.reset();
        bool draw_cursor=has_focus()&&cursor_timer.until_now()<0.5;

        if(space_for_n_characters>=(int)_text.size()) // if enough space
        {
            space_for_n_characters=_text.size();
            e.img.draw_text(e.offset_x+4,e.offset_y+3,_text,_text_color,_text_size);

            if(!draw_cursor)
                return;

            int i=0;
            if(cursor_position>0)
                i=e.img.text_length(_text,_text_size,cursor_position);

            i+=3;
            e.img.draw_line(e.offset_x+i,e.offset_y+3,e.offset_x+i,e.offset_y+_text_size+3,_text_color);
        }
        else
        {
            if((int)cursor_position<space_for_n_characters/2)    // display first n characters
            {
                std::string displayed_text=_text.substr(0,space_for_n_characters);
                e.img.draw_text(e.offset_x+4,e.offset_y+4,displayed_text,_text_color,_text_size);

                if(!draw_cursor)
                    return;

                int i=0;
                if(cursor_position>0)
                    i=e.img.text_length(displayed_text,_text_size,cursor_position);

                i+=3;
                e.img.draw_line(e.offset_x+i,e.offset_y+3,e.offset_x+i,e.offset_y+_text_size+3,_text_color);
            }
            else if(cursor_position>=_text.size()-space_for_n_characters/2)    // display last n characters
            {
                std::string displayed_text=_text.substr(_text.size()-space_for_n_characters,space_for_n_characters);
                e.img.draw_text(e.offset_x+4,e.offset_y+4,displayed_text,_text_color,_text_size);

                if(!draw_cursor)
                    return;

                int i=0;
                if(cursor_position>0)
                    i=e.img.text_length(displayed_text,_text_size,cursor_position-(_text.size()-space_for_n_characters));

                i+=3;
                e.img.draw_line(e.offset_x+i,e.offset_y+3,e.offset_x+i,e.offset_y+_text_size+3,_text_color);
            }
            else                                                            // center on the cursor
            {
                std::string displayed_text=_text.substr(cursor_position-space_for_n_characters/2,space_for_n_characters);
                e.img.draw_text(e.offset_x+4,e.offset_y+4,displayed_text,_text_color,_text_size);

                if(!draw_cursor)
                    return;

                int i=0;
                if(cursor_position>0)
                    i=e.img.text_length(displayed_text,_text_size,space_for_n_characters/2);

                i+=3;
                e.img.draw_line(e.offset_x+i,e.offset_y+3,e.offset_x+i,e.offset_y+_text_size+3,_text_color);
            }
        }
    });
    on_key_press([this](lfgui::event_key ek)
    {
        cursor_timer.reset();
        if(ek.key==key::Key_Enter||ek.key==lfgui::key::Key_Return)
            return;

        if(ek.key==key::Key_Backspace)
        {
            if(!cursor_position)
                return;

            while((_text[cursor_position-1]&0xC0)==0x80)    // remove whole UTF-8 characters
            {
                _text.erase(cursor_position-1,1);
                cursor_position--;
            }
            _text.erase(cursor_position-1,1);
            cursor_position--;
        }
        else if(ek.key==key::Key_Delete)
            _text.erase(cursor_position,1);
        else if(ek.key==key::Key_Left)
        {
            while((_text[cursor_position-1]&0xC0)==0x80)    // wander whole UTF-8 characters
                if(cursor_position>0)
                    cursor_position--;
            if(cursor_position>0)
                cursor_position--;
        }
        else if(ek.key==key::Key_Right)
        {
            while((_text[cursor_position+1]&0xC0)==0x80)    // wander whole UTF-8 characters
                cursor_position++;
            cursor_position++;
        }
        else
        {
            _text.insert(cursor_position,ek.character_unicode);
            cursor_position+=ek.character_unicode.size();
        }

        cursor_position=std::min(cursor_position,_text.size());
    });

    on_focus_out([]{}); // widgets get redrawn when they have signals connected and that's all that should be done here (to remove the highlight effect and the cursor).
    set_hover_cursor(mouse_cursor::beam);
}

}
