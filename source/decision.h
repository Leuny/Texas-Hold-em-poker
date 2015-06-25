#ifndef DECISION_H
#define DECISION_H

#define	MASK_COLOR	0xF0//花色掩码
#define	MASK_VALUE	0x0F//数值掩码
#define FULL_COUNT   52

//牌型
typedef enum{
	HIGH_CARD=0,//高牌   
	ONE_PAIR=1,//一对  
	TWO_PAIR=2,//两对 
	//NEI_STRAIGHT=3,//内听顺子 
	THREE_OF_A_KIND=4,//三条 
 	//FOUR_STRAIGHT=5,//顺子听牌 
 	//FOUR_SAME_COLOR=6,//同花听牌
	STRAIGHT=7,//顺子   
	FLUSH=8,//同花   
	FULL_HOUSE=9,///葫芦  
	FOUR_OF_A_KIND=10,//四条  
	STRAIGHT_FLUSH=11,//同花顺  	
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
