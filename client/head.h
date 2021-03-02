#ifndef HEAD_H__
#define HEAD_h__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#define PORT 2020
#define IP "192.168.131.129"
#define SIZE 200
/*
自定义协议：
	struct Msg{
		char type;
		char name[20];
		char pwd[20];
		char buf[1024];
	}
	type:约定数据类型
		E：出错
		L：登陆
		R：注册
		W: 查询单词
		H：查询历史
	name:用户名(type为L和R时生效)
	pwd:登录密码(type为L和R时生效)
	buf:数据内容
*/
struct Msg{
	char type;
	char name[20];
	char pwd[20];
	char buf[SIZE];
};

int msg_size;
int cfd;//流式套接字

//1、主菜单
void Menu();
//2、停顿函数
void wait();
//3、关于界面
void about();
//4、登陆界面
void login();
//5、登陆
int log_server();
//6、注册
void regis_server();
//7、连接服务器
int connect_server();
//8、发送消息
void send_msg(struct Msg* msg,int size);
//9、接收消息
void recv_msg();
//10、判断是否可以登陆
void is_log(struct Msg* msg);
//11、电子词典主界面
void dictory();
//12、判断是否注册成功
void is_regis(struct Msg* msg);
//13、查询单词
void find();
//14、查询历史记录
void get_history(struct Msg* msg);
//15、查询历史记录
void history();



#endif //HEAD_H_
