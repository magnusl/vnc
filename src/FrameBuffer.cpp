#include "FrameBuffer.h"
#include <memory>
#include <iostream>
#include <assert.h>

using namespace std;

namespace VNC
{
    /* Function:        FrameBuffer::FrameBuffer    
     * Description:     Allocates the required memory for the frame buffer
     */
    FrameBuffer::FrameBuffer()
    {
        m_data = NULL;

        m_format.bigEndianFlag  = 0;        // not using Big Endian
        m_format.bitsPerPixel   = 32;
        m_format.blueMax        = 255;
        m_format.blueShift      = 8;
        m_format.depth          = 24;
        m_format.greenMax       = 255;
        m_format.greenShift     = 16;
        m_format.redMax         = 255;
        m_format.redShift       = 24;
        m_format.trueColourFlag = 1;        // using true colour.

        m_bytesPerPixel = 4;        // 4 bytes per pixel
    }

    /*
     *
     */
    bool FrameBuffer::init(uint32 w, uint32 h)
    {
        m_width = w;
        m_height = h;
        m_data = new (std::nothrow) unsigned char[w * h * 4];
        if(!m_data) return false;
        return true;
    }

    /* Function:        FrameBuffer::blockCopy
     * Description:     Performs a block copy.
     */
    int FrameBuffer::blockCopy(unsigned int srcx,unsigned int srcy,unsigned int dstx,
        unsigned int dsty,unsigned int width,unsigned int height)
    {
        unsigned char * src, * dst;
        unsigned int lineWidth = width * m_bytesPerPixel;
        // for all the lines.
        for(unsigned int y = srcy; y < (srcy + height);y++)
        {
            // calculate the source and destination.
            src = &m_data[(srcy * m_width) + srcx];
            dst = &m_data[(dsty * m_width) + dstx];
            // copy the line
            memcpy(dst, src, lineWidth);
        }
        return STATUS_SUCCESS;
    }

    /* Function:        FrameBuffer::updateRect
     * Read the raw data.
     */
    int FrameBuffer::updateRect(const rectangle & rect, IStream * stream)
    {
        unsigned char * ptr = 0;
        if(rect.x == 0 && (rect.width == m_width)) {
            // can copy everything with one read
            ptr = &m_data[(m_width * rect.y + rect.x) * m_bytesPerPixel];
            if(!stream->readBytes(ptr, rect.width * rect.height * m_bytesPerPixel)) {
                cerr << "Could not read the pixel data" << endl;
                return FATAL_ERROR;
            }
            return STATUS_SUCCESS;
        } else {
            // More than one read is required.
            unsigned int scanwidth = rect.width * m_bytesPerPixel;
            for(uint32 y = rect.y; y < (rect.y + rect.height); ++y) {
                // read the data
                unsigned char * ptr = &m_data[(m_width * y + rect.x) * 4];
                if(!stream->readBytes(ptr, scanwidth)) {
                    cerr << "Could not read the pixel data" << endl;
                    return FATAL_ERROR;
                }
            }
        }
        return STATUS_SUCCESS;
    }

    /* Function:        FrameBuffer::drawFilledRect
     * Description:     Draws a filled rectangle.
     */
    void FrameBuffer::drawFilledRect(const rectangle & rect, const unsigned char * pixel)
    {
        unsigned char * dst, * row;
        if(m_bytesPerPixel == sizeof(int))
        {
            unsigned int * idst;
            unsigned int col = *reinterpret_cast<const unsigned int *>(pixel);
            for(unsigned int y = rect.y; y < (rect.y + rect.height);y++)
            {
                row = &m_data[(y * m_width) * m_bytesPerPixel];
                idst = (unsigned int *) (row + rect.x * m_bytesPerPixel);
                for(unsigned int x = rect.x; x < (rect.width + rect.x); x++)
                {
                    //idst = (unsigned int *)(row + (x * m_bytesPerPixel));
                    *(idst++) = col;
                    
                    //memcpy(idst, pixel, 4);
                    
                    /*

                    *idst = col;
                    ++idst; // point to the next pixel.*/
                }
            }
        } else
        {
        
            // decode the pixeldata
            for(unsigned int y = rect.y; y < (rect.y + rect.height);y++)
            {
                row = &m_data[(y * m_width) * m_bytesPerPixel];
                for(unsigned int x = rect.x; x < (rect.width + rect.x); x++)
                {
                    dst = row + (x * m_bytesPerPixel);
                    memcpy(dst, pixel, m_bytesPerPixel);
                    //dst += m_bytesPerPixel;
                }
            }
        }
    }

    /*
     *
     */
    const PIXEL_FORMAT & FrameBuffer::getPixelFormat()
    {
        return m_format;
    }
    
};