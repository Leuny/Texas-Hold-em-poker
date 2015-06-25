#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include "decision.h"
#include "list.h"

int socket_fd = -1;//套接字fd 
int gameround=0; //游戏圈数 
CurBoard curboard; //存储当前局信息的结构体 
char buf[20];  //存储待发送的行动消息 
unsigned char position; //存储我处在的位置 
unsigned char Action;//存储对手行动 
List *IdList; //存储 玩家ID的链表指针 

bool timeout=false;
bool finish=false;
pthread_t pid1,pid2;

//牌数组 
const int Card[52]={
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,	//红桃A - K
0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,	//方片A - K
0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,	//梅花A - K
0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,	//黑桃A - K
};


unsigned char process_seat_msg(char*buf,char* pid)
{

	/*
	seat/ 
	button: 6666 2000 6000 
	small blind: 8888 2000 6000 
	big blind: 1111 2000 6000 
	5555 14000 8000 
	4444 2000 8000 
	2222 2000 6000 
	7777 2000 6000 
	3333 2000 6000 
	/seat
	*/
	//printf("start precess seat-msg\n");
	//将玩家放入列表 
	int i=0;
	char num[23];
	char buffer[2048];
	strcpy(buffer,buf);
	IdList=CreateList();
	AppendId(IdList,buffer);
	
	//计算开牌时玩家数量 
	curboard.player_num=GetLength(IdList);
	
	unsigned char index=1;
	char* delim1=strchr(buf,':');
	char* delim2;
	char* PtrToPid;
	while(delim1!=NULL)
	{
		PtrToPid=delim1+2;
		delim2=strchr(PtrToPid,' ');
		if(!delim2)break;
		*delim2='\0';
		if(!strcmp(PtrToPid,pid))
		{
			break;
		}
		index++;
		delim1=strchr(delim2+1,':');
	}
	if(delim1==NULL)//not find pid in previous
	{
		while(1)//not find pid
		{
			delim1=strchr(delim2+1,'\n');
			if(!delim1)
			{
				break;
			}
			PtrToPid=delim1+1;
			delim2=strchr(PtrToPid,' ');
			if(!delim2)
			{
				break;
			}
			*delim2='\0';
			if(!strcmp(PtrToPid,pid))
			{
				break;	
			}
			
			index++;
		}
	}
	//printf("finish precess seat-msg\n");
	//玩家有8个时位置的分析 
	if(curboard.player_num==8)
	{
		if(index==8||index==1)
			return 2;
		else if(index==2)
			return 3;
		else if(index==3)
			return 4;
		else if(index==4||index==5)
			return 0;
		else return 1;
	}
	//玩家有7个时位置的分析 
	else if(curboard.player_num==7)
	{
		if(index==1||index==7)
			return 2;
		else if(index==2)
			return 3;
		else if(index==3)
			return 4;
		else if(index==4)
			return 0;
		else return 1;
	}
	//玩家有6个时位置的分析 		
	else if(curboard.player_num==6)
	{
	
		if(index==1)
			return 2;
		else if(index==2)
			return 3;
		else if(index==3)
			return 4;
		else if(index==4)
			return 0;
		else return 1;	
	}
	//玩家有5个以下时位置进行折中处理 	
	else return 2;			
}

void process_holdcards_msg(char*BUF)
{
	//printf("start process hold msg\n");
	int i=0,point=2;
	char p;
	char color;
	char* delim1;
	char* delim2;
	char* delim3;
	delim1=strchr(BUF,'\n');
	while(i<2)
	{
		if(!delim1)break;
		color=*(delim1+1);
		delim2=strchr(delim1+1,' ');
		if(!delim2)break;
		p=*(delim2+1);
		if(p!='1'&&p!='J'&&p!='Q'&&p!='K'&&p!='A')
		{
			point=p-'0';	
		}
		else
		{
			switch(p)
			{
				case '1':point=10;break;
				case 'J':point=11;break;
				case 'Q':point=12;break;
				case 'K':point=13;break;
				case 'A':point=1;break;
				default:break;
			}		
		}
		delim3=strchr(delim2+1,'\n');
		delim1=delim3;
	
		switch(color)
		{
			//黑桃 
			case 'S':
				curboard.PockCard[i]=0x30+point;
				break;
			//红桃 
			case 'H':
				curboard.PockCard[i]=point;
				break;
			//梅花 
			case 'C':
				curboard.PockCard[i]=0x20+point;
				break;
			//方片 
			case 'D':
				curboard.PockCard[i]=0x10+point;
				break;
			default:break;
			
		}
			i++;
	}
	//printf("finish process hold msg\n");
}

void process_flop_msg(char*BUF)
{
	//printf("start process flop msg\n");
	int i=0,point=2;
	char p;
	char color;
	char* delim1;
	char*delim2;
	char*delim3;
	delim1=strchr(BUF,'\n');
	while(i<3)
	{
		if(!delim1)break;
		color=*(delim1+1);
		delim2=strchr(delim1+1,' ');
		if(!delim2)break;
		p=*(delim2+1);
		if(p!='1'&&p!='J'&&p!='Q'&&p!='K'&&p!='A')
		point=p-'0';
		else
		{
			switch(p)
			{
				case '1':point=10;break;
				case 'J':point=11;break;
				case 'Q':point=12;break;
				case 'K':point=13;break;
				case 'A':point=1;break;
				default:break;
			}		
		}
		delim3=strchr(delim2+1,'\n');
		delim1=delim3;

		switch(color)
		{
			//黑桃 
		case 'S':
			curboard.BoardCard[i]=0x30+point;
			break;
		//红桃 
		case 'H':
			curboard.BoardCard[i]=point;
			break;
		//梅花 
		case 'C':
			curboard.BoardCard[i]=0x20+point;
			break;
		//方片 
		case 'D':
			curboard.BoardCard[i]=0x10+point;
			break;
		default:break;
			
		}
			i++;
	}
	//printf("finish process flop msg\n");
}

void process_turn_msg(char*BUF)
{
	//printf("start process turn msg\n");
	int i=0,point=2;
	char p;
	char color;
	char* delim1;
	char*delim2;
	char*delim3;
	delim1=strchr(BUF,'\n');
	if(!delim1)return;
	color=*(delim1+1);
	delim2=strchr(delim1+1,' ');
	if(!delim2)return;
	p=*(delim2+1);
	if(p!='1'&&p!='J'&&p!='Q'&&p!='K'&&p!='A')
		point=p-'0';
	else
	{
		switch(p)
		{
			case '1':point=10;break;
			case 'J':point=11;break;
			case 'Q':point=12;break;
			case 'K':point=13;break;
			case 'A':point=1;break;
			default:break;
		}		
	}
	
	switch(color)
	{
		//黑桃 
		case 'S':
			curboard.BoardCard[3]=0x30+point;
			break;
		//红桃 
		case 'H':
			curboard.BoardCard[3]=point;
			break;
		//梅花 
		case 'C':
			curboard.BoardCard[3]=0x20+point;
			break;
		//方片 
		case 'D':
			curboard.BoardCard[3]=0x10+point;
			break;
		default:break;
	}
	//printf("finish process turn msg\n");
}

void process_river_msg(char*BUF)
{
	//printf("start process river msg\n");
	int i=0,point=2;
	char color;
	char p;
	char* delim1;
	char*delim2;
	char*delim3;
	delim1=strchr(BUF,'\n');
	if(!delim1)return;
	color=*(delim1+1);
	delim2=strchr(delim1+1,' ');
	if(!delim2)return;
	p=*(delim2+1);
	if(p!='1'&&p!='J'&&p!='Q'&&p!='K'&&p!='A')
		point=p-'0';
	else
	{
		switch(p)
		{
			case '1':point=10;break;
			case 'J':point=11;break;
			case 'Q':point=12;break;
			case 'K':point=13;break;
			case 'A':point=1;break;
			default:break;
		}		
	}
	
	switch(color)
	{
		//黑桃 
		case 'S':
			curboard.BoardCard[4]=0x30+point;
			break;
		//红桃 
		case 'H':
			curboard.BoardCard[4]=point;
			break;
		//梅花 
		case 'C':
			curboard.BoardCard[4]=0x20+point;
			break;
		//方片 
		case 'D':
			curboard.BoardCard[4]=0x10+point;
			break;
		default:break;
	}
	//printf("finish process river msg\n");
}


int GetBetOfPlayer(char *buf,List *h)
{

	//printf("enter bet of player\n");
	List *L,*where;
	char buff[2048];
	strcpy(buff,buf);
	
	char* delim1=strchr(buf,'\n');
	if(delim1==NULL)return -1;
	char *ID_start,*ID_end;
	/*将当前局所有玩家投的注额填入列表*/
	char *bet_start,*bet_end;
	while(true)
	{
		ID_start=delim1+1;
		ID_end=strchr(ID_start,' ');
		if(!ID_end) break;
		*ID_end='\0';
		L=search(h,ID_start); 
		
		if(L!=NULL)
		{
			;
		}
		else
		{
			bet_end=ID_end+1;
		}
		
		//在列表里找到要找的ID 
		if(L!=NULL)
		{
			char *delim2=strchr(ID_end+1,' ');
			if(!delim2)break; 
			
			char *delim3=strchr(delim2+1,' ');
			if(!delim3)break;
			
			bet_start=delim3+1;
			bet_end=strchr(bet_start,' ');
			if(!bet_end)break;
			*bet_end='\0';
			int bet=atoi(bet_start);
			
			char *prebet=strchr(L->name,' ');
			if(!prebet)break;
			int PreBet=atoi(prebet+1);
	
			char curbet[10];
			snprintf(curbet, sizeof(curbet) - 1, "%d",bet-PreBet);
			strcpy(prebet+1,curbet);
	
		}
		char *delim4=strchr(bet_end+1,'\n');
		if(delim4==NULL)break;
		delim1=delim4;	
	}
	
	/*在inquire消息中查找第一个不是fold的玩家*/ 
	char* first;
	char *lim1=strchr(buff,'\n');
	while(1)
	{
		
		char* lim2=strchr(lim1+1,' ');
		if(lim2==NULL)break;
		char* lim3=strchr(lim2+1,' '); 
		if(lim3==NULL)break;
		char* lim4=strchr(lim3+1,' ');
		if(lim4==NULL)break;
		char* lim5=strchr(lim4+1,' '); 
		if(lim5==NULL)break;
		
		char action=*(lim5+1);	
		if(action!='f')
		{
			first=lim1+1;
			char *space=strchr(first,' ');
			if(!space)break;
			*space='\0';
			break;
		}
		lim1=strchr(lim2,'\n');
		if(!lim1)break;		
	}
	
	if(!first)return -1;
	where=search(h,first);
	if(!where)return -1;
	char *Begin=strchr(where->name,' ');
	if(!Begin)
	{	
		return -1;	
	}	
	int BET=atoi(Begin+1);
	//printf("out bet of player\n");
	
	return BET;	
} 

unsigned char process_inquire_msg(char*BUF)
{
	//printf("start process inquire msg\n");
	/*
	send inquire msg to player 8888 
	inquire/ 
	6666 1700 6000 300 raise 
	3333 1800 6000 200 call 
	7777 1800 6000 200 call 
	2222 1800 6000 200 call 
	4444 2000 8000 0 fold 
	5555 13800 8000 200 raise 
	1111 1900 6000 100 blind 
	8888 1950 6000 50 blind 
	total pot: 1250 
	/inquire 
	recv from player 8888: fold
	*/
	//printf("start process inquire msg\n");	
	char buf1[2048],buf2[2048];
	strcpy(buf1,BUF);
	strcpy(buf2,BUF);
		
	int fold_num=0,call_num=0,raise_num=0,blind_num=0;
	char* delim1=strchr(buf1,'\n');
	if(delim1==NULL)return 0;
	int i=0;
	char *ID_start,*ID_end;
	while(true)
	{	
		ID_start=delim1+1;
		char* delim2=strchr(delim1+1,' ');
		/*ID_start--ID_end提取玩家ID*/
		if(delim2!=NULL)
		{
			ID_end=delim2;
			*ID_end='\0';	
		}
		
		if(delim2==NULL)break;
		char* delim3=strchr(delim2+1,' '); 
		if(delim3==NULL)break;
		char* delim4=strchr(delim3+1,' ');
		if(delim4==NULL)break;
		char* delim5=strchr(delim4+1,' '); 
		if(delim5==NULL)break;
		
		char action=*(delim5+1);
		switch(action)
		{
			case 'f':
				fold_num++;
				if(ID_start!=NULL)
				Delet(IdList,ID_start);//有玩家弃牌，将该玩家从玩家列表删除 
				break;
			case 'c':call_num++;break;
			case 'r':raise_num++;break;
			case 'b':blind_num++;break;
			case 'a':raise_num++;break;//all_in当做加注算
			default:break;
		}
		char *delim6=strchr(delim5+1,'\n');
		if(delim6==NULL)break;
		delim1=delim6;
		i++;	
	}
	//printf("finish process inquire msg\n");
	
	/*计算当前上家投注额*/
	curboard.curbet=GetBetOfPlayer(buf2,IdList);
	if(curboard.curbet<=0)curboard.curbet=0;
	
	/*计算玩家列表长度，从而得出当前正在游戏玩家的个数*/ 
	curboard.player_num=GetLength(IdList);
	if(curboard.player_num<=0)curboard.player_num=1;

	/*如果玩家个数较多，则对手的行动信息很重要，需要进行统计*/ 
	if(curboard.player_num>=4)
	{
		if(fold_num==i-1-blind_num)return 0;
		else if(call_num==1&&raise_num==0)return 1;
		else if(call_num>=2&&raise_num==0)return 2;
		else if(call_num==0&&raise_num==1)return 3;
		else if(call_num>=1&&raise_num==1)return 4;
		else return 5;	
	}
	/*玩家较少，对手行动信息没那么重要，将对手的行动折中处理*/ 
	else return 1;	
}


void *Timeout(void* parm)
{
	//printf("i am in time thread\n");
	timeout=false;
	struct timeval start, end;
	//开始计时
	gettimeofday(&start, NULL);
	while(1)
	{
		usleep(1);
		gettimeofday(&end, NULL);
		int interval = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
		if((interval/1000.0)>400)
		{
			timeout=true;
			break;
		}
		pthread_testcancel();
	} 
	//printf("i am out time thread\n");	
}

void *Caculate(void* buf)
{
	//printf("i am in caculate thread\n");
	int i=6000;
	finish=false;
	//计算过程 
	char* p=(char*)buf;
	Action=process_inquire_msg(p);//对手的行动 
	switch(gameround)
	{
		//preflop 
		case 0:	
			PreFlodDecision(curboard.PockCard,Action,position,&curboard);//preflop 决策	
			break;
		//flop 
		case 3:
			Decision(&curboard,3); 
			break;
		//turn
		case 4:
			Decision(&curboard,4);
			break;
		//river 
		case 5:
			Decision(&curboard,5); 
			break;
		default:break;
	}
	finish=true;
	//printf("i am out caculate thread\n");
}

/* 处理server的消息 */
int process_server_message(int fd, char* buffer,char *id)
{	
	char msg[2048];
	char* cp;
	if(buffer)
	strcpy(msg,buffer);
	msg[4]='\0';
	//printf("process server msg\n");
	
	/*处理seat-msg*/ 
	if(strcmp(msg,"seat")==0)
	{	
		/*
		可能的粘包格式： 
		seat/ 
		button: 1111 2000 8000 
		small blind: 5555 2000 8000 
		big blind: 7777 2000 8000 
		4444 2000 8000 
		3333 2000 8000 
		6666 2000 8000 
		2222 2000 8000 
		8888 2000 8000 
		/seat 
		blind/ 
		5555: 50 
		7777: 100 
		/blind 
		hold/ 
		HEARTS 5 
		CLUBS 5 
		/hold
		*/ 
		gameround=0;
		position=process_seat_msg(buffer,id);
		/*以下处理粘包*/
		cp=strstr(msg+5,"hold");
		if(cp!=NULL)
		{
			//收到手牌消息 
			//printf("receive hold-msg\n");
			process_holdcards_msg(cp); 
		}
	}
	/*处理gameover-msg*/
	else if(strcmp(msg,"game")==0)
	{
		//printf("gameover-msg\n");
		return -1;
	}
	/*处理blind-msg*/
	else if(strcmp(msg,"blin")==0)
	{
		//printf("receive bind-msg\n");
		/*
		可能的粘包格式：	
		blind/ 
		5555: 50 
		7777: 100 
		/blind 
		hold/ 
		HEARTS 5 
		CLUBS 5 
		/hold
		*/ 
		/*以下处理粘包*/
		cp=strstr(buffer+4,"hold");
		if(cp!=NULL)
		{
			//printf("receive hold-msg\n");
			//收到手牌消息 
			process_holdcards_msg(cp);
		}	
	}
	/*处理hold-msg*/
	else if(strcmp(msg,"hold")==0)
	{
		//printf("receive hold-msg\n");
		process_holdcards_msg(buffer);
		
	}
	/*处理inquire-msg*/
	else if(strcmp(msg,"inqu")==0)
	{	
	
		//printf("receive inquire-msg\n");
		/*收到询问消息，创建两个线程，线程1用于计时，线程2用于计算*/ 
		timeout=false;
		finish=false;
		//printf("create time thread\n");
		pthread_create(&pid1,NULL,Timeout,NULL);//开启计时线程
		//printf("end create time thread\n");
		//printf("create caculate thread\n");
		pthread_create(&pid2,NULL,Caculate,buffer); //开启计算线程 
		//printf("end create caculate thread\n");
		while(finish==false&&timeout==false)
		{
			;//等待	
		}
		if(finish)
		{
			//计算完成 
			//printf("finish..\n");
			send(fd, buf, strlen(buf) + 1, 0);
			pthread_cancel(pid1);
		}
		else if(timeout)
		{
			//计算超时
			//printf("timeout..\n");
			strcpy(buf,"fold\n");
			send(fd, buf, strlen(buf) + 1, 0);
			pthread_cancel(pid2);
		}
		pthread_join(pid1,NULL);
		pthread_join(pid2,NULL);	
	}
	/*处理flop-msg*/
	else if(strcmp(msg,"flop")==0)
	{
		//printf("receive flop-msg\n");
		gameround=3;
		process_flop_msg(buffer);
		
	}
	/*处理turn-msg*/
	else if(strcmp(msg,"turn")==0)
	{
		//printf("receive turn-msg\n");
		gameround=4;
		process_turn_msg(buffer); 
	}
	/*处理river-msg*/
	else if(strcmp(msg,"rive")==0)
	{
		//printf("receive river-msg\n");
		gameround=5;
		process_river_msg(buffer);
	}
	/*处理showdown-msg*/
	else if(strcmp(msg,"show")==0)
	{
		//printf("receive show-msg\n");
		gameround=0;
	}
	/*处理pot-win-msg*/
	else if(strcmp(msg,"pot-")==0)
	{
		//printf("receive pot-win-msg\n");
		gameround=0;
	}
	return 0;
}

int main(int argc, char *argv[])
{

	/*验证输入参数个数*/ 
	if (argc< 6)
	{
		printf("Usage: <%s server_ip server_port my_ip my_port id>\n", argv[0]);
		return -1;
	}
	/* 获取输入参数 */
	in_addr_t server_ip = inet_addr(argv[1]);
	in_port_t server_port = htons(atoi(argv[2])); 
	in_addr_t my_ip = inet_addr(argv[3]);
	in_port_t my_port = htons(atoi(argv[4])); 
	int id = atoi(argv[5]);
	
	/* 创建socket*/
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd< 0)
	{
		perror("create socket failed!\n");
		return -1;
	}
	
	/*设置socket选项，地址重复使用，防止程序非正常退出导致下次启动时bind失败 */
	int is_reuse_addr = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&is_reuse_addr, sizeof(is_reuse_addr));
	
	/*绑定指定ip和port，防止被server拒绝 */
	struct sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = my_ip;
	my_addr.sin_port = my_port;
	if(bind(socket_fd, (struct sockaddr*)&my_addr, sizeof(my_addr)))
	{
		perror("bind failed!\n"); 
		return -1;
	}
	
	/*连接server*/
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = server_ip;
	server_addr.sin_port = server_port;
	while(connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		/*启动失败时睡眠100ms再重试，这样保证无论server先起还是后起，都不会有问题*/
		usleep(100*1000); 
	}
	
	/*向server发送注册消息*/
	char reg_msg[50] = {'\0'};
	snprintf(reg_msg, sizeof(reg_msg) - 1, "reg: %d %s \n", id, "gql"); 
	send(socket_fd, reg_msg, strlen(reg_msg) + 1, 0);
	
	/* 开始接收服务器消息，进行德州扑克游戏*/
	while(1)
	{
		//先将接收buffer清空 
		char buffer[2048]={'\0'};
		//printf("i am waiting for msg\n");
		int len=recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
		//printf("i have received msg from server len=%d\n",len);
		int re; 
		if(len>0)
		{  
			re=process_server_message(socket_fd, buffer,argv[5]);
			if(re<0)break;//收到gameover消息，该场游戏结束，退出循环 
		}
	}	
	/*关闭socket连接*/
	close(socket_fd);	
	return 0;
}
