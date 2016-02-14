# LFGUI
Lightweight Flexible Graphical User Interface

This is a C++11 GUI toolkit designed to be:
- lightweight: being easy to integrate and having no required dependencies that require complicate building 
- flexible: easily customizable and designed so that every aspect can be customized easily
- a toolkit: no framework that takes all control away
- easily integratable: currently there's a Qt (a C++ GUI Framework) wrapper to use it inside a Qt application and a wrapper to use it inside the Urho3D game engine. The first wrapper is currently around 100 lines long and the second around 200. It can be easily integrated into anything that can display images.
- easy to use: some GUI systems are way more complicated to use than necessary
- customizable: other GUI systems tend to be hard to customize. LFGUI tries to use sane defaults to produce results without excessive amounts of code.

It is not:
- designed to completely replace something like Qt
- designed to imitate a certain look and feel, like of the operating system it is running on

One of the main targets are highly customized GUIs. Qt doesn't really help with those.

LFGUI is still in a relatively early development stage. There's no keyboard support and no layout system yet.

LFGUI draws everything on one resulting image to be easily integratable.  
It uses the cIMG library (http://cimg.eu/) which is just one header file and offers various image editing functions like drawing text.

LFGUI is mainly based on a lot of experience with Qt and having to fight with its shortcomings:

### Shortcomings of Qt

There a pre-compiler called MOC which is often criticized and has several bugs like sometimes not building correctly and one being forced to manually rebuild the whole project.  
Qts stylesheets system (that is like CSS in HTML used to change the appearance of widgets) is quite terrible. It can't be accessed via code directly, its just text. Just adding a property may break the stylesheet completely. The sub-selecting via "QWidgetName{...}" can't be used together with the plain properties like "font-size:12px;", this makes editing stylesheets per code unnecessarily complicated. Mistakes like typos are hard to find as there is no proper error reporting.  
Qts own widgets are hard to customize, one has to make a subclass for every tiny bit like drawing a centered text or an icon on a button with a slight offset, which is just one line of code by itself.  
Qt takes a lot of control away and forces one to use its thread and event system which makes often more work than necessary. Many things have to be done from the main thread for no apparent reason.  
Qt is HUGE and is hard to build.  
Qts layouts are not powerful enough.  
When doing custom widgets in Qt one often has to create some kind of own "primitive GUI system" inside Qt because Qt doesn't help at all with highly customized widgets.  
Qts signal and slot system requires a precompiler, is complicated to use and has no priorities.

### Concepts & Features

#### Image Based Rendering
  
Every widget has an image which it is drawing into in its redraw() function. The redraw function draws the current widget and then every child of this widget. The uppermost widget is of the type lfgui::gui as it is also acting as the manager of this LFGUI instance (there can be multiple instances active).  
This "software rendering approach" may be slower than a hardware accelerated approach, which some other GUI systems choose, but it is also more flexible and portable. Performance seems good so far even when rendering the full GUI completely every frame while having a 3D scene in the background.

#### Signal & Events

LFGUI has a lightweight signal event system. It simply uses std::functions ordered in a map with a priority.
lfgui::signal is a class with instances that are data member of many classes (like widgets), that want to emit some kind of event.   Functions or lambdas can be assigned to signals like this:  
`  button_save->on_mouse_click([this]{save();});`  
The signal class uses the operator() to append functions/lambdas.  
Optionally the priority can be set (otherwise the default of 0 is used):  
`  button_save->on_paint(-1,[this](lfgui::image& img){img.draw_image(10,10,background_image);});`  
All connected functions/lambdas are called sorted by their priority in ascending order.

All widgets that draw something are doing so by using their on_paint signal. This allows to replace the drawing code on a per-instance basis easily. Some widgets like the slider use multiple layer, this allows to add custom drawing code inbetween these layers. For example to manipulate the already drawn pixels by multiplying them with a color or mixing them with a gradient.

#### Hierachical Widgets

All widgets can have children. Childrens are positioned relative to their parent widget.
