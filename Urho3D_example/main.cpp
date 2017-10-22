/**
* Released under the same permissive MIT-license as Urho3D.
* https://raw.githubusercontent.com/urho3d/Urho3D/master/License.txt
*/

#include <string>
#include <memory>
#include <fstream>
#include <sstream>

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Audio.h>


#include "../lfgui/lfgui_wrapper_urho3d.h"
#include "../lfgui/slider.h"
#include "../lfgui/label.h"
#include "../lfgui/button.h"
#include "../common_sample_code.h"

using namespace Urho3D;

/// \brief Calls SetModel on the given model and tries to load the model file and all texture files mentioned in a model_name+".txt".
/// model_name is supposed to have no file extension. Example: "Data/Models/Box", loads the model "Data/Models/Box.mdl".
/// It's a template to support all model classes like AnimatedModel and StaticModel.
template<typename T>
void set_model(T* model,Urho3D::ResourceCache* cache,std::string model_name)
{
    std::string filename_model=model_name;
    model->SetModel(cache->GetResource<Urho3D::Model>(Urho3D::String(filename_model.append(".mdl").c_str())));
    std::string filename_txt=model_name;
    filename_txt.append(".txt");
    std::ifstream file(filename_txt.c_str());
    std::string line;
    if(file.is_open())
        for(int i=0;getline(file,line);i++)
            model->SetMaterial(i,cache->GetResource<Urho3D::Material>(Urho3D::String(line.c_str())));
}

/// \brief SampleApplication main class mainly used for setup.
class SampleApplication : public Application
{
public:
    SharedPtr<Scene> scene_;
    Node* cameraNode_;
    SharedPtr<Urho3D::Node> node_rotating_planet;
    Urho3D::Text* window_text;
    SharedPtr<Urho3D::Window> window;
    Urho3D::Terrain* terrain;
    Urho3D::Camera* camera_;
    SharedPtr<Node> skyNode;
    SharedPtr<Node> node_torch;
    SharedPtr<Node> lightNode;
    SharedPtr<Cursor> cursor;
    std::unique_ptr<lfgui::wrapper_urho3d::gui> gui;

    SampleApplication(Context * context) : Application(context) {}

    virtual void Setup()
    {
        engineParameters_["FullScreen"]=false;
        engineParameters_["WindowWidth"]=1280;
        engineParameters_["WindowHeight"]=720;
        engineParameters_["WindowResizable"]=true;
        engineParameters_["Multisample"]=8;
        engineParameters_["VSync"]=true;
    }

    virtual void Start()
    {
        // create our LFGUI instance and set up the example widgets
        gui.reset(new lfgui::wrapper_urho3d::gui(GetContext()));
        setup_sample_gui(gui.get());

        // the rest below is creating a 3D scene and a Urho3D::GUI window and is not related to LFGUI

        ResourceCache* cache=GetSubsystem<ResourceCache>();

        scene_=new Scene(context_);
        scene_->CreateComponent<Octree>();
        scene_->CreateComponent<DebugRenderer>();

        cameraNode_=scene_->CreateChild("Camera");
        camera_=cameraNode_->CreateComponent<Camera>();
        camera_->SetFarClip(5000);
        camera_->SetNearClip(0.1);
        camera_->SetFov(75);
        SoundListener* listener=cameraNode_->CreateComponent<SoundListener>();
        GetSubsystem<Audio>()->SetListener(listener);
        GetSubsystem<Audio>()->SetMasterGain(SOUND_MUSIC,0.3);

        Renderer* renderer=GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_,scene_,cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0,viewport);
        renderer->SetShadowMapSize(1024);
        renderer->SetHDRRendering(true);
        renderer->SetShadowQuality(SHADOWQUALITY_PCF_24BIT);

        RenderPath* effectRenderPath=viewport->GetRenderPath();
        effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/AutoExposure.xml"));
        //effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
        effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR_stronger.xml"));
        effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));

        Node* zoneNode=scene_->CreateChild("Zone");
        Zone* zone=zoneNode->CreateComponent<Zone>();
        zone->SetBoundingBox(BoundingBox(-5000.0f,5000.0f));
        zone->SetFogStart(1000.0f);
        zone->SetFogEnd(2000.0f);
        zone->SetAmbientColor(Color(0.1,0.1,0.1));

        SubscribeToEvent(E_KEYDOWN,URHO3D_HANDLER(SampleApplication,HandleKeyDown));
        SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(SampleApplication,HandleUpdate));

        cameraNode_->SetPosition(Vector3(0,0,0));
        cameraNode_->SetDirection(Vector3::FORWARD);

        // create a transparent window with some text to display things like help and FPS
        {
            XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
            UI* ui=GetSubsystem<UI>();
            ui->GetRoot()->SetDefaultStyle(style);
            cursor=new Cursor(context_);
            cursor->SetStyleAuto(style);
            ui->SetCursor(cursor);
            GetSubsystem<Input>()->SetMouseVisible(true);

            window=new Window(context_);
            ui->GetRoot()->AddChild(window);
            window->SetStyle("Window");
            window->SetSize(600,70);
            window->SetColor(Color(.0,.15,.3,.5));
            window->SetAlignment(HA_LEFT,VA_TOP);

            window_text=new Text(context_);
            window_text->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),14);
            window_text->SetColor(Color(.8,.85,.9));
            window_text->SetAlignment(HA_LEFT,VA_TOP);
            window_text->SetText("Hello Urho!");
            window->AddChild(window_text);

            DebugHud* debugHud=engine_->CreateDebugHud();
            debugHud->SetDefaultStyle(style);
        }

        // a rotating planet
        {
            node_rotating_planet=scene_->CreateChild("Planet");
            node_rotating_planet->SetPosition(Vector3(-4,1.6,6));
            node_rotating_planet->Scale(2);
            StaticModel* boxObject=node_rotating_planet->CreateComponent<StaticModel>();
            boxObject->SetModel(cache->GetResource<Model>("Models/planet.mdl"));
            boxObject->SetMaterial(cache->GetResource<Material>("Materials/planet_dsn.xml"));
            boxObject->SetCastShadows(true);
        }

        // skybox
        {
            skyNode=scene_->CreateChild("Sky");
            skyNode->SetScale(1500.0f);
            Skybox* skybox=skyNode->CreateComponent<Skybox>();
            skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
            skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));
        }

        // a torch with a light, sound and particle effects
        {
            node_torch=scene_->CreateChild("Torch");
            //Vector3 pos(Vector3(3,-0.3,6));
            Vector3 pos(Vector3(3,0.3,6));
            node_torch->SetPosition(pos);

            // this model has quite a long loading time. Commented out for now. Example scene should be improved anyway.
            /*StaticModel* boxObject=node_torch->CreateComponent<StaticModel>();
            set_model(boxObject,cache,"Data/Models/torch");
            boxObject->SetCastShadows(true);
            boxObject->SetOccludee(true);
            boxObject->SetShadowDistance(200);
            boxObject->SetDrawDistance(200);*/

            Node* lightNode=node_torch->CreateChild();
            //lightNode->Translate(Vector3(0,2,0));
            Light* light=lightNode->CreateComponent<Light>();
            light->SetLightType(LIGHT_POINT);
            light->SetRange(50);
            light->SetBrightness(1.2);
            light->SetColor(Color(1.0,0.6,0.3,1.0));
            light->SetCastShadows(true);
            light->SetShadowDistance(200);
            light->SetDrawDistance(200);

            Node* n_particle=node_torch->CreateChild();
            //n_particle->Translate(Vector3(0,1.6,0));
            ParticleEmitter* emitter=n_particle->CreateComponent<ParticleEmitter>();
            emitter->SetEffect(cache->GetResource<ParticleEffect>("Particle/torch_fire.xml"));
            emitter=n_particle->CreateComponent<ParticleEmitter>();
            emitter->SetEffect(cache->GetResource<ParticleEffect>("Particle/torch_smoke.xml"));

            Sound* sound_torch=cache->GetResource<Sound>("Sounds/torch.ogg");
            sound_torch->SetLooped(true);
            SoundSource3D* sound_torch_source=n_particle->CreateComponent<SoundSource3D>();
            sound_torch_source->SetNearDistance(1);
            sound_torch_source->SetFarDistance(50);
            sound_torch_source->SetSoundType(SOUND_EFFECT);
            sound_torch_source->Play(sound_torch);
        }

        // sun
        {
            lightNode=scene_->CreateChild("Light");
            Light* light=lightNode->CreateComponent<Light>();
            light->SetLightType(LIGHT_DIRECTIONAL);
            light->SetCastShadows(true);
            light->SetShadowBias(BiasParameters(0.0005f,0.9f));
            light->SetShadowCascade(CascadeParameters(10.0f,50.0f,200.0f,0.0f,0.8f));
            //light->SetShadowResolution(1.0);
            //light->SetBrightness(1.0);
            light->SetColor(Color(1.0,0.9,0.8,1));
            lightNode->SetDirection(Vector3::FORWARD);
            lightNode->Yaw(-150);   // horizontal
            lightNode->Pitch(30);   // vertical
            lightNode->Translate(Vector3(0,0,-20000));

            BillboardSet* billboardObject=lightNode->CreateComponent<BillboardSet>();
            billboardObject->SetNumBillboards(1);
            billboardObject->SetMaterial(cache->GetResource<Material>("Materials/sun.xml"));
            billboardObject->SetSorted(true);
            Billboard* bb=billboardObject->GetBillboard(0);
            bb->size_=Vector2(10000,10000);
            bb->rotation_=Random()*360.0f;
            bb->enabled_=true;
            billboardObject->Commit();
        }

        {
            Node* terrainNode=scene_->CreateChild("Terrain");
            terrainNode->SetPosition(Vector3(3.0f,-0.4f));
            terrain=terrainNode->CreateComponent<Terrain>();
            terrain->SetPatchSize(128);
            terrain->SetSpacing(Vector3(2,0.5,2));
            terrain->SetSmoothing(true);
            terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.png"));
            terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
            terrain->SetCastShadows(true);
            //terrain->SetOccluder(true);
        }

        GetSubsystem<Input>()->SetMouseGrabbed(false);
    }

    virtual void Stop()
    {
    }

    void HandleUpdate(StringHash eventType,VariantMap& eventData)
    {
        float timeStep=eventData[Update::P_TIMESTEP].GetFloat();

        std::string str="WASD, mouse and shift to move. T to toggle fill mode,\nG to toggle GUI, Tab to toggle mouse mode, Esc to quit.\n";
        {
            std::ostringstream ss;
            ss<<1/timeStep;
            std::string s(ss.str());
            str.append(s.substr(0,6));
        }
        str.append(" FPS ");
        String s(str.c_str(),str.size());
        window_text->SetText(s);

        node_rotating_planet->Rotate(Quaternion(0,-22*timeStep,0));

        // Movement speed as world units per second
        float MOVE_SPEED=10.0f;
        // Mouse sensitivity as degrees per pixel
        const float MOUSE_SENSITIVITY=0.1f;

        // camera movement
        Input* input=GetSubsystem<Input>();
        if(input->GetQualifierDown(1))  // 1 is shift, 2 is ctrl, 4 is alt
            MOVE_SPEED*=10;
        if(input->GetKeyDown('W'))
            cameraNode_->Translate(Vector3(0,0, 1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('S'))
            cameraNode_->Translate(Vector3(0,0,-1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('A'))
            cameraNode_->Translate(Vector3(-1,0,0)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('D'))
            cameraNode_->Translate(Vector3( 1,0,0)*MOVE_SPEED*timeStep);

        if(!GetSubsystem<Input>()->IsMouseVisible())
        {
            IntVector2 mouseMove=input->GetMouseMove();

            static float yaw_=0;
            static float pitch_=0;
            yaw_+=MOUSE_SENSITIVITY*mouseMove.x_;
            pitch_+=MOUSE_SENSITIVITY*mouseMove.y_;
            pitch_=Clamp(pitch_,-90.0f,90.0f);
            // Reset rotation and set yaw and pitch again
            cameraNode_->SetDirection(Vector3::FORWARD);
            cameraNode_->Yaw(yaw_);
            cameraNode_->Pitch(pitch_);
        }
    }

    void HandleKeyDown(StringHash eventType,VariantMap& eventData)
    {
        using namespace KeyDown;
        int key=eventData[P_KEY].GetInt();

        if(key==KEY_TAB)
        {
            if(GetSubsystem<Input>()->GetMouseMode()==MM_RELATIVE)
            {
                GetSubsystem<Input>()->SetMouseMode(MM_ABSOLUTE);
                cursor->SetVisible(true);
            }
            else
            {
                GetSubsystem<Input>()->SetMouseMode(MM_RELATIVE);
                cursor->SetVisible(false);
            }
        }
        else if(key==KEY_ESC)
            engine_->Exit();
        else if(key==KEY_G)
        {
            //window_text->SetVisible(!window_text->IsVisible());
            gui->set_visible(!gui->visible());
        }
        else if(key==KEY_T)
            camera_->SetFillMode(camera_->GetFillMode()==FILL_WIREFRAME?FILL_SOLID:FILL_WIREFRAME);
        else if(key==KEY_1)
        {
            IntVector2 v=terrain->WorldToHeightMap(cameraNode_->GetWorldPosition());
            Image* i=terrain->GetHeightMap();
            for(int x=-10;x<10;x++)
                for(int y=-10;y<10;y++)
                    i->SetPixel(v.x_+x,v.y_+y,i->GetPixel(v.x_+x,v.y_+y)+Color(0.1,0.1,0.1));
            terrain->ApplyHeightMap();
        }
        else if(key==KEY_2)
        {
            IntVector2 v=terrain->WorldToHeightMap(cameraNode_->GetWorldPosition());
            Texture2D* t=(Texture2D*)terrain->GetMaterial()->GetTexture(TU_DIFFUSE);
            uint32_t c=Color(1,0,0).ToUInt();
            for(int x=-10;x<10;x++)
                for(int y=-10;y<10;y++)
                    t->SetData(0,v.x_+x,v.y_+y,1,1,&c);
            terrain->GetMaterial()->SetTexture(TU_DIFFUSE,t);
        }
        else if(key==KEY_F2)
        {
            DebugHud* debugHud=GetSubsystem<DebugHud>();
            if (debugHud->GetMode()!=DEBUGHUD_SHOW_ALL)
                debugHud->SetMode(DEBUGHUD_SHOW_ALL);
            else
                debugHud->SetMode(DEBUGHUD_SHOW_NONE);
        }
        else if (key == KEY_F3)
        {
            DebugHud* debugHud=GetSubsystem<DebugHud>();
            if (debugHud->GetMode()!=DEBUGHUD_SHOW_ALL)
                debugHud->SetMode(DEBUGHUD_SHOW_ALL);
            else
                debugHud->SetMode(DEBUGHUD_SHOW_NONE);
        }
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(SampleApplication)
