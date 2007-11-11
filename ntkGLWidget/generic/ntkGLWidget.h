/*
 * ntkGLWidget.h --
 *
 * This file contains definitions for the C-implemeted part of a Tcl
 * interface to the OpenGL library
 *
 * Copyright (c) 2007 by Arnulf P. Wiedemann
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: ntkGLWidget.h,v 1.1.2.1 2007/11/11 10:08:14 wiede Exp $
 */

/*
 * ------------------------------------------------------------------------
 *      PACKAGE:  ntkGLWidget
 *  DESCRIPTION:  Tcl ntk interface to the OpenGL library
 * ------------------------------------------------------------------------
 */

#ifndef NTK_GL_WIDGET_H_INCLUDED
#define NTK_GL_WIDGET_H_INCLUDED

#include <string.h>
#include <ctype.h>
#include <tcl.h>

#ifndef TCL_ALPHA_RELEASE
#   define TCL_ALPHA_RELEASE    0
#endif
#ifndef TCL_BETA_RELEASE
#   define TCL_BETA_RELEASE     1
#endif
#ifndef TCL_FINAL_RELEASE
#   define TCL_FINAL_RELEASE    2
#endif

#define TCL_GL_MAJOR_VERSION      0
#define TCL_GL_MINOR_VERSION      1
#define TCL_GL_RELEASE_LEVEL      TCL_ALPHA_RELEASE
#define TCL_GL_RELEASE_SERIAL     0

#define NTK_GL_WIDGET_VERSION            "0.1"
#define NTK_GL_WIDGET_PATCH_LEVEL        "0.1.0.0"
#define NTK_GL_WIDGET_NAMESPACE          "::ntk::glWidget"


#endif /* NTK_GL_WIDGET_H_INCLUDED */

