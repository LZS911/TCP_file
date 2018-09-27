#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>


#pragma pack(1)

//文件描述信息
typedef struct _file_info
{
	char name[51];
	unsigned int size;

} file_info;

#pragma pack()


file_info fi;
const char* file_path;

void* send_thr(void* arg);


int main(int argc, char** argv)
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage error!\n");
		exit(1);
	}

	struct stat st;
	if(stat(argv[2], &st) == -1)
	{
		perror("Get file info fail");
		exit(1);
	}

	if(!S_ISREG(st.st_mode))
	{
		fprintf(stderr, "Sended file is not a regular file!\n");
		exit(1);
	}



	fi.size = st.st_size;

	char* p = NULL;
	p = strrchr(argv[2], '/');
	if(p == NULL)
		strcpy(fi.name, argv[2]);
	else
		strcpy(fi.name, p + 1);

	file_path = argv[2];
	

	signal(SIGPIPE, SIG_IGN);



	int sock_listen;
	sock_listen = socket(AF_INET, SOCK_STREAM, 0);

	//setsockopt函数：设置套接字属性
	//将套接字的SO_REUSEADDR属性设置为1，即允许地址复用
	int optval = 1;
	setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));


	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET; //指定地址家族为Internet地址家族
	myaddr.sin_addr.s_addr = INADDR_ANY; //指定IP地址为本机任意IP
	myaddr.sin_port = htons(atoi(argv[1])); //指定端口号

	if(bind(sock_listen, (struct sockaddr*)&myaddr, sizeof(myaddr)) == -1)
	{
		perror("bind error");
		exit(1);
	}

	listen(sock_listen, 5);

	struct sockaddr_in clnaddr;
	socklen_t len;

	while(1)
	{	
		int sock_conn; //连接套接字，用于和相应的客户端通信

		len = sizeof(clnaddr);
		sock_conn = accept(sock_listen, (struct sockaddr*)&clnaddr, &len);

		if(sock_conn == -1)
		{
			perror("accept error");
			continue;
		}

		pthread_t tid;
		if(pthread_create(&tid, NULL, send_thr, (void*)(long)sock_conn))
		{
			perror("create new thread fail");
			close(sock_conn);
		}
	}
	
	close(sock_listen);	
	
	return 0;
}


void* send_thr(void* arg)
{
	int sock_conn = (long)arg;

	pthread_detach(pthread_self());

	struct sockaddr_in clnaddr;
	socklen_t len = sizeof(clnaddr);
	getpeername(sock_conn, (struct sockaddr*)&clnaddr, &len);

	//接收客户端连接请求成功
	printf("\n客户端%s:%d上线！\n", inet_ntoa(clnaddr.sin_addr), ntohs(clnaddr.sin_port));

	//设置send和recv超时时间都为2S
	struct timeval timeout = {2, 0};
	setsockopt(sock_conn, SOL_SOCKET,SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
	setsockopt(sock_conn, SOL_SOCKET,SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

	if(-1 == send(sock_conn, &fi, sizeof(fi), 0))
	{
		printf("\n客户端%s:%d下线！\n", inet_ntoa(clnaddr.sin_addr), ntohs(clnaddr.sin_port));
		close(sock_conn);
		return NULL;
	}


	FILE* fp = NULL;
	int ret;
	char buff[1024];

	fp = fopen(file_path, "rb");	

	if(fp == NULL)
	{
		perror("Open sended file fail");
		exit(1);
	}

	while(!feof(fp))
	{
		ret = fread(buff, 1, sizeof(buff), fp);
		if(-1 == send(sock_conn, buff, ret, 0))
			break;
	}

	fclose(fp);


	printf("\n客户端%s:%d下线！\n", inet_ntoa(clnaddr.sin_addr), ntohs(clnaddr.sin_port));
	close(sock_conn);	

	return NULL;
}

