#ifndef _DECODER_H_
#define _DECODER_H_

#include "SocketLayer.h"
#include "FrameBuffer.h"

namespace VNC
{
    /* Class:           Decoder
     * Description:     Decodes the rectangle data.
     */
    class Decoder
    {
    public:
        // Constructor.
        Decoder(FrameBuffer * buff) : m_buffer(buff) {
        }

        virtual int decode(rectangle & rect, SocketLayer &) = 0;

    protected:
        FrameBuffer * m_buffer;
    };
};

#endif