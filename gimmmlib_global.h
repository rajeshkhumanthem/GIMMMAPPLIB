#ifndef GIMMMLIB_GLOBAL_H
#define GIMMMLIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GIMMMLIB_LIBRARY)
#  define GIMMMLIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define GIMMMLIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // GIMMMLIB_GLOBAL_H
