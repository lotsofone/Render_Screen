#ifndef POLYGON_H
#define POLYGON_H
#include <qdebug.h>
#include <QString>
#include "point2.h"

class Polygon
{
private:
    struct point{
        long x;
        long y;
        struct point* next;
        point(long x, long y){this->x = x;this->y = y;this->next = nullptr;}
        point(){this->next = nullptr;}
    };
    int pointNum;
    struct point* leadPoint;
    struct point* lastPoint;
    struct point* itrPoint;
    __round_div(__int64 a, long b);
public:
    Polygon();
    ~Polygon();
    Polygon &operator =(const Polygon &p2);
    int getPointNum();
    class iterator{
        friend class Polygon;
    protected:
        struct point *pointer, *lastpointer;
    public:
        iterator &next();
        bool operator ==(const iterator &other);
        bool operator !=(const iterator &other);
    };
    iterator begin();
    iterator end();
    Point2 get_point(iterator itr);
    bool mid(long a, long b, long c);
    void insertPoint(long x, long y);
    void translate(long x, long y);//平移
    void close();//关闭多边形
    void simplify();//删除无用点
    void cut_into_rect(long xbeg, long ybeg, long xend, long yend);//裁剪到矩形内
    int cover_times(long a, long b);
    struct four_divition_return_type
    {Polygon *p;four_divition_return_type(){p = new Polygon[4];};~four_divition_return_type(){delete []p;};
        struct four_divition_return_type &operator =(const struct four_divition_return_type &other);
    };
    void set_point(struct iterator itr, long x, long y);
    four_divition_return_type four_divition(long xc, long yc);//x，y两条线把多边形分成四份并返回第一份的指针。可以用ret[i]来访问余下部分
    int even_distribution_in_rect(long xbeg, long ybeg, long xend, long yend);//多边形是否均匀分布在四个数表示的矩形中的每个位置
    int goes_on_the_extension_lines_of_rect(long xbeg, long ybeg, long xend, long yend);//多边形是否沿着矩形边缘走
    QString debug_string();
    void clear();//清除多边形的点
};

#endif // POLYGON_H
