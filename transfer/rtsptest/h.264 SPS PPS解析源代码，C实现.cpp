<http://www.61ic.com/> 	
	���ã���ӭ����61ic�� 	| [��¼]
<http://www.61ic.com/User/User_Login.asp> 	[ע��]
<http://www.61ic.com/Reg/User_Reg.asp> 	��������
<http://www.61ic.com/User/User_GetPassword.asp> 	| ��Ϊ��ҳ <#> 	����
<http://www.61ic.com/code/forumdisplay.php?fid=429>

 . ��վ��ҳ </Index.html> . ҵ������ </news/Index.html> . �������
</Technology/Index.html> . �ƶ����� </Mobile/Index.html> . TIר��
</Article/Index.html> . ADIר�� </ADI/Index.html> . FPGAר��
</FPGA/Index.html> . ���빤�� <http://www.61ic.com/code> . �ٷ��̳�
<http://www.61ic.com.cn> . 
 . �ר�� <http://www.61ic.com/code/forumdisplay.php?fid=234> . ��Ʒ��
�� </Product/Index.html> . ������� </fanga/Index.html> . ǰ�ؿƼ�
</Services/Index.html> . TI��Դ </Download/Index.html> . ADI��Դ
</AdiSoft/Index.html> . FPGA��Դ </FPGADownload/Index.html> . ��������
</vip/Index.html> . ��Ʒչʾ </Shop/Index.html> .  	
	�����ղ� <javascript:window.external.addFavorite('http://www.61ic.com
/','61IC��������');>
	���ʽ </Services/Government/Announcement/200712/17338.html>
	��ϵ���� <mailto:web_61ic@163.com>

	�����ڵ�λ�ã� 61IC�������� <http://www.61ic.com/> >> TIר��
</Article/Index.html> >> TI DaVinci
</Article/DaVinci/Index.html> >> TMS320DM646x SOC
</Article/DaVinci/TMS320DM646x/Index.html> >> ����

  	*h.264 SPS PPS����Դ���룬Cʵ��* 	          ���� 	�����壺С
<javascript:fontZoomA();> �� <javascript:fontZoomB();>��

h.264 SPS PPS����Դ���룬Cʵ��
���ߣ�mantis_1�� </ShowAuthor.asp?ChannelID=1&
AuthorName=mantis_1984>    ������Դ��mantis_1984
</ShowCopyFrom.asp?ChannelID=1&SourceName=mantis_1984>    �������    ��
��ʱ�䣺2013-8-8    <javascript:d=document;t=d.selection?
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


/*��1��bit*/
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


/*��n��bits��n���ܳ���32*/
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


/*ָ�����ײ�����������ο�h264��׼��9��*/
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


/*ָ�����ײ�������� ue*/
static int parse_ue(void *buf)
{
    return parse_codenum(buf);
}


/*ָ�����ײ�������� se*/
static int parse_se(void *buf)
{   
    int ret = 0;
    int code_num;


    code_num = parse_codenum(buf);
    ret = (code_num + 1) >> 1;
    ret = (code_num & 0x01)? ret : -ret;


    return ret;
}


/*������ڴ��ͷ�*/
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
*����ʱ���Ƿ���vui.time_scaleֵ�ر���֣�����16777216��������ѯԭ������:
*
*http://www.cnblogs.com/eustoma/archive/2012/02/13/2415764.html
*  H.264����ʱ����ÿ��NALǰ�����ʼ�� 0x000001���������������м�⵽��ʼ
�룬��ǰNAL������
Ϊ�˷�ֹNAL�ڲ�����0x000001�����ݣ�h.264�����'��ֹ���� emulation
prevention"���ƣ�
�ڱ�����һ��NALʱ�������������������0x00�ֽڣ����ں������һ��0x03��
����������NAL�ڲ���⵽0x000003�����ݣ��Ͱ�0x03�������ָ�ԭʼ���ݡ�
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
        /*���0x000003*/
        val = (tmp_ptr[i]^0x00) + (tmp_ptr[i+1]^0x00) + (tmp_ptr[i+2]^0x03);
        if(val == 0)
        {
            /*�޳�0x03*/
            for(j=i+2; j<tmp_buf_size-1; j++)
            {
                tmp_ptr[j] = tmp_ptr[j+1];
            }


            /*��Ӧ��bufsizeҪ��С*/
            ptr->buf_size--;
        }
    }


    /*���¼���total_bit*/
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


/*������Ϣ��ӡ*/
#if 0
/*SPS ��Ϣ��ӡ������ʹ��*/
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


/*sps �������壬ԭ��ο�h264��׼
*�����sps����Ҫȥ��NALͷ����00 00 00 01
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


/*pps������ԭ��ο�h264��׼?
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
                /*��ط����������⣬��u(v)���ƫ��*/
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


    /*pps ����δ���
    * more_rbsp_data()��֪���ʵ�֣�ʱ��ԭ��
    * ��ʱ���ã�û�����롣FIXME: zhaochenhui 20130219
    */
    /*TODO*/


exit:
    get_bit_context_free(buf);
    return ret;
}


/*_*/


ǰһ��ʱ���sps��Ϣ��pps��Ϣ�������룬pps��û����ɣ���Ҫ���Բο�h.264��
׼��ɡ�


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
 * �ɲο�H264��׼��7�ں͸�¼D E
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
    uint8_t *buf;         /*ָ��SPS start*/
    int      buf_size;    /*SPS ����*/
    int     bit_pos;      /*bit�Ѷ�ȡλ��*/
    int     total_bit;    /*bit�ܳ���*/
    int     cur_bit_pos;  /*��ǰ��ȡλ��*/
}get_bit_context;


/***********************************************************************
 * function: h264dec_seq_parameter_set()
 * description:
 *   fenxi h264 sps infomation
 * parameters:
 *   *buf :     buf ptr, ��ͬ��00 00 00 01 X7����
 *   *sps_ptr:  spsָ�룬����SPS��Ϣ
 * 
 * return:
 *   0K 0��ERR -1
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
 *   *buf :     buf ptr, ��ͬ��00 00 00 01 X8����
 *   *pps_ptr:  ppsָ�룬����PPS��Ϣ
 * 
 * return:
 *   0K 0��ERR -1
 * byproduct:
 *   none
 * note:
 *   ����ʱ��ԭ����ʱδ���ԣ���more_rbsp_data()δʵ�֡�ĿǰPPS����������
 ***********************************************************************/
int h264dec_picture_parameter_set(void *buf, PPS *pps_ptr);


#endif
/*_*/

����¼�룺admin    ���α༭��admin 
# 	��һƪ���£� Libosip/libeXosip ��������Լ���DM6467��ʹ��
</Article/DaVinci/TMS320DM646x/201308/49911.html>

# ��һƪ���£� û����
���������� </Article/Comment.asp?ArticleID=49912>���������ղ�
</User/User_Favorite.asp?Action=Add&ChannelID=1&InfoID=49912>�������ߺ�
�� </Article/SendMail.asp?ArticleID=49912>������ӡ����
</Article/Print.asp?ArticleID=49912>�����رմ���
<javascript:window.close();>��

*�����ȵ�* 		*�����Ƽ�* 		*�������*
		Libosip/libeXosip ������롭
</Article/DaVinci/TMS320DM646x/201308/49911.html>
TI C6000ϵ��DSP֮CCS5.2���롭 </Article/C6000/C64X/201306/49203.html>
TMS320C6000ϵ��DSP֮Ӳ��ϵ�� </Article/C6000/C64X/201306/49202.html>
TMS320C6000ϵ��DSP֮оƬ֧�� </Article/C6000/C64X/201306/49201.html>
TMS320C6000ϵ��DSP֮CCS5.2�� </Article/C6000/C64X/201306/49200.html>
Ƕ��ʽDSPӦ�õĵ͹����Ż��ߡ� </Article/DaVinci/DM644X/200709/16261.html>
��DM642�Ͽ���Ƕ��ʽ�������ӡ� </Article/C6000/C64X/200605/4777.html>
Ƕ��ʽDSP�ϵ���Ƶ����� </Article/C6000/C64X/200604/4647.html>

����*�������ۣ�*��ֻ��ʾ����10������������ֻ�������ѹ۵㣬�뱾վ������
�أ���

��������
<http://www.61ic.com/Services/Government/Announcement/201207/43465.html>
| ��ϵ��ʽ
<http://www.61ic.com/Services/Government/Announcement/201207/43463.html>
| �������� </FriendSite/Index.asp> | ��Ȩ���� </Copyright.asp> | �̱�ע
��֤
<http://www.61ic.com/Services/Government/Announcement/201207/43466.html>
	<http://www.hngawj.net:80/><http://www.hngawj.net:80/public/record/welcome.jsp?num=4302000100001> վ����61 ��ICP��13001086��-2 <mailto:web_61ic@163.com><http://www.cnzz.com/stat/website.php?web_id=2255655> 	

