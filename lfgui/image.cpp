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
    cimage.reset(new cimg(width,height,1,4));
    clear();
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

image& image::resize_nearest(int w,int h){cimage->resize(w,h,1,4,1);return *this;}
image& image::resize_linear(int w,int h){cimage->resize(w,h,1,4,3);return *this;}
image& image::resize_cubic(int w,int h){cimage->resize(w,h,1,4,5);return *this;}
image& image::crop(int x,int y,int w,int h){cimage->crop(x,y,x+w,y+h);return *this;}

void image::blend_pixel(int x,int y,color c)
{
    //if(x<0||y<0||x>=width()||y>=height()) // useful for debugging
    //    throw std::logic_error("");
    if(c.a==0)
        return;
    int count=width()*height();
    int i=x+y*width();
    cimage->_data[i]=(cimage->_data[i]*(255-c.a)+c.b*c.a)/255;
    cimage->_data[i+count]=(cimage->_data[i+count]*(255-c.a)+c.g*c.a)/255;
    cimage->_data[i+count*2]=(cimage->_data[i+count*2]*(255-c.a)+c.r*c.a)/255;
    cimage->_data[i+count*3]=std::min(255,cimage->_data[i+count*3]+c.a);
}

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
            else if(outcodeOut&position::left)      // point is to the left of clip rectangle
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

void image::draw_text(int x,int y,const std::string& text,const color& color_foreground,int font_size,float opacity,alignment a)
{
    //cimage->draw_text(x,y,text.c_str(),color_foreground.array,0,opacity,font_size);
    const cimg_library::CImgList<uint8_t>& font=cimg_library::CImgList<uint8_t>::font(font_size,true);
    int w=0;
    for(auto c:text)
        w+=font[c]._width;

    if(a==alignment::center)
        x-=w/2;
    else if(a==alignment::right)
        x-=w;
    cimage->draw_text(x,y,text.c_str(),color_foreground.array,0,opacity,font_size);
}

void image::draw_rect(int x,int y,int width,int height,color color_foreground)
{
    cimage->draw_rectangle(x,y,x+width,y+height,color_foreground.array);
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
            if(vec[i].y<(double)pixelY&&vec[j].y>=(double)pixelY
             ||vec[j].y<(double)pixelY&&vec[i].y>=(double)pixelY)
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
    for(int y=0;y<end_y;y++)
    {
        for(int x=0;x<end_x;x++)
        {
            if(0<=target_x&&0<=target_y)
                blend_pixel(target_x,target_y,img.get_pixel(x,y));
            target_x++;
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

void image::clear()
{
    cimage->fill((uint8_t)0,(uint8_t)0);
}

image::~image(){}

}
