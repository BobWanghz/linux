#include<stdio.h>
#include"frame.h"
#include<string.h>

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
int copy_frame(unsigned char* buf,struct frame_head frame)
{
	buf[0] = frame.head;
	memcpy(&buf[1],&frame.length,2);
	buf[3] = frame.cmd_type;
	buf[4] = frame.cmd;
	memcpy(&buf[5],&frame.cmd_seq,2);
	memcpy(&buf[7],&frame.expend,2);
	buf[9] = frame.status;
	memcpy(&buf[10],frame.device_id,DEVICE_LEN);
	memcpy(&buf[26],&frame.token_len,2);
	memcpy(&buf[28],&frame.data_len,2);
	return 0;
}
int send_frame(int s,const void* msg,int len,unsigned int flags)
{
	struct frame_head frame;
	unsigned char token[] = "wanghauzhong";
	unsigned char send_buf[512];
	unsigned char tail[4];
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
	copy_frame(send_buf,frame);
	//memcpy(send_buf,(char *)&frame,FRAME_LEN);
	printf("%s\n",send_buf);
	strcat(send_buf,token);
	printf("%s\n",send_buf);
	strcat(send_buf,msg);
	printf("%s\n",send_buf);
	send_len = strlen(send_buf);
	unsigned short crc = cal_crc16(send_buf,send_len);
	sprintf(tail,"%hd",crc);
	tail[2] = 0x55;
	strcat(send_buf,tail);
	send_len = strlen(send_buf);
	print_frame(&frame);
	printf("token:%s\n",token);
	printf("data:%s\n",msg);
	printf("crc:%hu\n",crc);
	printf("tail:%x\n",0x55);
	//for(i = 0;i < strlen(send_buf);++i)
	//	printf("%x\n",send_buf[i]);
	printf("length of send_buf:%d\n",send_len);
	//printf("\n");
	return send(s,send_buf,send_len,flags);
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
	printf("length of buf:%d\n",strlen(buf));
	memset(token,0,token);
	memset(data,0,512);
	memcpy(frame,buf,FRAME_LEN);
	print_frame(frame);
	memcpy(token,&buf[FRAME_LEN],frame->token_len);
	memcpy(data,&buf[FRAME_LEN + frame->token_len],frame->data_len);
	printf("token:%s\n",token);
	printf("data:%s\n",data);
	unsigned char crc = buf[strlen(buf) - 3];
	printf("crc:%hu\rn",crc);
	unsigned char tail = buf[strlen(buf) - 1];
	printf("tail:%x\n",0x55);
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
