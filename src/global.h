#ifndef GLOBAL_H
#define GLOBAL_H

#include <QModelIndex>
#include <QMetaType>

#if defined _WIN32
    #ifdef BUILDING_QMH
        #define QMH_EXPORT __declspec(dllexport)
        #define QMH_EXPORT __declspec(dllexport)
    #else
        #define QMH_EXPORT __declspec(dllimport)
        #define QMH_EXPORT __declspec(dllimport)
    #endif
#else
    #ifdef BUILDING_QMH
        #if __GNUC__ >= 4
            #define QMH_EXPORT __attribute__ ((visibility("default")))
        #else
            #define QMH_EXPORT
        #endif
    #else
        #if __GNUC__ >= 4
            #define QMH_EXPORT __attribute__ ((visibility("hidden")))
        #else
            #define QMH_EXPORT
        #endif
    #endif
#endif

Q_DECLARE_METATYPE(QModelIndex)

#endif // GLOBAL_H

