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

int socket_fd = -1;//�׽���fd 
int gameround=0; //��ϷȦ�� 
CurBoard curboard; //�洢��ǰ����Ϣ�Ľṹ�� 
char buf[20];  //�洢�����͵��ж���Ϣ 
unsigned char position; //�洢�Ҵ��ڵ�λ�� 
unsigned char Action;//�洢�����ж� 
List *IdList; //�洢 ���ID������ָ�� 

bool timeout=false;
bool finish=false;
pthread_t pid1,pid2;

//������ 
const int Card[52]={
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,	//����A - K
0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,	//��ƬA - K
0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,	//÷��A - K
0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,	//����A - K
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
	//����ҷ����б� 
	int i=0;
	char num[23];
	char buffer[2048];
	strcpy(buffer,buf);
	IdList=CreateList();
	AppendId(IdList,buffer);
	
	//���㿪��ʱ������� 
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
	//�����8��ʱλ�õķ��� 
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
	//�����7��ʱλ�õķ��� 
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
	//�����6��ʱλ�õķ��� 		
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
	//�����5������ʱλ�ý������д��� 	
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
			//���� 
			case 'S':
				curboard.PockCard[i]=0x30+point;
				break;
			//���� 
			case 'H':
				curboard.PockCard[i]=point;
				break;
			//÷�� 
			case 'C':
				curboard.PockCard[i]=0x20+point;
				break;
			//��Ƭ 
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
			//���� 
		case 'S':
			curboard.BoardCard[i]=0x30+point;
			break;
		//���� 
		case 'H':
			curboard.BoardCard[i]=point;
			break;
		//÷�� 
		case 'C':
			curboard.BoardCard[i]=0x20+point;
			break;
		//��Ƭ 
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
		//���� 
		case 'S':
			curboard.BoardCard[3]=0x30+point;
			break;
		//���� 
		case 'H':
			curboard.BoardCard[3]=point;
			break;
		//÷�� 
		case 'C':
			curboard.BoardCard[3]=0x20+point;
			break;
		//��Ƭ 
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
		//���� 
		case 'S':
			curboard.BoardCard[4]=0x30+point;
			break;
		//���� 
		case 'H':
			curboard.BoardCard[4]=point;
			break;
		//÷�� 
		case 'C':
			curboard.BoardCard[4]=0x20+point;
			break;
		//��Ƭ 
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
	/*����ǰ���������Ͷ��ע�������б�*/
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
		
		//���б����ҵ�Ҫ�ҵ�ID 
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
	
	/*��inquire��Ϣ�в��ҵ�һ������fold�����*/ 
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
		/*ID_start--ID_end��ȡ���ID*/
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
				Delet(IdList,ID_start);//��������ƣ�������Ҵ�����б�ɾ�� 
				break;
			case 'c':call_num++;break;
			case 'r':raise_num++;break;
			case 'b':blind_num++;break;
			case 'a':raise_num++;break;//all_in������ע��
			default:break;
		}
		char *delim6=strchr(delim5+1,'\n');
		if(delim6==NULL)break;
		delim1=delim6;
		i++;	
	}
	//printf("finish process inquire msg\n");
	
	/*���㵱ǰ�ϼ�Ͷע��*/
	curboard.curbet=GetBetOfPlayer(buf2,IdList);
	if(curboard.curbet<=0)curboard.curbet=0;
	
	/*��������б��ȣ��Ӷ��ó���ǰ������Ϸ��ҵĸ���*/ 
	curboard.player_num=GetLength(IdList);
	if(curboard.player_num<=0)curboard.player_num=1;

	/*�����Ҹ����϶࣬����ֵ��ж���Ϣ����Ҫ����Ҫ����ͳ��*/ 
	if(curboard.player_num>=4)
	{
		if(fold_num==i-1-blind_num)return 0;
		else if(call_num==1&&raise_num==0)return 1;
		else if(call_num>=2&&raise_num==0)return 2;
		else if(call_num==0&&raise_num==1)return 3;
		else if(call_num>=1&&raise_num==1)return 4;
		else return 5;	
	}
	/*��ҽ��٣������ж���Ϣû��ô��Ҫ�������ֵ��ж����д���*/ 
	else return 1;	
}


void *Timeout(void* parm)
{
	//printf("i am in time thread\n");
	timeout=false;
	struct timeval start, end;
	//��ʼ��ʱ
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
	//������� 
	char* p=(char*)buf;
	Action=process_inquire_msg(p);//���ֵ��ж� 
	switch(gameround)
	{
		//preflop 
		case 0:	
			PreFlodDecision(curboard.PockCard,Action,position,&curboard);//preflop ����	
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

/* ����server����Ϣ */
int process_server_message(int fd, char* buffer,char *id)
{	
	char msg[2048];
	char* cp;
	if(buffer)
	strcpy(msg,buffer);
	msg[4]='\0';
	//printf("process server msg\n");
	
	/*����seat-msg*/ 
	if(strcmp(msg,"seat")==0)
	{	
		/*
		���ܵ�ճ����ʽ�� 
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
		/*���´���ճ��*/
		cp=strstr(msg+5,"hold");
		if(cp!=NULL)
		{
			//�յ�������Ϣ 
			//printf("receive hold-msg\n");
			process_holdcards_msg(cp); 
		}
	}
	/*����gameover-msg*/
	else if(strcmp(msg,"game")==0)
	{
		//printf("gameover-msg\n");
		return -1;
	}
	/*����blind-msg*/
	else if(strcmp(msg,"blin")==0)
	{
		//printf("receive bind-msg\n");
		/*
		���ܵ�ճ����ʽ��	
		blind/ 
		5555: 50 
		7777: 100 
		/blind 
		hold/ 
		HEARTS 5 
		CLUBS 5 
		/hold
		*/ 
		/*���´���ճ��*/
		cp=strstr(buffer+4,"hold");
		if(cp!=NULL)
		{
			//printf("receive hold-msg\n");
			//�յ�������Ϣ 
			process_holdcards_msg(cp);
		}	
	}
	/*����hold-msg*/
	else if(strcmp(msg,"hold")==0)
	{
		//printf("receive hold-msg\n");
		process_holdcards_msg(buffer);
		
	}
	/*����inquire-msg*/
	else if(strcmp(msg,"inqu")==0)
	{	
	
		//printf("receive inquire-msg\n");
		/*�յ�ѯ����Ϣ�����������̣߳��߳�1���ڼ�ʱ���߳�2���ڼ���*/ 
		timeout=false;
		finish=false;
		//printf("create time thread\n");
		pthread_create(&pid1,NULL,Timeout,NULL);//������ʱ�߳�
		//printf("end create time thread\n");
		//printf("create caculate thread\n");
		pthread_create(&pid2,NULL,Caculate,buffer); //���������߳� 
		//printf("end create caculate thread\n");
		while(finish==false&&timeout==false)
		{
			;//�ȴ�	
		}
		if(finish)
		{
			//������� 
			//printf("finish..\n");
			send(fd, buf, strlen(buf) + 1, 0);
			pthread_cancel(pid1);
		}
		else if(timeout)
		{
			//���㳬ʱ
			//printf("timeout..\n");
			strcpy(buf,"fold\n");
			send(fd, buf, strlen(buf) + 1, 0);
			pthread_cancel(pid2);
		}
		pthread_join(pid1,NULL);
		pthread_join(pid2,NULL);	
	}
	/*����flop-msg*/
	else if(strcmp(msg,"flop")==0)
	{
		//printf("receive flop-msg\n");
		gameround=3;
		process_flop_msg(buffer);
		
	}
	/*����turn-msg*/
	else if(strcmp(msg,"turn")==0)
	{
		//printf("receive turn-msg\n");
		gameround=4;
		process_turn_msg(buffer); 
	}
	/*����river-msg*/
	else if(strcmp(msg,"rive")==0)
	{
		//printf("receive river-msg\n");
		gameround=5;
		process_river_msg(buffer);
	}
	/*����showdown-msg*/
	else if(strcmp(msg,"show")==0)
	{
		//printf("receive show-msg\n");
		gameround=0;
	}
	/*����pot-win-msg*/
	else if(strcmp(msg,"pot-")==0)
	{
		//printf("receive pot-win-msg\n");
		gameround=0;
	}
	return 0;
}

int main(int argc, char *argv[])
{

	/*��֤�����������*/ 
	if (argc< 6)
	{
		printf("Usage: <%s server_ip server_port my_ip my_port id>\n", argv[0]);
		return -1;
	}
	/* ��ȡ������� */
	in_addr_t server_ip = inet_addr(argv[1]);
	in_port_t server_port = htons(atoi(argv[2])); 
	in_addr_t my_ip = inet_addr(argv[3]);
	in_port_t my_port = htons(atoi(argv[4])); 
	int id = atoi(argv[5]);
	
	/* ����socket*/
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd< 0)
	{
		perror("create socket failed!\n");
		return -1;
	}
	
	/*����socketѡ���ַ�ظ�ʹ�ã���ֹ����������˳������´�����ʱbindʧ�� */
	int is_reuse_addr = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&is_reuse_addr, sizeof(is_reuse_addr));
	
	/*��ָ��ip��port����ֹ��server�ܾ� */
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
	
	/*����server*/
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = server_ip;
	server_addr.sin_port = server_port;
	while(connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		/*����ʧ��ʱ˯��100ms�����ԣ�������֤����server�����Ǻ��𣬶�����������*/
		usleep(100*1000); 
	}
	
	/*��server����ע����Ϣ*/
	char reg_msg[50] = {'\0'};
	snprintf(reg_msg, sizeof(reg_msg) - 1, "reg: %d %s \n", id, "gql"); 
	send(socket_fd, reg_msg, strlen(reg_msg) + 1, 0);
	
	/* ��ʼ���շ�������Ϣ�����е����˿���Ϸ*/
	while(1)
	{
		//�Ƚ�����buffer��� 
		char buffer[2048]={'\0'};
		//printf("i am waiting for msg\n");
		int len=recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
		//printf("i have received msg from server len=%d\n",len);
		int re; 
		if(len>0)
		{  
			re=process_server_message(socket_fd, buffer,argv[5]);
			if(re<0)break;//�յ�gameover��Ϣ���ó���Ϸ�������˳�ѭ�� 
		}
	}	
	/*�ر�socket����*/
	close(socket_fd);	
	return 0;
}
