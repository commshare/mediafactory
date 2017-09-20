<http://www.61ic.com/> 	
	您好，欢迎来到61ic！ 	| [登录]
<http://www.61ic.com/User/User_Login.asp> 	[注册]
<http://www.61ic.com/Reg/User_Reg.asp> 	忘记密码
<http://www.61ic.com/User/User_GetPassword.asp> 	| 设为首页 <#> 	帮助
<http://www.61ic.com/code/forumdisplay.php?fid=429>

 . 网站首页 </Index.html> . 业界新闻 </news/Index.html> . 设计中心
</Technology/Index.html> . 移动技术 </Mobile/Index.html> . TI专栏
</Article/Index.html> . ADI专栏 </ADI/Index.html> . FPGA专栏
</FPGA/Index.html> . 代码工厂 <http://www.61ic.com/code> . 官方商城
<http://www.61ic.com.cn> . 
 . 活动专区 <http://www.61ic.com/code/forumdisplay.php?fid=234> . 新品快
递 </Product/Index.html> . 解决方案 </fanga/Index.html> . 前沿科技
</Services/Index.html> . TI资源 </Download/Index.html> . ADI资源
</AdiSoft/Index.html> . FPGA资源 </FPGADownload/Index.html> . 下载中心
</vip/Index.html> . 产品展示 </Shop/Index.html> .  	
	加入收藏 <javascript:window.external.addFavorite('http://www.61ic.com
/','61IC电子在线');>
	付款方式 </Services/Government/Announcement/200712/17338.html>
	联系我们 <mailto:web_61ic@163.com>

	您现在的位置： 61IC电子在线 <http://www.61ic.com/> >> TI专栏
</Article/Index.html> >> TI DaVinci
</Article/DaVinci/Index.html> >> TMS320DM646x SOC
</Article/DaVinci/TMS320DM646x/Index.html> >> 正文

  	*h.264 SPS PPS解析源代码，C实现* 	          ★★★ 	【字体：小
<javascript:fontZoomA();> 大 <javascript:fontZoomB();>】

h.264 SPS PPS解析源代码，C实现
作者：mantis_1… </ShowAuthor.asp?ChannelID=1&
AuthorName=mantis_1984>    文章来源：mantis_1984
</ShowCopyFrom.asp?ChannelID=1&SourceName=mantis_1984>    点击数：    更
新时间：2013-8-8    <javascript:d=document;t=d.selection?
(d.selection.type!='None'?d.selection.createRange().text:''):(d.getSelection?d.getSelection():'');void(vivi=window.open('http://vivi.sina.com.cn/collect/icollect.php?pid=2008&title='+escape(d.title)+'&url='+escape(d.location.href)+'&desc='+escape(t),'vivi','scrollbars=no,width=480,height=480,left=75,top=20,status=no,resizable=yes'));vivi.focus();>


/***************************************************************************************
History:       
1. Date:
Author:
Modification:
2. ...
 ***************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> /* for uint32_t, etc */
#include "sps_pps.h"


/* report level */
#define RPT_ERR (1) // error, system error
#define RPT_WRN (2) // warning, maybe wrong, maybe OK
#define RPT_INF (3) // important information
#define RPT_DBG (4) // debug information


static int rpt_lvl = RPT_WRN; /* report level: ERR, WRN, INF, DBG */


/* report micro */
#define RPT(lvl, ...) \
    do { \
        if(lvl <= rpt_lvl) { \
            switch(lvl) { \
                case RPT_ERR: \
                    fprintf(stderr, "\"%s\" line %d [err]: ", __FILE__,
__LINE__); \
                    break; \
                case RPT_WRN: \
                    fprintf(stderr, "\"%s\" line %d [wrn]: ", __FILE__,
__LINE__); \
                    break; \
                case RPT_INF: \
                    fprintf(stderr, "\"%s\" line %d [inf]: ", __FILE__,
__LINE__); \
                    break; \
                case RPT_DBG: \
                    fprintf(stderr, "\"%s\" line %d [dbg]: ", __FILE__,
__LINE__); \
                    break; \
                default: \
                    fprintf(stderr, "\"%s\" line %d [???]: ", __FILE__,
__LINE__); \
                    break; \
                } \
                fprintf(stderr, __VA_ARGS__); \
                fprintf(stderr, "\n"); \
        } \
    } while(0)


#define MAX_LEN 32


/*读1个bit*/
static int get_1bit(void *h)
{
    get_bit_context *ptr = (get_bit_context *)h;
    int ret = 0;
    uint8_t *cur_char = NULL;
    uint8_t shift;


    if(NULL == ptr)
    {
        RPT(RPT_ERR, "NULL pointer");
        ret = -1;
        goto exit;
    }


    cur_char = ptr->buf + (ptr->bit_pos >> 3);
    shift = 7 - (ptr->cur_bit_pos);
    ptr->bit_pos++;
    ptr->cur_bit_pos = ptr->bit_pos & 0x7;
    ret = ((*cur_char) >> shift) & 0x01;
    
exit:
    return ret;
}


/*读n个bits，n不能超过32*/
static int get_bits(void *h, int n)
{
    get_bit_context *ptr = (get_bit_context *)h;
    uint8_t temp[5] = {0};
    uint8_t *cur_char = NULL;
    uint8_t nbyte;
    uint8_t shift;
    uint32_t result;
    uint64_t ret = 0;


    if(NULL == ptr)
    {
        RPT(RPT_ERR, "NULL pointer");
        ret = -1;
        goto exit;      
    }
    
    if(n > MAX_LEN)
    {  
        n = MAX_LEN;
    }


    if((ptr->bit_pos + n) > ptr->total_bit)
    {
        n = ptr->total_bit- ptr->bit_pos;
    }
        
    cur_char = ptr->buf+ (ptr->bit_pos>>3);
    nbyte = (ptr->cur_bit_pos + n + 7) >> 3;
    shift = (8 - (ptr->cur_bit_pos + n))& 0x07;


    if(n == MAX_LEN)
    {
        RPT(RPT_DBG, "12(ptr->bit_pos(:%d) + n(:%d)) >
ptr->total_bit(:%d)!!! ",\
                ptr->bit_pos, n, ptr->total_bit);
        RPT(RPT_DBG, "0x%x 0x%x 0x%x 0x%x", (*cur_char),
*(cur_char+1),*(cur_char+2),*(cur_char+3));
    }


    memcpy(&temp[5-nbyte], cur_char, nbyte);
    ret = (uint32_t)temp[0] << 24;
    ret = ret << 8;
    ret = ((uint32_t)temp[1]<<24)|((uint32_t)temp[2] << 16)\
                        |((uint32_t)temp[3] << 8)|temp[4];


    ret = (ret >> shift) & (((uint64_t)1<<n) - 1);


    result = ret;
    ptr->bit_pos += n;
    ptr->cur_bit_pos = ptr->bit_pos & 0x7;
    
exit:
    return result;
}


/*指数哥伦布编码解析，参考h264标准第9节*/
static int parse_codenum(void *buf)
{
    uint8_t leading_zero_bits = -1;
    uint8_t b;
    uint32_t code_num = 0;
    
    for(b=0; !b; leading_zero_bits++)
    {
        b = get_1bit(buf);
    }
    
    code_num = ((uint32_t)1 << leading_zero_bits) - 1 + get_bits(buf,
leading_zero_bits);


    return code_num;
}


/*指数哥伦布编码解析 ue*/
static int parse_ue(void *buf)
{
    return parse_codenum(buf);
}


/*指数哥伦布编码解析 se*/
static int parse_se(void *buf)
{   
    int ret = 0;
    int code_num;


    code_num = parse_codenum(buf);
    ret = (code_num + 1) >> 1;
    ret = (code_num & 0x01)? ret : -ret;


    return ret;
}


/*申请的内存释放*/
static void get_bit_context_free(void *buf)
{
    get_bit_context *ptr = (get_bit_context *)buf;


    if(ptr)
    {
        if(ptr->buf)
        {
            free(ptr->buf);
        }


        free(ptr);
    }
}


/*
*调试时总是发现vui.time_scale值特别奇怪，总是16777216，后来查询原因如下:
*
*http://www.cnblogs.com/eustoma/archive/2012/02/13/2415764.html
*  H.264编码时，在每个NAL前添加起始码 0x000001，解码器在码流中检测到起始
码，当前NAL结束。
为了防止NAL内部出现0x000001的数据，h.264又提出'防止竞争 emulation
prevention"机制，
在编码完一个NAL时，如果检测出有连续两个0x00字节，就在后面插入一个0x03。
当解码器在NAL内部检测到0x000003的数据，就把0x03抛弃，恢复原始数据。
   0x000000  >>>>>>  0x00000300
   0x000001  >>>>>>  0x00000301
   0x000002  >>>>>>  0x00000302
   0x000003  >>>>>>  0x00000303
*/
static void *de_emulation_prevention(void *buf)
{
    get_bit_context *ptr = NULL;
    get_bit_context *buf_ptr = (get_bit_context *)buf;
    int i = 0, j = 0;
    uint8_t *tmp_ptr = NULL;
    int tmp_buf_size = 0;
    int val = 0;


    if(NULL == buf_ptr)
    {
        RPT(RPT_ERR, "NULL ptr");
        goto exit;
    }


    ptr = (get_bit_context *)malloc(sizeof(get_bit_context));
    if(NULL == ptr)
    {
        RPT(RPT_ERR, "NULL ptr");
        goto exit;       
    }


    memcpy(ptr, buf_ptr, sizeof(get_bit_context));


    ptr->buf = (uint8_t *)malloc(ptr->buf_size);
    if(NULL == ptr->buf)
    {
        RPT(RPT_ERR, "NULL ptr");
        goto exit;
    }


    memcpy(ptr->buf, buf_ptr->buf, buf_ptr->buf_size);


    tmp_ptr = ptr->buf;
    tmp_buf_size = ptr->buf_size;
    for(i=0; i<(tmp_buf_size-2); i++)
    {
        /*检测0x000003*/
        val = (tmp_ptr[i]^0x00) + (tmp_ptr[i+1]^0x00) + (tmp_ptr[i+2]^0x03);
        if(val == 0)
        {
            /*剔除0x03*/
            for(j=i+2; j<tmp_buf_size-1; j++)
            {
                tmp_ptr[j] = tmp_ptr[j+1];
            }


            /*相应的bufsize要减小*/
            ptr->buf_size--;
        }
    }


    /*重新计算total_bit*/
    ptr->total_bit = ptr->buf_size << 3;
    
    return (void *)ptr;
    
exit:
    get_bit_context_free(ptr);    
    return NULL;
}




/*VUI info*/
static int vui_parameters_set(void *buf, vui_parameters_t *vui_ptr)
{
    int ret = 0;
    int SchedSelIdx = 0;


    if(NULL == vui_ptr || NULL == buf)
    {
        RPT(RPT_ERR,"ERR null pointer\n");
        ret = -1;
        goto exit;
    }


    vui_ptr->aspect_ratio_info_present_flag = get_1bit(buf);
    if(vui_ptr->aspect_ratio_info_present_flag)
    {
        vui_ptr->aspect_ratio_idc = get_bits(buf, 8);
        if(vui_ptr->aspect_ratio_idc == Extended_SAR)
        {
            vui_ptr->sar_width = get_bits(buf, 16);
            vui_ptr->sar_height = get_bits(buf, 16);
        }
    }


    vui_ptr->overscan_info_present_flag = get_1bit(buf);
    if(vui_ptr->overscan_info_present_flag)
    {
        vui_ptr->overscan_appropriate_flag = get_1bit(buf);
    }


    vui_ptr->video_signal_type_present_flag = get_1bit(buf);
    if(vui_ptr->video_signal_type_present_flag)
    {
        vui_ptr->video_format = get_bits(buf, 3);
        vui_ptr->video_full_range_flag = get_1bit(buf);
        
        vui_ptr->colour_description_present_flag = get_1bit(buf);
        if(vui_ptr->colour_description_present_flag)
        {
            vui_ptr->colour_primaries = get_bits(buf, 8);
            vui_ptr->transfer_characteristics = get_bits(buf, 8);
            vui_ptr->matrix_coefficients = get_bits(buf, 8);
        }
    }


    vui_ptr->chroma_loc_info_present_flag = get_1bit(buf);
    if(vui_ptr->chroma_loc_info_present_flag)
    {
        vui_ptr->chroma_sample_loc_type_top_field = parse_ue(buf);
        vui_ptr->chroma_sample_loc_type_bottom_field = parse_ue(buf);
    }


    vui_ptr->timing_info_present_flag = get_1bit(buf);
    if(vui_ptr->timing_info_present_flag)
    {
        vui_ptr->num_units_in_tick = get_bits(buf, 32);
        vui_ptr->time_scale = get_bits(buf, 32);
        vui_ptr->fixed_frame_rate_flag = get_1bit(buf);
    } 


    vui_ptr->nal_hrd_parameters_present_flag = get_1bit(buf);
    if(vui_ptr->nal_hrd_parameters_present_flag)
    {
        vui_ptr->cpb_cnt_minus1 = parse_ue(buf);
        vui_ptr->bit_rate_scale = get_bits(buf, 4);
        vui_ptr->cpb_size_scale = get_bits(buf, 4);


        for(SchedSelIdx=0; SchedSelIdx<=vui_ptr->cpb_cnt_minus1;
SchedSelIdx++)
        {
            vui_ptr->bit_rate_value_minus1[SchedSelIdx] = parse_ue(buf);
            vui_ptr->cpb_size_value_minus1[SchedSelIdx] = parse_ue(buf);
            vui_ptr->cbr_flag[SchedSelIdx] = get_1bit(buf);
        }


        vui_ptr->initial_cpb_removal_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->cpb_removal_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->dpb_output_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->time_offset_length = get_bits(buf, 5);
    }
    


    vui_ptr->vcl_hrd_parameters_present_flag = get_1bit(buf);
    if(vui_ptr->vcl_hrd_parameters_present_flag)
    {
        vui_ptr->cpb_cnt_minus1 = parse_ue(buf);
        vui_ptr->bit_rate_scale = get_bits(buf, 4);
        vui_ptr->cpb_size_scale = get_bits(buf, 4);
        
        for(SchedSelIdx=0; SchedSelIdx<=vui_ptr->cpb_cnt_minus1;
SchedSelIdx++)
        {
            vui_ptr->bit_rate_value_minus1[SchedSelIdx] = parse_ue(buf);
            vui_ptr->cpb_size_value_minus1[SchedSelIdx] = parse_ue(buf);
            vui_ptr->cbr_flag[SchedSelIdx] = get_1bit(buf);
        }


        vui_ptr->initial_cpb_removal_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->cpb_removal_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->dpb_output_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->time_offset_length = get_bits(buf, 5);        
    }


    if(vui_ptr->nal_hrd_parameters_present_flag \
           || vui_ptr->vcl_hrd_parameters_present_flag)
    {
        vui_ptr->low_delay_hrd_flag = get_1bit(buf);
    }


    vui_ptr->pic_struct_present_flag = get_1bit(buf);
    
    vui_ptr->bitstream_restriction_flag = get_1bit(buf);
    if(vui_ptr->bitstream_restriction_flag)
    {
        vui_ptr->motion_vectors_over_pic_boundaries_flag = get_1bit(buf);
        vui_ptr->max_bytes_per_pic_denom = parse_ue(buf);
        vui_ptr->max_bits_per_mb_denom = parse_ue(buf);
        vui_ptr->log2_max_mv_length_horizontal= parse_ue(buf);
        vui_ptr->log2_max_mv_length_vertical = parse_ue(buf);
        vui_ptr->num_reorder_frames = parse_ue(buf);
        vui_ptr->max_dec_frame_buffering = parse_ue(buf);
    }


exit:
    return ret;
}


/*调试信息打印*/
#if 0
/*SPS 信息打印，调试使用*/
static void sps_info_print(SPS* sps_ptr)
{
    if(NULL != sps_ptr)
    {
        RPT(RPT_DBG, "profile_idc: %d", sps_ptr->profile_idc);    
        RPT(RPT_DBG, "constraint_set0_flag: %d",
sps_ptr->constraint_set0_flag); 
        RPT(RPT_DBG, "constraint_set1_flag: %d",
sps_ptr->constraint_set1_flag); 
        RPT(RPT_DBG, "constraint_set2_flag: %d",
sps_ptr->constraint_set2_flag); 
        RPT(RPT_DBG, "constraint_set3_flag: %d",
sps_ptr->constraint_set3_flag); 
        RPT(RPT_DBG, "reserved_zero_4bits: %d",
sps_ptr->reserved_zero_4bits);
        RPT(RPT_DBG, "level_idc: %d", sps_ptr->level_idc); 
        RPT(RPT_DBG, "seq_parameter_set_id: %d",
sps_ptr->seq_parameter_set_id); 
        RPT(RPT_DBG, "chroma_format_idc: %d", sps_ptr->chroma_format_idc);
        RPT(RPT_DBG, "separate_colour_plane_flag: %d",
sps_ptr->separate_colour_plane_flag); 
        RPT(RPT_DBG, "bit_depth_luma_minus8: %d",
sps_ptr->bit_depth_luma_minus8);    
        RPT(RPT_DBG, "bit_depth_chroma_minus8: %d",
sps_ptr->bit_depth_chroma_minus8); 
        RPT(RPT_DBG, "qpprime_y_zero_transform_bypass_flag: %d",
sps_ptr->qpprime_y_zero_transform_bypass_flag); 
        RPT(RPT_DBG, "seq_scaling_matrix_present_flag: %d",
sps_ptr->seq_scaling_matrix_present_flag); 
        //RPT(RPT_INF, "seq_scaling_list_present_flag:%d",
sps_ptr->seq_scaling_list_present_flag); 
        RPT(RPT_DBG, "log2_max_frame_num_minus4: %d",
sps_ptr->log2_max_frame_num_minus4);
        RPT(RPT_DBG, "pic_order_cnt_type: %d", sps_ptr->pic_order_cnt_type);
        RPT(RPT_DBG, "num_ref_frames: %d", sps_ptr->num_ref_frames);
        RPT(RPT_DBG, "gaps_in_frame_num_value_allowed_flag: %d",
sps_ptr->gaps_in_frame_num_value_allowed_flag);
        RPT(RPT_DBG, "pic_width_in_mbs_minus1: %d",
sps_ptr->pic_width_in_mbs_minus1);
        RPT(RPT_DBG, "pic_height_in_map_units_minus1: %d",
sps_ptr->pic_height_in_map_units_minus1); 
        RPT(RPT_DBG, "frame_mbs_only_flag: %d",
sps_ptr->frame_mbs_only_flag);
        RPT(RPT_DBG, "mb_adaptive_frame_field_flag: %d",
sps_ptr->mb_adaptive_frame_field_flag);
        RPT(RPT_DBG, "direct_8x8_inference_flag: %d",
sps_ptr->direct_8x8_inference_flag);
        RPT(RPT_DBG, "frame_cropping_flag: %d",
sps_ptr->frame_cropping_flag);
        RPT(RPT_DBG, "frame_crop_left_offset: %d",
sps_ptr->frame_crop_left_offset);
        RPT(RPT_DBG, "frame_crop_right_offset: %d",
sps_ptr->frame_crop_right_offset);
        RPT(RPT_DBG, "frame_crop_top_offset: %d",
sps_ptr->frame_crop_top_offset);
        RPT(RPT_DBG, "frame_crop_bottom_offset: %d",
sps_ptr->frame_crop_bottom_offset);    
        RPT(RPT_DBG, "vui_parameters_present_flag: %d",
sps_ptr->vui_parameters_present_flag);


        if(sps_ptr->vui_parameters_present_flag)
        {
            RPT(RPT_DBG, "aspect_ratio_info_present_flag: %d",
sps_ptr->vui_parameters.aspect_ratio_info_present_flag);
            RPT(RPT_DBG, "aspect_ratio_idc: %d",
sps_ptr->vui_parameters.aspect_ratio_idc);
            RPT(RPT_DBG, "sar_width: %d",
sps_ptr->vui_parameters.sar_width);
            RPT(RPT_DBG, "sar_height: %d",
sps_ptr->vui_parameters.sar_height);
            RPT(RPT_DBG, "overscan_info_present_flag: %d",
sps_ptr->vui_parameters.overscan_info_present_flag);
            RPT(RPT_DBG, "overscan_info_appropriate_flag: %d",
sps_ptr->vui_parameters.overscan_appropriate_flag);
            RPT(RPT_DBG, "video_signal_type_present_flag: %d",
sps_ptr->vui_parameters.video_signal_type_present_flag);
            RPT(RPT_DBG, "video_format: %d",
sps_ptr->vui_parameters.video_format);
            RPT(RPT_DBG, "video_full_range_flag: %d",
sps_ptr->vui_parameters.video_full_range_flag);
            RPT(RPT_DBG, "colour_description_present_flag: %d",
sps_ptr->vui_parameters.colour_description_present_flag);
            RPT(RPT_DBG, "colour_primaries: %d",
sps_ptr->vui_parameters.colour_primaries);
            RPT(RPT_DBG, "transfer_characteristics: %d",
sps_ptr->vui_parameters.transfer_characteristics);
            RPT(RPT_DBG, "matrix_coefficients: %d",
sps_ptr->vui_parameters.matrix_coefficients);
            RPT(RPT_DBG, "chroma_loc_info_present_flag: %d",
sps_ptr->vui_parameters.chroma_loc_info_present_flag);
            RPT(RPT_DBG, "chroma_sample_loc_type_top_field: %d",
sps_ptr->vui_parameters.chroma_sample_loc_type_top_field);
            RPT(RPT_DBG, "chroma_sample_loc_type_bottom_field: %d",
sps_ptr->vui_parameters.chroma_sample_loc_type_bottom_field);
            RPT(RPT_DBG, "timing_info_present_flag: %d",
sps_ptr->vui_parameters.timing_info_present_flag);
            RPT(RPT_DBG, "num_units_in_tick: %d",
sps_ptr->vui_parameters.num_units_in_tick);
            RPT(RPT_DBG, "time_scale: %d",
sps_ptr->vui_parameters.time_scale);
            RPT(RPT_DBG, "fixed_frame_rate_flag: %d",
sps_ptr->vui_parameters.fixed_frame_rate_flag);
            RPT(RPT_DBG, "nal_hrd_parameters_present_flag: %d",
sps_ptr->vui_parameters.nal_hrd_parameters_present_flag);
            RPT(RPT_DBG, "cpb_cnt_minus1: %d",
sps_ptr->vui_parameters.cpb_cnt_minus1);
            RPT(RPT_DBG, "bit_rate_scale: %d",
sps_ptr->vui_parameters.bit_rate_scale);
            RPT(RPT_DBG, "cpb_size_scale: %d",
sps_ptr->vui_parameters.cpb_size_scale);
            RPT(RPT_DBG, "initial_cpb_removal_delay_length_minus1: %d",
sps_ptr->vui_parameters.initial_cpb_removal_delay_length_minus1);
            RPT(RPT_DBG, "cpb_removal_delay_length_minus1: %d",
sps_ptr->vui_parameters.cpb_removal_delay_length_minus1);
            RPT(RPT_DBG, "dpb_output_delay_length_minus1: %d",
sps_ptr->vui_parameters.dpb_output_delay_length_minus1);
            RPT(RPT_DBG, "time_offset_length: %d",
sps_ptr->vui_parameters.time_offset_length);
            RPT(RPT_DBG, "vcl_hrd_parameters_present_flag: %d",
sps_ptr->vui_parameters.vcl_hrd_parameters_present_flag);
            RPT(RPT_DBG, "low_delay_hrd_flag: %d",
sps_ptr->vui_parameters.low_delay_hrd_flag);
            RPT(RPT_DBG, "pic_struct_present_flag: %d",
sps_ptr->vui_parameters.pic_struct_present_flag);
            RPT(RPT_DBG, "bitstream_restriction_flag: %d",
sps_ptr->vui_parameters.bitstream_restriction_flag);
            RPT(RPT_DBG, "motion_vectors_over_pic_boundaries_flag: %d",
sps_ptr->vui_parameters.motion_vectors_over_pic_boundaries_flag);
            RPT(RPT_DBG, "max_bytes_per_pic_denom: %d",
sps_ptr->vui_parameters.max_bytes_per_pic_denom);
            RPT(RPT_DBG, "max_bits_per_mb_denom: %d",
sps_ptr->vui_parameters.max_bits_per_mb_denom);
            RPT(RPT_DBG, "log2_max_mv_length_horizontal: %d",
sps_ptr->vui_parameters.log2_max_mv_length_horizontal);
            RPT(RPT_DBG, "log2_max_mv_length_vertical: %d",
sps_ptr->vui_parameters.log2_max_mv_length_vertical);
            RPT(RPT_DBG, "num_reorder_frames: %d",
sps_ptr->vui_parameters.num_reorder_frames);
            RPT(RPT_DBG, "max_dec_frame_buffering: %d",
sps_ptr->vui_parameters.max_dec_frame_buffering);
        }
        
    }
}
#endif


/*sps 解析主体，原理参考h264标准
*传入的sps数据要去掉NAL头，即00 00 00 01
*/
int h264dec_seq_parameter_set(void *buf_ptr, SPS *sps_ptr)
{
    SPS *sps = sps_ptr;
    int ret = 0;
    int profile_idc = 0;
    int i,j,last_scale, next_scale, delta_scale;
    void *buf = NULL;


    last_scale = 8;
    next_scale = 8;
    
    if(NULL == buf_ptr || NULL == sps)
    {
        RPT(RPT_ERR,"ERR null pointer\n");
        ret = -1;
        goto exit;
    }
    
    memset((void *)sps, 0, sizeof(SPS));


    buf = de_emulation_prevention(buf_ptr);
    if(NULL == buf)
    {
        RPT(RPT_ERR,"ERR null pointer\n");
        ret = -1;
        goto exit;  
    }
    
    sps->profile_idc          = get_bits(buf, 8);
    sps->constraint_set0_flag = get_1bit(buf);  
    sps->constraint_set1_flag = get_1bit(buf);  
    sps->constraint_set2_flag = get_1bit(buf);  
    sps->constraint_set3_flag = get_1bit(buf); 
    sps->reserved_zero_4bits  = get_bits(buf, 4);
    sps->level_idc            = get_bits(buf, 8);
    sps->seq_parameter_set_id = parse_ue(buf);
 
    profile_idc = sps->profile_idc;
    if( (profile_idc == 100) || (profile_idc == 110) || (profile_idc ==
122) || (profile_idc == 244)
    || (profile_idc  == 44) || (profile_idc == 83) || (profile_idc  ==
86) || (profile_idc == 118) ||\
    (profile_idc == 128))
    {
        sps->chroma_format_idc = parse_ue(buf);
        if(sps->chroma_format_idc == 3)
        {
            sps->separate_colour_plane_flag = get_1bit(buf);
        }


        sps->bit_depth_luma_minus8 = parse_ue(buf);
        sps->bit_depth_chroma_minus8 = parse_ue(buf);
        sps->qpprime_y_zero_transform_bypass_flag = get_1bit(buf);
        
        sps->seq_scaling_matrix_present_flag = get_1bit(buf);
        if(sps->seq_scaling_matrix_present_flag)
        {
            for(i=0; i<((sps->chroma_format_idc != 3)?8:12); i++)
            {
                sps->seq_scaling_list_present_flag[i] = get_1bit(buf);
                if(sps->seq_scaling_list_present_flag[i])
                {
                    if(i<6)
                    {
                        for(j=0; j<16; j++)
                        {
                            if(next_scale != 0)
                            {
                                delta_scale = parse_se(buf);
                                next_scale = (last_scale + delta_scale +
256)%256;
                                sps->UseDefaultScalingMatrix4x4Flag[i] =
((j == 0) && (next_scale == 0));
                            }
                            sps->ScalingList4x4[i][j] = (next_scale ==
0)?last_scale:next_scale;
                            last_scale = sps->ScalingList4x4[i][j];
                        }
                    }
                    else
                    {
                        int ii = i-6;
                        next_scale = 8;
                        last_scale = 8;
                        for(j=0; j<16; j++)
                        {
                            if(next_scale != 0)
                            {
                                delta_scale = parse_se(buf);
                                next_scale = (last_scale + delta_scale +
256)%256;
                                sps->UseDefaultScalingMatrix8x8Flag[ii]
= ((j == 0) && (next_scale == 0));
                            }
                            sps->ScalingList8x8[ii][j] = (next_scale ==
0)?last_scale:next_scale;
                            last_scale = sps->ScalingList8x8[ii][j];
                        }
                    }
                }
            }
        }
    }


    sps->log2_max_frame_num_minus4 = parse_ue(buf);
    sps->pic_order_cnt_type = parse_ue(buf);
    if(sps->pic_order_cnt_type == 0)
    {
        sps->log2_max_pic_order_cnt_lsb_minus4 = parse_ue(buf);
    }
    else if(sps->pic_order_cnt_type == 1)
    {
        sps->delta_pic_order_always_zero_flag = get_1bit(buf);
        sps->offset_for_non_ref_pic = parse_se(buf);
        sps->offset_for_top_to_bottom_field = parse_se(buf);


        sps->num_ref_frames_in_pic_order_cnt_cycle = parse_ue(buf);
        for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            sps->offset_for_ref_frame_array[i] = parse_se(buf);
        }
    }


    sps->num_ref_frames = parse_ue(buf);
    sps->gaps_in_frame_num_value_allowed_flag = get_1bit(buf);
    sps->pic_width_in_mbs_minus1 = parse_ue(buf);
    sps->pic_height_in_map_units_minus1 = parse_ue(buf);
    
    sps->frame_mbs_only_flag = get_1bit(buf);
    if(!sps->frame_mbs_only_flag)
    {
        sps->mb_adaptive_frame_field_flag = get_1bit(buf);
    }


    sps->direct_8x8_inference_flag = get_1bit(buf);
    
    sps->frame_cropping_flag = get_1bit(buf);
    if(sps->frame_cropping_flag)
    {
        sps->frame_crop_left_offset = parse_ue(buf);
        sps->frame_crop_right_offset = parse_ue(buf);
        sps->frame_crop_top_offset = parse_ue(buf);
        sps->frame_crop_bottom_offset = parse_ue(buf);
    }


    sps->vui_parameters_present_flag = get_1bit(buf);
    if(sps->vui_parameters_present_flag)
    {
        vui_parameters_set(buf, &sps->vui_parameters);
    }


    //sps_info_print(sps);
exit:
    get_bit_context_free(buf);
    return ret;
}


/*pps解析，原理参考h264标准?
*/
int h264dec_picture_parameter_set(void *buf_ptr, PPS *pps_ptr)
{
    PPS *pps = pps_ptr;
    int ret = 0;
    void *buf = NULL;
    int iGroup = 0;
    int i = 0;


    if(NULL == buf_ptr || NULL == pps_ptr)
    {
        RPT(RPT_ERR, "NULL pointer\n");
        ret = -1;
        goto exit;
    }


    memset((void *)pps, 0, sizeof(PPS));


    buf = de_emulation_prevention(buf_ptr);
    if(NULL == buf)
    {
        RPT(RPT_ERR,"ERR null pointer\n");
        ret = -1;
        goto exit;  
    }


    pps->pic_parameter_set_id = parse_ue(buf);
    pps->seq_parameter_set_id = parse_ue(buf);
    pps->entropy_coding_mode_flag = get_1bit(buf);
    pps->pic_order_present_flag = get_1bit(buf);
    
    pps->num_slice_groups_minus1 = parse_ue(buf);
    if(pps->num_slice_groups_minus1 > 0)
    {
        pps->slice_group_map_type = parse_ue(buf);
        if(pps->slice_group_map_type == 0)
        {
            for(iGroup=0; iGroup<=pps->num_slice_groups_minus1; iGroup++)
            {
                pps->run_length_minus1[iGroup] = parse_ue(buf);
            }
        }
        else if(pps->slice_group_map_type == 2)
        {
            for(iGroup=0; iGroup<=pps->num_slice_groups_minus1; iGroup++)
            {
                pps->top_left[iGroup] = parse_ue(buf);
                pps->bottom_right[iGroup] = parse_ue(buf);
            }
        }
        else if(pps->slice_group_map_type == 3 \
                ||pps->slice_group_map_type == 4\
                ||pps->slice_group_map_type == 5)
        {
            pps->slice_group_change_direction_flag = get_1bit(buf);
            pps->slice_group_change_rate_minus1 = parse_ue(buf);
        }
        else if(pps->slice_group_map_type == 6)
        {
            pps->pic_size_in_map_units_minus1 = parse_ue(buf);
            for(i=0; i<pps->pic_size_in_map_units_minus1; i++)
            {
                /*这地方可能有问题，对u(v)理解偏差*/
                pps->slice_group_id[i] = get_bits(buf,
pps->pic_size_in_map_units_minus1);
            }
        }
    }


    pps->num_ref_idx_10_active_minus1 = parse_ue(buf);
    pps->num_ref_idx_11_active_minus1 = parse_ue(buf);
    pps->weighted_pred_flag = get_1bit(buf);
    pps->weighted_bipred_idc = get_bits(buf, 2);
    pps->pic_init_qp_minus26 = parse_se(buf); /*relative26*/
    pps->pic_init_qs_minus26 = parse_se(buf); /*relative26*/
    pps->chroma_qp_index_offset = parse_se(buf);
    pps->deblocking_filter_control_present_flag = get_1bit(buf);
    pps->constrained_intra_pred_flag = get_1bit(buf);
    pps->redundant_pic_cnt_present_flag = get_1bit(buf);


    /*pps 解析未完成
    * more_rbsp_data()不知如何实现，时间原因
    * 暂时搁置，没有深入。FIXME: zhaochenhui 20130219
    */
    /*TODO*/


exit:
    get_bit_context_free(buf);
    return ret;
}


/*_*/


前一段时间对sps信息和pps信息分析代码，pps还没有完成，需要可以参考h.264标
准完成。


/***************************************************************************************
History:       
1. Date:
Author:
Modification:
2. ...
 ***************************************************************************************/
#ifndef _sps_pps_H_
#define _sps_pps_H_


/***
 * Sequence parameter set 
 * 可参考H264标准第7节和附录D E
 */
#define Extended_SAR 255
typedef struct vui_parameters{
    int aspect_ratio_info_present_flag; //0 u(1) 
    int aspect_ratio_idc;               //0 u(8) 
    int sar_width;                      //0 u(16) 
    int sar_height;                     //0 u(16) 
    int overscan_info_present_flag;     //0 u(1) 
    int overscan_appropriate_flag;      //0 u(1) 
    int video_signal_type_present_flag; //0 u(1) 
    int video_format;                   //0 u(3) 
    int video_full_range_flag;          //0 u(1) 
    int colour_description_present_flag; //0 u(1) 
    int colour_primaries;                //0 u(8) 
    int transfer_characteristics;        //0 u(8) 
    int matrix_coefficients;             //0 u(8) 
    int chroma_loc_info_present_flag;     //0 u(1) 
    int chroma_sample_loc_type_top_field;  //0 ue(v) 
    int chroma_sample_loc_type_bottom_field; //0 ue(v) 
    int timing_info_present_flag;          //0 u(1) 
    uint32_t num_units_in_tick;           //0 u(32) 
    uint32_t time_scale;                 //0 u(32) 
    int fixed_frame_rate_flag;           //0 u(1) 
    int nal_hrd_parameters_present_flag; //0 u(1)
    int cpb_cnt_minus1;                 //0 ue(v)
    int bit_rate_scale;                 //0 u(4)
    int cpb_size_scale;                 //0 u(4)
    int bit_rate_value_minus1[16];      //0 ue(v)
    int cpb_size_value_minus1[16];      //0 ue(v)
    int cbr_flag[16];                   //0 u(1)
    int initial_cpb_removal_delay_length_minus1; //0 u(5)
    int cpb_removal_delay_length_minus1;         //0 u(5)
    int dpb_output_delay_length_minus1;         //0 u(5)
    int time_offset_length;                      //0 u(5)
    int vcl_hrd_parameters_present_flag;         //0 u(1)
    int low_delay_hrd_flag;                      //0 u(1)
    int pic_struct_present_flag;                 //0 u(1)
    int bitstream_restriction_flag;              //0 u(1)
    int motion_vectors_over_pic_boundaries_flag;  //0 ue(v)
    int max_bytes_per_pic_denom;                  //0 ue(v)
    int max_bits_per_mb_denom;                    //0 ue(v)
    int log2_max_mv_length_horizontal;            //0 ue(v)
    int log2_max_mv_length_vertical;              //0 ue(v)
    int num_reorder_frames;                       //0 ue(v)
    int max_dec_frame_buffering;                  //0 ue(v)
}vui_parameters_t;
 
typedef struct SPS{
    int profile_idc;
    int constraint_set0_flag;
    int constraint_set1_flag;
    int constraint_set2_flag;
    int constraint_set3_flag;
    int reserved_zero_4bits;
    int level_idc;
    int seq_parameter_set_id; //ue(v)
    int chroma_format_idc; //ue(v)
    int separate_colour_plane_flag; //u(1)
    int bit_depth_luma_minus8; //0 ue(v) 
    int bit_depth_chroma_minus8; //0 ue(v) 
    int qpprime_y_zero_transform_bypass_flag; //0 u(1) 
    int seq_scaling_matrix_present_flag; //0 u(1)
    int seq_scaling_list_present_flag[12];
    int UseDefaultScalingMatrix4x4Flag[6];
    int UseDefaultScalingMatrix8x8Flag[6];
    int ScalingList4x4[6][16];
    int ScalingList8x8[6][64];
    int log2_max_frame_num_minus4; //0 ue(v)
    int pic_order_cnt_type; //0 ue(v)
    int log2_max_pic_order_cnt_lsb_minus4; //
    int delta_pic_order_always_zero_flag;           //u(1)
    int offset_for_non_ref_pic;                     //se(v)
    int offset_for_top_to_bottom_field;            //se(v)
    int num_ref_frames_in_pic_order_cnt_cycle;    //ue(v) 
    int offset_for_ref_frame_array[16];           //se(v)
    int num_ref_frames;                           //ue(v)
    int gaps_in_frame_num_value_allowed_flag;    //u(1)
    int pic_width_in_mbs_minus1;                //ue(v)
    int pic_height_in_map_units_minus1;         //u(1)
    int frame_mbs_only_flag;                  //0 u(1) 
    int mb_adaptive_frame_field_flag;           //0 u(1) 
    int direct_8x8_inference_flag;              //0 u(1) 
    int frame_cropping_flag;                    //u(1)
    int frame_crop_left_offset;                //ue(v)
    int frame_crop_right_offset;                //ue(v)
    int frame_crop_top_offset;                  //ue(v)
    int frame_crop_bottom_offset;            //ue(v)
    int vui_parameters_present_flag;            //u(1)
    vui_parameters_t vui_parameters;
}SPS;


/***
 * Picture parameter set
 */
typedef struct PPS{
    int pic_parameter_set_id;
    int seq_parameter_set_id;
    int entropy_coding_mode_flag;
    int pic_order_present_flag;
    int num_slice_groups_minus1;
    int slice_group_map_type;
    int run_length_minus1[32];
    int top_left[32];
    int bottom_right[32];
    int slice_group_change_direction_flag;
    int slice_group_change_rate_minus1;
    int pic_size_in_map_units_minus1;
    int slice_group_id[32];
    int num_ref_idx_10_active_minus1;
    int num_ref_idx_11_active_minus1;
    int weighted_pred_flag;
    int weighted_bipred_idc;
    int pic_init_qp_minus26;
    int pic_init_qs_minus26;
    int chroma_qp_index_offset;
    int deblocking_filter_control_present_flag;
    int constrained_intra_pred_flag;
    int redundant_pic_cnt_present_flag;
    int transform_8x8_mode_flag;
    int pic_scaling_matrix_present_flag;
    int pic_scaling_list_present_flag[32];
    int second_chroma_qp_index_offset;
    int scaling_list_4x4[6][16];
    int scaling_list_8x8[2][64];
}PPS;


typedef struct get_bit_context{
    uint8_t *buf;         /*指向SPS start*/
    int      buf_size;    /*SPS 长度*/
    int     bit_pos;      /*bit已读取位置*/
    int     total_bit;    /*bit总长度*/
    int     cur_bit_pos;  /*当前读取位置*/
}get_bit_context;


/***********************************************************************
 * function: h264dec_seq_parameter_set()
 * description:
 *   fenxi h264 sps infomation
 * parameters:
 *   *buf :     buf ptr, 需同步00 00 00 01 X7后传入
 *   *sps_ptr:  sps指针，保存SPS信息
 * 
 * return:
 *   0K 0，ERR -1
 * byproduct:
 *   none
 * note:
 *   none
 ***********************************************************************/
int h264dec_seq_parameter_set(void *buf, SPS *sps_ptr);




/***********************************************************************
 * function: h264dec_picture_parameter_set()
 * description:
 *   fenxi h264 pps infomation
 * parameters:
 *   *buf :     buf ptr, 需同步00 00 00 01 X8后传入
 *   *pps_ptr:  pps指针，保存PPS信息
 * 
 * return:
 *   0K 0，ERR -1
 * byproduct:
 *   none
 * note:
 *   由于时间原因暂时未调试，对more_rbsp_data()未实现。目前PPS解析不可用
 ***********************************************************************/
int h264dec_picture_parameter_set(void *buf, PPS *pps_ptr);


#endif
/*_*/

文章录入：admin    责任编辑：admin 
# 	上一篇文章： Libosip/libeXosip 交叉编译以及在DM6467的使用
</Article/DaVinci/TMS320DM646x/201308/49911.html>

# 下一篇文章： 没有了
【发表评论 </Article/Comment.asp?ArticleID=49912>】【加入收藏
</User/User_Favorite.asp?Action=Add&ChannelID=1&InfoID=49912>】【告诉好
友 </Article/SendMail.asp?ArticleID=49912>】【打印此文
</Article/Print.asp?ArticleID=49912>】【关闭窗口
<javascript:window.close();>】

*最新热点* 		*最新推荐* 		*相关文章*
		Libosip/libeXosip 交叉编译…
</Article/DaVinci/TMS320DM646x/201308/49911.html>
TI C6000系列DSP之CCS5.2编译… </Article/C6000/C64X/201306/49203.html>
TMS320C6000系列DSP之硬件系… </Article/C6000/C64X/201306/49202.html>
TMS320C6000系列DSP之芯片支… </Article/C6000/C64X/201306/49201.html>
TMS320C6000系列DSP之CCS5.2… </Article/C6000/C64X/201306/49200.html>
嵌入式DSP应用的低功耗优化策… </Article/DaVinci/DM644X/200709/16261.html>
在DM642上开发嵌入式的网络视… </Article/C6000/C64X/200605/4777.html>
嵌入式DSP上的视频编解码 </Article/C6000/C64X/200604/4647.html>

　　*网友评论：*（只显示最新10条。评论内容只代表网友观点，与本站立场无
关！）

关于我们
<http://www.61ic.com/Services/Government/Announcement/201207/43465.html>
| 联系方式
<http://www.61ic.com/Services/Government/Announcement/201207/43463.html>
| 友情链接 </FriendSite/Index.asp> | 版权申明 </Copyright.asp> | 商标注
册证
<http://www.61ic.com/Services/Government/Announcement/201207/43466.html>
	<http://www.hngawj.net:80/><http://www.hngawj.net:80/public/record/welcome.jsp?num=4302000100001> 站长：61 湘ICP备13001086号-2 <mailto:web_61ic@163.com><http://www.cnzz.com/stat/website.php?web_id=2255655> 	

