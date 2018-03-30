#ifndef _RFB_H_
#define _RFB_H_

#include "SocketLayer.h"
#include "rectangle.h"
#include "FrameBuffer.h"
#include "vnc.h"
#include <string>
#include <ctime>

namespace VNC
{
    /* Class:           RFB
     * Description:     Remote Framebuffer Protocol.
     */
    class RFB
    {
    public:
        RFB();
        
        // Initalizes everything.
        int init(const char * password);

        enum MessageTypes {nFrameBufferUpdate = 0, nSetColourMapEntries , nBell, nServerCutText};

        FrameBuffer & getFrameBuffer() {return m_buffer;}

        int connect(const char *, const char *);
        int update();
        int requestUpdate(bool refresh);
        int updateMouse(uint16, uint16, unsigned char);
        time_t getLastUpdate() {return m_lastUpdate;}
        int keyevent(int, bool);
        void close();

    protected:

        // performs the VNC DES challenge response
        bool desChallenge(const byte * src, byte * dst, byte * key, size_t keylength);
        bool RFB::paddAndMirrorKey(const byte * src, size_t keylength, byte * dst);

        int handshake();            // performs the handshake
        int securityHandshake();    // the security handshake
        int initHandshake();

        int initSecurity3_3();      // version 3.3 
        int initSecurity3_7() {return FATAL_ERROR;}         // version 3.7
        int initSecurity3_8();

        int performSecurityCheck(byte);

        int sendPixelFormat(const PIXEL_FORMAT & format);
        int setEncodings();

        bool decideVersion(uint32 major, uint32 minor, uint32 & clientMajor, uint32 & clientMinor);
        bool buildVersionString(std::string &, unsigned int major, unsigned int minor);
        byte decideSecurityType(byte *, uint32);

        int authVNC();                      // VNC authentication.
        int decode(const rectangle &);      // decodes the frame buffer data
        
        int readPacket();
        SocketLayer m_socket;           // the socket layer.
        FrameBuffer m_buffer;           // the frame buffer.
        PIXEL_FORMAT m_format;          // the pixel format to use.
        char m_serverProtocolVersion[PROTOCOL_VERSION_LENGTH + 1];
        byte m_sharedFlag;

        static int32 encodings[4];
        bool m_bPassword;               // true if a password was supplied.
        std::string m_password;
        uint32 m_major, m_minor;        // the version used.
        time_t m_lastUpdate;
        bool m_bUpdating;
    };
};

#endif