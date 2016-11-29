#ifndef SHAPETYPES_H
#define SHAPETYPES_H

// List of all DRAKON icons
enum class ShapeTypes
{
    DELAY,
    BRANCH_BEGIN,

    // NOT OR PARTIALLY IMPLEMENTED >>>
    TERMINATOR,
    GO_TO_BRANCH,

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
#endif // SHAPETYPES_H
