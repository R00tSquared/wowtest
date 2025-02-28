// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
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

/* ScriptData
SDName: Dire_Maul
SD%Complete: ?
SDComment: Quest support: 7481, 7482
SDCategory: Dire_Maul
EndScriptData */

/* ContentData
go_kariel_remains
EndContentData */

#include "precompiled.h"

#define QUEST_ELVEN_LEGENDS_HORDE 7481
#define QUEST_ELVEN_LEGENDS_ALLY 7482
bool GOUse_go_kariel_remains(Player *player, GameObject* _GO)
{
    if((player->GetQuestStatus(7481) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(7482) == QUEST_STATUS_INCOMPLETE))
    {
        if(Unit* lydros = FindCreature(14368, 20, player))
            ((Creature*)lydros)->Say(-1200386, LANG_UNIVERSAL, 0);
        player->AreaExploredOrEventHappens(QUEST_ELVEN_LEGENDS_ALLY);
        player->AreaExploredOrEventHappens(QUEST_ELVEN_LEGENDS_HORDE);
    }
    return true;
}

void AddSC_dire_maul()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name="go_kariel_remains";
    newscript->pGOUse =  &GOUse_go_kariel_remains;
    newscript->RegisterSelf();
}