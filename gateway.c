#include "frame.h"

#if 1
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
#endif
