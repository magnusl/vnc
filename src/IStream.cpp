#include "IStream.h"
#include "common.h"
#include <iostream>

using namespace std;

namespace VNC
{
    bool IStream::writeInt32(int32 num)
    {
        num = VNC_INT32_SWAP(num);
        return writeBytes((const byte *) &num, sizeof(num));
    }

    bool IStream::writeUint32(uint32 num)
    {
        num = VNC_INT32_SWAP(num);
        return writeBytes((const byte *)&num, sizeof(num));
    }

    bool IStream::writeInt16(int16 num)
    {
        num = VNC_INT16_SWAP(num);
        return writeBytes((const byte *) &num, sizeof(num));
    }

    bool IStream::writeUint16(uint16 num)
    {
        num = VNC_INT16_SWAP(num);
        return writeBytes((const byte *) &num, sizeof(num));
    }

    /*
     *
     */
    bool IStream::readInt32(int32 & num)
    {
        int32 n;
        if(!readBytes((unsigned char *)&n, sizeof(n)))
            return false;
        num = VNC_INT32_SWAP(n);
        return true;
    }

    bool IStream::readUint32(uint32 & num)
    {
        uint32 n;
        if(!readBytes((unsigned char *)&n, sizeof(n)))
            return false;
        num = VNC_INT32_SWAP(n);
        return true;
    }

    bool IStream::readInt16(int16 & num)
    {
        int16 n;
        if(!readBytes((unsigned char *)&n, sizeof(n)))
            return false;
        num = VNC_INT16_SWAP(n);
        return true;
    }

    bool IStream::readUint16(uint16 & num)
    {
        uint16 n;
        if(!readBytes((unsigned char *)&n, sizeof(n)))
            return false;
        num = VNC_INT16_SWAP(n);
        return true;
    }

    /* Function:        IStream::readPixelFormat
     * Description:     Reads the PIXEL_FORMAR struct from the stream.
     */
    bool IStream::readPixelFormat(PIXEL_FORMAT & pf)
    {
        unsigned char padding[4];
        if(!readByte(pf.bitsPerPixel) ||
            !readByte(pf.depth) ||
            !readByte(pf.bigEndianFlag) ||
            !readByte(pf.trueColourFlag) ||
            !readUint16(pf.redMax) ||
            !readUint16(pf.greenMax) ||
            !readUint16(pf.blueMax) ||
            !readByte(pf.redShift) ||
            !readByte(pf.greenShift) ||
            !readByte(pf.blueShift) ||
            !readBytes(padding, 3))
        {
            cerr << "Could not read the pixel format from the stream" << endl;
            return false;
        }
        return true;
    }


    /* Function:        IStream::writePixelFormat
     * Description:     Writes a PIXEL_FORMAT struct to the stream.
     */
    bool IStream::writePixelFormat(const PIXEL_FORMAT & pf)
    {
        static unsigned char padding[] = {0xDE, 0xAD, 0xC0, 0xDE};
        if(!writeByte(pf.bitsPerPixel) ||
            !writeByte(pf.depth) ||
            !writeByte(pf.bigEndianFlag) ||
            !writeByte(pf.trueColourFlag) ||
            !writeUint16(pf.redMax) ||
            !writeUint16(pf.greenMax) ||
            !writeUint16(pf.blueMax) ||
            !writeByte(pf.redShift) ||
            !writeByte(pf.greenShift) ||
            !writeByte(pf.blueShift) ||
            !writeBytes(padding, 3))
        {
            cerr << "Could not write the PIXEL_FORMAT struct to the stream" << endl;
            return false;
        }
        return true;
    }



};