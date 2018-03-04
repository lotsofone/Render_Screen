    学生项目，带有高深度抗锯齿结合透明处理的多边形渲染程序。
### 主要功能：
输入多边形参数和颜色（rgba），输出渲染结果QImage对象。
### 主要特色：
通过四叉树算法实现高深度抗锯齿，时间复杂度随抗锯齿分度增加为O(n)级增长。从无抗锯齿到16*16抗锯齿只增加到16倍时间消耗。
可以记录某个色块在这一个像素的哪个位置。当多个多边形同时交在一个像素时，能正确根据这几个多边形在这个像素的可见面积计算出该像素的正确颜色。

### 使用方法：
```
    //Render_Screen 对象  Render_Screen(宽度，高度，像素分度，抗锯齿深度)  抗锯齿深度为0即无抗锯齿，为1即2*2抗锯齿，为2即4*4抗锯齿，为3即8*8抗锯齿，为4即16*16抗锯齿，以此类推
    Render_Screen scr(320,240,2048,4);
    //设置多边形
    Polygon ply3;
    ply3.insertPoint(15*2048,19*2048);ply3.insertPoint(260*2048,0*2048);ply3.insertPoint(320*2048,240*2048);
    ply3.close();
    //往src上画上一次多边形
    scr.add_polygon(ply3, 0, 112, 234, 255);
    //取出QImage对象并设置为label的图形以显示出来
    QPixmap pixmap; pixmap.convertFromImage(scr.draw_image());
    ui->label->setPixmap(pixmap);
```

### 绘制效果
```
    Polygon ply3;
    Render_Screen scr(320,240,2048,4);

    ply3.insertPoint(15*2048,19*2048);ply3.insertPoint(260*2048,0*2048);ply3.insertPoint(320*2048,240*2048);
    ply3.close();
    scr.add_polygon(ply3, 0, 112, 234, 34);
    ply3.clear();

    ply3.insertPoint(123*2048,32*2048);ply3.insertPoint(260*2048,221*2048);ply3.insertPoint(67*2048,113*2048);
    ply3.close();
    scr.add_polygon(ply3, 255, 67, 56, 55);
    ply3.clear();

    QPixmap pixmap; pixmap.convertFromImage(scr.draw_image());
    ui->label->setPixmap(pixmap);
```
如此可以绘制出两个三角形

<image src="https://github.com/lotsofone/image-repository/blob/master/Render_Screen/1.png"/>

```
    Polygon ply3;
    Render_Screen scr(320,240,2048,4);
        ply3.insertPoint(15*2048,19*2048);ply3.insertPoint(260*2048,0*2048);ply3.insertPoint(56*2048,240*2048);ply3.insertPoint(112*2048,221*2048);
    ply3.close();

    for(int i=1; i<100; i++){
        scr.add_polygon(ply3, 0, 112, 234, 11);
    }

    QPixmap pixmap; pixmap.convertFromImage(scr.draw_image());
    ui->label->setPixmap(pixmap);
```
重复以低alpha值画100次相同图形不会导致在边界出现锯齿

<image src="https://github.com/lotsofone/image-repository/blob/master/Render_Screen/2.png"/>

```
    Polygon ply3;
    Render_Screen scr(320,240,2048,4);

    for(int i=0; i<360; i++){
        ply3.insertPoint((100+80*sin(i*3.141592654/180))*2048,(100+80*cos(i*3.141592654/180))*2048);
    }
    ply3.close();
    scr.add_polygon(ply3, 0, 112, 234, 123);

    QPixmap pixmap; pixmap.convertFromImage(scr.draw_image());
    ui->label->setPixmap(pixmap);
```
画圆

<image src="https://github.com/lotsofone/image-repository/blob/master/Render_Screen/3.png"/>
