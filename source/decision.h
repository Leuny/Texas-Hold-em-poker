#ifndef DECISION_H
#define DECISION_H

#define	MASK_COLOR	0xF0//��ɫ����
#define	MASK_VALUE	0x0F//��ֵ����
#define FULL_COUNT   52

//����
typedef enum{
	HIGH_CARD=0,//����   
	ONE_PAIR=1,//һ��  
	TWO_PAIR=2,//���� 
	//NEI_STRAIGHT=3,//����˳�� 
	THREE_OF_A_KIND=4,//���� 
 	//FOUR_STRAIGHT=5,//˳������ 
 	//FOUR_SAME_COLOR=6,//ͬ������
	STRAIGHT=7,//˳��   
	FLUSH=8,//ͬ��   
	FULL_HOUSE=9,///��«  
	FOUR_OF_A_KIND=10,//����  
	STRAIGHT_FLUSH=11,//ͬ��˳  	
}Type;

typedef struct{
	int PockCard[2];
	int BoardCard[5];
	int player_num;
	int curbet;
}CurBoard;

typedef unsigned char u8;

extern const int Card[52];
extern char buf[20];

typedef enum{
	false,
	true
}bool;

u8 GetCardValue(u8 CardData);
u8 GetCardColor(u8 CardData);
u8 GetCardLogicValue(u8 CardData);
int Pow(int x,int k);
bool Find(int list[],u8 n,u8 k);
int GetHandValue(int CardData[],u8 CardCount);
void SortCardList(u8 CardData[], u8 CardCount);
void RandNum(int a[],u8 n,u8 k);
void GetDifferenceSet(const int A[],int nA,int B[],int nB,int C[],int *nC);
void GetCardData(const int CardData[],int card[][5],u8 n);
void GetOpponentCard(int OpponentCard[],int n,int BoardCard[],int CardSet[]);
void ShuffleCard(const int CardData[],int Card[],int n);
float GetHandStrength(int MyPockCard[],int BoardCard[],int n,int player_num);
float GetPotOdds(int curBet,int PotBet);
float RateOfReturn(float HandStrength,float PotOdds);
void PreFlodDecision(int CardData[],u8 action,u8 position,CurBoard *curboard);
void Decision(CurBoard *curboard,int round);

#endif
