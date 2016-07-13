#ifndef __FRAME_H__
#define __FRAME_H__

#define MAXTU 1400  //max transform unit
#define HEAD 0x55  //head of frame_head
#define TAIL 0x55 //tail of frame_head

#define DEVICE_LEN 16 //device_id use DEVICE_LEN bytes
//command type define
#define LINK 0xA0  //command type is link operation
#define ACK_P2P 0xEE //confirmation of point to point
#define DATA_TX 0xAA //TX data
#define COMMUNICATION 0xEF //communication between service and APP
#define TERMINAL_OPRATION 0xF0 //terminal opration

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;
struct frame_head
{
	u8 head;  //head of frame_head 0
	u16 length;  //frame_head length 1
	u8 cmd_type;  //command type 3
	u8 cmd;       //command 4
	u16 cmd_seq;  //command sequence 5
	u16 expend;            //expend  field 7
	u8 status;     //9
	u8 device_id[DEVICE_LEN];  //the id of device 10
	u16 token_len;    //the length of token 25
	u16 data_len;	
}__attribute__((packed));

struct send_buffer
{
	u8 id[10];
	u8 data[10][512];
};

u16 cal_crc16(u8* buf,unsigned int len);
int init_frame(struct frame_head *frame);
int set_cmd(struct frame_head *frame,u8 cmd_type,u8 cmd);
int set_status(struct frame_head *frame,u8 status);
int set_device_id(struct frame_head *frame,u8 * device_id);
int get_frame(struct frame_head *frame,u8* buf);
int send_frame(int s,const void *msg,int len,unsigned int flags);
void print_frame(struct frame_head *frame);
int copy_to_frame(struct frame_head* f,u8* buf);
int copy_fram_frame(u8* buf,struct frame_head* f);

#define EXPEND 0x00
//u8 device_id_global[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xA,0xB,0xC,0xD,0xE,0xF};

#endif
