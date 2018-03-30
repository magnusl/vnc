#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include "PIXEL_FORMAT.h"
#include "rectangle.h"
#include "RGB.h"
#include "IStream.h"

namespace VNC
{
    /* Class:           FrameBuffer
     * Description:     The frame buffer,
     */
    class FrameBuffer
    {
    public:
        FrameBuffer();

        bool init(uint32 w, uint32 h);
        // returns the pixel format to use.
        const PIXEL_FORMAT & getPixelFormat();
        const byte * getDataPtr() {return m_data;}
        uint32 getWidth() {return m_width;}
        uint32 getHeight() {return m_height;}
        // copys a block inside the FB to another position.
        int blockCopy(unsigned int srcx, 
            unsigned int srcy,
            unsigned int dstx,
            unsigned int dsty,
            unsigned int width,
            unsigned int height);
        int updateRect(const rectangle &, IStream *);
        // draws a filled rect
        void drawFilledRect(const rectangle &, const unsigned char *);
        uint32 bytesPerPixel() {return m_bytesPerPixel;}
    private:

        unsigned char * m_data;                         // the RGB data.
        unsigned int m_width,                           // the width of the frame buffer
            m_height;                                   // the height of the frame buffer.
        PIXEL_FORMAT m_format;                          // the pixel format used.
        uint32 m_bytesPerPixel;
    };
};

#endif