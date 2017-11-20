#ifndef LFGUI_WRAPPER_QT
#define LFGUI_WRAPPER_QT

#include <QImage>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTimer>
#include <QOpenGLFunctions>
#include <QCoreApplication>

#ifdef Q_OS_WIN
#include "gl/GLU.h"
#else
#include <glu.h>
#endif

#include "lfgui.h"
#include "../stk_debugging.h"
#include "../stk_timer.h"

namespace lfgui
{
namespace wrapper_qt
{

/// \brief This load_image function uses the QImage(QString) function. That means that it can also used the ressource
/// packing functionality of Qt (packing the ressources into the program, like the .exe, and adressing them with
/// ":/example.png").
inline lfgui::image load_image(const std::string& path)
{
    QImage qimage(QString::fromStdString(path));
    if(qimage.isNull())
    {
        //QMessageBox::critical(0,"LFGUI Error","LFGUI Error: The image \""+QString::fromStdString(path)+"\" could not be loaded.");
        std::cerr<<("LFGUI Error: The image \""+QString::fromStdString(path)+"\" could not be loaded.").toStdString()<<std::endl;
        return lfgui::image(1,1);
    }

    qimage.convertToFormat(QImage::Format_ARGB32);
    image img(qimage.width(),qimage.height());

#ifdef LFGUI_SEPARATE_COLOR_CHANNELS
    auto count=qimage.width()*qimage.height();
    uint8_t* data=qimage.bits();
    uint8_t* d=img.data();
    for(int i=0;i<count;i++,data+=4)
    {
        d[i]=data[0];
        d[i+count]=data[1];
        d[i+count*2]=data[2];
        d[i+count*3]=data[3];
    }
#else
    memcpy(img.data(),qimage.bits(),qimage.width()*qimage.height()*4);
#endif

    return img;
}

/// \brief The LFGUI Qt Wrapper. It is also a QWidget and can therefore be simply used as a QWidget.
class gui : public lfgui::gui,public QWidget
{
public:
    QImage qimage;
    QTimer *timer;  ///< a Qt timer used to do stuff like making a blinking cursor
    stk::timer fps_timer;

    gui(int width=1,int height=1) : lfgui::gui(width,height),qimage(width,height,QImage::Format_ARGB32)
    {
        lfgui::image::load=lfgui::wrapper_qt::load_image;
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        timer=new QTimer(this);
        fps_timer.reset();
        connect(timer,&QTimer::timeout,[this]{check_redraw();});
        timer->start(1000.0/(max_fps*4.0));
        on_resize([this](point p){img=std::move(image(p.x,p.y));});
        img=std::move(image(width,height));
    }

    void check_redraw()
    {
        if(fps_timer.until_now()>1.0/max_fps)
        {
            redraw(img,0,0);
            fps_timer.reset();
        }
    }

    void set_max_fps(int max_fps)
    {
std::cout<<"set_max_fps "<<max_fps<<std::endl;
        if(max_fps<1)
            return;
        this->max_fps=max_fps;
        //timer->setInterval(1000.0/(max_fps*4.0));
        timer->setInterval(1);
    }

    int width()const{return lfgui::widget::width();}
    int height()const{return lfgui::widget::height();}

    void redraw(image&,int,int) override
    {
//stk::timer _("REDRAW");
        STK_PROFILER_POINT("GUI redraw");
        img.clear();
        lfgui::widget::redraw(img,0,0);

{
//stk::timer _("REDRAW");
//STK_PROFILER_POINT("Qt redraw")
#ifdef LFGUI_SEPARATE_COLOR_CHANNELS
        uint8_t* data=qimage.bits();
        int count=qimage.width()*qimage.height();
        int count2=count*2;
        int count3=count*3;
        uint8_t* p=img.data();
        {
            int i=0;
#ifdef __SSE2__
            for(;i<count/64*64;i+=16,data+=64)
            {
                __m128i ib=_mm_loadu_si128((__m128i*)(p+i));
                __m128i ig=_mm_loadu_si128((__m128i*)(p+i+count));
                __m128i ir=_mm_loadu_si128((__m128i*)(p+i+count2));
                __m128i ia=_mm_loadu_si128((__m128i*)(p+i+count3));

                __m128i b0r0b7r7=_mm_unpacklo_epi8(ib,ir);
                __m128i g0a0g7a7=_mm_unpacklo_epi8(ig,ia);
                __m128i o1=_mm_unpacklo_epi8(b0r0b7r7,g0a0g7a7);
                __m128i o2=_mm_unpackhi_epi8(b0r0b7r7,g0a0g7a7);
                __m128i b8r8bFrF=_mm_unpackhi_epi8(ib,ir);
                __m128i g8a8gFaF=_mm_unpackhi_epi8(ig,ia);
                __m128i o3=_mm_unpacklo_epi8(b8r8bFrF,g8a8gFaF);
                __m128i o4=_mm_unpackhi_epi8(b8r8bFrF,g8a8gFaF);

                _mm_storeu_si128((__m128i*)(data),o1);
                _mm_storeu_si128((__m128i*)(data+16),o2);
                _mm_storeu_si128((__m128i*)(data+32),o3);
                _mm_storeu_si128((__m128i*)(data+48),o4);
            }
#endif
            for(;i<count;i++,data+=4)
            {
                data[0]=p[i];
                data[1]=p[i+count];
                data[2]=p[i+count2];
                data[3]=p[i+count3];
            }
        }
#else
        memcpy(qimage.bits(),img.data(),qimage.width()*qimage.height()*4);
#endif
}
{
        STK_PROFILER_POINT("Qt repaint")
        //repaint();
        update();
}
    }

    void resizeEvent(QResizeEvent* e) override
    {
        dirty=true;
        QWidget::resizeEvent(e);
        qimage=QImage(QWidget::width(),QWidget::height(),QImage::Format_ARGB32);
        lfgui::widget::resize(QWidget::width(),QWidget::height());
    }

    void paintEvent(QPaintEvent* e) override
    {
    //stk::timer _("Qt paintEvent");
        QWidget::paintEvent(e);

        QPainter painter(this);
        painter.drawImage(0,0,qimage);
        /*painter.beginNativePainting();
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, 64, 64);
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
        painter.endNativePainting();*/
    }

    void mousePressEvent(QMouseEvent* e) override
    {
        insert_event_mouse_press(e->x(),e->y(),e->button(),e->buttons());
    }

    void mouseReleaseEvent(QMouseEvent* e) override
    {
        insert_event_mouse_release(e->x(),e->y(),e->button(),e->buttons());
    }

    void mouseMoveEvent(QMouseEvent* e) override
    {
        QWidget::mouseMoveEvent(e);
        insert_event_mouse_move(e->x(),e->y());
        check_redraw();
//std::cout<<timer->remainingTime()<<std::endl;
//        QCoreApplication::processEvents();
//        if(timer->remainingTime()==0)
//            redraw(img,0,0);
//update();
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
