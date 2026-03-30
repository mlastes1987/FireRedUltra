#include "global.h"
#include "constants/trainers.h"

static enum TrainerPicID GetHoennTrainerPic(enum Gender gender)
{
    return gender == MALE ? TRAINER_PIC_RS_BRENDAN : TRAINER_PIC_RS_MAY;
}

static enum TrainerPicID GetKantoTrainerPic(enum Gender gender)
{
    return gender == MALE ? TRAINER_PIC_RED : TRAINER_PIC_LEAF;
}

enum TrainerPicID GetPlayerTrainerPic(enum Gender gender, enum GameVersion version)
{
    switch (version)
    {
        case VERSION_SAPPHIRE:
        case VERSION_RUBY:
        case VERSION_EMERALD:
            return GetHoennTrainerPic(gender);
        case VERSION_LEAF_GREEN:
        case VERSION_FIRE_RED:
        default:
            return GetKantoTrainerPic(gender);
    }
}

STATIC_ASSERT(TRAINERS_COUNT <= MAX_TRAINERS_COUNT, tooManyTrainers);
