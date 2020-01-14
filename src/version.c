#include <mintpl/version.h>

#define _STR(VAL) #VAL
#define _STRINGIZED(MAJ, MIN, PAT) _STR(MAJ)"."_STR(MIN)"."_STR(PAT)

#define MINTPL_VERSION _STRINGIZED(\
    MINTPL_VERSION_MAJOR,\
    MINTPL_VERSION_MINOR,\
    MINTPL_VERSION_PATCH\
)

const char* mtpl_version() { 
    return MINTPL_VERSION;
}

