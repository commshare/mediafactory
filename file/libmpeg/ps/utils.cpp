#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>

#include "utils.h"

/*** 
 *@remark:  讲传入的数据按地位一个一个的压入数据 
 *@param :  buffer   [in]  压入数据的buffer 
 *          count    [in]  需要压入数据占的位数 
 *          bits     [in]  压入的数值 
 */  
inline void bits_write(bits_buffer_s *p_buffer, int i_count, unsigned long i_bits)  
{  
    while (i_count > 0)  
    {  
        i_count--;  
  
        if ((i_bits >> i_count ) & 0x01)  
        {  
            p_buffer->p_data[p_buffer->i_data] |= p_buffer->i_mask;  
        }  
        else  
        {  
            p_buffer->p_data[p_buffer->i_data] &= ~p_buffer->i_mask;  
        }  
        p_buffer->i_mask >>= 1;  
        if (p_buffer->i_mask == 0)  
        {  
            p_buffer->i_data++;  
            p_buffer->i_mask = 0x80;  
        }  
    }  
}

