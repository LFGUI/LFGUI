#include "example.h"

#include <atomic>

int main(int argc, char *argv[])
{
    stk::stacktrace::instance().signal_handler=[](int,const char* signal_name)
    {
    STK_TRACE_CERR
        stk::stacktrace::instance().print(signal_name);
        QMessageBox::critical(0,"ERROR: Signal Received",QString::fromStdString(stk::stacktrace::instance().describe(signal_name)));
        exit(-1);
    };
    if(setjmp(stk::assert_handler::handler()))
    {
    STK_TRACE_CERR
        std::cerr<<stk::assert_handler::text()<<std::endl<<std::flush;
        QMessageBox::critical(0,"ERROR: Assert Triggered",QString::fromStdString(stk::assert_handler::text()));
        exit(-1);
    }
    STK_STACKTRACE

    QApplication a(argc, argv);
    Example w;
    w.show();

    return a.exec();
}
