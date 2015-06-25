#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "list.h"

/*����������ĺ���*/
List* CreateList() 
{
	List *p,*h;
	if((h=(List *)malloc(sizeof(List)))==NULL) /*����ռ䲢���*/
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
	
	h->name[0]='\0'; /*�ѱ�ͷ�����������ÿ�*/
	h->next=NULL; /*�ѱ�ͷ���������ÿ�*/
	return h;
}
/*������ĩβ����ַ���*/
void Append(List* L,char *ch)
{
	List *p,*s;
	p=L; /*pָ���ͷ���*/
	while(p->next)
	{
		p=p->next;
	}
	
	if((s= (List *) malloc(sizeof(List)))==NULL) /*�����´洢�ռ䲢���*/
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
	
	/*��s�ĵ�ַ����p��ָ��Ľ������������Ͱ�p��s��ָ��Ľ������������*/
	p->next=s; 
	s->name=ch;
	s->next=NULL;		
}

/*��������ĺ���������hָ��������ı�ͷָ�룬xָ����Ҫ���ҵ��ַ���*/
List* search(List *h,char *x) 
{

	List *p;
	char *y; /*���������������ַ�����ָ��*/
	char str[10];
	p=h->next; 
	while(p!=NULL)
	{
		y=p->name;
		strcpy(str,y);
		char *id=strchr(str,' ');
		if(!id)break;
		*id='\0';
		if(strcmp(str,x)==0) //�������������������Ҫ���ҵ��ַ����Ƚϣ�����ͬ�򷵻�0������������
		return p; //��������Ҫ���ҽ��ĵ�ַ
		else p=p->next;
		
	}
	return p;
}

/*
��һ�����Һ��������ص�����һ�����Һ�����ֱ��ǰ������ָ��
*hΪ��ͷָ�룬xΪָ��Ҫ���ҵ��ַ�����ָ��
��ʵ�˺������㷨������Ĳ����㷨��һ���ģ�ֻ�Ƕ���һ��ָ��s������s����ָ��ָ��p��ָ��Ľ���ֱ��ǰ��
�������s����Ҫ���ҵĽ���ǰһ�����
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

/*ɾ������������yΪҪɾ���Ľ���ָ�룬xΪҪɾ���Ľ���ǰһ������ָ��*/
void del(List *x,List *y) 
{
	List *s;
	s=y;
	x->next=y->next;
	free(s);
}
/*��ȡ������*/
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
/*ɾ��������chָ����ַ���*/
void Delet(List* head,char* ch)
{
	List *cur=search(head,ch);
	List *pre=Search(head,ch);
	if(cur!=NULL&&pre!=NULL)
	del(pre,cur);
}
//������ID������ 
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


