#ifndef SHAPETYPES_H
#define SHAPETYPES_H

// List of all DRAKON icons
enum class ShapeTypes
{
    // IMPLEMENTED >>>

    // NOT IMPLEMENTED >>>
    TERMINATOR,
    BRANCH_BEGIN,
    GO_TO_BRANCH,
    DELAY,
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
