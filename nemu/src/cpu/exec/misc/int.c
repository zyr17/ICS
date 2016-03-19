#include "cpu/exec/helper.h"

#define DATA_BYTE 2
#include "int-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "int-template.h"
#undef DATA_BYTE

make_helper_v(int_i)


