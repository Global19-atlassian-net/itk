//========================================================================
// GLFW - An OpenGL framework
// File:        x11_joystick.c
// Platform:    X11 (Unix)
// API version: 2.6
// WWW:         http://glfw.sourceforge.net
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Camilla Berglund
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "platform.h"


//========================================================================
// Note: Only Linux joystick input is supported at the moment. Other
// systems will behave as if there are no joysticks connected.
//========================================================================

#ifdef linux
#define _GLMWFW_USE_LINUX_JOYSTICKS
#endif // linux



//************************************************************************
//****                  GLMWFW internal functions                       ****
//************************************************************************

#ifdef _GLMWFW_USE_LINUX_JOYSTICKS

//------------------------------------------------------------------------
// Here are the Linux joystick driver v1.x interface definitions that we
// use (we do not want to rely on <linux/joystick.h>):
//------------------------------------------------------------------------

#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

// Joystick event types
#define JS_EVENT_BUTTON     0x01    /* button pressed/released */
#define JS_EVENT_AXIS       0x02    /* joystick moved */
#define JS_EVENT_INIT       0x80    /* initial state of device */

// Joystick event structure
struct js_event {
    unsigned int  time;    /* (u32) event timestamp in milliseconds */
    signed short  value;   /* (s16) value */
    unsigned char type;    /* (u8)  event type */
    unsigned char number;  /* (u8)  axis/button number */
};

// Joystick IOCTL commands
#define JSIOCGVERSION  _IOR('j', 0x01, int)   /* get driver version (u32) */
#define JSIOCGAXES     _IOR('j', 0x11, char)  /* get number of axes (u8) */
#define JSIOCGBUTTONS  _IOR('j', 0x12, char)  /* get number of buttons (u8) */

#endif // _GLMWFW_USE_LINUX_JOYSTICKS


//========================================================================
// _glmwfwInitJoysticks() - Initialize joystick interface
//========================================================================

void _glmwfwInitJoysticks( void )
{
#ifdef _GLMWFW_USE_LINUX_JOYSTICKS
    int  k, n, fd, joy_count;
    char *joy_base_name, joy_dev_name[ 20 ];
    int  driver_version = 0x000800;
    char ret_data;
#endif // _GLMWFW_USE_LINUX_JOYSTICKS
    int  i;

    // Start by saying that there are no sticks
    for( i = 0; i <= GLMWFW_JOYSTICK_LAST; ++ i )
    {
        _glmwfwJoy[ i ].Present = GL_FALSE;
    }

#ifdef _GLMWFW_USE_LINUX_JOYSTICKS

    // Try to open joysticks (nonblocking)
    joy_count = 0;
    for( k = 0; k <= 1 && joy_count <= GLMWFW_JOYSTICK_LAST; ++ k )
    {
        // Pick joystick base name
        switch( k )
        {
        case 0:
            joy_base_name = "/dev/input/js";  // USB sticks
            break;
        case 1:
            joy_base_name = "/dev/js";        // "Legacy" sticks
            break;
        default:
            continue;                         // (should never happen)
        }

        // Try to open a few of these sticks
        for( i = 0; i <= 50 && joy_count <= GLMWFW_JOYSTICK_LAST; ++ i )
        {
            sprintf( joy_dev_name, "%s%d", joy_base_name, i );
            fd = open( joy_dev_name, O_NONBLOCK );
            if( fd != -1 )
            {
                // Remember fd
                _glmwfwJoy[ joy_count ].fd = fd;
    
                // Check that the joystick driver version is 1.0+
                ioctl( fd, JSIOCGVERSION, &driver_version );
                if( driver_version < 0x010000 )
                {
                    // It's an old 0.x interface (we don't support it)
                    close( fd );
                    continue;
                }
    
                // Get number of joystick axes
                ioctl( fd, JSIOCGAXES, &ret_data );
                _glmwfwJoy[ joy_count ].NumAxes = (int) ret_data;
    
                // Get number of joystick buttons
                ioctl( fd, JSIOCGBUTTONS, &ret_data );
                _glmwfwJoy[ joy_count ].NumButtons = (int) ret_data;
    
                // Allocate memory for joystick state
                _glmwfwJoy[ joy_count ].Axis =
                    (float *) ckalloc( sizeof(float) *
                                      _glmwfwJoy[ joy_count ].NumAxes );
                if( _glmwfwJoy[ joy_count ].Axis == NULL )
                {
                    close( fd );
                    continue;
                }
                _glmwfwJoy[ joy_count ].Button =
                    (unsigned char *) ckalloc( sizeof(char) *
                                     _glmwfwJoy[ joy_count ].NumButtons );
                if( _glmwfwJoy[ joy_count ].Button == NULL )
                {
                    ckfree( (char *)_glmwfwJoy[ joy_count ].Axis );
                    close( fd );
                    continue;
                }

                // Clear joystick state
                for( n = 0; n < _glmwfwJoy[ joy_count ].NumAxes; ++ n )
                {
                    _glmwfwJoy[ joy_count ].Axis[ n ] = 0.0f;
                }
                for( n = 0; n < _glmwfwJoy[ joy_count ].NumButtons; ++ n )
                {
                    _glmwfwJoy[ joy_count ].Button[ n ] = GLMWFW_RELEASE;
                }
    
                // The joystick is supported and connected
                _glmwfwJoy[ joy_count ].Present = GL_TRUE;
                joy_count ++;
            }
        }
    }

#endif // _GLMWFW_USE_LINUX_JOYSTICKS

}


//========================================================================
// _glmwfwTerminateJoysticks() - Close all opened joystick handles
//========================================================================

void _glmwfwTerminateJoysticks( void )
{

#ifdef _GLMWFW_USE_LINUX_JOYSTICKS

    int i;

    // Close any opened joysticks
    for( i = 0; i <= GLMWFW_JOYSTICK_LAST; ++ i )
    {
        if( _glmwfwJoy[ i ].Present )
        {
            close( _glmwfwJoy[ i ].fd );
            ckfree( (char *)_glmwfwJoy[ i ].Axis );
            ckfree( (char *)_glmwfwJoy[ i ].Button );
            _glmwfwJoy[ i ].Present = GL_FALSE;
        }
    }

#endif // _GLMWFW_USE_LINUX_JOYSTICKS

}


//========================================================================
// _glmwfwPollJoystickEvents() - Empty joystick event queue
//========================================================================

static void _glmwfwPollJoystickEvents( void )
{

#ifdef _GLMWFW_USE_LINUX_JOYSTICKS

    struct js_event e;
    int    i;

    // Get joystick events for all GLMWFW joysticks
    for( i = 0; i <= GLMWFW_JOYSTICK_LAST; ++ i )
    {
        // Is the stick present?
        if( _glmwfwJoy[ i ].Present )
        {
            // Read all queued events (non-blocking)
            while( read(_glmwfwJoy[i].fd, &e, sizeof(struct js_event)) > 0 )
            {
                // We don't care if it's an init event or not
                e.type &= ~JS_EVENT_INIT;

                // Check event type
                switch( e.type )
                {
                case JS_EVENT_AXIS:
                    _glmwfwJoy[ i ].Axis[ e.number ] = (float) e.value /
                                                             32767.0f;
                    // We need to change the sign for the Y axes, so that
                    // positive = up/forward, according to the GLMWFW spec.
                    if( e.number & 1 )
                    {
                        _glmwfwJoy[ i ].Axis[ e.number ] =
                            -_glmwfwJoy[ i ].Axis[ e.number ];
                    }
                    break;

                case JS_EVENT_BUTTON:
                    _glmwfwJoy[ i ].Button[ e.number ] =
                        e.value ? GLMWFW_PRESS : GLMWFW_RELEASE;
                    break;

                default:
                    break;
                }
            }
        }
    }

#endif // _GLMWFW_USE_LINUX_JOYSTICKS

}


//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

//========================================================================
// _glmwfwPlatformGetJoystickParam() - Determine joystick capabilities
//========================================================================

int _glmwfwPlatformGetJoystickParam( GLMWFWWindow *winPtr, int joy, int param )
{
    // Is joystick present?
    if( !_glmwfwJoy[ joy ].Present )
    {
        return 0;
    }

    switch( param )
    {
    case GLMWFW_PRESENT:
        return GL_TRUE;

    case GLMWFW_AXES:
        return _glmwfwJoy[ joy ].NumAxes;

    case GLMWFW_BUTTONS:
        return _glmwfwJoy[ joy ].NumButtons;

    default:
        break;
    }

    return 0;
}


//========================================================================
// _glmwfwPlatformGetJoystickPos() - Get joystick axis positions
//========================================================================

int _glmwfwPlatformGetJoystickPos( GLMWFWWindow *winPtr, int joy, float *pos, int numaxes )
{
    int       i;

    // Is joystick present?
    if( !_glmwfwJoy[ joy ].Present )
    {
        return 0;
    }

    // Update joystick state
    _glmwfwPollJoystickEvents();

    // Does the joystick support less axes than requested?
    if( _glmwfwJoy[ joy ].NumAxes < numaxes )
    {
        numaxes = _glmwfwJoy[ joy ].NumAxes;
    }

    // Copy axis positions from internal state
    for( i = 0; i < numaxes; ++ i )
    {
        pos[ i ] = _glmwfwJoy[ joy ].Axis[ i ];
    }

    return numaxes;
}


//========================================================================
// _glmwfwPlatformGetJoystickButtons() - Get joystick button states
//========================================================================

int _glmwfwPlatformGetJoystickButtons( GLMWFWWindow *winPtr, int joy, unsigned char *buttons,
    int numbuttons )
{
    int       i;

    // Is joystick present?
    if( !_glmwfwJoy[ joy ].Present )
    {
        return 0;
    }

    // Update joystick state
    _glmwfwPollJoystickEvents();

    // Does the joystick support less buttons than requested?
    if( _glmwfwJoy[ joy ].NumButtons < numbuttons )
    {
        numbuttons = _glmwfwJoy[ joy ].NumButtons;
    }

    // Copy button states from internal state
    for( i = 0; i < numbuttons; ++ i )
    {
        buttons[ i ] = _glmwfwJoy[ joy ].Button[ i ];
    }

    return numbuttons;
}
