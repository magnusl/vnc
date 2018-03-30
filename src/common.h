#ifndef _COMMON_H_
#define _COMMON_H_

#define STATUS_SUCCESS      0
#define FATAL_ERROR         -1
#define HANDSHAKE_FAILED    -2

#define VNC_INT16_SWAP(x) ((x >> 8) | (x << 8))
#define VNC_INT32_SWAP(x) ((x >> 24) | ((x >> 8) & 0xFF00) | (x << 24)  | ((x << 8) & 0xFF0000))

typedef unsigned char byte;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed int int32;
typedef signed short int16;

#endif