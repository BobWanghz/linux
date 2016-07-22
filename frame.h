#ifndef __FRAME_H__
#define __FRAME_H__

#define MAXTU 1400  //max transform unit
#define HEAD 0x55  //head of frame
#define TAIL 0x55 //tail of frame

#define DEVICE_LEN 16 //device_id use DEVICE_LEN bytes
#define MAX_DATA_LEN 512
#define BUFFER_COUNT 10
//command type define
#define LINK 0xA0  //command type is link operation
#define ACK_P2P 0xEE //confirmation of point to point
#define DATA_TX 0xAA //TX data
#define COMMUNICATION 0xEF //communication between service and APP
#define TERMINAL_OPRATION 0xF0 //terminal opration

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;
struct frame
{
	u8 head;
	u16 length;  //frame length 1
	u8 cmd_type;  //command type 3
	u8 cmd;       //command 4
	u16 cmd_seq;  //command sequence 5
	u16 expend;            //expend  field 7
	u8 status;     //9
	u8 device_id[DEVICE_LEN];  //the id of device 10
	u16 token_len;    //the length of token 25
	u8* token;
	u16 data_len;
	u8* data;
	u16 crc;
	u8 tail;
}__attribute__((packed));

/*
stuct receive_buffer
{
	u8 buf[MAX_DATA_LEN];
	u8 stat;		//0 means this buffer is empty,1 -- used
	u16 length;
};
*/
struct buffer
{
	u8 data[BUFFER_COUNT][MAX_DATA_LEN];
	u8 first;
	u8 last;
};
u16 cal_crc16(u8* buf,unsigned int len);
int init_frame(struct frame *f);
int set_cmd(struct frame *f,u8 cmd_type,u8 cmd);
int set_status(struct frame *f,u8 status);
int set_device_id(struct frame *f,u8 * device_id);
int get_frame(struct frame *f,u8* buf);
int send_frame(int s,unsigned int flags);
void print_frame(struct frame *f);
int copy_to_frame(struct frame* f,u8* buf);
int copy_fram_frame(u8* buf,struct frame* f);

#define EXPEND 0x00
//u8 device_id_global[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xA,0xB,0xC,0xD,0xE,0xF};

#endif
