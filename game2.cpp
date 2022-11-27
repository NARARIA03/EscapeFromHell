#define _CRT_SECURE_NO_WARNINGS

#include "DxLib.h"      //  Dx 라이브러리를 포함하여 C언어의 기술을 확장
#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <math.h>
#include <stdlib.h>   // random 함수용
#include <time.h>     //srand 함수 용 time 함수 사용


/*=======================================================================================================================================================================*/


//  게임 내 기본 설정용 const변수들
const char* Title = "Escape from Hell 1.0";    //  게임 이름
const int Window_WIDTH = 1268;               //  게임창 가로 길이
const int Window_HEIGHT = 892;              //  게임창 세로 길이
int nkey[256];								//  키가 눌러진 프레임 수를 저장한다. char 형으로 만들게 되면 공간이 부족해 오랬동안 키를 누르면 멈추는 버그가 있었다.
#define PI 3.141592                         //  PI값 (나중에 무기 사용할 때 무기 발사 방향 때문에 필요하다


/*=======================================================================================================================================================================*/


typedef struct                              //  몬스터 좌표와 이미지, 체력, 존재여부 등이 담긴 구조체
{
	double MobX;                            //  몬스터 x좌표
	double MobY;                            //  몬스터 y좌표
	int MobImg;                             //  몬스터 이미지 파일
	int MobHeart;                           //  몬스터 체력
	int MobExist;                           //  몬스터 존재 여부(1=T, 0=F) 
}Monster;

typedef struct                              //  캐릭터 좌표와 이미지, 방향 등이 담긴 구조체
{
	int CharX;                              //  캐릭터 x좌표
	int CharY;                              //  캐릭터 y좌표
	int CharImg[16];                        //  캐릭터 이미지 파일
	int CharDesign;                         //  캐릭터 바라보는 방향
	int Bullet[16];                         //  캐릭터 공격 모션 이미지 파일
	int Heart[3];                           //  캐릭터 체력 이미지 파일
}Character;

typedef struct                              //	보스 관련 정보를 저장해둔 구조체
{
	double BossX;							//	보스 x 좌표 
	double BossY;							//	보스 y 좌표
	int BossImg[4];							//	보스 이미지 파일
	int BossDesign;							//	보스 바라보는 방향
	int BossHeart;							//	보스 체력
	int BossExist;							//	보스 존재 여부 (0 = 소환x, 1 = 소환, 2 = 플레이어에 의해 사망)
}Boss;

typedef struct                              //	게임 관련 이미지 파일을 저장하는 구조체
{
	int map;								//	맵 이미지 저장용
	int start;								//	시작 화면 이미지 저장용
	int howtoplay;							//	플레이 방법 이미지 저장용
	int bossmap;							//	보스 전용 맵 이미지 저장용
	int gameover;							//	게임 오버 이미지 저장용
	int gamewin;							//	게임 클리어 이미지 저장용
}Image;

typedef struct								// 카서스 q 스킬
{
	int fire_x;								// 딱콩 x 좌표
	int fire_y;								// 딱콩 y 좌표
	int Boss_fire_img;						// 딱콩 이미지
}Boss_attack;

typedef struct								// 1초 동안 캐릭터가 움직인 값 측정 
{
	int up;									// 위 쪽
	int down;								// 아래 쪽
	int left;								// 왼 쪽
	int right;								// 오른 쪽
}Movement;

typedef struct								// 길다란 불 
{
	int x;									// 길다란 불 x좌표
	int y;									// 길다란 불 y좌표
	double angle;							// 길다란 불 각도
	int Boss_straight_fire_img;				// 길다란 불 이미지
}Straight_fire;

typedef struct
{
	int gunsound;							//	총소리 저장용
	int reloadsound;						//	장전소리 저장용
	int charhitsound;						//	캐릭터가 공격 받았을 때 소리 저장용
	int doorsound;							//	문에 들어갈 때 소리 저장용
}Sound;


/*=======================================================================================================================================================================*/


// hitbox에 맞고, 무적시간이 지나있을 때 -> damage 계산하는 함수
void hit(Character* character, int* health, time_t* immortal_t, Sound sound)		
{
	*immortal_t = time(NULL);										// 무적시간 초기화
	*health -= 1;                                                   // 체력 -1
	PlaySoundMem(sound.charhitsound, DX_PLAYTYPE_BACK);				//	공격 당한 효과음 출력

	if (character->CharDesign == 12)                                // 바라보는 방향으로 튕겨나게 함
	{
		if (character->CharY + 90 > 720)							//	만약 캐릭터가 밀려났을 때 맵 밖으로 나가진다면
			character->CharY = 720;									//	맵의 끝까지만 밀려남
		else
			character->CharY += 90;									//	맵 밖으로 나가지 않는다면 90만큼 밀려남
	}
	else if (character->CharDesign == 4)
	{
		if (character->CharX - 90 < 240)							//	만약 캐릭터가 밀려났을 때 맵 밖으로 나가진다면
			character->CharX = 240;									//	맵의 끝까지만 밀려남
		else
			character->CharX -= 90;									//	맵 밖으로 나가지 않는다면 90만큼 밀려남
	}
	else if (character->CharDesign == 0)
	{
		if (character->CharY - 90 < 300)							//	만약 캐릭터가 밀려났을 때 맵 밖으로 나가진다면
			character->CharY = 300;									//	맵의 끝까지만 밀려남
		else
			character->CharY -= 90;									//	맵 밖으로 나가지 않는다면 90만큼 밀려남
	}
	else if (character->CharDesign == 8)
	{
		if (character->CharX + 90 > 1028)							//	만약 캐릭터가 밀려났을 때 맵 밖으로 나가진다면
			character->CharX = 1028;								//	맵의 끝까지만 밀려남
		else
			character->CharX += 90;									//	맵 밖으로 나가지 않는다면 90만큼 밀려남
	}
}

//  키 입력 상태 업데이트 하는 함수
int UpdateKey(void)
{
	char tmpkey[256];    //  현재 키의 입력 상태를 저장
	GetHitKeyStateAll(tmpkey);  //  모든 키의 입력 상태를 받는 함수 GetHitKeyStateAll 을 이용해서 모든 키의 입력 상태를 tmpKey에 저장한다

	for (int i = 0; i < 256; i++)   //  모든 키에 대해 검사한다
	{
		if (tmpkey[i] != 0) //  만약 사용자가 tmpkey[i]값에 해당하는 키를 누르고 있다면
			nkey[i]++;  //  nkey[i]값을 1 더한다
		else
			nkey[i] = 0;    //  만약 사용자가 tmpkey[i] 값에 해당하는 키를 누르고 있지 않다면 nkey[i]값은 0이 된다.
	}
	return 0;
}

//  DxLib 기본 설정 모아둔 함수
void DxLib_Set()
{
	ChangeWindowMode(TRUE); //  창모드로 실행하는 명령
	SetWindowSizeChangeEnableFlag(false, false);
	SetMainWindowText(Title);   //  실행 창의 이름 변경
	SetGraphMode(Window_WIDTH, Window_HEIGHT, 32);  //  창의 크기를 X = WIDTH, Y = HEIGHT로 변경, 32는 색 깊이를 의미한다
	DxLib_Init();   //  DxLib 초기화
	SetDrawScreen(DX_SCREEN_BACK);  //  2중 버퍼링을 위한 세팅
}

//  키보드 입력에 따라 캐릭터좌표 계산하는 함수
void Character_Move(Character* character, time_t* character_history, int* character_history_x, int* character_history_y, Movement* movement)
{
	if (nkey[KEY_INPUT_RIGHT] >= 1)                                 //  오른쪽 방향키를 클릭했다면
	{
		if (character->CharX < Window_WIDTH - 240)                  //  오른쪽 끝에 닿지 않았다면
		{
			character->CharX = character->CharX + 4;                //  x좌표 +4
			character->CharDesign = 4;                              //  캐릭터가 오른쪽을 보게 만듬
			if (*character_history + 2 > time(NULL))
			{
				movement->right += 1;								// 오른쪽 눌렀음을 저장
			}
			else
			{
				*character_history = time(NULL);					// time 초기화
				*character_history_x = character->CharX - movement->right + movement->left;		// character 1초 전의 위치를 저장함
				*character_history_y = character->CharY - movement->down + movement->up;
				movement->up = 0;									// 1초 지났으니 방향키 누른 횟수 모두 초기화
				movement->down = 0;
				movement->left = 0;
				movement->right = 0;
			}
		}
	}
	if (nkey[KEY_INPUT_LEFT] >= 1)                                  //  왼쪽 방향키를 클릭했다면
	{
		if (character->CharX > 240)                                 //  왼쪽 끝에 닿지 않았다면
		{
			character->CharX = character->CharX - 4;                //  x좌표 -4
			character->CharDesign = 8;                              //  캐릭터가 왼쪽을 보게 만듬
			if (*character_history + 2 > time(NULL))
			{
				movement->left += 1;
			}
			else
			{
				*character_history = time(NULL);						// time 초기화
				*character_history_x = character->CharX - movement->right + movement->left;		// character 1초 전의 위치를저장함
				*character_history_y = character->CharY - movement->down + movement->up;
				movement->up = 0;									// 1초 지났으니 방향키 누른 횟수 모두 초기화
				movement->down = 0;
				movement->left = 0;
				movement->right = 0;
			}
		}
	}
	if (nkey[KEY_INPUT_UP] >= 1)                                    //  위쪽 방향키를 클릭했다면
	{
		if (character->CharY > 300)                                 //  위쪽 끝에 닿지 않았다면
		{
			character->CharY = character->CharY - 4;                //  y좌표 -4 (아래로 갈수록 + 이므로)
			character->CharDesign = 12;                             //  캐릭터가 위를 보게 만듬
			if (*character_history + 2 > time(NULL))
			{
				movement->up += 1;
			}
			else
			{
				*character_history = time(NULL);						// time 초기화
				*character_history_x = character->CharX - movement->right + movement->left;		// character 1초 전의 위치를저장함
				*character_history_y = character->CharY - movement->down + movement->up;
				movement->up = 0;									// 1초 지났으니 방향키 누른 횟수 모두 초기화
				movement->down = 0;
				movement->left = 0;
				movement->right = 0;
			}
		}
	}
	if (nkey[KEY_INPUT_DOWN] >= 1)                                  //  아래쪽 방향키를 클릭했다면
	{
		if (character->CharY < Window_HEIGHT - 180)                 //  아래쪽 끝에 닿지 않았다면
		{
			character->CharY = character->CharY + 4;                //  y좌표 +4
			character->CharDesign = 0;                              //  캐릭터가 아래를 보게 만듬
			if (*character_history + 2 > time(NULL))
			{
				movement->down += 1;
			}
			else
			{
				*character_history = time(NULL);					// time 초기화
				*character_history_x = character->CharX - movement->right + movement->left;		// character 1초 전의 위치를저장함
				*character_history_y = character->CharY - movement->down + movement->up;
				movement->up = 0;								// 1초 지났으니 방향키 누른 횟수 모두 초기화
				movement->down = 0;
				movement->left = 0;
				movement->right = 0;
			}
		}
	}
	if (*character_history + 2 <= time(NULL))			// 방향키를 누르지 않아도 1초마다 확인해줌
	{
		*character_history = time(NULL);							// time 초기화
		*character_history_x = character->CharX - movement->right + movement->left;			// character 1초 전의 위치를저장함
		*character_history_y = character->CharY - movement->down + movement->up;
		movement->up = 0;										// 1초 지났으니 방향키 누른 횟수 모두 초기화
		movement->down = 0;
		movement->left = 0;
		movement->right = 0;
	}
	return;		//	함수 종료
}

//  계산된 캐릭터 좌표에 캐릭터 출력하는 함수
void Print_Character(Character* character)
{
	DrawRotaGraph(character->CharX, character->CharY, 1.6, 0.0, character->CharImg[character->CharDesign], TRUE);
}

//  맵을 바꿀 때 사용하기 편하라고 만든 맵 출력 함수
void Print_Map(int map_data)
{
	DrawRotaGraph(630, 450, 1.6, 0.0, map_data, TRUE);
}

//  A키를 누르면 캐릭터 위치에서 총 쏘는 함수
void Shot_Bullet(Character* character, int* remain_bullet, time_t* reload_t, time_t* fire_t, int* bullet_time, Sound sound)
{

	if (*remain_bullet > 0)                                            // 총알이 남아있다면
	{
		if (*fire_t + 1 <= time(NULL))                                 // 전 총알을 발사한 시간에서 1초 이상 지났을 때
		{
			if (nkey[KEY_INPUT_A] >= 1 && nkey[KEY_INPUT_A] < 50)      //  만약 A키를 눌렀다면 & 꾹 누르고 있지 않는다면
			{
				if (character->CharDesign == 12)    //캐릭터가 위쪽을 보고 있음
					DrawRotaGraph(character->CharX, character->CharY - 60, 4, (PI / 2), character->Bullet[9], TRUE);

				if (character->CharDesign == 4)     //캐릭터가 오른쪽을 보고 있음
					DrawRotaGraph(character->CharX + 50, character->CharY - 5, 4, PI, character->Bullet[9], TRUE);

				if (character->CharDesign == 0)     //캐릭터가 아래쪽을 보고 있음
					DrawRotaGraph(character->CharX, character->CharY + 50, 4, -(PI / 2), character->Bullet[9], TRUE);

				if (character->CharDesign == 8)     //캐릭터가 왼쪽을 보고 있음
					DrawRotaGraph(character->CharX - 60, character->CharY - 5, 4, 2 * PI, character->Bullet[9], TRUE);
				PlaySoundMem(sound.gunsound, DX_PLAYTYPE_BACK);			//	총알 발사 효과음 출력
				*remain_bullet -= 1;                                    // 총알 1개 소모

				*fire_t = time(NULL);                                   // 총알 발사한 시간 저장
			}
		}
	}
	else
	{
		DrawFormatString(650, 110, GetColor(255, 255, 255), "reloading...");
		if (*bullet_time == 0)                             // 총알이 0
		{
			PlaySoundMem(sound.reloadsound, DX_PLAYTYPE_BACK);	//	재장전 효과음 출력
			*reload_t = time(NULL);                        // 총알이 없는 시점의 시간 저장
			*bullet_time = 1;                              // 장전 중인 상태
		}
		if (*reload_t + 2 <= time(NULL))                   // 총알이 없는 시점의 시간에서 2초 지났을 때
		{
			*bullet_time = 0;                              // 장전 완료된 상태
			*remain_bullet += 10;                          // 10발 장전
		}
	}
}

//MOB의 좌표 랜덤 설정, 체력 초기화, 존재 값 1(존재함)로 변화시킴
void MOB_XY(Monster* Mob)
{
	if (Mob->MobExist == 0)                       // 몬스터가 존재하지 않으면
	{
		Mob->MobX = (rand() % 500) + 250;         //  몬스터 x좌표 랜덤
		Mob->MobY = (rand() % 250) + 350;         //  몬스터 y좌표 랜덤
		Mob->MobHeart = 100;                      //  몬스터 체력 100
		Mob->MobExist = 1;					      //  몬스터 존재여부 1로 바꿈
	}
}

//  몬스터를 출력하는 함수
void Print_Mob(Monster* Mob)
{
	if (Mob->MobHeart > 0 && Mob->MobExist == 1)			//	몬스터가 살아 있다면
	{
		DrawRotaGraph((int)Mob->MobX, (int)Mob->MobY, 1.6, 0.0, Mob->MobImg, TRUE);	//	몬스터 출력
		DrawFormatString((int)Mob->MobX - 28, (int)Mob->MobY + 20, GetColor(255, 255, 255), "HP:%d", Mob->MobHeart);	//	아래에 체력 정보 출력
	}
}

//  체력 아이콘 출력하는 함수
void Print_Heart(Character* character, int health)
{
	for (int i = 0; i < health; i++)		//	체력 만큼만 반복함
	{
		DrawRotaGraph(900 + (35 * i), 130, 1, 0.0, character->Heart[0], TRUE);		//	체력 만큼 하트그림 출력
	}
}

// 몬스터 움직이기(대각선으로 움직임)
void Move_Mob(Character* character, Monster* Mob, int* health, time_t* immortal_t, Sound sound)
{
	if (Mob->MobHeart > 0)                                                  //    몬스터가 살아있다면
	{
		int diff_x = (int)character->CharX - (int)Mob->MobX;                // 캐릭터 x 좌표 - 몬스터 x 좌표
		int diff_y = (int)character->CharY - (int)Mob->MobY;                // 캐릭터 y 좌표 - 몬스터 y 좌표

		if (diff_x > 0)                                                     // 캐릭터가 몬스터 기준 오른쪽에 있으면
			Mob->MobX += 0.3;                                               // 몬스터도 오른쪽으로 이동
		else                                                                // 캐릭터가 몬스터 기준 왼쪽에 있으면
			Mob->MobX -= 0.3;                                               // 몬스터도 왼쪽으로 이동

		if (diff_y > 0)                                                     // 캐릭터가 몬스터 기준 아래쪽에 있을 때
			Mob->MobY += 0.3;                                               // 몬스터도 아래쪽으로 이동
		else                                                                // 캐릭터가 몬스터 기준 윗쪽에 있을 때
			Mob->MobY -= 0.3;                                               // 몬스터도 위쪽으로 이동

		if (abs(diff_x) <= 30 && abs(diff_y) <= 30)							// hitbox에 맞았으면
			if (*immortal_t + 1 < time(NULL))
				hit(character, health, immortal_t, sound);					// hit 함수 실행

	}
	else
	{
		Mob->MobX = 0;    //    몬스터가 죽었다면 X Y 좌표를 좌상단에 짱박아둠
		Mob->MobY = 0;
		Mob->MobExist = 2;	//	몬스터를 죽였다면 exist=2 -> flag 역할
	}
}

// 몬스터에게 & 보스에게 공격했을 때 체력 계산 함수 (몬스터는 한 개체에 한번 반복하므로 상관 없으나, 보스는 메인문에서 5번 반복하므로 데미지를 5로 나누어서 계산한다 
void Attack_Monster(Character* character, Monster* Mob, Boss* boss, time_t* fire_t, int bullet_time)
{
	if (nkey[KEY_INPUT_A] == 1 && *fire_t + 1 <= time(NULL) && bullet_time == 0)	//	총알 연타가 안 되고, 장전 중일 때 데미지가 들어가지 않도록 하는 조건문 (bullet_time = 1이면 장전 중이므로 조건문에 들어가지 못 함)
	{
		int diffx = (int)character->CharX - (int)Mob->MobX;							//	캐릭터와 몬스터의 x거리
		int diffy = (int)character->CharY - (int)Mob->MobY;							//	캐릭터와 몬스터의 y거리
		int Bdiffx = (int)character->CharX - (int)boss->BossX;						//	캐릭터와 보스의 x거리
		int Bdiffy = (int)character->CharY - (int)boss->BossY;						//	캐릭터와 보스의 y거리

		//	if if 구조로 해야만, 몬스터와 보스가 겹쳤을 때 데미지가 제대로 들어간다. if else로 하면 한 대상만 데미지가 들어간다
		
		if (character->CharDesign == 12)										//	캐릭터가 위쪽을 보고 있을때
		{
			if (diffy <= 90 && diffy >= 1 && diffx <= 20 && diffx >= -20)		//	위쪽 총알 발사 히트박스(몬스터용)
			{
				if (rand() % 100 < 33)											//	33%의 확률안에 해당한다면
					Mob->MobHeart -= 75;										//	75의 체력 감소(크리티컬데미지)
				else
					Mob->MobHeart -= 25;
			}
			if (Bdiffy <= 90 && Bdiffy >= 1 && Bdiffx <= 20 && Bdiffx >= -20)	//	위쪽 총알 발사 히트박스(보스용)
			{
				if (rand() % 100 < 33)											//	33%의 확률안에 해당한다면
					boss->BossHeart -= 15;										//	75의 체력 감소(크리티컬데미지)
				else
					boss->BossHeart -= 5;
			}
		}
		else if (character->CharDesign == 0)									//	캐릭터가 아래쪽을 보고 있을때
		{
			if (diffy >= -90 && diffy <= -1 && diffx <= 20 && diffx >= -20)		//	아래쪽 총알 발사 히트박스(몬스터용)
			{
				if (rand() % 100 < 33)											//	33%의 확률안에 해당한다면
					Mob->MobHeart -= 75;										//	75의 체력 감소(크리티컬데미지)
				else
					Mob->MobHeart -= 25;
			}
			if (Bdiffy >= -90 && Bdiffy <= -1 && Bdiffx <= 20 && Bdiffx >= -20)	//	아래쪽 총알 발사 히트박스(보스용)
			{
				if (rand() % 100 < 33)											//	33%의 확률안에 해당한다면
					boss->BossHeart -= 15;										//	75의 체력 감소(크리티컬데미지)
				else
					boss->BossHeart -= 5;
			}
		}
		else if (character->CharDesign == 4)									//	캐릭터가 오른쪽을 보고 있을때
		{
			if (diffx <= -1 && diffx >= -90 && diffy <= 20 && diffy >= -20)		//	오른쪽 총알 발사 히트박스(몬스터용)
			{
				if (rand() % 100 < 33)											//	33%의 확률안에 해당한다면									
					Mob->MobHeart -= 75;										//	75의 체력 감소(크리티컬데미지)
				else
					Mob->MobHeart -= 25;
			}
			if (Bdiffx <= -1 && Bdiffx >= -90 && Bdiffy <= 20 && Bdiffy >= -20)	//	오른쪽 총알 발사 히트박스(보스용)
			{
				if (rand() % 100 < 33)											//	33%의 확률안에 해당한다면									
					boss->BossHeart -= 15;										//	75의 체력 감소(크리티컬데미지)
				else
					boss->BossHeart -= 5;
			}
		}
		else if (character->CharDesign == 8)									//	캐릭터가 왼쪽을 보고 있을때(몬스터용)
		{
			if (diffx >= 1 && diffx <= 90 && diffy <= 20 && diffy >= -20)		//	왼쪽 총알 발사 히트박스
			{
				if (rand() % 100 < 33)											//	33%의 확률안에 해당한다면
					Mob->MobHeart -= 75;										//	75의 체력 감소(크리티컬데미지)
				else
					Mob->MobHeart -= 25;						
			}
			if (Bdiffx >= 1 && Bdiffx <= 90 && Bdiffy <= 20 && Bdiffy >= -20)	//	왼쪽 총알 발사 히트박스(보스용)
			{
				if (rand() % 100 < 33)											//	33%의 확률안에 해당한다면
					boss->BossHeart -= 15;										//	75의 체력 감소(크리티컬데미지)
				else
					boss->BossHeart -= 5;										//	크리티컬이 아니라면 -25
			}
		}
	}
}

//	몬스터를 다 잡으면 다음 맵으로 이동할 수 있게 하는 함수
void Move_Map(Character* character, Monster mob[], int* map_x, int* map_y, Sound sound)	//	캐릭터 구조체, 몬스터 구조체 배열, 다음 맵으로 이동하기 위한 문 방향
{

	int i;
	int sum;
	//	MobExist 값은 소환 전=0, 소환=1, 사망=2 이다. 따라서 0이면 소환한 후 1로 바뀌고, 죽게 되면 2로 바뀐다
	sum = mob[0].MobExist + mob[1].MobExist + mob[2].MobExist + mob[3].MobExist + mob[4].MobExist;

	if (sum == 10)                                //    만약 몬스터가 전부 죽었다면
	{
		DrawFormatString(628, 350, GetColor(255, 255, 255), "↑");		//	문으로 들어갈 수 있다는 표시
		DrawFormatString(1000, 500, GetColor(255, 255, 255), "→");		//	문으로 들어갈 수 있다는 표시
		DrawFormatString(628, 700, GetColor(255, 255, 255), "↓");		//	문으로 들어갈 수 있다는 표시
		DrawFormatString(300, 500, GetColor(255, 255, 255), "←");		//	문으로 들어갈 수 있다는 표시

		if ((character->CharX) <= 660 && (character->CharX) >= 580 && (character->CharY <= 300))	// 위쪽 문
		{
			character->CharY = 712;										//	아랫문에서 나옴
			for (i = 0; i < 5; i++)
				mob[i].MobExist = 0;									//	몬스터를 소환 전으로 다시 초기화 (몬스터재생성을 위해)
			*map_y -= 1;												//	맵y값이 1 감소
		}
		else if ((character->CharX >= 1025) && (character->CharY) >= 454 && (character->CharY) <= 522)	// 오른쪽 문
		{
			character->CharX = 238;										//	왼쪽 문에서 나옴
			for (i = 0; i < 5; i++)
				mob[i].MobExist = 0;									//	몬스터를 소환 전으로 다시 초기화 (몬스터재생성을 위해)	
			*map_x += 1;												//	맵x값이 1 증가
		}
		else if ((character->CharX) >= 594 && (character->CharX) <= 666 && character->CharY >= 708)	// 아래쪽 문
		{
			character->CharY = 300;										//	위쪽 문에서 나옴
			for (i = 0; i < 5; i++)
				mob[i].MobExist = 0;									//	몬스터를 소환 전으로 다시 초기화 (몬스터재생성을 위해)
			*map_y += 1;												//	맵y값이 1 증가
		}
		else if ((character->CharY) >= 464 && (character->CharY) <= 544 && character->CharX <= 240)	// 왼쪽 문
		{
			character->CharX = 1030;									//	오른쪽 문에서 나옴
			for (i = 0; i < 5; i++)
				mob[i].MobExist = 0;									//	몬스터를 소환 전으로 다시 초기화 (몬스터재생성을 위해)
			*map_x -= 1;												//	맵x값이 1 감소
		}
		PlaySoundMem(sound.doorsound, DX_PLAYTYPE_BACK);				//	문 열리는 효과음 출력
	}
}

//	보스 움직이게 하는 함수
void boss_move(Boss* boss, Character* character, time_t* immortal_t, int* health)
{
	double diff_x = character->CharX - boss->BossX;			//	diffx는 캐릭터와 보스의 x좌표 차
	double diff_y = character->CharY - boss->BossY;			//	diffy는 캐릭터와 보스의 y좌표 차

	if (boss->BossExist == 0)							//	보스가 살아 있으면
	{
		if (diff_x >= 0 && diff_y >= 0)				//	캐릭터가 보스보다 오른쪽 아래에 있을 때
		{
			boss->BossX += 0.15;
			boss->BossY += 0.15;
			diff_x = fabs(diff_x);					//	거리계산을 위한 절댓값
			diff_y = fabs(diff_y);
			if (diff_x > diff_y)					//	x좌표 거리가 더 크면
				boss->BossDesign = 2;				//	보스가 오른쪽을 보게 함
			else
				boss->BossDesign = 0;				//	y좌표 거리가 더 크면 보스가 아래쪽을 보게 함
		}
		else if (diff_x >= 0 && diff_y < 0)			//	캐릭터가 보스보다 오른쪽 위에 있을 때
		{
			boss->BossX += 0.15;
			boss->BossY -= 0.15;
			diff_x = fabs(diff_x);					//	거리계산을 위한 절댓값
			diff_y = fabs(diff_y);
			if (diff_x > diff_y)					//	x좌표 거리가 더 크면
				boss->BossDesign = 2;				//	보스가 오른쪽을 보게 함
			else
				boss->BossDesign = 3;				//	y좌표 거리가 더 크면 보스가 위쪽을 보게 함
		}
		else if (diff_x < 0 && diff_y >= 0)		//	캐릭터가 보스보다 왼쪽 아래에 있을 때
		{
			boss->BossX -= 0.15;
			boss->BossY += 0.15;
			diff_x = fabs(diff_x);				//	거리계산을 위한 절댓값
			diff_y = fabs(diff_y);
			if (diff_x > diff_y)				//	x좌표 거리가 더 크면
				boss->BossDesign = 1;			//	보스가 왼쪽을 보게 함
			else
				boss->BossDesign = 0;			//	y좌표 거리가 더 크면 보스가 아래쪽을 보게 함
		}
		else if (diff_x < 0 && diff_y < 0)		//	캐릭터가 보스보다 왼쪽 위에 있을 때
		{
			boss->BossX -= 0.15;
			boss->BossY -= 0.15;
			diff_x = fabs(diff_x);				//	거리계산을 위한 절댓값
			diff_y = fabs(diff_y);
			if (diff_x > diff_y)				//	x좌표 거리가 더 크면
				boss->BossDesign = 1;			//	보스가 왼쪽을 보게 함
			else
				boss->BossDesign = 3;			//	y좌표 거리가 더 크면 보스가 위쪽을 보게 함
		}
	}
	else if (boss->BossExist == 2)					//	보스가 플레이어에 의해 죽으면
	{
		boss->BossX = 0;							//	보스 좌표를 좌상단으로 고정
		boss->BossY = 0;
	}
	if (diff_x < 40 && diff_y < 40 && *immortal_t + 1 < time(NULL))       // 만약 보스와 캐릭터가 부딫혔을 때
	{
		*immortal_t = time(NULL);
		*health -= 1;                                                   // 체력 -1

		if (character->CharDesign == 12)                                // 튕겨나게 함
		{
			if (character->CharY + 90 > 720)							//	튕겨나지는 곳이 맵 밖이라면 튕겨나가지 않게 함
				character->CharY = 720;
			else
				character->CharY += 90;									//	튕겨나가지는 곳이 맵 밖이 아니라면 튕겨나가게 함
		}
		else if (character->CharDesign == 4)
		{
			if (character->CharX - 90 < 240)							//	튕겨나지는 곳이 맵 밖이라면 튕겨나가지 않게 함
				character->CharX = 240;
			else
				character->CharX -= 90;									//	튕겨나가지는 곳이 맵 밖이 아니라면 튕겨나가게 함
		}
		else if (character->CharDesign == 0)
		{
			if (character->CharY - 90 < 300)							//	튕겨나지는 곳이 맵 밖이라면 튕겨나가지 않게 함
				character->CharY = 300;
			else
				character->CharY -= 90;									//	튕겨나가지는 곳이 맵 밖이 아니라면 튕겨나가게 함
		}
		else if (character->CharDesign == 8)
		{
			if (character->CharX + 90 > 1028)							//	튕겨나지는 곳이 맵 밖이라면 튕겨나가지 않게 함
				character->CharX = 1028;
			else
				character->CharX += 90;									//	튕겨나가지는 곳이 맵 밖이 아니라면 튕겨나가게 함
		}
	}
}

//	게임 시작 전 메뉴 함수
void Start(Image image)
{
	int howtoplay_Flag = 0;													//	조작법 창을 선택하면 이 변수값을 1로 바꾼다.

	//ScreenFlip ProcessMessage ClearDrawScreen UpdateKey 함수는 모두 정상 작동할 때 0을 반환한다
	while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0 && UpdateKey() == 0)
	{
		if (howtoplay_Flag == 0)											//	조작법 창을 선택하지 않았다면
		{
			DrawRotaGraph(630, 450, 1.6, 0.0, image.start, TRUE);			//	메인 화면 출력

			if (nkey[KEY_INPUT_1] >= 1)										//	1번 키 (게임시작) 를 눌렀다면
				break;														//	Start함수 종료 -> 게임 구동부로 들어가게 됨

			else if (nkey[KEY_INPUT_2] >= 1)								//	2번 키 ( 조작법 ) 를 눌렀다면
				howtoplay_Flag = 1;											//	조작법 선택여부 변수를 1로 바꿈, 따라서 else문으로 들어가게 됨

			else if (nkey[KEY_INPUT_3] >= 1)								//	3번 키 ( 게임 종료 ) 를 눌렀다면
				DxLib_End();												//	게임 종료

		}
		else if (howtoplay_Flag == 1)										//	조작법을 선택했다면
		{
			DrawRotaGraph(630, 450, 1.6, 0.0, image.howtoplay, TRUE);		//	조작법 화면 출력
			if (nkey[KEY_INPUT_ESCAPE] >= 1)										//	esc 키 ( 뒤로 가기 ) 를 눌렀다면
				howtoplay_Flag = 0;											//	조작법 선택여부 변수를 0으로 바꿔 메인 화면으로 돌아감
		}
	}
}

//	보스 출력하는 함수
void Boss_Print(Boss* boss)
{
	if (boss->BossHeart > 0)
	{
		DrawRotaGraph(boss->BossX, boss->BossY, 3, 0.0, boss->BossImg[boss->BossDesign], TRUE);    //    보스 프린트
		DrawFormatString((int)boss->BossX - 28, (int)boss->BossY + 40, GetColor(255, 255, 255), "HP:%d", boss->BossHeart);    //    아래에 체력 정보 출력
	}
	else
		boss->BossExist = 2;            //    보스가 플레이어에 의해 사망했다는 정보
}

//	보스에게 공격 시 보스 체력 깎이게 하는 함수
void Attack_Boss_1(Boss_attack* boss_attack, Character* character,
	const int character_history_x, const int character_history_y, time_t boss_fire_time, int* health, time_t* immortal_t, Sound sound)
{
	if (boss_fire_time + 1 >= time(NULL))											// 1초마다 카서스 q 실행
	{
		boss_attack->fire_x = character_history_x + rand() % 20 - 10;				// 1초 전의 캐릭터 x 좌표에서 +- 10 한 x 좌표 설정
		boss_attack->fire_y = character_history_y + rand() % 20 - 10;				// 1초 전의 캐릭터 y 좌표에서 +- 10 한 y 좌표 설정
		DrawRotaGraph(boss_attack->fire_x, boss_attack->fire_y, 0.3, 0.0, boss_attack->Boss_fire_img, TRUE);		// 카서스 q 소환
	}

	if (abs(character->CharX - boss_attack->fire_x) <= 10 && abs(character->CharY - boss_attack->fire_y) <= 10)		// hitbox에 맞으면
		if (*immortal_t + 1 < time(NULL))																			// 무적시간이 지나면
			hit(character, health, immortal_t, sound);																		// hit 함수 실행
}

void Attack_Boss_2(Straight_fire* straight_fire, Character* character, time_t boss_fire_time, int* health, time_t* immortal_t, Boss* boss, Sound sound)
{
	if (boss_fire_time + 2 >= time(NULL))
		DrawRotaGraph(straight_fire->x, straight_fire->y, 0.3, straight_fire->angle, straight_fire->Boss_straight_fire_img, TRUE);

	int diffx = character->CharX - straight_fire->x;
	int diffy = character->CharY - straight_fire->y;
	if (boss->BossDesign == 2 || boss->BossDesign == 1)											
		if (abs(diffx) <= 200 && abs(diffy) <= 35)										// 오른쪽, 왼쪽 불 발사 시 데미지 계산
			if (*immortal_t + 1 < time(NULL))
				hit(character, health, immortal_t, sound);
	if(boss->BossDesign == 0 || boss->BossDesign == 3)									// 위, 아래 불 발사 시 데미지 계산
		if(abs(diffx) <= 35 && abs(diffy) <= 200)
			if (*immortal_t + 1 < time(NULL))
				hit(character, health, immortal_t, sound);
}

// boss design 2 : 오른쪽/	 0 : 아래/	 3: 위/	 1:왼
void Attack_Boss_2_xy(Straight_fire* straight_fire, time_t* straight_fire_time, Boss* boss)
{
	if (*straight_fire_time + 2 < time(NULL))
	{
		if (boss->BossDesign == 2)
		{
			straight_fire->angle = 0.0;
			straight_fire->x = boss->BossX + 200;
			straight_fire->y = boss->BossY;
		}
		if (boss->BossDesign == 1)
		{
			straight_fire->angle = 0.0;
			straight_fire->x = boss->BossX - 200;
			straight_fire->y = boss->BossY;
		}
		if (boss->BossDesign == 3)
		{
			straight_fire->angle = PI / 2;
			straight_fire->x = boss->BossX;
			straight_fire->y = boss->BossY - 200;
		}
		if (boss->BossDesign == 0)
		{
			straight_fire->angle = PI / 2;
			straight_fire->x = boss->BossX;
			straight_fire->y = boss->BossY + 200;
		}
		*straight_fire_time = time(NULL);
	}
}

//	게임 오버 출력용 함수
void Gameover(int image)
{
	//ScreenFlip ProcessMessage ClearDrawScreen UpdateKey 함수는 모두 정상 작동할 때 0을 반환한다
	while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0 && UpdateKey() == 0)
	{
		DrawRotaGraph(630, 450, 2, 0.0, image, TRUE);		//	게임 오버 이미지 출력
		if (nkey[KEY_INPUT_ESCAPE] >= 1)					//	만약 사용자가 ESC 키를 눌렀다면
			return;											//	함수 종료 -> 프로그램 종료
	}
}

//	게임 클리어 출력용 함수
void GameClear(int image)
{
	//ScreenFlip ProcessMessage ClearDrawScreen UpdateKey 함수는 모두 정상 작동할 때 0을 반환한다
	while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0 && UpdateKey() == 0)
	{
		DrawRotaGraph(630, 450, 2, 0.0, image, TRUE);		//	게임 클리어 이미지 출력
		if (nkey[KEY_INPUT_ESCAPE] >= 1)					//	만약 사용자가 ESC 키를 눌렀다면
			DxLib_End();									//	게임 종료
	}
}

//	게임이 클리어되었는지 계산하는 함수
void CheckGameClear(Boss boss, Monster monster[], int countmonster, int health, int image)
{
	int sum = 0;
	for (int i = 0; i < countmonster; i++)
		sum += monster[i].MobExist;			//	모든 몬스터의 exist값을 더한다, 0 = 소환전, 1 = 소환됨, 2 = 플레이어에 의해 사망

	if (boss.BossExist == 2 && sum == 10 && health >= 1)	//	만약 보스가 죽었고, 몬스터가 모두 죽었고, 캐릭터 체력이 1이상이라면
		GameClear(image);					//	게임 클리어 함수로 넘어감
	else
		return;								//	조건이 만족되지 않았다면 함수만 종료	
}


/*=======================================================================================================================================================================*/


//  메인 함수이자 게임 구동부
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)   
{
	DxLib_Set();    //  라이브러리 기본 설정
	
	//	필요한 구조체 선언
	time_t reload_t;                                                        // 장전을 시작한 때
	time_t fire_t = { 0 };                                                  // 총알을 발사한 때
	time_t immortal_t;                                                      // 피격 시 무적시간
	time_t character_history = time(NULL);
	time_t boss_fire_time = { 0 };
	time_t straight_fire_time = { 0 };
	Character character = { 0 };                                            //  캐릭터 구조체 선언
	Monster monster[20] = { 0 };                                            //  몬스터 구조체 선언
	Movement movement = { 0 };												// 방향키 입력 횟수 체크
	Boss boss = { 0 };														//	보스 구조체 선언
	Straight_fire straight_fire = { 0 };									// 일직선 불덩이 x,y좌표 구조체 선언
	Boss_attack boss_attack = { 0 };										// 보스 스킬 x, y좌표 선언
	Image image;															//	이미지 구조체 선언
	Sound sound;															//	효과음 구조체 선언

	//	필요한 변수 선언
	int Font1 = CreateFontToHandle("NULL", 70, 5, DX_FONTTYPE_ANTIALIASING_EDGE);   //  FONT
	int i;																			//  반복문 전용 변수
	int selector = 4;																//	시작 화면에서 게임시작 : 1, 플레이 방법 : 2, 종료 : 3
	int health = 7;																	//  체력 3개
	int remain_bullet = 10;															//  총알 10발
	int bullet_time = 0;															//  0 : 장전하지 않는 상태 , 1 : 장전 중인 상태
	int map_x = 0;																	//  맵 x좌표
	int map_y = 0;																	//  맵 y좌표
	int character_history_x = 0;													//	1초전 캐릭터 위치좌표 초기화
	int character_history_y = 0;													//	1초전 캐릭터 위치좌표 초기화
	character.CharX = 634;															//  캐릭터 기본 x좌표
	character.CharY = 486;															//  캐릭터 기본 y좌표
	LoadDivGraph("Image\\character.png", 16, 4, 4, 32, 32, character.CharImg);      //  캐릭터 이미지 저장
	character.CharDesign = 0;														//  캐릭터 기본 방향 0으로 설정
	LoadDivGraph("Image\\heart.png", 3, 3, 1, 32, 32, character.Heart);             //  체력 아이콘 저장
	LoadDivGraph("Image\\bullet.png", 16, 4, 4, 32, 32, character.Bullet);          //  총알 이미지 저장
	srand(time(NULL));																//	몬스터 소환 시마다 위치가 랜덤이 되도록, 게임 실행 시마다 보스 스테이지 위치가 랜덤이 되도록
	
	//	보스 관련 변수 저장
	int boss_room_x = rand() % 7 - 3;												//  보스 맵 x좌표 -3~3 랜덤
	int boss_room_y = rand() % 7 - 3;												//  보스 맵 y좌표 -3~3 랜덤
	boss_attack.Boss_fire_img = LoadGraph("Image\\fire.png");						//  보스 스킬 1. 펑
	straight_fire.Boss_straight_fire_img = LoadGraph("Image\\straight_fire.png");	//	보스 스킬 2. 1자펑
	for (int i = 0; i < 20; i++)
		monster[i].MobImg = LoadGraph("Image\\monster.png");
	LoadDivGraph("Image\\Boss.png", 4, 1, 4, 32, 32, boss.BossImg);         //  보스 이미지 저장
	boss.BossX = 634;														//	보스 기본 x좌표 초기화
	boss.BossY = 486;														//	보스 기본 y좌표 초기화
	boss.BossHeart = 2000;													//	보스 기본 체력 초기화
	int boss_map_flag = 0;													//	현재 스테이지가 보스맵인지 체크하는 flag (0 = 일반맵, 1 = 보스맵 )

	//	이미지 관련 구조체 변수값 저장
	image.map = LoadGraph("Image\\game_map.png");                           //  맵 이미지 저장
	image.start = LoadGraph("Image\\game_intro.png");						//	시작 화면 이미지 저장
	image.bossmap = LoadGraph("Image\\game map_boss.png");					//	보스맵 이미지 저장
	image.howtoplay = LoadGraph("Image\\howtoplay.png");					//	조작 방법 이미지 저장
	image.gameover = LoadGraph("Image\\gameover.png");						//	게임 오버 이미지 저장
	image.gamewin = LoadGraph("Image\\gameclear.png");						//	게임 클리어 이미지 저장
	
	//	효과음 관련 구조체 변수값 저장
	sound.gunsound = LoadSoundMem("Sound\\gun_sound.mp3");					//	총 효과음 저장
	sound.reloadsound = LoadSoundMem("Sound\\gunreload.wav");				//	장전 효과음 저장
	sound.charhitsound = LoadSoundMem("Sound\\charhit.mp3");				//	캐릭터 공격당한 효과음 저장
	sound.doorsound = LoadSoundMem("Sound\\opendoor.wav");					//	문 열고 들어갈 때 효과음 저장


	/*=======================================================================================================================================================================*/


	//	게임 시작
	Start(image);

	//  게임 구동부
	//ScreenFlip ProcessMessage ClearDrawScreen UpdateKey 함수는 모두 정상 작동할 때 0을 반환한다
	while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0 && UpdateKey() == 0)
	{
		if (boss_map_flag == 0)
			Print_Map(image.map);
		else if (boss_map_flag == 1)
			Print_Map(image.bossmap);
		Print_Map(image.map);						//  맵 출력
		Character_Move(&character, &character_history, &character_history_x, &character_history_y, &movement);					//  사용자 키입력 받아서 캐릭터 좌표 계산
		Print_Character(&character);                //  계산된 캐릭터 좌표에 캐릭터 출력
		for (i = 0; i < 5; i++)
			MOB_XY(&monster[i]);					//	몬스터 최초 좌표 생성
		for (i = 0; i < 5; i++)                     //  몬스터 움직이게 함
			Move_Mob(&character, &monster[i], &health, &immortal_t, sound);
		for (i = 0; i < 5; i++)
			Print_Mob(&monster[i]);					// 몬스터 출력
		for (i = 0; i < 5; i++)
			Attack_Monster(&character, &monster[i], &boss, &fire_t, bullet_time);				//	몬스터에게 공격하면 히트박스 계산해서 체력 감소시킴

		Shot_Bullet(&character, &remain_bullet, &reload_t, &fire_t, &bullet_time, sound);       //캐릭터 위치에서 무기 발사
		Print_Heart(&character, health);														//	체력 출력
		if (map_x == boss_room_x && map_y == boss_room_y)										//	만약 보스 스테이지에 도달했다면
		{
			boss_map_flag = 1;
			boss_move(&boss, &character, &immortal_t, &health);									//	보스 이동 계산
			Boss_Print(&boss);																	//  보스 그림 출력
			Attack_Boss_1(&boss_attack, &character, character_history_x, character_history_y, boss_fire_time, &health, &immortal_t, sound);		// 카서스 q
			Attack_Boss_2_xy(&straight_fire, &straight_fire_time, &boss);						// 2초마다 공격 위치 초기화
			Attack_Boss_2(&straight_fire, &character, boss_fire_time, &health, &immortal_t, &boss, sound);	// 일자로 불 쏨
			boss_fire_time = time(NULL);														// 카서스 q 쿨 초기화
			CheckGameClear(boss, monster, 5, health, image.gamewin);							//	게임 클리어 조건과 비교해서 클리어했다면 클리어함수로 넘어가는 함수 사용
		}
		else
			Move_Map(&character, monster, &map_x, &map_y, sound);								//	보스 스테이지가 아니라면 몬스터를 다 죽이면 맵을 이동할 수 있게 하는 함수 사용

		//	글씨 각종 정보 출력
		DrawFormatString(750, 130, GetColor(255, 255, 255), "Bullet : %d", remain_bullet);                      //  남은 총알 개수 출력
		DrawFormatString(300, 150, GetColor(255, 255, 255), "Current Location X : %d, Y : %d", map_x, map_y);
		DrawFormatString(300, 130, GetColor(255, 255, 255), "BOSS Location X : %d, Y : %d", boss_room_x, boss_room_y);
	
		if (health == 0)            //  남은 체력이 0이면
			break;                  //  반복문 탈출 (게임 종료)
	}
	Gameover(image.gameover);		//	게임 오버 출력
	DxLib_End();					//	프로그램 종료
}