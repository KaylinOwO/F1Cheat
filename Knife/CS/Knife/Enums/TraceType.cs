namespace Knife.Enums {
    enum TraceType {
        Everything = 0,
        WorldOnly,               // NOTE: This does *not* test static props!!!
        EntitiesOnly,            // NOTE: This version will *not* test static props
        EverythingFilterProps,  // NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
    };
}
