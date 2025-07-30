#include "CCommon.h"
#include "CExampleGame.h"

////////////////////////////////////////////////////////////////
// 共通関数
////////////////////////////////////////////////////////////////
// 乱数取得
LONG GetMyRandVal(LONG lMin, LONG lMax)
{
    LONG lRand, lRange;

    do
    {
        lRand = (LONG)rand();
    } while(lRand == RAND_MAX);

    lRange = lMax - lMin + 1;
    lRand = (LONG)((DOUBLE)lRand / RAND_MAX * lRange) + lMin;

    return lRand;
}

