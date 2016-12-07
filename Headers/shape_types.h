#ifndef SHAPETYPES_H
#define SHAPETYPES_H

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
        ACTION_MODULE,
        QUESTION,

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

#endif // SHAPETYPES_H
