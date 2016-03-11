#ifndef LFGUI_QT_HYBRID_H
#define LFGUI_QT_HYBRID_H

#include <iostream>

#include <QWidget>
#include <QLayout>
#include <QPainter>

#include "signal.h"
#include "general.h"

namespace lfgui
{

/// @brief This class is a QWidget that adopts some of the LFGUI concepts like signals.
class qt_hybrid : public QWidget
{
    Q_OBJECT
public:
    signal<QPainter&> on_paint;
    std::vector<std::pair<QWidget*,widget_geometry>> widgets;

    qt_hybrid(QWidget *parent=0):QWidget(parent)
    {

    }

    void paintEvent(QPaintEvent* e) override
    {
        calc_children();
        QWidget::paintEvent(e);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing,true);
        on_paint.call(painter);
    }

    template<typename T>
    typename std::enable_if<std::is_base_of<QWidget,T>::value,T*>::type add_widget(T* w,widget_geometry g=widget_geometry())
    {
        widgets.emplace_back(std::pair<QWidget*,widget_geometry>(w,g));
        w->setParent(this);
        return w;
    }

    void calc_children()
    {
        for(std::pair<QWidget*,widget_geometry>& w:widgets)
        {
            //lfgui::rect r=w->geometry.calc_geometry(parentWidget()->width(),parentWidget()->height());
            lfgui::rect r=w.second.calc_geometry(width(),height());
            w.first->setGeometry(r.x,r.y,r.width,r.height);
            w.first->setFixedSize(r.width,r.height);
        }
    }
};

}   // namespace lfgui

#endif // LFGUI_QT_HYBRID_H
