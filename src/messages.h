#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#define RFB_CLIENT_SETPIXELFORMAT        0
#define RFB_CLIENT_FIXCOLORMAPENTRIES    1
#define RFB_CLIENT_SETENCODINGS          2
#define RFB_CLIENT_FBUPDATEREQUEST       3
#define RFB_CLIENT_KEYEVENT              4
#define RFB_CLIENT_POINTEREVENT          5
#define RFB_CLIENT_CUTTEXT               6

// Server -> client message types
#define RFB_SERVER_FBUPDATE              0
#define RFB_SERVER_SETCOLORMAPENTRIES    1
#define RFB_SERVER_BELL                  2
#define RFB_SERVER_CUTTEXT               3

#endif