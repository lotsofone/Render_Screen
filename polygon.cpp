#include "polygon.h"
#include <iostream>
#include <list>
#include <queue>

Polygon::__round_div(__int64 a, long b)
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

Polygon::Polygon()
{
    leadPoint = new struct point(0,0);
    lastPoint = leadPoint;
    pointNum = 0;
}

Polygon::~Polygon()
{
    while(leadPoint!=nullptr)
    {
        lastPoint = leadPoint->next;
        delete leadPoint;
        leadPoint = lastPoint;
    }
}

Polygon &Polygon::operator =(const Polygon &p2)
{
    pointNum = p2.pointNum;
    //清空多边形，使其只有一个点
    while(leadPoint->next!=nullptr)
    {
        lastPoint = leadPoint->next; delete leadPoint; leadPoint = lastPoint;
    }
    //闭合点赋值
    *leadPoint = *(p2.leadPoint);
    //其余点赋值
    struct point *pfp = p2.leadPoint; struct point *ca;
    while(pfp->next!=nullptr)
    {
        pfp = pfp->next;
        ca = new struct point;
        ca->x = pfp->x; ca->y = pfp->y; lastPoint->next = ca; lastPoint = ca;
    }
    return *this;
}

int Polygon::getPointNum(){
    return pointNum;
}

Polygon::iterator &Polygon::iterator::next()
{
    if(pointer!=nullptr)
    {
        lastpointer = pointer; pointer = pointer->next;
    }
    return *this;
}

bool Polygon::iterator::operator ==(const Polygon::iterator &other){return pointer == other.pointer;}
bool Polygon::iterator::operator !=(const Polygon::iterator &other){return pointer != other.pointer;}

Polygon::iterator Polygon::begin()
{
    iterator itr;
    itr.lastpointer = leadPoint; itr.pointer = leadPoint->next;
    return itr;
}
Polygon::iterator Polygon::end()
{
    iterator itr;
    itr.lastpointer = lastPoint; itr.pointer = nullptr;
    return itr;
}
Point2 Polygon::get_point(Polygon::iterator itr)
{
    Point2 p;
    p.x = itr.pointer->x; p.y = itr.pointer->y; return p;
}

bool Polygon::mid(long a, long b, long c)
{
    if(a<c && b>c){
        return true;
    }
    else if(a>c && b<c){
        return true;
    }
    else{
        return false;
    }
}
void Polygon::insertPoint(long x, long y)
{
    struct point* p1 = new point(x,y);
    lastPoint->next = p1;
    lastPoint = p1;
    pointNum++;
}
void Polygon::translate(long x, long y)//平移
{
    struct point *pp;
    pp = leadPoint;
    while(pp!=nullptr)
    {
        pp->x+=x; pp->y+=y; pp = pp->next;
    }
}

void Polygon::close()
{
    leadPoint->x = lastPoint->x; leadPoint->y = lastPoint->y;
}

void Polygon::simplify()//删除无用点
{
    if(pointNum>2)
    {
        std::list<struct point*> passed_points;
        struct point *f0,*f1,*f2;
        //删除无用点的操作在循环链表上较为容易处理，最后再拆开还原成原来的样子
        lastPoint->next = leadPoint->next;
        f0 = lastPoint; f1 = f0->next; f2 = f1->next;
        while(1)
        {
            //qDebug()<<f0->x<<f0->y<<" "<<f1->x<<f1->y<<" "<<f2->x<<f2->y<<" ";
            if((__int64)(f1->x-f0->x)*(f2->y-f1->y)-(__int64)(f1->y-f0->y)*(f2->x-f1->x)==0)//f1-f0 corss f2-f1 == 0
            {//三点一线，删中间的点
                if(!passed_points.empty())
                {
                    if(f1 == passed_points.front())passed_points.pop_front();
                }
                f0->next = f2; delete f1; pointNum--;
                if(pointNum<=2)//点不足的情况就清空，然后返回
                {
                    pointNum = 0;delete f0->next;delete f0; leadPoint->next = nullptr; return;
                }
                if(!passed_points.empty())
                {
                    f0 = passed_points.back(); passed_points.pop_back();
                }
                f1=f0->next; f2 = f1->next;
            }
            else
            {
                passed_points.push_back(f0);
                if(passed_points.front()==f0->next)break;//代表处理完成
                f0=f0->next; f1=f0->next; f2=f1->next;
            }
        }
        //到达这里的时候是处理完成且至少有三个点
         lastPoint = f0; leadPoint->next = lastPoint->next; lastPoint->next = nullptr;
         leadPoint->x = lastPoint->x; leadPoint->y = lastPoint->y;
         return;
    }
    else
    {
        pointNum = 0;
        if(leadPoint->next!=nullptr)
        {
            if(leadPoint->next->next!=nullptr)delete leadPoint->next->next;
            delete leadPoint->next;
            leadPoint->next = nullptr;
        }
        return;
    }
}

void Polygon::cut_into_rect(long xbeg, long ybeg, long xend, long yend)//裁剪到矩形内
{
    std::queue<std::pair<long, long>>que;
    long x1,x2,y1,y2,dx,dy;
    if(xbeg==xend||ybeg==yend)
    {
        clear();return;
    }
    if(xbeg>xend){x1=xbeg;xbeg=xend;xend=x1;}
    if(ybeg>yend){y1=ybeg;ybeg=yend;yend=y1;}

    struct point *pp;
    pp = leadPoint;
    while(pp->next!=nullptr)
    {
        x1 = pp->x; y1 = pp->y; x2 = pp->next->x; y2 = pp->next->y;
        dx=x2-x1;dy=y2-y1;
        long yedge1, yedge2;
        //插入头点
        if(ybeg<=y1&&y1<=yend)
        {
            que.push(std::make_pair(x1,y1));
        }
        if(dy==0){pp = pp->next;continue;}//交点不存在，同时避免下面出现除以0的情况
        //判断方向
        if(dy>0){yedge1=ybeg;yedge2=yend;}
        else{yedge1=yend;yedge2=ybeg;}
        long insx;
        if((y1<yedge1&&yedge1<y2)||(y1>yedge1&&yedge1>y2))//在y=yedge1处有第一个交点
        {
            insx = __round_div(((__int64)yedge1-y1)*dx,dy)+x1;
            que.push(std::make_pair(insx, yedge1));
        }
        if((y1<yedge2&&yedge2<y2)||(y1>yedge2&&yedge2>y2))//在y=yedge2处有交点
        {
            insx = __round_div(((__int64)yedge2-y1)*dx,dy)+x1;
            que.push(std::make_pair(insx, yedge2));
        }
        pp = pp->next;
    }
    //获得y裁剪之后的点序列，在que中。注意没有头点重复
    clear();
    //下面进行x裁剪，裁剪到x=xbeg x=xend范围内
    if(que.empty())return;
    x1 = que.back().first; y1=que.back().second;
    while(!que.empty())
    {
        x2=que.front().first; y2=que.front().second;
        //qDebug()<<x1/2048<<y1/2048<<x2/2048<<y2/2048;
        dx=x2-x1;dy=y2-y1;
        long xedge1, xedge2;
        //插入头点
        if(xbeg<=x1&&x1<=xend)
        {
            insertPoint(x1,y1);
        }
        if(dx==0){que.pop();x1=x2;y1=y2;continue;}//交点不存在，同时避免下面出现除以0的情况
        //判断方向
        if(dx>0){xedge1=xbeg;xedge2=xend;}
        else{xedge1=xend;xedge2=xbeg;}
        long insy;
        if((x1<xedge1&&xedge1<x2)||(x1>xedge1&&xedge1>x2))//在x=xedge1处有第一个交点
        {
            insy = __round_div(((__int64)xedge1-x1)*dy,dx)+y1;
            insertPoint(xedge1, insy);
        }
        if((x1<xedge2&&xedge2<x2)||(x1>xedge2&&xedge2>x2))//在y=yedge2处有交点
        {
            insy = __round_div(((__int64)xedge2-x1)*dy,dx)+y1;
            insertPoint(xedge2, insy);
        }
        que.pop();x1=x2;y1=y2;
    }
    //裁剪完成，最后简化处理
    close();
    simplify();
}

int Polygon::cover_times(long a, long b)
{
    int result_half = 0; int result_half_half = 0;
    struct point *current = leadPoint;
    long x1,y1,x2,y2;
    while(current->next!=nullptr)
    {
        x1 = current->x; y1 = current->y; x2 = current->next->x; y2 = current->next->y;
        __int64 le = (__int64)(x2-x1)*(b-y1)-(__int64)(y2-y1)*(a-x1);
        if(le!=0)
        {
            if(le>0&&y2>=b&&b>y1)result_half++;
            if(le>0&&y2>b&&b>=y1)result_half++;
            if(le<0&&y2<=b&&b<y1)result_half--;
            if(le<0&&y2<b&&b<=y1)result_half--;
        }
        else//点(a,b)在这条边所在的直线上
        {
            if(y2>=b&&b>y1)result_half_half++;
            if(y2>b&&b>=y1)result_half_half++;
            if(y2<=b&&b<y1)result_half_half--;
            if(y2<b&&b<=y1)result_half_half--;
        }
        current = current->next;
    }
    //if(result_half%2!=0)qDebug()<<"int Polygon::cover_times(long a, long b) point in edge with"<<a<<b<<"in"<<debug_string();
    return result_half/2+result_half_half/4;
    /*
    int result = 0;
    struct point *current = leadPoint->next;
    for(int i=1; i<=pointNum; i++){
        long x1,y1,x2,y2;
        if(i!=pointNum){
            x1=current->x, y1=current->y, x2=current->next->x, y2=current->next->y;
            current=current->next;
        }
        else{
            x1=current->x, y1=current->y;
            x2=leadPoint->x,y2=leadPoint->y;
        }
        //判断平行
        if(y1 == y2){
            if(x1 > a)
                if(y1 == b){
                    if(x1 > x2){
                        result-=2;
                    }
                    else if(x1 < x2){
                        result+=2;
                    }
                }
        }
        else if( ((__int64)(y1-b)*(a-x2) == (__int64)(b-y2)*(x1-a)) && mid(x1,x2,a)){
            if(y2>y1){
                result++;
            }
            else{
                result--;
            }
        }
        else if(mid(y1,y2,b)){
            if((b-y1)*((__int64)x1*y2-(__int64)x1*y1+x2-x1) >= (__int64)a*(y2-y1)){
                result+=2;
            }
        }
        else if(y1 == b && x1 >= a){
            result+=2;
        }
    }
    //qDebug()<<a<<","<<b<<"in"<<debug_string()<<"result in"<<result;
    return result;*/
}

Polygon::four_divition_return_type &Polygon::four_divition_return_type::operator =(const Polygon::four_divition_return_type &other){
    for(int i=0;i<4;i++)p[i]=other.p[i];
    return *this;
}

void Polygon::set_point(struct iterator itr, long x, long y)
{
    itr.pointer->x=x;itr.pointer->y=y;
}

Polygon::four_divition_return_type Polygon::four_divition(long xc, long yc)//x，y两条线把多边形分成四份并返回第一份的指针。可以用ret[i]来访问余下部分
{
    four_divition_return_type ret;
    struct point *pp;
    pp = leadPoint;
    long x1,y1,x2,y2;
    long dx,dy;
    std::queue<std::pair<long,long>> que[2];
    //先进行沿y=yc线（水平线）的分割，把裁剪的点信息存放到que[0]和que[1]
    //之后进行x=xc线的分割，que[0]分割进入ret.p[0]和ret.p[1]，que[1]同理
    while(pp->next!=nullptr)//水平分割
    {
        x1 = pp->x; y1 = pp->y; x2 = pp->next->x; y2 = pp->next->y;
        dx = x2-x1; dy=y2-y1;
        //这里遍历所有的边，每次得到(x1,y1)→(x2,y2)
        //插入头点
        if(y1<=yc)que[0].push(std::make_pair(x1,y1));
        if(y1>=yc)que[1].push(std::make_pair(x1,y1));
        //求交点
        long insx;
        if((y1<=yc&&y2<=yc)||(y1>=yc&&y2>=yc))//取出的线在水平线同侧，没有需要多插入的交点
        {
            ;
        }
        else//取出的线与y=yc线有交点
        {
            insx = dy==0?x1:__round_div((__int64)(yc-y1)*dx,dy)+x1;//求交点的x值。条件判断避免除数为0
            que[0].push(std::make_pair(insx,yc));
            que[1].push(std::make_pair(insx,yc));
        }
        pp = pp->next;
    }
    //水平分割之后，得到队列，下面做两次垂直分割，沿x=xc
    for(int i=0;i<2;i++)
    {
        //que[0]分割后进入ret.p[0]和ret.p[1]，que[1]分割后进入ret.p[2]和ret.p[3]
        //int ip = i+2;
        if(que[i].empty())continue;//有可能分割之后得到空队列
        x1 = que[i].back().first; y1 = que[i].back().second;
        while(!que[i].empty())
        {
            x2 = que[i].front().first; y2 = que[i].front().second;
            //得到(x1,y1)→(x2,y2)
            dx = x2-x1; dy=y2-y1;
            //插入头点
            if(x1<=xc)ret.p[i].insertPoint(x1,y1);
            if(x1>=xc)ret.p[i+2].insertPoint(x1,y1);
            //求交点
            long insy;
            if((x1<=xc&&x2<=xc)||(x1>=xc&&x2>=xc))//取出的线在垂直线同侧，没有需要多插入的交点
            {
                ;
            }
            else//取出的线与x=xc线有交点
            {
                insy = dx==0?y1:__round_div((__int64)(xc-x1)*dy,dx)+y1;//求交点的y值
                ret.p[i].insertPoint(xc,insy);
                ret.p[i+2].insertPoint(xc,insy);
            }
            //循环尾处理
            que[i].pop();
            x1=x2;y1=y2;
        }
    }
    //qDebug()<<"close and simplify";
    for(int i=0;i<4;i++)
    {
        ret.p[i].close();//闭合处理
        ret.p[i].simplify();//删除无用点
    }
    //qDebug()<<"returning";
    return ret;
}

int Polygon::even_distribution_in_rect(long xbeg, long ybeg, long xend, long yend)//多边形是否均匀分布在四个数表示的矩形中的每个位置
{
    struct point *pp;
    pp = leadPoint;
    long x1,y1,x2,y2;
    if(xbeg>xend){x1=xbeg;xbeg=xend;xend=x1;};
    if(ybeg>yend){y1=ybeg;ybeg=yend;yend=y1;};
    while(pp->next!=nullptr)
    {
        x1 = pp->x; y1 = pp->y; x2 = pp->next->x; y2 = pp->next->y;
        if((x1>=xend&&x2>=xend)||(x1<=xbeg&&x2<=xbeg)||(y1>=yend&&y2>=yend)||(y1<=ybeg&&y2<=ybeg))
        {pp=pp->next;continue;}//直接排除这条线段
        if((x1<xend&&x1>xbeg&&y1<yend&&y1>ybeg)||(x2<xend&&x2>xbeg&&y2<yend&&y2>ybeg))return 0;//点在矩形内，不能分布均匀
        __int64 f1,f2; int f3;
        f1 = (__int64)(x2-x1)*(yend-y1)-(__int64)(y2-y1)*(xbeg-x1);
        f2 = (__int64)(x2-x1)*(ybeg-y1)-(__int64)(y2-y1)*(xbeg-x1);
        f3 = ((x1<xbeg&&xbeg<x2)||(x1>xbeg&&xbeg>x2));
        if(f3&&((f1>0&&f2<0)||(f1<0&&f2>0)))return 0;//与左边界相交 xbeg
        f1 = (__int64)(x2-x1)*(yend-y1)-(__int64)(y2-y1)*(xend-x1);
        f2 = (__int64)(x2-x1)*(ybeg-y1)-(__int64)(y2-y1)*(xend-x1);
        f3 = ((x1<xend&&xend<x2)||(x1>xend&&xend>x2));
        if(f3&&((f1>0&&f2<0)||(f1<0&&f2>0)))return 0;//与右边界相交 xend
        f1 = (__int64)(x2-x1)*(ybeg-y1)-(__int64)(y2-y1)*(xbeg-x1);
        f2 = (__int64)(x2-x1)*(ybeg-y1)-(__int64)(y2-y1)*(xend-x1);
        f3 = ((y1<ybeg&&ybeg<y2)||(y1>ybeg&&ybeg>y2));
        if(f3&&((f1>0&&f2<0)||(f1<0&&f2>0)))return 0;//与下边界相交 ybeg
        f1 = (__int64)(x2-x1)*(yend-y1)-(__int64)(y2-y1)*(xbeg-x1);
        f2 = (__int64)(x2-x1)*(yend-y1)-(__int64)(y2-y1)*(xend-x1);
        f3 = ((y1<yend&&yend<y2)||(y1>yend&&yend>y2));
        if(f3&&((f1>0&&f2<0)||(f1<0&&f2>0)))return 0;//与上边界相交 yend
        pp=pp->next;
    }
    return 1;
}

int Polygon::goes_on_the_extension_lines_of_rect(long xbeg, long ybeg, long xend, long yend)//多边形是否沿着矩形边缘走
{
    struct point *pp;
    pp = leadPoint;
    long x1,y1,x2,y2;
    if(xbeg>xend){x1=xbeg;xbeg=xend;xend=x1;};
    if(ybeg>yend){y1=ybeg;ybeg=yend;yend=y1;};
    while(pp->next!=nullptr)
    {
        x1 = pp->x; y1 = pp->y; x2 = pp->next->x; y2 = pp->next->y;
        if(x1!=x2)//不可能沿着x方向走
        {
            if(y1!=y2 || (y1!=ybeg&&y1!=yend))return 0;
        }
        else if(y1!=y2)
        {
            if(x1!=xbeg&&x1!=xend)return 0;
        }
        else
        {
            if(x1!=xbeg&&x1!=xend&&y1!=ybeg&&y1!=yend)return 0;
        }
        pp = pp->next;
    }
    return 1;
}

QString Polygon::debug_string()
{
    QString str,s;
    str = "polygon";
    struct point *current = leadPoint->next;
    if(current==nullptr)str+=" empty";
    while(current!=nullptr)
    {
        str+=" "+s.setNum(current->x);
        str+=","+s.setNum(current->y);
        current = current->next;
    }
    return str;
}

void Polygon::clear()
{
    pointNum = 0;
    lastPoint = leadPoint->next;
    while(lastPoint!=nullptr)
    {
        delete leadPoint; leadPoint = lastPoint; lastPoint = lastPoint->next;
    }
    lastPoint = leadPoint;
}
