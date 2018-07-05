#include "rtsptypes.h"

int rtsp_get_error_info(int code, const char** errorinfo)
{
    static const char *reason45x[] = 
    {
        "Parameter Not Understood", // 451
        "Conference Not Found", // 452
        "Not Enough Bandwidth", // 453
        "Session Not Found", // 454
        "Method Not Valid in This State", // 455
        "Header Field Not Valid for Resource", // 456
        "Invalid Range", // 457
        "Parameter Is Read-Only", // 458
        "Aggregate Operation Not Allowed", // 459
        "Only Aggregate Operation Allowed", // 460
        "Unsupported Transport", // 461
        "Destination Unreachable", // 462
    };

    if(451 <= code && code < 451+sizeof(reason45x)/sizeof(reason45x[0]))
        *errorinfo = reason45x[code-451];

    switch(code)
    {
    case 505:
        *errorinfo = "RTSP Version Not Supported";\
        break;
    case 551:
        *errorinfo = "Option not supported";
        break;
    default:
        return -1;
    }

    return 0;
}
