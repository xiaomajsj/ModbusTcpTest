#include "resize.h"

Resize::Resize()
{

}

Resize::~Resize()
{

}
void Resize::ObjectResize(QWidget *object, double resizeParameterW, double resizeParameterH)
{
    object->setGeometry(object->x()*resizeParameterW,object->y()*resizeParameterH,object->width()*resizeParameterW,object->height()*resizeParameterH);
}

void  Resize::IconResize(QWidget *object, double resizeParameterW, double resizeParameterH)
{
    object->setGeometry(object->x()*resizeParameterW,object->y()*resizeParameterH,object->width()*resizeParameterH,object->height()*resizeParameterH);
}
void  Resize::IconResize2(QWidget *object, double resizeParameterW, double resizeParameterH)
{
    object->setGeometry(object->x()*resizeParameterW,object->y()*resizeParameterH,object->width()*resizeParameterW,object->height()*resizeParameterW);
}
void  Resize::resizeFont(QWidget *object, QString fontfamily, int originsize, double resizeParameterW, double resizeParameterH)
{
    int resize=originsize*((resizeParameterH+resizeParameterW)/2);
    QFont font(fontfamily, resize);
    object->setFont(font);
}
