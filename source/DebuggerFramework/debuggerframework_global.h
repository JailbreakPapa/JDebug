#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(DEBUGGERFRAMEWORK_LIB)
#  define DEBUGGERFRAMEWORK_EXPORT Q_DECL_EXPORT
# else
#  define DEBUGGERFRAMEWORK_EXPORT Q_DECL_IMPORT
# endif
#else
# define DEBUGGERFRAMEWORK_EXPORT
#endif
