/*************************************************************************
    > File Name: file_client.c
    > Author: gll
    > Mail: lzs123@911gll.cn 
    > Created Time: Fri 21 Sep 2018 08:54:44 AM CST
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#pragma pack(1)
//文件描述信息
typedef struct _file_info
{
	char name[51];
	unsigned int size;

}file_info;

#pragma pack()

int main(int argc, char **argv)
{
	if(3 != argc)
	{
		fprintf(stderr, "参数错误!");
	}

	//第一步:创建套接字
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);

/*//第二步:绑定地址(可以省略)
	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	myaddr.sin_port = htons(8888);

	if(bind(sock, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1)
	{
		perror("bind error");
		//exit(1);
	}
*/
	//第三步:连接服务器
	//指定服务器地址
	struct sockaddr_in srvaddr;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_addr.s_addr = inet_addr(argv[1]);
	srvaddr.sin_port = htons(atoi(argv[2]));

	//连接服务器地址
	if(connect(sock, (struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
	{
		perror("connect error");
		exit(1);
	}
	
	int ret = 0, ret1 = 0;
	
	file_info fi;

	ret = recv(sock, &fi, sizeof(fi), 0);	
	

	char *buff = (char *)malloc(1024 * 1024); 

	if(ret > 0)
	{	
		FILE *fp = NULL;

		fp = fopen(fi.name, "wb");
	
		if(NULL == fp)
		{
			perror("fopen");
			exit(1);
		}	
		
		while(ret1 < fi.size)		
		{	
			ret = recv(sock, buff, 1024 * 1024, 0);
			fwrite(buff, 1, ret, fp);
			ret1 += ret;
			printf("正在传输>>>>>>%.2f%\n", ((float)ret1 / (float)fi.size) * 100);
			usleep(100000);
			system("clear");
		}
		fclose(fp);
	}
	
	free(buff);
	close(sock);

	return 0;
}
