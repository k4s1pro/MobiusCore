/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#ifndef OUTDOOR_PVP_SI_
#define OUTDOOR_PVP_SI_

#include "OutdoorPvP.h"

enum OutdoorPvPSISpells
{
    SI_SILITHYST_FLAG_GO_SPELL = 29518,
    SI_SILITHYST_FLAG = 29519,
    SI_TRACES_OF_SILITHYST = 29534,
    SI_CENARION_FAVOR = 30754
};

const uint32 SI_MAX_RESOURCES = 200;

const uint8 OutdoorPvPSIBuffZonesNum = 3;

const uint32 OutdoorPvPSIBuffZones[OutdoorPvPSIBuffZonesNum] = { 1377, 3428, 3429 };

const uint32 SI_AREATRIGGER_H = 4168;

const uint32 SI_AREATRIGGER_A = 4162;

const uint32 SI_TURNIN_QUEST_CM_A = 17090;

const uint32 SI_TURNIN_QUEST_CM_H = 18199;

const uint32 SI_SILITHYST_MOUND = 181597;

enum SI_WorldStates
{
    SI_GATHERED_A = 2313,
    SI_GATHERED_H = 2314,
    SI_SILITHYST_MAX = 2317
};

class OutdoorPvPSI : public OutdoorPvP
{
    public:
        OutdoorPvPSI();

        bool SetupOutdoorPvP() override;

        void HandlePlayerEnterZone(Player* player, uint32 zone) override;
        void HandlePlayerLeaveZone(Player* player, uint32 zone) override;

        bool Update(uint32 diff) override;

        void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;

        void SendRemoveWorldStates(Player* player) override;

        bool HandleAreaTrigger(Player* player, uint32 trigger, bool entered) override;

        bool HandleDropFlag(Player* player, uint32 spellId) override;

        bool HandleCustomSpell(Player* player, uint32 spellId, GameObject* go) override;

        void UpdateWorldState();

    private:
        uint32 m_Gathered_A;
        uint32 m_Gathered_H;

        uint32 m_LastController;
};

#endif
