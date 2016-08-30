#include "image.h"
#include "../external/cimg/CImg.h"

using namespace std;

namespace lfgui
{

image::image(std::string filename)
{
    *this=load(filename);
}

image::image(int width,int height)
{
    cimage.reset(new cimg(std::max(0,width),std::max(0,height),1,4));
}

image::image(const image& o)
{
    cimage.reset(new cimg(*o.cimage));
}

image& image::operator=(const image& o)
{
    cimage.reset(new cimg(*o.cimage));
    return *this;
}

int image::width()const{return cimage->width();}
int image::height()const{return cimage->height();}
uint8_t* image::data() const {return cimage->_data;}

image& image::resize_nearest(int w,int h){cimage->resize(std::max(0,w),std::max(0,h),1,4,1);return *this;}
image& image::resize_linear(int w,int h){cimage->resize(std::max(0,w),std::max(0,h),1,4,3);return *this;}
image& image::resize_cubic(int w,int h){cimage->resize(std::max(0,w),std::max(0,h),1,4,5);return *this;}
image& image::crop(int x,int y,int w,int h){cimage->crop(x,y,x+w,y+h);return *this;}

// based on https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
namespace
{
enum position{inside=0,left=1,right=2,bottom=4,top=8};

int compute_out_code(int x,int y,int max_x,int max_y)
{
    int code=position::inside;  // initialised as being inside of clip window

    if(x<0)                     // to the left of clip window
        code|=position::left;
    else if(x>max_x)            // to the right of clip window
        code|=position::right;
    if(y<0)                     // below the clip window
        code|=position::bottom;
    else if(y>max_y)            // above the clip window
        code|=position::top;

    return code;
}

/// \brief Returns true if the line should not be drawn (has been completely clipped).
bool clip_line(int& x0,int& y0,int& x1,int& y1,int max_x,int max_y)
{
    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    int outcode0=compute_out_code(x0,y0,max_x,max_y);
    int outcode1=compute_out_code(x1,y1,max_x,max_y);

    while(true)
    {
        if(!(outcode0|outcode1))    // Bitwise OR is 0. Trivially accept and get out of loop
            break;
        else if (outcode0 & outcode1)
            return true;
        else                        // failed both tests, so calculate the line segment to clip
        {
            // from an outside point to an intersection with clip edge
            int x,y;

            // At least one endpoint is outside the clip rectangle; pick it.
            int outcodeOut=outcode0?outcode0:outcode1;

            // Now find the intersection point;
            // use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
            if(outcodeOut&position::top)            // point is above the clip rectangle
            {
                x=x0+(x1-x0)*float(max_y-y0)/(y1-y0);
                y=max_y;
            }
            else if(outcodeOut&position::bottom)    // point is below the clip rectangle
            {
                x=x0+(x1-x0)*float(0-y0)/(y1-y0);
                y=0;
            }
            else if(outcodeOut&position::right)     // point is to the right of clip rectangle
            {
                y=y0+(y1-y0)*float(max_x-x0)/(x1-x0);
                x=max_x;
            }
            else// if(outcodeOut&position::left)      // point is to the left of clip rectangle
            {
                y=y0+(y1-y0)*float(0-x0)/(x1-x0);
                x=0;
            }

            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if(outcodeOut==outcode0)
            {
                x0=x;
                y0=y;
                outcode0=compute_out_code(x0,y0,max_x,max_y);
            }
            else
            {
                x1=x;
                y1=y;
                outcode1=compute_out_code(x1,y1,max_x,max_y);
            }
        }
    }

    return false;
}
}

// found at http://members.chello.at/~easyfilter/bresenham.html
void image::draw_line(int x0,int y0,int x1,int y1,color c)
{
    if(clip_line(x0,y0,x1,y1,width()-1,height()-1))
        return;

    int dx= abs(x1-x0);
    int sx=x0<x1?1:-1;
    int dy=-abs(y1-y0);
    int sy=y0<y1?1:-1;
    int err=dx+dy,e2; /* error value e_xy */

    for(;;)
    {
        blend_pixel(x0,y0,c);
        if(x0==x1&&y0==y1)
            break;
        e2=2*err;
        if(e2>=dy)
        {
            err+=dy;
            x0+=sx;
        } /* e_xy+e_x > 0 */
        if(e2<=dx)
        {
            err+=dx;
            y0+=sy;
        } /* e_xy+e_y < 0 */
    }
}

// based on http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
inline float distance_point_line(int point_x,int point_y,int line_x0,int line_y0,int line_x1,int line_y1)
{
    int A=point_x-line_x0;
    int B=point_y-line_y0;
    int C=line_x1-line_x0;
    int D=line_y1-line_y0;

    int dot=A*C+B*D;
    float len_sq=C*C+D*D;
    float param=-1;
    if(len_sq!=0) //in case of 0 length line
        param=dot/len_sq;

    int xx,yy;

    if(param<0)
    {
        xx=line_x0;
        yy=line_y0;
    }
    else if(param>1)
    {
        xx=line_x1;
        yy=line_y1;
    }
    else
    {
        xx=line_x0+param*C;
        yy=line_y0+param*D;
    }

    int dx=point_x-xx;
    int dy=point_y-yy;
    return sqrt(dx*dx+dy*dy);
}

void image::draw_line(int x0,int y0,int x1,int y1,color c,float w,float fading_start)
{
    if(clip_line(x0,y0,x1,y1,width()-1,height()-1))
        return;

    int dx=x1-x0;
    int dy=y1-y0;
    if(dx==0&&dy==0)
    {
        blend_pixel_safe(x0,y0,c);
        return;
    }

    // calculate a polygon that is the outer shape of our line. Like a bounding box, but not axis aligned.
    std::vector<point> polygon;
    {
        float d=sqrt(dx*dx+dy*dy);
        float rx=dx/d;
        float ry=dy/d;
        rx*=w;
        ry*=w;

        polygon.emplace_back(x0+ry-rx,y0-rx-ry);
        polygon.emplace_back(x1+ry+rx,y1-rx+ry);
        polygon.emplace_back(x1-ry+rx,y1+rx+ry);
        polygon.emplace_back(x0-ry-rx,y0+rx-ry);
    }

    // draw the actual line
    {
        std::vector<int> edges;
        edges.resize(height());
        int image_width=width();
        int image_height=height();

        int pixelX,pixelY,i,j,swap,node_count;

        //  Loop through the rows of the image.
        for(pixelY=0;pixelY<image_height;pixelY++)
        {
            //  Build a list of nodes.
            edges.resize(0);
            j=4-1;
            for(i=0;i<4;i++)
            {
                if((polygon[i].y<(double)pixelY&&polygon[j].y>=(double)pixelY)
                 ||(polygon[j].y<(double)pixelY&&polygon[i].y>=(double)pixelY))
                    edges.push_back((polygon[i].x+(pixelY-polygon[i].y)/double(polygon[j].y-polygon[i].y)*(polygon[j].x-polygon[i].x)));
                j=i;
            }
            node_count=edges.size();

            //  Sort the nodes, via a simple "Bubble" sort.
            i=0;
            while(i<node_count-1)
            {
                if(edges[i]>edges[i+1])
                {
                    swap=edges[i];
                    edges[i]=edges[i+1];
                    edges[i+1]=swap;
                    if(i)
                        i--;
                }
                else
                    i++;
            }

            //  Fill the pixels between node pairs.
            for(i=0;i<node_count;i+=2)
            {
                if(edges[i  ]>=image_width)
                    break;
                if(edges[i+1]> 0 )
                {
                    if(edges[i  ]<0 )
                        edges[i  ]=0 ;
                    if(edges[i+1]>image_width)
                        edges[i+1]=image_width;

                    for(pixelX=edges[i];pixelX<edges[i+1];pixelX++)
                    {
                        float s=distance_point_line(pixelX,pixelY,x0,y0,x1,y1)/w;
                        s-=fading_start/2;
                        if(s>0)
                            s/=(1.0f-fading_start);
                        int si=s*255;
                        si=std::min(255,si);
                        si=std::max(0,si);
                        si=255-si;
                        blend_pixel(pixelX,pixelY,c.alpha_multiplied(si));
                    }
                }
            }
        }
    }
}

void image::draw_rect(int x,int y,int width,int height,color color_foreground)
{
    lfgui::rect r=rect();
    int x_start=std::max(x,r.left());
    int y_start=std::max(y,r.top());
    int x_end=std::min(x+width,r.right());
    int y_end=std::min(y+height,r.bottom());
    int w=this->width();
    int i;
    uint8_t* d=data();
    int c=count();
    if(color_foreground.a==255)
    {
        for(y=y_start;y<y_end;y++)
        {
            i=y*w+x_start;
            memset(d+i,color_foreground.b,x_end-x_start);
            memset(d+i+c,color_foreground.g,x_end-x_start);
            memset(d+i+c*2,color_foreground.r,x_end-x_start);
            memset(d+i+c*3,255,x_end-x_start);
        }
    }
    else
    {
        for(y=y_start;y<y_end;y++)
        {
            x=x_start;
#ifdef __SSE2__
            __m128i v0=_mm_set1_epi32(0);
            __m128i v255=_mm_set1_epi16(255);
            __m128i v257=_mm_set1_epi16(257);
            __m128i cfga=_mm_set1_epi8(color_foreground.a);
            __m128i cfga_1=_mm_set1_epi16(color_foreground.a);
            __m128i cfga_1_neg=_mm_sub_epi16(v255,cfga_1);
            for(;x<((x_end-x_start)/16*16);x+=16)
            {
                uint8_t* d=data();
                i=y*w+x;

                __m128i cfg_1=_mm_set1_epi16(color_foreground.b);
                cfg_1=_mm_mullo_epi16(cfg_1,cfga_1);
                __m128i cbg=_mm_loadu_si128((const __m128i*)(d+i));
                __m128i cbg_1=_mm_unpacklo_epi8(cbg,v0);
                __m128i cbg_2=_mm_unpackhi_epi8(cbg,v0);
                cbg_1=_mm_mullo_epi16(cbg_1,cfga_1_neg);
                cbg_2=_mm_mullo_epi16(cbg_2,cfga_1_neg);
                cbg_1=_mm_adds_epu16(cbg_1,cfg_1);
                cbg_2=_mm_adds_epu16(cbg_2,cfg_1);
                cbg_1=_mm_mulhi_epu16(cbg_1,v257);
                cbg_2=_mm_mulhi_epu16(cbg_2,v257);
                cbg_1=_mm_packus_epi16(cbg_1,cbg_2);
                _mm_storeu_si128((__m128i*)(d+i),cbg_1);

                i+=c;
                cfg_1=_mm_set1_epi16(color_foreground.g);
                cfg_1=_mm_mullo_epi16(cfg_1,cfga_1);
                cbg=_mm_loadu_si128((const __m128i*)(d+i));
                cbg_1=_mm_unpacklo_epi8(cbg,v0);
                cbg_2=_mm_unpackhi_epi8(cbg,v0);
                cbg_1=_mm_mullo_epi16(cbg_1,cfga_1_neg);
                cbg_2=_mm_mullo_epi16(cbg_2,cfga_1_neg);
                cbg_1=_mm_adds_epu16(cbg_1,cfg_1);
                cbg_2=_mm_adds_epu16(cbg_2,cfg_1);
                cbg_1=_mm_mulhi_epu16(cbg_1,v257);
                cbg_2=_mm_mulhi_epu16(cbg_2,v257);
                cbg_1=_mm_packus_epi16(cbg_1,cbg_2);
                _mm_storeu_si128((__m128i*)(d+i),cbg_1);

                i+=c;
                cfg_1=_mm_set1_epi16(color_foreground.r);
                cfg_1=_mm_mullo_epi16(cfg_1,cfga_1);
                cbg=_mm_loadu_si128((const __m128i*)(d+i));
                cbg_1=_mm_unpacklo_epi8(cbg,v0);
                cbg_2=_mm_unpackhi_epi8(cbg,v0);
                cbg_1=_mm_mullo_epi16(cbg_1,cfga_1_neg);
                cbg_2=_mm_mullo_epi16(cbg_2,cfga_1_neg);
                cbg_1=_mm_adds_epu16(cbg_1,cfg_1);
                cbg_2=_mm_adds_epu16(cbg_2,cfg_1);
                cbg_1=_mm_mulhi_epu16(cbg_1,v257);
                cbg_2=_mm_mulhi_epu16(cbg_2,v257);
                cbg_1=_mm_packus_epi16(cbg_1,cbg_2);
                _mm_storeu_si128((__m128i*)(d+i),cbg_1);

                i+=c;
                cbg=_mm_loadu_si128((const __m128i*)(d+i));
                cbg_1=_mm_unpacklo_epi8(cbg,v0);
                cbg_2=_mm_unpackhi_epi8(cbg,v0);
                cbg_1=_mm_packus_epi16(cbg_1,cbg_2);
                cbg_1=_mm_adds_epu8(cbg_1,cfga);
                _mm_storeu_si128((__m128i*)(d+i),cbg_1);
            }
#endif
            for(;x<x_end;x++)
            {
                i=y*w+x;
                d[i]=(d[i]*(255-color_foreground.a)+color_foreground.b*color_foreground.a)*257>>16;
                i+=c;
                d[i]=(d[i]*(255-color_foreground.a)+color_foreground.g*color_foreground.a)*257>>16;
                i+=c;
                d[i]=(d[i]*(255-color_foreground.a)+color_foreground.r*color_foreground.a)*257>>16;
                i+=c;
                auto a=d[i]+color_foreground.a;
                d[i]=a>255?255:a;
            }
        }
    }
}

// based on http://alienryderflex.com/polygon_fill/
void image::draw_polygon(const std::vector<point>& vec,color c)
{
    int vec_size=vec.size();
    std::vector<int> edges;
    edges.resize(height());
    int image_width=width();
    int image_height=height();

    int pixelX,pixelY,i,j,swap,node_count;

    //  Loop through the rows of the image.
    for(pixelY=0;pixelY<image_height;pixelY++)
    {
        //  Build a list of nodes.
        edges.resize(0);
        j=vec_size-1;
        for(i=0;i<vec_size;i++)
        {
            if((vec[i].y<(double)pixelY&&vec[j].y>=(double)pixelY)
             ||(vec[j].y<(double)pixelY&&vec[i].y>=(double)pixelY))
                edges.push_back((vec[i].x+(pixelY-vec[i].y)/double(vec[j].y-vec[i].y)*(vec[j].x-vec[i].x)));
            j=i;
        }
        node_count=edges.size();

        //  Sort the nodes, via a simple "Bubble" sort.
        i=0;
        while(i<node_count-1)
        {
            if(edges[i]>edges[i+1])
            {
                swap=edges[i];
                edges[i]=edges[i+1];
                edges[i+1]=swap;
                if(i)
                    i--;
            }
            else
                i++;
        }

        //  Fill the pixels between node pairs.
        for(i=0;i<node_count;i+=2)
        {
            if(edges[i  ]>=image_width)
                break;
            if(edges[i+1]> 0 )
            {
                if(edges[i  ]<0 )
                    edges[i  ]=0 ;
                if(edges[i+1]>image_width)
                    edges[i+1]=image_width;
                for(pixelX=edges[i];pixelX<edges[i+1];pixelX++)
                    blend_pixel(pixelX,pixelY,c);
            }
        }
    }
}

void image::draw_image(int start_x,int start_y,const image& img)
{
    int end_x=start_x+img.width();
    int end_y=start_y+img.height();
    if(end_x>width())
        end_x=width();
    if(end_y>height())
        end_y=height();
    end_x-=start_x;
    end_y-=start_y;

    int target_x=start_x;
    int target_y=start_y;
    int index;
    int img_index;
    uint8_t* img_data=img.data();
    int count=width()*height();
    int img_count=img.width()*img.height();
    for(int y=0;y<end_y;y++)
    {
        index=target_y*width()+start_x;
        img_index=y*img.width();
        int x_offset=0;
        if(0>target_x)
        {
            x_offset=-target_x;
            target_x+=x_offset;
            index+=x_offset;
            img_index+=x_offset;
        }
        if(0<=target_y)
        {
            int x=x_offset;
#ifdef __SSE2__
            __m128i v0=_mm_set1_epi32(0);
            __m128i v255=_mm_set1_epi16(255);
            __m128i v257=_mm_set1_epi16(257);
            for(;x<((end_x-x_offset)/16*16);x+=16)
            {
                uint8_t* d=data();
                int offset1=0;
                int offset2=0;

                __m128i input2_a=_mm_loadu_si128((const __m128i*)(img_data+img_index+img_count*3));
                __m128i input2_a_1=_mm_unpacklo_epi8(input2_a,v0);
                __m128i input2_a_2=_mm_unpackhi_epi8(input2_a,v0);
                __m128i input2_a_1_neg=_mm_sub_epi8(v255,input2_a_1);
                __m128i input2_a_2_neg=_mm_sub_epi8(v255,input2_a_2);

                __m128i input1_b;
                __m128i input2_b;
                __m128i input1_b_1;
                __m128i input1_b_2;
                __m128i input2_b_1;
                __m128i input2_b_2;

                input1_b=_mm_loadu_si128((const __m128i*)(d+index+offset1));
                input2_b=_mm_loadu_si128((const __m128i*)(img_data+img_index+offset2));

                input1_b_1=_mm_unpacklo_epi8(input1_b,v0);
                input1_b_2=_mm_unpackhi_epi8(input1_b,v0);

                input2_b_1=_mm_unpacklo_epi8(input2_b,v0);
                input2_b_2=_mm_unpackhi_epi8(input2_b,v0);

                input1_b_1=_mm_mullo_epi16(input1_b_1,input2_a_1_neg);
                input1_b_2=_mm_mullo_epi16(input1_b_2,input2_a_2_neg);

                input2_b_1=_mm_mullo_epi16(input2_b_1,input2_a_1);
                input2_b_2=_mm_mullo_epi16(input2_b_2,input2_a_2);

                input1_b_1=_mm_adds_epu16(input1_b_1,input2_b_1);
                input1_b_2=_mm_adds_epu16(input1_b_2,input2_b_2);

                input1_b_1=_mm_mulhi_epu16(input1_b_1,v257);
                input1_b_2=_mm_mulhi_epu16(input1_b_2,v257);

                input1_b=_mm_packus_epi16(input1_b_1,input1_b_2);
                _mm_storeu_si128((__m128i*)(d+index+offset1),input1_b);

                offset1+=count;
                offset2+=img_count;

                input1_b=_mm_loadu_si128((const __m128i*)(d+index+offset1));
                input2_b=_mm_loadu_si128((const __m128i*)(img_data+img_index+offset2));

                input1_b_1=_mm_unpacklo_epi8(input1_b,v0);
                input1_b_2=_mm_unpackhi_epi8(input1_b,v0);

                input2_b_1=_mm_unpacklo_epi8(input2_b,v0);
                input2_b_2=_mm_unpackhi_epi8(input2_b,v0);

                input1_b_1=_mm_mullo_epi16(input1_b_1,input2_a_1_neg);
                input1_b_2=_mm_mullo_epi16(input1_b_2,input2_a_2_neg);

                input2_b_1=_mm_mullo_epi16(input2_b_1,input2_a_1);
                input2_b_2=_mm_mullo_epi16(input2_b_2,input2_a_2);

                input1_b_1=_mm_adds_epu16(input1_b_1,input2_b_1);
                input1_b_2=_mm_adds_epu16(input1_b_2,input2_b_2);

                input1_b_1=_mm_mulhi_epu16(input1_b_1,v257);
                input1_b_2=_mm_mulhi_epu16(input1_b_2,v257);

                input1_b=_mm_packus_epi16(input1_b_1,input1_b_2);
                _mm_storeu_si128((__m128i*)(d+index+offset1),input1_b);

                offset1+=count;
                offset2+=img_count;

                input1_b=_mm_loadu_si128((const __m128i*)(d+index+offset1));
                input2_b=_mm_loadu_si128((const __m128i*)(img_data+img_index+offset2));

                input1_b_1=_mm_unpacklo_epi8(input1_b,v0);
                input1_b_2=_mm_unpackhi_epi8(input1_b,v0);

                input2_b_1=_mm_unpacklo_epi8(input2_b,v0);
                input2_b_2=_mm_unpackhi_epi8(input2_b,v0);

                input1_b_1=_mm_mullo_epi16(input1_b_1,input2_a_1_neg);
                input1_b_2=_mm_mullo_epi16(input1_b_2,input2_a_2_neg);

                input2_b_1=_mm_mullo_epi16(input2_b_1,input2_a_1);
                input2_b_2=_mm_mullo_epi16(input2_b_2,input2_a_2);

                input1_b_1=_mm_adds_epu16(input1_b_1,input2_b_1);
                input1_b_2=_mm_adds_epu16(input1_b_2,input2_b_2);

                input1_b_1=_mm_mulhi_epu16(input1_b_1,v257);
                input1_b_2=_mm_mulhi_epu16(input1_b_2,v257);

                input1_b=_mm_packus_epi16(input1_b_1,input1_b_2);
                _mm_storeu_si128((__m128i*)(d+index+offset1),input1_b);

                __m128i input1_a=_mm_loadu_si128((const __m128i*)(d+index+count*3));
                __m128i input1_a_1=_mm_unpacklo_epi8(input1_a,v0);
                __m128i input1_a_2=_mm_unpackhi_epi8(input1_a,v0);

                input1_a_1=_mm_adds_epi16(input1_a_1,input2_a_1);
                input1_a_2=_mm_adds_epi16(input1_a_2,input2_a_2);

                offset1+=count;
                input1_a=_mm_packus_epi16(input1_a_1,input1_a_2);
                _mm_storeu_si128((__m128i*)(d+index+offset1),input1_a);

                target_x+=16;
                index+=16;
                img_index+=16;
            }
#endif
            for(;x<end_x;x++)
            {
                blend_pixel(index,count,img_data[img_index],img_data[img_index+img_count],img_data[img_index+img_count*2],img_data[img_index+img_count*3]);
                target_x++;
                index++;
                img_index++;
            }
        }
        target_x=start_x;
        target_y++;
    }
}

void image::draw_image(int start_x,int start_y,const image& img,float opacity)
{
    int end_x=start_x+img.width();
    int end_y=start_y+img.height();
    if(end_x>width())
        end_x=width();
    if(end_y>height())
        end_y=height();
    end_x-=start_x;
    end_y-=start_y;

    int target_x=start_x;
    int target_y=start_y;
    for(int y=0;y<end_y;y++)
    {
        for(int x=0;x<end_x;x++)
        {
            if(0<=target_x&&0<=target_y)
                blend_pixel(target_x,target_y,img.get_pixel(x,y).alpha_multiplied(opacity));
            target_x++;
        }
        target_x=start_x;
        target_y++;
    }
}

void image::draw_image_solid(int start_x,int start_y,const image& img)
{
    int end_x=start_x+img.width();
    int end_y=start_y+img.height();
    if(end_x>width())
        end_x=width();
    if(end_y>height())
        end_y=height();
    end_x-=start_x;
    end_y-=start_y;

    int target_x=start_x;
    int target_y=start_y;
    auto source=img.data();
    auto target=data();
    int source_count=img.width()*img.height();
    int target_count=width()*height();
    for(int y=0;y<end_y;y++)
    {
        /*for(int x=0;x<end_x;x++)
        {
            if(0<=target_x&&0<=target_y)
            {
                int source_i=x+y*img.width();
                int target_i=target_x+target_y*width();
                target[target_i]=source[source_i];
                target[target_i+target_count]=source[source_i+source_count];
                target[target_i+target_count*2]=source[source_i+source_count*2];
                target[target_i+target_count*3]=source[source_i+source_count*3];
        }
            target_x++;
        }*/
        memcpy(target+target_x+target_y*width(),source+y*img.width(),img.width());
        memcpy(target+target_x+target_y*width()+target_count,source+y*img.width()+source_count,img.width());
        memcpy(target+target_x+target_y*width()+target_count*2,source+y*img.width()+source_count*2,img.width());
        memcpy(target+target_x+target_y*width()+target_count*3,source+y*img.width()+source_count*3,img.width());
        //target_x=start_x;
        int i=target_x+target_y*width();
        int j=y*img.width();
        memcpy(data()+i,img.data()+j,1);
        target_y++;
    }
}

void image::draw_image_corners_stretched(int border_width,const image& img)
{
    int w=img.width();
    int h=img.height();

    // draw corners
    draw_image(0,0,img.cropped(0,0,w/2,h/2).scale(border_width,border_width));                                               // top left
    draw_image(width()-border_width,0,img.cropped(w/2,0,w/2,h/2).scale(border_width,border_width));                          // top right
    draw_image(0,height()-border_width,img.cropped(0,h/2,w/2,h/2).scale(border_width,border_width));                         // bottom left
    draw_image(width()-border_width,height()-border_width,img.cropped(w/2,h/2,w/2,h/2).scale(border_width,border_width));    // bottom right

    // draw borders
    draw_image(border_width,0,img.cropped(w/2,0,0,h/2).scale(width()-border_width*2,border_width));                          // top
    draw_image(border_width,height()-border_width,img.cropped(w/2,h/2,0,h/2).scale(width()-border_width*2,border_width));    // bottom
    draw_image(0,border_width,img.cropped(0,h/2,w/2,0).scale(border_width,height()-border_width*2));                         // left
    draw_image(width()-border_width,border_width,img.cropped(w/2,h/2,w/2,0).scale(border_width,height()-border_width*2));    // right

    // draw center
    draw_image(border_width,border_width,img.cropped(w/2,h/2,0,0).scale(width()-border_width*2,height()-border_width*2));
}

void image::fill(color c)
{
    int size=count();
    auto d=data();

    memset(d,c.b,size);
    d+=size;
    memset(d,c.g,size);
    d+=size;
    memset(d,c.r,size);
    d+=size;
    memset(d,c.a,size);
}

void image::clear()
{
    memset(data(),0,count()*4);
}

image::~image(){}

image& image::multiply(color c)
{
    int size=count();
    auto d=data();
    for(int i=0;i<size;i++)
    {
        *d=(*d)*c.b/255;
        d++;
    }
    for(int i=0;i<size;i++)
    {
        *d=(*d)*c.g/255;
        d++;
    }
    for(int i=0;i<size;i++)
    {
        *d=(*d)*c.r/255;
        d++;
    }
    return *this;
}

image& image::add(color c)
{
    int size=count();
    auto d=data();
    for(int i=0;i<size;i++)
    {
        *d=std::min(255,(*d)+c.b);
        d++;
    }
    for(int i=0;i<size;i++)
    {
        *d=std::min(255,(*d)+c.g);
        d++;
    }
    for(int i=0;i<size;i++)
    {
        *d=std::min(255,(*d)+c.r);
        d++;
    }
    return *this;
}

void image::draw_text(int x,int y,const std::string& text,const color& color,int font_size,alignment a,font& f)
{
    int x_orig=x;
    int w=f.text_length(text,font_size);
    if(a==alignment::center)
        x-=w/2;
    else if(a==alignment::right)
        x-=w;
    const char* end=text.data()+text.size();
    for(const char* data=text.data();data<end;data++)
    {
        if(*data=='\n')
        {
            x=x_orig;
            y+=font_size;
        }
        else
            draw_character(x,y,lfgui::utf8_to_unicode(data,end-data),color,font_size);
    }
}

void image::draw_character(int& x,int y,unsigned int character,const color& color,int font_size,font& f)
{
    if(character==' ')
    {
        x+=font_size/3;
        return;
    }
    if(character=='\t')
    {
        x+=font_size/3*4;
        return;
    }
    if(character<0x20)
        return;
    const font::bitmap& b=f.get_glyph_cached(character,font_size);
    for(int y2=0;y2<b.height();y2++)
        for(int x2=0;x2<b.width();x2++) // just adding 13 seems weird. Maybe there has to be some other calculation.
            blend_pixel_safe(x+x2+b.x0,y+y2+b.y0+13,color.alpha_multiplied(b.data[x2+y2*b.width()]));
    x+=b.width()+1;
}

void image::draw_path(const std::vector<point>& vec,color _color,bool connect_last_point_with_first)
{
    if(vec.size()<2)
        return;
    auto first_point=vec.begin();
    auto second_point=first_point;
    second_point++;
    while(second_point!=vec.end())
    {
        draw_line(first_point->x,first_point->y,second_point->x,second_point->y,_color);
        first_point=second_point;
        second_point++;
    }
    if(connect_last_point_with_first)
    {
        second_point=vec.begin();
        draw_line(first_point->x,first_point->y,second_point->x,second_point->y,_color);
    }
}

image image::rotated90() const
{
    image ret(height(),width());

    const int w=width();
    const int h=height();
    const int count=w*h;
    uint8_t* source=data();
    uint8_t* target=ret.data();

    for(int y=0;y<h;y++)
        for(int x=0;x<w;x++)
        {
            int i=x+y*w;
            int j=y+x*h;
            target[j]=source[i];
            target[j+count]=source[i+count];
            target[j+count*2]=source[i+count*2];
            target[j+count*3]=source[i+count*3];
        }

    return ret;
}

image& image::rotate180()
{
    const int w=width();
    const int h=height();
    const int count=w*h;
    uint8_t* source=data();
    const int end=count*4-1;
    uint8_t temp[4];

    for(int y=0;y<h;y++)
        for(int x=0;x<w;x++)
        {
            int i=x+y*w;
            temp[0]=source[i];
            temp[1]=source[i+count];
            temp[2]=source[i+count*2];
            temp[3]=source[i+count*3];
            source[i]=source[end-(i+count*3)];
            source[i+count]=source[end-(i+count*2)];
            source[i+count*2]=source[end-(i+count)];
            source[i+count*3]=source[end-i];
            source[end-i]=temp[3];
            source[end-(i+count)]=temp[2];
            source[end-(i+count*2)]=temp[1];
            source[end-(i+count*3)]=temp[0];
        }

    return *this;
}

image image::rotated270() const
{
    image ret(height(),width());

    const int w=width();
    const int h=height();
    const int count=w*h;
    uint8_t* source=data();
    uint8_t* target=ret.data();
    const int end=count*4-1;

    for(int y=0;y<h;y++)
        for(int x=0;x<w;x++)
        {
            int i=x+y*w;
            int j=y+x*h;
            target[end-(j+count*3)]=source[i];
            target[end-(j+count*2)]=source[i+count];
            target[end-(j+count)]=source[i+count*2];
            target[end-j]=source[i+count*3];
        }

    return ret;
}

}
