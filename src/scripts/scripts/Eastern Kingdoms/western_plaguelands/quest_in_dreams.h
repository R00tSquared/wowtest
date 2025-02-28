#include "precompiled.h"
#include "escort_ai.h"
#ifndef DEF_INDREAMS_H
#define DEF_INDREAMS_H

enum
{
    QUEST_SCARLET_SUBTERFUGE    = 5862,
    QUEST_IN_DREAMS             = 5944,
    
    NPC_TAELAN                  = 1842,
    NPC_CAVALIER                = 1836,
    NPC_ISILLIEN                = 1840,
    NPC_CRIMSON                 = 12128,
    NPC_TIRION                  = 12126,

    FIRST_CAVALIER_GUID         = 48190,
    SECOND_CAVALIER_GUID        = 48188,
    THIRD_CAVALIER_GUID         = 49594,
    FOURTH_CAVALIER_GUID        = 52005,

    SAY_CAVALIER_1              = -1001074,
    SAY_CAVALIER_2              = -1001075,
    SAY_CAVALIER_3              = -1001106,
    SAY_CAVALIER_4              = -1001073,
    SAY_CAVALIER_5              = -1001072,
    SAY_CAVALIER_6              = -1001107,
    SAY_CAVALIER_7              = -1001108,
    SAY_CAVALIER_8              = -1001109,
    SAY_TAELAN_NOO              = -1001076,
    SAY_TAELAN_1                = -1001077,
    SAY_TAELAN_2                = -1001078,
    SAY_TAELAN_3                = -1001079,
    EMOTE_TAELAN_1              = -1001080,
    SAY_TAELAN_4                = -1001081,
    SAY_TAELAN_5                = -1001083,
    SAY_TAELAN_6                = -1001084,
    SAY_ISILLIEN_1              = -1001082,
    SAY_ISILLIEN_2              = -1001085,
    SAY_ISILLIEN_3              = -1001086,
    SAY_ISILLIEN_4              = -1001087,
    EMOTE_ISILLIEN_1            = -1001088,
    SAY_ISILLIEN_5              = -1001089,
    SAY_ISILLIEN_6              = -1001090,
    SAY_ISILLIEN_7              = -1001092,
    SAY_TIRION_1                = -1001094,
    SAY_ISILLIEN_8              = -1001095,
    SAY_TIRION_2                = -1001096,
    SAY_TIRION_3                = -1001097,
    SAY_ISILLIEN_9              = -1001098,
    SAY_TIRION_4                = -1001099,
    SAY_TIRION_5                = -1001101,
    SAY_TIRION_6                = -1001103,
    SAY_TIRION_7                = -1001104,
    SAY_TIRION_8                = -1001105,

    TAELAN_MOUNT                = 2402,
    TIRION_MOUNT                = 2404,

    // Taelan's Spells
    SPELL_DEVOTION_AURA         = 17232,
    SPELL_HOLY_LIGHT            = 15493,
    SPELL_MURDER                = 14158,
    SPELL_HOLY_CLEAVE           = 18819,
    SPELL_TAELAN_SUFFERING      = 18810,
    SPELL_LAY_ON_HANDS          = 633,

    // Cavalierses Spells
    SPELL_BACKHAND              = 6253,
    SPELL_CLEAVE                = 15284,
    SPELL_MORTAL_STRIKE         = 16856,

    // Isillien's spells
    SPELL_DOMINATE_MIND         = 20604,
    SPELL_MANA_BURN             = 15800,
    SPELL_MIND_BLAST            = 17194,
    SPELL_MIND_FLAY             = 17165,
    SPELL_FLASH_HEAL            = 10917,
    SPELL_GREATER_HEAL          = 10965,
    SPELL_POWER_SHIELD          = 10901,
    SPELL_TAELAN_DEATH          = 18969,

    // Tirion Spells
    SPELL_CRUSADER_STRIKE       = 14518,
    SPELL_HOLY_STRIKE           = 17143,

    // Crimson Elite Spells
    SPELL_RETRIBUTION_AURA      = 8990,

    SPELL_DISGUISE              = 17961,
};

struct SpawnCoordsStruct
{
    float x, y, z, o;
};

static const SpawnCoordsStruct SpawnCoordsIsillien =
{
    2686.4042f, -1932.2364f, 72.1442f, 2.13f
};

static const SpawnCoordsStruct SpawnCoordsTirion =
{
    2640.65f, -1914.08f, 71.35f, 0.33f
};

static const SpawnCoordsStruct SpawnCoordsCrimsonInner[] = 
{
    { 2683.9707f, -1934.0002f, 72.1442f, 2.13f },
    { 2689.6513f, -1930.7386f, 72.1442f, 2.13f }
};

static const SpawnCoordsStruct SpawnCoordsCrimsonOuter[] =
{
    { 2701.0891f, -1864.9894f, 67.5103f, 3.86f },
    { 2703.7336f, -1866.5752f, 67.8296f, 4.01f },
    { 2699.1355f, -1861.3512f, 67.5433f, 3.99f }
};

static const SpawnCoordsStruct MoveCoordsCrimsonInner[] =
{
    { 2675.8896f, -1923.2290f, 69.5838f, 0.0f },
    { 2683.6467f, -1917.1948f, 69.0735f, 0.0f }
};

static const SpawnCoordsStruct MoveCoordsCavaliers[] =
{
    { 2944.5576f, -1389.7694f, 167.2379f, 3.19f },
    { 2941.2106f, -1388.1791f, 167.2379f, 5.39f },
    { 2940.7463f, -1391.0331f, 167.2379f, 4.73f },
    { 2942.8647f, -1392.1495f, 167.2379f, 3.94f } 
};

/*
 * In Dreams support interface
 */

enum InDreamsIndex
{
    INDEX_TAELAN = 0,
    INDEX_ISILLIEN = 1,
    INDEX_TIRION = 2
};

class InDreamsInterface
{
public:
    InDreamsInterface()
    {
        m_uiGuids = new ObjectGuid[3];
    }

    ~InDreamsInterface()
    {
        if (m_uiGuids)
            delete m_uiGuids;
    }

    ObjectGuid* m_uiGuids;

    Creature* GetCreature(Unit* pSource, InDreamsIndex index) const
    {
        return pSource ? pSource->GetMap()->GetCreature(m_uiGuids[index]) : nullptr;
    }

    void AddGuid(ObjectGuid objectGuid, InDreamsIndex index) const
    {
        m_uiGuids[index] = objectGuid;
    }

    template <typename AI>
    AI* GetAI(Unit* pSource, InDreamsIndex index) const
    {
        if (auto pCreature = GetCreature(pSource, index))
        {
            if (auto pCreatureAI = static_cast<AI*>(pCreature->AI()))
                return pCreatureAI;
        }

        return nullptr;
    }
};

/*######
## npc_isillien
######*/

struct npc_isillienAI : npc_escortAI
{
    explicit npc_isillienAI(Creature* pCreature);

    void Reset();
    void SetEvent(int32 m_uiEvent);
    void EnterEvadeMode();
    void WaypointReached(uint32 uiPointId);
    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage);
    void GuardsDoDespawn() const;
    void GuardsDoAttack(Unit* pTarget, bool inner, bool start);
    void GuardsDoSummon(bool inner);
    void TimerSayAndGo(const uint32 uiDiff, uint32 uiUpd, int32 textId, Emote emote = EMOTE_ONESHOT_NONE, bool incSub = true);
    void Event1SummonCrimson();
    void Event2TalkBeforeFight(const uint32 uiDiff);
    void Event3FightTaelan(const uint32 uiDiff);
    void Event4KillTaelan(const uint32 uiDiff);
    void Event5TalkBeforeTirionArrival(const uint32 uiDiff);
    void Event6TirionArrival(const uint32 uiDiff);
    void Event7FightTirion(const uint32 uiDiff);
    void DoAttack(const uint32 uiDiff);
    void UpdateEscortAI(const uint32 uiDiff);
    static void GroupRemoveDisguise(Player* player);
    void FailEscort() const;

    bool m_isTirionHere;
    std::shared_ptr<InDreamsInterface> m_pInterface;

private:
    int8  m_uiEventCount;
    int8  m_uiSubEventCount;
    uint32 m_uiEventTimer;

    uint32 m_uiManaBurnTimer;
    uint32 m_uiPowerShieldTimer;
    uint32 m_uiMindBlastTimer;
    uint32 m_uiMindFlayTimer;
    uint32 m_uiDominateMindTimer;
    uint32 m_uiHealTimer;
    uint32 m_uiGlobalCooldownTimer;
    uint32 m_uiDominateMindFadingTimer;

    ObjectGuid pPlayerGuid;

    std::list<ObjectGuid> pCrimsonInner;
    std::list<ObjectGuid> pCrimsonOuter;

    bool m_isTaelanDead;
    bool m_bControllingPlayer;
};

/*######
## npc_scarlet_cavalier
######*/

struct npc_scarlet_cavalierAI : ScriptedAI
{
    explicit npc_scarlet_cavalierAI(Creature* pCreature);

    void Reset();
    void Event1WhatsUp(Creature* pTaelan);
    void Event2SurroundTaelan() const;
    void Event3AttackTaelan(Creature* pTaelan);
    void DoAttack(const uint32 uiDiff);
    void UpdateAI(const uint32 uiDiff);
    void MovementInform(uint32 uiMotionType, uint32 uiPointId);
    void JustDied(Unit* /*pKiller*/);

private:
    uint8  m_uiEventCount;
    uint32 m_uiEventTimer;

    uint32 m_uiMortalStrikeTimer;
    uint32 m_uiCleaveTimer;
    uint32 m_uiBackhandTimer;

    ObjectGuid m_pTaelanGuid;
};

/*######
## npc_taelan
######*/

struct npc_taelanAI : npc_escortAI
{
    explicit npc_taelanAI(Creature* pCreature);

    void Reset();
    void ResetCreature();
    void EnterEvadeMode();
    void JustDied(Unit* /*pKiller*/);
    void WaypointReached(uint32 uiPointId);
    bool CavaliersAreDead();
    static bool IsCavalierNear(uint32 counter);
    Player* GetPlayer();
    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell);
    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage);
    void MoveInLineOfSight(Unit* /*pWho*/);
    void Event1ReceivedPresent();
    void Event2FightCavaliers(const uint32 uiDiff);
    void Event4EscortStart(const uint32 uiDiff);
    void Event5MountAndGo(const uint32 uiDiff);
    void Event6FaceIsillien(const uint32 uiDiff);
    void Event7CompleteQuest();
    void Event8GraceShutdown(const uint32 uiDiff);
    void TimerSayAndGo(const uint32 uiDiff, uint32 uiUpd, int32 textId, bool incSub = true);
    void DoAttack(const uint32 uiDiff);
    void DoHeal(const uint32 uiDiff);
    void UpdateEscortAI(const uint32 uiDiff);

    bool    m_isDead;
    std::shared_ptr<InDreamsInterface> m_pInterface;

private:
    int8   m_uiEventCount;
    int8   m_uiSubEventCount;
    uint32  m_uiEventTimer;

    std::list<ObjectGuid> m_lCavaliersGUID;

    uint32  m_uiHolyCleaveTimer;
    uint32  m_uiHolyLightTimer;
    uint32  m_uiCrusaderStrikeTimer;
    uint32  m_uiLayOnHandsTimer;
    
    bool    m_killedCavaliers;
    bool    m_mountPhase;
};

/*######
## npc_tirion
######*/

struct npc_tirion_fordring_in_dreamsAI : npc_escortAI
{
    explicit npc_tirion_fordring_in_dreamsAI(Creature* pCreature);

    void Reset();
    void WaypointReached(uint32 uiPointId);
    void MovementInform(uint32 uiMotionType, uint32 uiPointId);
    void TimerSayAndGo(const uint32 uiDiff, uint32 uiUpd, int32 textId, Emote emote = EMOTE_ONESHOT_NONE, bool incEvent = true);
    void DoAttack(const uint32 uiDiff);
    void UpdateEscortAI(const uint32 uiDiff);
    void JustDied(Unit* pKiller);
    void FailEscort() const;

    std::shared_ptr<InDreamsInterface> m_pInterface;

private:
    uint8   m_uiEventCount;
    uint32  m_uiEventTimer;

    uint32  m_uiCrusaderStrikeTimer;
    uint32  m_uiHolyStrikeTimer;
    uint32  m_uiHolyCleaveTimer;
    uint32  m_uiDevotionAuraTimer;

    bool    m_isIsillienDead;
    bool    m_IsillienReady;
};

/*
 * Crimson Elite
 */

struct npc_crimson_eliteAI : ScriptedAI
{
    explicit npc_crimson_eliteAI(Creature* pCreature);

    void Reset();
    void EnterEvadeMode();
    void DoAttack(const uint32 uiDiff);
    void UpdateAI(const uint32 uiDiff);

private:
    uint32 m_uiCrusaderStrikeTimer;
    uint32 m_uiHolyStrikeTimer;
    uint32 m_uiRetributionAuraCheckTimer;
};

#endif