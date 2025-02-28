// Owned by DeathSide, Trentone
bool isWeaponGossip(uint32 uiAction);
void getInfoFromAction(uint32 uiAction, uint8 & gossip, uint8 & type, uint8 & slot, uint32 & itemId);
void DeleteTrans(Player* Plr, uint8 slot, uint32 itemId, bool Weapon);
void AddTrans(Player* Plr, uint8 slot, bool Weapon, Item* scriptItem);
void SelectTrans(Player* Plr, uint8 slot, uint8 type, uint32 itemId, bool Weapon);
const static uint32 item_transmog_skills[20] = // 20 - 19 + 1 for shields
{
    SKILL_AXES,     SKILL_2H_AXES,  SKILL_BOWS,          SKILL_GUNS         , SKILL_MACES,
    SKILL_2H_MACES, SKILL_POLEARMS, SKILL_SWORDS,        SKILL_2H_SWORDS    , 0,
    SKILL_STAVES,   0,              0,                   SKILL_FIST_WEAPONS , 0,
    SKILL_DAGGERS,  SKILL_THROWN,   0,                   SKILL_CROSSBOWS    , SKILL_WANDS
};
const static bool CanIUseHoldable[MAX_CLASSES-1] =
{
    false, false, false, false, true, false, false, true, true, false, true
};
/*
    CLASS_WARRIOR       = 1,
    CLASS_PALADIN       = 2,
    CLASS_HUNTER        = 3,
    CLASS_ROGUE         = 4,
    CLASS_PRIEST        = 5,
    CLASS_DEATH_KNIGHT  = 6,
    CLASS_SHAMAN        = 7,
    CLASS_MAGE          = 8,
    CLASS_WARLOCK       = 9,
    // CLASS_UNK2       = 10,unused
    CLASS_DRUID         = 11,
    */