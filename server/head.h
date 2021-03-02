/*================================================================
*   Copyright (C) 2021 hqyj study demo file.
*   
*   文件名称：head.h
*   创 建 者：JiangYing
*   创建日期：2021年01月08日
*   描    述：电子词典服务器
*
================================================================*/
#ifndef HEAD_H__
#define HEAD_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sqlite3.h>
#include <time.h>

#define PORT 2020
#define IP "0.0.0.0"
#define SIZE 200

//自定义通信协议结构体
struct Msg{
	char type;//通信类型
	char name[20];//用户名
	char pwd[20];//密码
	char buf[SIZE];//数据
};
//线程参数传递结构体
typedef struct{
	int cntfd;
	struct sockaddr_in cin;
}THR;
//回调函数callback的结构体
typedef struct{
	int cntfd;
	char type;
	int flag;
}CB;
//回调函数callback_login/regist的结构体
typedef struct{
	int cntfd;
	struct Msg *msg;
	int flag;
}CBL;


int msg_size;

//1、启动服务器
void open_server();
//2、数据接收线程
void* handler(void* arg);
//3、向数据库写数据
void write_db();
//4、注册
void regist(int cntfd,struct Msg* msg);
//5、向客户端发送消息
void sendto_client(int cntfd,struct Msg* msg,int size);
//6、登陆
void login(int cntfd,struct Msg* msg);
//7、查询单词
void get_words(int cntfd,struct Msg* msg);
//8、数据库查询回调函数
int callback(void* arg,int f_num,char** f_value,char** f_name);
//9、获取查询历史
void get_history(int cntfd,struct Msg* msg);
//10、用户表查询回调函数
int callback_login(void* arg,int f_num,char** f_value,char** f_name);
//11、注册用户的回调函数
int callback_regist(void* arg,int f_num,char** f_value,char** f_name);
//12、历史记录用的回调函数
int callback_history(void* arg,int f_num,char** f_value,char** f_name);







#endif //HEAD_H__

