
#include "lfgui/lfgui.h"
#include "lfgui/label.h"
#include "lfgui/button.h"
#include "lfgui/checkbox.h"
#include "lfgui/radio.h"
#include "lfgui/lineedit.h"
#include "lfgui/window.h"

#define PI		3.14159265358979323846

// Set up some sample widgets. This is done in all LFGUI samples (for every wrapper) so that all show the same GUI.
inline void setup_sample_gui(lfgui::widget* gui)
{
    gui->on_paint([&](lfgui::image& img)
    {
        {
            // draw a filled yellow star (with 10 points)
            float r=50;
            float r2=20;
            img.draw_polygon({{50+int(sin(0.0*PI)*r),100+int(-cos(0.0*PI)*r)},{50+int(sin(0.2*PI)*r2),100+int(-cos(0.2*PI)*r2)},
                              {50+int(sin(0.4*PI)*r),100+int(-cos(0.4*PI)*r)},{50+int(sin(0.6*PI)*r2),100+int(-cos(0.6*PI)*r2)},
                              {50+int(sin(0.8*PI)*r),100+int(-cos(0.8*PI)*r)},{50+int(sin(1.0*PI)*r2),100+int(-cos(1.0*PI)*r2)},
                              {50+int(sin(1.2*PI)*r),100+int(-cos(1.2*PI)*r)},{50+int(sin(1.4*PI)*r2),100+int(-cos(1.4*PI)*r2)},
                              {50+int(sin(1.6*PI)*r),100+int(-cos(1.6*PI)*r)},{50+int(sin(1.8*PI)*r2),100+int(-cos(1.8*PI)*r2)}},
                              {255,255,128});
        }
        {
            // draw a red star (only the outline)
            float r=50;
            float r2=20;
            img.draw_path({{150+int(sin(0.0*PI)*r),100+int(-cos(0.0*PI)*r)},{150+int(sin(0.2*PI)*r2),100+int(-cos(0.2*PI)*r2)},
                            {150+int(sin(0.4*PI)*r),100+int(-cos(0.4*PI)*r)},{150+int(sin(0.6*PI)*r2),100+int(-cos(0.6*PI)*r2)},
                            {150+int(sin(0.8*PI)*r),100+int(-cos(0.8*PI)*r)},{150+int(sin(1.0*PI)*r2),100+int(-cos(1.0*PI)*r2)},
                            {150+int(sin(1.2*PI)*r),100+int(-cos(1.2*PI)*r)},{150+int(sin(1.4*PI)*r2),100+int(-cos(1.4*PI)*r2)},
                            {150+int(sin(1.6*PI)*r),100+int(-cos(1.6*PI)*r)},{150+int(sin(1.8*PI)*r2),100+int(-cos(1.8*PI)*r2)}},
                            {255,64,64},true);
        }

        // draw some blue lines
        for(int i=10;i<=190;i+=20)
            img.draw_line(100,100,i,200,{64,64,128},(i+10)/20);

        // draw text centered at the bottom with different alignments
        img.draw_text(img.width()/2,img.height()-90,"alignment left",{55,55,0},16,lfgui::alignment::left);
        img.draw_text(img.width()/2,img.height()-70,"alignment centered",{55,55,0},16,lfgui::alignment::center);
        img.draw_text(img.width()/2,img.height()-50,"alignment right",{55,55,0},16,lfgui::alignment::right);
    });

    gui->on_resize([&](lfgui::point p)
    {
        std::cout<<"gui resized to "<<p.x<<":"<<p.y<<std::endl;
    });

    {
        auto mover=gui->add_child(new lfgui::widget(20,210,200,70));
        mover->on_paint=[&](lfgui::image& img)
        {
            img.draw_rect(0,0,img.width(),img.height(),{222,222,255,192});
            img.draw_text(10,3,"I move on left and right\nmouse clicks and\nmouse wheel movement",{64,0,0});
        };
        mover->on_mouse_press([mover](lfgui::event_mouse e)
        {
            if(e.button.left)
                mover->translate(10,0);
            else if(e.button.right)
                mover->translate(-10,0);
        });
        mover->on_mouse_wheel([mover](lfgui::event_mouse e)
        {
            mover->translate(-e.wheel_movement);
        });
    }
    {
        lfgui::window* movable=gui->add_child(new lfgui::window(280,10,240,285,"example window",false,true));

        {
            auto movable2=movable->add_child_to_content_widget(new lfgui::widget(15,5,150,20));
            movable2->on_paint([&](lfgui::image& img)
            {
                img.draw_rect(0,0,img.width(),img.height(),{64,64,64});
                img.draw_text(10,3,"I can be dragged",{255,0,255});
            });
            movable2->on_mouse_drag([movable2](lfgui::event_mouse e){movable2->translate(e.movement);});
        }
        movable->add_child_to_content_widget(new lfgui::label(10,35,150,20,"example slider",lfgui::color{128,22,92},20));
        {
            auto slider=movable->add_child_to_content_widget(new lfgui::slider(50,55,100,20));
            slider->on_value_change([](float v)
            {
                std::cout<<"slider 1: "<<v<<std::endl;
            });
        }
        {
            auto slider=movable->add_child_to_content_widget(new lfgui::slider(50,80,180,50));
            slider->on_value_change([](float v)
            {
                std::cout<<"slider 2: "<<v<<std::endl;
            });

            // show that signal handlers can stop events
            slider->on_value_change([](float)        {std::cout<<"### 1"<<std::endl;});
            slider->on_value_change([](float,bool& b){std::cout<<"### 2"<<std::endl;b=true;});  // set the bool& to true to stop the event at this handler
            slider->on_value_change([]               {std::cout<<"### 3"<<std::endl;});         // this handler is not called. Also the parameter can be ommited.
        }
        {
            auto button=movable->add_child_to_content_widget(new lfgui::button(50,125,100,25,"New window",lfgui::color({30,30,30})));
            button->on_mouse_click([gui]
            {
                static int count=1; // give the window unique titles (no technical reason, just to have different titles)
                lfgui::window* new_window=gui->add_child(new lfgui::window(rand()%(gui->width()-150),rand()%(gui->height()-100),170,100,"window #"+std::to_string(count),true));
                new_window->add_child_to_content_widget(new lfgui::label("This window is not\nresizeable but closable."))->set_size(0,0,1,1);
                new_window->on_close([]{std::cout<<"closed"<<std::endl;});
                new_window->on_reject([]{std::cout<<"rejected"<<std::endl;});
                count++;
            });
        }

        {
            movable->add_child_to_content_widget(new lfgui::checkbox( 20,155,100,16,"option 1",lfgui::color({30,30,30})));
            movable->add_child_to_content_widget(new lfgui::checkbox(120,155,100,16,"option 2",lfgui::color({130,20,20}),true));
        }

        {
            auto radio_1=movable->add_child_to_content_widget(new lfgui::radio( 20,180,100,16,"radio A 1",lfgui::color(30,30,30)));
            auto radio_2=movable->add_child_to_content_widget(new lfgui::radio( 20,200,100,16,"radio A 2",lfgui::color(30,30,30),true));
            auto radio_3=movable->add_child_to_content_widget(new lfgui::radio( 20,220,100,16,"radio A 3",lfgui::color(30,30,30)));
            radio_2->group_with(radio_1);
            radio_3->group_with(radio_1);
        }
        {
            auto radio_1=movable->add_child_to_content_widget(new lfgui::radio(120,180,100,16,"radio B 1",lfgui::color(0,80,130)));
            auto radio_2=movable->add_child_to_content_widget(new lfgui::radio(120,200,100,16,"radio B 2",lfgui::color(0,80,130)));
            auto radio_3=movable->add_child_to_content_widget(new lfgui::radio(120,220,100,16,"radio B 3",lfgui::color(0,80,130)));
            radio_2->group_with(radio_1);
            radio_3->group_with(radio_1);
            radio_3->set_checked();
        }
    }

    {
        static lfgui::color color_background({55,10,150,200});
        auto movable=gui->add_child(new lfgui::widget(30,320,520,190));
        movable->on_paint([&](lfgui::image& img)
        {
            img.draw_rect(0,0,img.width(),img.height(),color_background);
            img.draw_text(10,3,"I can be dragged vertically",{255,255,255});
        });
        movable->on_mouse_drag([movable](lfgui::event_mouse e){movable->translate(0,e.movement.y);});

        movable->add_child(new lfgui::label(10,30,200,20,"Change the color:",lfgui::color{255,222,192},20));

        lfgui::slider* slider_r=movable->add_child(new lfgui::slider(10,60,100,25,0,255,color_background.r));
        slider_r->on_value_change([&](float v){color_background.r=v;});
        slider_r->img_handle_normal.multiply(lfgui::color({255,128,128}));
        slider_r->img_handle_hover.multiply(lfgui::color({255,128,128}));
        slider_r->img_handle_pressed.multiply(lfgui::color({255,128,128}));
        slider_r->img_background.multiply(lfgui::color({255,128,128}));

        lfgui::slider* slider_g=movable->add_child(new lfgui::slider(10,90,100,25,0,255,color_background.g));
        slider_g->on_value_change([&](float v){color_background.g=v;});
        slider_g->img_handle_normal.multiply({128,255,128});
        slider_g->img_handle_hover.multiply({128,255,128});
        slider_g->img_handle_pressed.multiply({128,255,128});
        slider_g->img_background.multiply({128,255,128});

        lfgui::slider* slider_b=movable->add_child(new lfgui::slider(10,120,100,25,0,255,color_background.b));
        slider_b->on_value_change([&](float v){color_background.b=v;});
        slider_b->img_handle_normal.multiply({128,128,255});
        slider_b->img_handle_hover.multiply({128,128,255});
        slider_b->img_handle_pressed.multiply({128,128,255});
        slider_b->img_background.multiply({128,128,255});

        lfgui::slider* slider_a=movable->add_child(new lfgui::slider(10,150,100,25,0,255,color_background.a));
        slider_a->on_value_change([&](float v){color_background.a=v;});

        lfgui::widget* paint_area=movable->add_child(new lfgui::widget(210,10,300,170));
        std::cout<<paint_area->rect()<<std::endl;
        static lfgui::image painted_image(300,170);
        paint_area->on_paint([&](lfgui::image& img)
        {
            img.draw_rect(0,0,img.width(),img.height(),{255,255,255});
            img.draw_image(0,0,painted_image);
        });
        paint_area->on_mouse_drag([&](lfgui::event_mouse e)
        {
            painted_image.draw_line(e.old_pos,e.pos,lfgui::color(color_background.r,color_background.g,color_background.b),4,0.5);
        });
    }

    {
        gui->add_child(new lfgui::lineedit(550,120,200,20,"Edit this text!",lfgui::color{0,0,64}))->focus();
        gui->add_child(new lfgui::lineedit(550,150,240,20,"Edit this other text!",lfgui::color{64,0,0}));
    }

    {
        gui->add_child(new lfgui::button(50,500,150,30,"top left corner",lfgui::color({50,0,0})))->set_pos(0,0,0,0);
        gui->add_child(new lfgui::button(50,500,150,30,"top right corner",lfgui::color({50,0,0})))->set_pos(0,0,1,0)->set_offset(-1,0);
        gui->add_child(new lfgui::button(50,500,150,30,"bottom left corner",lfgui::color({50,0,0})))->set_pos(0,0,0,1)->set_offset(0,-1);
        gui->add_child(new lfgui::button(50,500,150,30,"bottom right corner",lfgui::color({50,0,0})))->set_pos(0,0,1,1)->set_offset(-1,-1);
/*
        gui->add_child(new lfgui::button(50,500,150,30,"top center",lfgui::color({0,50,0})))->set_pos(0,0,0.5,0)->set_offset(-0.5,0);
        gui->add_child(new lfgui::button(50,500,150,30,"right center",lfgui::color({0,50,0})))->set_pos(0,0,1,0.5)->set_offset(-1,-0.5);
        gui->add_child(new lfgui::button(50,500,150,30,"bottom center",lfgui::color({0,50,0})))->set_pos(0,0,0.5,1)->set_offset(-0.5,-1);
        gui->add_child(new lfgui::button(50,500,150,30,"left center",lfgui::color({0,50,0})))->set_pos(0,0,0,0.5)->set_offset(0,-0.5);

        gui->add_child(new lfgui::button(50,500,150,30,"center",lfgui::color({0,0,50})))->set_pos(0,0,0.5,0.5)->set_offset(-0.5,-0.5);
        */
    }
}
