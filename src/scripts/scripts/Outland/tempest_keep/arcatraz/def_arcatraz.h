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

enum Arcatraz
{
    ENCOUNTERS                              = 10,

    TYPE_ZEREKETH                           = 1,
    TYPE_DALLIAH                            = 2,
    TYPE_SOCCOTHRATES                       = 3,
    TYPE_HARBINGERSKYRISS                   = 4,
    TYPE_WARDEN_1                           = 5,
    TYPE_WARDEN_2                           = 6,
    TYPE_WARDEN_3                           = 7,
    TYPE_WARDEN_4                           = 8,
    DATA_MELLICHAR                          = 9,
    DATA_SPHERE_SHIELD                      = 10,
    DATA_POD_A                              = 11,
    DATA_POD_B                              = 12,
    DATA_POD_D                              = 13,
    DATA_POD_G                              = 14,
    DATA_POD_O                              = 15,
    TYPE_SOC_DAL_INTRO                      = 16,
    TYPE_INTROEVENT                         = 17,

    MAX_WARDENS                             = 7,

    NPC_ARCATRAZ_DEFENDER                   = 20857,
    NPC_ARCATRAZ_WARDEN                     = 20859,
    NPC_PROTEAN_NIGHTMARE                   = 20864,
    NPC_PROTEAN_HORROR                      = 20865,
    NPC_SOCCOTHRATES                        = 20886,
    MELLICHAR                               = 20904,    //skyriss will kill this unit

    SAY_SOCCOTHRATES_AGGRO                  = -1552039,
    SAY_SOCCOTHRATES_DEATH                  = -1552043,
    
    CONTAINMENT_CORE_SECURITY_FIELD_ALPHA   = 184318,   //door opened when Wrath-Scryer Soccothrates dies
    CONTAINMENT_CORE_SECURITY_FIELD_BETA    = 184319,   //door opened when Dalliah the Doomsayer dies
    POD_ALPHA                               = 183961,   //pod first boss wave
    POD_BETA                                = 183963,   //pod second boss wave
    POD_DELTA                               = 183964,   //pod third boss wave
    POD_GAMMA                               = 183962,   //pod fourth boss wave
    POD_OMEGA                               = 183965,   //pod fifth boss wave
    SEAL_SPHERE                             = 184802,   //shield 'protecting' mellichar
    
    
};

static const float EntranceMoveLoc[3] = {82.020f, 0.306f, -11.026f};
static const float EntranceSpawnLoc[4] = {173.471f, -0.138f, -10.101f, 3.123f};