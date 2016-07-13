#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __TRACE__
#define DEBUG(fmt,...)\
	printf(""__FILE__":%d: "fmt,__LINE__,##__VA_ARGS__)
	//printf("%s(%d)-<%s>: "##fmt,__FILE__,__LINE__,__func__,##__VA_ARGS__)
#else
#define DEBUG(fmt,...)\
	printf(fmt,##__VA_ARGS__)
#endif


#endif
