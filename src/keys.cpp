#include "VNC.h"
#include "SDL.h"
#include "common.h"

/* Function:        TranslateSDLtoX11   
 *
 */
int TranslateSDLtoX11(int keysym)
{
    switch( keysym )
    {
    case SDLK_RETURN:       return 0xFF0D;
    case SDLK_BACKSPACE:    return 0xFF08;
    case SDLK_TAB:          return 0xFF09;
    case SDLK_NUMLOCK:      return 0xFF7F;
    case SDLK_CAPSLOCK:     return 0xFFE5;
    case SDLK_SCROLLOCK:    return 0xFF14;
    case SDLK_RSHIFT:       return 0xFFE2;
    case SDLK_LSHIFT:       return 0xFFE1;
    case SDLK_RCTRL:        return 0xFFE4;
    case SDLK_LCTRL:        return 0xFFE3;
    case SDLK_RALT:         return 0xFFEA;
    case SDLK_LALT:         return 0xFFE9;
    case SDLK_RMETA:        return 0xFFE8;
    case SDLK_LMETA:        return 0xFFE7;
    case SDLK_LSUPER:       return 0xFFEB;
    case SDLK_RSUPER:       return 0xFFEC;
    case SDLK_MODE:         return 0xFF7E;
    case SDLK_COMPOSE:      return 0xFF20;
    case SDLK_INSERT:       return 0xFF63;
    case SDLK_DELETE:       return 0xFFFF;
    case SDLK_HOME:         return 0xFF50;
    case SDLK_END:          return 0xFF57;
    case SDLK_PAGEUP:       return 0xFF55;
    case SDLK_PAGEDOWN:     return 0xFF56;
    case SDLK_UP:           return 0xFF52;
    case SDLK_DOWN:         return 0xFF54;
    case SDLK_LEFT:       return 0xFF51;
    case SDLK_RIGHT:      return 0xFF53;
    case SDLK_F1:         return 0xFFBE;
    case SDLK_F2:         return 0xFFBF;
    case SDLK_F3:         return 0xFFC0;
    case SDLK_F4:         return 0xFFC1;
    case SDLK_F5:         return 0xFFC2;
    case SDLK_F6:         return 0xFFC3;
    case SDLK_F7:         return 0xFFC4;
    case SDLK_F8:         return 0xFFC5;
    case SDLK_F9:         return 0xFFC6;
    case SDLK_F10:        return 0xFFC7;
    case SDLK_F11:        return 0xFFC8;
    case SDLK_F12:        return 0xFFC9;
    case SDLK_F13:          return 0xFFCA;
    case SDLK_F14:          return 0xFFCB;
    case SDLK_F15:          return 0xFFCC;
    case SDLK_AT:           return (uint32) '@';
    case SDLK_DOLLAR:       return (uint32) '$';
    case SDLK_HASH:         return (uint32) '#';
    case SDLK_EXCLAIM:      return (uint32) '!';
    case SDLK_LESS:         return (uint32) '<';
    case SDLK_EQUALS:       return (uint32) '=';
    case SDLK_GREATER:      return (uint32) '>';
    case SDLK_SEMICOLON:    return (uint32) ';';
    case SDLK_COLON:        return (uint32) ':';
    case SDLK_UNDERSCORE:   return (uint32) '_';

    default:
        {
            if(SDL_GetModState() & KMOD_SHIFT)
            {
                if(keysym >= 'a' && keysym <= 'z')
                    keysym += 'A' - 'a';
            }
        }
        return keysym;
    }
}

/*
 *
 */
int CheckKeyCombo(int keysym)
{
    SDLMod mod = SDL_GetModState();
    if(mod & KMOD_LCTRL && mod & KMOD_LALT)
    {
        switch(keysym)
        {
        case SDLK_ESCAPE:   
            return VNC_QUIT;            // ctrl-alt-escape = exit
        case SDLK_F1:
            return VNC_FULLSCREEN;      // ctrl-alt-f1 = fullscreen.
        default:
            return 0;
        }
    }
    return 0;
}