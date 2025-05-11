#include <genesis.h>
#include "sprites.h"


typedef struct
{
    char *name;         // Display name
    char *text;         // Display name
    
} TextData;

// Contains all data properties for game actors
typedef struct
{
    char *name;         // Display name
    f32 x;              // X position (fixed point)
    f32 y;              // Y position (fixed point)
    char *phrase;       // Dialogue text
    f32 speed;          // Movement speed
    u16 id;             // Unique ID
    s16 hp;             // Hit points
    u16 type;           // Actor type
    u8 pal;             // Palette index
    bool priority;      // Render priority
    bool flipH;         // Horizontal flip
    Object target;         // Horizontal flip
} ActorData;

// Base game object with sprite and data
typedef struct
{
    union
    {
        ActorData data; // Named access
        ActorData;      // Anonymous access
    };
    Sprite *sprite;     // Sprite reference
} GameObject;

// Player-specific object
typedef struct
{
    GameObject;         // Inherits GameObject
} Player;

// Enemy-specific object
typedef struct
{
    GameObject;         // Inherits GameObject
} Enemy;

#include "objects.h"

// Constants
#define ENEMY_COUNT (sizeof(enemiesData)/sizeof(enemiesData[0]))
#define STAT_LINES 20
#define TEXT_BUFFER 40
#define UI_PALETTE PAL1
#define TEXT_PALETTE PAL3

// Game state
static Player player;
static Enemy enemies[ENEMY_COUNT];
static u16 currentEnemyIndex = 0;

// Sprite definitions
static const SpriteDefinition *enemySprDefs[ENEMY_COUNT] = {
    &sprDefEnemy01,
    &sprDefEnemy02,
    &sprDefEnemy03
};

// Forward declarations
static void Game_Init();

static void Joy_Handler(u16 joy, u16 changed, u16 state);

static void GameObject_Init(GameObject *obj, const ActorData *data, const SpriteDefinition *sprDef);

static void UI_DrawStats(const ActorData *actor);

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
    GameObject_Init((GameObject *) &player, playersData[0], &sprDefSonic);
    SPR_setAnim(player.sprite, 1);
    
    // Initialize enemies
    for (u16 i = 0; i < ENEMY_COUNT; i++)
    {
        GameObject_Init((GameObject *) &enemies[i], enemiesData[i], enemySprDefs[i]);
    }
    
    // Set up input
    JOY_setEventHandler(Joy_Handler);
    
    // Setup UI
    VDP_setTextPalette(TEXT_PALETTE);
    VDP_drawText("Use DPAD to list char's stats", 5, 0);
    UI_DrawStats(enemiesData[currentEnemyIndex]);
}

// Initialize a game object
static void GameObject_Init(GameObject *obj, const ActorData *data, const SpriteDefinition *sprDef)
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
static void UI_DrawStats(const ActorData *actor)
{
    char text[STAT_LINES][TEXT_BUFFER];
    u16 y = 0;
    
    sprintf(text[y++], " TMX OBJECT IMPORTED DATA:");
    sprintf(text[y++], "   target: %p", actor);
    sprintf(text[y++], "");
    sprintf(text[y++], "   name(string): %-3s", actor->name);
    sprintf(text[y++], "   id: %d", actor->id);
    sprintf(text[y++], "   X:%03d, Y:%03d", F32_toInt(actor->x), F32_toInt(actor->y));
    sprintf(text[y++], "   palette ind: %d", actor->pal);
    sprintf(text[y++], "   priority: %-5s", actor->priority ? "TRUE" : "FALSE");
    sprintf(text[y++], "   flipH: %-5s", actor->flipH ? "TRUE" : "FALSE");
    sprintf(text[y++], "   target: %p", actor->target);
    sprintf(text[y++], "   type: %-5s", actor->priority ? "TRUE" : "FALSE");
    sprintf(text[y++], "   speed: %02d.%d", F32_toInt(actor->speed), (u16) (mulu(F32_frac(actor->speed),
                                                                                 100) >> FIX32_FRAC_BITS));
    sprintf(text[y++], "   hp: %d", actor->hp);
    sprintf(text[y++], "   phrase: %-69s", actor->phrase);
    
    VDP_setTextPalette(UI_PALETTE);
    for (u16 i = 0; i < STAT_LINES; i++)
    {
        VDP_drawTextBG(BG_B, text[i], 0, 2 + i);
    }
    
    VDP_setTextPalette(TEXT_PALETTE);
    UI_DrawMarker("^");
    
    VDP_drawTextBG(BG_B, textData[0]->text, 0, 2 + y + 5);
    
}

// Draw selection marker
static void UI_DrawMarker(const char *symbol)
{
    const ActorData *actor = enemiesData[currentEnemyIndex];
    const SpriteDefinition *sprDef = enemySprDefs[currentEnemyIndex];
    
    s16 x = (F32_toInt(actor->x) >> 3) + (sprDef->w >> 4);
    s16 y = (F32_toInt(actor->y) >> 3) + (sprDef->h >> 3) + 1;
    
    VDP_drawTextBG(BG_B, symbol, x, y);
}