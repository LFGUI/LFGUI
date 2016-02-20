#ifndef LFGUI_WRAPPER_URHO3D
#define LFGUI_WRAPPER_URHO3D

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Graphics/Texture.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "lfgui.h"

namespace lfgui
{
namespace wrapper_urho3d
{

inline lfgui::image load_image(std::string path);

/// \brief The LFGUI wrapper for Urho3D (http://urho3d.org/ or http://urho3d.github.io/). It will automatically
/// subscribe to all events it needs and display itself on the screen.
///
/// Example usage:
/// \code
/// class SampleApplication : public Application
/// {
/// public:
///     std::unique_ptr<lfgui::wrapper_urho3d::gui> gui;
///     ...
///
///     virtual void Start()
///     {
///         // create our LFGUI instance and set up the example widgets
///         gui.reset(new lfgui::wrapper_urho3d::gui(GetContext()));
///         setup_sample_gui(gui.get());
///         ...
/// \endcode
class gui : public lfgui::gui,public Urho3D::Object
{
    Urho3D::SharedPtr<Urho3D::Sprite> _sprite;
    Urho3D::SharedPtr<Urho3D::Image> _image;
    Urho3D::SharedPtr<Urho3D::Texture2D> _texture;
    static gui*& _instance(){static gui* inst;return inst;}
public:
    Urho3D::Context* _context;

    gui(Urho3D::Context* context) : lfgui::gui(10,10),_context(context),Urho3D::Object(context)
    {
        Urho3D::IntVector2 render_size=GetSubsystem<Urho3D::Graphics>()->GetRenderTargetDimensions();
        lfgui::gui::resize(render_size.x_,render_size.y_);

        _instance()=this;
        lfgui::image::load=lfgui::wrapper_urho3d::load_image;

        Urho3D::ResourceCache* cache=_context->GetSubsystem<Urho3D::ResourceCache>();
        _image=new Urho3D::Image(_context);
        _sprite=_context->GetSubsystem<Urho3D::UI>()->GetRoot()->CreateChild<Urho3D::Sprite>();
        _texture=new Urho3D::Texture2D(_context);
        _texture->SetFilterMode(Urho3D::TextureFilterMode::FILTER_NEAREST);
        _texture->SetNumLevels(1);
        _sprite->SetTexture(_texture);
        _sprite->SetBlendMode(Urho3D::BlendMode::BLEND_ALPHA);

        SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN,URHO3D_HANDLER(gui,e_mouse_press));
        SubscribeToEvent(Urho3D::E_MOUSEBUTTONUP,URHO3D_HANDLER(gui,e_mouse_release));
        SubscribeToEvent(Urho3D::E_MOUSEMOVE,URHO3D_HANDLER(gui,e_mouse_move));
        SubscribeToEvent(Urho3D::E_MOUSEWHEEL,URHO3D_HANDLER(gui,e_mouse_wheel));
        SubscribeToEvent(Urho3D::E_UPDATE,URHO3D_HANDLER(gui,e_update));
        SubscribeToEvent(Urho3D::E_SCREENMODE,URHO3D_HANDLER(gui,e_resize));
    }

    static gui* instance(){return _instance();}
    int width()const{return lfgui::widget::width();}
    int height()const{return lfgui::widget::height();}

    void redraw() override
    {
        lfgui::widget::redraw();

        if(_texture->GetWidth()!=width()||_texture->GetHeight()!=height())
            _texture->SetSize(width(),height(),Urho3D::Graphics::GetRGBAFormat(),Urho3D::TEXTURE_STATIC);
        if(_image->GetWidth()!=width()||_image->GetHeight()!=height())
            _image->SetSize(width(),height(),4);

        int h=height();
        int w=width();
        int count=w*h;
        int countx2=count*2;
        int countx3=count*3;
        // due to the channel seperation in the first image, the pixel have to be copyed byte/color vise
        uint8_t* data_target=_image->GetData();
        uint8_t* data_source=this->img.data();
        uint8_t* data_source_end=this->img.data()+count;
        for(;data_source<data_source_end;)
        {
            *data_target=*(data_source+countx2);    // cIMG has the colors as BGRA and Urho3D as RGBA
            data_target++;
            *data_target=*(data_source+count);
            data_target++;
            *data_target=*data_source;
            data_target++;
            *data_target=*(data_source+countx3);
            data_target++;
            data_source++;
        }

        _texture->SetData(0,0,0,w,h,_image->GetData());

        if(_sprite->GetWidth()!=_texture->GetWidth()||_sprite->GetHeight()!=_texture->GetHeight())
            _sprite->SetSize(_texture->GetWidth(),_texture->GetHeight());
    }

    void e_update(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData)
    {
        redraw();
    }

    void e_mouse_press(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData)
    {
        lfgui::event_mouse em;
        Urho3D::Input* input=_context->GetSubsystem<Urho3D::Input>();
        Urho3D::IntVector2 p=input->GetMousePosition();

        unsigned button=eventData[Urho3D::MouseButtonDown::P_BUTTON].GetInt();
        if(button==Urho3D::MOUSEB_LEFT)
            em.button.left=1;
        else if(button==Urho3D::MOUSEB_RIGHT)
            em.button.right=1;
        else if(button==Urho3D::MOUSEB_MIDDLE)
            em.button.middle=1;
        else if(button==Urho3D::MOUSEB_X1)
            em.button.extra_1=1;
        else if(button==Urho3D::MOUSEB_X2)
            em.button.extra_2=1;

        em.button_state.left=input->GetMouseButtonDown(Urho3D::MOUSEB_LEFT);
        em.button_state.right=input->GetMouseButtonDown(Urho3D::MOUSEB_RIGHT);
        em.button_state.middle=input->GetMouseButtonDown(Urho3D::MOUSEB_MIDDLE);
        em.button_state.extra_1=input->GetMouseButtonDown(Urho3D::MOUSEB_X1);
        em.button_state.extra_2=input->GetMouseButtonDown(Urho3D::MOUSEB_X2);

        insert_event_mouse_press(p.x_,p.y_,em.button.all,em.button_state.all);
    }

    void e_mouse_release(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData)
    {
        lfgui::event_mouse em;
        Urho3D::Input* input=_context->GetSubsystem<Urho3D::Input>();
        Urho3D::IntVector2 p=input->GetMousePosition();

        unsigned button=eventData[Urho3D::MouseButtonUp::P_BUTTON].GetInt();
        if(button==Urho3D::MOUSEB_LEFT)
            em.button.left=1;
        else if(button==Urho3D::MOUSEB_RIGHT)
            em.button.right=1;
        else if(button==Urho3D::MOUSEB_MIDDLE)
            em.button.middle=1;
        else if(button==Urho3D::MOUSEB_X1)
            em.button.extra_1=1;
        else if(button==Urho3D::MOUSEB_X2)
            em.button.extra_2=1;

        em.button_state.left=input->GetMouseButtonDown(Urho3D::MOUSEB_LEFT);
        em.button_state.right=input->GetMouseButtonDown(Urho3D::MOUSEB_RIGHT);
        em.button_state.middle=input->GetMouseButtonDown(Urho3D::MOUSEB_MIDDLE);
        em.button_state.extra_1=input->GetMouseButtonDown(Urho3D::MOUSEB_X1);
        em.button_state.extra_2=input->GetMouseButtonDown(Urho3D::MOUSEB_X2);

        insert_event_mouse_release(p.x_,p.y_,em.button.all,em.button_state.all);
    }

    void e_mouse_move(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData)
    {
        Urho3D::Input* input=_context->GetSubsystem<Urho3D::Input>();
        Urho3D::IntVector2 p=input->GetMousePosition();
        insert_event_mouse_move(p.x_,p.y_);
    }

    void e_mouse_wheel(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData)
    {
        int wheel=eventData[Urho3D::MouseWheel::P_WHEEL].GetInt();
        Urho3D::Input* input=_context->GetSubsystem<Urho3D::Input>();
        // In Urho the mouse wheel can only move vertically also the value is only 1 or -1 and LFGUI (like Qt) uses higher values
        insert_event_mouse_wheel(0,wheel*15);  // 15 is kinda where Qt's value is
    }

    void e_resize(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData)
    {
        int w=eventData[Urho3D::ScreenMode::P_WIDTH].GetInt();
        int h=eventData[Urho3D::ScreenMode::P_HEIGHT].GetInt();
        lfgui::widget::resize(w,h);
    }

    URHO3D_OBJECT(gui,Urho3D::Object)
};

/// \brief Uses the Urho3D::ResourceCache to load an image. Fails silently by returning a 1x1 image. Urho3D does
/// automatically log a loading error in his error log.
inline lfgui::image load_image(std::string path)
{
    Urho3D::String str(path.c_str(),path.size());
    Urho3D::SharedPtr<Urho3D::Image> i(gui::instance()->_context->GetSubsystem<Urho3D::ResourceCache>()->GetResource<Urho3D::Image>(str));

    if(!i)  // not loaded properly. Urho3D does automatically log this in his error log.
        return lfgui::image(1,1);

    image img(i->GetWidth(),i->GetHeight());

    for(int y=0;y<i->GetHeight();y++)
        for(int x=0;x<i->GetWidth();x++)
        {
            Urho3D::Color c(i->GetPixel(x,y));
            img.set_pixel(x,y,lfgui::color(c.r_*255,c.g_*255,c.b_*255,c.a_*255));
        }

    return img;
}

}       // namespace wrapper_urho3d
}       // namespace lfgui

#endif // LFGUI_WRAPPER_URHO3D
