/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#include "ScriptMgr.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "GameObject.h"
#include "InstanceScript.h"
#include "Map.h"
#include "shadow_labyrinth.h"

DoorData const doorData[] =
{
    { GO_REFECTORY_DOOR,        DATA_BLACKHEART_THE_INCITER,    DOOR_TYPE_PASSAGE },
    { GO_SCREAMING_HALL_DOOR,   DATA_GRANDMASTER_VORPIL,        DOOR_TYPE_PASSAGE },
    { 0,                        0,                              DOOR_TYPE_ROOM } // END
};

class instance_shadow_labyrinth : public InstanceMapScript
{
    public:
        instance_shadow_labyrinth() : InstanceMapScript(SLScriptName, 555) { }

        struct instance_shadow_labyrinth_InstanceMapScript : public InstanceScript
        {
            instance_shadow_labyrinth_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
            {
                SetHeaders(DataHeader);
                SetBossNumber(EncounterCount);
                LoadDoorData(doorData);

                FelOverseerCount      = 0;
            }

            void OnCreatureCreate(Creature* creature) override
            {
                switch (creature->GetEntry())
                {
                    case NPC_AMBASSADOR_HELLMAW:
                        AmbassadorHellmawGUID = creature->GetGUID();
                        break;
                    case NPC_GRANDMASTER_VORPIL:
                        GrandmasterVorpilGUID = creature->GetGUID();
                        break;
                    case NPC_FEL_OVERSEER:
                        if (creature->IsAlive())
                        {
                            ++FelOverseerCount;
                            if (Creature* hellmaw = instance->GetCreature(AmbassadorHellmawGUID))
                                hellmaw->AI()->DoAction(ACTION_AMBASSADOR_HELLMAW_BANISH);
                        }
                        break;
                    default:
                        break;
                }
            }

            void OnGameObjectCreate(GameObject* go) override
            {
                switch (go->GetEntry())
                {
                    case GO_REFECTORY_DOOR:
                    case GO_SCREAMING_HALL_DOOR:
                        AddDoor(go, true);
                        break;
                    default:
                        break;
                }
            }

            void OnGameObjectRemove(GameObject* go) override
            {
                switch (go->GetEntry())
                {
                    case GO_REFECTORY_DOOR:
                    case GO_SCREAMING_HALL_DOOR:
                        AddDoor(go, false);
                        break;
                    default:
                        break;
                }
            }

            void OnUnitDeath(Unit* unit) override
            {
                Creature* creature = unit->ToCreature();
                if (!creature)
                    return;

                if (creature->GetEntry() == NPC_FEL_OVERSEER)
                {
                    if (FelOverseerCount)
                        --FelOverseerCount;

                    if (!FelOverseerCount)
                        if (Creature* hellmaw = instance->GetCreature(AmbassadorHellmawGUID))
                            hellmaw->AI()->DoAction(ACTION_AMBASSADOR_HELLMAW_INTRO);
                }
            }

            uint32 GetData(uint32 type) const override
            {
                switch (type)
                {
                    case DATA_FEL_OVERSEER:
                        return !FelOverseerCount ? 1 : 0;
                    default:
                        break;
                }
                return 0;
            }

            ObjectGuid GetGuidData(uint32 type) const override
            {
                switch (type)
                {
                    case DATA_GRANDMASTER_VORPIL:
                        return GrandmasterVorpilGUID;
                    default:
                        break;
                }
                return ObjectGuid::Empty;
            }

        protected:
            ObjectGuid AmbassadorHellmawGUID;
            ObjectGuid GrandmasterVorpilGUID;
            uint32 FelOverseerCount;
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const override
        {
            return new instance_shadow_labyrinth_InstanceMapScript(map);
        }
};

void AddSC_instance_shadow_labyrinth()
{
    new instance_shadow_labyrinth();
}
