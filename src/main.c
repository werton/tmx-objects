#include <genesis.h>
#include "sprites.h"

typedef struct
{
    char *name;
    f32 x;
    f32 y;
    f32 speed;
    u16 id;
    u16 hp;
    u16 type;
    u8 pal;
    bool priority;
    bool flipH;
} CharacterData;

typedef struct
{
    Sprite *sprite;
    union
    {
        CharacterData;
        CharacterData data;
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

Player player;
Enemy enemies[4];

void GameInit();
void Player_Create();
void Joy_Update();
void Joy_EventHandler(u16 joy, u16 changed, u16 state);

int main(bool hardReset)
{
    GameInit();
    
    while (TRUE)
    {
        Joy_Update();
        SPR_update();
        SYS_doVBlankProcess();
    }
    
    return 0;
}

void GameInit()
{
    
    VDP_drawText("Hello world !", 12, 12);
    SPR_init();
    Player_Create();
    // Set up joypad event handler
    JOY_setEventHandler(Joy_EventHandler);
}

void Player_Create()
{
    
    player.data = *playersData[0];
    
    // load palette
    PAL_setPalette(PAL0, eggman.palette->data, CPU);
    
    kprintf("x: %d", F32_toInt(player.x));
    kprintf("y: %d", F32_toInt(player.y));
    kprintf("speed: %d", F32_toInt(player.speed));
    
    // init sonic sprite
    player.sprite = SPR_addSprite(&eggman, F32_toInt(player.x), F32_toInt(player.y), TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
}



// Sets the position of a game object
void Object_SetPos(GameObject *object, f32 x, f32 y, bool isRelativePos)
{
    if (isRelativePos)
    {
        object->x += x;
        object->y += y;
    }
    else
    {
        object->x = x;
        object->y = y;
    }
    
    SPR_setPosition(object->sprite, F32_toInt(object->x), F32_toInt(object->y));
}

// Moves an object based on input constants direction
void Object_Move(GameObject *object, u16 value)
{
    if (value & BUTTON_RIGHT)
        Object_SetPos(object, object->speed, F32(0), TRUE);
    else if (value & BUTTON_LEFT)
        Object_SetPos(object, -object->speed, F32(0), TRUE);
    
    if (value & BUTTON_UP)
        Object_SetPos(object, F32(0), -object->speed, TRUE);
    else if (value & BUTTON_DOWN)
        Object_SetPos(object, F32(0), object->speed, TRUE);
}

// Handles joypad input events
void Joy_Update()
{
    u16 state = JOY_readJoypad(JOY_1);
    
    if (state & BUTTON_RIGHT)
        Object_Move((GameObject *) &player, BUTTON_RIGHT);
    else if (state & BUTTON_LEFT)
        Object_Move((GameObject *) &player, BUTTON_LEFT);
    
    if (state & BUTTON_UP)
        Object_Move((GameObject *) &player, BUTTON_UP);
    else if (state & BUTTON_DOWN)
        Object_Move((GameObject *) &player, BUTTON_DOWN);
    
}

// Handles joypad input events
void Joy_EventHandler(u16 joy, u16 changed, u16 state)
{
    // Handle speed adjustment
    
    if (changed & state & BUTTON_B)
    {
    
    }
    
}