#include <genesis.h>
#include "sprites.h"

typedef struct
{
    char *name;
    f32 x;
    f32 y;
    char *phrase;
    f32 speed;
    u16 id;
    u16 hp;
    u16 type;
    u8 pal;
    bool priority;
    bool flipH;
} ActorData;

typedef struct
{
    Sprite *sprite;
    union
    {
        ActorData;
        ActorData data;
    };
    
} GameObject;

typedef struct
{
    GameObject;
} Player;

typedef struct
{
    GameObject;
} Enemy;

#include "objects.h"

#define ENEMY_COUNT  (sizeof(enemiesData)/sizeof(enemiesData[0]))

Player player;
Enemy enemies[3];
u16 objectIndex;

const SpriteDefinition *enemySprDefs[ENEMY_COUNT] = {&sprDefEnemy01, &sprDefEnemy02, &sprDefEnemy03};

void GameInit();

void Player_Create();


void Joy_EventHandler(u16 joy, u16 changed, u16 state);

void GameObject_Create(GameObject *gameObject, const ActorData *charData, const SpriteDefinition *sprDef);

void ActorData_DrawStats(const ActorData *actorData);

void UI_DrawCharUnderCurrentActor(char *symbol);

int main(bool hardReset)
{
    if (!hardReset)
        SYS_hardReset();
    
    GameInit();
    
    while (TRUE)
    {
        
        SPR_update();
        SYS_doVBlankProcess();
    }
    
    return 0;
}

void GameInit()
{
    SPR_init();
    

    
    GameObject_Create((GameObject *) &player, playersData[0], &sprDefSonic);
    SPR_setAnim(player.sprite, 1);
    
    for (u16 i = 0; i < ENEMY_COUNT; i++)
        GameObject_Create((GameObject *) enemies + i, enemiesData[i], enemySprDefs[i]);
    
    // Set up joypad event handler
    JOY_setEventHandler(Joy_EventHandler);
    
    VDP_setTextPalette(PAL3);

    VDP_drawText("Use DPAD to list char's stats", 5, 0);
}

void GameObject_Create(GameObject *gameObject, const ActorData *charData, const SpriteDefinition *sprDef)
{
    gameObject->data = *charData;
    
    // load palette
    PAL_setPalette(gameObject->pal, sprDef->palette->data, DMA);
    gameObject->sprite = SPR_addSprite(sprDef, F32_toInt(gameObject->x), F32_toInt(gameObject->y),
                                       TILE_ATTR(gameObject->pal, TRUE, FALSE, gameObject->flipH));
}

// Handles joypad input events
void Joy_EventHandler(u16 joy, u16 changed, u16 state)
{
    UI_DrawCharUnderCurrentActor(" ");
    
    // Handle speed adjustment
    if (changed & state & BUTTON_RIGHT)
    {
        if (objectIndex == ENEMY_COUNT - 1)
            objectIndex = 0;
        else
            objectIndex++;
    }
    else if (changed & state & BUTTON_LEFT)
    {
        if (objectIndex == 0)
            objectIndex = ENEMY_COUNT - 1;
        else
            objectIndex--;
    }
    
    ActorData_DrawStats(enemiesData[objectIndex]);
}

// Draw UI text
void ActorData_DrawStats(const ActorData *actorData)
{
    char str[14][40];
    u16 y = 0;

    // ================================== Pool usage =======================================
    // Draw the number of allocated / free objects for each pool
    sprintf(str[y++], " TMX OBJECT IMPORTED DATA:");
    sprintf(str[y++], "");
    sprintf(str[y++], "   name: %-3s", actorData->name);
    sprintf(str[y++], "   id: %d", actorData->id);
    sprintf(str[y++], "   X:%03d, Y:%03d", F32_toInt(actorData->x), F32_toInt(actorData->y));
    sprintf(str[y++], "   palette ind: %d", actorData->pal);
    sprintf(str[y++], "   priority: %-5s", (actorData->priority) ? "TRUE" : "FALSE");
    sprintf(str[y++], "   flipH: %-5s", (actorData->flipH) ? "TRUE" : "FALSE");
    sprintf(str[y++], "   type: %-5s", (actorData->priority) ? "TRUE" : "FALSE");
    sprintf(str[y++], "   speed: %02d.%d", F32_toInt(actorData->speed), (u16) (mulu(F32_frac(actorData->speed), 100) >> FIX32_FRAC_BITS));
    sprintf(str[y++], "   hp: %d", actorData->hp);
    sprintf(str[y++], "   phrase: %-69s", actorData->phrase);

    
    VDP_setTextPalette(PAL1);
    for (u16 i = 0; i < 15; i++)
        VDP_drawTextBG(BG_B, str[i], 0, 2 + i);

    VDP_setTextPalette(PAL3);
    UI_DrawCharUnderCurrentActor("^");
}

void UI_DrawCharUnderCurrentActor(char *symbol)
{
    VDP_drawTextBG(BG_B, symbol, (F32_toInt(enemiesData[objectIndex]->x) >> 3) + (enemySprDefs[objectIndex]->w >> 4),
                   (F32_toInt(enemiesData[objectIndex]->y) >> 3) + (enemySprDefs[objectIndex]->h >> 3) + 1);
}