#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h> 
#include "decision.h"

//获取牌面值 
u8 GetCardValue(u8 CardData) 
{ 
	return CardData&MASK_VALUE; 
}
//获取花色 
u8 GetCardColor(u8 CardData) 
{ 
	return CardData&MASK_COLOR; 
}
//获取牌逻辑值 
u8 GetCardLogicValue(u8 CardData)
{
	//获取牌面值
	u8 bCardValue=GetCardValue(CardData);
	//获取牌逻辑值
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
//获取一手牌的牌编码 
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
	
	//最小顺子A 2 3 4 5 
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
		//同花顺 
		int Value=GetCardLogicValue(CardData[0]);
		if(Value==14&&GetCardLogicValue(CardData[1])==5)
		return ((STRAIGHT_FLUSH<<20)+(5<<16));
		else
		return ((STRAIGHT_FLUSH<<20)+(Value<<16));		
	}
	else if((SameColor==true)&&(ShunCard==false)) 
	{
		//同花
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
		//顺子 
		int Value=GetCardLogicValue(CardData[0]);
		if(GetCardLogicValue(CardData[1])==5)
		return ((STRAIGHT<<20)+(5<<16));
		else
		return ((STRAIGHT<<20)+(Value<<16));	
	} 
	
	//按条数分类 
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

//将牌进行排序 
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

//随机产生小于n的k个数 
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
获取A-B的差集合 
结果保存在C集合中 
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

//从n个数中获取5个数放在二维数组card中 
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
MyPockCard：我方手牌 
BoardCard：公共牌 
n:公共牌个数 
player_num: 玩家个数 
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
	//翻牌圈 
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
			//洗牌放进牌堆中 
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
			
			pthread_testcancel();//用于测试取消点 
		}
		return count/1000.0;	
	}
	//转牌圈 
	else if(n==4)
	{
		int DifferenceSet[52],nDiff,k;
		int count=0,OpponentValue[player_num-1],value;
		int card[player_num-1][6][5],j;
		int mycard[6][5];
		//获取我方牌力 
		GetCardData(MyHand,mycard,6);
		value=GetHandValue(mycard[0], 5);
		for(j=1;j<6;j++)
		{
			if(value<GetHandValue(mycard[j], 5))
			value=GetHandValue(mycard[j], 5);	
		}
		//移去MyHand 
		GetDifferenceSet(Card,52,MyHand,6,DifferenceSet,&nDiff);
		for(i=0;i<1000;i++)
		{
			//洗牌放进牌堆中 
			int CardStack[nDiff];
			ShuffleCard(DifferenceSet,CardStack,nDiff);
			//为每一个玩家发2张牌并加上公共牌  
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
			
			pthread_testcancel();//用于测试取消点 	
		}
		return count/1000.0; 			
	} 
	else
	{	
		int DifferenceSet[52],nDiff,k;
		int count=0,OpponentValue[player_num-1],value;
		int card[player_num-1][21][5],j;
		int mycard[21][5];
		//获取我方牌力  
		GetCardData(MyHand,mycard,7);
		value=GetHandValue(mycard[0], 5);
		for(j=1;j<21;j++)
		{
			if(value<GetHandValue(mycard[j], 5))
			value=GetHandValue(mycard[j], 5);	
		}
		//移去MyHand  
		GetDifferenceSet(Card,52,MyHand,7,DifferenceSet,&nDiff);
		for(i=0;i<1000;i++)
		{
			//洗牌放进牌堆中 
			int CardStack[nDiff];
			ShuffleCard(DifferenceSet,CardStack,nDiff);
			//为每一个玩家发2张牌并加上公共牌 
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
			pthread_testcancel();//用于测试取消点 	
		}
		return count/1000.0; 		
	}	
}

/*********************************************************
 action含义：
 0-所有在你之前的玩家都弃牌
 1-跟注=1 
 2- 跟注>=2
 3-加注=1，跟注=0 
 4 加注=1，跟注>=1 
 position的含义：
 0-前面位 
 1-中间位 
 2-后面位 
 3-小盲注位 
 4-大盲注位 
************************************************************/ 
void PreFlodDecision(int CardData[],u8 action,u8 position,CurBoard *curboard)
{
	memset(buf,0,20);
	if(curboard->player_num<=1)strcpy(buf,"call\n");//剩下自己 
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
		
		//超强牌 AA KK QQ AK 
		if((firstvalue==secondvalue&&firstvalue>=12)||(firstvalue==14&&secondvalue==13))
		{
			if(curboard->curbet>4000)
			strcpy(buf,"fold\n");
			else
			strcpy(buf,"raise\n");		
		}
		//强牌 JJ TT 99 AQ AJs 
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
		//中等牌 AJo ATs KQ
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
		//强投机牌88-22 KJs KTs QJs QTs JTs T9s 
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
		//混合牌 KJo KTo QJo QTo JTo A9s A8s A7s A6s A5s A4s A3s A2s K9s 98s 87s 76s 65s 54s 
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
//翻牌/转牌/荷牌圈决策 
curboard--当期牌局数据结构体 
round--当前回合 3=flop,4=turn,5=river 
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



