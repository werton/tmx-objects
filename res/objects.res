OBJECTS    playersData    "tmx_map/objects_map.tmx"    "ActorLayer"    "id:u16;name:string;x:f32;y:f32;visible:bool;type:u32;sprDefInd:u32;pal:u8;flipH:bool;flipV:bool;priority:bool;enabled:bool;phrase:string;speed:f32;hp:s16;target:object"   "sortby:x"   "ActorData"  "TiledPlayer"

OBJECTS    enemiesData    "tmx_map/objects_map.tmx"    "ActorLayer"    "id:u16;name:string;x:f32;y:f32;visible:bool;type:u32;sprDefInd:u32;pal:u8;flipH:bool;flipV:bool;priority:bool;enabled:bool;phrase:string;speed:f32;hp:s16;target:object"   "sortby:x"   "ActorData"  "TiledEnemy"

OBJECTS    itemsData    "tmx_map/objects_map.tmx"    "ItemLayer"    "id:u16;name:string;x:f32;y:f32;visible:bool;type:u32;sprDefInd:u32;pal:u8;flipH:bool;flipV:bool;priority:bool;enabled:bool;hp:s16"   "sortby:id"   "ItemData"  "TMX_Item"

OBJECTS    textData     "tmx_map/objects_map.tmx"    "DecorLayer"    "id:u16;name:string;x:f32;y:f32;visible:bool;type:u32;text:string"   "sortby:id"   "TextData"  "TMX_Text"