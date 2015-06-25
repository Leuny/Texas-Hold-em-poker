#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> 
#include "decision.h"

//��ȡ����ֵ 
u8 GetCardValue(u8 CardData) 
{ 
	return CardData&MASK_VALUE; 
}
//��ȡ��ɫ 
u8 GetCardColor(u8 CardData) 
{ 
	return CardData&MASK_COLOR; 
}
//��ȡ���߼�ֵ 
u8 GetCardLogicValue(u8 CardData)
{
	//��ȡ����ֵ
	u8 bCardValue=GetCardValue(CardData);
	//��ȡ���߼�ֵ
	return (bCardValue==1)?(bCardValue+13):bCardValue;
}

int Pow(int x,int k)
{
	int value=1,i;
	for(i=0;i<k;i++)
	{
		value*=x;
	}
	return value;
}

bool Find(int list[],u8 n,u8 k)
{
	u8 i;
	for(i=0;i<n;i++)
	{
		if(list[i]==k)break;
	}
	if(i==n)return false;
}	
//��ȡһ���Ƶ��Ʊ��� 
int GetHandValue(int CardData[],u8 CardCount)
{	
	u8 i;
	bool SameColor=true,ShunCard=true;
	u8 FirstColor=GetCardColor(CardData[0]);
	u8 FirstValue=GetCardLogicValue(CardData[0]);
	for (i=1;i<CardCount;i++)
	{
		if (GetCardColor(CardData[i])!=FirstColor) SameColor=false;
		if (FirstValue!=(GetCardLogicValue(CardData[i])+i)) ShunCard=false;
		if ((SameColor==false)&&(ShunCard==false)) break;
	}
	
	//��С˳��A 2 3 4 5 
	if((ShunCard == false)&&(FirstValue == 14))
	{
		u8 j=1;
		for (j=1;j<CardCount;j++)
		{
			u8 LogicValue=GetCardLogicValue(CardData[i]);
			if (FirstValue!=(LogicValue+i+8)) break;
		}
		if( j == CardCount)
		{
			ShunCard==true;
		} 
			
	}	
	if ((SameColor==true)&&(ShunCard==true))
	{
		//ͬ��˳ 
		int Value=GetCardLogicValue(CardData[0]);
		if(Value==14&&GetCardLogicValue(CardData[1])==5)
		return ((STRAIGHT_FLUSH<<20)+(5<<16));
		else
		return ((STRAIGHT_FLUSH<<20)+(Value<<16));		
	}
	else if((SameColor==true)&&(ShunCard==false)) 
	{
		//ͬ��
		u8 arr[13],i;	
		for(i=0;i<13;i++)	
			arr[i]=0;
		for(i=0;i<CardCount;i++)
		{
			u8 logicvalue=GetCardLogicValue(CardData[i]);
			arr[logicvalue-2]++;
		}
		u8 k=0,a[5];
		for(i=0;i<13;i++)
		{
			if(arr[i]!=0)
			{
				a[k++]=i+2;
			}
		}
		return ((FLUSH<<20)+(a[4]<<16)+(a[3]<<12)+(a[2]<<8)+(a[1]<<4)+a[0]);
	}
	else if ((SameColor==false)&&(ShunCard==true)) 
	{
		//˳�� 
		int Value=GetCardLogicValue(CardData[0]);
		if(GetCardLogicValue(CardData[1])==5)
		return ((STRAIGHT<<20)+(5<<16));
		else
		return ((STRAIGHT<<20)+(Value<<16));	
	} 
	
	//���������� 
	u8 arr[13];	
	for(i=0;i<13;i++)	
		arr[i]=0;
	for(i=0;i<CardCount;i++)
	{
		u8 logicvalue=GetCardLogicValue(CardData[i]);
		arr[logicvalue-2]++;
	
	}
	
	int sum=0,k=0;
	int a[5];
	for(i=0;i<13;i++)
	{
		if(arr[i]!=0)
		{
			sum+=arr[i]*Pow(10,k);	
			a[k++]=i+2;
		}
	}

	switch(sum)
	{
		case 311:return ((THREE_OF_A_KIND<<20)+(a[2]<<16)+(a[1]<<12)+(a[0]<<8));break;
		case 131:return ((THREE_OF_A_KIND<<20)+(a[1]<<16)+(a[2]<<12)+(a[0]<<8));break;
		case 113:return ((THREE_OF_A_KIND<<20)+(a[0]<<16)+(a[2]<<12)+(a[1]<<8));break;	
		case 32:return ((FULL_HOUSE<<20)+(a[1]<<16)+(a[0]<<12));break;
		case 23:return ((FULL_HOUSE<<20)+(a[0]<<16)+(a[1]<<12));break;
		case 41:return ((FOUR_OF_A_KIND<<20)+(a[1]<<16)+(a[0]<<12));break;
		case 14:return ((FOUR_OF_A_KIND<<20)+(a[0]<<16)+(a[1]<<12));break;
		case 221:return ((TWO_PAIR<<20)+(a[2]<<16)+(a[1]<<12)+(a[0]<<8));break;
		case 212:return ((TWO_PAIR<<20)+(a[2]<<16)+(a[0]<<12)+(a[1]<<8));break;
		case 122:return ((TWO_PAIR<<20)+(a[1]<<16)+(a[0]<<12)+(a[2]<<8));break;		
		case 2111:return ((ONE_PAIR<<20)+(a[3]<<16)+(a[2]<<12)+(a[1]<<8)+(a[0]<<4));break;			
		case 1211:return ((ONE_PAIR<<20)+(a[2]<<16)+(a[3]<<12)+(a[1]<<8)+(a[0]<<4));break;
		case 1121:return ((ONE_PAIR<<20)+(a[1]<<16)+(a[3]<<12)+(a[2]<<8)+(a[0]<<4));break;	
		case 1112:return ((ONE_PAIR<<20)+(a[0]<<16)+(a[3]<<12)+(a[2]<<8)+(a[1]<<4));break;
		case 11111:return ((HIGH_CARD<<20)+(a[4]<<16)+(a[3]<<12)+(a[2]<<8)+(a[1]<<4)+a[0]);break;
		default:break;
	}
	return 0;	
}

//���ƽ������� 
void SortCardList(u8 CardData[], u8 CardCount)
{
	u8 LogicValue[FULL_COUNT];
	u8 i;
	for (i=0;i<CardCount;i++) 
		LogicValue[i]=GetCardLogicValue(CardData[i]);	
	bool bSorted=true;
	u8 TempData,bLast=CardCount-1;
	do
	{
		bSorted=true;
		u8 i;
		for (i=0;i<bLast;i++)
		{
			if ((LogicValue[i]<LogicValue[i+1])||
				((LogicValue[i]==LogicValue[i+1])&&(CardData[i]<CardData[i+1])))
			{
				TempData=CardData[i];
				CardData[i]=CardData[i+1];
				CardData[i+1]=TempData;
				TempData=LogicValue[i];
				LogicValue[i]=LogicValue[i+1];
				LogicValue[i+1]=TempData;
				bSorted=false;
			}	
		}
		bLast--;
	} while(bSorted==false);
}

//�������С��n��k���� 
void RandNum(int a[],u8 n,u8 k)
{
	u8 i,Pos,temp;
	for(i=0;i<n;i++) 
		a[i]=i;
	for(i=0;i<k;i++)
	{
		Pos=rand()%n;
		temp=a[i];
		a[i]=a[Pos];
		a[Pos]=temp;	
	}
}

/******************************************************
��ȡA-B�Ĳ�� 
���������C������ 
********************************************************/
void GetDifferenceSet(const int A[],int nA,int B[],int nB,int C[],int *nC)
{
	u8 i=0,k=0;
	for(i=0;i<nA;i++)
	{
		if(Find(B,nB,A[i]))continue;
		else{
			C[k++]=A[i];
		}
	}
	*nC=k;	
}

//��n�����л�ȡ5�������ڶ�ά����card�� 
void GetCardData(const int CardData[],int card[][5],u8 n)
{
	u8 k=0,j,i;
	if(n==6)
	{
		for(j=0;j<n;j++)
		{
			for(i=0;i<n;i++)
			{
				if(i!=j)
				card[j][k++]=CardData[i];
			}
			k=0;	
		}	
	}
	else 
	{
		int B[21][2];
		int k=0,n;
		for(i=0;i<6;i++)
		{
			for(j=i+1;j<7;j++)
			{
				B[k][0]=CardData[i];
				B[k][1]=CardData[j];
				k++;
			}
		}
		for(i=0;i<21;i++)
		GetDifferenceSet(CardData,7,B[i],2,card[i],&n);
	}
	
}

void GetOpponentCard(int OpponentCard[],int n,int BoardCard[],int CardSet[])
{	
	u8 i;
	for(i=0;i<n-2;i++)
	{
		OpponentCard[i]=BoardCard[i];
	}
	for(i=0;i<2;i++)
	OpponentCard[i+n-2]=CardSet[i];
}

void ShuffleCard(const int CardData[],int card[],int n)
{	
	int i=0;
	int RandPos[n];
	RandNum(RandPos,n,n);
	for(i=0;i<n;i++)
	{
		card[i]=CardData[RandPos[i]];
	}	
}

/********************************************************
MyPockCard���ҷ����� 
BoardCard�������� 
n:�����Ƹ��� 
player_num: ��Ҹ��� 
********************************************************/
float GetHandStrength(int MyPockCard[],int BoardCard[],int n,int player_num)
{
	int i=0;
	int MyHand[n+2];
	int OpponentHand[player_num][n+2];
	for(i=0;i<n+2;i++)
	{
		if(i<2)
		MyHand[i]=MyPockCard[i];
		else
		MyHand[i]=BoardCard[i-2];
	}
	//����Ȧ 
	if(n==3)
	{		
		int OpponentValue[player_num-1];
		int value=GetHandValue(MyHand, 5);
		int DifferenceSet[52],nDiff;
		int count=0;
		GetDifferenceSet(Card,52,MyHand,5,DifferenceSet,&nDiff);
		for(i=0;i<1000;i++)
		{
			int j=0;
			//ϴ�ƷŽ��ƶ��� 
			int CardStack[nDiff];
			ShuffleCard(DifferenceSet,CardStack,nDiff);
			for(j=0;j<player_num-1;j++)
			{
				GetOpponentCard(OpponentHand[j],5,BoardCard,CardStack+2*j);	
			}	
			for(j=0;j<player_num-1;j++)
			{
				OpponentValue[j]=GetHandValue(OpponentHand[j], 5);
			}
			int maxvalue=OpponentValue[0];
			for(j=1;j<player_num-1;j++)
			{
				if(maxvalue<OpponentValue[j])
				maxvalue=OpponentValue[j];
			}
			if(value>maxvalue)count++;
			
			pthread_testcancel();//���ڲ���ȡ���� 
		}
		return count/1000.0;	
	}
	//ת��Ȧ 
	else if(n==4)
	{
		int DifferenceSet[52],nDiff,k;
		int count=0,OpponentValue[player_num-1],value;
		int card[player_num-1][6][5],j;
		int mycard[6][5];
		//��ȡ�ҷ����� 
		GetCardData(MyHand,mycard,6);
		value=GetHandValue(mycard[0], 5);
		for(j=1;j<6;j++)
		{
			if(value<GetHandValue(mycard[j], 5))
			value=GetHandValue(mycard[j], 5);	
		}
		//��ȥMyHand 
		GetDifferenceSet(Card,52,MyHand,6,DifferenceSet,&nDiff);
		for(i=0;i<1000;i++)
		{
			//ϴ�ƷŽ��ƶ��� 
			int CardStack[nDiff];
			ShuffleCard(DifferenceSet,CardStack,nDiff);
			//Ϊÿһ����ҷ�2���Ʋ����Ϲ�����  
			for(j=0;j<player_num-1;j++)
			{
				GetOpponentCard(OpponentHand[j],6,BoardCard,CardStack+j*2);	
			}
			for(j=0;j<player_num-1;j++)
			{
				GetCardData(OpponentHand[j],card[j],6);	
			}
			for(k=0;k<player_num-1;k++)
			{
				OpponentValue[k]=GetHandValue(card[k][0], 5);
				for(j=1;j<6;j++)
				{
					if(OpponentValue[k]<GetHandValue(card[k][j], 5))
					OpponentValue[k]=GetHandValue(card[k][j], 5);
				}
			}
			int maxvalue=OpponentValue[0];
			for(k=1;k<player_num;k++)
			{
				if(maxvalue<OpponentValue[k])
				maxvalue=OpponentValue[k];
			}
			if(value>maxvalue)count++;
			
			pthread_testcancel();//���ڲ���ȡ���� 	
		}
		return count/1000.0; 			
	} 
	else
	{	
		int DifferenceSet[52],nDiff,k;
		int count=0,OpponentValue[player_num-1],value;
		int card[player_num-1][21][5],j;
		int mycard[21][5];
		//��ȡ�ҷ�����  
		GetCardData(MyHand,mycard,7);
		value=GetHandValue(mycard[0], 5);
		for(j=1;j<21;j++)
		{
			if(value<GetHandValue(mycard[j], 5))
			value=GetHandValue(mycard[j], 5);	
		}
		//��ȥMyHand  
		GetDifferenceSet(Card,52,MyHand,7,DifferenceSet,&nDiff);
		for(i=0;i<1000;i++)
		{
			//ϴ�ƷŽ��ƶ��� 
			int CardStack[nDiff];
			ShuffleCard(DifferenceSet,CardStack,nDiff);
			//Ϊÿһ����ҷ�2���Ʋ����Ϲ����� 
			for(j=0;j<player_num-1;j++)
			{
				GetOpponentCard(OpponentHand[j],7,BoardCard,CardStack+j*2);	
			}
			for(j=0;j<player_num-1;j++)
			{
				GetCardData(OpponentHand[j],card[j],7);	
			}
			for(k=0;k<player_num-1;k++)
			{
				OpponentValue[k]=GetHandValue(card[k][0], 5);
				for(j=1;j<21;j++)
				{
					if(OpponentValue[k]<GetHandValue(card[k][j], 5))
					OpponentValue[k]=GetHandValue(card[k][j], 5);
				}
			}
			int maxvalue=OpponentValue[0];
			for(k=1;k<player_num;k++)
			{
				if(maxvalue<OpponentValue[k])
				maxvalue=OpponentValue[k];
			}
			if(value>maxvalue)count++;
			pthread_testcancel();//���ڲ���ȡ���� 	
		}
		return count/1000.0; 		
	}	
}

/*********************************************************
 action���壺
 0-��������֮ǰ����Ҷ�����
 1-��ע=1 
 2- ��ע>=2
 3-��ע=1����ע=0 
 4 ��ע=1����ע>=1 
 position�ĺ��壺
 0-ǰ��λ 
 1-�м�λ 
 2-����λ 
 3-Сäעλ 
 4-��äעλ 
************************************************************/ 
void PreFlodDecision(int CardData[],u8 action,u8 position,CurBoard *curboard)
{
	memset(buf,0,20);
	if(curboard->player_num<=1)strcpy(buf,"call\n");//ʣ���Լ� 
	else 
	{
		
		if(GetCardLogicValue(CardData[0])<GetCardLogicValue(CardData[1]))
		{
			u8 temp=CardData[0];
			CardData[0]=CardData[1];
			CardData[1]=temp;
		}
		
		u8 firstvalue=GetCardLogicValue(CardData[0]);
		u8 secondvalue=GetCardLogicValue(CardData[1]); 
		u8 firstcolor=GetCardColor(CardData[0]);
		u8 secondcolor=GetCardColor(CardData[1]);
		
		//��ǿ�� AA KK QQ AK 
		if((firstvalue==secondvalue&&firstvalue>=12)||(firstvalue==14&&secondvalue==13))
		{
			if(curboard->curbet>4000)
			strcpy(buf,"fold\n");
			else
			strcpy(buf,"raise\n");		
		}
		//ǿ�� JJ TT 99 AQ AJs 
		else if((firstvalue==secondvalue&&firstvalue>=9&&firstvalue<=11)||
			(firstvalue==14&&secondvalue==12)||
			(firstcolor==secondcolor&&firstvalue==14&&secondvalue==11))
		{	
			if(curboard->curbet>2200)
			{
				strcpy(buf,"fold\n");	
			}
			else
			{
				
				if(action==0)
				{
					switch(position)
					{
						case 0:strcpy(buf,"raise\n");break;
						case 1:strcpy(buf,"raise\n");break;
						case 2:strcpy(buf,"raise\n");break;
						case 3:strcpy(buf,"raise\n");break;
						case 4:strcpy(buf,"raise\n");break;
						default:break;
					}	
				}
				else if(action==1)
				{
					switch(position)
					{
						case 0:strcpy(buf,"raise\n");break;
						case 1:strcpy(buf,"raise\n");break;
						case 2:strcpy(buf,"raise\n");break;
						case 3:strcpy(buf,"raise\n");break;
						case 4:strcpy(buf,"raise\n");break;
						default:break;
					}	
				}
				else if(action==2)
				{
					switch(position)
					{
						case 0:strcpy(buf,"raise\n");break;
						case 1:strcpy(buf,"raise\n");break;
						case 2:strcpy(buf,"raise\n");break;
						case 3:strcpy(buf,"raise\n");break;
						case 4:strcpy(buf,"raise\n");break;
						default:break;
					}	
				}
				else if(action==3)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"raise\n");break;
						case 2:strcpy(buf,"raise\n");break;
						case 3:strcpy(buf,"raise\n");break;
						case 4:strcpy(buf,"raise\n");break;
						default:break;
					}	
				}
				else if(action==4)
				{
					switch(position)
					{
						case 0:strcpy(buf,"call\n");break;
						case 1:strcpy(buf,"call\n");break;
						case 2:strcpy(buf,"call\n");break;
						case 3:strcpy(buf,"call\n");break;
						case 4:strcpy(buf,"call\n");break;
						default:break;
					}	
				}
				else
				{
					strcpy(buf,"fold\n");
				}
				
			}
		}
		//�е��� AJo ATs KQ
		else if((firstcolor!=secondcolor&&firstvalue==14&&secondvalue==11)||
				(firstcolor==secondcolor&&firstvalue==14&&secondvalue==10)||(firstvalue==13&&secondvalue==12))			
		{
			if(curboard->curbet>2000)
			{
				strcpy(buf,"fold\n");
			}
			else
			{
				if(action==0)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"raise\n");break;
						case 3:strcpy(buf,"raise\n");break;
						case 4:strcpy(buf,"raise\n");break;
						default:break;
					}	
				}
				else if(action==1)
				{
					switch(position)
					{
						case 0:strcpy(buf,"flod\n");break;
						case 1:strcpy(buf,"raise\n");break;
						case 2:strcpy(buf,"raise\n");break;
						case 3:strcpy(buf,"raise\n");break;
						case 4:strcpy(buf,"raise\n");break;
						default:break;
					}	
				}
				else if(action==2)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"raise\n");break;
						case 2:strcpy(buf,"raise\n");break;
						case 3:strcpy(buf,"raise\n");break;
						case 4:strcpy(buf,"raise\n");break;
						default:break;
					}	
				}
				else if(action==3)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"fold\n");break;
						case 3:strcpy(buf,"fold\n");break;
						case 4:strcpy(buf,"call\n");break;
						default:break;
					}	
				}
				else if(action==4)
				{
					if(firstcolor==secondcolor&&firstvalue==13&&secondvalue==12)
					strcpy(buf,"call\n");
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"fold\n");break;
						case 3:strcpy(buf,"fold\n");break;
						case 4:strcpy(buf,"call\n");break;
						default:break;
					}	
				}
				else
				{
					strcpy(buf,"fold\n");
				}	
			}
		}
		//ǿͶ����88-22 KJs KTs QJs QTs JTs T9s 
		else if((firstvalue==secondvalue&&firstvalue>=2&&firstvalue<=8)||
				(firstcolor==secondcolor&&firstvalue==13&&secondvalue==11)||
				(firstcolor==secondcolor&&firstvalue==13&&secondvalue==10)||
				(firstcolor==secondcolor&&firstvalue==12&&secondvalue==11)||
				(firstcolor==secondcolor&&firstvalue==12&&secondvalue==10)||
				(firstcolor==secondcolor&&firstvalue==11&&secondvalue==10)||
				(firstcolor==secondcolor&&firstvalue==10&&secondvalue==9))
		{
			if(curboard->curbet>1800)
			{
		
				strcpy(buf,"fold\n");
			}
			else
			{
				if(action==0)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"check\n");break;
						case 3:strcpy(buf,"raise\n");break;
						case 4:strcpy(buf,"raise\n");break;
						default:break;
					}	
				}
				else if(action==1)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"raise\n");break;
						case 3:strcpy(buf,"raise\n");break;
						case 4:strcpy(buf,"raise\n");break;
						default:break;
					}	
				}
				else if(action==2)
				{
					switch(position)
					{
						case 0:strcpy(buf,"call\n");break;
						case 1:strcpy(buf,"call\n");break;
						case 2:strcpy(buf,"call\n");break;
						case 3:strcpy(buf,"call\n");break;
						case 4:strcpy(buf,"fold\n");break;
						default:break;
					}	
				}
				else if(action==3)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"fold\n");break;
						case 3:strcpy(buf,"call\n");break;
						case 4:strcpy(buf,"call\n");break;
						default:break;
					}	
				}
				else if(action==4)
				{
					switch(position)
					{
						case 0:strcpy(buf,"call\n");break;
						case 1:strcpy(buf,"call\n");break;
						case 2:strcpy(buf,"call\n");break;
						case 3:strcpy(buf,"call\n");break;
						case 4:strcpy(buf,"call\n");break;
						default:break;
					}	
				}
				else
				{
					strcpy(buf,"fold\n");
				}
			}
		}
		//����� KJo KTo QJo QTo JTo A9s A8s A7s A6s A5s A4s A3s A2s K9s 98s 87s 76s 65s 54s 
		else if((firstcolor!=secondcolor&&firstvalue<=13&&firstvalue>=11 &&secondvalue<=11&&secondvalue>=10)||
				(firstcolor==secondcolor&&firstvalue==14&&secondvalue>=2&&secondvalue<=9)||
				(firstcolor==secondcolor&&firstvalue==13&&secondvalue==9)||
				(firstcolor==secondcolor&&firstvalue>=5&&firstvalue<=9&&firstvalue-1==secondvalue))
		{
			if(curboard->curbet>1500)
			{
				strcpy(buf,"fold\n");
			}
			else
			{
				if(action==0)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"raise\n");break;
						case 3:strcpy(buf,"raise\n");break;
						case 4:strcpy(buf,"raise\n");break;
						default:break;
					}	
				}
				else if(action==1)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"fold\n");break;
						case 3:strcpy(buf,"call\n");break;
						case 4:strcpy(buf,"fold\n");break;
						default:break;
					}	
				}
				else if(action==2)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"call\n");break;
						case 3:strcpy(buf,"call\n");break;
						case 4:strcpy(buf,"fold\n");break;
						default:break;
					}	
				}
				else if(action==3)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"fold\n");break;
						case 3:strcpy(buf,"fold\n");break;
						case 4:strcpy(buf,"fold\n");break;
						default:break;
					}	
				}
				else if(action==4)
				{
					switch(position)
					{
						case 0:strcpy(buf,"fold\n");break;
						case 1:strcpy(buf,"fold\n");break;
						case 2:strcpy(buf,"fold\n");break;
						case 3:strcpy(buf,"fold\n");break;
						case 4:strcpy(buf,"fold\n");break;
						default:break;
					}	
				}
			}
		}
		else
		{
			strcpy(buf,"fold\n");
		}
	}		
}


/*******************************************************
//����/ת��/����Ȧ���� 
curboard--�����ƾ����ݽṹ�� 
round--��ǰ�غ� 3=flop,4=turn,5=river 
********************************************************/
void Decision(CurBoard *curboard,int round)
{	
	if(curboard->player_num<=1)
	{
		strcpy(buf,"call\n");
	}
	else
	{	
		float HS=GetHandStrength(curboard->PockCard,curboard->BoardCard,round,curboard->player_num);
		if(HS>0.98)
		{
			if(curboard->curbet>4000)
			strcpy(buf,"fold\n");
			else
			strcpy(buf,"raise\n");
		}
		else if(HS>0.88)
		{
			if(curboard->curbet>2200)
			strcpy(buf,"fold\n");
			else
			strcpy(buf,"raise\n");
		}
		else if(HS>0.78)
		{
			if(curboard->curbet>2000)
			strcpy(buf,"fold\n");
			else
			strcpy(buf,"call\n");
		}
		else if(HS>0.68)
		{
			if(curboard->curbet>1800)
			strcpy(buf,"fold\n");
			else
			strcpy(buf,"call\n");
		}
		else if(HS>0.5)
		{
			if(curboard->curbet>1500)
			strcpy(buf,"fold\n");
			else
			strcpy(buf,"check\n");
		}
		else
		{
			strcpy(buf,"fold\n");
		}
	}
	
}



