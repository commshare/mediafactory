#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>

#define PS_HDR_LEN  14  
#define SYS_HDR_LEN 18  
#define PSM_HDR_LEN 20//24  
#define PES_HDR_LEN 19  
#define RTP_HDR_LEN 12  
#define RTP_VERSION 1

#define RTP_MAX_PACKET_BUFF 1300
#define PS_PES_PAYLOAD_SIZE 65535-50

/* @remark: 结构体定义 */  
typedef struct  
{  
    int i_size;             // p_data字节数  
    int i_data;             // 当前操作字节的位置  
    unsigned char i_mask;   // 当前操作位的掩码  
    unsigned char *p_data;  // bits buffer  
} bits_buffer_s;  

/*** 
 *@remark:  讲传入的数据按地位一个一个的压入数据 
 *@param :  buffer   [in]  压入数据的buffer 
 *          count    [in]  需要压入数据占的位数 
 *          bits     [in]  压入的数值 
 */  
void bits_write(bits_buffer_s *p_buffer, int i_count, unsigned long i_bits);
