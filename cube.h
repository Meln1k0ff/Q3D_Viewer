#ifndef CUBE_H
#define CUBE_H

#include <QWidget>
#include <QGLWidget>
#include <QMouseEvent>

class Cube : public QGLWidget
{
    Q_OBJECT
public:
     Cube(QWidget *parent = 0);
protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
private:
    void draw();
    int faceAtPosition(const QPoint &pos);
    GLfloat rotationX;
    GLfloat rotationY;
    GLfloat rotationZ;
    QColor faceColors[6];
    QPoint lastPos;
signals:

public slots:

};

#endif // CUBE_H
