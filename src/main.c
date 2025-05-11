#include <genesis.h>
#include "sprites.h"

typedef enum
{
    DEF_SONIC,
    DEF_BUZZ,
    DEF_CRAB,
    DEF_BOT,
    DEF_DISPLAY,
    DEF_COUNT
} SpriteDefEnum;

typedef enum
{
    TYPE_PLAYER,
    TYPE_ENEMY,
    TYPE_ITEM,
    TYPE_COUNT,
} ObjectType;

// Contains all data properties for game actors
typedef struct
{
    char *name;                 // Display name
    f32 x;                      // X position (fixed point)
    f32 y;                      // Y position (fixed point)
    ObjectType type;
    SpriteDefEnum sprDefInd;
    u16 id;                     // Unique ID
    u8 pal;                     // Palette index
    bool flipH;                 // Horizontal flip
    bool priority;              // Render priority
    bool enabled;               // Render priority
} TMX_BaseObjectData;

// Contains all data properties for game actors
typedef struct
{
    TMX_BaseObjectData;
    s16 hp;                     // Hit points
} TMX_ItemData;

// Contains all data properties for game actors
typedef struct
{
    TMX_BaseObjectData;
    char *phrase;               // Dialogue text
    f32 speed;                  // Movement speed
    s16 hp;                     // Hit points
    void *target;               // Horizontal flip
} TMX_ActorData;

// Base game object with sprite and data
typedef struct
{
    union
    {
        TMX_BaseObjectData data; // Named access
        TMX_BaseObjectData;      // Anonymous access
    };
    Sprite *sprite;             // Sprite reference
} GameObject;

// Base game object with sprite and data
typedef struct
{
    union
    {
        TMX_ItemData data;      // Named access
        TMX_ItemData;           // Anonymous access
    };
    Sprite *sprite;             // Sprite reference
} GameItem;

// Base game object with sprite and data
typedef struct
{
    union
    {
        TMX_ActorData data;     // Named access
        TMX_ActorData;          // Anonymous access
    };
    Sprite *sprite;             // Sprite reference
} GameActor;

typedef struct
{
    GameItem;                   // Inherits GameObject
    u16 size;                   // Not used
} Item;

// Player-specific object
typedef struct
{
    GameActor;                  // Inherits GameObject
    u16 lives;                  // Not used
} Player;

// Enemy-specific object
typedef struct
{
    GameActor;                  // Inherits GameObject
    V2ff32 wayPoint;            // Not used
} Enemy;

#include "objects.h"

// Constants
#define PLAYER_COUNT            (sizeof(playersData)/sizeof(playersData[0]))
#define ENEMY_COUNT             (sizeof(enemiesData)/sizeof(enemiesData[0]))
#define ITEM_COUNT              (sizeof(itemsData)/sizeof(itemsData[0]))
#define OBJECTS_COUNT           (ENEMY_COUNT+ITEM_COUNT+PLAYER_COUNT)
#define STAT_LINES              20
#define TEXT_BUFFER             40

#define STR(x) #x

// Game state
static Player player;
static Player players[PLAYER_COUNT];
static Enemy enemies[ENEMY_COUNT];
static Item items[ITEM_COUNT];
static u16 selectedObjectIndex = 0;
TMX_BaseObjectData *objectsList[OBJECTS_COUNT];

const char sprDefNames[DEF_COUNT][20] = {
    [DEF_SONIC] = STR(DEF_SONIC),
    [DEF_BUZZ] = STR(DEF_BUZZ),
    [DEF_CRAB] = STR(DEF_CRAB),
    [DEF_BOT] = STR(DEF_BOT),
    [DEF_DISPLAY] = STR(DEF_DISPLAY),
};

const char objectTypeNames[TYPE_COUNT][20] = {
    [TYPE_PLAYER] = STR(TYPE_PLAYER),
    [TYPE_ENEMY] = STR(TYPE_ENEMY),
    [TYPE_ITEM] = STR(TYPE_ITEM),
};


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

static void UI_DrawCursor(const char *symbol1, const char *symbol2);

static u16 UI_DrawObjectData(const TMX_BaseObjectData *object);

static void UI_DrawActorData(const TMX_ActorData *actor);

static void UI_DrawItemData(const TMX_ItemData *item);

static void UI_DrawData(const TMX_BaseObjectData *object);

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
    
    // Initialize enemies
    for (u16 i = 0; i < ENEMY_COUNT; i++)
    {
        GameActor_Init((GameActor *) &enemies[i], enemiesData[i], spriteDefs[enemiesData[i]->sprDefInd]);
        objectsList[i] = (TMX_BaseObjectData *) &enemies[i];
    }
    
    // Initialize enemies
    for (u16 i = 0; i < ITEM_COUNT; i++)
    {
        GameItem_Init((GameItem *) &items[i], itemsData[i], spriteDefs[itemsData[i]->sprDefInd]);
        objectsList[ENEMY_COUNT + i] = (TMX_BaseObjectData *) &items[i];
    }
    
    // Initialize player
    for (u16 i = 0; i < PLAYER_COUNT; i++)
    {
        GameActor_Init((GameActor *) &player, playersData[i], spriteDefs[playersData[i]->sprDefInd]);
        objectsList[ENEMY_COUNT + ITEM_COUNT + i] = (TMX_BaseObjectData *) &player;
    }
    
    SPR_setAnim(player.sprite, 1);
    // Set up input
    JOY_setEventHandler(Joy_Handler);
    
    // Setup UI
    VDP_setTextPalette(PAL2);
    VDP_drawText("USE DPAD TO SWITCH CHARACTER", 6, 0);
    UI_DrawData((const TMX_BaseObjectData *) objectsList[selectedObjectIndex]);
    UI_DrawCursor(">>", "<<");
}

// Initialize a game object
static void GameObject_Init(GameObject *obj, const TMX_BaseObjectData *data, const SpriteDefinition *sprDef)
{
    obj->data = *data;
    
    PAL_setPalette(obj->data.pal, sprDef->palette->data, DMA);
    obj->sprite = SPR_addSprite(sprDef, F32_toInt(obj->data.x), F32_toInt(obj->data.y),
        TILE_ATTR(obj->data.pal, TRUE, FALSE, obj->data.flipH));
}

// Initialize a game object
static void GameItem_Init(GameItem *obj, const TMX_ItemData *data, const SpriteDefinition *sprDef)
{
    obj->data = *data;
    
    PAL_setPalette(obj->data.pal, sprDef->palette->data, DMA);
    obj->sprite = SPR_addSprite(sprDef, F32_toInt(obj->data.x), F32_toInt(obj->data.y),
                                TILE_ATTR(obj->data.pal, TRUE, FALSE, obj->data.flipH));
}

// Initialize a game object
static void GameActor_Init(GameActor *obj, const TMX_ActorData *data, const SpriteDefinition *sprDef)
{
    obj->data = *data;
    
    PAL_setPalette(obj->data.pal, sprDef->palette->data, DMA);
    obj->sprite = SPR_addSprite(sprDef, F32_toInt(obj->data.x), F32_toInt(obj->data.y),
                                TILE_ATTR(obj->data.pal, TRUE, FALSE, obj->data.flipH));
}

// Handle joypad input
static void Joy_Handler(u16 joy, u16 changed, u16 state)
{
    UI_DrawCursor("  ", "  ");
    
    if (changed & state & BUTTON_RIGHT)
        selectedObjectIndex = (selectedObjectIndex == OBJECTS_COUNT - 1) ? 0 : selectedObjectIndex + 1;
    else if (changed & state & BUTTON_LEFT)
        selectedObjectIndex = (selectedObjectIndex == 0) ? OBJECTS_COUNT - 1 : selectedObjectIndex - 1;
    
    UI_DrawData((const TMX_BaseObjectData *) objectsList[selectedObjectIndex]);
    
    UI_DrawCursor(">>", "<<");
}

void UI_DrawStringsArray(const char *text, u16 fromY, u16 length)
{
    VDP_setTextPalette(PAL1);
    for (u16 i = 0; i < length; i++)
    {
        if (text + i * TEXT_BUFFER != NULL)
            VDP_drawTextBG(BG_A, text + i * TEXT_BUFFER, 0, 2 + fromY + i);
    }
}


// Draw actor statistics
static u16 UI_DrawBaseObjectData(const TMX_BaseObjectData *object)
{
    static char text[STAT_LINES][TEXT_BUFFER];
    u16 y = 0;
    
    sprintf(text[y++], "");
    sprintf(text[y++], " _______ TMX DATA _______");
    sprintf(text[y++], " Name:     %-11s", object->name);
    sprintf(text[y++], " Type:     %-15s", objectTypeNames[object->type]);
    sprintf(text[y++], " Id:       %d", object->id);
    sprintf(text[y++], " Enabled:  %-5s", object->enabled ? "TRUE" : "FALSE");
    sprintf(text[y++], " Pos:      X:%03d, Y:%03d", F32_toInt(object->x), F32_toInt(object->y));
    sprintf(text[y++], " SprDefInd:%-15s", sprDefNames[object->sprDefInd]);
    sprintf(text[y++], " PalInd:   %d", object->pal);
    sprintf(text[y++], " Prio:     %-5s", object->priority ? "TRUE" : "FALSE");
    sprintf(text[y++], " FlipH:    %-5s", object->flipH ? "TRUE" : "FALSE");
    
    UI_DrawStringsArray((const char *) text, 0, y);
    return y;
}


// Draw actor statistics
static void UI_DrawActorData(const TMX_ActorData *actor)
{
    static char text[STAT_LINES][TEXT_BUFFER];
    u16 y = UI_DrawBaseObjectData((const TMX_BaseObjectData *) actor);
    
    sprintf(text[y++], " Target:   %s(ptr:%p) %-10s", (actor->target != NULL) ? ((TMX_ActorData *) actor->target)->name : "NONE",
            actor->target, "");
    sprintf(text[y++], " Speed:    %02d.%d %-25s", F32_toInt(actor->speed), (u16) (mulu(F32_frac(actor->speed), 100) >> FIX32_FRAC_BITS),
            "");
    sprintf(text[y++], " HP:       %-29d", actor->hp);
    sprintf(text[y++], " Phrase:   %-70s", actor->phrase);
    y++;
    sprintf(text[y++], " ______________________________________");
    
    UI_DrawStringsArray((const char *) text, 0, y);
}

// Draw item statistics
static void UI_DrawItemData(const TMX_ItemData *item)
{
    static char text[STAT_LINES][TEXT_BUFFER];
    u16 y = UI_DrawBaseObjectData((const TMX_BaseObjectData *) item);
    
    sprintf(text[y++], " HP:       %-29d", item->hp);
    sprintf(text[y++], " ______________________________________");
    sprintf(text[y++], "%-40s", "");
    sprintf(text[y++], "%-40s", "");
    sprintf(text[y++], "%-40s", "");
    sprintf(text[y++], "%-40s", "");
    
    UI_DrawStringsArray((const char *) text, 0, y);
}

static void UI_DrawData(const TMX_BaseObjectData *object)
{
    switch (object->type)
    {
        case TYPE_PLAYER:
        case TYPE_ENEMY:
            UI_DrawActorData((const TMX_ActorData *) object);
            break;
        
        case TYPE_ITEM:
            UI_DrawItemData((const TMX_ItemData *) object);
            break;
    }
}

// Draw selection marker
static void UI_DrawCursor(const char *symbol1, const char *symbol2)
{
    const TMX_BaseObjectData *object = objectsList[selectedObjectIndex];
    const SpriteDefinition *sprDef = spriteDefs[object->sprDefInd];
    
    s16 x1 = (F32_toInt(object->x) >> 3) - (sprDef->w >> 4);
    s16 x2 = (F32_toInt(object->x) >> 3) + (sprDef->w>>3) + (sprDef->w >> 4) - 1;
    s16 y = (F32_toInt(object->y) >> 3) + (sprDef->h >> 4);
    
    VDP_setTextPalette(PAL3);
    VDP_drawTextBG(BG_B, symbol1, x1, y);
    VDP_drawTextBG(BG_B, symbol2, x2, y);
}
