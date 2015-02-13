#ifndef OLOLORD_GLOBAL_H
#define OLOLORD_GLOBAL_H

#include <QtGlobal>

#if defined(OLOLORD_BUILD_LIB)
#   define OLOLORD_EXPORT Q_DECL_EXPORT
#else
#   define OLOLORD_EXPORT Q_DECL_IMPORT
#endif

#endif // OLOLORD_GLOBAL_H
