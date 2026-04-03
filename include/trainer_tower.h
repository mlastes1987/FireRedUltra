#ifndef GUARD_TRAINER_TOWER_H
#define GUARD_TRAINER_TOWER_H


extern u32 *gTrainerTowerVBlankCounter;

enum TrainerClassID GetTrainerTowerOpponentClass(void);
enum TrainerPicID GetTrainerTowerTrainerFrontSpriteId(void);
void FreeTrainerTowerBattleStruct(void);
void GetTrainerTowerOpponentLoseText(u8 *dest, u8 opponentIdx);
void GetTrainerTowerOpponentName(u8 *text);
void GetTrainerTowerOpponentWinText(u8 *dest, u8 opponentIdx);
void InitTrainerTowerBattleStruct(void);
void PrintTrainerTowerRecords(void);
void ResetTrainerTowerResults(void);

#endif //GUARD_TRAINER_TOWER_H
