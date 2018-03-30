#ifdef WIN32
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#endif

#include "SDL.h"
#include <string>
#include <sstream>
#include <iostream>
#include "RFB.h"
#include <ctime>
#include "VNC.h"

using namespace std;
string g_host, g_port;  

int TranslateSDLtoX11(int keysym);
int CheckKeyCombo(int keysym);      

/* Function:        parse_args
 * Description:     Parses the command line arguments.
 */
bool parse_args(int argc, char ** argv)
{
    // vnc host port [flags]
    if(argc < 3) {
        cout << "Usage: " << argv[0] << " hostname port [flags]" << endl;
        return false;
    }
    g_host = argv[1];
    g_port = argv[2];
    
    return true;
}


/*
 *
 */
int keyPressed(VNC::RFB & rfb, SDL_Event & ev)
{
    // Translate it to X11
    int keysym = TranslateSDLtoX11(ev.key.keysym.sym);
    return rfb.keyevent(keysym, ev.type == SDL_KEYDOWN);
}

int main(int argc, char **argv)
{
    // parse the command line arguments
    /*if(!parse_args(argc, argv)) {
        return -1;
    }*/
    g_host = "192.168.0.188";
    g_port = "5900";

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,0), &wsaData);
    VNC::RFB rfb_protocol;      // create the rfb instance.
    // now connect to the server
    if(rfb_protocol.connect(g_host.c_str(), g_port.c_str()) != STATUS_SUCCESS)
    {
        cout << "Could not connect to the host" << endl;
        return -1;
    }
    // connected
    if(rfb_protocol.init("apa") != STATUS_SUCCESS)
    {
        cout << "Could not initalize the RFB protocol" << endl;
        return -1;
    }

    // Now get the frame buffer
    VNC::FrameBuffer & fb = rfb_protocol.getFrameBuffer();
    VNC::PIXEL_FORMAT pf = fb.getPixelFormat();

    uint32 width = fb.getWidth(),
        height = fb.getHeight(),
        depth = pf.bitsPerPixel;

    // Initalize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        cerr << "Video initialization failed: " << SDL_GetError() << endl;
        SDL_Quit( );
        return -1;
    }

    // Create the window
    SDL_Surface * screen = SDL_SetVideoMode(width, height, depth, SDL_DOUBLEBUF | SDL_HWSURFACE);
    if(screen == NULL)
    {
        cerr << "Could not create the SDL surface" << endl;
        SDL_Quit();
        return -1;
    }

    // now create the surface for the frame buffer.
    SDL_Surface * render = 
        SDL_CreateRGBSurfaceFrom((void *)fb.getDataPtr(), width, height, 32, width * 4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000);
    
    if(render == NULL) {
        cerr << "Could not create the render surface" << endl;
        SDL_Quit();
        return -1;
    }

    rfb_protocol.requestUpdate(true);       // want everything.
    SDL_Event ev;
    bool bQuit = false;
    while(!bQuit)
    {
        /*
         * The RFB stuff, check for updates.
         */
        if(rfb_protocol.update() != STATUS_SUCCESS)
            break;

        /*
         * The SDL Stuff
         */
        SDL_BlitSurface(render, NULL, screen, NULL);
        SDL_Flip(screen);
        
        /*
         * Handle events
         */
        if( SDL_PollEvent( &ev ))
        {
            switch(ev.type)
            {
            case SDL_KEYDOWN:
                {
                    int combo = CheckKeyCombo(ev.key.keysym.sym);
                    if(combo == VNC_QUIT)
                        bQuit = true;
                    else if(combo == VNC_FULLSCREEN)
                        SDL_WM_ToggleFullScreen(screen);
                    else {
                        if(keyPressed(rfb_protocol, ev) != STATUS_SUCCESS)
                            bQuit = true;
                    }
                }
                break;
            case SDL_KEYUP:
                if(keyPressed(rfb_protocol, ev) != STATUS_SUCCESS)
                        bQuit = true;
                break;
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                    {
                        int mouse_x, mouse_y;
                        int mouse_buttons = SDL_GetMouseState( &mouse_x, &mouse_y );
                        rfb_protocol.updateMouse(mouse_x, mouse_y,0);
                    break;
                    }
            }
        }
        Sleep(1);
    }

    // Shutdown.
    rfb_protocol.close();
    SDL_Quit( );
    return 0;
}