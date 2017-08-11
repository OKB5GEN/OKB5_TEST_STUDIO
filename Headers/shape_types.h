#ifndef SHAPE_TYPES_H
#define SHAPE_TYPES_H

#include <QObject>

class DRAKON: public QObject
{
    Q_OBJECT

public:
    enum IconType
    {
        ACTION_MODULE,
        SUBPROGRAM,
        BRANCH_BEGIN,
        GO_TO_BRANCH,
        SELECT_STATE, // partially DRAKON icon
        CONDITION, // partially DRAKON icon
        CYCLE, // partially DRAKON icon
        ACTION_MATH,
        DELAY,
        OUTPUT,
        PARALLEL_PROCESS,
        TERMINATOR,

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
