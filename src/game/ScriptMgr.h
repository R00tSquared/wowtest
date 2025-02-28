/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef HELLGROUND_SCRIPTMGR_H
#define HELLGROUND_SCRIPTMGR_H

#include "ace/Singleton.h"

#include "Common.h"

struct AreaTriggerEntry;
struct CinematicSequencesEntry;
class Aura;
class Creature;
class CreatureAI;
class GameObject;
class InstanceData;
class Item;
class Map;
class Object;
class Player;
class Quest;
class SpellCastTargets;
class Unit;
class WorldObject;

struct SpellEntry;

struct ScriptInfo
{
    uint32 id;
    uint32 delay;
    uint32 command;
    uint32 datalong;
    uint32 datalong2;
    int32  dataint;
    float x;
    float y;
    float z;
    float o;
};

typedef std::multimap<uint32, ScriptInfo> ScriptMap;
typedef std::map<uint32, ScriptMap > ScriptMapMap;
extern ScriptMapMap sQuestEndScripts;
extern ScriptMapMap sQuestStartScripts;
extern ScriptMapMap sSpellScripts;
extern ScriptMapMap sGameObjectScripts;
extern ScriptMapMap sEventScripts;
extern ScriptMapMap sWaypointScripts;

#define MAX_SCRIPTS         5000                            //72 bytes each (approx 351kb)
#define VISIBLE_RANGE       (166.0f)                        //MAX visible range (size of grid)
#define DEFAULT_TEXT        "<Hellground Script Text Entry Missing!>"
#define TEXT_SOURCE_RANGE -1000000                          //the amount of entries each text source has available

//TODO: find better namings and definitions.
//N=Neutral, A=Alliance, H=Horde.
//NEUTRAL or FRIEND = Hostility to player surroundings (not a good definition)
//ACTIVE or PASSIVE = Hostility to environment surroundings.
enum eEscortFaction
{
    FACTION_ESCORT_A_NEUTRAL_PASSIVE = 10,
    FACTION_ESCORT_H_NEUTRAL_PASSIVE = 33,
    FACTION_ESCORT_N_NEUTRAL_PASSIVE = 113,

    FACTION_ESCORT_A_NEUTRAL_ACTIVE = 231,
    FACTION_ESCORT_H_NEUTRAL_ACTIVE = 232,
    FACTION_ESCORT_N_NEUTRAL_ACTIVE = 250,

    FACTION_ESCORT_N_FRIEND_PASSIVE = 290,
    FACTION_ESCORT_N_FRIEND_ACTIVE = 495,

    FACTION_ESCORT_A_PASSIVE = 774,
    FACTION_ESCORT_H_PASSIVE = 775,

    FACTION_ESCORT_N_ACTIVE = 1986,
    FACTION_ESCORT_H_ACTIVE = 2046
};

struct ScriptPointMove
{
    uint32 uiCreatureEntry;
    uint32 uiPointId;
    float  fX;
    float  fY;
    float  fZ;
    uint32 uiWaitTime;
};

struct StringTextData
{
    uint32 SoundId;
    uint8  Type;
    uint32 Language;
    uint32 Emote;
};

struct Script
{
    Script() :
        pGossipHelloPlayer(NULL), pGossipSelectPlayer(NULL), pGossipHello(NULL), pGossipSelect(NULL), pGossipSelectGO(NULL), pGossipHelloGo(NULL),
        pGossipSelectWithCode(NULL), pGossipSelectGOWithCode(NULL),
        pDialogStatusNPC(NULL), pDialogStatusGO(NULL),
        pQuestAcceptNPC(NULL), pQuestAcceptGO(NULL), pQuestAcceptItem(NULL),
        pQuestRewardedNPC(NULL), pQuestRewardedGO(NULL),
        pGOUse(NULL), pItemUse(NULL), pGossipSelectItem(NULL), pGossipSelectItemWithCode(NULL), pAreaTrigger(NULL), pCompletedCinematic(NULL),
        pProcessEventId(NULL), pReceiveEmote(NULL),
        pEffectDummyNPC(NULL), pEffectDummyGO(NULL), pEffectDummyItem(NULL), pEffectAuraDummy(NULL),
        GetAI(NULL), GetInstanceData(NULL),

        //spell scripts
        pSpellTargetMap(NULL), pSpellHandleEffect(NULL)
    {}

    std::string Name;

    bool (*pGossipHelloPlayer)(Player*, uint32 option);
    bool (*pGossipSelectPlayer)(Player*, uint32, uint32, uint32 option);
    bool (*pGossipSelectPlayerWithCode)(Player*, uint32, uint32, const char*, uint32 option);

    bool (*pGossipHello)(Player*, Creature*);
    bool (*pGossipHelloGo)(Player*, GameObject*);
    bool (*pGossipSelect)(Player*, Creature*, uint32, uint32);
    bool (*pGossipSelectGO)(Player*, GameObject*, uint32, uint32);
    bool (*pGossipSelectWithCode)(Player*, Creature*, uint32, uint32, const char*);
    bool (*pGossipSelectGOWithCode)(Player*, GameObject*, uint32, uint32, const char*);
    uint32(*pDialogStatusNPC)(Player*, Creature*);
    uint32(*pDialogStatusGO)(Player*, GameObject*);
    bool (*pQuestAcceptNPC)(Player*, Creature*, Quest const*);
    bool (*pQuestAcceptGO)(Player*, GameObject*, Quest const*);
    bool (*pQuestAcceptItem)(Player*, Item*, Quest const*);
    bool (*pQuestRewardedNPC)(Player*, Creature*, Quest const*);
    bool (*pQuestRewardedGO)(Player*, GameObject*, Quest const*);
    bool (*pGOUse)(Player*, GameObject*);
    bool (*pItemUse)(Player*, Item*, SpellCastTargets const&);
    bool (*pGossipSelectItem)(Player*, Item*, uint32, uint32, SpellCastTargets const&);
    bool (*pGossipSelectItemWithCode)(Player*, Item*, uint32, uint32, SpellCastTargets const&, const char*);
    bool (*pAreaTrigger)(Player*, AreaTriggerEntry const*);
    bool (*pCompletedCinematic)(Player*, CinematicSequencesEntry const*);
    bool (*pProcessEventId)(uint32, Object*, Object*, bool);
    bool (*pEffectDummyNPC)(Unit*, uint32, uint32, Creature*);
    bool (*pEffectDummyGO)(Unit*, uint32, uint32, GameObject*);
    bool (*pEffectDummyItem)(Unit*, uint32, uint32, Item*);
    bool (*pEffectAuraDummy)(const Aura*, bool);

    bool (*pReceiveEmote)(Player*, Creature*, uint32);

    //spell scripts
    bool (*pSpellTargetMap)(Unit*, std::list<Unit*>&, SpellCastTargets const&, SpellEntry const*, uint32);
    bool (*pSpellHandleEffect)(Unit* pCaster, Unit* pUnit, Item* pItem, GameObject* pGameObject, SpellEntry const* pSpell, uint32 effectIndex);

    CreatureAI* (*GetAI)(Creature*);
    InstanceData* (*GetInstanceData)(Map*);

    void RegisterSelf(bool bReportError = true);
};

class ScriptMgr
{
    friend class ACE_Singleton<ScriptMgr, ACE_Null_Mutex>;
    ScriptMgr();

    typedef std::vector<std::string> ScriptNameMap;

    public:
        ~ScriptMgr();

        //Maps and lists
        typedef UNORDERED_MAP<int32, StringTextData> TextDataMap;
        typedef UNORDERED_MAP<uint32, std::vector<ScriptPointMove> > PointMoveMap;

        //Database
        void LoadScriptTexts();
        void LoadScriptWaypoints();

        //Retrive from storage
        StringTextData const* GetTextData(int32 uiTextId) const
        {
            TextDataMap::const_iterator itr = m_mTextDataMap.find(uiTextId);

            if (itr == m_mTextDataMap.end())
                return NULL;

            return &itr->second;
        }

        std::vector<ScriptPointMove> const &GetPointMoveList(uint32 uiCreatureEntry) const
        {
            static std::vector<ScriptPointMove> vEmpty;

            PointMoveMap::const_iterator itr = m_mPointMoveMap.find(uiCreatureEntry);

            if (itr == m_mPointMoveMap.end())
                return vEmpty;

            return itr->second;
        }

        void LoadGameObjectScripts();
        void LoadQuestEndScripts();
        void LoadQuestStartScripts();
        void LoadEventScripts();
        void LoadEventIdScripts();
        void LoadSpellIdScripts();
        void LoadSpellScripts();
        void LoadWaypointScripts();

        void LoadDbScriptStrings();

        void LoadScriptNames();
        void LoadAreaTriggerScripts();
        void LoadCompletedCinematicScripts();

        uint32 GetAreaTriggerScriptId(uint32 triggerId) const;
        uint32 GetCompletedCinematicScriptId(uint32 cinematicId) const;
        uint32 GetEventIdScriptId(uint32 eventId) const;
        uint32 GetSpellIdScriptId(uint32 eventId) const;

        ScriptNameMap &GetScriptNames() { return m_scriptNames; }
        const char * GetScriptName(uint32 id) { return id < m_scriptNames.size() ? m_scriptNames[id].c_str() : ""; }
        uint32 GetScriptId(const char *name);

        bool LoadScriptLibrary();
        void InitScriptLibrary();
        void FreeScriptLibrary();
        void UnloadScriptLibrary();

        CreatureAI* GetCreatureAI(Creature* pCreature);
        InstanceData* CreateInstanceData(Map* pMap);

        bool OnGossipHello(Player* pPlayer, Creature* pCreature);
        bool OnGossipHello(Player* pPlayer, GameObject* pGameObject);
        bool OnGossipHello(Player* pPlayer, uint32 script_id, uint32 option = 0);
        bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action, const char* code);
        bool OnGossipSelect(Player* pPlayer, GameObject* pGameObject, uint32 sender, uint32 action, const char* code);
        bool OnGossipSelect(Player* pPlayer, uint32 script_id, uint32 uiSender, uint32 uiAction, const char* code, uint32 option);
        bool OnGossipSelectWithCode(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction, const char* sCode);
        bool OnGossipSelectWithCode(Player* pPlayer, GameObject* pGo, uint32 uiSender, uint32 uiAction, const char* sCode);
        bool OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool OnQuestAccept(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest);
        bool OnQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest);
        bool OnQuestRewarded(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
        bool OnQuestRewarded(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest);
        uint32 GetDialogStatus(Player* pPlayer, Creature* pCreature);
        uint32 GetDialogStatus(Player* pPlayer, GameObject* pGameObject);
        bool OnGameObjectUse(Player* pPlayer, GameObject* pGameObject);
        bool OnItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets);
        bool OnGossipSelectItem(Player* pPlayer, Item* pItem, uint32 uiSender, uint32 uiAction, SpellCastTargets const& targets, const char* code);
        bool OnAreaTrigger(Player* pPlayer, AreaTriggerEntry const* atEntry);
        bool OnCompletedCinematic(Player* pPlayer, CinematicSequencesEntry const* cinematic);
        bool OnProcessEvent(uint32 eventId, Object* pSource, Object* pTarget, bool isStart);
        bool OnEffectDummy(Unit* pCaster, uint32 spellId, uint32 effIndex, Creature* pTarget);
        bool OnEffectDummy(Unit* pCaster, uint32 spellId, uint32 effIndex, GameObject* pTarget);
        bool OnEffectDummy(Unit* pCaster, uint32 spellId, uint32 effIndex, Item* pTarget);
        bool OnAuraDummy(Aura const* pAura, bool apply);

        bool OnReceiveEmote(Player *pPlayer, Creature *pCreature, uint32 emote);

        // spell scripts
        bool OnSpellSetTargetMap(Unit* pCaster, std::list<Unit*> &unitList, SpellCastTargets const&, SpellEntry const *pSpell, uint32 effectIndex);
        bool OnSpellHandleEffect(Unit *pCaster, Unit* pUnit, Item* pItem, GameObject* pGameObject, SpellEntry const *pSpell, uint32 effectIndex);

        int num_sc_scripts;
        Script *m_scripts[MAX_SCRIPTS];
    private:
        void LoadScripts(ScriptMapMap& scripts, const char* tablename);
        void CheckScripts(ScriptMapMap const& scripts,std::set<int32>& ids);

        typedef UNORDERED_MAP<uint32, uint32> AreaTriggerScriptMap;
        typedef UNORDERED_MAP<uint32, uint32> CompletedCinematicScriptMap;
        typedef UNORDERED_MAP<uint32, uint32> EventIdScriptMap;
        typedef UNORDERED_MAP<uint32, uint32> SpellIdScriptMap;

        AreaTriggerScriptMap            m_AreaTriggerScripts;
        CompletedCinematicScriptMap     m_CompletedCinematicScripts;
        EventIdScriptMap                m_EventIdScripts;
        SpellIdScriptMap                m_SpellIdScripts;

        ScriptNameMap   m_scriptNames;
        TextDataMap     m_mTextDataMap;                     //additional data for text strings
        PointMoveMap    m_mPointMoveMap;                    //coordinates for waypoints
};

#define sScriptMgr (*ACE_Singleton<ScriptMgr, ACE_Null_Mutex>::instance())

HELLGROUND_IMPORT_EXPORT uint32 GetAreaTriggerScriptId(uint32 triggerId);
HELLGROUND_IMPORT_EXPORT uint32 GetCompletedCinematicScriptId(uint32 triggerId);
HELLGROUND_IMPORT_EXPORT uint32 GetScriptId(const char *name);
HELLGROUND_IMPORT_EXPORT uint32 GetEventIdScriptId(uint32 eventId);
HELLGROUND_IMPORT_EXPORT uint32 GetSpellIdScriptId(uint32 eventId);

//Generic scripting text function
void DoScriptText(int32 textEntry, WorldObject* pSource, Unit* target = NULL, bool withoutPrename = false);
void DoGlobalScriptText(int32 iTextEntry, const char *npcName, Map *map);
void ScriptText(int32 textEntry, Unit* pSource, Unit* target = NULL);

#endif
