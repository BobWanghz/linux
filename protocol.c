#include<stdio.h>
#include"frame.h"
#include<string.h>
#define TAIL_LEN 3
#define FRAME_LEN 30

unsigned char device_id_global[DEVICE_LEN] = "abcdabcdabcdabc";
unsigned char* token;
unsigned char * data;
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

int set_cmd(struct frame_head *frame,unsigned char cmd_type,unsigned char cmd)
{
	frame->cmd_type = cmd_type;
	frame->cmd = cmd;
	return 0;
}

int set_status(struct frame_head *frame,unsigned char status)
{
	frame->status = status;
	return 0;
}

int set_device_id(struct frame_head *frame,unsigned char * device_id)
{
	if(device_id == NULL){
		return -1;
	}
	memcpy(frame->device_id,device_id,DEVICE_LEN);
	return 0;
}
int copy_from_frame(unsigned char* buf,struct frame_head *frame)
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

int copy_to_frame(struct frame_head* f,unsigned char* buf)
{
	f->head = buf[0];
	f->length = (unsigned short)buf[1];
	f->cmd_type = buf[3];
	f->cmd = buf[4];
	f->cmd_seq = (unsigned short)buf[5];
	f->expend = (unsigned short)buf[7];
	f->status = buf[9];
	memcpy(f->device_id,&buf[10],DEVICE_LEN);
	f->token_len = (unsigned short)buf[26];
	f->data_len = (unsigned short)buf[28];
	print_frame(f);
	return 0;
}
int send_frame(int s,const void* msg,int len,unsigned int flags)
{
	struct frame_head frame;
	unsigned char token[] = "wanghauzhong";
	unsigned char send_buf[512];
	int i = 0;
	unsigned short token_length = strlen(token);
	int send_len = 0;

	memset(send_buf,0,512);
	if(init_frame(&frame) != 0){
		perror("init_frame error");
	}
	if(set_status(&frame,0x02) != 0){
		perror("set_status error");
	}
	if(set_cmd(&frame,0xA0,0x01) != 0){
		perror("set_cmd error");
	}
	if(set_device_id(&frame,device_id_global) != 0){
		perror("set_device_id error");
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
	send_len = strlen(send_buf);
	unsigned short crc = cal_crc16(send_buf,send_len);
	memcpy(&send_buf[FRAME_LEN + token_length + len],&crc,2);
	send_buf[FRAME_LEN + token_length + len + 2] = 0x55;
	send_len = strlen(send_buf);
	print_frame(&frame);
	printf("token:%s\n",token);
	printf("data:%s\n",msg);
	printf("crc:%x\n",crc);
	printf("tail:%x\n",0x55);
	for(i = 0;i < frame.length;++i)
		printf("%x ",send_buf[i]);
	printf("\n");
	return send(s,send_buf,frame.length,flags);
}

//crc16
unsigned short cal_crc16(unsigned char * buf,unsigned int len)
{
	if(buf == NULL)
		return 0;
	unsigned short crc16 = 0xFFFF;
	int i,j;
	for(i = 0;i<len;i++){
		crc16 ^= *(buf+i);
		for(j = 0;j < 8;j++){
			unsigned short tmp = crc16 & 0x0001;
			if(tmp){
				crc16 ^= 0xA001;
			}
		}
	}
	return crc16;
}

//Get frame_head from buf recieved
int get_frame(struct frame_head *frame,unsigned char* buf)
{
	if(NULL == buf)
		return -1;
	unsigned char token[30];
	unsigned char data[512];
	memset(token,0,30);
	memset(data,0,512);
	int i = 0;
	copy_to_frame(frame,buf);
	for(i = 0;i < frame->length;i++)
		printf("%x ",buf[i]);
	printf("\n");
	memcpy(token,&buf[FRAME_LEN],frame->token_len);
	memcpy(data,&buf[FRAME_LEN + frame->token_len],frame->data_len);
	printf("token:%s\n",token);
	printf("data:%s\n",data);
	unsigned short crc ;;
	memcpy(&crc,&buf[frame->length -3],2);
	printf("crc:%x\n",crc);
	unsigned char tail = buf[frame->length - 1];
	printf("tail:%x\n",tail);
}

/*
int get_frame(struct frame_head *frame,unsigned char* buf)
{
	if(NULL == buf)
		return -1;
	frame->length = buf[1];	//frame length
	frame->token_len = buf[10 + DEVICE_LEN -1];	//frame token length
	unsigned int crc_len = frame->length - 3;	//the length of data need to calculate crc
	unsigned char *crc_buf = malloc(crc_len);
	memcpy(crc_buf,buf,crc_len);
	if(frame->crc != cal_crc16(crc_buf,crc_len)){
		perror("CRC16 error!");
		//There should have error handle function
		return -1;
	}
	memcpy(frame->token,(unsigned char *)buf[DEVICE_LEN],frame->token_len); 
	memcpy(frame->data,(unsigned char *)buf[DEVICE_LEN+frame->token_len],frame->length - frame->token_len - DEVICE_LEN - 3);
	//unsigned char *crc_buf = malloc(crc_len);	//malloc a buf for data need to calculate crc
	//memcpy(crc_buf,buf,10 + DEVICE_LEN + 2);
	//crc_buf = strcat(crc_buf,frame.token);
	//crc_buf = strcat(crc_buf.frame.data);
	//memcpy(crc_buf,buf,crc_len);		
	frame->crc = buf[frame->length -3];
	
	frame->head = buf[0];
	frame->cmd_type = buf[3];
	frame->cmd = buf[4];
	frame->cmd_seq = buf[5];
	frame->expend = buf[7];
	frame->status = buf[9];
	memcpy(frame->device_id,(unsigned char *)buf[10],DEVICE_LEN);
	frame->token = (unsigned char *)buf[10 + DEVICE_LEN + 2 -1];
	frame->data = (unsigned char *)buf[frame->token_len + DEVICE_LEN + 10 + 2 -1];
	frame->tail = buf[frame->length - 1];
	printf("get_frame() ok !\n");
	return 0;
}
*/
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
int gateway(struct frame_head frame)
{
	unsigned char cmd_type = frame.cmd_type;
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
		perror("error command type ");
		return -1;
	}
	return 0;
}

//handle link opration,data store the frame_head.data
int gw_link(struct frame_head frame,unsigned char* data)
{
	data = frame.data;
	int len = sizeof(data);
	if(frame.cmd == 0x00){//sign up and login
	
	}else if(frame.cmd == 0x01){  //heart beaten
	
	}else if(frame.cmd == 0x55){  //point to point communication request
	
	}else{
		perror("error with command");
		return 1;
	}
}
*/
