#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "list.h"

/*建立单链表的函数*/
List* CreateList() 
{
	List *p,*h;
	if((h=(List *)malloc(sizeof(List)))==NULL) /*分配空间并检测*/
	{
		perror("malloc failed!!\n");
		exit(0);
	}
	h->name=(char*)malloc(sizeof(char)*20);
	if(h->name==NULL)
	{
		perror("malloc failed!!\n");
		exit(0);
	}
	
	h->name[0]='\0'; /*把表头结点的数据域置空*/
	h->next=NULL; /*把表头结点的链域置空*/
	return h;
}
/*向链表末尾添加字符串*/
void Append(List* L,char *ch)
{
	List *p,*s;
	p=L; /*p指向表头结点*/
	while(p->next)
	{
		p=p->next;
	}
	
	if((s= (List *) malloc(sizeof(List)))==NULL) /*分配新存储空间并检测*/
	{
		perror("malloc failed!!\n");
		exit(0);
	}
	s->name=(char*)malloc(sizeof(char)*20);
	if(s->name==NULL)
	{
		perror("malloc failed!!\n");
		exit(0);
	}
	
	/*把s的地址赋给p所指向的结点的链域，这样就把p和s所指向的结点连接起来了*/
	p->next=s; 
	s->name=ch;
	s->next=NULL;		
}

/*查找链表的函数，其中h指针是链表的表头指针，x指针是要查找的字符串*/
List* search(List *h,char *x) 
{

	List *p;
	char *y; /*保存结点数据域内字符串的指针*/
	char str[10];
	p=h->next; 
	while(p!=NULL)
	{
		y=p->name;
		strcpy(str,y);
		char *id=strchr(str,' ');
		if(!id)break;
		*id='\0';
		if(strcmp(str,x)==0) //把数据域里的姓名与所要查找的字符串比较，若相同则返回0，即条件成立
		return p; //返回与所要查找结点的地址
		else p=p->next;
		
	}
	return p;
}

/*
另一个查找函数，返回的是上一个查找函数的直接前驱结点的指针
*h为表头指针，x为指向要查找的字符串的指针
其实此函数的算法与上面的查找算法是一样的，只是多了一个指针s，并且s总是指向指针p所指向的结点的直接前驱
结果返回s即是要查找的结点的前一个结点
*/
List* Search(List *h,char *x)
{
	List *p,*s;
	char *y;
	char str[10];
	p=h->next;
	s=h;
	while(p!=NULL)
	{
		y=p->name;
		strcpy(str,y);
		char *id=strchr(str,' ');
		if(!id)break;
		*id='\0';
		
		if(strcmp(str,x)==0)
		return s;
		else
		{
			p=p->next;
			s=s->next;
		}
	}
	if(p==NULL)
	return p;
}

/*删除函数，其中y为要删除的结点的指针，x为要删除的结点的前一个结点的指针*/
void del(List *x,List *y) 
{
	List *s;
	s=y;
	x->next=y->next;
	free(s);
}
/*获取链表长度*/
int GetLength(List* h)
{
	int i=0;
	List* p;
	p=h;
	while(p->next)
	{
		i++;
		p=p->next;
	}
	return i;
}
/*删除链表中ch指向的字符串*/
void Delet(List* head,char* ch)
{
	List *cur=search(head,ch);
	List *pre=Search(head,ch);
	if(cur!=NULL&&pre!=NULL)
	del(pre,cur);
}
//添加玩家ID到链表 
void AppendId(List *head,char* buf)
{

	char* p1=strstr(buf,"/seat");
	if(p1)
	*(p1+5)='\0';
	
	char *PtrToPid;
	char *delim2;
	char* delim1=strchr(buf,':');	
	while(delim1!=NULL)
	{
		PtrToPid=delim1+2;
		delim2=strchr(PtrToPid,' ');
		if(!delim2)break;
		*(delim2+1)='0';
		*(delim2+2)='\0';		
		Append(head,PtrToPid);
		delim1=strchr(delim2+3,':');	
	}	
	while(1)
	{
		delim1=strchr(delim2+3,'\n');
		PtrToPid=delim1+1;
		delim2=strchr(PtrToPid,' ');
		if(delim2==NULL)break;
		
		*(delim2+1)='0';
		*(delim2+2)='\0';		
		Append(head,PtrToPid);
	}		
}


