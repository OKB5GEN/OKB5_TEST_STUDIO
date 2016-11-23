#ifndef SHAPETYPES_H
#define SHAPETYPES_H

// List of all DRAKON icons
enum class ShapeTypes
{
    // IMPLEMENTED >>>

    // NOT IMPLEMENTED >>>
    TITLE,
    END,
    ACTION,
    QUESTION,
    CHOICE,
    CASE,
    HEADLINE,
    ADDRESS,
    INSERTION,
    SHELF,
    PARAMS,
    FOR_BEGIN,
    FOR_END,
    OUTPUT,
    INPUT,
    PAUSE,
    DELAY,
    START_TIMER,
    SYNCHRONIZER,
    PARALLEL_PROCESS,
    COMMENT,
    RIGHT_COMMENT,
    LEFT_COMMENT,
    LOOP_ARROW,
    SILHOUETTE_ARROW,
    CONNECTOR,
    CONCURRENT_PROCESS,

    DRAKON_ELEMENTS_COUNT,

    CONNECT_LINE, // no drakon
    ARROW, // no drakon
    VALENCY_POINT, // no drakon

    TOTAL_COUNT // always must be last
};
#endif // SHAPETYPES_H
