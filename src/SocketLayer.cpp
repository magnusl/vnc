#include <Winsock2.h>
#include "SocketLayer.h"
#include <iostream>

using namespace std;

namespace VNC
{
    /* Function:        SocketLayer::connect
     * Description:     Connects to the server.
     */
    int VNC::SocketLayer::connect(const char * addr, const char * port)
    {
        addrinfo * info_list, hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        int nret = getaddrinfo(addr, port,&hints, &info_list);
        if(nret != 0) {
            wcerr << L"getaddrinfo failed" << gai_strerror(WSAGetLastError()) << endl;
            return FATAL_ERROR;
        }
    
        m_sock = socket(info_list->ai_family, SOCK_STREAM, IPPROTO_TCP);
        if(m_sock == INVALID_SOCKET)
        {
            cerr << "Could not create the socket" << endl;
            return FATAL_ERROR;
        }

        nret = ::connect(m_sock, (SOCKADDR*)info_list->ai_addr, (int)info_list->ai_addrlen);
        if(nret != 0) {
            cerr << "Could not connect to the server" << endl;
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }

    /* Function:        SocketLayer::readBytes
     * Description:     Reads data from the socket.
     */
    bool SocketLayer::readBytes(unsigned char * dst, int num)
    {
        // Wait until all the data has been read.
        int nret = recv(m_sock, (char *) dst, num , MSG_WAITALL);
        if(nret == num) return true;
        else if(nret == 0) {
            // the connection was closed
            return false;
        } else if(nret == SOCKET_ERROR) {
            // error while reading data
            return false;
        } else
            return false;
    }

    /* Function:        SocketLayer::writeBytes
     * Description:     Writes data to the socket.
     */
    bool SocketLayer::writeBytes(const unsigned char * src, int num)
    {
        int nret, bytesSent = 0;
        while(bytesSent < num)
        {
            nret = send(m_sock, (const char *)(src + bytesSent), num - bytesSent, 0);
            if(SOCKET_ERROR == nret)
                return false;
            bytesSent += nret;
        }
        return true;
    }

    /* Function:        readPossible
     * Description:     Checks if data is available for reading.
     */
    bool SocketLayer::readPossible()
    {
        int nret;
        TIMEVAL tv = {0, 0};
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(m_sock, &readset);
        nret = select(0, &readset, 0, 0, &tv);
        if(nret == SOCKET_ERROR) {
            return false;
        } else if(nret == 0) {
            return false;
        } else {
            return true;
        }
    }

    /*
     *
     */
    void SocketLayer::close()
    {
        closesocket(m_sock);
    }
};