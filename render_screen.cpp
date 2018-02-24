#include "render_screen.h"
#include <queue>
#include <algorithm>
#include <QImage>

Render_Screen::_color Render_Screen::_color_lay_color(_color &cnear, _color &cfar)
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


void Render_Screen::__clear_sons(struct tree_unit *tr)
{
    if(tr->num_of_sons!=0)
    {
        for(int i=0;i<4;i++)
        {
            if(tr->sons[i]!=nullptr)
            {
                if(tr->sons[i]->pixel!=nullptr)delete tr->sons[i]->pixel;
                __clear_sons(tr->sons[i]);delete tr->sons[i];tr->sons[i]=nullptr;
            }
        }
        tr->num_of_sons=0;
    }
}

Render_Screen::tree_unit * Render_Screen::__clone_tree(struct tree_unit *tr)//克隆一棵矩形树，返回新树的指针
{
    if(tr==nullptr)return nullptr;
    struct tree_unit *cloned = new struct tree_unit;
    if(tr->pixel!=nullptr)
    {
        tr->pixel = new BB_Tree_Pixel(_pixel_max_depth,_pixel_unit_length); *cloned->pixel = *(tr->pixel);
    }
    *cloned = *tr;//赋值。没有重载tree_unit的赋值运算符，不会直接复制孩子
    for(int i=0; i<4;i++)
    {
        cloned->sons[i] = __clone_tree(tr->sons[i]);
    }
    return cloned;
}

void Render_Screen::__lay_color_to_node(struct tree_unit *node, struct _color &color)
{
    if(node->xend-node->xbeg==1&&node->yend-node->ybeg==1)//单个像素，直接叠加到像素上
    {
        if(node->pixel==nullptr)qDebug()<<"Error in void Render_Screen::__lay_color_to_node, pixel not created";
        node->pixel->__render_screen_lay_color(color.r,color.g,color.b,color.a);
    }
    else if(color.a>=_color_max)//完全填色，直接覆盖
    {
        __clear_sons(node);
        node->color = color;
    }
    else
    {
        int i;
        //添加本色
        if(node->num_of_sons<4)//子节点不满，本层颜色有意义
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
    }
}

void Render_Screen::__add_polygon_to_node(struct tree_unit *node, Polygon *p, struct _color &color)
{
    //qDebug()<<"void Render_Screen::__add_polygon_to_node area"<<node->xbeg<<node->ybeg<<node->xend<<node->yend<<p->debug_string();
    if(node==nullptr)qDebug()<<"Error in void Render_Screen::__add_polygon_to_node, node not created";
    if(node->xend-node->xbeg==1&&node->yend-node->ybeg==1)//已经是一个像素大小了，直接添加到像素上
    {//对多边形做变换，使其适应像素坐标系
        if(node->pixel==nullptr)qDebug()<<"Error in void Render_Screen::__add_polygon_to_node, pixel not created";
        Polygon pc; long offx = -node->xbeg; long offy = -node->ybeg; offx*=_unit_length; offy*=_unit_length;
        int multiplier = _pixel_unit_length/_unit_length;
        std::queue<std::pair<long,long>> que;
        Polygon::iterator itr = p->begin();
        while(itr!=p->end())
        {
            Point2 pit = p->get_point(itr);
            que.push(std::make_pair(pit.x+offx,pit.y+offy));
            itr=itr.next();
        }
        while(!que.empty())
        {
            pc.insertPoint(que.front().first*multiplier,que.front().second*multiplier);
            que.pop();
        }
        pc.close();
        node->pixel->add_polygon(pc, color.r*255/_color_max, color.g*255/_color_max, color.b*255/_color_max, color.a*255/_color_max);
        return;
    }
    long cx = (node->xbeg+node->xend)/2; long cy = (node->ybeg+node->yend)/2;
    if(p->goes_on_the_extension_lines_of_rect(node->xbeg*_unit_length, node->ybeg*_unit_length,
                                              node->xend*_unit_length, node->yend*_unit_length))
    {
        //颜色添加到本层即可，直接叠色。次数为多边形p的重叠次数
        int times = p->cover_times((long)(node->xbeg+node->xend)*_unit_length/2, (long)(node->ybeg+node->yend)*_unit_length/2);
        if(times<0)times = -times;
        for(int i=0;i<times;i++)
        {
            __lay_color_to_node(node, color);
        }
    }
    else//需要四分
    {
        //四分要处理的情况包括该节点没有孩子，该节点有部分孩子，该节点孩子为满。孩子不为满时且这个区域的矩形不为空时要生成孩子
        Polygon::four_divition_return_type divideds = p->four_divition(cx*_unit_length, cy*_unit_length);
        //qDebug()<<"four_divide "+p->debug_string()<<"at"<<cx<<cy<<"get:";
        //for(int t=0;t<4;t++)qDebug()<<divideds.p[t].debug_string();
        //qDebug()<<"four divition done";
        for(int i=0; i<4; i++)
        {
            //qDebug()<<i<<divideds.p[i].debug_string();
            if(divideds.p[i].getPointNum()<=2)//分解后的多边形为没有非0覆盖的多边形
            {
                if(divideds.p[i].getPointNum()>0)qDebug()
                        <<"Warning in void Render_Screen::__add_polygon_to_node, wrong polygon allowed to arrive here:"
                       <<divideds.p[i].debug_string();
                continue;
            }
            if(node->sons[i]==nullptr)//还没有这个孩子
            {
                node->sons[i] = new struct tree_unit;
                node->sons[i]->color = node->color;
                node->num_of_sons++;
                int x = i/2; int y = i%2;
                node->sons[i]->xbeg = (x?cx:node->xbeg);
                node->sons[i]->xend = (x?node->xend:cx);
                node->sons[i]->ybeg = (y?cy:node->ybeg);
                node->sons[i]->yend = (y?node->yend:cy);
                if(node->sons[i]->yend==node->sons[i]->ybeg||node->sons[i]->xend==node->sons[i]->xbeg)
                    qDebug()<<"Error in void Render_Screen::__add_polygon_to_node";
                if(node->sons[i]->yend-node->sons[i]->ybeg==1&&node->sons[i]->xend-node->sons[i]->xbeg==1)//是一个像素大小了就创建像素并传递颜色
                {
                    node->sons[i]->pixel = new BB_Tree_Pixel(_pixel_max_depth,_pixel_unit_length);
                    node->sons[i]->pixel->__render_screen_set_color(node->color.r, node->color.g, node->color.b, node->color.a);
                }
            }
            //qDebug()<<"adding";
            __add_polygon_to_node(node->sons[i], &(divideds.p[i]), color);
        }
        //qDebug()<<"endcounting";
    }
}

Render_Screen::__round_div(__int64 a, long b)
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

void Render_Screen::_draw_node_to_image(struct tree_unit *node, QImage &ret)
{
    if(node->xend-node->xbeg==1&&node->yend-node->ybeg==1)//是一个像素的时候直接画像素并返回
    {

        //qDebug()<<"pixel fill"<<node->xbeg<<node->ybeg<<"with color"<<node->pixel->r()<<node->pixel->g()<<node->pixel->b()<<node->pixel->a();
        ret.setPixelColor(node->xbeg, node->ybeg, QColor(node->pixel->r(),node->pixel->g(),node->pixel->b(), node->pixel->a()));
        //ret.setPixelColor(node->xbeg, _height-1-node->ybeg, QColor(node->pixel->r(),node->pixel->g(),node->pixel->b(), node->pixel->a()));
        return;
    }
    long cx = (node->xbeg+node->xend)/2; long cy = (node->ybeg+node->yend)/2;
    for(int i=0;i<4;i++)
    {
        int x = i/2; int y = i%2;
        if(node->sons[i]==nullptr)//则以本层颜色为准
        {
            long sxbeg = (x?cx:node->xbeg);
            long sxend = (x?node->xend:cx);
            long sybeg = (y?cy:node->ybeg);
            long syend = (y?node->yend:cy);
            QColor tclr(node->color.r>_color_max?255:(node->color.r<0?0:(node->color.r*510+_color_max)/(_color_max*2)),
                        node->color.g>_color_max?255:(node->color.g<0?0:(node->color.g*510+_color_max)/(_color_max*2)),
                        node->color.b>_color_max?255:(node->color.b<0?0:(node->color.b*510+_color_max)/(_color_max*2)),
                        node->color.a>_color_max?255:(node->color.a<0?0:(node->color.a*510+_color_max)/(_color_max*2)));
            //qDebug()<<"area fill"<<sxbeg<<sybeg<<sxend<<syend<<"with color"<<node->color.r<<node->color.g<<node->color.b<<node->color.a;
            for(int x=sxbeg; x<sxend; x++)
            {
                for(int y = sybeg; y<syend; y++)
                {
                    ret.setPixelColor(x, y, tclr);//ret.setPixelColor(x, _height-y-1, tclr);
                }
            }
            /*for(int x=sxbeg; x<sxend; x++)
            {
                ret.setPixelColor(x, _height-sybeg-1, tclr);
                ret.setPixelColor(x, _height-syend, tclr);
            }
            for(int y = sybeg; y<syend; y++)
            {
                ret.setPixelColor(sxbeg, _height-y-1, tclr);
                ret.setPixelColor(sxend-1, _height-y-1, tclr);
            }*/
        }
        else//以孩子颜色为准
        {
            _draw_node_to_image(node->sons[i], ret);
        }
    }
}

void Render_Screen::add_polygon(Polygon &p, int color[4])
{
    if(color[3]==0)return;//alpha为0没必要添加
    struct _color c; c.r = color[0]*_color_max; c.g = color[1]*_color_max; c.b = color[2]*_color_max; c.a = color[3]*_color_max;
    c.r+=c.r+255;c.g+=c.g+255;c.b+=c.b+255;c.a+=c.a+255;
    c.r/=510;c.g/=510;c.b/=510;c.a/=510;
    if(c.r<0)c.r=0;if(c.g<0)c.g=0;if(c.b<0)c.b=0;if(c.a<0)c.a=0;
    Polygon pc; pc = p;
    pc.cut_into_rect(0, 0, _width*_unit_length, _height*_unit_length);
    __add_polygon_to_node(&main_tree, &pc, c);
}

void Render_Screen::add_polygon(Polygon &p, int r, int g, int b, int a)
{
    if(a==0)return;//alpha为0没必要添加
    struct _color c; c.r = r*_color_max; c.g = g*_color_max; c.b = b*_color_max; c.a = a*_color_max;
    c.r+=c.r+255;c.g+=c.g+255;c.b+=c.b+255;c.a+=c.a+255;
    c.r/=510;c.g/=510;c.b/=510;c.a/=510;
    if(c.r<0)c.r=0;if(c.g<0)c.g=0;if(c.b<0)c.b=0;if(c.a<0)c.a=0;
    Polygon pc; pc = p;
    //qDebug()<<"void Render_Screen::cut_into_rect";
    //qDebug()<<pc.debug_string();
    pc.cut_into_rect(0, 0, _width*_unit_length, _height*_unit_length);
    //qDebug()<<"void Render_Screen::add_polygon"<<pc.debug_string();
    __add_polygon_to_node(&main_tree, &pc, c);
}

void Render_Screen::clear()
{
    __clear_sons(&main_tree);
    main_tree.color.a = 0;
}

QImage Render_Screen::draw_image()
{
    QImage ret(_width,_height,QImage::Format_ARGB32);
    _draw_node_to_image(&main_tree, ret);
    return ret;
}

Render_Screen::Render_Screen(int width, int height, int unit_length, int pixel_max_depth)
    :_unit_length(unit_length), _width(width), _height(height), _pixel_max_depth(pixel_max_depth)
{
    main_tree.xbeg = 0; main_tree.xend = _width;
    main_tree.ybeg = 0; main_tree.yend = _height;
    main_tree.num_of_sons = 0;
    main_tree.sons[0]=nullptr;main_tree.sons[1]=nullptr;main_tree.sons[2]=nullptr;main_tree.sons[3]=nullptr;
    main_tree.color.a = 0;
    _pixel_unit_length = _unit_length;
    while(_pixel_unit_length<=16384)_pixel_unit_length*=2;

    _color_max = 32640;
    if(_width==1&&_height==1)main_tree.pixel = new BB_Tree_Pixel(_pixel_max_depth,_pixel_unit_length);
}

Render_Screen::~Render_Screen()
{
    __clear_sons(&main_tree);
    if(main_tree.pixel!=nullptr)delete main_tree.pixel;
}

Render_Screen &Render_Screen::operator = (const Render_Screen &p2)
{
    __clear_sons(&main_tree);
    main_tree = p2.main_tree;
    _unit_length = p2._unit_length; _pixel_max_depth = p2._pixel_max_depth;
    _color_max = p2._color_max;
    if(main_tree.pixel!=nullptr)delete main_tree.pixel;
    if(p2.main_tree.pixel!=nullptr){main_tree.pixel = new BB_Tree_Pixel(_pixel_max_depth,_pixel_unit_length);
        *main_tree.pixel = *p2.main_tree.pixel;};
    for(int i=0;i<4;i++)
    {
        main_tree.sons[i] = __clone_tree(p2.main_tree.sons[i]);
    }
    return *this;
}

int Render_Screen::unit_length(){return _unit_length;};
int Render_Screen::width(){return _width;};
int Render_Screen::height(){return _height;};
