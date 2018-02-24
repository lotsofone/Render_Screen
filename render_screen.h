#ifndef RENDER_SCREEN_H
#define RENDER_SCREEN_H

#include "qdebug.h"
#include "bb_tree_pixel.h"


class Render_Screen
{
private:
    int _unit_length;//单位长度
    int _pixel_unit_length;
    int _pixel_max_depth;
    int _width; int _height;//分辨率
    int _color_max;
    struct _color//这个单位的颜色以32640为最大值
    {
        long r,g,b,a;
        color(){r=0;g=0;b=0;a=0;};
    };

    struct _color _color_lay_color(_color &cnear, _color &cfar);

    struct tree_unit
    {
        int xbeg, ybeg, xend, yend;
        long num_of_covering_pixels;
        int num_of_sons;
        struct _color color;
        BB_Tree_Pixel *pixel;
        struct tree_unit *sons[4];
        tree_unit()
        {
            num_of_sons = 0; color.a = 0; pixel = nullptr; num_of_covering_pixels=0;
            sons[0]=nullptr; sons[1]=nullptr; sons[2]=nullptr; sons[3]=nullptr;
        }
    };
    struct tree_unit main_tree;
    void __clear_sons(struct tree_unit *tr);
    tree_unit *__clone_tree(struct tree_unit *tr);
    void __lay_color_to_node(struct tree_unit *node, struct _color &color);
    void __add_polygon_to_node(struct tree_unit *node, Polygon *p, struct _color &color);
    __round_div(__int64 a, long b);
    void _draw_node_to_image(struct tree_unit *node,QImage &ret);
public:
    void add_polygon(Polygon &p, int color[4]);
    void add_polygon(Polygon &p, int r, int g, int b, int a=255);
    void clear();
    QImage draw_image();
    Render_Screen &operator =(const Render_Screen &p2);
    Render_Screen(int w, int h, int ul = 2048, int pixel_max_depth=4);
    ~Render_Screen();

    int unit_length();//渲染屏的单位长度
    int width();//渲染屏的高度
    int height();//渲染屏的宽度
};

#endif // RENDER_SCREEN_H
