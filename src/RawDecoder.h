#ifndef _RAWDECODER_H_
#define _RAWDECODER_H_

#include "Decoder.h"

namespace VNC
{
    /* Class:           RawDecoder
     * Description:     Decodes a raw rectangle.
     */
    class RawDecoder : public Decoder
    {
    public:
        RawDecoder(FrameBuffer * buff) : Decoder(buff) {
        }

        /* Function:        decode
         * Description:     Decodes the rectangle.
         */
        int decode(rectangle & rect, SocketLayer & socket)
        {
            // get the number of bytes per pixel
            unsigned int bytesPerPixel = m_buffer->bytesPerPixel();
            // now read the raw pixel data.
            if(!socket.read(m_tempBuff, rect.width * rect.height * bytesPerPixel)) {
                cerr << "Could not read the raw pixel data" << endl;
                return FATAL_ERROR;
            }
            // the data is read now let the framebuffer interperet it.
            if(!m_buffer->updateRect(rect, m_tempBuff)) {
                cerr << "Could not update the Frame buffer" << endl;
                return FATAL_ERROR;
            }
            // return success.
            return STATUS_SUCCESS;
        }
    }
};

#endif