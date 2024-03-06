#pragma once

#  define VIUMSG_EXPORT __declspec(dllexport)

enum VIUMessageType
{
	CONNECTED	= 0x01,  //连接成功
	USR_MSGDATA = 0x02,  //用户端发送过来的数据
	ACK_MSGDATA = 0x03,  //用户端发送过来的ACK消息
};

/* 
  desc:	sdk recvd msg callback to app
  parameter:	MessageType -> msg type
				const char*	-> data content
				size_t      -> data size
  */

typedef void (*OnViuMessageCallback) (VIUMessageType, char*, size_t);


extern "C"
{
	/*
	desc:	get sdk version
	return: char*bu	-> version
	*/
	VIUMSG_EXPORT const char* get_sdk_version();

	/*
	desc: init sdk and begin connect
	parameter:	OnViuMessageCallback -> callback
	            int reconnection_interval, if connect failed, retry time interval
	return:		0 -> failed; 1-> successed 
	*/
	VIUMSG_EXPORT int on_start_connect(OnViuMessageCallback callback, int reconnection_interval = 1);

	/*
	desc: send message function
	parameter:	char* -> data content
				size_t-> data size
	return:		0 -> failed; 1-> successed
	*/
	VIUMSG_EXPORT int on_send_msg(char* databuf, size_t size);

	/* disconnect and uninit */
	VIUMSG_EXPORT void on_stop_connect();

}

