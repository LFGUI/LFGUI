
#include "lfgui/lfgui.h"
#include "lfgui/label.h"
#include "lfgui/button.h"

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
            img.draw_line(100,100,i,200,{128,128,255});

        // draw text centered at the bottom with different alignments
        img.draw_text(img.width()/2,img.height()-90,"alignment left",{55,55,0},16,1,lfgui::alignment::left);
        img.draw_text(img.width()/2,img.height()-70,"alignment centered",{55,55,0},16,1,lfgui::alignment::center);
        img.draw_text(img.width()/2,img.height()-50,"alignment right",{55,55,0},16,1,lfgui::alignment::right);
    });

    gui->on_resize([&](lfgui::point p)
    {
        std::cout<<"gui resized to "<<p.x<<":"<<p.y<<std::endl;
    });

    {
        auto mover=gui->add_child<lfgui::widget>(20,210,200,70);
        mover->on_paint=[&](lfgui::image& img)
        {
            img.draw_rect(0,0,img.width(),img.height(),{192,192,255});
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
        auto movable=gui->add_child<lfgui::widget>(300,50,250,200);
        movable->on_paint=[&](lfgui::image& img)
        {
            img.draw_rect(0,0,img.width(),img.height(),{0,0,0});
            img.draw_text(10,3,"I can be dragged",{255,255,255});
        };
        movable->on_mouse_drag([movable](lfgui::event_mouse e){movable->translate(e.movement);});

        {
            auto movable2=movable->add_child<lfgui::widget>(50,20,180,20);
            movable2->on_paint([&](lfgui::image& img)
            {
                img.draw_rect(0,0,img.width(),img.height(),{64,64,64});
                img.draw_text(10,3,"I can be dragged too",{255,0,255});
            });
            movable2->on_mouse_drag([movable2](lfgui::event_mouse e){movable2->translate(e.movement);});
        }
        movable->add_child<lfgui::label>(10,50,150,20,"great stuff here:",lfgui::color{255,222,192},20);
        {
            auto slider=movable->add_child<lfgui::slider>(50,75,100,20);
            slider->on_value_change([](float v)
            {
                std::cout<<"slider 1: "<<v<<std::endl;
            });
        }
        {
            auto slider=movable->add_child<lfgui::slider>(50,95,180,50);
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
            auto button=movable->add_child<lfgui::button>(50,155,100,25,"CLICK ME",lfgui::color({30,30,30}));
            button->on_mouse_click([](lfgui::event_mouse e)
            {
                std::cout<<"button pressed"<<e.pos<<std::endl;
            });
        }
    }

    {
        static lfgui::color color_background({75,50,25});
        auto movable=gui->add_child<lfgui::widget>(300,320,220,160);
        movable->on_paint([&](lfgui::image& img)
        {
            img.draw_rect(0,0,img.width(),img.height(),color_background);
            img.draw_text(10,3,"I can be dragged vertically",{255,255,255});
        });
        movable->on_mouse_drag([movable](lfgui::event_mouse e){movable->translate(0,e.movement.y);});

        movable->add_child<lfgui::label>(10,30,200,20,"Change the color:",lfgui::color{255,222,192},20);

        lfgui::slider* slider_r=movable->add_child<lfgui::slider>(10,60,100,25,0,255,color_background.r);
        slider_r->on_value_change([&](float v){color_background.r=v;});
        slider_r->img_handle_normal.multiply(lfgui::color({255,128,128}));
        slider_r->img_handle_hover.multiply(lfgui::color({255,128,128}));
        slider_r->img_handle_pressed.multiply(lfgui::color({255,128,128}));
        slider_r->img_background.multiply(lfgui::color({255,128,128}));

        lfgui::slider* slider_g=movable->add_child<lfgui::slider>(10,90,100,25,0,255,color_background.g);
        slider_g->on_value_change([&](float v){color_background.g=v;});
        slider_g->img_handle_normal.multiply({128,255,128});
        slider_g->img_handle_hover.multiply({128,255,128});
        slider_g->img_handle_pressed.multiply({128,255,128});
        slider_g->img_background.multiply({128,255,128});

        lfgui::slider* slider_b=movable->add_child<lfgui::slider>(10,120,100,25,0,255,color_background.b);
        slider_b->on_value_change([&](float v){color_background.b=v;});
        slider_b->img_handle_normal.multiply({128,128,255});
        slider_b->img_handle_hover.multiply({128,128,255});
        slider_b->img_handle_pressed.multiply({128,128,255});
        slider_b->img_background.multiply({128,128,255});
    }

    {
        gui->add_child<lfgui::button>(50,500,150,30,"top left corner",lfgui::color({50,0,0}))->set_pos(0,0,0,0);
        gui->add_child<lfgui::button>(50,500,150,30,"top right corner",lfgui::color({50,0,0}))->set_pos(0,0,1,0)->set_offset(-1,0);
        gui->add_child<lfgui::button>(50,500,150,30,"bottom left corner",lfgui::color({50,0,0}))->set_pos(0,0,0,1)->set_offset(0,-1);
        gui->add_child<lfgui::button>(50,500,150,30,"bottom right corner",lfgui::color({50,0,0}))->set_pos(0,0,1,1)->set_offset(-1,-1);

        gui->add_child<lfgui::button>(50,500,150,30,"top center",lfgui::color({0,50,0}))->set_pos(0,0,0.5,0)->set_offset(-0.5,0);
        gui->add_child<lfgui::button>(50,500,150,30,"right center",lfgui::color({0,50,0}))->set_pos(0,0,1,0.5)->set_offset(-1,-0.5);
        gui->add_child<lfgui::button>(50,500,150,30,"bottom center",lfgui::color({0,50,0}))->set_pos(0,0,0.5,1)->set_offset(-0.5,-1);
        gui->add_child<lfgui::button>(50,500,150,30,"left center",lfgui::color({0,50,0}))->set_pos(0,0,0,0.5)->set_offset(0,-0.5);

        gui->add_child<lfgui::button>(50,500,150,30,"center",lfgui::color({0,0,50}))->set_pos(0,0,0.5,0.5)->set_offset(-0.5,-0.5);
    }
}
