#ifndef EXAMPLE_GAME_H
#define EXAMPLE_GAME_H

#include <windows.h>

////////////////////////////////////////////////////////////////
// ゲーム画面用の構造体
////////////////////////////////////////////////////////////////
// スプライト
typedef struct tag_sprite
{
    DWORD dwSpriteID;  // 識別子
    RECT rcImage;  // イメージ内座標
    int nPosX;  // 表示座標(X)
    int nPosY;  // 表示座標(Y)
    int nState;  // 状態
    BYTE byAlpha;  // アルファブレンド
    int nCurrentCell;  // 表示セル
    int nMaxCell;  // 最大セル数
    DWORD dwDelay;  // 描画更新カウント
    DWORD dwLastTickCount;  // 経過時間カウント
} SPRITE;

// 操作キャラ
typedef struct tag_playercharacter
{
    SPRITE sp;
    int nLifeMax;
    int nLife;
    int nLifeTmp;
    WORD nIsAttack;
    WORD nWeaponType;
    WORD nWeaponTypeOld;
    WORD nWeaponLevel[3];
    int nMoveX;

    tag_playercharacter();
    void InitData(DWORD dwTickCount = 0);
    void OnMoveX(int x);
    void ChangeSpriteState();
    void SetSpriteRect();
} MYPLAYERCHARACTER;

// 敵キャラ
typedef struct
{
    SPRITE sp;
    int nLifeMax;
    int nLife;
    WORD nWeaponType;
    BOOL bDispLife;
    DWORD dwScore;
    int nAttack;
    int nMoveX;
    int nMoveY;
} MYENEMY;

// 弾
typedef struct 
{
    SPRITE sp;
    int nMoveX;
    int nMoveY;
} MYBULLET;

#endif  // EXAMPLE_GAME_H
