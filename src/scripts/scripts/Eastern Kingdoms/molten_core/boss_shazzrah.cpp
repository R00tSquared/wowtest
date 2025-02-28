// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Shazzrah
SD%Complete: 75
SDComment: Teleport NYI
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "def_molten_core.h"

#define SPELL_ARCANEEXPLOSION           19712
#define SPELL_SHAZZRAHCURSE             19713
#define SPELL_DEADENMAGIC               19714
#define SPELL_COUNTERSPELL              19715

struct boss_shazzrahAI : public ScriptedAI
{
    boss_shazzrahAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    Timer _ChangedNameArcaneExplosion_Timer;
    Timer _ChangedNameShazzrahCurse_Timer;
    Timer _ChangedNameDeadenMagic_Timer;
    Timer _ChangedNameCountspell_Timer;
    Timer _ChangedNameBlink_Timer;

    void Reset()
    {
        _ChangedNameArcaneExplosion_Timer.Reset(6000);                       //These times are probably wrong
        _ChangedNameShazzrahCurse_Timer.Reset(10000);
        _ChangedNameDeadenMagic_Timer.Reset(24000);
        _ChangedNameCountspell_Timer.Reset(15000);
        _ChangedNameBlink_Timer.Reset(30000);

        if (pInstance)
            pInstance->SetData(DATA_SHAZZRAH_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_SHAZZRAH_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_SHAZZRAH_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameArcaneExplosion_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_ARCANEEXPLOSION);
            _ChangedNameArcaneExplosion_Timer = 5000 + rand()%4000;
        }

        if (_ChangedNameShazzrahCurse_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target) DoCast(target,SPELL_SHAZZRAHCURSE);

            _ChangedNameShazzrahCurse_Timer = 25000 + rand()%5000;
        }

        if (_ChangedNameDeadenMagic_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_DEADENMAGIC);
            _ChangedNameDeadenMagic_Timer = 35000;
        }

        if (_ChangedNameCountspell_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_COUNTERSPELL);
            _ChangedNameCountspell_Timer = 16000 + rand()%4000;
        }

        if (_ChangedNameBlink_Timer.Expired(diff))
        {
            // Teleporting him to a random gamer and casting Arcane Explosion after that.
            // Blink is not working cause of LoS System we need to do this hardcoded.
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);

            if(target)
            {
            DoTeleportTo(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
            DoCast(target,SPELL_ARCANEEXPLOSION);
            _ChangedNameArcaneExplosion_Timer.Reset(5000 + rand()%4000);
            DoResetThreat();
            }

            _ChangedNameBlink_Timer = 45000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_shazzrah(Creature *_Creature)
{
    return new boss_shazzrahAI (_Creature);
}

void AddSC_boss_shazzrah()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_shazzrah";
    newscript->GetAI = &GetAI_boss_shazzrah;
    newscript->RegisterSelf();
}


