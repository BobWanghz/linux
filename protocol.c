#include<stdio.h>
#include"frame.h"
#include<string.h>

#define __DEBUG__ 1
#include"debug.h"
#define TAIL_LEN 3
#define FRAME_LEN 30

u8 device_id_global[DEVICE_LEN] = "UbcdUbcdUbcdUbc";
u8* token;
u8 * data;
/* Set frame,all function return 0 when succeed,return -1 when error*/
int init_frame(struct frame_head *frame)
{
	frame->head = HEAD;
	frame->expend = EXPEND;
	frame->token_len = 0; 
	frame->data_len = 0;
	frame->length = FRAME_LEN;
	return 0;
}

int set_cmd(struct frame_head *frame,u8 cmd_type,unsigned char cmd)
{
	frame->cmd_type = cmd_type;
	frame->cmd = cmd;
	return 0;
}

int set_status(struct frame_head *frame,u8 status)
{
	frame->status = status;
	return 0;
}

int set_device_id(struct frame_head *frame,u8 * device_id)
{
	if(device_id == NULL){
		return -1;
	}
	memcpy(frame->device_id,device_id,DEVICE_LEN);
	return 0;
}
int copy_from_frame(u8* buf,struct frame_head *frame)
{
	int i = 0;
	buf[0] = frame->head;
	memcpy(&buf[1],&frame->length,2);
	buf[3] = frame->cmd_type;
	buf[4] = frame->cmd;
	memcpy(&buf[5],&frame->cmd_seq,2);
	memcpy(&buf[7],&frame->expend,2);
	buf[9] = frame->status;
	memcpy(&buf[10],frame->device_id,DEVICE_LEN);
	memcpy(&buf[26],&frame->token_len,2);
	memcpy(&buf[28],&frame->data_len,2);
	return 0;
}

int copy_to_frame(struct frame_head* f,u8* buf)
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
	f->data_len = (u16)buf[28];
	print_frame(f);
	return 0;
}
int send_frame(int s,const void* msg,int len,unsigned int flags)
{
	struct frame_head frame;
	u8 token[] = "wanghauzhong";
	u8 send_buf[512];
	u8 buffer[512];
	int i = 0;
	u16 token_length = strlen(token);

	memset(send_buf,0,512);
	memset(buffer,0,512);
	if(init_frame(&frame) != 0){
		DEBUG("init_frame error");
	}
	if(set_status(&frame,0x02) != 0){
		DEBUG("set_status error");
	}
	if(set_cmd(&frame,0xA0,0x01) != 0){
		DEBUG("set_cmd error");
	}
	if(set_device_id(&frame,device_id_global) != 0){
		DEBUG("set_device_id error");
	}
	frame.data_len += len;
	frame.token_len += token_length;
	frame.length += len;
	frame.length += token_length;
	frame.length += TAIL_LEN;
	copy_from_frame(send_buf,&frame);
	//memcpy(send_buf,(char *)&frame,FRAME_LEN);
	memcpy(&send_buf[FRAME_LEN],token,token_length);
	memcpy(&send_buf[FRAME_LEN + token_length],msg,len);
	u16 crc = cal_crc16(send_buf,frame.length);
	memcpy(&send_buf[FRAME_LEN + token_length + len],&crc,2);
	send_buf[FRAME_LEN + token_length + len + 2] = 0x55;
	print_frame(&frame);
	printf("token:%s\n",token);
	printf("data:%s\n",msg);
	printf("crc:%x\n",crc);
	printf("tail:%x\n",0x55);
	for(i = 0;i < frame.length;++i)
		printf("%x ",send_buf[i]);
	printf("\n");
	int send_length = encode(buffer,send_buf);
	for(i = 0;i < send_length;i++){
		printf("%x ",buffer[i]);
	}
	printf("\n");
	DEBUG("send_length = %d\n",send_length);
	return send(s,buffer,send_length,flags);
}

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

/*
int get_length(u8* buf)		//get the length after encode but decode before
{
	if(NULL == buf){
		DEBUG("NULL pointer error!\n");
		return 0;
	}
	int i = 1;
	int buf_len = 2;
	while(buf[i] != 0x55){
		buf_len ++;
	}
	return buf_len;
}
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
//crc16
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

//Get frame_head from buf recieved
int get_frame(struct frame_head *frame,u8* buf)
{
	if(NULL == buf)
		return -1;
	u8 token[30];
	u8 data[512];
	u8 buffer[512];
	memset(token,0,30);
	memset(data,0,512);
	memset(buffer,0,512);
	int length = decode(buffer,buf);
	int i = 0;
	for(i = 0;i < length;i++)
		printf("%x ",buffer[i]);
	printf("\n");
	copy_to_frame(frame,buffer);
	printf("\n");
	u16 crc_get;
	memcpy(&crc_get,&buffer[frame->length -3],2);
	u16 crc_cal = cal_crc16(buffer,frame->length - 3);
	DEBUG("crc:%x\n",crc_cal);
	if(crc_get != crc_cal)
		DEBUG("crc error!\n");
	memcpy(token,&buffer[FRAME_LEN],frame->token_len);
	memcpy(data,&buffer[FRAME_LEN + frame->token_len],frame->data_len);
	printf("token:%s\n",token);
	printf("data:%s\n",data);
	u8 tail = buffer[frame->length - 1];
	printf("tail:%x\n",tail);
}

void print_frame(struct frame_head *frame)
{
	printf("head = %x\n",frame->head);
	printf("length = %hu\n",frame->length);
	printf("command type = %x\n",frame->cmd_type);
	printf("command = %x\n",frame->cmd);
	printf("command sequence = %hu\n",frame->cmd_seq);
	printf("expend = %hu\n",frame->expend);
	printf("status = %x\n",frame->status);
	printf("device_id is:%s\n",frame->device_id);
	printf("token_len = %hu\n",frame->token_len);
	printf("data_len is:%d\n",frame->data_len);
}

/*
int gateway(struct frame_head *frame)
{
	u8 cmd_type = frame->cmd_type;
	if(cmd_type == LINK){ 
		gw_link(frame);   //deal with link control
	}else if(cmd_type == DATA_TX){
		gw_data_tx(frame);  //handle data TX
	}else if(cmd_type == TERMINAL_OPRATION){
		gw_terminal_op(frame); //handle opration with terminal
	}else if(cmd_type == COMMUNICATION){
		gw_comm(frame); //handle communication with app
	}else{
		//error handle 
		DEBUG("error command type ");
		return -1;
	}
	return 0;
}

//handle link opration,data store the frame_head.data
int gw_link(struct frame_head* frame)
{
	if(frame.cmd == 0x00){//sign up and login
	
	}else if(frame.cmd == 0x01){  //heart beaten
	
	}else if(frame.cmd == 0x55){  //point to point communication request
	
	}else{
		DEBUG("error with command");
		return 1;
	}
}

int gw_data_tx(struct frame_head* f)
{

}

int gw_terminal_op(struct frame_head* f)
{

}

int gw_comm(u8* buf)
{
}
*/
