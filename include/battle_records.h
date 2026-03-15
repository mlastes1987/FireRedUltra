#ifndef GUARD_BATTLE_RECORDS_H
#define GUARD_BATTLE_RECORDS_H

extern u8 gRecordsWindowId;

void ClearPlayerLinkBattleRecords(void);
void UpdatePlayerLinkBattleRecords(enum BattlerId battler);
void RemoveRecordsWindow(void);

#endif // GUARD_BATTLE_RECORDS_H
