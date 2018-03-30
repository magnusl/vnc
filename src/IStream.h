#ifndef _ISTREAM_H_
#define _ISTREAM_H_

#include "common.h"
#include "PIXEL_FORMAT.h"

namespace VNC
{
    class IStream
    {
    public:
        virtual bool readBytes(unsigned char *, int) = 0;
        virtual bool writeBytes(const unsigned char *, int) = 0;

        bool writeInt32(int32);             // writes a 32 bit signed integer
        bool writeUint32(uint32);       // writes a 32 bit unsigned integer.
        bool writeUint16(uint16);
        bool writeInt16(int16);

        bool readInt16(int16 &);
        bool readUint16(uint16 &);
        bool readInt32(int32 &);
        bool readUint32(uint32 &);

        bool readByte(unsigned char & b) {return readBytes(&b, sizeof(byte));}
        bool writeByte(unsigned char b) {return writeBytes(&b,sizeof(byte));}

        bool readPixelFormat(PIXEL_FORMAT &);
        bool writePixelFormat(const PIXEL_FORMAT &);
    };
};

#endif