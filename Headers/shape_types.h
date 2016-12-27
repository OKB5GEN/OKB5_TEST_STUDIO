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

        // NOT OR PARTIALLY IMPLEMENTED >>>
        QUESTION,
        ACTION_MODULE,

        SUBPROGRAM,

        SWITCH,
        CASE,
        FOR_BEGIN,
        FOR_END,
        OUTPUT,
        INPUT,
        START_TIMER,
        SYNCHRONIZER,
        PARALLEL_PROCESS,

        SHELF, // subprogram input (top), subprogram output (bottom)

        SHAPES_COUNT // always must be last
    };

    Q_ENUM(IconType)
};

#endif // SHAPE_TYPES_H
