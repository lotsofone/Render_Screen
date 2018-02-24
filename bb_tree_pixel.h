#ifndef BB_TREE_PIXEL_H
#define BB_TREE_PIXEL_H

#include "polygon.h"

class BB_Tree_Pixel
{
private:
    int _unit_length, _max_depth;
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
        int num_of_sons;
        struct _color color;
        struct _color color_sum;
        int depth;
        struct tree_unit *sons[4];
        tree_unit()
        {
            num_of_sons = 0; color.a = 0; color_sum.a = 0;
            sons[0]=nullptr; sons[1]=nullptr; sons[2]=nullptr; sons[3]=nullptr;
        }
    };
    struct tree_unit main_tree;
    void __clear_sons(struct tree_unit *tr);
    void __create_sons(struct tree_unit *tr);
    tree_unit *__clone_tree(struct tree_unit *tr);
    void __count_color_sum(struct tree_unit *node);
    void __lay_color_to_node(struct tree_unit *node, struct _color &color);
    void __add_polygon_to_node(struct tree_unit *node, Polygon *p, _color &color);
    long __round_div(__int64 a, long b);
public:
    void add_polygon(Polygon &p, int color[4]);
    void add_polygon(Polygon &p, int r, int g, int b, int a=255);
    int r();int g();int b();int a();
    void clear();
    void set_color(int r, int g, int b, int a);
    void __render_screen_set_color(int r, int g, int b, int a);
    void __render_screen_lay_color(int r, int g, int b, int a);
    BB_Tree_Pixel(int max_depth = 0, int unit_length = 1024*16);
    ~BB_Tree_Pixel();
    BB_Tree_Pixel &operator =(const BB_Tree_Pixel &p2);
    //QImage debug_image();
};

#endif // BB_TREE_PIXEL_H
