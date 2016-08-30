#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "../lfgui/lfgui_wrapper_qt.h"
#include "../lfgui/slider.h"
#include "../lfgui/button.h"
#include "../common_sample_code.h"

#include <QApplication>
#include <QMainWindow>

class Example : public QMainWindow
{
    Q_OBJECT
    lfgui::wrapper_qt::gui* gui=0;
public:
    Example(QWidget *parent = 0) : QMainWindow(parent)
    {
        lfgui::ressource_path="../lfgui_data/";
        gui=new lfgui::wrapper_qt::gui;
        resize(800,600);
        setStyleSheet("background-color:#ccc;");
        setCentralWidget(gui);
        setWindowTitle("LFGUI Qt example");

        setup_sample_gui(gui);
    }
};

#endif // EXAMPLE_H
