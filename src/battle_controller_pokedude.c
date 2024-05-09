#include "global.h"
#include "gflib.h"
#include "task.h"
#include "party_menu.h"
#include "pokeball.h"
#include "data.h"
#include "util.h"
#include "m4a.h"
#include "link.h"
#include "event_data.h"
#include "item_menu.h"
#include "strings.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "reshow_battle_screen.h"
#include "teachy_tv.h"
#include "constants/songs.h"
#include "constants/moves.h"
#include "constants/pokemon.h"

struct PokedudeTextScriptHeader
{
    u8 btlcmd;
    u8 side;
    u16 stringid;
    void (*callback)(u32 battler);
};

struct PokedudeBattlePartyInfo
{
    u8 side;
    u8 level;
    u16 species;
    u16 moves[MAX_MON_MOVES];
    u8 nature;
    u8 gender;
};

static void PokedudeHandleLoadMonSprite(u32 battler);
static void PokedudeHandleSwitchInAnim(u32 battler);
static void PokedudeHandleReturnMonToBall(u32 battler);
static void PokedudeHandleDrawTrainerPic(u32 battler);
static void PokedudeHandleTrainerSlide(u32 battler);
static void PokedudeHandleTrainerSlideBack(u32 battler);
static void PokedudeHandleFaintAnimation(u32 battler);
static void PokedudeHandlePaletteFade(u32 battler);
static void PokedudeHandleSuccessBallThrowAnim(u32 battler);
static void PokedudeHandleBallThrowAnim(u32 battler);
static void PokedudeHandlePause(u32 battler);
static void PokedudeHandleMoveAnimation(u32 battler);
static void PokedudeHandlePrintString(u32 battler);
static void PokedudeHandlePrintSelectionString(u32 battler);
static void PokedudeHandleChooseAction(u32 battler);
static void PokedudeHandleUnknownYesNoBox(u32 battler);
static void PokedudeHandleChooseMove(u32 battler);
static void PokedudeHandleChooseItem(u32 battler);
static void PokedudeHandleChoosePokemon(u32 battler);
static void PokedudeHandleCmd23(u32 battler);
static void PokedudeHandleHealthBarUpdate(u32 battler);
static void PokedudeHandleExpUpdate(u32 battler);
static void PokedudeHandleStatusIconUpdate(u32 battler);
static void PokedudeHandleStatusAnimation(u32 battler);
static void PokedudeHandleStatusXor(u32 battler);
static void PokedudeHandleDataTransfer(u32 battler);
static void PokedudeHandleDMA3Transfer(u32 battler);
static void PokedudeHandlePlayBGM(u32 battler);
static void PokedudeHandleCmd32(u32 battler);
static void PokedudeHandleTwoReturnValues(u32 battler);
static void PokedudeHandleChosenMonReturnValue(u32 battler);
static void PokedudeHandleOneReturnValue(u32 battler);
static void PokedudeHandleOneReturnValue_Duplicate(u32 battler);
static void PokedudeHandleCmd37(u32 battler);
static void PokedudeHandleCmd38(u32 battler);
static void PokedudeHandleCmd39(u32 battler);
static void PokedudeHandleCmd40(u32 battler);
static void PokedudeHandleHitAnimation(u32 battler);
static void PokedudeHandleCmd42(u32 battler);
static void PokedudeHandlePlaySE(u32 battler);
static void PokedudeHandlePlayFanfare(u32 battler);
static void PokedudeHandleFaintingCry(u32 battler);
static void PokedudeHandleIntroSlide(u32 battler);
static void PokedudeHandleIntroTrainerBallThrow(u32 battler);
static void PokedudeHandleDrawPartyStatusSummary(u32 battler);
static void PokedudeHandleHidePartyStatusSummary(u32 battler);
static void PokedudeHandleEndBounceEffect(u32 battler);
static void PokedudeHandleSpriteInvisibility(u32 battler);
static void PokedudeHandleBattleAnimation(u32 battler);
static void PokedudeHandleLinkStandbyMsg(u32 battler);
static void PokedudeHandleResetActionMoveSelection(u32 battler);
static void PokedudeHandleCmd55(u32 battler);
static void PokedudeCmdEnd(u32 battler);

static void PokedudeAction_PrintVoiceoverMessage(u32 battler);
static void PokedudeAction_PrintMessageWithHealthboxPals(u32 battler);
static void PokedudeBufferExecCompleted(u32 battler);
static void PokedudeSimulateInputChooseAction(u32 battler);
static void PokedudeBufferRunCommand(u32 battler);
static bool8 HandlePokedudeVoiceoverEtc(u32 battler);
static void PokedudeSimulateInputChooseMove(u32 battler);
static void WaitForMonSelection(u32 battler);
static void CompleteWhenChoseItem(u32 battler);
static void Intro_WaitForShinyAnimAndHealthbox(u32 battler);
static void Task_LaunchLvlUpAnim(u8 taskId);
static void DestroyExpTaskAndCompleteOnInactiveTextPrinter(u8 taskId);
static void CompleteOnInactiveTextPrinter2(u32 battler);
static void Task_PrepareToGiveExpWithExpBar(u8 taskId);
static void Task_GiveExpWithExpBar(u8 taskId);
static void Task_UpdateLvlInHealthbox(u8 taskId);
// static void StartSendOutAnim(u8 battlerId);
static void PokedudeDoMoveAnimation(u32 battler);
static void Task_StartSendOutAnim(u8 taskId);
static const u8 *GetPokedudeText(void);

static void (*const sPokedudeBufferCommands[CONTROLLER_CMDS_COUNT])(u32 battler) =
{
    [CONTROLLER_GETMONDATA]               = BtlController_HandleGetMonData,
    [CONTROLLER_GETRAWMONDATA]            = BtlController_HandleGetRawMonData,
    [CONTROLLER_SETMONDATA]               = BtlController_HandleSetMonData,
    [CONTROLLER_SETRAWMONDATA]            = BtlController_HandleSetRawMonData,
    [CONTROLLER_LOADMONSPRITE]            = PokedudeHandleLoadMonSprite,
    [CONTROLLER_SWITCHINANIM]             = PokedudeHandleSwitchInAnim,
    [CONTROLLER_RETURNMONTOBALL]          = PokedudeHandleReturnMonToBall,
    [CONTROLLER_DRAWTRAINERPIC]           = PokedudeHandleDrawTrainerPic,
    [CONTROLLER_TRAINERSLIDE]             = PokedudeHandleTrainerSlide,
    [CONTROLLER_TRAINERSLIDEBACK]         = PokedudeHandleTrainerSlideBack,
    [CONTROLLER_FAINTANIMATION]           = PokedudeHandleFaintAnimation,
    [CONTROLLER_PALETTEFADE]              = PokedudeHandlePaletteFade,
    [CONTROLLER_SUCCESSBALLTHROWANIM]     = PokedudeHandleSuccessBallThrowAnim,
    [CONTROLLER_BALLTHROWANIM]            = PokedudeHandleBallThrowAnim,
    [CONTROLLER_PAUSE]                    = PokedudeHandlePause,
    [CONTROLLER_MOVEANIMATION]            = PokedudeHandleMoveAnimation,
    [CONTROLLER_PRINTSTRING]              = PokedudeHandlePrintString,
    [CONTROLLER_PRINTSTRINGPLAYERONLY]    = PokedudeHandlePrintSelectionString,
    [CONTROLLER_CHOOSEACTION]             = PokedudeHandleChooseAction,
    [CONTROLLER_UNKNOWNYESNOBOX]          = PokedudeHandleUnknownYesNoBox,
    [CONTROLLER_CHOOSEMOVE]               = PokedudeHandleChooseMove,
    [CONTROLLER_OPENBAG]                  = PokedudeHandleChooseItem,
    [CONTROLLER_CHOOSEPOKEMON]            = PokedudeHandleChoosePokemon,
    [CONTROLLER_23]                       = PokedudeHandleCmd23,
    [CONTROLLER_HEALTHBARUPDATE]          = PokedudeHandleHealthBarUpdate,
    [CONTROLLER_EXPUPDATE]                = PokedudeHandleExpUpdate,
    [CONTROLLER_STATUSICONUPDATE]         = PokedudeHandleStatusIconUpdate,
    [CONTROLLER_STATUSANIMATION]          = PokedudeHandleStatusAnimation,
    [CONTROLLER_STATUSXOR]                = PokedudeHandleStatusXor,
    [CONTROLLER_DATATRANSFER]             = PokedudeHandleDataTransfer,
    [CONTROLLER_DMA3TRANSFER]             = PokedudeHandleDMA3Transfer,
    [CONTROLLER_PLAYBGM]                  = PokedudeHandlePlayBGM,
    [CONTROLLER_32]                       = PokedudeHandleCmd32,
    [CONTROLLER_TWORETURNVALUES]          = PokedudeHandleTwoReturnValues,
    [CONTROLLER_CHOSENMONRETURNVALUE]     = PokedudeHandleChosenMonReturnValue,
    [CONTROLLER_ONERETURNVALUE]           = PokedudeHandleOneReturnValue,
    [CONTROLLER_ONERETURNVALUE_DUPLICATE] = PokedudeHandleOneReturnValue_Duplicate,
    [CONTROLLER_CLEARUNKVAR]              = PokedudeHandleCmd37,
    [CONTROLLER_SETUNKVAR]                = PokedudeHandleCmd38,
    [CONTROLLER_CLEARUNKFLAG]             = PokedudeHandleCmd39,
    [CONTROLLER_TOGGLEUNKFLAG]            = PokedudeHandleCmd40,
    [CONTROLLER_HITANIMATION]             = PokedudeHandleHitAnimation,
    [CONTROLLER_CANTSWITCH]               = PokedudeHandleCmd42,
    [CONTROLLER_PLAYSE]                   = PokedudeHandlePlaySE,
    [CONTROLLER_PLAYFANFAREORBGM]         = PokedudeHandlePlayFanfare,
    [CONTROLLER_FAINTINGCRY]              = PokedudeHandleFaintingCry,
    [CONTROLLER_INTROSLIDE]               = PokedudeHandleIntroSlide,
    [CONTROLLER_INTROTRAINERBALLTHROW]    = PokedudeHandleIntroTrainerBallThrow,
    [CONTROLLER_DRAWPARTYSTATUSSUMMARY]   = PokedudeHandleDrawPartyStatusSummary,
    [CONTROLLER_HIDEPARTYSTATUSSUMMARY]   = PokedudeHandleHidePartyStatusSummary,
    [CONTROLLER_ENDBOUNCE]                = PokedudeHandleEndBounceEffect,
    [CONTROLLER_SPRITEINVISIBILITY]       = PokedudeHandleSpriteInvisibility,
    [CONTROLLER_BATTLEANIMATION]          = PokedudeHandleBattleAnimation,
    [CONTROLLER_LINKSTANDBYMSG]           = PokedudeHandleLinkStandbyMsg,
    [CONTROLLER_RESETACTIONMOVESELECTION] = PokedudeHandleResetActionMoveSelection,
    [CONTROLLER_ENDLINKBATTLE]            = PokedudeHandleCmd55,
    [CONTROLLER_TERMINATOR_NOP]           = PokedudeCmdEnd,
};

// unknown unused data
static const u8 sUnused[] = { 0x48, 0x48, 0x20, 0x5a, 0x50, 0x50, 0x50, 0x58 };

#define pdHealthboxPal1  simulatedInputState[0]
#define pdHealthboxPal2  simulatedInputState[1]
#define pdScriptNum      simulatedInputState[2]
#define pdMessageNo      simulatedInputState[3]

static void PokedudeDummy(u32 battler)
{
}

void SetControllerToPokedude(u32 battler)
{
    gBattlerControllerEndFuncs[battler] = PokedudeBufferExecCompleted;
    gBattlerControllerFuncs[battler] = PokedudeBufferRunCommand;
    *(&gBattleStruct->pdScriptNum) = gSpecialVar_0x8004;
    gBattleStruct->pdMessageNo = 0;
}

static void PokedudeBufferRunCommand(u32 battler)
{
    if (gBattleControllerExecFlags & gBitTable[battler])
    {
        if (gBattleResources->bufferA[battler][0] < NELEMS(sPokedudeBufferCommands))
        {
            if (!HandlePokedudeVoiceoverEtc(battler))
                sPokedudeBufferCommands[gBattleResources->bufferA[battler][0]](battler);
        }
        else
        {
            PokedudeBufferExecCompleted(battler);
        }
    }
}

static void HandleInputChooseAction(u32 battler)
{
    PokedudeSimulateInputChooseAction(battler);
}

static void CompleteOnBattlerSpriteCallbackDummy(u32 battler)
{
    if (gSprites[gBattlerSpriteIds[battler]].callback == SpriteCallbackDummy)
        PokedudeBufferExecCompleted(battler);
}

static void CompleteOnBattlerSpritePosX_0(u32 battler)
{
    if (gSprites[gBattlerSpriteIds[battler]].animEnded == TRUE
        && gSprites[gBattlerSpriteIds[battler]].x2 == 0)
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim)
        {
            TryShinyAnimation(battler, &gEnemyParty[gBattlerPartyIndexes[battler]]);
        }
        else if (gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim)
        {
            gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim = FALSE;
            gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim = FALSE;
            FreeSpriteTilesByTag(ANIM_TAG_GOLD_STARS);
            FreeSpritePaletteByTag(ANIM_TAG_GOLD_STARS);
            PokedudeBufferExecCompleted(battler);
        }
    }
}

static void CompleteOnInactiveTextPrinter(u32 battler)
{
    if (!IsTextPrinterActive(0))
        PokedudeBufferExecCompleted(battler);
}

static void Pokedude_SetBattleEndCallbacks(u32 battler)
{
    if (!gPaletteFade.active)
    {
        gMain.inBattle = FALSE;
        gMain.callback1 = gPreBattleCallback1;
        SetMainCallback2(gMain.savedCallback);
    }
}

static void SwitchIn_HandleSoundAndEnd(u32 battler)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].specialAnimActive)
    {
        CreateTask(Task_PlayerController_RestoreBgmAfterCry, 10);
        HandleLowHpMusicChange(&gPlayerParty[gBattlerPartyIndexes[battler]], battler);
        PokedudeBufferExecCompleted(battler);
    }
}

static void SwitchIn_CleanShinyAnimShowSubstitute(u32 battler)
{
    if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy
        && gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim)
    {
        gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim = FALSE;
        FreeSpriteTilesByTag(ANIM_TAG_GOLD_STARS);
        FreeSpritePaletteByTag(ANIM_TAG_GOLD_STARS);
        if (gBattleSpritesDataPtr->battlerData[battler].behindSubstitute)
            InitAndLaunchSpecialAnimation(battler, battler, battler, B_ANIM_MON_TO_SUBSTITUTE);
        gBattlerControllerFuncs[battler] = SwitchIn_HandleSoundAndEnd;
    }
}

static void SwitchIn_TryShinyAnimShowHealthbox(u32 battler)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim
        && !gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive)
        TryShinyAnimation(battler, &gPlayerParty[gBattlerPartyIndexes[battler]]);
    if (gSprites[gBattleControllerData[battler]].callback == SpriteCallbackDummy
        && !(gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive))
    {
        DestroySprite(&gSprites[gBattleControllerData[battler]]);
        UpdateHealthboxAttribute(gHealthboxSpriteIds[battler],
                                 &gPlayerParty[gBattlerPartyIndexes[battler]],
                                 HEALTHBOX_ALL);
        StartHealthboxSlideIn(battler);
        SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);
        CopyBattleSpriteInvisibility(battler);
        gBattlerControllerFuncs[battler] = SwitchIn_CleanShinyAnimShowSubstitute;
    }
}

static void CompleteOnSpecialAnimDone(u32 battler)
{
    if (!gDoingBattleAnim)
        PokedudeBufferExecCompleted(battler);
}

static void Intro_DelayAndEnd(u32 battler)
{
    if (--gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay == 255)
    {
        gBattleSpritesDataPtr->healthBoxesData[battler].introEndDelay = 0;
        PokedudeBufferExecCompleted(battler);
    }
}

static void PokedudeHandleInputChooseMove(u32 battler)
{
    PokedudeSimulateInputChooseMove(battler);
}

static void OpenPartyMenuToChooseMon(u32 battler)
{
    if (!gPaletteFade.active)
    {
        gBattlerControllerFuncs[battler] = WaitForMonSelection;
        DestroyTask(gBattleControllerData[battler]);
        FreeAllWindowBuffers();
        Pokedude_OpenPartyMenuInBattle();
    }
}

static void WaitForMonSelection(u32 battler)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
    {
        if (gPartyMenuUseExitCallback == TRUE)
            BtlController_EmitChosenMonReturnValue(battler, 1, gSelectedMonPartyId, gBattlePartyCurrentOrder);
        else
            BtlController_EmitChosenMonReturnValue(battler, 1, 6, NULL);
        PokedudeBufferExecCompleted(battler);
    }
}

static void OpenBagAndChooseItem(u32 battler)
{
    u8 callbackId;

    if (!gPaletteFade.active)
    {
        gBattlerControllerFuncs[battler] = CompleteWhenChoseItem;
        ReshowBattleScreenDummy();
        FreeAllWindowBuffers();
        switch (gSpecialVar_0x8004)
        {
        case TTVSCR_STATUS:
        default:
            callbackId = ITEMMENULOCATION_TTVSCR_STATUS;
            break;
        case TTVSCR_CATCHING:
            callbackId = ITEMMENULOCATION_TTVSCR_CATCHING;
            break;
        }
        InitPokedudeBag(callbackId);
    }
}

static void CompleteWhenChoseItem(u32 battler)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
    {
        BtlController_EmitOneReturnValue(battler, 1, gSpecialVar_ItemId);
        PokedudeBufferExecCompleted(battler);
    }
}

static void Intro_TryShinyAnimShowHealthbox(u32 battler)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim
        && !gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive)
        TryShinyAnimation(battler, &gPlayerParty[gBattlerPartyIndexes[battler]]);
    if (!gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].triedShinyMonAnim
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].ballAnimActive)
        TryShinyAnimation(BATTLE_PARTNER(battler), &gPlayerParty[gBattlerPartyIndexes[BATTLE_PARTNER(battler)]]);
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].ballAnimActive && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].ballAnimActive)
    {
        if (IsDoubleBattle() && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
        {
            DestroySprite(&gSprites[gBattleControllerData[BATTLE_PARTNER(battler)]]);
            UpdateHealthboxAttribute(gHealthboxSpriteIds[BATTLE_PARTNER(battler)],
                                     &gPlayerParty[gBattlerPartyIndexes[BATTLE_PARTNER(battler)]],
                                     HEALTHBOX_ALL);
            StartHealthboxSlideIn(BATTLE_PARTNER(battler));
            SetHealthboxSpriteVisible(gHealthboxSpriteIds[BATTLE_PARTNER(battler)]);
        }
        DestroySprite(&gSprites[gBattleControllerData[battler]]);
        UpdateHealthboxAttribute(gHealthboxSpriteIds[battler],
                                 &gPlayerParty[gBattlerPartyIndexes[battler]],
                                 HEALTHBOX_ALL);
        StartHealthboxSlideIn(battler);
        SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);
        gBattleSpritesDataPtr->animationData->introAnimActive = FALSE;
        gBattlerControllerFuncs[battler] = Intro_WaitForShinyAnimAndHealthbox;
    }
}

static void Intro_WaitForShinyAnimAndHealthbox(u32 battler)
{
    bool32 r4 = FALSE;

    if (gSprites[gHealthboxSpriteIds[battler]].callback == SpriteCallbackDummy)
        r4 = TRUE;
    if (r4
        && gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim
        && gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].finishedShinyMonAnim)
    {
        gBattleSpritesDataPtr->healthBoxesData[battler].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[battler].finishedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(battler)].finishedShinyMonAnim = FALSE;
        FreeSpriteTilesByTag(ANIM_TAG_GOLD_STARS);
        FreeSpritePaletteByTag(ANIM_TAG_GOLD_STARS);
        CreateTask(Task_PlayerController_RestoreBgmAfterCry, 10);
        HandleLowHpMusicChange(&gPlayerParty[gBattlerPartyIndexes[battler]], battler);
        gBattlerControllerFuncs[battler] = Intro_DelayAndEnd;
    }
}

#define tExpTask_monId      data[0]
#define tExpTask_gainedExp  data[1]
#define tExpTask_battler    data[2]
#define tExpTask_frames     data[10]

static void Task_GiveExpToMon(u8 taskId)
{
    u32 monId = (u8)gTasks[taskId].tExpTask_monId;
    u8 battler = gTasks[taskId].tExpTask_battler;
    s16 gainedExp = gTasks[taskId].tExpTask_gainedExp;

    if (IsDoubleBattle() == TRUE || monId != gBattlerPartyIndexes[battler]) // Give exp without moving the expbar.
    {
        struct Pokemon *mon = &gPlayerParty[monId];
        u16 species = GetMonData(mon, MON_DATA_SPECIES);
        u8 level = GetMonData(mon, MON_DATA_LEVEL);
        u32 currExp = GetMonData(mon, MON_DATA_EXP);
        u32 nextLvlExp = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1];

        if (currExp + gainedExp >= nextLvlExp)
        {
            SetMonData(mon, MON_DATA_EXP, &nextLvlExp);
            CalculateMonStats(mon);
            gainedExp -= nextLvlExp - currExp;
            BtlController_EmitTwoReturnValues(battler, 1, RET_VALUE_LEVELED_UP, gainedExp);
            if (IsDoubleBattle() == TRUE
                && ((u16)monId == gBattlerPartyIndexes[battler] || (u16)monId == gBattlerPartyIndexes[battler ^ BIT_FLANK]))
                gTasks[taskId].func = Task_LaunchLvlUpAnim;
            else
                gTasks[taskId].func = DestroyExpTaskAndCompleteOnInactiveTextPrinter;
        }
        else
        {
            currExp += gainedExp;
            SetMonData(mon, MON_DATA_EXP, &currExp);
            gBattlerControllerFuncs[battler] = CompleteOnInactiveTextPrinter2;
            DestroyTask(taskId);
        }
    }
    else
    {
        gTasks[taskId].func = Task_PrepareToGiveExpWithExpBar;
    }
}

static void Task_PrepareToGiveExpWithExpBar(u8 taskId)
{
    u8 monIndex = gTasks[taskId].tExpTask_monId;
    s32 gainedExp = gTasks[taskId].tExpTask_gainedExp;
    u8 battlerId = gTasks[taskId].tExpTask_battler;
    struct Pokemon *mon = &gPlayerParty[monIndex];
    u8 level = GetMonData(mon, MON_DATA_LEVEL);
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 exp = GetMonData(mon, MON_DATA_EXP);
    u32 currLvlExp = gExperienceTables[gSpeciesInfo[species].growthRate][level];
    u32 expToNextLvl;

    exp -= currLvlExp;
    expToNextLvl = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1] - currLvlExp;
    SetBattleBarStruct(battlerId, gHealthboxSpriteIds[battlerId], expToNextLvl, exp, -gainedExp);
    PlaySE(SE_EXP);
    gTasks[taskId].func = Task_GiveExpWithExpBar;
}

static void Task_GiveExpWithExpBar(u8 taskId)
{
    if (gTasks[taskId].tExpTask_frames < 13)
    {
        ++gTasks[taskId].tExpTask_frames;
    }
    else
    {
        u8 monId = gTasks[taskId].tExpTask_monId;
        s16 gainedExp = gTasks[taskId].tExpTask_gainedExp;
        u8 battler = gTasks[taskId].tExpTask_battler;
        s16 newExpPoints;

        newExpPoints = MoveBattleBar(battler, gHealthboxSpriteIds[battler], EXP_BAR, 0);
        SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);
        if (newExpPoints == -1) // The bar has been filled with given exp points.
        {
            u8 level;
            s32 currExp;
            u16 species;
            s32 expOnNextLvl;

            m4aSongNumStop(SE_EXP);
            level = GetMonData(&gPlayerParty[monId], MON_DATA_LEVEL);
            currExp = GetMonData(&gPlayerParty[monId], MON_DATA_EXP);
            species = GetMonData(&gPlayerParty[monId], MON_DATA_SPECIES);
            expOnNextLvl = gExperienceTables[gSpeciesInfo[species].growthRate][level + 1];
            if (currExp + gainedExp >= expOnNextLvl)
            {
                SetMonData(&gPlayerParty[monId], MON_DATA_EXP, &expOnNextLvl);
                CalculateMonStats(&gPlayerParty[monId]);
                gainedExp -= expOnNextLvl - currExp;
                BtlController_EmitTwoReturnValues(battler, 1, RET_VALUE_LEVELED_UP, gainedExp);
                gTasks[taskId].func = Task_LaunchLvlUpAnim;
            }
            else
            {
                currExp += gainedExp;
                SetMonData(&gPlayerParty[monId], MON_DATA_EXP, &currExp);
                gBattlerControllerFuncs[battler] = CompleteOnInactiveTextPrinter2;
                DestroyTask(taskId);
            }
        }
    }
}

static void Task_LaunchLvlUpAnim(u8 taskId)
{
    u8 battlerId = gTasks[taskId].tExpTask_battler;
    u8 monIndex = gTasks[taskId].tExpTask_monId;

    if (IsDoubleBattle() == TRUE && monIndex == gBattlerPartyIndexes[battlerId ^ BIT_FLANK])
        battlerId ^= BIT_FLANK;
    InitAndLaunchSpecialAnimation(battlerId, battlerId, battlerId, B_ANIM_LVL_UP);
    gTasks[taskId].func = Task_UpdateLvlInHealthbox;
}

static void Task_UpdateLvlInHealthbox(u8 taskId)
{
    u8 battlerId = gTasks[taskId].tExpTask_battler;

    if (!gBattleSpritesDataPtr->healthBoxesData[battlerId].specialAnimActive)
    {
        u8 monIndex = gTasks[taskId].tExpTask_monId;

        GetMonData(&gPlayerParty[monIndex], MON_DATA_LEVEL);  // Unused return value.
        if (IsDoubleBattle() == TRUE && monIndex == gBattlerPartyIndexes[battlerId ^ BIT_FLANK])
            UpdateHealthboxAttribute(gHealthboxSpriteIds[battlerId ^ BIT_FLANK], &gPlayerParty[monIndex], HEALTHBOX_ALL);
        else
            UpdateHealthboxAttribute(gHealthboxSpriteIds[battlerId], &gPlayerParty[monIndex], HEALTHBOX_ALL);
        gTasks[taskId].func = DestroyExpTaskAndCompleteOnInactiveTextPrinter;
    }
}

static void DestroyExpTaskAndCompleteOnInactiveTextPrinter(u8 taskId)
{
    u8 monIndex = gTasks[taskId].tExpTask_monId;
    u8 battlerId;

    GetMonData(&gPlayerParty[monIndex], MON_DATA_LEVEL); // Unused return value
    battlerId = gTasks[taskId].tExpTask_battler;
    gBattlerControllerFuncs[battlerId] = CompleteOnInactiveTextPrinter2;
    DestroyTask(taskId);
}

static void FreeMonSpriteAfterFaintAnim(u32 battler)
{
    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
    {
        if (gSprites[gBattlerSpriteIds[battler]].y + gSprites[gBattlerSpriteIds[battler]].y2 > DISPLAY_HEIGHT)
        {
            FreeOamMatrix(gSprites[gBattlerSpriteIds[battler]].oam.matrixNum);
            DestroySprite(&gSprites[gBattlerSpriteIds[battler]]);
            SetHealthboxSpriteInvisible(gHealthboxSpriteIds[battler]);
            PokedudeBufferExecCompleted(battler);
        }
    }
    else
    {
        if (!gSprites[gBattlerSpriteIds[battler]].inUse)
        {
            SetHealthboxSpriteInvisible(gHealthboxSpriteIds[battler]);
            PokedudeBufferExecCompleted(battler);
        }
    }
}

static void CompleteOnHealthbarDone(u32 battler)
{
    s16 hpValue = MoveBattleBar(battler, gHealthboxSpriteIds[battler], HEALTH_BAR, 0);

    SetHealthboxSpriteVisible(gHealthboxSpriteIds[battler]);
    if (hpValue != -1)
    {
        UpdateHpTextInHealthbox(gHealthboxSpriteIds[battler], HP_CURRENT, hpValue, gBattleMons[battler].maxHP);
    }
    else
    {
        HandleLowHpMusicChange(&gPlayerParty[gBattlerPartyIndexes[battler]], battler);
        PokedudeBufferExecCompleted(battler);
    }
}

static void CompleteOnInactiveTextPrinter2(u32 battler)
{
    if (!IsTextPrinterActive(0))
        PokedudeBufferExecCompleted(battler);
}

static void DoHitAnimBlinkSpriteEffect(u32 battler)
{
    u8 spriteId = gBattlerSpriteIds[battler];

    if (gSprites[spriteId].data[1] == 32)
    {
        gSprites[spriteId].data[1] = 0;
        gSprites[spriteId].invisible = FALSE;
        gDoingBattleAnim = FALSE;
        PokedudeBufferExecCompleted(battler);
    }
    else
    {
        if ((gSprites[spriteId].data[1] % 4) == 0)
            gSprites[spriteId].invisible ^= 1;
        ++gSprites[spriteId].data[1];
    }
}

static void DoSwitchOutAnimation(u32 battler)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].specialAnimActive)
    {
        FreeSpriteOamMatrix(&gSprites[gBattlerSpriteIds[battler]]);
        DestroySprite(&gSprites[gBattlerSpriteIds[battler]]);
        SetHealthboxSpriteInvisible(gHealthboxSpriteIds[battler]);
        PokedudeBufferExecCompleted(battler);
    }
}

static void CompleteOnBattlerSpriteCallbackDummy2(u32 battler)
{
    if (gSprites[gBattlerSpriteIds[battler]].callback == SpriteCallbackDummy)
        PokedudeBufferExecCompleted(battler);
}

static void CompleteOnFinishedBattleAnimation(u32 battler)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].animFromTableActive)
        PokedudeBufferExecCompleted(battler);
}

static void CompleteOnFinishedStatusAnimation(u32 battler)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[battler].statusAnimActive)
        PokedudeBufferExecCompleted(battler);
}

static void PokedudeBufferExecCompleted(u32 battler)
{
    gBattlerControllerFuncs[battler] = PokedudeBufferRunCommand;
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        u8 playerId = GetMultiplayerId();

        PrepareBufferDataTransferLink(battler, 2, 4, &playerId);
        gBattleResources->bufferA[battler][0] = CONTROLLER_TERMINATOR_NOP;
    }
    else
    {
        gBattleControllerExecFlags &= ~gBitTable[battler];
    }
}

static void PokedudeHandleLoadMonSprite(u32 battler)
{
    u32 y;
    u16 species = GetMonData(&gEnemyParty[gBattlerPartyIndexes[battler]], MON_DATA_SPECIES);

    BattleLoadMonSpriteGfx(&gEnemyParty[gBattlerPartyIndexes[battler]], battler);
    y = GetBattlerSpriteDefault_Y(battler);
    SetMultiuseSpriteTemplateToPokemon(species, GetBattlerPosition(battler));
    gBattlerSpriteIds[battler] = CreateSprite(&gMultiuseSpriteTemplate,
                                                     GetBattlerSpriteCoord(battler, BATTLER_COORD_X_2),
                                                     y,
                                                     GetBattlerSpriteSubpriority(battler));
    gSprites[gBattlerSpriteIds[battler]].x2 = -DISPLAY_WIDTH;
    gSprites[gBattlerSpriteIds[battler]].data[0] = battler;
    gSprites[gBattlerSpriteIds[battler]].data[2] = species;
    gSprites[gBattlerSpriteIds[battler]].oam.paletteNum = battler;
    StartSpriteAnim(&gSprites[gBattlerSpriteIds[battler]], gBattleMonForms[battler]);
    SetBattlerShadowSpriteCallback(battler, GetMonData(&gEnemyParty[gBattlerPartyIndexes[battler]], MON_DATA_SPECIES));
    gBattlerControllerFuncs[battler] = CompleteOnBattlerSpritePosX_0;
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleSwitchInAnim(u32 battler)
{
    ClearTemporarySpeciesSpriteData(battler, gBattleResources->bufferA[battler][2]);
    gBattlerPartyIndexes[battler] = gBattleResources->bufferA[battler][1];
    BattleLoadMonSpriteGfx(&gPlayerParty[gBattlerPartyIndexes[battler]], battler);
    gActionSelectionCursor[battler] = 0;
    gMoveSelectionCursor[battler] = 0;
    StartSendOutAnim(battler, FALSE);
    gBattlerControllerFuncs[battler] = SwitchIn_TryShinyAnimShowHealthbox;
}

static void PokedudeHandleReturnMonToBall(u32 battler)
{
    if (gBattleResources->bufferA[battler][1] == 0)
    {
        InitAndLaunchSpecialAnimation(battler, battler, battler, B_ANIM_SWITCH_OUT_PLAYER_MON);
        gBattlerControllerFuncs[battler] = DoSwitchOutAnimation;
    }
    else
    {
        FreeSpriteOamMatrix(&gSprites[gBattlerSpriteIds[battler]]);
        DestroySprite(&gSprites[gBattlerSpriteIds[battler]]);
        SetHealthboxSpriteInvisible(gHealthboxSpriteIds[battler]);
        PokedudeBufferExecCompleted(battler);
    }
}

static void PokedudeHandleDrawTrainerPic(u32 battler)
{
    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
    {
        DecompressTrainerBackPalette(TRAINER_BACK_PIC_POKEDUDE, battler);
        SetMultiuseSpriteTemplateToTrainerBack(TRAINER_BACK_PIC_POKEDUDE, GetBattlerPosition(battler));
        gBattlerSpriteIds[battler] = CreateSprite(&gMultiuseSpriteTemplate,
                                                         80,
                                                         (8 - gTrainerBackPicCoords[TRAINER_BACK_PIC_POKEDUDE].size) * 4 + 80,
                                                         30);
        gSprites[gBattlerSpriteIds[battler]].x2 = DISPLAY_WIDTH;
        gSprites[gBattlerSpriteIds[battler]].data[0] = -2;
        gSprites[gBattlerSpriteIds[battler]].oam.paletteNum = battler;
        gSprites[gBattlerSpriteIds[battler]].callback = SpriteCB_TrainerSlideIn;
    }
    else
    {
        u32 tranerPicid = TRAINER_PIC_PROFESSOR_OAK;
        DecompressTrainerFrontPic(tranerPicid, battler);
        SetMultiuseSpriteTemplateToTrainerBack(tranerPicid, GetBattlerPosition(battler));
        gBattlerSpriteIds[battler] = CreateSprite(&gMultiuseSpriteTemplate,
                                                         176,
                                                         (8 - gTrainerFrontPicCoords[tranerPicid].size) * 4 + 40,
                                                         GetBattlerSpriteSubpriority(battler));
        gSprites[gBattlerSpriteIds[battler]].x2 = -DISPLAY_WIDTH;
        gSprites[gBattlerSpriteIds[battler]].data[0] = 2;
        gSprites[gBattlerSpriteIds[battler]].oam.paletteNum = IndexOfSpritePaletteTag(gTrainerFrontPicPaletteTable[tranerPicid].tag);
        gSprites[gBattlerSpriteIds[battler]].data[5] = gSprites[gBattlerSpriteIds[battler]].oam.tileNum;
        gSprites[gBattlerSpriteIds[battler]].oam.tileNum = GetSpriteTileStartByTag(gTrainerFrontPicTable[tranerPicid].tag);
        gSprites[gBattlerSpriteIds[battler]].oam.affineParam = tranerPicid;
        gSprites[gBattlerSpriteIds[battler]].callback = SpriteCB_TrainerSlideIn;
    }
    gBattlerControllerFuncs[battler] = CompleteOnBattlerSpriteCallbackDummy;
}

static void PokedudeHandleTrainerSlide(u32 battler)
{
    DecompressTrainerBackPalette(TRAINER_BACK_PIC_POKEDUDE, battler);
    SetMultiuseSpriteTemplateToTrainerBack(TRAINER_BACK_PIC_POKEDUDE, GetBattlerPosition(battler));
    gBattlerSpriteIds[battler] = CreateSprite(&gMultiuseSpriteTemplate,
                                                     80,
                                                     (8 - gTrainerBackPicCoords[TRAINER_BACK_PIC_POKEDUDE].size) * 4 + 80,
                                                     30);
    gSprites[gBattlerSpriteIds[battler]].oam.paletteNum = battler;
    gSprites[gBattlerSpriteIds[battler]].x2 = -96;
    gSprites[gBattlerSpriteIds[battler]].data[0] = 2;
    gSprites[gBattlerSpriteIds[battler]].callback = SpriteCB_TrainerSlideIn;
    gBattlerControllerFuncs[battler] = CompleteOnBattlerSpriteCallbackDummy2;
}

static void PokedudeHandleTrainerSlideBack(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleFaintAnimation(u32 battler)
{
    if (gBattleSpritesDataPtr->healthBoxesData[battler].animationState == 0)
    {
        if (gBattleSpritesDataPtr->battlerData[battler].behindSubstitute)
            InitAndLaunchSpecialAnimation(battler, battler, battler, B_ANIM_SUBSTITUTE_TO_MON);
        ++gBattleSpritesDataPtr->healthBoxesData[battler].animationState;
    }
    else
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].specialAnimActive)
        {
            gBattleSpritesDataPtr->healthBoxesData[battler].animationState = 0;
            if (GetBattlerSide(battler) == B_SIDE_PLAYER)
            {
                HandleLowHpMusicChange(&gPlayerParty[gBattlerPartyIndexes[battler]], battler);
                PlaySE12WithPanning(SE_FAINT, SOUND_PAN_ATTACKER);
                gSprites[gBattlerSpriteIds[battler]].data[1] = 0;
                gSprites[gBattlerSpriteIds[battler]].data[2] = 5;
                gSprites[gBattlerSpriteIds[battler]].callback = SpriteCB_FaintSlideAnim;
            }
            else
            {
                PlaySE12WithPanning(SE_FAINT, SOUND_PAN_TARGET);
                gSprites[gBattlerSpriteIds[battler]].callback = SpriteCB_FaintOpponentMon;
            }
            gBattlerControllerFuncs[battler] = FreeMonSpriteAfterFaintAnim;
        }
    }
}

static void PokedudeHandlePaletteFade(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleSuccessBallThrowAnim(u32 battler)
{
    gBattleSpritesDataPtr->animationData->ballThrowCaseId = BALL_3_SHAKES_SUCCESS;
    gDoingBattleAnim = TRUE;
    InitAndLaunchSpecialAnimation(battler, battler, GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), B_ANIM_BALL_THROW);
    gBattlerControllerFuncs[battler] = CompleteOnSpecialAnimDone;
}

static void PokedudeHandleBallThrowAnim(u32 battler)
{
    u8 ballThrowCaseId = gBattleResources->bufferA[battler][1];

    gBattleSpritesDataPtr->animationData->ballThrowCaseId = ballThrowCaseId;
    gDoingBattleAnim = TRUE;
    InitAndLaunchSpecialAnimation(battler, battler, GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), B_ANIM_BALL_THROW);
    gBattlerControllerFuncs[battler] = CompleteOnSpecialAnimDone;
}

static void PokedudeHandlePause(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleMoveAnimation(u32 battler)
{
    u16 move = gBattleResources->bufferA[battler][1] | (gBattleResources->bufferA[battler][2] << 8);

    gAnimMoveTurn = gBattleResources->bufferA[battler][3];
    gAnimMovePower = gBattleResources->bufferA[battler][4] | (gBattleResources->bufferA[battler][5] << 8);
    gAnimMoveDmg = gBattleResources->bufferA[battler][6] | (gBattleResources->bufferA[battler][7] << 8) | (gBattleResources->bufferA[battler][8] << 16) | (gBattleResources->bufferA[battler][9] << 24);
    gAnimFriendship = gBattleResources->bufferA[battler][10];
    gWeatherMoveAnim = gBattleResources->bufferA[battler][12] | (gBattleResources->bufferA[battler][13] << 8);
    gAnimDisableStructPtr = (struct DisableStruct *)&gBattleResources->bufferA[battler][16];
    gTransformedPersonalities[battler] = gAnimDisableStructPtr->transformedMonPersonality;
    if (IsMoveWithoutAnimation(move, gAnimMoveTurn)) // always returns FALSE
    {
        PokedudeBufferExecCompleted(battler);
    }
    else
    {
        gBattleSpritesDataPtr->healthBoxesData[battler].animationState = 0;
        gBattlerControllerFuncs[battler] = PokedudeDoMoveAnimation;
    }
}

static void PokedudeDoMoveAnimation(u32 battler)
{
    u16 move = gBattleResources->bufferA[battler][1] | (gBattleResources->bufferA[battler][2] << 8);

    switch (gBattleSpritesDataPtr->healthBoxesData[battler].animationState)
    {
    case 0:
        if (gBattleSpritesDataPtr->battlerData[battler].behindSubstitute)
            InitAndLaunchSpecialAnimation(battler, battler, battler, B_ANIM_SUBSTITUTE_TO_MON);
        gBattleSpritesDataPtr->healthBoxesData[battler].animationState = 1;
        break;
    case 1:
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].specialAnimActive)
        {
            SetBattlerSpriteAffineMode(ST_OAM_AFFINE_OFF);
            DoMoveAnim(move);
            gBattleSpritesDataPtr->healthBoxesData[battler].animationState = 2;
        }
        break;
    case 2:
        gAnimScriptCallback();
        if (!gAnimScriptActive)
        {
            SetBattlerSpriteAffineMode(ST_OAM_AFFINE_NORMAL);
            if (gBattleSpritesDataPtr->battlerData[battler].behindSubstitute)
                InitAndLaunchSpecialAnimation(battler, battler, battler, B_ANIM_MON_TO_SUBSTITUTE);
            gBattleSpritesDataPtr->healthBoxesData[battler].animationState = 3;
        }
        break;
    case 3:
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].specialAnimActive)
        {
            CopyAllBattleSpritesInvisibilities();
            TrySetBehindSubstituteSpriteBit(battler, gBattleResources->bufferA[battler][1] | (gBattleResources->bufferA[battler][2] << 8));
            gBattleSpritesDataPtr->healthBoxesData[battler].animationState = 0;
            PokedudeBufferExecCompleted(battler);
        }
        break;
    }
}

static void PokedudeHandlePrintString(u32 battler)
{
    u16 *stringId;

    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    stringId = (u16 *)(&gBattleResources->bufferA[battler][2]);
    BufferStringBattle(battler, *stringId);
    if (BattleStringShouldBeColored(*stringId))
        BattlePutTextOnWindow(gDisplayedStringBattle, (B_WIN_MSG | B_TEXT_FLAG_NPC_CONTEXT_FONT));
    else
        BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
    gBattlerControllerFuncs[battler] = CompleteOnInactiveTextPrinter;
}

static void PokedudeHandlePrintSelectionString(u32 battler)
{
    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
        PokedudeHandlePrintString(battler);
    else
        PokedudeBufferExecCompleted(battler);
}

static void HandleChooseActionAfterDma3(u32 battler)
{
    if (!IsDma3ManagerBusyWithBgCopy())
    {
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = 160;
        gBattlerControllerFuncs[battler] = HandleInputChooseAction;
    }
}

static void PokedudeHandleChooseAction(u32 battler)
{
    s32 i;

    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
    {
        gBattlerControllerFuncs[battler] = HandleChooseActionAfterDma3;
        BattlePutTextOnWindow(gText_EmptyString3, B_WIN_MSG);
        BattlePutTextOnWindow(gText_BattleMenu, B_WIN_ACTION_MENU);
        for (i = 0; i < MAX_MON_MOVES; ++i)
            ActionSelectionDestroyCursorAt((u8)i);
        ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
        BattleStringExpandPlaceholdersToDisplayedString(gText_WhatWillPkmnDo);
        BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_ACTION_PROMPT);
    }
    else
    {
        gBattlerControllerFuncs[battler] = HandleInputChooseAction;
    }
}

static void PokedudeHandleUnknownYesNoBox(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleChooseMoveAfterDma3(u32 battler)
{
    if (!IsDma3ManagerBusyWithBgCopy())
    {
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = 320;
        gBattlerControllerFuncs[battler] = PokedudeHandleInputChooseMove;
    }
}

static void PokedudeHandleChooseMove(u32 battler)
{
    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
    {
        InitMoveSelectionsVarsAndStrings(battler);
        gBattlerControllerFuncs[battler] = PokedudeHandleChooseMoveAfterDma3;
    }
    else
    {
        gBattlerControllerFuncs[battler] = PokedudeHandleInputChooseMove;
    }
}

static void PokedudeHandleChooseItem(u32 battler)
{
    s32 i;

    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
    gBattlerControllerFuncs[battler] = OpenBagAndChooseItem;
    gBattlerInMenuId = battler;
    for (i = 0; i < 3; ++i)
        gBattlePartyCurrentOrder[i] = gBattleResources->bufferA[battler][i + 1];
}

static void PokedudeHandleChoosePokemon(u32 battler)
{
    s32 i;

    gBattleControllerData[battler] = CreateTask(TaskDummy, 0xFF);
    gTasks[gBattleControllerData[battler]].data[0] = gBattleResources->bufferA[battler][1] & 0xF;
    *(&gBattleStruct->battlerPreventingSwitchout) = gBattleResources->bufferA[battler][1] >> 4;
    *(&gBattleStruct->playerPartyIdx) = gBattleResources->bufferA[battler][2];
    *(&gBattleStruct->abilityPreventingSwitchout) = (gBattleResources->bufferA[battler][3] & 0xFF) | (gBattleResources->bufferA[battler][7] << 8);
    for (i = 0; i < 3; ++i)
        gBattlePartyCurrentOrder[i] = gBattleResources->bufferA[battler][4 + i];
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
    gBattlerControllerFuncs[battler] = OpenPartyMenuToChooseMon;
    gBattlerInMenuId = battler;
}

static void PokedudeHandleCmd23(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleHealthBarUpdate(u32 battler)
{
    s16 hpVal;
    struct Pokemon *mon;

    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
        mon = &gPlayerParty[gBattlerPartyIndexes[battler]];
    else
        mon = &gEnemyParty[gBattlerPartyIndexes[battler]];
    LoadBattleBarGfx(0);
    hpVal = gBattleResources->bufferA[battler][2] | (gBattleResources->bufferA[battler][3] << 8);
    if (hpVal != INSTANT_HP_BAR_DROP)
    {
        u32 maxHP = GetMonData(mon, MON_DATA_MAX_HP);
        u32 curHP = GetMonData(mon, MON_DATA_HP);

        SetBattleBarStruct(battler, gHealthboxSpriteIds[battler], maxHP, curHP, hpVal);
    }
    else
    {
        u32 maxHP = GetMonData(mon, MON_DATA_MAX_HP);

        SetBattleBarStruct(battler, gHealthboxSpriteIds[battler], maxHP, 0, hpVal);
        UpdateHpTextInHealthbox(gHealthboxSpriteIds[battler], HP_CURRENT, 0, maxHP);
    }
    gBattlerControllerFuncs[battler] = CompleteOnHealthbarDone;
}

static void PokedudeHandleExpUpdate(u32 battler)
{
    u8 monId = gBattleResources->bufferA[battler][1];

    if (GetMonData(&gPlayerParty[monId], MON_DATA_LEVEL) >= MAX_LEVEL)
    {
        PokedudeBufferExecCompleted(battler);
    }
    else
    {
        s16 expPointsToGive;
        u8 taskId;

        LoadBattleBarGfx(1);
        GetMonData(&gPlayerParty[monId], MON_DATA_SPECIES);  // Unused return value.
        expPointsToGive = T1_READ_16(&gBattleResources->bufferA[battler][2]);
        taskId = CreateTask(Task_GiveExpToMon, 10);
        gTasks[taskId].tExpTask_monId = monId;
        gTasks[taskId].tExpTask_gainedExp = expPointsToGive;
        gTasks[taskId].tExpTask_battler = battler;
        gBattlerControllerFuncs[battler] = PokedudeDummy;
    }
}

static void PokedudeHandleStatusIconUpdate(u32 battler)
{
    if (!IsBattleSEPlaying(battler))
    {
        u8 battlerId;
        struct Pokemon *mon;

        if (GetBattlerSide(battler) == B_SIDE_PLAYER)
            mon = &gPlayerParty[gBattlerPartyIndexes[battler]];
        else
            mon = &gEnemyParty[gBattlerPartyIndexes[battler]];
        UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], mon, HEALTHBOX_STATUS_ICON);
        battlerId = battler;
        gBattleSpritesDataPtr->healthBoxesData[battlerId].statusAnimActive = FALSE;
        gBattlerControllerFuncs[battler] = CompleteOnFinishedStatusAnimation;
    }
}

static void PokedudeHandleStatusAnimation(u32 battler)
{
    if (!IsBattleSEPlaying(battler))
    {
        InitAndLaunchChosenStatusAnimation(battler, gBattleResources->bufferA[battler][1],
                                           gBattleResources->bufferA[battler][2] | (gBattleResources->bufferA[battler][3] << 8) | (gBattleResources->bufferA[battler][4] << 16) | (gBattleResources->bufferA[battler][5] << 24));
        gBattlerControllerFuncs[battler] = CompleteOnFinishedStatusAnimation;
    }
}

static void PokedudeHandleStatusXor(u32 battler)
{
    struct Pokemon *mon;
    u8 val;

    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
        mon = &gPlayerParty[gBattlerPartyIndexes[battler]];
    else
        mon = &gEnemyParty[gBattlerPartyIndexes[battler]];
    val = GetMonData(mon, MON_DATA_STATUS) ^ gBattleResources->bufferA[battler][1];
    SetMonData(mon, MON_DATA_STATUS, &val);
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleDataTransfer(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleDMA3Transfer(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandlePlayBGM(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleCmd32(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleTwoReturnValues(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleChosenMonReturnValue(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleOneReturnValue(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleOneReturnValue_Duplicate(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleCmd37(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleCmd38(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleCmd39(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleCmd40(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleHitAnimation(u32 battler)
{
    if (gSprites[gBattlerSpriteIds[battler]].invisible == TRUE)
    {
        PokedudeBufferExecCompleted(battler);
    }
    else
    {
        gDoingBattleAnim = TRUE;
        gSprites[gBattlerSpriteIds[battler]].data[1] = 0;
        DoHitAnimHealthboxEffect(battler);
        gBattlerControllerFuncs[battler] = DoHitAnimBlinkSpriteEffect;
    }
}

static void PokedudeHandleCmd42(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandlePlaySE(u32 battler)
{
    PlaySE(gBattleResources->bufferA[battler][1] | (gBattleResources->bufferA[battler][2] << 8));
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandlePlayFanfare(u32 battler)
{
    PlayFanfare(gBattleResources->bufferA[battler][1] | (gBattleResources->bufferA[battler][2] << 8));
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleFaintingCry(u32 battler)
{
    struct Pokemon *mon;
    u16 species;

    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
        mon = &gPlayerParty[gBattlerPartyIndexes[battler]];
    else
        mon = &gEnemyParty[gBattlerPartyIndexes[battler]];
    species = GetMonData(mon, MON_DATA_SPECIES);
    PlayCry_Normal(species, 25);
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleIntroSlide(u32 battler)
{
    HandleIntroSlide(gBattleResources->bufferA[battler][1]);
    gIntroSlideFlags |= 1;
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleIntroTrainerBallThrow(u32 battler)
{
    u8 paletteNum;
    u8 taskId;

    SetSpritePrimaryCoordsFromSecondaryCoords(&gSprites[gBattlerSpriteIds[battler]]);
    gSprites[gBattlerSpriteIds[battler]].data[0] = 50;
    gSprites[gBattlerSpriteIds[battler]].data[2] = -40;
    gSprites[gBattlerSpriteIds[battler]].data[4] = gSprites[gBattlerSpriteIds[battler]].y;
    gSprites[gBattlerSpriteIds[battler]].callback = StartAnimLinearTranslation;
    gSprites[gBattlerSpriteIds[battler]].data[5] = battler;
    StoreSpriteCallbackInData6(&gSprites[gBattlerSpriteIds[battler]], SpriteCB_FreePlayerSpriteLoadMonSprite);
    StartSpriteAnim(&gSprites[gBattlerSpriteIds[battler]], 1);
    paletteNum = AllocSpritePalette(0xD6F8);
    LoadCompressedPalette(gTrainerBackPicPaletteTable[TRAINER_BACK_PIC_POKEDUDE].data, OBJ_PLTT_ID(paletteNum), PLTT_SIZE_4BPP);
    gSprites[gBattlerSpriteIds[battler]].oam.paletteNum = paletteNum;
    taskId = CreateTask(Task_StartSendOutAnim, 5);
    gTasks[taskId].data[0] = battler;
    if (gBattleSpritesDataPtr->healthBoxesData[battler].partyStatusSummaryShown)
        gTasks[gBattlerStatusSummaryTaskId[battler]].func = Task_HidePartyStatusSummary;
    gBattleSpritesDataPtr->animationData->introAnimActive = TRUE;
    gBattlerControllerFuncs[battler] = PokedudeDummy;
}

// static void StartSendOutAnim(u8 battlerId)
// {
//     u16 species;

//     gBattleSpritesDataPtr->battlerData[battlerId].transformSpecies = SPECIES_NONE;
//     gBattlerPartyIndexes[battlerId] = gBattleResources->bufferA[battlerId][1];
//     species = GetMonData(&gPlayerParty[gBattlerPartyIndexes[battlerId]], MON_DATA_SPECIES);
//     gBattleControllerData[battlerId] = CreateInvisibleSpriteWithCallback(SpriteCB_WaitForBattlerBallReleaseAnim);
//     SetMultiuseSpriteTemplateToPokemon(species, GetBattlerPosition(battlerId));
//     gBattlerSpriteIds[battlerId] = CreateSprite(&gMultiuseSpriteTemplate,
//                                                 GetBattlerSpriteCoord(battlerId, BATTLER_COORD_X_2),
//                                                 GetBattlerSpriteDefault_Y(battlerId),
//                                                 GetBattlerSpriteSubpriority(battlerId));
//     gSprites[gBattleControllerData[battlerId]].data[1] = gBattlerSpriteIds[battlerId];
//     gSprites[gBattlerSpriteIds[battlerId]].data[0] = battlerId;
//     gSprites[gBattlerSpriteIds[battlerId]].data[2] = species;
//     gSprites[gBattlerSpriteIds[battlerId]].oam.paletteNum = battlerId;
//     StartSpriteAnim(&gSprites[gBattlerSpriteIds[battlerId]], gBattleMonForms[battlerId]);
//     gSprites[gBattlerSpriteIds[battlerId]].invisible = TRUE;
//     gSprites[gBattlerSpriteIds[battlerId]].callback = SpriteCallbackDummy;
//     gSprites[gBattleControllerData[battlerId]].data[0] = DoPokeballSendOutAnimation(battlerId, 0, POKEBALL_PLAYER_SENDOUT);
// }

static void Task_StartSendOutAnim(u8 taskId)
{
    if (gTasks[taskId].data[1] < 31)
    {
        ++gTasks[taskId].data[1];
    }
    else
    {
        u8 battler = gTasks[taskId].data[0];
        gBattleResources->bufferA[battler][1] = gBattlerPartyIndexes[battler];
        StartSendOutAnim(battler, FALSE);

        gBattlerControllerFuncs[battler] = Intro_TryShinyAnimShowHealthbox;
        DestroyTask(taskId);
    }
}

static void PokedudeHandleDrawPartyStatusSummary(u32 battler)
{
    if (gBattleResources->bufferA[battler][1] != 0
        && GetBattlerSide(battler) == B_SIDE_PLAYER)
    {
        PokedudeBufferExecCompleted(battler);
    }
    else
    {
        gBattleSpritesDataPtr->healthBoxesData[battler].partyStatusSummaryShown = TRUE;
        gBattlerStatusSummaryTaskId[battler] = CreatePartyStatusSummarySprites(battler,
                                                                                      (struct HpAndStatus *)&gBattleResources->bufferA[battler][4],
                                                                                      gBattleResources->bufferA[battler][1],
                                                                                      gBattleResources->bufferA[battler][2]);
        PokedudeBufferExecCompleted(battler);
    }
}

static void PokedudeHandleHidePartyStatusSummary(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleEndBounceEffect(u32 battler)
{
    EndBounceEffect(battler, BOUNCE_HEALTHBOX);
    EndBounceEffect(battler, BOUNCE_MON);
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleSpriteInvisibility(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleBattleAnimation(u32 battler)
{
    u8 animationId = gBattleResources->bufferA[battler][1];
    u16 argument = gBattleResources->bufferA[battler][2] | (gBattleResources->bufferA[battler][3] << 8);

    if (TryHandleLaunchBattleTableAnimation(battler, battler, battler, animationId, argument))
        PokedudeBufferExecCompleted(battler);
    else
        gBattlerControllerFuncs[battler] = CompleteOnFinishedBattleAnimation;
}

static void PokedudeHandleLinkStandbyMsg(u32 battler)
{
    switch (gBattleResources->bufferA[battler][1])
    {
    case 0:
    case 1:
        EndBounceEffect(battler, BOUNCE_HEALTHBOX);
        EndBounceEffect(battler, BOUNCE_MON);
        break;
    case 2:
        break;
    }
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleResetActionMoveSelection(u32 battler)
{
    PokedudeBufferExecCompleted(battler);
}

static void PokedudeHandleCmd55(u32 battler)
{
    gBattleOutcome = gBattleResources->bufferA[battler][1];
    FadeOutMapMusic(5);
    BeginFastPaletteFade(FAST_FADE_OUT_TO_BLACK);
    PokedudeBufferExecCompleted(battler);
    if (!(gBattleTypeFlags & BATTLE_TYPE_IS_MASTER) && gBattleTypeFlags & BATTLE_TYPE_LINK)
        gBattlerControllerFuncs[battler] = Pokedude_SetBattleEndCallbacks;
}

static void PokedudeCmdEnd(u32 battler)
{
}

// Script handlers

struct PokedudeInputScript
{
    // 0-3 for selection, 4 to repeat action, 255 to repeat move
    u8 cursorPos[MAX_BATTLERS_COUNT];
    u8 delay[MAX_BATTLERS_COUNT];
};

static const struct PokedudeInputScript sInputScripts_ChooseAction_Battle[] =
{
    {
        .cursorPos = {0, 0},
        .delay     = {64, 0}
    },
    {
        .cursorPos = {4, 4},
        .delay     = {0, 0}
    },
};

static const struct PokedudeInputScript sInputScripts_ChooseAction_Status[] =
{
    {
        .cursorPos = {0, 0},
        .delay     = {64, 0}
    },
    {
        .cursorPos = {1, 0},
        .delay     = {64, 0}
    },
    {
        .cursorPos = {0, 0},
        .delay     = {64, 0}
    },
};

static const struct PokedudeInputScript sInputScripts_ChooseAction_Matchups[] =
{
    {
        .cursorPos = {0, 0},
        .delay     = {64, 0}
    },
    {
        .cursorPos = {2, 0},
        .delay     = {64, 0}
    },
    {
        .cursorPos = {0, 0},
        .delay     = {64, 0}
    },
};

static const struct PokedudeInputScript sInputScripts_ChooseAction_Catching[] =
{
    {
        .cursorPos = {0, 0},
        .delay     = {64, 0}
    },
    {
        .cursorPos = {0, 0},
        .delay     = {64, 0}
    },
    {
        .cursorPos = {1, 0},
        .delay     = {64, 0}
    },
};

static const struct PokedudeInputScript *const sInputScripts_ChooseAction[] =
{
    [TTVSCR_BATTLE]   = sInputScripts_ChooseAction_Battle,
    [TTVSCR_STATUS]   = sInputScripts_ChooseAction_Status,
    [TTVSCR_MATCHUPS] = sInputScripts_ChooseAction_Matchups,
    [TTVSCR_CATCHING] = sInputScripts_ChooseAction_Catching,
};

static const struct PokedudeInputScript sInputScripts_ChooseMove_Battle[] =
{
    {
        .cursorPos = {  2,   2},
        .delay     = { 64,   0}
    },
    {
        .cursorPos = {255, 255},
        .delay     = {  0,   0}
    },
};

static const struct PokedudeInputScript sInputScripts_ChooseMove_Status[] =
{
    {
        .cursorPos = {  2,   2},
        .delay     = { 64,   0}
    },
    {
        .cursorPos = {  2,   0},
        .delay     = { 64,   0}
    },
    {
        .cursorPos = {  2,   0},
        .delay     = { 64,   0}
    },
    {
        .cursorPos = {255, 255},
        .delay     = {  0,   0}
    },
};

static const struct PokedudeInputScript sInputScripts_ChooseMove_Matchups[] =
{
    {
        .cursorPos = {  2,   0},
        .delay     = { 64,   0}
    },
    {
        .cursorPos = {  0,   0},
        .delay     = { 64,   0}
    },
    {
        .cursorPos = {  0,   0},
        .delay     = { 64,   0}
    },
    {
        .cursorPos = {255, 255},
        .delay     = {  0,   0}
    },
};

static const struct PokedudeInputScript sInputScripts_ChooseMove_Catching[] =
{
    {
        .cursorPos = {  0,   2},
        .delay     = { 64,   0}
    },
    {
        .cursorPos = {  2,   2},
        .delay     = { 64,   0}
    },
    {
        .cursorPos = {255, 255},
        .delay     = {  0,   0}
    },
};

static const struct PokedudeInputScript *const sInputScripts_ChooseMove[] =
{
    [TTVSCR_BATTLE]   = sInputScripts_ChooseMove_Battle,
    [TTVSCR_STATUS]   = sInputScripts_ChooseMove_Status,
    [TTVSCR_MATCHUPS] = sInputScripts_ChooseMove_Matchups,
    [TTVSCR_CATCHING] = sInputScripts_ChooseMove_Catching,
};

static const struct PokedudeTextScriptHeader sPokedudeTextScripts_Battle[] =
{
    {
        .btlcmd = CONTROLLER_CHOOSEACTION,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_PRINTSTRING,
        .side = B_SIDE_OPPONENT,
        .stringid = STRINGID_USEDMOVE,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_CHOOSEACTION,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_PRINTSTRING,
        .side = B_SIDE_PLAYER,
        .stringid = STRINGID_PKMNGAINEDEXP,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
};

static const struct PokedudeTextScriptHeader sPokedudeTextScripts_Status[] =
{
    {
        .btlcmd = CONTROLLER_CHOOSEACTION,
        .side = B_SIDE_PLAYER,
        .callback = NULL,
    },
    {
        .btlcmd = CONTROLLER_CHOOSEACTION,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintMessageWithHealthboxPals,
    },
    {
        .btlcmd = CONTROLLER_OPENBAG,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_PRINTSTRING,
        .side = B_SIDE_OPPONENT,
        .stringid = STRINGID_USEDMOVE,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_PRINTSTRING,
        .side = B_SIDE_PLAYER,
        .stringid = STRINGID_PKMNGAINEDEXP,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
};

static const struct PokedudeTextScriptHeader sPokedudeTextScripts_Matchups[] =
{
    {
        .btlcmd = CONTROLLER_PRINTSTRING,
        .side = B_SIDE_OPPONENT,
        .stringid = STRINGID_USEDMOVE,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_CHOOSEACTION,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_CHOOSEPOKEMON,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_PRINTSTRING,
        .side = B_SIDE_OPPONENT,
        .stringid = STRINGID_USEDMOVE,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_CHOOSEACTION,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_CHOOSEMOVE,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_PRINTSTRING,
        .side = B_SIDE_PLAYER,
        .stringid = STRINGID_PKMNGAINEDEXP,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
};

static const struct PokedudeTextScriptHeader sPokedudeTextScripts_Catching[] =
{
    {
        .btlcmd = CONTROLLER_CHOOSEACTION,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_CHOOSEACTION,
        .side = B_SIDE_PLAYER,
        .callback = NULL,
    },
    {
        .btlcmd = CONTROLLER_CHOOSEACTION,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_PRINTSTRING,
        .side = B_SIDE_OPPONENT,
        .stringid = STRINGID_PKMNFASTASLEEP,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_OPENBAG,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
    {
        .btlcmd = CONTROLLER_ENDLINKBATTLE,
        .side = B_SIDE_PLAYER,
        .callback = PokedudeAction_PrintVoiceoverMessage,
    },
};

static const struct PokedudeTextScriptHeader *const sPokedudeTextScripts[] =
{
    [TTVSCR_BATTLE]   = sPokedudeTextScripts_Battle,
    [TTVSCR_STATUS]   = sPokedudeTextScripts_Status,
    [TTVSCR_MATCHUPS] = sPokedudeTextScripts_Matchups,
    [TTVSCR_CATCHING] = sPokedudeTextScripts_Catching,
};

static const u8 *const sPokedudeTexts_Battle[] =
{
    Pokedude_Text_SpeedierBattlerGoesFirst,
    Pokedude_Text_MyRattataFasterThanPidgey,
    Pokedude_Text_BattlersTakeTurnsAttacking,
    Pokedude_Text_MyRattataWonGetsEXP,
};

static const u8 *const sPokedudeTexts_Status[] =
{
    Pokedude_Text_UhOhRattataPoisoned,
    Pokedude_Text_UhOhRattataPoisoned,
    Pokedude_Text_HealStatusRightAway,
    Pokedude_Text_UsingItemTakesTurn,
    Pokedude_Text_YayWeManagedToWin,
};

static const u8 *const sPokedudeTexts_TypeMatchup[] =
{
    Pokedude_Text_WaterNotVeryEffectiveAgainstGrass,
    Pokedude_Text_GrassEffectiveAgainstWater,
    Pokedude_Text_LetsTryShiftingMons,
    Pokedude_Text_ShiftingUsesTurn,
    Pokedude_Text_ButterfreeDoubleResistsGrass,
    Pokedude_Text_ButterfreeGoodAgainstOddish,
    Pokedude_Text_YeahWeWon,
};

static const u8 *const sPokedudeTexts_Catching[] =
{
    Pokedude_Text_WeakenMonBeforeCatching,
    Pokedude_Text_WeakenMonBeforeCatching,
    Pokedude_Text_BestIfTargetStatused,
    Pokedude_Text_CantDoubleUpOnStatus,
    Pokedude_Text_LetMeThrowBall,
    Pokedude_Text_PickBestKindOfBall,
};

static const struct PokedudeBattlePartyInfo sParties_Battle[] =
{
    {
        .side = B_SIDE_PLAYER,
        .level = 15,
        .species = SPECIES_RATTATA,
        .moves = { MOVE_TACKLE, MOVE_TAIL_WHIP, MOVE_HYPER_FANG, MOVE_QUICK_ATTACK },
        .nature = NATURE_LONELY,
        .gender = MALE,
    },
    {
        .side = B_SIDE_OPPONENT,
        .level = 18,
        .species = SPECIES_PIDGEY,
        .moves = { MOVE_TACKLE, MOVE_SAND_ATTACK, MOVE_GUST, MOVE_QUICK_ATTACK },
        .nature = NATURE_NAUGHTY,
        .gender = MALE,
    },
    {0xFF}
};

static const struct PokedudeBattlePartyInfo sParties_Status[] =
{
    {
        .side = B_SIDE_PLAYER,
        .level = 15,
        .species = SPECIES_RATTATA,
        .moves = { MOVE_TACKLE, MOVE_TAIL_WHIP, MOVE_HYPER_FANG, MOVE_QUICK_ATTACK },
        .nature = NATURE_LONELY,
        .gender = MALE,
    },
    {
        .side = B_SIDE_OPPONENT,
        .level = 14,
        .species = SPECIES_ODDISH,
        .moves = { MOVE_ABSORB, MOVE_SWEET_SCENT, MOVE_POISON_POWDER },
        .nature = NATURE_RASH,
        .gender = MALE,
    },
    {0xFF}
};

static const struct PokedudeBattlePartyInfo sParties_Matchups[] =
{
    {
        .side = B_SIDE_PLAYER,
        .level = 15,
        .species = SPECIES_POLIWAG,
        .moves = { MOVE_WATER_GUN, MOVE_HYPNOSIS, MOVE_BUBBLE },
        .nature = NATURE_RASH,
        .gender = MALE,
    },
    {
        .side = B_SIDE_PLAYER,
        .level = 15,
        .species = SPECIES_BUTTERFREE,
        .moves = { MOVE_CONFUSION, MOVE_POISON_POWDER, MOVE_STUN_SPORE, MOVE_SLEEP_POWDER },
        .nature = NATURE_RASH,
        .gender = MALE,
    },
    {
        .side = B_SIDE_OPPONENT,
        .level = 14,
        .species = SPECIES_ODDISH,
        .moves = { MOVE_ABSORB, MOVE_SWEET_SCENT, MOVE_POISON_POWDER },
        .nature = NATURE_RASH,
        .gender = MALE,
    },
    {0xFF}
};

static const struct PokedudeBattlePartyInfo sParties_Catching[] =
{
    {
        .side = B_SIDE_PLAYER,
        .level = 15,
        .species = SPECIES_BUTTERFREE,
        .moves = { MOVE_CONFUSION, MOVE_POISON_POWDER, MOVE_SLEEP_POWDER, MOVE_STUN_SPORE },
        .nature = NATURE_RASH,
        .gender = MALE,
    },
    {
        .side = B_SIDE_OPPONENT,
        .level = 11,
        .species = SPECIES_JIGGLYPUFF,
        .moves = { MOVE_SING, MOVE_DEFENSE_CURL, MOVE_POUND },
        .nature = NATURE_CAREFUL,
        .gender = MALE,
    },
    {0xFF}
};


static const struct PokedudeBattlePartyInfo *const sPokedudeBattlePartyPointers[] =
{
    [TTVSCR_BATTLE]   = sParties_Battle,
    [TTVSCR_STATUS]   = sParties_Status,
    [TTVSCR_MATCHUPS] = sParties_Matchups,
    [TTVSCR_CATCHING] = sParties_Catching,
};

struct PokedudeBattlerState *gPokedudeBattlerStates[MAX_BATTLERS_COUNT];

static void PokedudeSimulateInputChooseAction(u32 battler)
{
    const struct PokedudeInputScript *script_p = sInputScripts_ChooseAction[gBattleStruct->pdScriptNum];

    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
    {
        DoBounceEffect(battler, BOUNCE_HEALTHBOX, 7, 1);
        DoBounceEffect(battler, BOUNCE_MON, 7, 1);
    }
    if (script_p[gPokedudeBattlerStates[battler]->action_idx].delay[battler] == gPokedudeBattlerStates[battler]->timer)
    {
        if (GetBattlerSide(battler) == B_SIDE_PLAYER)
            PlaySE(SE_SELECT);
        gPokedudeBattlerStates[battler]->timer = 0;
        switch (script_p[gPokedudeBattlerStates[battler]->action_idx].cursorPos[battler])
        {
        case 0:
            BtlController_EmitTwoReturnValues(battler, 1, B_ACTION_USE_MOVE, 0);
            break;
        case 1:
            BtlController_EmitTwoReturnValues(battler, 1, B_ACTION_USE_ITEM, 0);
            break;
        case 2:
            BtlController_EmitTwoReturnValues(battler, 1, B_ACTION_SWITCH, 0);
            break;
        case 3:
            BtlController_EmitTwoReturnValues(battler, 1, B_ACTION_RUN, 0);
            break;
        }
        PokedudeBufferExecCompleted(battler);
        ++gPokedudeBattlerStates[battler]->action_idx;
        if (script_p[gPokedudeBattlerStates[battler]->action_idx].cursorPos[battler] == 4)
            gPokedudeBattlerStates[battler]->action_idx = 0;
    }
    else
    {
        if (gActionSelectionCursor[battler] != script_p[gPokedudeBattlerStates[battler]->action_idx].cursorPos[battler]
            && script_p[gPokedudeBattlerStates[battler]->action_idx].delay[battler] / 2 == gPokedudeBattlerStates[battler]->timer)
        {
            PlaySE(SE_SELECT);
            ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
            gActionSelectionCursor[battler] = script_p[gPokedudeBattlerStates[battler]->action_idx].cursorPos[battler];
            ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
        }
        ++gPokedudeBattlerStates[battler]->timer;
    }
}

static void PokedudeSimulateInputChooseMove(u32 battler)
{
    const struct PokedudeInputScript *script_p = sInputScripts_ChooseMove[gBattleStruct->pdScriptNum];

    if (script_p[gPokedudeBattlerStates[battler]->move_idx].delay[battler] == gPokedudeBattlerStates[battler]->timer)
    {
        if (GetBattlerSide(battler) == B_SIDE_PLAYER)
            PlaySE(SE_SELECT);
        gPokedudeBattlerStates[battler]->timer = 0;
        BtlController_EmitTwoReturnValues(battler, 1,
                                          B_ACTION_EXEC_SCRIPT,
                                          script_p[gPokedudeBattlerStates[battler]->move_idx].cursorPos[battler] | ((battler ^ BIT_SIDE) << 8));
        PokedudeBufferExecCompleted(battler);
        ++gPokedudeBattlerStates[battler]->move_idx;
        if (script_p[gPokedudeBattlerStates[battler]->move_idx].cursorPos[battler] == 255)
            gPokedudeBattlerStates[battler]->move_idx = 0;
    }
    else
    {
        if (script_p[gPokedudeBattlerStates[battler]->move_idx].cursorPos[battler] != gMoveSelectionCursor[battler]
            && script_p[gPokedudeBattlerStates[battler]->move_idx].delay[battler] / 2 == gPokedudeBattlerStates[battler]->timer)
        {
            PlaySE(SE_SELECT);
            MoveSelectionDestroyCursorAt(gMoveSelectionCursor[battler]);
            gMoveSelectionCursor[battler] = script_p[gPokedudeBattlerStates[battler]->move_idx].cursorPos[battler];
            MoveSelectionCreateCursorAt(gMoveSelectionCursor[battler], 0);
        }
        ++gPokedudeBattlerStates[battler]->timer;
    }
}

static bool8 HandlePokedudeVoiceoverEtc(u32 battler)
{
    const struct PokedudeTextScriptHeader *header_p = sPokedudeTextScripts[gBattleStruct->pdScriptNum];
    const u16 * bstringid_p = (const u16 *)&gBattleResources->bufferA[battler][2];

    if (gBattleResources->bufferA[battler][0] != header_p[gBattleStruct->pdMessageNo].btlcmd)
        return FALSE;
    if (battler != header_p[gBattleStruct->pdMessageNo].side)
        return FALSE;
    if (gBattleResources->bufferA[battler][0] == CONTROLLER_PRINTSTRING && header_p[gBattleStruct->pdMessageNo].stringid != *bstringid_p)
        return FALSE;
    if (header_p[gBattleStruct->pdMessageNo].callback == NULL)
    {
        gBattleStruct->pdMessageNo++;
        return FALSE;
    }
    gBattlerControllerFuncs[battler] = header_p[gBattleStruct->pdMessageNo].callback;
    gPokedudeBattlerStates[battler]->timer = 0;
    gPokedudeBattlerStates[battler]->msg_idx = header_p[gBattleStruct->pdMessageNo].stringid;
    gBattleStruct->pdMessageNo++;
    return TRUE;
}

static void ReturnFromPokedudeAction(u32 battler)
{
    gPokedudeBattlerStates[battler]->timer = 0;
    gBattlerControllerFuncs[battler] = PokedudeBufferRunCommand;
}

static void PokedudeAction_PrintVoiceoverMessage(u32 battler)
{
    switch (gPokedudeBattlerStates[battler]->timer)
    {
    case 0:
        if (!gPaletteFade.active)
        {
            BeginNormalPaletteFade(0xFFFFFF7F, 4, 0, 8, RGB_BLACK);
            ++gPokedudeBattlerStates[battler]->timer;
        }
        break;
    case 1:
        if (!gPaletteFade.active)
        {
            gPokedudeBattlerStates[battler]->saved_bg0y = gBattle_BG0_Y;
            BtlCtrl_DrawVoiceoverMessageFrame();
            ++gPokedudeBattlerStates[battler]->timer;
        }
        break;
    case 2:
        gBattle_BG0_Y = 0;
        BattleStringExpandPlaceholdersToDisplayedString(GetPokedudeText());
        BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_OAK_OLD_MAN);
        ++gPokedudeBattlerStates[battler]->timer;
        break;
    case 3:
        if (!IsTextPrinterActive(24) && JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(0xFFFFFF7F, 4, 8, 0, RGB_BLACK);
            ++gPokedudeBattlerStates[battler]->timer;
        }
        break;
    case 4:
        if (!gPaletteFade.active)
        {
            if (gPokedudeBattlerStates[battler]->msg_idx == STRINGID_PKMNGAINEDEXP)
            {
                BattleStopLowHpSound();
                PlayBGM(MUS_VICTORY_WILD);
            }
            gBattle_BG0_Y = gPokedudeBattlerStates[battler]->saved_bg0y;
            BtlCtrl_RemoveVoiceoverMessageFrame();
            ReturnFromPokedudeAction(battler);
        }
        break;
    }
}

static void PokedudeAction_PrintMessageWithHealthboxPals(u32 battler)
{
    switch (gPokedudeBattlerStates[battler]->timer)
    {
    case 0:
        if (!gPaletteFade.active)
        {
            DoLoadHealthboxPalsForLevelUp(&gBattleStruct->pdHealthboxPal2,
                                          &gBattleStruct->pdHealthboxPal1,
                                          GetBattlerAtPosition(B_POSITION_PLAYER_LEFT));
            BeginNormalPaletteFade(0xFFFFFF7F, 4, 0, 8, RGB_BLACK);
            ++gPokedudeBattlerStates[battler]->timer;
        }
        break;
    case 1:
        if (!gPaletteFade.active)
        {
            u32 mask = (gBitTable[gBattleStruct->pdHealthboxPal2] | gBitTable[gBattleStruct->pdHealthboxPal1]) << 16;

            ++mask; // It's possible that this is influenced by other functions, as
            --mask; // this also striked in battle_controller_oak_old_man.c but was naturally fixed.
            BeginNormalPaletteFade(mask, 4, 8, 0, RGB_BLACK);
            ++gPokedudeBattlerStates[battler]->timer;
        }
        break;
    case 2:
        if (!gPaletteFade.active)
        {
            BtlCtrl_DrawVoiceoverMessageFrame();
            ++gPokedudeBattlerStates[battler]->timer;
        }
        break;
    case 3:
        BattleStringExpandPlaceholdersToDisplayedString(GetPokedudeText());
        BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_OAK_OLD_MAN);
        ++gPokedudeBattlerStates[battler]->timer;
        break;
    case 4:
        if (!IsTextPrinterActive(24) && JOY_NEW(A_BUTTON))
        {
            u32 mask;

            PlaySE(SE_SELECT);
            mask = (gBitTable[gBattleStruct->pdHealthboxPal2] | gBitTable[gBattleStruct->pdHealthboxPal1]) << 16;
            ++mask;
            --mask;
            BeginNormalPaletteFade(mask, 4, 0, 8, RGB_BLACK);
            ++gPokedudeBattlerStates[battler]->timer;
        }
        break;
    case 5:
        if (!gPaletteFade.active)
        {
            BeginNormalPaletteFade(0xFFFFFF7F, 4, 8, 0, RGB_BLACK);
            ++gPokedudeBattlerStates[battler]->timer;
        }
        break;
    case 6:
        if (!gPaletteFade.active)
        {
            if (gPokedudeBattlerStates[battler]->msg_idx == STRINGID_PKMNGAINEDEXP)
            {
                BattleStopLowHpSound();
                PlayBGM(MUS_VICTORY_WILD);
            }
            DoFreeHealthboxPalsForLevelUp(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT));
            BtlCtrl_RemoveVoiceoverMessageFrame();
            ReturnFromPokedudeAction(battler);
        }
        break;
    }
}

static const u8 *GetPokedudeText(void)
{
    switch (gBattleStruct->pdScriptNum)
    {
    case TTVSCR_BATTLE:
    default:
        return sPokedudeTexts_Battle[gBattleStruct->pdMessageNo - 1];
    case TTVSCR_STATUS:
        return sPokedudeTexts_Status[gBattleStruct->pdMessageNo - 1];
    case TTVSCR_MATCHUPS:
        return sPokedudeTexts_TypeMatchup[gBattleStruct->pdMessageNo - 1];
    case TTVSCR_CATCHING:
        return sPokedudeTexts_Catching[gBattleStruct->pdMessageNo - 1];
    }
}

void InitPokedudePartyAndOpponent(void)
{
    s32 i, j;
    struct Pokemon *mon;
    s32 myIdx = 0;
    s32 opIdx = 0;
    const struct PokedudeBattlePartyInfo *data;

    gBattleTypeFlags = BATTLE_TYPE_POKEDUDE;
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    data = sPokedudeBattlePartyPointers[gSpecialVar_0x8004];
    i = 0;
    do
    {
        if (data[i].side == B_SIDE_PLAYER)
            mon = &gPlayerParty[myIdx++];
        else
            mon = &gEnemyParty[opIdx++];
        CreateMonWithGenderNatureLetter(mon, data[i].species, data[i].level, 0, data[i].gender, data[i].nature, 0);
        for (j = 0; j < 4; ++j)
            SetMonMoveSlot(mon, data[i].moves[j], j);
    } while (data[++i].side != 0xFF);
}
