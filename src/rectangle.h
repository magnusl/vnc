#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

namespace VNC
{
    /* Struct:          rectangle
     *
     */
    struct rectangle
    {
        rectangle() {}

        rectangle(short _x, short _y, short _w, short _h) 
        {
            x = _x;
            y = _y;
            width = _w;
            height = _h;
        }
        unsigned short x, y, width, height;
        int encoding;
    };
};

#endif