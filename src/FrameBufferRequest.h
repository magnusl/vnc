#ifndef _FRAMEBUFFERREQUEST_H_
#define _FRAMEBUFFERREQUEST_H_

namespace VNC
{
    /* Struct:          FrameBufferRequest
     * Description:     Requests that the frame buffer is updated.
     */
#pragma push(pack, 1)
    struct FrameBufferRequest
    {
        unsigned char type;
        unsigned char inc;
        unsigned short x, y, width, height;
    };
#pragma pop(pack)
};

#endif