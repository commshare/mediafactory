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

/* @remark: 结构体定义 */  
typedef struct  
{  
    int i_size;             // p_data字节数  
    int i_data;             // 当前操作字节的位置  
    unsigned char i_mask;   // 当前操作位的掩码  
    unsigned char *p_data;  // bits buffer  
} bits_buffer_s;  

struct Data_Info_s 
{
    unsigned long long s64CurPts;
    int IFrame;
    unsigned short u16CSeq;
    unsigned int u32Ssrc;
    char szBuff[RTP_MAX_PACKET_BUFF];
};

/*** 
 *@remark:  讲传入的数据按地位一个一个的压入数据 
 *@param :  buffer   [in]  压入数据的buffer 
 *          count    [in]  需要压入数据占的位数 
 *          bits     [in]  压入的数值 
 */  
inline void bits_write(bits_buffer_s *p_buffer, int i_count, unsigned long i_bits);
