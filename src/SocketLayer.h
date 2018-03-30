#ifndef _SOCKETLAYER_H_
#define _SOCKETLAYER_H_

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#endif

#include "IStream.h"

namespace VNC
{
    /* Class:           SocketLayer
     * Description:     Socket layer implementation.
     */
    class SocketLayer : public IStream
    {
    public:
        // connects to the server
        int connect(const char *, const char *);
        // writes data to the socket
        bool writeBytes(const unsigned char *, int);
        // returns true if it's possible to read data.
        bool readPossible();
        // reads data from the socket
        bool readBytes(unsigned char *, int);
        // sets nonblocking mode
        void close();
    protected:
        // Windows implementation
        SOCKET m_sock;
    };
};

#endif