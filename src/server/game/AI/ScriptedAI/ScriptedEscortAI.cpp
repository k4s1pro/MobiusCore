/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

/* ScriptData
SDName: Npc_EscortAI
SD%Complete: 100
SDComment:
SDCategory: Npc
EndScriptData */

#include "ScriptedEscortAI.h"
#include "Creature.h"
#include "Group.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"

enum Points
{
    POINT_LAST_POINT    = 0xFFFFFF,
    POINT_HOME          = 0xFFFFFE
};

npc_escortAI::npc_escortAI(Creature* creature) : ScriptedAI(creature),
    m_uiWPWaitTimer(2500),
    m_uiPlayerCheckTimer(1000),
    m_uiEscortState(STATE_ESCORT_NONE),
    MaxPlayerDistance(DEFAULT_MAX_PLAYER_DISTANCE),
    m_pQuestForEscort(NULL),
    m_bIsActiveAttacker(true),
    m_bIsRunning(false),
    m_bCanInstantRespawn(false),
    m_bCanReturnToStart(false),
    DespawnAtEnd(true),
    DespawnAtFar(true),
    ScriptWP(false),
    HasImmuneToNPCFlags(false)
{ }

void npc_escortAI::AttackStart(Unit* who)
{
    if (!who)
        return;

    if (me->Attack(who, true))
    {
        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
            me->GetMotionMaster()->MovementExpired();

        if (IsCombatMovementAllowed())
            me->GetMotionMaster()->MoveChase(who);
    }
}

Player* npc_escortAI::GetPlayerForEscort()
{
    return ObjectAccessor::GetPlayer(*me, m_uiPlayerGUID);
}

//see followerAI
bool npc_escortAI::AssistPlayerInCombatAgainst(Unit* who)
{
    if (!who || !who->GetVictim())
        return false;

    //experimental (unknown) flag not present
    if (!(me->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_CAN_ASSIST))
        return false;

    //not a player
    if (!who->EnsureVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
        return false;

    //never attack friendly
    if (me->IsFriendlyTo(who))
        return false;

    //too far away and no free sight?
    if (me->IsWithinDistInMap(who, GetMaxPlayerDistance()) && me->IsWithinLOSInMap(who))
    {
        //already fighting someone?
        if (!me->GetVictim())
        {
            AttackStart(who);
            return true;
        }
        else
        {
            who->SetInCombatWith(me);
            me->AddThreat(who, 0.0f);
            return true;
        }
    }

    return false;
}

void npc_escortAI::MoveInLineOfSight(Unit* who)
{
    if (me->HasReactState(REACT_AGGRESSIVE) && !me->HasUnitState(UNIT_STATE_STUNNED) && who->isTargetableForAttack() && who->isInAccessiblePlaceFor(me))
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING) && AssistPlayerInCombatAgainst(who))
            return;

        if (!me->CanFly() && me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
            return;

        if (me->IsHostileTo(who))
        {
            float fAttackRadius = me->GetAttackDistance(who);
            if (me->IsWithinDistInMap(who, fAttackRadius) && me->IsWithinLOSInMap(who))
            {
                if (!me->GetVictim())
                {
                    // Clear distracted state on combat
                    if (me->HasUnitState(UNIT_STATE_DISTRACTED))
                    {
                        me->ClearUnitState(UNIT_STATE_DISTRACTED);
                        me->GetMotionMaster()->Clear();
                    }

                    AttackStart(who);
                }
                else if (me->GetMap()->IsDungeon())
                {
                    who->SetInCombatWith(me);
                    me->AddThreat(who, 0.0f);
                }
            }
        }
    }
}

void npc_escortAI::JustDied(Unit* /*killer*/)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING) || !m_uiPlayerGUID || !m_pQuestForEscort)
        return;

    if (Player* player = GetPlayerForEscort())
    {
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* groupRef = group->GetFirstMember(); groupRef != NULL; groupRef = groupRef->next())
                if (Player* member = groupRef->GetSource())
                    if (member->IsInMap(player))
                        member->FailQuest(m_pQuestForEscort->GetQuestId());
        }
        else
            player->FailQuest(m_pQuestForEscort->GetQuestId());
    }
}

void npc_escortAI::JustRespawned()
{
    m_uiEscortState = STATE_ESCORT_NONE;

    if (!IsCombatMovementAllowed())
        SetCombatMovement(true);

    //add a small delay before going to first waypoint, normal in near all cases
    m_uiWPWaitTimer = 2500;

    if (me->getFaction() != me->GetCreatureTemplate()->faction)
        me->RestoreFaction();

    Reset();
}

void npc_escortAI::ReturnToLastPoint()
{
    float x, y, z, o;
    me->GetHomePosition(x, y, z, o);
    me->GetMotionMaster()->MovePoint(POINT_LAST_POINT, x, y, z);
}

void npc_escortAI::EnterEvadeMode(EvadeReason /*why*/)
{
    me->RemoveAllAuras();
    me->DeleteThreatList();
    me->CombatStop(true);
    me->SetLootRecipient(NULL);

    if (HasEscortState(STATE_ESCORT_ESCORTING))
    {
        AddEscortState(STATE_ESCORT_RETURNING);
        ReturnToLastPoint();
        LOG_DEBUG("scripts", "EscortAI has left combat and is now returning to last point");
    }
    else
    {
        me->GetMotionMaster()->MoveTargetedHome();
        if (HasImmuneToNPCFlags)
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        Reset();
    }
}

bool npc_escortAI::IsPlayerOrGroupInRange()
{
    if (Player* player = GetPlayerForEscort())
    {
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* groupRef = group->GetFirstMember(); groupRef != NULL; groupRef = groupRef->next())
                if (Player* member = groupRef->GetSource())
                    if (me->IsWithinDistInMap(member, GetMaxPlayerDistance()))
                        return true;
        }
        else if (me->IsWithinDistInMap(player, GetMaxPlayerDistance()))
            return true;
    }

    return false;
}

void npc_escortAI::UpdateAI(uint32 diff)
{
    //Waypoint Updating
    if (HasEscortState(STATE_ESCORT_ESCORTING) && !me->GetVictim() && m_uiWPWaitTimer && !HasEscortState(STATE_ESCORT_RETURNING))
    {
        if (m_uiWPWaitTimer <= diff)
        {
            //End of the line
            if (CurrentWP == WaypointList.end())
            {
                if (DespawnAtEnd)
                {
                    LOG_DEBUG("scripts", "EscortAI reached end of waypoints");

                    if (m_bCanReturnToStart)
                    {
                        float fRetX, fRetY, fRetZ;
                        me->GetRespawnPosition(fRetX, fRetY, fRetZ);

                        me->GetMotionMaster()->MovePoint(POINT_HOME, fRetX, fRetY, fRetZ);

                        m_uiWPWaitTimer = 0;

                        LOG_DEBUG("scripts", "EscortAI are returning home to spawn location: %u, %f, %f, %f", POINT_HOME, fRetX, fRetY, fRetZ);
                        return;
                    }

                    if (m_bCanInstantRespawn)
                    {
                        me->setDeathState(JUST_DIED);
                        me->Respawn();
                    }
                    else
                        me->DespawnOrUnsummon();

                    return;
                }
                else
                {
                    LOG_DEBUG("scripts", "EscortAI reached end of waypoints with Despawn off");

                    return;
                }
            }

            if (!HasEscortState(STATE_ESCORT_PAUSED))
            {
                me->GetMotionMaster()->MovePoint(CurrentWP->id, CurrentWP->x, CurrentWP->y, CurrentWP->z);
                LOG_DEBUG("scripts", "EscortAI start waypoint %u (%f, %f, %f).", CurrentWP->id, CurrentWP->x, CurrentWP->y, CurrentWP->z);

                WaypointStart(CurrentWP->id);

                m_uiWPWaitTimer = 0;
            }
        }
        else
            m_uiWPWaitTimer -= diff;
    }

    //Check if player or any member of his group is within range
    if (HasEscortState(STATE_ESCORT_ESCORTING) && !m_uiPlayerGUID.IsEmpty() && !me->GetVictim() && !HasEscortState(STATE_ESCORT_RETURNING))
    {
        if (m_uiPlayerCheckTimer <= diff)
        {
            if (DespawnAtFar && !IsPlayerOrGroupInRange())
            {
                LOG_DEBUG("scripts", "EscortAI failed because player/group was to far away or not found");

                if (m_bCanInstantRespawn)
                {
                    me->setDeathState(JUST_DIED);
                    me->Respawn();
                }
                else
                    me->DespawnOrUnsummon();

                return;
            }

            m_uiPlayerCheckTimer = 1000;
        }
        else
            m_uiPlayerCheckTimer -= diff;
    }

    UpdateEscortAI(diff);
}

void npc_escortAI::UpdateEscortAI(uint32 /*diff*/)
{
    if (!UpdateVictim())
        return;

    DoMeleeAttackIfReady();
}

void npc_escortAI::MovementInform(uint32 moveType, uint32 pointId)
{
    if (moveType != POINT_MOTION_TYPE || !HasEscortState(STATE_ESCORT_ESCORTING))
        return;

    //Combat start position reached, continue waypoint movement
    if (pointId == POINT_LAST_POINT)
    {
        LOG_DEBUG("scripts", "EscortAI has returned to original position before combat");

        me->SetWalk(!m_bIsRunning);
        RemoveEscortState(STATE_ESCORT_RETURNING);

        if (!m_uiWPWaitTimer)
            m_uiWPWaitTimer = 1;
    }
    else if (pointId == POINT_HOME)
    {
        LOG_DEBUG("scripts", "EscortAI has returned to original home location and will continue from beginning of waypoint list.");

        CurrentWP = WaypointList.begin();
        m_uiWPWaitTimer = 1;
    }
    else
    {
        //Make sure that we are still on the right waypoint
        if (CurrentWP->id != pointId)
        {
            LOG_ERROR("misc", "TSCR ERROR: EscortAI reached waypoint out of order %u, expected %u, creature entry %u", pointId, CurrentWP->id, me->GetEntry());
            return;
        }

        LOG_DEBUG("scripts", "EscortAI Waypoint %u reached", CurrentWP->id);

        //Call WP function
        WaypointReached(CurrentWP->id);

        m_uiWPWaitTimer = CurrentWP->WaitTimeMs + 1;

        ++CurrentWP;
    }
}

/*
void npc_escortAI::OnPossess(bool apply)
{
    // We got possessed in the middle of being escorted, store the point
    // where we left off to come back to when possess is removed
    if (HasEscortState(STATE_ESCORT_ESCORTING))
    {
        if (apply)
            me->GetPosition(LastPos.x, LastPos.y, LastPos.z);
        else
        {
            Returning = true;
            me->GetMotionMaster()->MovementExpired();
            me->GetMotionMaster()->MovePoint(WP_LAST_POINT, LastPos.x, LastPos.y, LastPos.z);
        }
    }
}
*/

void npc_escortAI::AddWaypoint(uint32 id, float x, float y, float z, uint32 waitTime)
{
    Escort_Waypoint t(id, x, y, z, waitTime);

    WaypointList.push_back(t);

    // i think SD2 no longer uses this function
    ScriptWP = true;
    /*PointMovement wp;
    wp.m_uiCreatureEntry = me->GetEntry();
    wp.m_uiPointId = id;
    wp.m_fX = x;
    wp.m_fY = y;
    wp.m_fZ = z;
    wp.m_uiWaitTime = WaitTimeMs;
    PointMovementMap[wp.m_uiCreatureEntry].push_back(wp);*/
}

void npc_escortAI::FillPointMovementListForCreature()
{
    ScriptPointVector const* movePoints = sScriptSystemMgr->GetPointMoveList(me->GetEntry());
    if (!movePoints)
        return;

    for (ScriptPointVector::const_iterator itr = movePoints->begin(); itr != movePoints->end(); ++itr)
    {
        Escort_Waypoint point(itr->uiPointId, itr->fX, itr->fY, itr->fZ, itr->uiWaitTime);
        WaypointList.push_back(point);
    }
}

void npc_escortAI::SetRun(bool on)
{
    if (on)
    {
        if (!m_bIsRunning)
            me->SetWalk(false);
        else
            LOG_DEBUG("scripts", "EscortAI attempt to set run mode, but is already running.");
    }
    else
    {
        if (m_bIsRunning)
            me->SetWalk(true);
        else
            LOG_DEBUG("scripts", "EscortAI attempt to set walk mode, but is already walking.");
    }

    m_bIsRunning = on;
}

/// @todo get rid of this many variables passed in function.
void npc_escortAI::Start(bool isActiveAttacker /* = true*/, bool run /* = false */, ObjectGuid playerGUID /* = 0 */, Quest const* quest /* = NULL */, bool instantRespawn /* = false */, bool canLoopPath /* = false */, bool resetWaypoints /* = true */)
{
    if (me->GetVictim())
    {
        LOG_ERROR("scripts.escortai", "TSCR ERROR: EscortAI (script: %s, creature entry: %u) attempts to Start while in combat", me->GetScriptName().c_str(), me->GetEntry());
        return;
    }

    if (HasEscortState(STATE_ESCORT_ESCORTING))
    {
        LOG_ERROR("scripts.escortai", "EscortAI (script: %s, creature entry: %u) attempts to Start while already escorting", me->GetScriptName().c_str(), me->GetEntry());
        return;
    }

    if (!ScriptWP && resetWaypoints) // sd2 never adds wp in script, but tc does
    {
        if (!WaypointList.empty())
            WaypointList.clear();
        FillPointMovementListForCreature();
    }

    if (WaypointList.empty())
    {
        LOG_ERROR("scripts", "EscortAI (script: %s, creature entry: %u) starts with 0 waypoints (possible missing entry in script_waypoint. Quest: %u).",
            me->GetScriptName().c_str(), me->GetEntry(), quest ? quest->GetQuestId() : 0);
        return;
    }

    //set variables
    m_bIsActiveAttacker = isActiveAttacker;
    m_bIsRunning = run;

    m_uiPlayerGUID = playerGUID;
    m_pQuestForEscort = quest;

    m_bCanInstantRespawn = instantRespawn;
    m_bCanReturnToStart = canLoopPath;

    if (m_bCanReturnToStart && m_bCanInstantRespawn)
        LOG_DEBUG("scripts", "EscortAI is set to return home after waypoint end and instant respawn at waypoint end. Creature will never despawn.");

    if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
    {
        me->GetMotionMaster()->MovementExpired();
        me->GetMotionMaster()->MoveIdle();
        LOG_DEBUG("scripts", "EscortAI start with WAYPOINT_MOTION_TYPE, changed to MoveIdle.");
    }

    //disable npcflags
    me->SetUInt64Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC))
    {
        HasImmuneToNPCFlags = true;
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
    }

    LOG_DEBUG("scripts", "EscortAI started with " UI64FMTD " waypoints. ActiveAttacker = %d, Run = %d, %s", uint64(WaypointList.size()), m_bIsActiveAttacker, m_bIsRunning, m_uiPlayerGUID.ToString().c_str());

    CurrentWP = WaypointList.begin();

    //Set initial speed
    if (m_bIsRunning)
        me->SetWalk(false);
    else
        me->SetWalk(true);

    AddEscortState(STATE_ESCORT_ESCORTING);
}

void npc_escortAI::SetEscortPaused(bool on)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING))
        return;

    if (on)
        AddEscortState(STATE_ESCORT_PAUSED);
    else
        RemoveEscortState(STATE_ESCORT_PAUSED);
}

bool npc_escortAI::SetNextWaypoint(uint32 pointId, float x, float y, float z, float orientation)
{
    me->UpdatePosition(x, y, z, orientation);
    return SetNextWaypoint(pointId, false, true);
}

bool npc_escortAI::SetNextWaypoint(uint32 pointId, bool setPosition, bool resetWaypointsOnFail)
{
    if (!WaypointList.empty())
        WaypointList.clear();

    FillPointMovementListForCreature();

    if (WaypointList.empty())
        return false;

    size_t const size = WaypointList.size();
    Escort_Waypoint waypoint(0, 0, 0, 0, 0);
    do
    {
        waypoint = WaypointList.front();
        WaypointList.pop_front();
        if (waypoint.id == pointId)
        {
            if (setPosition)
                me->UpdatePosition(waypoint.x, waypoint.y, waypoint.z, me->GetOrientation());

            CurrentWP = WaypointList.begin();
            return true;
        }
    }
    while (!WaypointList.empty());

    // we failed.
    // we reset the waypoints in the start; if we pulled any, reset it again
    if (resetWaypointsOnFail && size != WaypointList.size())
    {
        if (!WaypointList.empty())
            WaypointList.clear();

        FillPointMovementListForCreature();
    }

    return false;
}

bool npc_escortAI::GetWaypointPosition(uint32 pointId, float& x, float& y, float& z)
{
    ScriptPointVector const* waypoints = sScriptSystemMgr->GetPointMoveList(me->GetEntry());
    if (!waypoints)
        return false;

    for (ScriptPointVector::const_iterator itr = waypoints->begin(); itr != waypoints->end(); ++itr)
    {
        if (itr->uiPointId == pointId)
        {
            x = itr->fX;
            y = itr->fY;
            z = itr->fZ;
            return true;
        }
    }

    return false;
}
