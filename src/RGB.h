#ifndef _RGB_H_
#define _RGB_H_

namespace VNC
{
    struct RGB
    {
        union
        {
            struct {
                unsigned char r, g, b, a;
            };
            unsigned int pixel;
        };
    };
};

#endif