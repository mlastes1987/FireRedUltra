#ifndef GUARD_CONSTANTS_MAP_TYPES_H
#define GUARD_CONSTANTS_MAP_TYPES_H

enum MapType
{
    MAP_TYPE_NONE,
    MAP_TYPE_TOWN,
    MAP_TYPE_CITY,        // Not used by any map. RSE use this map type to distinguish Town and City. FRLG make no distinction
    MAP_TYPE_ROUTE,
    MAP_TYPE_UNDERGROUND,
    MAP_TYPE_UNDERWATER,  // Not used by any map.
    MAP_TYPE_OCEAN_ROUTE, // Not used by any map.
    MAP_TYPE_UNKNOWN,     // Not used by any map.
    MAP_TYPE_INDOOR,
    MAP_TYPE_SECRET_BASE, // Not used by any map.
};


//Battle backgrounds
enum MapBattleScene
{
    MAP_BATTLE_SCENE_NORMAL,
    MAP_BATTLE_SCENE_GYM,
    MAP_BATTLE_SCENE_INDOOR_1, // Pokemon Tower, Tanoby ruins
    MAP_BATTLE_SCENE_INDOOR_2, // Pokemon Mansion, Power Plant, Rocket Hideout/Warehouse
    MAP_BATTLE_SCENE_LORELEI,
    MAP_BATTLE_SCENE_BRUNO,
    MAP_BATTLE_SCENE_AGATHA,
    MAP_BATTLE_SCENE_LANCE,
    MAP_BATTLE_SCENE_LINK,
    MAP_BATTLE_SCENE_MAGMA,
    MAP_BATTLE_SCENE_AQUA,
    MAP_BATTLE_SCENE_FRONTIER,
    MAP_BATTLE_SCENE_LEADER,
    MAP_BATTLE_SCENE_CHAMPION,
    MAP_BATTLE_SCENE_GROUDON,
    MAP_BATTLE_SCENE_KYOGRE,
    MAP_BATTLE_SCENE_RAYQUAZA,
};

#endif  // GUARD_CONSTANTS_MAP_TYPES_H
