// *****************************************************************************
//  TMX-Objects sample
//
//  This sample shows how you can load object data from TMX files (Tiled map format).
//  It shows loading of built-in 'name', 'x', 'y', and various custom fields using Tiled
//  types 'int', 'float', 'bool', 'string', 'object', 'Enum', placing objects of same and
//  different types on different and identical layers and filtering of objects at loading.
//
//  Use DPAD to toggle the active object and the description of its statistics
//
//  written by werton playskin on 05/2025
// *****************************************************************************

#include <genesis.h>
#include "sprites.h"

// Enumeration and type declaration

// Sprite definition enumerations
typedef enum
{
    DEF_SONIC,
    DEF_BUZZ,
    DEF_CRAB,
    DEF_BOT,
    DEF_DISPLAY,
    DEF_COUNT
} SpriteDefEnum;

// Object type enumerations
typedef enum
{
    TYPE_PLAYER,
    TYPE_ENEMY,
    TYPE_ITEM,
    TYPE_TEXT,
    TYPE_COUNT,
} ObjectTypeEnum;

// Base object with built-in fields data properties
typedef struct
{
    // built-in fields
    u16 id;                     // id
    char *name;                 // Display name
    f32 x;                      // X position (fixed point)
    f32 y;                      // Y position (fixed point)
    bool visible;               //
} BaseObjectData;

// Actor data properties
typedef struct
{
    union
    {
        BaseObjectData base;        // Named access
        BaseObjectData;             // Anonymous access
    };
    // custom fields
    ObjectTypeEnum type;        // Object type index
    SpriteDefEnum sprDefInd;    // Sprite definition index
    u8 pal;                     // Palette index
    bool flipH;                 // Horizontal flip flag
    bool flipV;                 // Horizontal flip flag
    bool priority;              // priority flag
    bool enabled;               // state flag (not used just for example)
} EntityData;

// Game item object with sprite and data
typedef struct
{
    EntityData;
    u16 hp;
} ItemData;

// Body data properties
typedef struct
{
    union
    {
        EntityData entity;        // Named access
        EntityData;             // Anonymous access
    };
    // custom fields
    char *phrase;               // Dialogue text
    f32 speed;                  // Movement speed (not used just for example)
    s16 hp;                     // Hit points (not used just for example)
    void *target;               // Target reference
} ActorData;

// Extended object data for text
typedef struct
{
    union
    {
        BaseObjectData base;        // Named access
        BaseObjectData;             // Anonymous access
    };
    ObjectTypeEnum type;        // Object type index
    //char *text;
} TextData;

// Game item object with sprite and data
typedef struct
{
    union
    {
        ItemData item;      // Named access
        ItemData;            // Anonymous access
    };
    Sprite *sprite;              // Sprite reference
} Item;

// Player-specific object
typedef struct
{
    union
    {
        ActorData actor;      // Named access
        ActorData;            // Anonymous access
    };
    u16 lives;                  // Lives count (not used just for example)
    Sprite *sprite;              // Sprite reference
} Player;

// Enemy-specific object
typedef struct
{
    union
    {
        ActorData actor;      // Named access
        ActorData;            // Anonymous access
    };             // Inherits Actor
    V2ff32 wayPoint;            // Movement waypoint (not used just for example)
    Sprite *sprite;              // Sprite reference
} Enemy;


#include "objects.h"

// Game constants
#define PLAYER_COUNT            (sizeof(playersData)/sizeof(playersData[0]))
#define ENEMY_COUNT             (sizeof(enemiesData)/sizeof(enemiesData[0]))
#define ITEM_COUNT              (sizeof(itemsData)/sizeof(itemsData[0]))
#define TEXT_COUNT              (sizeof(textData)/sizeof(textData[0]))
#define OBJECTS_COUNT           (ENEMY_COUNT+ITEM_COUNT+PLAYER_COUNT+TEXT_COUNT)
#define STAT_LINES              20
#define TEXT_BUFFER             40

// Stringification macro
#define STR(x) #x

// Game state variables
Player players[PLAYER_COUNT];
Enemy enemies[ENEMY_COUNT];
Item items[ITEM_COUNT];
TextData texts[TEXT_COUNT];
u16 selectedObjectIndex = 0;
EntityData *objectsList[OBJECTS_COUNT];

// Sprite definition names
const char sprDefNames[DEF_COUNT][20] = {
    [DEF_SONIC] = STR(DEF_SONIC),
    [DEF_BUZZ] = STR(DEF_BUZZ),
    [DEF_CRAB] = STR(DEF_CRAB),
    [DEF_BOT] = STR(DEF_BOT),
    [DEF_DISPLAY] = STR(DEF_DISPLAY),
};

// Object type names
const char objectTypeNames[TYPE_COUNT][20] = {
    [TYPE_PLAYER] = STR(TYPE_PLAYER),
    [TYPE_ENEMY] = STR(TYPE_ENEMY),
    [TYPE_ITEM] = STR(TYPE_ITEM),
    [TYPE_TEXT] = STR(TYPE_TEXT),
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
void Game_Init();
void Joy_Handler(u16 joy, u16 changed, u16 state);
void UI_DrawCursor(const char *symbol1, const char *symbol2);
u16 UI_DrawEntityData(const EntityData *entity);
void UI_DrawActorData(const ActorData *body);
void UI_DrawItemData(const ItemData *item);
void UI_DrawData(const EntityData *object);
void Sprite_Init(EntityData *entityData, Sprite **sprite, const SpriteDefinition *sprDef);


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
void Game_Init()
{
    SPR_init();
    
    // Initialize enemies
    for (u16 i = 0; i < ENEMY_COUNT; i++)
    {
        enemies[i].actor = *enemiesData[i];
        Sprite_Init((EntityData *)&enemies[i], &enemies[i].sprite, spriteDefs[enemiesData[i]->sprDefInd]);
        objectsList[i] = (EntityData *)&enemies[i];
    }

    kprintf("0");

    // Initialize items
    for (u16 i = 0; i < ITEM_COUNT; i++)
    {
        items[i].item = *itemsData[i];
        Sprite_Init((EntityData *)&items[i], &items[i].sprite, spriteDefs[itemsData[i]->sprDefInd]);
        objectsList[ENEMY_COUNT + i] = (EntityData *)&items[i];
    }
//    kprintf("1");
    // Initialize players
    for (u16 i = 0; i < PLAYER_COUNT; i++)
    {
        players[i].actor = *playersData[i];
//        memcpy(&players[i].actor, &playersData[i], sizeof(ActorData));
        Sprite_Init((EntityData *)&players[i].entity, &players[i].sprite,  spriteDefs[playersData[i]->sprDefInd]);
        objectsList[ENEMY_COUNT + ITEM_COUNT + i] = (EntityData *)&players[i];
    }
    kprintf("2");
    
    // Initialize text
    for (u16 i = 0; i < TEXT_COUNT; i++)
    {
        texts[i] = *textData[i];
        objectsList[ENEMY_COUNT + ITEM_COUNT + PLAYER_COUNT + i] = (EntityData *)&texts[i];
    }
    
    kprintf("text type %d", texts[0].type);
    
    // Set Sonic animation to 1
    SPR_setAnim(players[0].sprite, 1);

    // Set up input handler
    JOY_setEventHandler(Joy_Handler);

    // Setup UI
    VDP_setTextPalette(PAL2);
    VDP_drawText("USE DPAD TO SWITCH CHARACTER", 6, 0);
    UI_DrawData((const EntityData *)objectsList[selectedObjectIndex]);
    UI_DrawCursor(">>", "<<");

}


void Sprite_Init(EntityData *entityData, Sprite **sprite, const SpriteDefinition *sprDef)
{
    PAL_setPalette(entityData->pal, sprDef->palette->data, DMA);
    *sprite = SPR_addSprite(sprDef,
                                F32_toInt(entityData->x),
                                F32_toInt(entityData->y),
                                TILE_ATTR(entityData->pal, entityData->priority, entityData->flipV, entityData->flipH));
    if (!entityData->visible)
        SPR_setVisibility(*sprite, HIDDEN);
}

// Handle joypad input events
void Joy_Handler(u16 joy, u16 changed, u16 state)
{
    if (changed & state)
    {    // Clear current cursor
        UI_DrawCursor("  ", "  ");
    }
    
    // Handle direction input
    if (changed & state & BUTTON_LEFT)
        selectedObjectIndex = (selectedObjectIndex == OBJECTS_COUNT - 1) ? 0 : selectedObjectIndex + 1;
    else if (changed & state & BUTTON_RIGHT)
        selectedObjectIndex = (selectedObjectIndex == 0) ? OBJECTS_COUNT - 1 : (u16) (selectedObjectIndex - 1);
    
    if (changed & state)
    {    // Update displayed data
        UI_DrawData((const EntityData *) objectsList[selectedObjectIndex]);
        
        // Draw new cursor
        UI_DrawCursor(">>", "<<");
    }
}

// Draw an array of strings on screen
void UI_DrawStringsArray(const char *text, u16 fromY, u16 length)
{
    VDP_setTextPalette(PAL1);
    for (u16 i = 0; i < length; i++)
    {
        VDP_drawTextBG(BG_A, text + (fromY+i) * TEXT_BUFFER, 0, 2 + fromY + i);
    }
}

// Draw base object statistics
u16 UI_DrawBaseObjectData(const BaseObjectData *object)
{
    char text[STAT_LINES][TEXT_BUFFER];
    u16 y = 0;
    
    sprintf(text[y++], " ______ TMX DATA ______");
    sprintf(text[y++], " Name:     %-11s", object->name);
    sprintf(text[y++], " Id:       %-15u", object->id);
    sprintf(text[y++], " Pos:      X:%03ld, Y:%03ld", F32_toInt(object->x), F32_toInt(object->y));
    sprintf(text[y++], " Visible:  %-5s", object->visible ? "TRUE" : "FALSE");
    
    UI_DrawStringsArray((const char *)text, 0, y);
    return y;
}

// Draw base object statistics
u16 UI_DrawEntityData(const EntityData *entity)
{
    char text[STAT_LINES][TEXT_BUFFER];
    u16 start = UI_DrawBaseObjectData((const BaseObjectData *)entity);
    u16 y = start;
    
    sprintf(text[y++], " Type:     %-15s", objectTypeNames[entity->type]);
    sprintf(text[y++], " Enabled:  %-5s", entity->enabled ? "TRUE" : "FALSE");
    sprintf(text[y++], " SprDefInd:%-15s", sprDefNames[entity->sprDefInd]);
    sprintf(text[y++], " PalInd:   %d", entity->pal);
    sprintf(text[y++], " Prio:     %-5s", entity->priority ? "TRUE" : "FALSE");
    sprintf(text[y++], " FlipH:    %-5s", entity->flipH ? "TRUE" : "FALSE");
    sprintf(text[y++], " FlipV:    %-5s", entity->flipV ? "TRUE" : "FALSE");

    UI_DrawStringsArray((const char *)text, start, y-start);
    return y;
}

// Draw actor-specific statistics
void UI_DrawActorData(const ActorData *body)
{
    char text[STAT_LINES][TEXT_BUFFER];
    u16 start = UI_DrawEntityData((const EntityData *)body);
    u16 y = start;
    
    sprintf(text[y++], " Target:   %s(ptr:%p) %-10s",
            (body->target != NULL) ? ((ActorData *)body->target)->name : "NONE",
            body->target, "");
    sprintf(text[y++], " Speed:    %02ld.%d %-25s",
            F32_toInt(body->speed),
            (u16)(mulu(F32_frac(body->speed), 100) >> FIX32_FRAC_BITS), "");
    sprintf(text[y++], " HP:       %-29d", body->hp);
    sprintf(text[y++], " Phrase:   %-70s", body->phrase);
    y++;
    sprintf(text[y++], " ______________________________________");

    UI_DrawStringsArray((const char *)text, start, y-start);
}

// Draw item-specific statistics
void UI_DrawItemData(const ItemData *item)
{
    char text[STAT_LINES][TEXT_BUFFER];
    u16 start = UI_DrawEntityData((const EntityData *)item);
    u16 y = start;
    
    sprintf(text[y++], " HP:       %-29d", item->hp);
    sprintf(text[y++], " ______________________________________");
    sprintf(text[y++], "%-40s", "");
    sprintf(text[y++], "%-40s", "");
    sprintf(text[y++], "%-40s", "");
    sprintf(text[y++], "%-40s", "");
    
    UI_DrawStringsArray((const char *)text, start, y-start);
}

// Draw item-specific statistics
void UI_DrawTextData(const TextData *text)
{
    char str[STAT_LINES][TEXT_BUFFER];
    u16 start = UI_DrawBaseObjectData((const BaseObjectData *)text);
    u16 y = start;

    sprintf(str[y++], " Type:     %-15s", objectTypeNames[text->type]);
//    sprintf(str[y++], " text:   %-70s", text->text);
    sprintf(str[y++], " ______________________________________");
    sprintf(str[y++], "%-40s", "");
    sprintf(str[y++], "%-40s", "");
    sprintf(str[y++], "%-40s", "");
    sprintf(str[y++], "%-40s", "");
    sprintf(str[y++], "%-40s", "");
    sprintf(str[y++], "%-40s", "");
    sprintf(str[y++], "%-40s", "");
    sprintf(str[y++], "%-40s", "");
    sprintf(str[y++], "%-40s", "");
    sprintf(str[y++], "%-40s", "");
    sprintf(str[y++], "%-40s", "");

    UI_DrawStringsArray((const char *)str, start, y-start);
}

// Draw appropriate data based on an object type
void UI_DrawData(const EntityData *object)
{
//    kprintf("type: %d", object->type);
    
    switch (object->type)
    {
        case TYPE_PLAYER:
        case TYPE_ENEMY:
            UI_DrawActorData((const ActorData *)object);
            break;
        
        case TYPE_ITEM:
            UI_DrawItemData((const ItemData *)object);
            break;
        
        case TYPE_TEXT:
            UI_DrawTextData((const TextData *)object);
            break;
    }
}

// Draw selection cursor around a selected object
void UI_DrawCursor(const char *symbol1, const char *symbol2)
{
    const EntityData *object = objectsList[selectedObjectIndex];
    const SpriteDefinition *sprDef = spriteDefs[object->sprDefInd];
    
    s16 x1 = (F32_toInt(object->x) >> 3) - (sprDef->w >> 4);
    s16 x2 = (F32_toInt(object->x) >> 3) + (sprDef->w >> 3) + (sprDef->w >> 4) - 1;
    s16 y = (F32_toInt(object->y) >> 3) + (sprDef->h >> 4);
    
    VDP_setTextPalette(PAL3);
    VDP_drawTextBG(BG_B, symbol1, x1, y);
    VDP_drawTextBG(BG_B, symbol2, x2, y);
}