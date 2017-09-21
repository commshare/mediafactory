#ifndef __RTPH264_h__
#define __RTPH264_h__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>    

#include "rtp.h"
  
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char NRI:2;  
    unsigned char F:1;      
  
} NALU_HEADER; /**//* 1 BYTES */  
  
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char NRI:2;   
    unsigned char F:1;      
  
  
} FU_INDICATOR; /**//* 1 BYTES */  
  
typedef struct {  
    //byte 0  
    unsigned char TYPE:5;  
    unsigned char R:1;  
    unsigned char E:1;  
    unsigned char S:1;      
} FU_HEADER; /**//* 1 BYTES */        

typedef struct  
{  
    int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)  
    unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)  
    unsigned max_size;            //! Nal Unit Buffer size  
    int forbidden_bit;            //! should be always FALSE  
    int nal_reference_idc;        //! NALU_PRIORITY_xxxx  
    int nal_unit_type;            //! NALU_TYPE_xxxx      
    char *buf;                    //! contains the first byte followed by the EBSP  
    unsigned short lost_packets;  //! true, if packet loss is detected  
} NALU_t;  

typedef int (*DataCallback)(char *data, int length, void *pUserData);

int udp_proc(char *clientIP, int clientport, char *filePath);

int tcp_proc(char *filePath, DataCallback callback, void *pUserData);

#endif