#include "head.h"


char sql[300]="";
char *errmsg=NULL;
sqlite3* db=NULL;
char NAME[20]="";//当前登陆的用户名

//1、启动服务器
void open_server(){
	msg_size=sizeof(struct Msg);
	//创建套接字:socket
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd<0){
		perror("socket");
		return;
	}
	//允许端口快速复用
	int reuse=1;
	if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int))<0){
		perror("setsockopt");
		return;
	}
	//绑定服务器IP和端口:bind
	struct sockaddr_in sin;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(PORT);
	sin.sin_addr.s_addr=inet_addr(IP);

	if(bind(sfd,(struct sockaddr*)&sin,sizeof(sin))<0){
		perror("bind");
		return;
	}
	//启动数据库
	int res=sqlite3_open("./mysq.db",&db);
	if(res!=0){
		fprintf(stderr,"sqlite3_open:%s",sqlite3_errmsg(db));
		return;
	}
	//如果数据库为空，向数据库写入数据
	//write_db();
	//先清除表history
	bzero(sql,sizeof(sql));
	sprintf(sql,"drop table history");
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0){
		fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
		return;
	}
	//打开数据库中的历史记录表
	bzero(sql,sizeof(sql));
	sprintf(sql,"create table if not exists history(name char,time char,word char,means char)");
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0){
		fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
		return;
	}


	//监听：listen
	if(listen(sfd,4)<0){
		perror("listen");
		return;
	}
	//等待客户端连接
	struct sockaddr_in cin;
	socklen_t len=sizeof(cin);
	int port=0;
	char ip[20]="";
	int cntfd=-1;

	printf("------电子词典服务器启动------\n");

	while(1){
		//从等待队列中获得一个IP
		cntfd=accept(sfd,(struct sockaddr*)&cin,&len);
		if(cntfd<0){
			perror("accept");
			return ;
		}
		//获取连接上来的客户端的IP和端口
		port=0;
		port=ntohs(cin.sin_port);
		bzero(ip,sizeof(ip));
		inet_ntop(AF_INET,&cin.sin_addr,ip,20);

		fprintf(stderr,"[%s:%d]连接成功\n",ip,port);

		//创建线程处理连接事件
		pthread_t tid;
		THR thr;
		thr.cntfd=cntfd;
		thr.cin=cin;
		if(pthread_create(&tid,NULL,handler,(void*)&thr)<0){
			perror("pthread_create");
			return;
		}
	}
	//关闭文件描述符和数据库
	close(sfd);

	if(sqlite3_close(db)!=0){
		fprintf(stderr,"数据库关闭失败\n");
		return;
	}
	printf("------服务器关闭------\n");
}

//2、handler:数据接收线程函数
void* handler(void* arg){
	//分离线程
	pthread_detach(pthread_self());

	//传入的参数
	THR thr=*(THR*)arg;
	int port=ntohs(thr.cin.sin_port);
	char ip[20]="";
	inet_ntop(AF_INET,&thr.cin.sin_addr,ip,20);
	int cntfd=thr.cntfd;
	int res=0;
	struct Msg msg;

	while(1){
		//接收数据
		do{
			res=recv(cntfd,&msg,msg_size,0);
		}while(res<0 && errno==EINTR);

		if(res<0){
			perror("recv");
			return NULL;
		}

		if(res==0){
			fprintf(stderr,"[%s:%d]断开连接\n",ip,port);
			break;
		}
		//处理接收到的数据
		printf("%d->%c,%s,%s,%s\n",__LINE__,msg.type,msg.name,msg.pwd,msg.buf);
		switch(msg.type){
		case 'E':
			//出错
			fprintf(stderr,"客户端存在错误...\n");
			break;
		case 'R':
			//注册
			regist(cntfd,&msg);
			break;
		case 'L':
			//登陆
			login(cntfd,&msg);
			break;
		case 'W':
			//查询单词
			get_words(cntfd,&msg);
			break;
		case 'H':
			//查询历史记录
			get_history(cntfd,&msg);
			break;
		default:
			//异常
			fprintf(stderr,"[%s:%d]\n",ip,port);
			break;
		}

	}
	close(cntfd);
	pthread_exit(NULL);
}
//3、向数据库写数据
void write_db(){
	char buf[100]="";
	FILE* fd=fopen("./dict.txt","r");
	if(fd==NULL){
		perror("open");
		return;
	}
	//向数据库创建表
	bzero(sql,sizeof(sql));
	sprintf(sql,"create table if not exists dict(word char,means char)");
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0){
		fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
		return;
	}
	//向表中插入数据
	char r1[100]="";
	char r2[100]="";
	while(fgets(buf,100-1,fd)!=NULL){
		sscanf(buf,"%s %s",r1,r2);
		printf("%s %s\n",r1,r2);
		bzero(sql,sizeof(sql));
		sprintf(sql,"insert into dict values(\"%s\",\"%s\")",r1,r2);
	
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0){
			fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
			return;
		}
	
	}
	printf("插入数据成功\n");
	fclose(fd);
}
//4、regist:注册函数
void regist(int cntfd,struct Msg* msg){
	printf("regist\n");
	struct Msg sen;
	//打开数据库中的用户表
	sprintf(sql,"create table if not exists user(name char,pwd char)");
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0){
		fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
		return;
	}
	//查找注册用户是否存在
	bzero(sql,sizeof(sql));
	CBL cbl;
	cbl.cntfd=cntfd;
	cbl.msg=msg;
	cbl.flag=0;
	sprintf(sql,"select * from user where name=\"%s\"",msg->name);
	if(sqlite3_exec(db,sql,callback_regist,&cbl,&errmsg)!=0){
		fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
		return;
	}
	printf("%d\n",__LINE__);
	if(cbl.flag==0){
		fprintf(stderr,"用户不存在，正在注册...\n");
		bzero(sql,sizeof(sql));
		sprintf(sql,"insert into user values(\"%s\",\"%s\")",msg->name,msg->pwd);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0){
			fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
			return;
		}
		fprintf(stderr,"注册成功...\n");
		//注册成功向客户端发送消息
		bzero(&sen,sizeof(sen));
		sen.type='R';
		strcpy(sen.name,msg->name);
		strcpy(sen.pwd,msg->pwd);
		strcpy(sen.buf,"ok");

		sendto_client(cntfd,&sen,sizeof(sen));
	}
	return;
}

//4、sendto_client:向客户端发数据
void sendto_client(int cntfd,struct Msg* msg,int size){
	int res=0;
	do{
		res=send(cntfd,msg,msg_size,0);
	}while(res<0 && errno==EINTR);

	if(res<0){
		perror("send");
		return;
	}

	if(res==0){
		return;
	}
}
//5、login:登陆函数
void login(int cntfd,struct Msg* msg){
	struct Msg sen;
	//查找数据库用户表
	bzero(sql,sizeof(sql));
	sprintf(sql,"create table if not exists user(name char,pwd char)");
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0){
		fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
		return;
	}

	bzero(sql,sizeof(sql));
	CBL cbl;
	cbl.cntfd=cntfd;
	cbl.msg=msg;
	cbl.flag=0;
	sprintf(sql,"select * from user where name=\"%s\" and pwd=\"%s\"",msg->name,msg->pwd);
	if(sqlite3_exec(db,sql,callback_login,&cbl,&errmsg)!=0){
		fprintf(stderr,"sqlite3_exec %d:%s\n",__LINE__,errmsg);
		return;
	}
	if(cbl.flag==0){
		//用户不存在
		fprintf(stderr,"用户不存在，无法登陆...\n");
		
		bzero(&sen,sizeof(sen));
		sen.type='L';
		strcpy(sen.name,msg->name);
		strcpy(sen.pwd,msg->pwd);
		strcpy(sen.buf,"no");

		sendto_client(cntfd,&sen,sizeof(sen));
		
		return ;

	}
}

//6、get_word:查询单词
void get_words(int cntfd,struct Msg* msg){
	struct Msg sen;

	//从单词数据库中查询单词
	bzero(sql,sizeof(sql));
	CB cb;
	cb.type='W';
	cb.cntfd=cntfd;
	cb.flag=0;
	sprintf(sql,"select * from dict where word=\"%s\"",msg->buf);
	if(sqlite3_exec(db,sql,callback,&cb,&errmsg)!=0){
		fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
		return;
	}

	printf("cb.flag->%d\n",cb.flag);
	if(cb.flag==0){
		fprintf(stderr,"未查询到%s\n",msg->buf);
		sen.type='W';
		strcpy(sen.name,"");
		strcpy(sen.pwd,"");
		strcpy(sen.buf,"no");

		sendto_client(cntfd,&sen,sizeof(sen));

		//获取时间戳
		char mytime[40]="";
		time_t t;
		time(&t);
		struct tm* info=localtime(&t);
		sprintf(mytime,"%d-%02d-%02d %02d-%02d-%02d",
				info->tm_year+1900,
				info->tm_mon+1,
				info->tm_mday,
				info->tm_hour,
				info->tm_min,
				info->tm_sec);


		//将查询到的数据加入到历史记录中
		bzero(sql,sizeof(sql));
		char s1[20]="";
		char s2[100]="";
		strcpy(s1,msg->buf);
		strcpy(s2,"数据库中没有该单词...");
		sprintf(sql,"insert into history values(\"%s\",\"%s\",\"%s\",\"%s\")",NAME,mytime,s1,s2);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0){
			fprintf(stderr,"sqlite3_exec %d:%s\n",__LINE__,errmsg);
			return ;
		}

	}
	return;	
}
//7、callback:数据库查询回调函数
int callback(void* arg,int f_num,char** f_value,char** f_name){
	printf("callback\n");
	int i;
	CB *cb=(CB*)arg;
	cb->flag=1;
	int cntfd=cb->cntfd;
	struct Msg sen;
	sen.type=cb->type;

	//获取时间戳
	char mytime[40]="";
	time_t t;
	time(&t);
	struct tm* info=localtime(&t);
	sprintf(mytime,"%d-%02d-%02d %02d-%02d-%02d",
			info->tm_year+1900,
			info->tm_mon+1,
			info->tm_mday,
			info->tm_hour,
			info->tm_min,
			info->tm_sec);

	strcpy(sen.name,"");
	strcpy(sen.pwd,"");
	//将查询的数据发送给客户端{
	sprintf(sen.buf,"%s %s",f_value[0],f_value[1]);
	sendto_client(cntfd,&sen,sizeof(sen));
	//将查询到的数据加入到历史记录中
	bzero(sql,sizeof(sql));
	sprintf(sql,"insert into history values(\"%s\",\"%s\",\"%s\",\"%s\")",NAME,mytime,f_value[0],f_value[1]);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=0){
		fprintf(stderr,"sqlite3_exec %d:%s\n",__LINE__,errmsg);
		return -1;
	}
	return 0;
}
//8、get_history:获取查询历史
void get_history(int cntfd,struct Msg* msg){
	struct Msg sen;
	//将该用户的所有历史信息发送给客户端
	bzero(sql,sizeof(sql));
	CB cb;
	cb.type='H';
	cb.cntfd=cntfd;
	cb.flag=0;
	sprintf(sql,"select * from history where name=\"%s\"",NAME);
	if(sqlite3_exec(db,sql,callback_history,&cb,&errmsg)!=0){
		fprintf(stderr,"sqlite3_exec %d:%s\n",__LINE__,errmsg);
		return;
	}
	if(cb.flag==0){
		//没有历史记录
		fprintf(stderr,"未查询到历史记录\n");
		sen.type='H';
		strcpy(sen.name,"");
		strcpy(sen.pwd,"");
		strcpy(sen.buf,"no");
		
		sendto_client(cntfd,&sen,sizeof(sen));
		
		return;
	}

	sen.type='H';
	strcpy(sen.name,"");
	strcpy(sen.pwd,"");
	strcpy(sen.buf,"finish");

	sendto_client(cntfd,&sen,sizeof(sen));
	return;
}
//9、登陆的查询函数
int callback_login(void* arg,int f_num,char** f_value,char** f_name){
	printf("callback_login\n");
	struct Msg sen;
	CBL *cbl=(CBL*)arg;
	cbl->flag=1;
	int cntfd=cbl->cntfd;
	struct Msg* msg=cbl->msg;
	printf("%d->\n",__LINE__);

	//用户存在时
	fprintf(stderr,"用户存在,可以登陆...\n");
	strcpy(NAME,cbl->msg->name);

	bzero(&sen,sizeof(sen));
	sen.type='L';
	strcpy(sen.name,msg->name);
	strcpy(sen.pwd,msg->pwd);
	strcpy(sen.buf,"ok");

	sendto_client(cntfd,&sen,sizeof(sen));

	return 0;
}
//10、callback_regist:注册用户的回调函数
int callback_regist(void* arg,int f_num,char** f_value,char** f_name){

	printf("callback_regist\n");
	struct Msg sen;
	CBL *cbl=(CBL*)arg;
	cbl->flag=1;
	int cntfd=cbl->cntfd;
	struct Msg* msg=cbl->msg;

	//用户存在给客户端发送信息
	fprintf(stderr,"用户已经存在，无需注册...\n");
	//发送消息
	bzero(&sen,sizeof(sen));
	sen.type='R';
	strcpy(sen.name,msg->name);
	strcpy(sen.pwd,msg->pwd);
	strcpy(sen.buf,"no");

	sendto_client(cntfd,&sen,sizeof(sen));

	return 0;

}
//11、callback_history:数据库查询回调函数
int callback_history(void* arg,int f_num,char** f_value,char** f_name){
	printf("callback_history\n");
	int i;
	CB *cb=(CB*)arg;
	cb->flag=1;
	int cntfd=cb->cntfd;
	struct Msg sen;
	sen.type=cb->type;

	strcpy(sen.name,"");
	strcpy(sen.pwd,"");
	//将查询的数据发送给客户端{
	sprintf(sen.buf,"%s %s %s",f_value[1],f_value[2],f_value[3]);
	sendto_client(cntfd,&sen,sizeof(sen));
	return 0;
}

