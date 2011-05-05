#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef Q_OS_WIN
    #define QMH_IMPORT __declspec(dllimport)
#else
    #define QMH_IMPORT
#endif

#ifdef Q_OS_WIN
#ifdef BUILDING_QMH
    #define QMH_EXPORT __declspec(dllexport)
#else
    #define QMH_EXPORT QMH_IMPORT
#endif
#else
    #define QMH_EXPORT
#endif

Q_DECLARE_METATYPE(QModelIndex)

#endif // GLOBAL_H

