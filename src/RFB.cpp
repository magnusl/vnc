#include "RFB.h"
#include "vnc.h"
#include "common.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <openssl/des.h>

using namespace std;

namespace VNC
{

    int32 RFB::encodings[] = {1,2,5,0};     // Copyrect, RRE, Hextile, Raw
    /*
     *
     */
    RFB::RFB()
    {
        m_sharedFlag = 1;
        m_bUpdating = true;
    }

    /*
     *
     */
    int RFB::connect(const char * addr,const char * port)
    {
        return m_socket.connect(addr, port);
    }

    /*
     *
     */
    void RFB::close()
    {
        m_socket.close();
    }

    /*
     *
     */
    bool RFB::paddAndMirrorKey(const byte * src, size_t keylength, byte * dst)
    {
        if(keylength > 8) return false;
        memset(dst, 0, 8);
        memcpy(dst, src, keylength);
        // Now mirror the bits
    /*  for(size_t i = 0;i < keylength;i++) {
            dst[i] = (dst[i] >> 7) | ((dst[i] >> 5) & 0x02) | ((dst[i] >> 3) & 0x04)  | ((dst[i] >> 1) & 0x08) |
                (dst[i] << 7) | ((dst[i] << 5) & 0x70) | ((dst[i] << 3) & 0x20) | ((dst[i] << 1) & 0x10);
        }*/
        return true;
    }

    /*
     *
     */
    bool RFB::desChallenge(const byte * src, byte * dst, byte * key, size_t keylength)
    {
        unsigned char encrypt_key[8];
        if(!paddAndMirrorKey(key, keylength, encrypt_key)) {
            cerr << "Could not calculate the encryption key" << endl;
            return false;
        }
        des_key_schedule schedule;
        des_set_key((C_Block *)encrypt_key, schedule);
        // encrypt it
        des_ecb_encrypt((C_Block *) src, (C_Block *) dst, schedule, 1);
        des_ecb_encrypt((C_Block *) (src + 8), (C_Block *) (dst + 8), schedule, 1);
        return true;
    }

    /* Function:        RFB::init
     * Description:     Initalizes the RFB protocol.
     */
    int RFB::init(const char * password)
    {
        if(password != NULL) {
            if(strlen(password) > 8) {
                cerr << "Password is to long" << endl;
                return FATAL_ERROR;
            } else {
                m_bPassword = true;
                m_password = password;
            }
        }
        // First perform the version handshake
        if(handshake() != STATUS_SUCCESS)
            return FATAL_ERROR;
        // then the security handshake
        if(securityHandshake() != STATUS_SUCCESS)
            return FATAL_ERROR;
        // the the initalization handshake
        if(initHandshake() != STATUS_SUCCESS)
            return FATAL_ERROR;
        // then send the desired pixel format
        if(sendPixelFormat(m_buffer.getPixelFormat()) != STATUS_SUCCESS)
            return FATAL_ERROR;
        // set the encodings to use
        if(setEncodings() != STATUS_SUCCESS)
            return FATAL_ERROR;
        // everything is done
        return STATUS_SUCCESS;
    }


    /* Function:        RFB::handshake
     * Description:     Performs the RFB handshake.
     */
    int RFB::handshake()
    {
        // read the servers protocol version
        if(!m_socket.readBytes((unsigned char *)m_serverProtocolVersion, PROTOCOL_VERSION_LENGTH)) {
            // could not read the servers protocol version.
        }       
        // Null the string
        m_serverProtocolVersion[PROTOCOL_VERSION_LENGTH + 1] = 0;

        // parse the servers protocol version
        char ident[4];
        int major, minor;
        if(sscanf(m_serverProtocolVersion, "%3s %d.%d", ident, &major, &minor) != 3) {
            // could not parse the protocol version string
            cerr << "Invalid protocol version format" << endl;
            return -HANDSHAKE_FAILED;
        }
        // null the identifier
        ident[3] = 0;
        if(strncmp(ident, "RFB", strlen("RFB")) != 0) {
            // not a valid identifier.
            cerr << "Invalid protocol version identifier" << endl;
            return -HANDSHAKE_FAILED;
        }

        uint32 clientMajor, clientMinor;
        // decide what version to use
        if(!decideVersion(major, minor, clientMajor, clientMinor)) {
            cerr << "Could not decide the protocol version" << endl;
            return -HANDSHAKE_FAILED;
        }
        m_major = clientMajor;
        m_minor = clientMinor;

        // create the version string. 
        std::string versionString;
        if(!buildVersionString(versionString,clientMajor, clientMinor)) {
            cerr << "Could not create the version string" << endl;
            return -HANDSHAKE_FAILED;
        }

        // send the clients protocol version.
        if(!m_socket.writeBytes((unsigned char *)versionString.c_str(), PROTOCOL_VERSION_LENGTH - 1) ||
            !m_socket.writeByte(0x0A))
        {
            // could not write the protocol version
            cerr << "Could not write the client version string" << endl;
            return -HANDSHAKE_FAILED;
        }
        // the handshake was a success.
        return STATUS_SUCCESS;  
    }

    /* Function:        RFB::buildVersionString
     * Description:     Builds the version string to use.
     */
    bool RFB::buildVersionString(std::string & ver, unsigned int major, unsigned int minor)
    {
        if(major > 999 || minor > 999) return false;
        
        char * zeros1 = (major >= 10 ? (major >= 100 ? "" : "0") : "00"),
            * zeros2 = (minor >= 10 ? (minor >= 100 ? "" : "0") : "00");
        stringstream ss; 
        ss << "RFB " << zeros1 << major << "." << zeros2 << minor;  
        ver = ss.str();
        assert(ver.length() == 11);
        return true;
    }

    /* Function:        decideVersion
     * Description:     Decides what version to use. The client currently implements Version 3.8
     */
    bool RFB::decideVersion(uint32 major, uint32 minor, uint32 & clientMajor, uint32 & clientMinor)
    {
        // the client supports 3.8
        if((major < 3) || (major == 3 && minor < 2)) {
            cerr << "Invalid version number, version number < 3.3" << endl;
            return false;
        } else if(major == 3)
        {
            // use version 3
            clientMajor = 3;
            // get the lowest minor number, the client implements 3.8
            clientMinor = min(minor, 8);
            // is the minor version correct?
            if(clientMinor == 3 || clientMinor == 7 || clientMinor == 8) {
                return true;
            }
            else {
                cerr << "Invalid minor version number, the protocol version '3." << clientMinor << " is not defined" << endl;
                return false;
            }
            return true;
        } else {
            // use version 3.8
            clientMajor = 3;
            clientMinor = 8;
            return true;
        }
    }

    /* Function:        RFB::decideSecurityType
     * Description:     decides what security to use.
     */
    unsigned char RFB::decideSecurityType(byte * types, uint32 num)
    {
        if(m_bPassword) {
            // a password was supplied, so use VNC authentication if it's supported
            for(uint32 i = 0;i < num; i++) {
                if(types[i] == AUTH_VNC) {
                    cerr << "Using VNC authentication" << endl;
                    return AUTH_VNC;
                }
            }
            // VNC not supported
            return AUTH_NONE;
        } else {
            return AUTH_NONE;
        }
    }

    /* Function:        RFB::securityHandshake
     * Description:     Performs the security handshake.
     */
    int RFB::securityHandshake()
    {
        if(m_major == 3)
        {
            switch(m_minor)
            {
            case 3:
                // version 3.3 is used
                return initSecurity3_3();
            case 7:
                // version 3.7 is used.
                return initSecurity3_7();
            case 8:
                // version 3.8 is used
                return initSecurity3_8();
            default:
                return FATAL_ERROR;
            }
        } else {
            return FATAL_ERROR;
        }
    }

    /* Function:        RFB::initSecurity3_8
     * Description:     Performs the security handshake in version 3.8
     */
    int RFB::initSecurity3_8()
    {
        // read the supported security types.
        unsigned char num;
        if(!m_socket.readByte(num)) {
            cerr << "Could not read the number-of-security-types byte" << endl;
            return FATAL_ERROR;
        }
        // parse the data read,
        if(num == 0)
        {
            cerr << "Can't perform the security handshake, no options available" << endl;
            return FATAL_ERROR;
        } else {
            // read the security types.
            unsigned char types[255];
            unsigned char sType;
            if(!m_socket.readBytes(types, num)) {
                cerr << "Could not read the security types" << endl;
                return FATAL_ERROR;
            }
            // now decide the security type
            if(!(sType = decideSecurityType(types, num))) {
                cerr << "Could not decide what security to use" << endl;
                return FATAL_ERROR;
            }
            // write the selected security type
            if(!m_socket.writeByte(sType)) {
                return FATAL_ERROR;
            }
            // perform the security
            if(!performSecurityCheck(sType)) {
                // perform the actual security check.
                return FATAL_ERROR;
            }

            uint32 status;
            if(!m_socket.readUint32(status)) {
                cerr << "Could not read the security result message" << endl;
                return FATAL_ERROR;
            }
            // read the status byte
            if(status == 0) {
                // success
                return STATUS_SUCCESS;
            } else if(status == 1) {
                // failed
                cerr << "The security handshaking failed" << endl;
                // read the message that follows
                return FATAL_ERROR;
            } else {
                cerr << "Unknown response to the security handshake, aborting" << endl;
                return FATAL_ERROR;
            }
            return STATUS_SUCCESS;
        }
    }

    /* 
     *
     */
    int RFB::initSecurity3_3()
    {   
        // read the selected security type
        unsigned int type, status;

        if(!m_socket.readUint32(type))
        {
            cerr << "Could not read the security initalization message" << endl;
            return FATAL_ERROR;
        }
        if(performSecurityCheck(type) != STATUS_SUCCESS)
        {
            cerr << "Could not finalize the security handshale" << endl;
            return FATAL_ERROR;
        }
        if(type == AUTH_NONE)
            return STATUS_SUCCESS;

        // read the result
        if(!m_socket.readUint32(status)) {
            cerr << "Failed to read the security status reply" << endl;
            return FATAL_ERROR;
        }
        if(status == 0) {
            return STATUS_SUCCESS;
        } else {
            cerr << "The security handshake failed" << endl;
            return FATAL_ERROR;
        }
    }

    /* Function:        RFB::performSecurityCheck
     * Description:     Performs the actual security check
     */
    int RFB::performSecurityCheck(byte type)
    {
        switch(type)
        {
        case AUTH_NONE: 
                return STATUS_SUCCESS;
        case AUTH_VNC: 
                return authVNC();
        default:
            cerr << "Unknown security type selected, aborting" << endl;
            return FATAL_ERROR;
        }
    }


    /* Function:        RFB::authVNC
     * Description:     VNC Challenge-Response authentication.
     */
    int RFB::authVNC()
    {
        if(!m_bPassword) {
            cerr << "No password supplied for VNC authentication" << endl;
            return FATAL_ERROR;
        }

        byte challenge[16], response[16];
        if(!m_socket.readBytes(challenge, 16)) {
            cerr << "Could not read the VNC authentication challenge from the socket" << endl;
            return FATAL_ERROR;
        }
        // encrypt it using the password and send the response
        if(!desChallenge(challenge, response,(byte *) m_password.c_str(), m_password.length())) {
            cerr << "The VNC authentication attempt failed" << endl;
            return FATAL_ERROR;
        }

        if(!m_socket.writeBytes(response, 16)) {
            cerr << "Could not write the VNC challenge response to the server" << endl;
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }

    /* Function:        RFB::initHandshake
     * Description:     Performs the initalization handshake.
     */
    int RFB::initHandshake()
    {
        // ClientInit, send the shared flag
        if(!m_socket.writeByte(m_sharedFlag)) {
            cerr << "Could not write the shared flag" << endl;
            return FATAL_ERROR;
        }

        // ServerInit.
        unsigned short width, height;
        if(!m_socket.readUint16(width) || !m_socket.readUint16(height)) {
            // read the framebuffer dimensions
            cerr << "Could not read the framebuffer dimensions" << endl;
            return FATAL_ERROR;
        }
        // Read the pixel format
        PIXEL_FORMAT pf;
        if(!m_socket.readPixelFormat(pf)) {
            cerr << "Could not read the pixel format from the ServerInit message" << endl;
            return FATAL_ERROR;
        }

        cout << "Servers display format" << endl;
        cout << "Bits per pixel: " <<  (int) pf.bitsPerPixel << endl;
        cout << "True colour? " << (int) pf.trueColourFlag << endl;


        unsigned int nameLength;
        char * name = NULL;
        if(!m_socket.readUint32(nameLength)) {
            cerr << "Could not read the length of the name field in the ServerInit message" << endl;
            return FATAL_ERROR;
        }
        // allocate space for the name.
        if(!(name = new (std::nothrow) char[nameLength + 1])) {
            cerr << "Could not allocate memory for the name field" << endl;
            return FATAL_ERROR;
        }
        // read the name
        if(!m_socket.readBytes((byte *)name, nameLength)) {
            cerr << "Could not read the name from the field" << endl;
            return FATAL_ERROR;
        }

        if(!m_buffer.init(width, height)) {
            cerr << "Could not initalize the frame buffer" << endl;
            return FATAL_ERROR;
        }

        // The init handshake was a success.
        return STATUS_SUCCESS;
    }

    /*
     *
     */
    int RFB::sendPixelFormat(const PIXEL_FORMAT & format)
    {
        static unsigned char padding [] = {0xDE, 0xAD, 0xC0, 0xDE};
        if(!m_socket.writeByte(0) ||                    // write the type
            !m_socket.writeBytes(padding, 3 * sizeof(char)) ||      // write the padding
            // write the pixel format.
            !m_socket.writePixelFormat(format)) 
        {
            cerr << "Could not write the SetPixelFormat message" << endl;
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }

    /* Function:        RFB::setEncodings
     * Description:     Sets the encodings to use.
     */
    int RFB::setEncodings()
    {
        uint16 num = sizeof(RFB::encodings) / sizeof(int32);
        if(!m_socket.writeByte(2) ||    // set encodings
            !m_socket.writeByte(0) ||   // padding
            !m_socket.writeUint16(num)) // the number of encodings 
        {
            cerr << "Could not write the SetEncodings mesage" << endl;
            return FATAL_ERROR;
        }
        // Now write all the supported encodings
        for(uint16 i = 0;i < num;++i) {
            cout << "Client supports encoding: " << RFB::encodings[i] << endl;
            if(!m_socket.writeInt32(RFB::encodings[i])) {
                cerr << "Could not write the SetEncodings message" << endl;
                return FATAL_ERROR;
            }
        }
        return STATUS_SUCCESS;
    }

    /* Function:        RFB::run
     * Description:
     */
    int RFB::update()
    {
        if(m_socket.readPossible()) {
                readPacket();
        }
        if(!m_bUpdating)
        {
            if(difftime(time(0) , m_lastUpdate) > 0.0) {
                requestUpdate(false);
                m_bUpdating = true;
            }
        }
        return 0;
    }

    /* Function:        RFB::readPacket
     * Description:     Reads a packet.
     */
    int RFB::readPacket()
    {
        unsigned char type;
        if(!m_socket.readByte(type)) {
            cerr << "Could not read the message type" << endl;
            return FATAL_ERROR;
        }
        switch(type)
        {
        case nFrameBufferUpdate:
            {
                // A frame buffer update message
                unsigned char padding;
                unsigned short numRectangles;
                if(!m_socket.readByte(padding) || !m_socket.readUint16(numRectangles)) {
                    cerr << "Could not read the FrameBufferUpdate message" << endl;
                    return FATAL_ERROR;
                }
                // read all the rectangles.
                for(unsigned int i = 0;i < numRectangles;++i) {
                    rectangle rect;
                    if(!m_socket.readUint16(rect.x) ||
                        !m_socket.readUint16(rect.y) ||
                        !m_socket.readUint16(rect.width) ||
                        !m_socket.readUint16(rect.height) ||
                        !m_socket.readInt32(rect.encoding))
                    {
                        cerr << "Could not read the rectangle" << endl;
                        return FATAL_ERROR;
                    }
                    // parse the rectangle data.
                    if(decode(rect) != STATUS_SUCCESS) {
                        cerr << "Could not decode the frame buffer data" << endl;
                        return FATAL_ERROR;
                    }
                }
                // update the screen, but only when something changes.
                //requestUpdate(false);
                return STATUS_SUCCESS;
            }
        case nSetColourMapEntries:
            {
                unsigned char padding;
                unsigned short firstColour, numberOfColours;
                if(!m_socket.readByte(padding) ||
                    !m_socket.readUint16(firstColour) ||
                    !m_socket.readUint16(numberOfColours))
                {
                    cerr << "Could not read the SetColourMapEntries message" << endl;
                    return FATAL_ERROR;
                }
                for(unsigned int i = 0;i < numberOfColours; ++i)
                {
                    
                }
            }

        case nBell:
        case nServerCutText:
            break;
        default:
            cerr << "Unknown packet read" << endl;
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }

    /* Function:        RFB::decode
     * Description:     Decodes the rectangle data.
     */

#define NetUint16(x)        if(!m_socket.readUint16(x))     return FATAL_ERROR;
#define NetUint32(x)        if(!m_socket.readUint32(x))     return FATAL_ERROR;
#define NetInt16(x)         if(!m_socket.readInt16(x))      return FATAL_ERROR;
#define NetInt32(x)         if(!m_socket.readInt32(x))      return FATAL_ERROR;
#define NetBytes(x, len)    if(!m_socket.readBytes(x, len)) return FATAL_ERROR;
#define NetByte(x)          if(!m_socket.readByte(x))       return FATAL_ERROR;
#define NetPixel(dst)       if(!m_socket.readBytes(dst, bytesPerPixel)) return FATAL_ERROR;

    int RFB::decode(const rectangle & rect)
    {
        unsigned int numRectangles;
        unsigned char pixelValue[4];
        rectangle srect;
        unsigned short x, y;
        unsigned int bytesPerPixel = m_buffer.bytesPerPixel();
        unsigned char xyPos, whSize, numRects;
        unsigned char * ptr;

        switch(rect.encoding)
        {
        case 0:     // Raw
            {
                // Raw data, rect.w * rect.h * bytesPerPixel number of data
                if(m_buffer.updateRect(rect, &m_socket) != STATUS_SUCCESS)
                    return FATAL_ERROR;
                break;
            }
        case 1:     // CopyRect
            {
                NetUint16(x)
                NetUint16(y)
                if(m_buffer.blockCopy(x, y, rect.x, rect.y, rect.width, rect.height) != STATUS_SUCCESS)
                    return FATAL_ERROR;
                break;
            }
        case 2:
            {
                NetUint32(numRectangles)
                NetBytes(pixelValue, bytesPerPixel)
                // draws the background colour
                m_buffer.drawFilledRect(rect, pixelValue);
                // now draw the subrectangles.
                for(unsigned int i = 0; i < numRectangles; ++i) {
                    // Draw the subrectangles.
                    NetBytes(pixelValue, bytesPerPixel)
                    NetUint16(srect.x)
                    NetUint16(srect.y)
                    NetUint16(srect.width)
                    NetUint16(srect.height)
                    // Draw the filled rectangle.
                    m_buffer.drawFilledRect(srect, pixelValue);
                }   
            }
        case 5:
            {
                unsigned char typeMask;
                // for all the subrectangles in the width
                // the background and foregroundcolour.
                unsigned char background[4], foreground[4], temp[4];
                unsigned char * subcolor = 0;
                int subrectsColored;
            
                int tileheight, tilewidth;
                for(int tile_y = 0; tile_y < rect.height; tile_y += 16) {
                    tileheight = (((rect.height - tile_y) < 16) ? (rect.height - tile_y) : 16);
                    for(int tile_x = 0;tile_x < rect.width; tile_x += 16)
                    {
                        numRects = 0;
                        tilewidth = (((rect.width - tile_x) < 16) ? (rect.width - tile_x) : 16);
                        // read the type mask
                        NetByte(typeMask);
                        if(typeMask & 1) {
                            rectangle srect(rect.x + tile_x, rect.y + tile_y, tilewidth, tileheight);
                            m_buffer.updateRect(srect, &m_socket);
                        } else {
                            if(typeMask & 2) {
                                NetPixel(background)
                            }
                            if(typeMask & 4) {
                                NetPixel(foreground)
                            }
                            if(typeMask & 8) {
                                NetByte(numRects)
                            }
                            subrectsColored = typeMask & 16;
                                
                            // draw the background
                            rectangle tile_rect(rect.x + tile_x, rect.y + tile_y , tilewidth, tileheight);
                            m_buffer.drawFilledRect(tile_rect, background);

                            // now draw the subrects
                            for(unsigned int i = 0;i < numRects; ++i) {
                                if(subrectsColored) {
                                    NetPixel(temp)
                                    subcolor = temp;
                                } else {
                                    subcolor = foreground;
                                }
                                // draw the subrectangle
                                unsigned char xy, wh;
                                NetByte(xy)
                                NetByte(wh)
                                rectangle srect(tile_rect.x + (xy >> 4),        // x 
                                    tile_rect.y +(xy & 0x0f) ,                  // y
                                    (wh >> 4) + 1,                              // w
                                    (wh & 0x0f) + 1);                           // h
                                // draw the rectangle.
                                m_buffer.drawFilledRect(srect, subcolor);
                            }
                        }
                    }
                }
            }
        }
        // Get the update time
        m_lastUpdate = time(0);
        m_bUpdating = false;
        // return success
        return STATUS_SUCCESS;
    }

    /* Function:        RFB::requestUpdate
     * Description:     Used to request a update of the entire framebuffer.
     */
    int RFB::requestUpdate(bool refresh)
    {
        m_bUpdating = true;
        if(!m_socket.writeByte(3) || // type
            !m_socket.writeByte(refresh ? 0 : 1) ||
            !m_socket.writeUint16(0) || // x position
            !m_socket.writeUint16(0) || // y position
            !m_socket.writeUint16(m_buffer.getWidth()) ||
            !m_socket.writeUint16(m_buffer.getHeight()))
        {
            cerr<< "Could not write the frame buffer update request" << endl;
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }

    /* 
     *
     */
    int RFB::updateMouse(uint16 x, uint16 y, unsigned char flags)
    {
        cerr << "Sending mouse update: " << x << "," << y << endl;
        if(!m_socket.writeByte(5) ||
            !m_socket.writeByte(flags) ||
            !m_socket.writeUint16(x) ||
            !m_socket.writeUint16(y))
        {
            cerr << "Could not write the update mouse event" << endl;
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }

    /*
     *
     */
    int RFB::keyevent(int keysym, bool down)
    {
        if(!m_socket.writeByte(4) ||                // message type
            !m_socket.writeByte(down ? 1 : 0) ||    // downflag
            !m_socket.writeUint16(0) ||             // padding
            !m_socket.writeInt32(keysym))
        {
            return FATAL_ERROR;
        }
        return STATUS_SUCCESS;
    }
};