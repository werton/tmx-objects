#include <genesis.h>
#include "sprites.h"

typedef enum spiteDefEnum
{
    DEF_SONIC,
    DEF_BUZZ,
    DEF_CRAB,
    DEF_BOT,
    DEF_DISPLAY
} spriteDefEnum;


// Contains all data properties for game actors
typedef struct
{
    char *name;                 // Display name
    f32 x;                      // X position (fixed point)
    f32 y;                      // Y position (fixed point)
    u16 id;                     // Unique ID
    spriteDefEnum sprDefInd;
    u16 type;                   // Actor type
    u8 pal;                     // Palette index
    bool flipH;                 // Horizontal flip
    bool priority;              // Render priority
    bool enabled;               // Render priority
} TMX_BaseObjectData;

// Contains all data properties for game actors
typedef struct
{
    TMX_BaseObjectData;
    s16 hp;             // Hit points
} TMX_ItemData;

// Contains all data properties for game actors
typedef struct
{
    TMX_BaseObjectData;
    char *phrase;       // Dialogue text
    f32 speed;          // Movement speed
    s16 hp;             // Hit points
    void *target;         // Horizontal flip
} TMX_ActorData;

// Base game object with sprite and data
typedef struct
{
    union
    {
        TMX_BaseObjectData data; // Named access
        TMX_BaseObjectData;      // Anonymous access
    };
    Sprite *sprite;     // Sprite reference
} GameObject;

// Base game object with sprite and data
typedef struct
{
    union
    {
        TMX_ItemData data; // Named access
        TMX_ItemData;      // Anonymous access
    };
    Sprite *sprite;     // Sprite reference
} GameItem;

// Base game object with sprite and data
typedef struct
{
    union
    {
        TMX_ActorData data; // Named access
        TMX_ActorData;      // Anonymous access
    };
    Sprite *sprite;     // Sprite reference
} GameActor;

typedef struct
{
    GameItem;           // Inherits GameObject
    u16 size;           // Not used
} Item;

// Player-specific object
typedef struct
{
    GameActor;          // Inherits GameObject
    u16 lives;          // Not used
} Player;

// Enemy-specific object
typedef struct
{
    GameActor;          // Inherits GameObject
    V2ff32 wayPoint;    // Not used
} Enemy;

#include "objects.h"

// Constants
#define PLAYER_COUNT            (sizeof(playersData)/sizeof(playersData[0]))
#define ENEMY_COUNT             (sizeof(enemiesData)/sizeof(enemiesData[0]))
#define ITEM_COUNT              (sizeof(itemsData)/sizeof(itemsData[0]))
#define OBJECTS_COUNT           (ENEMY_COUNT+ITEM_COUNT+PLAYER_COUNT)
#define STAT_LINES              20
#define TEXT_BUFFER             40


// Game state
static Player player;
static Player players[PLAYER_COUNT];
static Enemy enemies[ENEMY_COUNT];
static Item items[ITEM_COUNT];
static u16 currentEnemyIndex = 0;

// Sprite definitions
const SpriteDefinition *spriteDefs[OBJECTS_COUNT] = {
    [DEF_SONIC] = &sprDefSonic,
    [DEF_BUZZ] = &sprDefBuzz,
    [DEF_CRAB] = &sprDefCrab,
    [DEF_BOT] = &sprDefBot,
    [DEF_DISPLAY] = &sprDefDisplay,
};

// Forward declarations
static void Game_Init();

static void Joy_Handler(u16 joy, u16 changed, u16 state);

static void GameObject_Init(GameObject *obj, const TMX_BaseObjectData *data, const SpriteDefinition *sprDef);

static void GameItem_Init(GameItem *obj, const TMX_ItemData *data, const SpriteDefinition *sprDef);

static void GameActor_Init(GameActor *obj, const TMX_ActorData *data, const SpriteDefinition *sprDef);

static void UI_DrawStats(const TMX_ActorData *actor);

static void UI_DrawMarker(const char *symbol);


// Entry point
int main(bool hardReset)
{
    if (!hardReset)
        SYS_hardReset();
    
    Game_Init();
    
    while (TRUE)
    {
        SPR_update();
        SYS_doVBlankProcess();
    }
    
    return 0;
}

// Initialize game state
static void Game_Init()
{
    SPR_init();
    
    // Initialize player
    for (u16 i = 0; i < PLAYER_COUNT; i++)
        GameActor_Init((GameActor *) &player, playersData[i], spriteDefs[playersData[i]->sprDefInd]);
    
    // Initialize enemies
    for (u16 i = 0; i < ENEMY_COUNT; i++)
        GameActor_Init((GameActor *) &enemies[i], enemiesData[i], spriteDefs[enemiesData[i]->sprDefInd]);
    
    // Initialize enemies
    for (u16 i = 0; i < ITEM_COUNT; i++)
        GameItem_Init((GameItem *) &items[i], itemsData[i], spriteDefs[itemsData[i]->sprDefInd]);
    
    SPR_setAnim(player.sprite, 1);
    // Set up input
    JOY_setEventHandler(Joy_Handler);
    
    // Setup UI
    VDP_setTextPalette(PAL2);
    VDP_drawText("USE DPAD TO SWITCH CHARACTER", 6, 0);
    UI_DrawStats(enemiesData[currentEnemyIndex]);
}

// Initialize a game object
static void GameObject_Init(GameObject *obj, const TMX_BaseObjectData *data, const SpriteDefinition *sprDef)
{
    obj->data = *data;
    
    PAL_setPalette(obj->data.pal, sprDef->palette->data, DMA);
    obj->sprite = SPR_addSprite(
        sprDef,
        F32_toInt(obj->data.x),
        F32_toInt(obj->data.y),
        TILE_ATTR(obj->data.pal, TRUE, FALSE, obj->data.flipH)
    );
}

// Initialize a game object
static void GameItem_Init(GameItem *obj, const TMX_ItemData *data, const SpriteDefinition *sprDef)
{
    obj->data = *data;
    
    PAL_setPalette(obj->data.pal, sprDef->palette->data, DMA);
    obj->sprite = SPR_addSprite(
        sprDef,
        F32_toInt(obj->data.x),
        F32_toInt(obj->data.y),
        TILE_ATTR(obj->data.pal, TRUE, FALSE, obj->data.flipH)
    );
}

// Initialize a game object
static void GameActor_Init(GameActor *obj, const TMX_ActorData *data, const SpriteDefinition *sprDef)
{
    obj->data = *data;
    
    PAL_setPalette(obj->data.pal, sprDef->palette->data, DMA);
    obj->sprite = SPR_addSprite(
        sprDef,
        F32_toInt(obj->data.x),
        F32_toInt(obj->data.y),
        TILE_ATTR(obj->data.pal, TRUE, FALSE, obj->data.flipH)
    );
}

// Handle joypad input
static void Joy_Handler(u16 joy, u16 changed, u16 state)
{
    UI_DrawMarker(" ");
    
    if (changed & state & BUTTON_RIGHT)
    {
        currentEnemyIndex = (currentEnemyIndex == ENEMY_COUNT - 1) ? 0 : currentEnemyIndex + 1;
    }
    else if (changed & state & BUTTON_LEFT)
    {
        currentEnemyIndex = (currentEnemyIndex == 0) ? ENEMY_COUNT - 1 : currentEnemyIndex - 1;
    }
    
    UI_DrawStats(enemiesData[currentEnemyIndex]);
}

// Draw actor statistics
static void UI_DrawStats(const TMX_ActorData *actor)
{
    static char text[STAT_LINES][TEXT_BUFFER];
    u16 y = 0;
    
    sprintf(text[y++], "");
    sprintf(text[y++], " _______ TMX DATA _______");
    sprintf(text[y++], " Name:   %-11s", actor->name);
    sprintf(text[y++], " Id:     %d", actor->id);
    sprintf(text[y++], " Pos:    X:%03d, Y:%03d", F32_toInt(actor->x), F32_toInt(actor->y));
    sprintf(text[y++], " PalInd: %d", actor->pal);
    sprintf(text[y++], " Prio:   %-5s", actor->priority ? "TRUE" : "FALSE");
    sprintf(text[y++], " FlipH:  %-5s", actor->flipH ? "TRUE" : "FALSE");
    sprintf(text[y++], " Target: %s(ptr:%p) %-5s", ((TMX_ActorData *) actor->target)->name, actor->target, "");
    sprintf(text[y++], " Type:   %-5s", actor->priority ? "TRUE" : "FALSE");
    sprintf(text[y++], " Speed:  %02d.%d", F32_toInt(actor->speed), (u16) (mulu(F32_frac(actor->speed), 100) >> FIX32_FRAC_BITS));
    sprintf(text[y++], " HP:     %d", actor->hp);
    sprintf(text[y++], " Phrase: %-70s", actor->phrase);
    sprintf(text[y + 1], " ______________________________________");
    
    VDP_setTextPalette(PAL1);
    for (u16 i = 0; i < STAT_LINES; i++)
    {
        if (text[i] != NULL)
            VDP_drawTextBG(BG_A, text[i], 0, 2 + i);
    }
    
    VDP_setTextPalette(PAL3);
    UI_DrawMarker("^");
    
}

// Draw selection marker
static void UI_DrawMarker(const char *symbol)
{
    const TMX_ActorData *actor = enemiesData[currentEnemyIndex];
    const SpriteDefinition *sprDef = spriteDefs[actor->sprDefInd];
    
    s16 x = (F32_toInt(actor->x) >> 3) + (sprDef->w >> 4);
    s16 y = (F32_toInt(actor->y) >> 3) + (sprDef->h >> 3) + 1;
    
    VDP_drawTextBG(BG_B, symbol, x, y);
    VDP_drawTextBG(BG_B, symbol, x, y + 1);
}