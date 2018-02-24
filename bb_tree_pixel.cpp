#include "bb_tree_pixel.h"
#include <iostream>

BB_Tree_Pixel::_color BB_Tree_Pixel::_color_lay_color(_color &cnear, _color &cfar)
{
    long far_rate = (_color_max-cnear.a)*cfar.a;
    long near_rate = cnear.a*_color_max;

    _color cret;
    cret.a = __round_div(far_rate+near_rate,_color_max);

    cret.r = __round_div((__int64)cnear.r*near_rate + (__int64)cfar.r*far_rate,(far_rate+near_rate));
    cret.g = __round_div((__int64)cnear.g*near_rate + (__int64)cfar.g*far_rate,(far_rate+near_rate));
    cret.b = __round_div((__int64)cnear.b*near_rate + (__int64)cfar.b*far_rate,(far_rate+near_rate));
    return cret;
}

void BB_Tree_Pixel::__clear_sons(struct tree_unit *tr)
{
    if(tr->num_of_sons!=0)
    {
        for(int i=0;i<4;i++)
        {
            if(tr->sons[i]!=nullptr)
            {
                __clear_sons(tr->sons[i]);delete tr->sons[i];tr->sons[i]=nullptr;
            }
        }
        tr->num_of_sons=0;
    }
}
void BB_Tree_Pixel::__create_sons(struct tree_unit *tr)
{
    if(tr->num_of_sons!=0)return;
    int i;
    int dx = (tr->xend-tr->xbeg)/2;
    int dy = (tr->yend-tr->ybeg)/2;
    int cx = tr->xbeg+dx; int cy = tr->ybeg+dy;
    int x,y;
    for(i=0;i<4;i++)
    {
        if(tr->sons[i]==nullptr)
        {
            tr->sons[i] = new struct tree_unit;
            tr->sons[i]->num_of_sons = 0;
            tr->sons[i]->sons[0]=nullptr;tr->sons[i]->sons[1]=nullptr;tr->sons[i]->sons[2]=nullptr;tr->sons[i]->sons[3]=nullptr;
            tr->sons[i]->color.a=0;
            tr->sons[i]->color_sum.a=0;
            tr->sons[i]->depth = tr->depth+1;
            x = i/2; y=i%2;
            tr->sons[i]->xbeg = cx-dx+x?dx:0;
            tr->sons[i]->xend = cx+x?dx:0;
            tr->sons[i]->ybeg = cy-dy+y?dy:0;
            tr->sons[i]->yend = cy+y?dy:0;
        }
    }
    tr->num_of_sons = 4;
}

BB_Tree_Pixel::tree_unit * BB_Tree_Pixel::__clone_tree(struct tree_unit *tr)//克隆一棵矩形树，返回新树的指针
{
    if(tr==nullptr)return nullptr;
    struct tree_unit *cloned = new struct tree_unit;
    *cloned = *tr;//赋值。没有重载tree_unit的赋值运算符，不会直接复制孩子
    for(int i=0; i<4;i++)
    {
        cloned->sons[i] = __clone_tree(tr->sons[i]);
    }
    return cloned;
}

void BB_Tree_Pixel::__count_color_sum(struct tree_unit *node)
{
    __int64 rf=0, gf=0, bf=0;
    long alpha_sum=0;
    for(int i=0;i<4;i++)
    {
        if(node->sons[i]==nullptr)//取本节点的本色
        {
            alpha_sum += node->color.a;
            rf += node->color.r * node->color.a;
            gf += node->color.g * node->color.a;
            bf += node->color.b * node->color.a;
        }
        else//取子节点的合成色
        {
            long a = node->sons[i]->color_sum.a;
            alpha_sum += a;
            rf += node->sons[i]->color_sum.r * a;
            gf += node->sons[i]->color_sum.g * a;
            bf += node->sons[i]->color_sum.b * a;
        }
    }
    node->color_sum.a=(alpha_sum+2)/4;
    if(alpha_sum!=0)
    {
        //除法的四舍五入处理
        rf+=rf+alpha_sum;gf+=gf+alpha_sum;bf+=bf+alpha_sum;
        alpha_sum+=alpha_sum;
        //执行除法公式，执行后自带四舍五入
        node->color_sum.r = rf/alpha_sum;
        node->color_sum.g = gf/alpha_sum;
        node->color_sum.b = bf/alpha_sum;
    }
}

void BB_Tree_Pixel::__lay_color_to_node(struct tree_unit *node, _color &color)
{
    if(color.a>=_color_max)//完全填色，直接覆盖
    {
        __clear_sons(node);
        node->color = color; node->color_sum = color;
    }
    else
    {
        int i;
        //添加本色
        if(node->num_of_sons<4)
        {
            node->color = _color_lay_color(color, node->color);
        }
        if(node->num_of_sons!=0)
        {
            for(i=0;i<4;i++)
            {
                if(node->sons[i]!=nullptr)
                {
                    __lay_color_to_node(node->sons[i], color);
                }
            }
        }
        //本色处理完毕，计算合成色
        __count_color_sum(node);
    }
}

void BB_Tree_Pixel::__add_polygon_to_node(struct tree_unit *node, Polygon *p, _color &color)
{
    //debug检查代码，避免多边形超出节点表示的矩形范围
    Polygon::iterator itr; Point2 checker;
    itr = p->begin();
    while(itr!=p->end())
    {
        checker = p->get_point(itr);
        if(checker.x<node->xbeg||checker.x>node->xend||checker.y<node->ybeg||checker.y>node->yend)
        {
            qDebug()<<"Warning in void BB_Tree_Pixel::__add_polygon_to_node with rect"<<node->xbeg<<node->ybeg<<
                      node->xend<<node->yend<<"and"<<p->debug_string();
        }
        itr = itr.next();
    }/**/
    long cx = (node->xbeg+node->xend)/2; long cy = (node->ybeg+node->yend)/2;
    if(p->goes_on_the_extension_lines_of_rect(node->xbeg, node->ybeg, node->xend, node->yend))
    {
        //颜色添加到本层即可，直接叠色。次数为多边形p的重叠次数
        int times = p->cover_times(cx, cy);
        if(times<0)times = -times;
        for(int i=0;i<times;i++)
        {
            __lay_color_to_node(node, color);
        }
    }
    else if(node->depth < _max_depth)//需要四分
    {
        //qDebug()<<"recting from depth ="<<node->depth<<"with area"<<node->xbeg<<node->ybeg<<node->xend<<node->yend;
        //四分要处理的情况包括该节点没有孩子，该节点有部分孩子，该节点孩子为满。孩子不为满时且这个区域的矩形不为空时要生成孩子
        Polygon::four_divition_return_type divideds = p->four_divition(cx, cy);
        //qDebug()<<"four_divide "+p->debug_string()<<"at"<<cx<<cy<<"get:";
        //for(int t=0;t<4;t++)qDebug()<<divideds.p[t].debug_string();
        //qDebug()<<"four divition done";
        for(int i=0; i<4; i++)
        {
            if(divideds.p[i].getPointNum()<=2)//分解后的多边形为没有非0覆盖的多边形
            {
                continue;
            }
            if(node->sons[i]==nullptr)//还没有这个孩子
            {
                node->sons[i] = new struct tree_unit;
                node->sons[i]->color = node->color;node->sons[i]->color_sum = node->color_sum;
                node->num_of_sons++;
                int x = i/2; int y = i%2;
                node->sons[i]->xbeg = (x?cx:node->xbeg);
                node->sons[i]->xend = (x?node->xend:cx);
                node->sons[i]->ybeg = (y?cy:node->ybeg);
                node->sons[i]->yend = (y?node->yend:cy);
                node->sons[i]->depth = node->depth+1;
            }
            __add_polygon_to_node(node->sons[i], &(divideds.p[i]), color);
        }
        __count_color_sum(node);
        //qDebug()<<"endcounting";
    }
    else//到达最大深度，不四分，做粗略计算，进行叠色操作
    {
        int times = p->cover_times(cx, cy);
        if(times<0)times = -times;
        for(int i=0;i<times;i++)
        {
            __lay_color_to_node(node, color);
        }
    }
}

long BB_Tree_Pixel::__round_div(__int64 a, long b)
{
    if((a>0&&b>0)||(a<0&&b<0))
    {
        return (a*2+b)/(b*2);
    }
    else
    {
        return (a*2-b)/(b*2);
    }
}

void BB_Tree_Pixel::add_polygon(Polygon &p, int color[4])
{
    if(color[3]==0)return;//alpha为0没必要添加
    struct _color c; c.r = color[0]*_color_max; c.g = color[1]*_color_max; c.b = color[2]*_color_max; c.a = color[3]*_color_max;
    c.r+=c.r+255;c.g+=c.g+255;c.b+=c.b+255;c.a+=c.a+255;
    c.r/=510;c.g/=510;c.b/=510;c.a/=510;
    if(c.r<0)c.r=0;if(c.g<0)c.g=0;if(c.b<0)c.b=0;if(c.a<0)c.a=0;
    Polygon pc; pc = p;
    __add_polygon_to_node(&main_tree, &pc, c);
}

void BB_Tree_Pixel::add_polygon(Polygon &p, int r, int g, int b, int a)
{
    if(a==0)return;//alpha为0没必要添加
    struct _color c; c.r = r*_color_max; c.g = g*_color_max; c.b = b*_color_max; c.a = a*_color_max;
    c.r+=c.r+255;c.g+=c.g+255;c.b+=c.b+255;c.a+=c.a+255;
    c.r/=510;c.g/=510;c.b/=510;c.a/=510;
    if(c.r<0)c.r=0;if(c.g<0)c.g=0;if(c.b<0)c.b=0;if(c.a<0)c.a=0;
    Polygon pc; pc = p;
    __add_polygon_to_node(&main_tree, &pc, c);
}

int BB_Tree_Pixel::r(){
    return (main_tree.color_sum.r>_color_max?255:
        (main_tree.color_sum.r<0?0:((long)main_tree.color_sum.r*510+_color_max))/(_color_max*2));}
int BB_Tree_Pixel::g(){
    return (main_tree.color_sum.g>_color_max?255:
        (main_tree.color_sum.g<0?0:((long)main_tree.color_sum.g*510+_color_max))/(_color_max*2));}
int BB_Tree_Pixel::b(){
    return (main_tree.color_sum.b>_color_max?255:
        (main_tree.color_sum.b<0?0:((long)main_tree.color_sum.b*510+_color_max))/(_color_max*2));}
int BB_Tree_Pixel::a(){
    return (main_tree.color_sum.a>_color_max?255:
        (main_tree.color_sum.a<0?0:((long)main_tree.color_sum.a*510+_color_max))/(_color_max*2));}

void BB_Tree_Pixel::clear()
{
    __clear_sons(&main_tree); main_tree.color.a=0; main_tree.color_sum.a=0;
}

void BB_Tree_Pixel::set_color(int r, int g, int b, int a)
{
    __clear_sons(&main_tree);
    main_tree.color.r = r*_color_max/255; main_tree.color.g=g*_color_max/255;
    main_tree.color.b = b*_color_max/255; main_tree.color.a=a*_color_max/255;
    main_tree.color_sum = main_tree.color;
}
void BB_Tree_Pixel::__render_screen_set_color(int r, int g, int b, int a)
{
    __clear_sons(&main_tree);
    main_tree.color.r = r; main_tree.color.g=g;
    main_tree.color.b = b; main_tree.color.a=a;
    main_tree.color_sum = main_tree.color;
}
void BB_Tree_Pixel::__render_screen_lay_color(int r, int g, int b, int a)
{
    struct _color clr;
    clr.r = r; clr.g=g;
    clr.b = b; clr.a=a;
    __lay_color_to_node(&main_tree, clr);
}
BB_Tree_Pixel::BB_Tree_Pixel(int max_depth, int unit_length):_unit_length(unit_length),  _max_depth(max_depth)
{
    //input limit
    if(_unit_length<4){_unit_length=4;qDebug()<<"BB_Tree_Pixel::BB_Tree_Pixel, unit_length too low, auto changed to 4";}
    int md = 0; int ull = _unit_length;
    while(ull>1)
    {
        md++;ull/=2;
    }
    if(_max_depth>md)
    {
        md--;
        qDebug()<<"BB_Tree_Pixel::BB_Tree_Pixel, max_depth too high, auto changed to"<<md;
        _max_depth = md;
    }

    main_tree.xbeg = 0; main_tree.xend = _unit_length;
    main_tree.ybeg = 0; main_tree.yend = _unit_length;
    main_tree.num_of_sons = 0;
    main_tree.sons[0]=nullptr;main_tree.sons[1]=nullptr;main_tree.sons[2]=nullptr;main_tree.sons[3]=nullptr;
    main_tree.color.a = 0;
    main_tree.color_sum.a = 0;
    main_tree.depth = 0;

    _color_max = 32640;
}

BB_Tree_Pixel::~BB_Tree_Pixel()
{
    __clear_sons(&main_tree);
}

BB_Tree_Pixel &BB_Tree_Pixel::operator = (const BB_Tree_Pixel &p2)
{
    __clear_sons(&main_tree);
    main_tree = p2.main_tree;
    _unit_length = p2._unit_length; _max_depth = p2._max_depth;
    _color_max = p2._color_max;
    for(int i=0;i<4;i++)
    {
        main_tree.sons[i] = __clone_tree(p2.main_tree.sons[i]);
    }
    return *this;
}
/*
QImage BB_Tree_Pixel::debug_image()
{

}
*/
