#ifndef SHAPE_TYPES_H
#define SHAPE_TYPES_H

#include <QObject>

class DRAKON: public QObject
{
    Q_OBJECT

public:
    enum IconType
    {
        TERMINATOR,
        BRANCH_BEGIN,
        GO_TO_BRANCH,
        ACTION_MATH,
        DELAY,
        QUESTION,
        ACTION_MODULE,
        SUBPROGRAM,
        OUTPUT,
        PARALLEL_PROCESS,

        // NOT IMPLEMENTED (NOT PLANNED YET)
//        SWITCH,
//        CASE,
//        FOR_BEGIN,
//        FOR_END,
//        INPUT,
//        START_TIMER,
//        SYNCHRONIZER,
//        SHELF,

        SHAPES_COUNT // always must be last
    };

    Q_ENUM(IconType)
};

#endif // SHAPE_TYPES_H
