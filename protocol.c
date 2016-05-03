#include<stdio.h>
#include"frame.h"
#include<string.h>

/* Set frame,all function return 0 when succeed,return -1 when error*/
int init_frame(struct gateway_frame frame)
{
	if(frame == NULL){
		perror("set frame error");
		return -1;
	}
	frame.head = HEAD;
	frame.tail = TAIL;
	frame.token_len = 0; //set
	frame.length = 15 + DEVICE_LEN;
	return 0;
}

int set_cmd(struct gateway_frame frame,unsigned char cmd_type,unsigned char cmd)
{
	if(frame == NULL){
		return -1;
	}
	frame.cmd_type = cmd_type;
	frame.cmd = cmd;
	return 0;
}

int set_status(struct gateway_frame frame,unsigned char status)
{
	if(frame == NULL){
		return -1;
	}
	frame.status = status;
	return 0;
}

int set_device_id(struct gateway_frame frame,unsigned char * device_id)
{
	if(frame == NULL || device_id == NULL){
		return -1;
	}
	memcpy(frame.device_id,device_id,DEVICE_LEN);
	return 0;
}

int set_token(struct gateway_frame frame,unsigned char *token)
{
	if(frame == NULL || token == NULL){
		return -1;
	}
	unsigned short token_len = sizeof(token);
	frame.token_len = token_len;
	memcpy(frame.token,token,token_len);
	//to add the token_len to total length of gateway_frame
	frame.length += token_len;
	return 0;
}
int set_data(struct gateway_frame frame,unsigned char *data)
{
	if(frame == NULL || data == NULL){
		return -1;
	}
	unsigned short data_len = sizeof(data);
	memcpy(frame.data,data,data_len);
	//Add length of data to total length
	frame.length += data_len;
	return 0;
}

int set_crc(struct gateway_frame frame)
{
	if(frame == NULL || frame.length <= 15 + DEVICE_LEN){
		return -1;
	}
	unsigned short crc_len = frame.length - 3 
	unsigned char *crc_buf = malloc(crc_len);
	memcpy(crc_buf,(unsigned char *)frame,crc_len);
	unsigned short crc16 = cal_crc16(crc_buf,crc_len);
	frame.crc = crc;
	return 0;
}

//crc16
unsigned short cal_crc16(unsigned char * buf,unsigned int len)
{
	if(buf == NULL)
		return 0;
	unsigned short crc16 = 0xFFFF;
	int i,j;
	for(i = 0;i<len;i++){
		crc16 ^= *buf[i];
		for(j = 0;j < 8;j++){
			unsigned short tmp = crc16 & 0x0001;
			if(tmp){
				crc16 ^= 0xA001;
			}
		}
	}
	return crc16;
}

//Get gateway_frame from buf recieved

int get_frame(struct gateway_frame frame,unsigned char* buf)
{
	if(buf == NULL)
		return -1;
	frame.length = buf[1];	//frame length
	frame.token_len = buf[10 + DEVICE_LEN -1];	//frame token length
	unsigned int crc_len = frame.length - 3;	//the length of data need to calculate crc
	unsigned char *crc_buf = malloc(crc_len);
	memcpy(crc_buf,buf,crc_len);
	if(frame.crc != cal_crc16(crc_buf,crc_len)){
		perror("CRC16 error!");
		//There should have error handle function
		return -1;
	}
	memcpy(frame.token,(unsigned char *)buf[DEVICE_LEN],frame.token_len); 
	memcpy(frame.data,(unsigned char *)buf[DEVICE_LEN+frame.token_len],frame.length - frame.token_len - DEVICE_LEN - 3);
	//unsigned char *crc_buf = malloc(crc_len);	//malloc a buf for data need to calculate crc
	//memcpy(crc_buf,buf,10 + DEVICE_LEN + 2);
	//crc_buf = strcat(crc_buf,frame.token);
	//crc_buf = strcat(crc_buf.frame.data);
	//memcpy(crc_buf,buf,crc_len);		
	frame.crc = buf[frame.length -3];
	
	frame.head = buf[0];
	frame.cmd_type = buf[3];
	frame.cmd = buf[4];
	frame.cmd_seq = buf[5];
	frame.expend = buf[7];
	frame.status = buf[9];
	memcpy(frame.device_id,(unsigned char *)buf[10],DEVICE_LEN);
	frame.token = (unsigned char *)buf[10 + DEVICE_LEN + 2 -1];
	frame.data = (unsigned char *)buf[frame.token_len + DEVICE_LEN + 10 + 2 -1];
	frame.tail = buf[frame.length - 1];
	return 0;
}

int gateway(struct gateway_frame frame)
{
	if(frame == NULL)
		return -1;
	unsigned char cmd_type = frame.cmd_type;
	if(cmd_type == LINK){ 
		gw_link(frame);   //deal with link control
	}else if(cmd_type == DATA_TX){
		gw_data_tx(frame)  //handle data TX
	}else if(cmd_type == TERMINAL_OPRATION){
		gw_terminal_op(frame); //handle opration with terminal
	}else if(cmd_type == COMMUNICATION){
		gw_comm(frame); //handle communication with app
	}else{
		//error handle 
		perror("error command type ");
		return 1;
	}
	return 0;
}

//handle link opration,data store the gateway_frame.data
int gw_link(struct gateway_frame frame,unsigned char* data)
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

