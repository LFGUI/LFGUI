# LFGUI
Lightweight Flexible Graphical User Interface

This is a C++11 GUI toolkit designed to be:
- lightweight: having no or at least optional dependencies on external libraries and being easy to integrate
- flexible: easily customizable and so designed that every aspect can be customized easily
- a toolkit: no framework that takes away all control
- easily integratable: currently there's a Qt (a C++ GUI Framework) wrapper to use it inside a Qt application and a wrapper to use it inside the Urho3D game engine. The first wrapper is currently around 100 lines long and the second around 200. It can be easily integrated into anything that can display images.

It is not:
- designed to completely replace something like Qt
- designed to imitate a certain look and feel, like of the operating system it is running on

It is mainly based on a lot of experience with Qt and having to fight with its shortcomings:
There a pre-compiler called MOC which is often critisized and has several bugs like sometimes not building correctly and one being forced to manually rebuild the whole project.
Qts stylesheets system (like CSS in HTML used to change the appearance of widgets) is totally FUBAR. It can't be accessed by code directly, its just text. Just adding a property may brake the stylesheet completely. The sub-selecting via "QWidgetName{...}" can't be used together with the plain properties like "font-size:12px;", this make editing stylesheets per code unnecesarry complicated. Mistakes like typos are hard to find as there's no error being reported.
Qt own widgets are hard to customize, one has to make a subclass for every tiny bit like drawing a centered text or an icon on a button with a slight offset, which is just one line by itself.
Qt takes a lot of control away and forces one to use its thread and event system which makes often more work as necessary. Many things have to be done from the main thread for no apparent reason.
Qt is HUGE and is hard to build.
Qts layouts are not mighty enough.
When doing custom widgets in Qt one often has to create some kind of own "primitive GUI system inside Qt" because Qt doesn't help at all with highly customized widgets.

One of the main targets are highly customized GUIs. Qt sucks at those.

LFGUI has a lightweight signal event system. It simply uses std::functions ordered in a map with a priority. Qts signal and slot system requires a pre compiler, is complicated to use and has no priorities.

LFGUI is still in a relatively early development stage. There's no keyboard support yet and there's no layout yet.

LFGUI draws everything on one resulting image to be easily integratable.
It uses the cIMG library (http://cimg.eu/) which is just one header file and offersr various image editing functions like drawing text.
