#ifndef _PIXEL_FORMAT_H_
#define _PIXEL_FORMAT_H_

#include "common.h"

namespace VNC
{
    struct PIXEL_FORMAT
    {
        byte bitsPerPixel;
        byte depth;
        byte bigEndianFlag;
        byte trueColourFlag;
        unsigned short redMax,
            greenMax,
            blueMax;
        byte redShift,
            greenShift,
            blueShift;
        byte padding[3];
    };
};

#endif