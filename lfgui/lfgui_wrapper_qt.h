#ifndef LFGUI_WRAPPER_QT
#define LFGUI_WRAPPER_QT

#include <QImage>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTimer>

#include "lfgui.h"

namespace lfgui
{
namespace wrapper_qt
{

/// \brief This load_image function uses the QImage(QString) function. That means that it can also used the ressource
/// packing functionality of Qt (packing the ressources into the program, like the .exe, and adressing them with
/// ":/example.png").
inline lfgui::image load_image(std::string path)
{
    QImage qimage(QString::fromStdString(path));
    if(qimage.isNull())
    {
        QMessageBox::critical(0,"LFGUI Error","LFGUI Error: The image \""+QString::fromStdString(path)+"\" could not be loaded.");
        return lfgui::image(1,1);
    }

    qimage.convertToFormat(QImage::Format_ARGB32);
    image img(qimage.width(),qimage.height());

    auto count=qimage.width()*qimage.height();
    uint8_t* data=qimage.bits();
    for(int i=0;i<count;i++,data+=4)
    {
        img.data()[i]=data[0];
        img.data()[i+count]=data[1];
        img.data()[i+count*2]=data[2];
        img.data()[i+count*3]=data[3];
    }

    return img;
}

/// \brief The LFGUI Qt Wrapper. It is also a QWidget and can therefore be simply used as a QWidget.
class gui : public lfgui::gui,public QWidget
{
public:
    QImage qimage;

    gui(int width=1,int height=1) : lfgui::gui(width,height),qimage(width,height,QImage::Format_ARGB32)
    {
        lfgui::image::load=lfgui::wrapper_qt::load_image;
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        QTimer *timer=new QTimer(this);
        connect(timer,&QTimer::timeout,[this]{redraw(img,0,0);});
        timer->start(1000/25);  // draw with up to 25 FPS
        on_resize([this](point p){img=image(p.x,p.y);});
        img=image(width,height);
    }

    int width()const{return lfgui::widget::width();}
    int height()const{return lfgui::widget::height();}

    void redraw(image&,int offset_x,int offset_y) override
    {
        img.clear();
        {
        stk::timer _("redraw GUI");
        lfgui::widget::redraw(img,offset_x,offset_y);
        }
        int count=qimage.width()*qimage.height();
        int count2=count*2;
        int count3=count*3;
        uint8_t* data=qimage.bits();
        uint8_t* p=img.data();
        for(int i=0;i<count;i++,data+=4)
        {
            data[0]=p[i];
            data[1]=p[i+count];
            data[2]=p[i+count2];
            data[3]=p[i+count3];
        }
        repaint();
    }

    void resizeEvent(QResizeEvent* e) override
    {
        dirty=true;
        QWidget::resizeEvent(e);
        qimage=QImage(QWidget::width(),QWidget::height(),QImage::Format_ARGB32);
        lfgui::widget::resize(QWidget::width(),QWidget::height());
        //redraw();
    }

    void paintEvent(QPaintEvent* e) override
    {
        QWidget::paintEvent(e);

        QPainter p(this);
        p.drawImage(0,0,qimage);
    }

    void mousePressEvent(QMouseEvent* e) override
    {
        insert_event_mouse_press(e->x(),e->y(),e->button(),e->buttons());
        //redraw();
    }

    void mouseReleaseEvent(QMouseEvent* e) override
    {
        insert_event_mouse_release(e->x(),e->y(),e->button(),e->buttons());
        //redraw();
    }

    void mouseMoveEvent(QMouseEvent* e) override
    {
        insert_event_mouse_move(e->x(),e->y());
        //redraw();
    }

    // Some input can't be handled with these functions and has to be handled differently: https://www.kdab.com/qt-input-method-depth/ http://doc.qt.io/qt-5/qinputmethod.html
    void keyPressEvent(QKeyEvent* e) override
    {
        QByteArray arr=e->text().toUtf8();
        std::string character(arr.data(),arr.size());

        if(character.size()==1&&character[0]<=0x1F)
        {
            if(character=="\r")
                character="\n";
            else
                character.clear();
        }

        insert_event_key_press((lfgui::key)e->key(),character);
        //redraw();
    }

    void keyReleaseEvent(QKeyEvent* e) override
    {
        QByteArray arr=e->text().toUtf8();
        std::string character(arr.data(),arr.size());

        insert_event_key_release((lfgui::key)e->key(),character);
        //redraw();
    }

    void wheelEvent(QWheelEvent* e) override
    {
        insert_event_mouse_wheel(e->angleDelta().x()/12,e->angleDelta().y()/12);
        //redraw();
    }

    void set_cursor(mouse_cursor c) override
    {
        if(c==mouse_cursor::beam)
            setCursor(Qt::IBeamCursor);
        else if(c==mouse_cursor::cross)
            setCursor(Qt::CrossCursor);
        else if(c==mouse_cursor::hand_open)
            setCursor(Qt::OpenHandCursor);
        else if(c==mouse_cursor::hand_closed)
            setCursor(Qt::ClosedHandCursor);
        else if(c==mouse_cursor::hand_pointing)
            setCursor(Qt::PointingHandCursor);
        else if(c==mouse_cursor::wait)
            setCursor(Qt::WaitCursor);
        else if(c==mouse_cursor::busy)
            setCursor(Qt::BusyCursor);
        else if(c==mouse_cursor::forbidden)
            setCursor(Qt::ForbiddenCursor);
        else if(c==mouse_cursor::whats_this)
            setCursor(Qt::WhatsThisCursor);
        else if(c==mouse_cursor::split_horizontal)
            setCursor(Qt::SplitHCursor);
        else if(c==mouse_cursor::split_vertical)
            setCursor(Qt::SplitVCursor);
        else if(c==mouse_cursor::size_all)
            setCursor(Qt::SizeAllCursor);
        else if(c==mouse_cursor::size_horizontal)
            setCursor(Qt::SizeHorCursor);
        else if(c==mouse_cursor::size_vertical)
            setCursor(Qt::SizeVerCursor);
        else if(c==mouse_cursor::size_topleft_bottomright)
            setCursor(Qt::SizeFDiagCursor);
        else if(c==mouse_cursor::size_topright_bottomleft)
            setCursor(Qt::SizeBDiagCursor);
        else
            setCursor(Qt::ArrowCursor);
    }
};

}       // namespace wrapper_qt
}       // namespace lfgui

#endif // LFGUI_WRAPPER_QT
