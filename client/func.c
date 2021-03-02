#include "head.h"

//主菜单
void Menu(){
	int option; 
	while(1){
		system("clear");
		printf("/*******************************************************************************/\n");
		printf("/*           +         +++++++++      +     ++++++++++        +     +          */\n");
		printf("/*           +                +        +             +        +     +          */\n");
		printf("/*       +++++++++           +          +     +++++  +     +++++++++++++       */\n");
		printf("/*       +   +   +          +                        +     +  +     +  +       */\n");
		printf("/*       +++++++++   +++++++++++++++  +++++   +++++  +     +++++++++++++       */\n");
		printf("/*       +   +   +          +            +    +   +  +     +  +     +  +       */\n");
		printf("/*       +++++++++          +           +     +++++  +   +++++++++++++++++     */\n");
		printf("/*           +              +          +   +         +        +      +         */\n");
		printf("/*           +     +      + +         +  +         + +       +        +        */\n");
		printf("/*           +++++++        +        +               +      +          +       */\n");
		printf("/*                       ————————————————                                      */\n");
		printf("/*                      |   1->登陆系统  |                                     */\n");
		printf("/*                      |   2->退出系统  |                                     */\n");
		printf("/*                      |   3->关于      |                                     */\n");
		printf("/*                       ————————————————                                      */\n");
		printf("/*                      ||-->请选择相应编号...                                 */\n");
		printf("/*                                                                             */\n");
		printf("/*                                                                             */\n");
		printf("/*******************************************************************************/\n");
		printf("||->");
		scanf("%d",&option);
		
		int res=0;
		switch(option){
		case 1:
			//登陆系统
			res=connect_server();
			if(res==-1){
				fprintf(stderr,"连接服务失败,请检查网络...\n");
				wait();
				continue;
			}
			login();
			break;
		case 2:
			//退出系统
			printf("电子词典退出...\n");
			return;
		case 3:
			//关于
			about();
			continue;
		default:
			//异常
			printf("您的选择有误，请重新输入...\n");
			wait();
			continue;
		}
	}
}

//2、pause函数
void wait(){
	fprintf(stderr,"任意键继续...\n");
	getchar();
	getchar();
}

//3、about:关于
void about(){
	while(1){
		char str[20]="";
		system("clear");
		printf("/*****************************************************/\n");
		printf("/*         软    件：电 子 词 典                     */\n");
		printf("/*         版    本：Version 1.0                     */\n");
		printf("/*         时    间：2021-01-08                      */\n");
		printf("/*         作    者：JiangYing                       */\n");
		printf("/*         联系方式：18846077784/2405596758@qq.com   */\n");
		printf("/*         描    述:                                 */\n");
		printf("/*                  这是一个依赖局域网的电子词典，   */\n");
		printf("/*               客户端连接服务器后可以查询单词释义；*/\n");
		printf("/*                                                   */\n");
		printf("/*                                                   */\n");
		printf("/*****************************************************/\n");
		printf("/*         ||->输入exit退出;                         */\n");
		printf("/*****************************************************/\n");
		
		bzero(str,sizeof(str));
		printf("||->");
		scanf("%s",str);
		if(strncasecmp(str,"exit",4)==0){
			printf("||->退出about界面\n");
			return;
		}
		else{
			printf("||->您的输入有误，请重新输入\n");
			wait();
			continue;
		}

	}
}

//4、login:登陆系统
void login(){
	char str[20]="";
	while(1){
		system("clear");
		printf("/***********************************************/\n");
		printf("/*                                             */\n");
		printf("/*           ||->1、登陆                       */\n");
		printf("/*           ||->2、注册                       */\n");
		printf("/*           ------------                      */\n");
		printf("/*           ||->输入exit退出界面...           */\n");
		printf("/*                                             */\n");
		printf("/***********************************************/\n");
		printf("||->");

		bzero(str,sizeof(str));
		scanf("%s",str);

		if(strcmp(str,"1")==0){
			//登陆
			log_server();
		}
		else if(strcmp(str,"2")==0){
			//注册
			regis_server();
		}
		else if(strncasecmp(str,"exit",4)==0){
			//退出
			return;
		}
		else{
			printf("||->您的输入有误，请重新输入...\n");
			wait();
			continue;
		}
	}
}

//5、connect_server：连接服务器
int connect_server(){
	msg_size=sizeof(struct Msg);
	//创建流式套接字:socket
	cfd=socket(AF_INET,SOCK_STREAM,0);
	if(cfd<0){
		perror("socket");
		return -1;
	}
	//允许端口快速复用
	int reuse=1;
	if(setsockopt(cfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int))<0){
		perror("setsockopt");
		return -1;
	}
	//绑定IP和端口号:bind(可缺省)
	
	//指定服务器信息
	struct sockaddr_in sin;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(PORT);
	sin.sin_addr.s_addr=inet_addr(IP);

	//连接服务器
	if(connect(cfd,(struct sockaddr*)&sin,sizeof(sin))<0){
		perror("connect");
		return -1;
	}

	//创建线程，负责接收来自服务器的数据

	printf("连接服务器成功\n");
	
	return 0;
}

//6、log_server:登陆
int  log_server(){
	struct Msg msg;
	struct Msg rcv;
	int res=0;

	//输入用户名
	getchar();
	printf("请输入 用户名||->");
	scanf("%s",msg.name);
	
	//输入密码
	printf("请输入 密码||->");
	scanf("%s",msg.pwd);

	//设置消息类型
	msg.type='L';
	strcpy(msg.buf,"log_server");

	//向服务器发送消息
	send_msg(&msg,sizeof(msg));

	fprintf(stderr,"查询中...\n");
	recv_msg();

	return 0;
}

//7、send_msg:发送消息
void send_msg(struct Msg* msg,int size){
	int res=0;
	do{
		res=send(cfd,msg,sizeof(*msg),0);
	}while(res<0 && errno==EINTR);

	if(res<0){
		perror("send");
		return ;
	}

	return;
}

//8、recv_msg:接收消息
void recv_msg(){
	//接收来自服务器的消息

	struct Msg rcv;
	int res;

	do{
		res=recv(cfd,&rcv,msg_size,0);
	}while(res<0 && errno==EINTR);

	if(res<0){
		perror("recv");
		return ;
	}

	if(res==0){
		printf("服务器关闭或网络忙,请退出...\n");
		return;
	}


	switch(rcv.type){
	case 'E':
		//出错
		break;
	case 'L':
		//登陆
		is_log(&rcv);
		break;
	case 'R':
		//注册
		is_regis(&rcv);
		break;
	case 'W':
		//查询单词
		if(strcmp(rcv.buf,"no")==0){
			fprintf(stderr,"数据库中没有这个单词...\n");
			return;
		}
		fprintf(stderr,"为您查询到：\n\t%s\n",rcv.buf);
		break;
	case 'H':
		//查询历史
		if(strcmp(rcv.buf,"no")==0){
			fprintf(stderr,"没有历史记录...\n");
			return;
		}
		get_history(&rcv);
		break;
	default:
		break;
	}
}

//9、is_log:判断是否登陆
void is_log(struct Msg* msg){
	if(strcmp(msg->buf,"ok")==0){
		fprintf(stderr,"成功登陆...\n");
		sleep(2);
		//进入电子词典界面
		system("clear");
		dictory();
	}
	else{
		fprintf(stderr,"您输入的用户名或密码不正确，请确认后重试...\n");
		wait();
		return;
	}
}
//10、dictory:电子词典主界面
void dictory(){
	char str[20]="";
	while(1){
		system("clear");
		printf("/****************************************************/\n");
		printf("/*                 欢迎使用电子词典                 */\n");
		printf("/*                                                  */\n");
		printf("/*                 ||->1、查询单词                  */\n");
		printf("/*                 ||->2、查看历史记录              */\n");
		printf("/*                -----------------------           */\n");
		printf("/*                 ||->输入exit退出当前界面         */\n");
		printf("/*                                                  */\n");
		printf("/****************************************************/\n");
		printf("||->");
		bzero(str,sizeof(str));
		getchar();
		scanf("%s",str);

		if(strncasecmp(str,"1",1)==0){
			//查询单词
			find();
			continue;
		}
		else if(strncasecmp(str,"2",1)==0){
			//查看历史记录
			history();
		}
		else if(strncasecmp(str,"exit",4)==0){
			//退出当前界面
			break;
		}
		else{
			printf("您的输入有误，请重新输入...\n");
			wait();
			continue;
		}
	}
}

//11、regis_server:注册
void regis_server(){
	struct Msg msg;
	char str[20]="";
	//填写注册的信息包
	msg.type='R';
	
	//新用户名
	getchar();
	fprintf(stderr,"||请输入 新用户名->");
	scanf("%s",msg.name);
	//新密码
A:  fprintf(stderr,"||请输入 新密码->");
	getchar();
	scanf("%s",msg.pwd);
	fprintf(stderr,"||请再次输入 新密码->");
	getchar();
	scanf("%s",str);
	
	if(strcmp(msg.pwd,str)!=0){
		fprintf(stderr,"两次的密码不一致，请重新输入...\n");
		goto A;
	}

	//发送数据
	send_msg(&msg,sizeof(msg));
	recv_msg();

	return;

}

//12、is_regis:判断是否注册成功
void is_regis(struct Msg* msg){
	if(strcmp(msg->buf,"ok")==0){
		system("clear");
		fprintf(stderr,"恭喜注册成功，请牢记密码...\n");
		wait();
		return;
	}
	else if(strcmp(msg->buf,"no")==0){
		fprintf(stderr,"用户已经存在...\n");
		wait();
		return;
	}
	else{
		fprintf(stderr,"对不起，注册失败，请重试...\n");
		wait();
		return;
	}
}
//13、find:查询单词
void find(){
	system("clear");
	struct Msg msg;
	char str[20]="";
	printf("/*******************************/\n");
	printf("/*                             */\n");
	printf("/*           单词查询          */\n");
	printf("/*        -------------        */\n");
	printf("/*         输入exit退出...     */\n");
	printf("/*******************************/\n");
	fflush(stdout);
	while(1){
		fprintf(stderr,"\n您需要查询的单词||->");
		scanf("%s",str);
		if(strncasecmp(str,"exit",4)==0){
			return;
		}
		msg.type='W';
		strcpy(msg.name,"");
		strcpy(msg.pwd,"");
		strcpy(msg.buf,str);

		send_msg(&msg,sizeof(msg));

		recv_msg();

	}
}
//14、gethistory:获得历史记录
void get_history(struct Msg* msg){
	int res=0;
	while(msg->type=='H' && strcmp(msg->buf,"finish")!=0){
		printf("%s\n",msg->buf);
		do{
			res=recv(cfd,msg,msg_size,0);
		}while(res<0 && errno==EINTR);

		if(res<0){
			perror("recv");
			return;
		}
		if(res==0){
			break;
		}
	}
	fprintf(stderr,"------------\n");
}
//15、查询历史记录
void history(){
	char str[20];
	struct Msg sen;
	while(1){
		system("clear");
		printf("/**********************************/\n");
		printf("/*          历史记录              */\n");
		printf("/*      ----------------          */\n");
		printf("/*        输入exit退出...         */\n");
		printf("/**********************************/\n");
		
		sen.type='H';
		strcpy(sen.name,"");
		strcpy(sen.pwd,"");
		strcpy(sen.buf,"");
		send_msg(&sen,sizeof(sen));

		recv_msg();

		printf("||->");
		scanf("%s",str);
		if(strcmp(str,"exit")==0){
			return;
		}
	}
}
