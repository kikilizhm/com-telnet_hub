

/*************************************
文件名： client.c 
linux 下socket网络编程简例  - 客户端程序
服务器端口设为 0x8888   （端口和地址可根据实际情况更改，或者使用参数传入）
服务器地址设为 192.168.1.104
作者:kikilizhm#163.com (将#换为@)
*/
 
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <string.h>


#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>
 
int connect_ser(char *ip, unsigned short portnum)
{
int cfd; /* 文件描述符 */
int recbytes;
int sin_size;
char buffer[1024]={0};    /* 接受缓冲区 */
struct sockaddr_in s_add,c_add; /* 存储服务端和本端的ip、端口等信息结构体 */
//unsigned short portnum=0x8888;  /* 服务端使用的通信端口，可以更改，需和服务端相同 */
 
printf("Hello,welcome to client !\r\n");
/* 建立socket 使用因特网，TCP流传输 */
cfd = socket(AF_INET, SOCK_STREAM, 0);
if(-1 == cfd)
{
    printf("socket fail ! \r\n");
    return -1;
}
printf("socket ok !\r\n");
/* 构造服务器端的ip和端口信息，具体结构体可以查资料 */
bzero(&s_add,sizeof(struct sockaddr_in));
s_add.sin_family=AF_INET;
s_add.sin_addr.s_addr= inet_addr(ip); /* ip转换为4字节整形，使用时需要根据服务端ip进行更改 */
s_add.sin_port=htons(portnum); /* 这里htons是将short型数据字节序由主机型转换为网络型，其实就是
    将2字节数据的前后两个字节倒换，和对应的ntohs效果、实质相同，只不过名字不同。htonl和ntohl是
    操作的4字节整形。将0x12345678变为0x78563412，名字不同，内容两两相同，一般情况下网络为大端，
    PPC的cpu为大端，x86的cpu为小端，arm的可以配置大小端，需要保证接收时字节序正确。
 */

printf("s_addr = %#x ,port : %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port); /* 这里打印出的是小端
    和我们平时看到的是相反的。 */
 
/* 客户端连接服务器，参数依次为socket文件描述符，地址信息，地址结构大小 */
if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
{
    printf("connect fail !\r\n");
    return -1;
}
printf("connect ok !\r\n");

return cfd; 
#if 0
/*连接成功,从服务端接收字符*/
if(-1 == (recbytes = read(cfd,buffer,1024)))
{
    printf("read data fail !\r\n");
    return -1;
}
printf("read ok\r\nREC:\r\n");
 
buffer[recbytes]='\0';
printf("%s\r\n",buffer);
 
getchar(); /* 此句为使程序暂停在此处，可以使用netstat查看当前的连接 */
close(cfd); /* 关闭连接，本次通信完成 */
return 0;

#endif
}


struct cominfo {
    char *name;
    int fd;
};
#define MAX_CLI_NUM  20
#define MAX_CONNECT_NUM 20

struct telnetinfo {
    char ser_ip[20];  /* 待链接的服务器ip */
    unsigned short ser_port; /* 待链接的服务器端口 */
    int con_ser_fd;
    int ser_fd;

    unsigned short share_port;  /* 待分享的端口 */
    int cli_fd[MAX_CLI_NUM];
};

#define TYPE_COM 0
#define TYPE_TELNET  1

struct hubinfo {

    int type; /* 0 com or 1 telnet */
    pthread_t pth_id;
    struct cominfo com;
    struct telnetinfo telnet;
};



/*************************************
文件名： server.c 
linux 下socket网络编程简例  - 服务端程序
服务器端口设为 0x8888   （端口和地址可根据实际情况更改，或者使用参数传入）
服务器地址设为 192.168.1.104
作者:kikilizhm#163.com (将#换为@)
*/
 

#include <fcntl.h>

int socket_ser_start(/*unsigned short portnum*/  struct hubinfo *info)
{
    int sfp,nfp; /* 定义两个描述符 */
    struct sockaddr_in s_add,c_add;
    int sin_size;
    unsigned short portnum = info->telnet.share_port;

    printf("Hello,welcome to my server !\r\n");
    sfp = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sfp)
    {
        printf("socket fail ! \r\n");
        return -1;
    }
    fcntl(sfp,F_SETFL,O_NONBLOCK);
    
    printf("socket ok !\r\n");
    
    /* 填充服务器端口地址信息，以便下面使用此地址和端口监听 */
    bzero(&s_add,sizeof(struct sockaddr_in));
    s_add.sin_family=AF_INET;
    s_add.sin_addr.s_addr=htonl(INADDR_ANY); /* 这里地址使用全0，即所有 */
    s_add.sin_port=htons(portnum);
    /* 使用bind进行绑定端口 */
    if(-1 == bind(sfp,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
    {
        printf("bind fail !\r\n");
        return -1;
    }
    printf("bind ok !\r\n");
    /* 开始监听相应的端口 */
    if(-1 == listen(sfp,5))
    {
        printf("listen fail !\r\n");
        return -1;
    }
    printf("listen ok\r\n");
    return sfp;
 #if 0
    while(1)
    {
        sin_size = sizeof(struct sockaddr_in);
        /* accept服务端使用函数，调用时即进入阻塞状态，等待用户进行连接，在没有客户端进行连接时，程序停止在此处，
        不会看到后面的打印，当有客户端进行连接时，程序马上执行一次，然后再次循环到此处继续等待。
        此处accept的第二个参数用于获取客户端的端口和地址信息。
            */
        nfp = accept(sfp, (struct sockaddr *)(&c_add), &sin_size);
        if(-1 == nfp)
        {
            printf("accept fail !\r\n");
            return -1;
        }
        printf("accept ok!\r\nServer start get connect from %#x : %#x\r\n",ntohl(c_add.sin_addr.s_addr),ntohs(c_add.sin_port));
        
        /* 这里使用write向客户端发送信息，也可以尝试使用其他函数实现 */
        if(-1 == write(nfp,"hello,welcome to my server \r\n",32))
        {
            printf("write fail!\r\n");
            return -1;
        }
        printf("write ok!\r\n");
        close(nfp);
    
    }
    close(sfp);
    return 0;
    #endif
}

int max(int x, int y)
{
    return (x > y) ? x : y;
}

int connect_telnet(struct hubinfo *info)
{
    struct telnetinfo *tinfo = &info->telnet;
    int fd = 0;
    fd_set rfds;
    struct sockaddr_in ipadd;
    struct timeval timeout = {3,0};
    char * readbuff[10] = {0};
    int ret  = 0;
    int share_ser_fd = -1;
    int max_fd = 0;
    int i = 0;
    int j = 0;

    fd = connect_ser(tinfo->ser_ip, tinfo->ser_port);
    if(fd < 0)
    {
        printf("connect ser %s:%d fail.\r\n", tinfo->ser_ip, tinfo->ser_port);
        return -1;
    }
    tinfo->con_ser_fd = fd;


    share_ser_fd = socket_ser_start(info);
    if(share_ser_fd < 0)
    {
        printf("start share servers fail, ret %d.\r\n", share_ser_fd);
        return -1;
    }
    tinfo->ser_fd = share_ser_fd;
    while(1)
    {
        FD_ZERO(&rfds);  /* 清空集合 */
        FD_SET(fd, &rfds);  /* 将fp添加到集合，后面的FD_ISSET和FD_SET没有必然关系，这里是添加检测 */
        FD_SET(0, &rfds);  /* 将fp添加到集合，后面的FD_ISSET和FD_SET没有必然关系，这里是添加检测 */
        max_fd = max(fd, 0);
        FD_SET(share_ser_fd, &rfds);
        max_fd = max(max_fd, share_ser_fd);

        for(i=0; i<MAX_CLI_NUM; i++)
        {
            if(tinfo->cli_fd[i] != 0)
            {
                FD_SET(tinfo->cli_fd[i], &rfds);
                max_fd = max(max_fd, tinfo->cli_fd[i]);
            }            
        }

        ret=select(max_fd + 1, &rfds, NULL, NULL, &timeout);
        //printf("\r\n你好  select ret = %d",ret);
        if(0 > ret)
        {
                close(fd);
                //fclose(ffp);
                return -1;
        }
        else if(0 == ret)
        {
            //break;
        }
        else
        {
            if(FD_ISSET(fd,&rfds))  /* 这里检测的是fp在集合中是否状态变化，即可以操作。 */
            {
                //printf("ser has msg.\r\n");
                //ret = recv(fd, readbuff, 9, 0);
                ret = read(fd, readbuff, 1);
		        //if(0 == ret)
                if((0 != ret) && (-1 != ret))
                {
                    printf("%s",readbuff);
                    fflush(stdout);
                    for(i=0; i<MAX_CLI_NUM; i++)
                    {
                        if(tinfo->cli_fd[i] != 0)
                        {
                            write(tinfo->cli_fd[i], readbuff, 1);
                            
                        }
                    }
                }
                else if(ret == 0)
                {
                    printf("no msg form ser.\r\n");
                }
                else
                {
                    /* code */
                    printf("connect to server disconnect.\r\n");
                    close(fd);
                    return -1;
                }                
            }

            if(FD_ISSET(0,&rfds))  /* 这里检测的是fp在集合中是否状态变化，即可以操作。 */
            {
                //printf("tty has msg.\r\n");
                ret = read(0, readbuff, 1);
		        if((0 != ret) && (-1 != ret))     /* 此处需要检测！否则ftp发送数据时，后面会循环接收到0字节数据 */
                {
                    write(fd,readbuff,1);
                    printf("%s", readbuff);
                    fflush(stdout);

                    for(i=0; i<MAX_CLI_NUM; i++)
                    {
                        if(tinfo->cli_fd[i] != 0)
                        {
                            write(tinfo->cli_fd[i], readbuff, 1);
                            //fflush(stdout);
                        }
                    }

                }
                else
                {
                    printf("tty no msg.\r\n");
                }
                
                
            }
            for(i=0; i<MAX_CLI_NUM; i++)
            {
                if(tinfo->cli_fd[i] != 0)
                {
                    if(FD_ISSET(tinfo->cli_fd[i],&rfds))
                    {
                        ret = read(tinfo->cli_fd[i], readbuff, 1);
                        if((0 != ret) && (-1 != ret))     /* 此处需要检测！否则ftp发送数据时，后面会循环接收到0字节数据 */
                        {
                            write(fd,readbuff,1);
                            printf("%s", readbuff);
                            fflush(stdout);

                            for(j=0; j<MAX_CLI_NUM; j++)
                            {
                                if((tinfo->cli_fd[j] != 0) && (tinfo->cli_fd[j] != tinfo->cli_fd[i]))
                                {
                                    if(-1 == write(tinfo->cli_fd[j], readbuff, 1))
                                    {
                                        tinfo->cli_fd[j] = 0;
                                        close(tinfo->cli_fd[j]);
                                    }
                                    //fflush(stdout);
                                }
                            }

                        }
                        else
                        {
                            printf("tty no msg.\r\n");
                        }
                    }
                    
                }
            }

            if(FD_ISSET(share_ser_fd,&rfds))  /* 这里检测的是fp在集合中是否状态变化，即可以操作。 */
            {
                int cfp; /* 定义两个描述符 */
                struct sockaddr_in c_add;
                int sin_size;
                    sin_size = sizeof(struct sockaddr_in);
                    /* accept服务端使用函数，调用时即进入阻塞状态，等待用户进行连接，在没有客户端进行连接时，程序停止在此处，
                    不会看到后面的打印，当有客户端进行连接时，程序马上执行一次，然后再次循环到此处继续等待。
                    此处accept的第二个参数用于获取客户端的端口和地址信息。
                        */
                    //printf("new client .\r\n");
                    cfp = accept(share_ser_fd, (struct sockaddr *)(&c_add), &sin_size);
                    if(-1 == cfp)
                    {
                        printf("accept fail !\r\n");
                        return -1;
                    }

                    for(i=0; i<MAX_CLI_NUM; i++)
                    {
                        if(tinfo->cli_fd[i] == 0)
                        {
                            tinfo->cli_fd[i] = cfp;

                            break;
                        }
                    }
                    if(i >= MAX_CLI_NUM)
                    {
                        if(-1 == write(cfp,"server close this connect for over max connects.\r\n",32))
                        {
                            printf("write fail!\r\n");
                            return -1;
                        }
                        printf("close connect for over max connects.\r\n");
                        close(cfp);
                        continue;
                    }
                    //printf("accept ok!\r\nServer start get connect from %#x : %#x\r\n",ntohl(c_add.sin_addr.s_addr),ntohs(c_add.sin_port));
                    
                    /* 这里使用write向客户端发送信息，也可以尝试使用其他函数实现 */
                    #if 1
                    if(-1 == write(cfp,"hello,welcome to my server \r\n",32))
                    {
                        printf("write fail!\r\n");
                        tinfo->cli_fd[i] = 0;
                        close(cfp);
                    }
                    #else
                    printf("write ok!\r\n");
                    close(cfp);
                    #endif
            }
        }
    /*
        memset (readbuff,0,10);
        ret = read(0, readbuff, 1);
        if((-1 != ret) && (0 != ret))
        {
            write(fd,readbuff,1);
            printf("%s ret %d", readbuff,ret);
            fflush(stdout);
        }
 */
    }


}

#include <pthread.h>

int pthread_create(
    pthread_t *restrict thread,  /*线程id*/
    const pthread_attr_t *restrict attr,    /*线程属性，默认可置为NULL，表示线程属性取缺省值*/
    void *(*start_routine)(void*),  /*线程入口函数*/ 
    void *restrict arg  /*线程入口函数的参数*/
    );

struct hubinfo *g_hubinfo = NULL;


int telnet_proc(struct hubinfo *info)
{
    ;
}

// hub telnet ip port shareport 
// hub com comnum shareport 

int main(int argc, char *argv[])
{
    struct hubinfo *hub = malloc(sizeof(struct hubinfo) * MAX_CONNECT_NUM);
    if(hub == NULL)
    {
        printf("malloc fail, for hub\r\n");
        return -1;
    }

    g_hubinfo = hub;

    if((argc == 5) || (argc == 4))
    {
        if(0 == memcmp(argv[1], "telnet", 7))
        {
            printf("share telnet.\r\n");
            hub->type = TYPE_TELNET;
            memcpy(hub->telnet.ser_ip, "127.0.0.1", strlen("127.0.0.1"));
            hub->telnet.ser_port = 1026;
            hub->telnet.share_port = 2026;
            //connect_telnet(hub);

            pthread_create(&hub[0].pth_id, NULL, &connect_telnet, &hub[0]);
            while(1) sleep(1);
        }
        else if(0 == memcmp(argv[1], "com", 4))
        {
            printf("share com.\r\n");
            return 0;
        }
        else
        {
            printf("Useage:\r\nhub telnet ip port shareport;\r\nhub com comnum shareport\r\n");
            return -1;
        }
        
    }

    //pthread_create()

    printf("Useage:\r\nhub telnet ip port shareport;\r\nhub com comnum shareport\r\n");
    return -1;
}