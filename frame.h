#define MAXTU 1400  //max transform unit
#define HEAD 0x55  //head of gateway_frame
#define TAIL 0x55 //tail of gateway_frame

#define DEVICE_LEN 16 //device_id use DEVICE_LEN bytes
//command type define
#define LINK 0xA0  //command type is link operation
#define ACK_P2P 0xEE //confirmation of point to point
#define DATA_TX 0xAA //TX data
#define COMMUNICATION 0xEF //communication between service and APP

struct gateway_frame
{
	unsigned char head;  //head of gateway_frame 0
	unsigned short length;  //gateway_frame length 1
	unsigned char cmd_type;  //command type 3
	unsigned char cmd;       //command 4
	unsigned short cmd_seq;  //command sequence 5
	short expend;            //expend  field 7
	unsigned char status;     //9
	unsigned char device_id[DEVICE_LEN];  //the id of device 10
	unsigned short token_len;    //the length of token 25
	char* token;    //26
	char* data;
	unsigned short crc;
	unsigned char tail;
}__attribute__(packed);




