#ifndef GUARD_CONSTANTS_FIELD_EFFECTS_H 
#define GUARD_CONSTANTS_FIELD_EFFECTS_H 


enum FieldEffect
{
    FLDEFF_NONE,
    FLDEFF_EXCLAMATION_MARK_ICON,
    FLDEFF_USE_CUT_ON_GRASS,
    FLDEFF_USE_CUT_ON_TREE,
    FLDEFF_SHADOW,
    FLDEFF_TALL_GRASS,
    FLDEFF_RIPPLE,
    FLDEFF_FIELD_MOVE_SHOW_MON,
    FLDEFF_ASH,
    FLDEFF_SURF_BLOB,
    FLDEFF_USE_SURF,
    FLDEFF_DUST,
    FLDEFF_USE_SECRET_POWER_CAVE,
    FLDEFF_JUMP_TALL_GRASS,
    FLDEFF_SAND_FOOTPRINTS,
    FLDEFF_JUMP_BIG_SPLASH,
    FLDEFF_SPLASH,
    FLDEFF_JUMP_SMALL_SPLASH,
    FLDEFF_LONG_GRASS,
    FLDEFF_JUMP_LONG_GRASS,
    FLDEFF_UNUSED_GRASS,
    FLDEFF_UNUSED_GRASS_2,
    FLDEFF_UNUSED_SAND,
    FLDEFF_UNUSED_WATER_SURFACING,
    FLDEFF_BERRY_TREE_GROWTH_SPARKLE,
    FLDEFF_DEEP_SAND_FOOTPRINTS,
    FLDEFF_POKECENTER_HEAL,
    FLDEFF_USE_SECRET_POWER_TREE,
    FLDEFF_USE_SECRET_POWER_SHRUB,
    FLDEFF_TREE_DISGUISE,
    FLDEFF_MOUNTAIN_DISGUISE,
    FLDEFF_NPCFLY_OUT,
    FLDEFF_FLY_OUT,
    FLDEFF_FLY_IN,
    FLDEFF_QUESTION_MARK_ICON_AND_EMOTE,
    // FLDEFF_EMOTE, // shares a slot with FLDEFF_QUESTION_MARK_ICON
    FLDEFF_FEET_IN_FLOWING_WATER,
    FLDEFF_BIKE_TIRE_TRACKS,
    FLDEFF_SAND_DISGUISE,
    FLDEFF_USE_ROCK_SMASH,
    FLDEFF_USE_DIG,
    FLDEFF_SAND_PILE,
    FLDEFF_USE_STRENGTH,
    FLDEFF_SHORT_GRASS,
    FLDEFF_HOT_SPRINGS_WATER,
    FLDEFF_USE_WATERFALL,
    FLDEFF_USE_DIVE,
    FLDEFF_POKEBALL,
    FLDEFF_X_ICON,
    FLDEFF_NOP_47,
    FLDEFF_NOP_48,
    FLDEFF_POP_OUT_OF_ASH,
    FLDEFF_LAVARIDGE_GYM_WARP,
    FLDEFF_SWEET_SCENT,
    FLDEFF_SAND_PILLAR,
    FLDEFF_BUBBLES,
    FLDEFF_SPARKLE,
    FLDEFF_SECRET_POWER_CAVE,
    FLDEFF_SECRET_POWER_TREE,
    FLDEFF_SECRET_POWER_SHRUB,
    FLDEFF_CUT_GRASS,
    FLDEFF_FIELD_MOVE_SHOW_MON_INIT,
    FLDEFF_USE_FLY_ANCIENT_TOMB,
    FLDEFF_PCTURN_ON,
    FLDEFF_HALL_OF_FAME_RECORD,
    FLDEFF_USE_TELEPORT,
    FLDEFF_SMILEY_FACE_ICON,
    FLDEFF_USE_VS_SEEKER,
    FLDEFF_DOUBLE_EXCL_MARK_ICON,
    FLDEFF_MOVE_DEOXYS_ROCK,
    FLDEFF_DESTROY_DEOXYS_ROCK,
    FLDEFF_PHOTO_FLASH,
    FLDEFF_TRACKS_SLITHER,
    FLDEFF_TRACKS_SPOT,
    FLDEFF_TRACKS_BUG,
    FLDEFF_SNOW_FOOTPRINTS,
    FLDEFF_COUNT,
};

#define NOT_IN_PARTY_MASK (1 << 12)

#define FLDEFF_PAL_TAG_GENERAL_0          0x1004
#define FLDEFF_PAL_TAG_GENERAL_1          0x1005
#define FLDEFF_PAL_TAG_GENERAL_2          0x1008
#define FLDEFF_PAL_TAG_POKEBALL_TRAIL     0x1009
#define FLDEFF_PAL_TAG_ASH                0x100D
#define FLDEFF_PAL_TAG_SMALL_SPARKLE      0x100F

#endif // GUARD_CONSTANTS_FIELD_EFFECTS_H
