#ifndef SHAPETYPES_H
#define SHAPETYPES_H

// List of all DRAKON icons
enum class ShapeTypes
{
    // IMPLEMENTED >>>
    TERMINATOR,
    BRANCH_BEGIN,
    GO_TO_BRANCH,
    DELAY,

    // NOT IMPLEMENTED >>>
    ACTION,
    QUESTION,
    CHOICE,
    CASE,
    INSERTION,
    SHELF,
    PARAMS,
    FOR_BEGIN,
    FOR_END,
    OUTPUT,
    INPUT,
    START_TIMER,
    SYNCHRONIZER,
    PARALLEL_PROCESS,
    CONCURRENT_PROCESS,

    TOTAL_COUNT // always must be last
};
#endif // SHAPETYPES_H
