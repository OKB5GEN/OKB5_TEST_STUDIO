#ifndef SHAPETYPES_H
#define SHAPETYPES_H

#include <QObject>

class DRAKON: public QObject
{
    Q_OBJECT

public:
    enum IconType
    {
        DELAY,
        BRANCH_BEGIN,
        GO_TO_BRANCH,

        // NOT OR PARTIALLY IMPLEMENTED >>>
        TERMINATOR,

        ACTION,
        QUESTION,
        SWITCH,
        CASE,
        SUBPROGRAM,
        FOR_BEGIN,
        FOR_END,
        OUTPUT,
        INPUT,
        START_TIMER,
        SYNCHRONIZER,
        PARALLEL_PROCESS,

        PARAMS,
        SHELF,

        SHAPES_COUNT // always must be last
    };

    Q_ENUM(IconType)
};

#endif // SHAPETYPES_H
