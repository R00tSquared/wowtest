// Owned by DeathSide, Trentone
typedef struct
{
    uint64 PlayerGUID;
    uint64 ItemsGUID[8];
} PlayerGUID_8Items;
#define TextIdStart 3

#define REAGENT_START 202001

const uint16 Reinforcements[REINFORCE_STAT_COUNT][REINFORCE_LEVEL_COUNT] = 
{
    {1585,1591,1598,1606,1614}, // Attack Power 8 20 34
    {2889,2651,2937,2504,2669}, // Spell Damage 5 12 20
    {2310,2316,2323,2331,2339}, // Healing 9 22 37
    {2892,2911,1044,1052,1060}, // Strength 4 10 17
    {90,358,1094,1102,1110}, // Agility 4 10 17
    {94,359,1120,1128,1136}, // Intellect 4 10 17
    {98,360,1146,1154,1162}, // Spirit 4 10 17
    {2868,411,1076,1088,1214}, // Stamina 6 14 23
    {2366,2371,2379,2386,2394}, // Mana per 5 sec 2 4 7
    {113,1946,1956,1954,1968}, // Defense rating 4 10 17
    {19,21,484}, // Damage 2 4 6
    {123,1889,2662}, // Armor 30 70 120	
};


bool FindReinforcementEntryByEnchantId(uint16 EnchantId, uint8 &StatType, uint8 &Level)
{
    for (StatType = 0; StatType < REINFORCE_STAT_COUNT; StatType++)
        for (Level = 0; Level < REINFORCE_LEVEL_COUNT; Level++)
        {
            if (Reinforcements[StatType][Level] == EnchantId)
                return true;
        }
    // if not found
    return false;
};