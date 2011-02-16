/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

// vim: expandtab

#ifndef QWT_GLOBAL_H
#define QWT_GLOBAL_H

#include <QtGlobal>
#if QT_VERSION < 0x040000
//#include <QModules>
#endif

// QWT_VERSION is (major << 16) + (minor << 8) + patch.

#define QWT_VERSION       0x050202
#define QWT_VERSION_STR   "5.2.2"

#if defined(Q_WS_WIN) || defined(Q_WS_S60)

#if defined(_MSC_VER) /* MSVC Compiler */
/* template-class specialization 'identifier' is already instantiated */
#pragma warning(disable: 4660)
#endif // _MSC_VER

#ifdef QWT_DLL

#if defined(QWT_MAKEDLL)     // create a Qwt DLL library
#define QWT_EXPORT  __declspec(dllexport)
#define QWT_TEMPLATEDLL
#else                        // use a Qwt DLL library
#define QWT_EXPORT  __declspec(dllimport)
#endif

#endif // QWT_DLL

#endif // Q_WS_WIN || Q_WS_S60

#ifndef QWT_EXPORT
#define QWT_EXPORT
#endif

// #define QWT_NO_COMPAT 1 // disable withdrawn functionality

#endif // QWT_GLOBAL_H
