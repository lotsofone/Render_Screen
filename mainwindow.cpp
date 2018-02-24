#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <render_screen.h>

#include<math.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer(this);
    //通过这两条可以做动画
    //timer->start(10);
    //connect(timer,SIGNAL(timeout()),this,SLOT(xytimerUpDate()));
    //多边形对象和一个渲染屏对象
    Polygon ply3;
    Render_Screen scr(320,240,2048,4);
    //设置多边形
    ply3.insertPoint(15*2048,19*2048);ply3.insertPoint(260*2048,0*2048);ply3.insertPoint(320*2048,240*2048);
    ply3.close();
    scr.add_polygon(ply3, 0, 112, 234, 255);
    QPixmap pixmap; pixmap.convertFromImage(scr.draw_image());
    ui->label->setPixmap(pixmap);
}

MainWindow::~MainWindow()
{
    delete ui;
}

const int fac = 1;

Render_Screen scr(480*fac,480*fac,2048,4);
double a=0;
void MainWindow::xytimerUpDate()//简单动画示例
{
    Polygon ply3;

    scr.clear();
    ply3.insertPoint(222*2048*cos(a)*fac+240*2048*fac,222*2048*sin(a)*fac+240*2048*fac);
    ply3.insertPoint(156*2048*cos(a+2)*fac+240*2048*fac,156*2048*sin(a+2)*fac+240*2048*fac);
    ply3.insertPoint(166*2048*cos(a+4.4)*fac+240*2048*fac,166*2048*sin(a+4.4)*fac+240*2048*fac);
    ply3.close();
    scr.add_polygon(ply3, 0, 255, 0, 255);
    a+=0.00628;
    QPixmap pixmap; pixmap.convertFromImage(scr.draw_image());
    ui->label->setPixmap(pixmap);
}

