#include "precompiled.h"

typedef struct
{
    uint64 playerGuid;
    uint32 playerBytes;
    uint8 facialFeature; // 0 byte of PLAYER_BYTES_2

} SavedChanges;

#define GOSSIP_SENDER_OPTION 50
#define GOSSIP_SENDER_SUBOPTION 51

#define PLAYER_BYTES_SKIN_COLOR_BYTE 0
#define PLAYER_BYTES_FACE_TYPE_BYTE  1
#define PLAYER_BYTES_HAIR_STYLE_BYTE 2
#define PLAYER_BYTES_HAIR_COLOR_BYTE 3

void SelectAppearanceBytes(Player* player, uint8 idx, int32 change)
{
    uint8 max = player->GetAppearanceMaxIdx(idx);
    int32 cur = player->GetByteValue(PLAYER_BYTES, idx);
    cur += change;

    if (cur > max)
        cur = 0;
    else if (cur < 0)
        cur = max;

    player->SetByteValue(PLAYER_BYTES, idx, cur);
    player->ForceDisplayUpdate();
}

void SelectAppearanceBytes_2(Player* player, int32 change)
{
    uint8 max = player->GetAppearanceMaxIdx_2();
    int32 cur = player->GetByteValue(PLAYER_BYTES_2, PLAYER_BYTES_2_FACIAL_FEATURE_BYTE);
    cur += change;

    if (cur > max)
        cur = 0;
    else if (cur < 0)
        cur = max;

    player->SetByteValue(PLAYER_BYTES_2, PLAYER_BYTES_2_FACIAL_FEATURE_BYTE, cur);
    player->ForceDisplayUpdate();
}

bool Hello_mw_npc_chudomuts(Player* player, Creature* pCreature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(16722), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF + 1);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(16723), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF + 2);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(16724), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF + 3);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(16725), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF + 4);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(16726), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF + 5);
    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
    return true;
}

bool Select_mw_npc_chudomuts(Player* player, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    // bpaction 0 - race change
    // bpaction 1 - gender change
    // bpaction 2 - name change    

    switch (uiAction)
    {
        // I want to change race (alliance)
    case GOSSIP_ACTION_INFO_DEF + 1:
    {
        switch (player->GetClass())
        {
        case CLASS_WARRIOR:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12929), GOSSIP_SENDER_MAIN, 1001 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12930), GOSSIP_SENDER_MAIN, 1004 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12931), GOSSIP_SENDER_MAIN, 1003 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12932), GOSSIP_SENDER_MAIN, 1007 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12933), GOSSIP_SENDER_MAIN, 1011 * 3);
            break;
        }
        case CLASS_PALADIN:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12929), GOSSIP_SENDER_MAIN, 1001 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12931), GOSSIP_SENDER_MAIN, 1003 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12933), GOSSIP_SENDER_MAIN, 1011 * 3);
            break;
        }
        case CLASS_ROGUE:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12929), GOSSIP_SENDER_MAIN, 1001 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12930), GOSSIP_SENDER_MAIN, 1004 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12931), GOSSIP_SENDER_MAIN, 1003 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12932), GOSSIP_SENDER_MAIN, 1007 * 3);
            break;
        }
        case CLASS_PRIEST:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12929), GOSSIP_SENDER_MAIN, 1001 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12930), GOSSIP_SENDER_MAIN, 1004 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12931), GOSSIP_SENDER_MAIN, 1003 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12933), GOSSIP_SENDER_MAIN, 1011 * 3);
            break;
        }
        case CLASS_MAGE:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12929), GOSSIP_SENDER_MAIN, 1001 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12932), GOSSIP_SENDER_MAIN, 1007 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12933), GOSSIP_SENDER_MAIN, 1011 * 3);
            break;
        }
        case CLASS_WARLOCK:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12929), GOSSIP_SENDER_MAIN, 1001 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12932), GOSSIP_SENDER_MAIN, 1007 * 3);
            break;
        }
        case CLASS_HUNTER:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12930), GOSSIP_SENDER_MAIN, 1004 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12931), GOSSIP_SENDER_MAIN, 1003 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12933), GOSSIP_SENDER_MAIN, 1011 * 3);
            break;
        }
        case CLASS_SHAMAN:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12933), GOSSIP_SENDER_MAIN, 1011 * 3);
            break;
        }
        case CLASS_DRUID:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12930), GOSSIP_SENDER_MAIN, 1004 * 3);
            break;
        }
        }

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
        break;
    }
    // I want to change race (horde)
    case GOSSIP_ACTION_INFO_DEF + 2:
    {
        switch (player->GetClass())
        {
        case CLASS_WARRIOR:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12934), GOSSIP_SENDER_MAIN, 1002 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12935), GOSSIP_SENDER_MAIN, 1005 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12936), GOSSIP_SENDER_MAIN, 1006 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12937), GOSSIP_SENDER_MAIN, 1008 * 3);
            break;
        }
        case CLASS_PALADIN:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12938), GOSSIP_SENDER_MAIN, 1010 * 3);
            break;
        }
        case CLASS_ROGUE:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12934), GOSSIP_SENDER_MAIN, 1002 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12935), GOSSIP_SENDER_MAIN, 1005 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12937), GOSSIP_SENDER_MAIN, 1008 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12938), GOSSIP_SENDER_MAIN, 1010 * 3);
            break;
        }
        case CLASS_PRIEST:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12935), GOSSIP_SENDER_MAIN, 1005 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12937), GOSSIP_SENDER_MAIN, 1008 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12938), GOSSIP_SENDER_MAIN, 1010 * 3);
            break;
        }
        case CLASS_MAGE:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12935), GOSSIP_SENDER_MAIN, 1005 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12937), GOSSIP_SENDER_MAIN, 1008 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12938), GOSSIP_SENDER_MAIN, 1010 * 3);
            break;
        }
        case CLASS_WARLOCK:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12934), GOSSIP_SENDER_MAIN, 1002 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12935), GOSSIP_SENDER_MAIN, 1005 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12938), GOSSIP_SENDER_MAIN, 1010 * 3);
            break;
        }
        case CLASS_HUNTER:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12934), GOSSIP_SENDER_MAIN, 1002 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12936), GOSSIP_SENDER_MAIN, 1006 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12937), GOSSIP_SENDER_MAIN, 1008 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12938), GOSSIP_SENDER_MAIN, 1010 * 3);
            break;
        }
        case CLASS_SHAMAN:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12934), GOSSIP_SENDER_MAIN, 1002 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12936), GOSSIP_SENDER_MAIN, 1006 * 3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12937), GOSSIP_SENDER_MAIN, 1008 * 3);
            break;
        }
        case CLASS_DRUID:
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12936), GOSSIP_SENDER_MAIN, 1006 * 3);
            break;
        }
        }

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
        break;
    }

    case GOSSIP_ACTION_INFO_DEF + 3: // change name
    {
        if (!player->HasItemCount(ITEM_TOKEN_NAME_CHANGE, 1))
        {
            ChatHandler(player).PSendSysMessage(16721, player->GetItemLink(ITEM_TOKEN_NAME_CHANGE).c_str());
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        else if (player->HasAtLoginFlag(AT_LOGIN_RENAME))
        {
            ChatHandler(player).PSendSysMessage(12983);
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        int32 bpaction = 2;
        player->CastCustomSpell(player, 55163, NULL, NULL, &bpaction, false);
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 4: // change gender
    {
        if (!player->HasItemCount(ITEM_TOKEN_GENDER_CHANGE, 1))
        {
            ChatHandler(player).PSendSysMessage(16721, player->GetItemLink(ITEM_TOKEN_GENDER_CHANGE).c_str());
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        int32 bpaction = 1;
        player->CastCustomSpell(player, 55163, NULL, &bpaction, NULL, false);
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 5: // change appearance
    {
        // take token if sender is from Main
        if (uiSender == 50)
        {
            if (!player->HasItemCount(ITEM_TOKEN_APPEARANCE_CHANGE, 1))
            {
                ChatHandler(player).PSendSysMessage(16721, player->GetItemLink(ITEM_TOKEN_APPEARANCE_CHANGE).c_str());
                player->CLOSE_GOSSIP_MENU();
                return true;
            }
            else
                player->DestroyItemCount(ITEM_TOKEN_APPEARANCE_CHANGE, 1, true, false, "VENDOR_BUY");

            ChatHandler(player).SendSysMessage(16729);
        }

        if (!player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM))
            player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);

        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15192), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF * 2 + 2);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15185), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF * 2 + 4);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15191), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF * 2 + 6);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15186), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF * 2 + 11);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15187), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF * 2 + 13);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15188), GOSSIP_SENDER_OPTION, GOSSIP_ACTION_INFO_DEF * 2 + 8);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
        break;
    }

    /*All GOSSIP_ACTION_INFO_DEF * 2 + X is Barber*/
    case GOSSIP_ACTION_INFO_DEF * 2 + 2:
        if (uiSender == GOSSIP_SENDER_SUBOPTION)
            SelectAppearanceBytes(player, PLAYER_BYTES_HAIR_STYLE_BYTE, 1);
        // previous - decrease it
    case GOSSIP_ACTION_INFO_DEF * 2 + 3:
        if (uiAction == GOSSIP_ACTION_INFO_DEF * 2 + 3 && uiSender == GOSSIP_SENDER_SUBOPTION)
            SelectAppearanceBytes(player, PLAYER_BYTES_HAIR_STYLE_BYTE, -1);
        // choose options again
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15189), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF * 2 + 2);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15190), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF * 2 + 3);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15188), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF + 5);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
        break;

        // hair color
        // next - increase hair color
    case GOSSIP_ACTION_INFO_DEF * 2 + 4:
        if (uiSender == GOSSIP_SENDER_SUBOPTION)
            SelectAppearanceBytes(player, PLAYER_BYTES_HAIR_COLOR_BYTE, 1);
        // previous - decrease hair color
    case GOSSIP_ACTION_INFO_DEF * 2 + 5:
        if (uiAction == GOSSIP_ACTION_INFO_DEF * 2 + 5 && uiSender == GOSSIP_SENDER_SUBOPTION)
            SelectAppearanceBytes(player, PLAYER_BYTES_HAIR_COLOR_BYTE, -1);
        // choose options again
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15189), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF * 2 + 4);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15190), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF * 2 + 5);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15188), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF + 5);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
        break;

        // facial feature
        // next - increase hair style
    case GOSSIP_ACTION_INFO_DEF * 2 + 6:
        if (uiSender == GOSSIP_SENDER_SUBOPTION)
            SelectAppearanceBytes_2(player, 1);
        // previous - decrease it
    case GOSSIP_ACTION_INFO_DEF * 2 + 7:
        if (uiAction == GOSSIP_ACTION_INFO_DEF * 2 + 7 && uiSender == GOSSIP_SENDER_SUBOPTION)
            SelectAppearanceBytes_2(player, -1);
        // choose options again
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15189), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF * 2 + 6);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15190), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF * 2 + 7);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15188), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF + 5);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
        break;

        // cannot affort
    case GOSSIP_ACTION_INFO_DEF * 2 + 8:
        player->CLOSE_GOSSIP_MENU();
        break;
    case GOSSIP_ACTION_INFO_DEF * 2 + 11:
        if (uiSender == GOSSIP_SENDER_SUBOPTION)
            SelectAppearanceBytes(player, PLAYER_BYTES_SKIN_COLOR_BYTE, 1);
        // previous - decrease it
    case GOSSIP_ACTION_INFO_DEF * 2 + 12:
        if (uiAction == GOSSIP_ACTION_INFO_DEF * 2 + 12 && uiSender == GOSSIP_SENDER_SUBOPTION)
            SelectAppearanceBytes(player, PLAYER_BYTES_SKIN_COLOR_BYTE, -1);
        // choose options again
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15189), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF * 2 + 11);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15190), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF * 2 + 12);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15188), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF + 5);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
        break;
    case GOSSIP_ACTION_INFO_DEF * 2 + 13:
        if (uiSender == GOSSIP_SENDER_SUBOPTION)
            SelectAppearanceBytes(player, PLAYER_BYTES_FACE_TYPE_BYTE, 1);
        // previous - decrease it
    case GOSSIP_ACTION_INFO_DEF * 2 + 14:
        if (uiAction == GOSSIP_ACTION_INFO_DEF * 2 + 14 && uiSender == GOSSIP_SENDER_SUBOPTION)
            SelectAppearanceBytes(player, PLAYER_BYTES_FACE_TYPE_BYTE, -1);
        // choose options again
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15189), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF * 2 + 13);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15190), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF * 2 + 14);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15188), GOSSIP_SENDER_SUBOPTION, GOSSIP_ACTION_INFO_DEF + 5);
        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
        break;
    }

    if (uiAction >= 1001 * 3 && uiAction <= 1011 * 3 && uiAction != 1009 * 3)
    {
        uint8 race = uiAction / 3 - 1000;

        uint32 item;
        switch (race)
        {
        case 1:
        case 4:
        case 3:
        case 7:
        case 11:
            item = ITEM_TOKEN_RACE_CHANGE_ALLIANCE;
            break;
        case 2:
        case 5:
        case 6:
        case 8:
        case 10:
            item = ITEM_TOKEN_RACE_CHANGE_HORDE;
            break;
        }

        if (!item || !player->HasItemCount(item, 1))
        {
            ChatHandler(player).PSendSysMessage(16721, player->GetItemLink(item).c_str());
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        if (player->m_changeRaceTo)
        {
            ChatHandler(player).SendSysMessage(player->GetSession()->GetHellgroundString(12940));
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        if (player->GetRace() != race)
        {
            int32 bpaction = race;
            player->CastCustomSpell(player, 55163, &bpaction, NULL, NULL, false);
            player->CLOSE_GOSSIP_MENU();
        }
        else
        {
            ChatHandler(player).SendSysMessage(player->GetSession()->GetHellgroundString(12939));
            player->CLOSE_GOSSIP_MENU();
        }
    }

    return true;
}

void AddSC_mw_npc_chudomuts()
{
    Script* newscript;
    newscript = new Script;
    newscript->Name = "mw_npc_chudomuts";
    newscript->pGossipHello = &Hello_mw_npc_chudomuts;
    newscript->pGossipSelect = &Select_mw_npc_chudomuts;
    newscript->RegisterSelf();
}