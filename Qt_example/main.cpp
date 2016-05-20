#include "example.h"

#include <atomic>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Example w;
    w.show();

    return a.exec();
}
