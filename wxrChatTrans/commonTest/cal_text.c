#include <stdio.h>

/* 玩家当前抽卡资源 */
typedef struct ArkAssets
{
    int organeStone;    // 源石
    int yellowCards;    // 黄票
    int purpleCards;    // 紫票
    int redStone;       // 合成玉
    int card;           // 单抽券
    int tenCards;       // 十连券
} ArkAssets;


int howMannyCards(ArkAssets assets)
{
    int ret = 0;
    if (assets.organeStone < 0 || assets.yellowCards < 0 || assets.purpleCards < 0\
     || assets.redStone < 0 || assets.card < 0 || assets.tenCards < 0)
    {
        printf("invalid data!\n");
        return -1;
    }

    int red_stone_free = ((assets.purpleCards > 9200) ? 46000 : assets.purpleCards * 5) + assets.redStone + (assets.card + assets.tenCards * 10) * 600;
    int red_stone_all = red_stone_free + assets.organeStone * 180 + (assets.yellowCards / 258) * 38 * 600;


    printf("不算源石与黄票的抽卡资源有%d抽，总计%d抽。\n", red_stone_free / 600, red_stone_all / 600);

    return ret;
}


int main()
{


    

    ArkAssets a1, a2;
    a1.card = 0;
    a1.tenCards = 0;
    a1.organeStone = 357;
    a1.purpleCards = 5280;
    a1.redStone = 76095;
    a1.yellowCards = 555;

    a2.card = 3;
    a2.tenCards = 0;
    a2.organeStone = 230;
    a2.purpleCards = 5130;
    a2.redStone = 41285;
    a2.yellowCards = 336;
    
    howMannyCards(a1);
    howMannyCards(a2);

    
    return 0;
}