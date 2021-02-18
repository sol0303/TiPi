#include "fonts.h"
#define GB2312_NUM 7614
#define GB2312_EACH_SIZE 24
extern const unsigned char gb2312_12_12[GB2312_NUM][GB2312_EACH_SIZE];

cFONT_extra Font1212CN = {
    &Font12,
    gb2312_12_12,
    sizeof(Font1212CN)/sizeof(char),  /*size of table*/
    16, /* ASCII Width */
    16, /* Width */
    12, /* Height */
};