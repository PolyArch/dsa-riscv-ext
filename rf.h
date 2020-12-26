#pragma once

enum DSARF {
#define MACRO(x) x,
#include "./rf.def"
#undef MACRO
};

const char* const REG_NAMES[] = {
#define MACRO(x) #x,
#include "./rf.def"
#undef MACRO
};
