#include<stdio.h>
#include"frame.h"
#include<string.h>

#define __TRACE__	1
#define __DEBUG__	1
#include"debug.h"
#define TAIL_LEN 3
#define FRAME_LEN 30
struct buffer tx_buf;
//struct buffer rx_buf;

u8 device_id_global[DEVICE_LEN] = "UbcdUbcdUbcdUbc";
u8* token;
u8 * data;

int init(void)
{
	tx_buf.first = 0;
	tx_buf.last = 0;
}
/* Set frame,all function return 0 when succeed,return -1 when error*/
int init_frame(struct frame *f)
{
	f->head = 0x55;
	f->expend = EXPEND;
	f->token_len = 0; 
	f->data_len = 0;
	f->length = FRAME_LEN;
	f->tail = 0x55;
	return 0;
}

int set_cmd(struct frame *frame,u8 cmd_type,unsigned char cmd)
{
	frame->cmd_type = cmd_type;
	frame->cmd = cmd;
	return 0;
}

int set_status(struct frame *f,u8 status)
{
	f->status = status;
	return 0;
}

int set_device_id(struct frame *f,u8 * device_id)
{
	if(device_id == NULL){
		return -1;
	}
	memcpy(f->device_id,device_id,DEVICE_LEN);
	return 0;
}

int set_token(struct frame *f,u8* token,u16 len)
{
	if(NULL == token){
		DEBUG("token pointer is NULL\n");
		return -1;
	}
	f->token = malloc(len);
	if(NULL == f->token){
		DEBUG("Malloc error\n");
		return -1;
	}
	memcpy(f->token,token,len);
	f->token_len = len;
	f->length += len;
	return 0;
}
int set_data(struct frame *f,u8* data,u16 len)
{
	if(NULL == data){
		DEBUG("data pointer is NULL\n");
		return -1;
	}
	f->data = malloc(len);
	if(NULL == f->data){
		DEBUG("Malloc error\n");
		return -1;
	}
	memcpy(f->data,data,len);
	f->data_len = len;
	f->length += len;
	return 0;
}

int copy_from_frame(u8* buf,struct frame *f)
{
	int i = 0;
	buf[0] = f->head;
	memcpy(&buf[1],&f->length,2);
	buf[3] = f->cmd_type;
	buf[4] = f->cmd;
	memcpy(&buf[5],&f->cmd_seq,2);
	memcpy(&buf[7],&f->expend,2);
	buf[9] = f->status;
	memcpy(&buf[10],f->device_id,DEVICE_LEN);
	memcpy(&buf[26],&f->token_len,2);
	memcpy(&buf[28],&f->token,f->token_len);
	memcpy(&buf[28 + f->token_len],&f->data_len,2);
	memcpy(&buf[28 + f->token_len + 2],f->data,f->data_len);
	memcpy(&buf[28 + f->token_len +2 + f->data_len],&f->crc,2);
	buf[f->length - 1] = f->tail;
	return 0;
}

int copy_to_frame(struct frame* f,u8* buf)
{
	f->head = buf[0];
	f->length = (u16)buf[1];
	f->cmd_type = buf[3];
	f->cmd = buf[4];
	f->cmd_seq = (u16)buf[5];
	f->expend = (u16)buf[7];
	f->status = buf[9];
	memcpy(f->device_id,&buf[10],DEVICE_LEN);
	f->token_len = (u16)buf[26];
	memcpy(f->token,&buf[28],f->token_len);
	f->data_len = (u16)buf[28 + f->token_len];
	memcpy(f->data,buf[28 + f->token_len + 2],f->data_len);
	f->crc = (u16)buf[28 + f->token_len + 2 + f->data_len];
	f->tail = buf[f->length -1];
	print_frame(f);
	return 0;
}
int set_frame(const void* msg,int len)
{
	if((tx_buf.last + 1)%BUFFER_COUNT == tx_buf.first){
		DEBUG("Send buffer is full!\n");
		return -1;
	}
	struct frame f;
	u8 token[] = "wanghauzhong";
	u8 send_buf[512];
	int i = 0;
	u16 token_length = strlen(token);
	
	memset(send_buf,0,512);
	if(init_frame(&f) != 0){
		DEBUG("init_frame error");
	}
	if(set_status(&f,0x02) != 0){
		DEBUG("set_status error");
	}
	if(set_cmd(&f,0xA0,0x01) != 0){
		DEBUG("set_cmd error");
	}
	if(set_device_id(&f,device_id_global) != 0){
		DEBUG("set_device_id error");
	}
	if(set_token(&f,token,token_length) != 0){
		DEBUG("set_token error");
	}
	if(set_data(&f,(u8 *)msg,len) != 0){
		DEBUG("set_data error");
	}
	f.length += TAIL_LEN;
	//memcpy(&send_buf[FRAME_LEN],token,token_length);
	//memcpy(&send_buf[FRAME_LEN + token_length],msg,len);
	u16 crc = cal_crc16(send_buf,f.length);
	f.crc = crc;
	copy_from_frame(send_buf,&f);
	//memcpy(&send_buf[FRAME_LEN + token_length + len],&crc,2);
	//send_buf[FRAME_LEN + token_length + len + 2] = 0x55;
	print_frame(&f);
	print_buf(send_buf,f.length);
	int send_length = encode(tx_buf.data[tx_buf.last],send_buf);
	print_buf(tx_buf.data[tx_buf.last],send_length);
	tx_buf.last = (tx_buf.last + 1)%BUFFER_COUNT;
	return 0;
}
int send_frame(int s,unsigned int flags)
{
	if(tx_buf.first == tx_buf.last){
		DEBUG("Send buffer is empty!\n");
		return -1;
	}
	u16 length = tx_buf.data[tx_buf.first][1];
	//memcpy(buf,tx_buf[first],length);
	tx_buf.first = (tx_buf.first + 1)%BUFFER_COUNT;
	return send(s,tx_buf.data[tx_buf.first],length,flags);
	
}

/**
 *	encode() will escape string.Except the srring's start and end,
 *	encode() will turn 0x55 to 0x54 0x01,turn 0x54 to 0x54 0x02.
 *	@param	dst is the string after escaping.
 *	@param	src is the string going to escape.
 *	@return return the length of string after escaping.
 */
int encode(u8* dst,u8* src)
{
	if(NULL == src || NULL == dst){
		DEBUG("NULL pointer!\n");
		return 0;
	}
	u16 before_len = (u16)src[1];
	int i = 0,j = 0;
	dst[0] = src[0];
	for(i = 1,j = 1;i < before_len - 1;i++){
		if(src[i] == 0x55){
			dst[j++] = 0x54;
			dst[j++] = 0x01;
		}else if(src[i] == 0x54){
			dst[j++] = 0x54;
			dst[j++] = 0x02;
		}else
			dst[j++] = src[i];
	}
	dst[j++] = src[i];
	return j;
}

/**
 *	decode() will turn the escaping string back.
 *	@param	dst is the string after decode(),is real data.
 *	@param src is the escaping string,is the string needed to decode().
 *	@return return the length of dst string.
 */
int decode(u8* dst,u8* src)
{
	if(NULL == dst || NULL == src){
		return 0;
	}
	int i = 1, j = 1;
	dst[0] = src[0];
	while(src[i] != 0x55){
		if(src[i] == 0x54){
			if(src[i + 1] == 0x01){
				dst[j++] = 0x55;
				i++;
			}
			else if(src[i + 1] == 0x02){
				dst[j++] = 0x54;
				i++;
			}
		}else
			dst[j++] = src[i];
		i++;
	}
	dst[j++] = src[i];
	return j;
}

/**
 *	cal_crc16() compute the crc16.
 *	@param	buf is the string need to be computed CRC16.
 *	@param	len is the length of buf.
 *	@return return the value of CRC16.
 */
u16 cal_crc16(u8 * buf,unsigned int len)
{
	if(NULL == buf)
		return 0;
	u16 crc16 = 0xFFFF;
	int i,j;
	for(i = 0;i<len;i++){
		crc16 ^= *(buf+i);
		for(j = 0;j < 8;j++){
			u16 tmp = crc16 & 0x0001;
			if(tmp){
				crc16 ^= 0xA001;
			}
		}
	}
	return crc16;
}

//Get frame from buf recieved
int get_frame(struct frame *f,u8* buf)
{	
	/*
	if(rx_buf.first == rx_buf.last){
		DEBUG("Receive buffer is empty!\n");
		return -1;
	}*/
	if(NULL == buf)
		return -1;
	u8 token[30];
	u8 data[512];
	u8 buffer[512];
	memset(token,0,30);
	memset(data,0,512);
	memset(buffer,0,512);
	int length = decode(buffer,buf);
	print_buf(buffer,length);
	copy_to_frame(f,buffer);
	u16 crc_get;
	memcpy(&crc_get,&buffer[f->length -3],2);
	u16 crc_cal = cal_crc16(buffer,f->length - 3);
	DEBUG("crc:%x\n",crc_cal);
	if(crc_get != crc_cal)
		DEBUG("crc error!\n");
	//memcpy(token,&buffer[FRAME_LEN],f->token_len);
	//memcpy(data,&buffer[FRAME_LEN + f->token_len],f->data_len);
	print_frame(f);
	return 0;
}

void print_frame(struct frame *f)
{
	printf("head = %x\n",f->head);
	printf("length = %hu\n",f->length);
	printf("command type = %x\n",f->cmd_type);
	printf("command = %x\n",f->cmd);
	printf("command sequence = %hu\n",f->cmd_seq);
	printf("expend = %hu\n",f->expend);
	printf("status = %x\n",f->status);
	printf("device_id is:%s\n",f->device_id);
	printf("token_len = %hu\n",f->token_len);
	printf("token:");
	print_buf(f->token,f->token_len);
	printf("data_len is:%d\n",f->data_len);
	printf("data:");
	print_buf(f->data,f->data_len);
}

void print_buf(u8* buf,u16 len)
{
	int i;
	for(i = 0;i < len;i++)
		printf("%X ",buf[i]);
	printf("\n");
}
