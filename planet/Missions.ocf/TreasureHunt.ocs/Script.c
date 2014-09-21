/**
	Treasure Hunt
	Find the treasure and swap it for a barrel of oil
	
	@authors Sven2
*/

static g_is_initialized; // set after first player join
static g_max_player_num; // max number of players that were ever joined
static g_plr_inventory; // array indexed by players: Array containing inventory of Clonk just before it died

static npc_pyrit, npc_dagobert, npc_tarzan;
static g_got_gem_task, g_got_oil, g_goal, g_treasure_collected;

func Initialize()
{
	SetNextMission("Mission.ocf/DarkCastle.ocs");
	return true;
}

func DoInit(int first_player)
{
	var flagpole = CreateObject(Flagpole, 210,1185, first_player);
	ClearFreeRect(530,1135, 50,2);
	// Start Intro.
	StartSequence("Intro", 0, flagpole);
	return true;
}

func InitializePlayer(int plr)
{
	// Players only
	if (GetPlayerType(plr)!=C4PT_User) return;
	// Scenario init
	if (!g_is_initialized) g_is_initialized = DoInit(plr);
	// Harsh zoom range
	for (var flag in [PLRZOOM_LimitMax, PLRZOOM_Direct])
		SetPlayerZoomByViewRange(plr,400,250,flag);
	SetPlayerViewLock(plr, true);
	// Create per-player-counted tools
	if (g_max_player_num < GetPlayerCount(C4PT_User))
	{
		++g_max_player_num;
		for (var obj in FindObjects(Find_ID(Chest)))
			if (obj.tool_spawn)
				obj->CreateContents(obj.tool_spawn);
	}
	// Initial join
	JoinPlayer(plr);
	GetCrew(plr)->CreateContents(Shovel);
	return true;
}

func RelaunchPlayer(int plr)
{
	var clonk = CreateObject(Clonk, 200, 1175, plr);
	clonk->MakeCrewMember(plr);
	SetCursor(plr, clonk);
	JoinPlayer(plr);
	// Recover carried objects
	// Do not recover pipes, because that would draw ugly lines across the landscape
	if (g_plr_inventory && g_plr_inventory[plr])
	{
		for (var obj in g_plr_inventory[plr])
			if (obj && obj->GetID() != Pipe) obj->Enter(clonk);
		g_plr_inventory[plr] = nil;
	}
	return true;
}

func JoinPlayer(int plr)
{
	// Place in village
	var crew;
	for(var index = 0; crew = GetCrew(plr, index); ++index)
	{
		var x = 190 + Random(20);
		var y = 1175;
		crew->SetPosition(x , y);
		crew->SetDir(DIR_Right);
		crew->DoEnergy(1000);
		AddEffect("IntRememberInventory", crew, 1, 0);
	}
	return true;
}

global func FxIntRememberInventoryStop(object clonk, fx, int reason, bool temp)
{
	if (!temp && reason == FX_Call_RemoveDeath)
	{
		var plr = clonk->GetOwner();
		if (plr != NO_OWNER)
		{
			if (!g_plr_inventory) g_plr_inventory = [];
			g_plr_inventory[plr] = [];
			var i=0,obj;
			while (obj=clonk->Contents(i)) g_plr_inventory[plr][i++] = obj;
		}
	}
	return FX_OK;

}


/* Enemy encounter messages */

func EncounterCastle(object enemy, object player)
{
	Dialogue->MessageBoxAll("$MsgEncounterCastle$", enemy, true);
	return true;
}

func EncounterFinal(object enemy, object player)
{
	Dialogue->MessageBoxAll("$MsgEncounterFinal$", enemy, true);
	return true;
}


/* Events */

func OnTreasureCollected(object treasure)
{
	g_treasure_collected = true;
	Dialogue->MessageBoxAll("$MsgTreasureCollected$", treasure->Contained(), true);
	// Dagobert has something new to say now
	if (npc_dagobert)
	{
		var dlg = Dialogue->FindByTarget(npc_dagobert);
		if (dlg) dlg->AddAttention();
	}
	return true;
}

func OnPlaneLoaded(object plane, object oil)
{
	if (!plane || !oil) return false; // disappeared in that one frame?
	oil->Enter(plane);
	g_goal->OnOilDelivered();
	return StartSequence("Outro", 0, plane);
}

static g_num_goldbars;
static const MAX_GOLD_BARS = 20;

func OnGoldBarCollected(object collector)
{
	++g_num_goldbars;
	UpdateLeagueScores();
	Dialogue->MessageBoxAll(Format("$MsgGoldBarCollected$", g_num_goldbars, MAX_GOLD_BARS), collector, true);
	return true;
}

public func OnGoalsFulfilled()
{
	GainMissionAccess("S2Treasure");
	UpdateLeagueScores();
	// Return true to force goal rule to not call GameOver() yet, as it will be done by outro sequence
	return true;
}

func OnGameOver()
{
	// In case gems are collected after game end.
	UpdateLeagueScores();
	return true;
}

func UpdateLeagueScores()
{
	// +50 for finishing and +5 for every gold bar
	var goal_finished = (g_goal && g_goal->IsFulfilled());
	return SetLeagueProgressScore(g_num_goldbars, g_num_goldbars * 5 + goal_finished * 50);
}

func OnInvincibleDamage(object damaged_target)
{
	// Closest Clonk remarks that the door is invincible
	if (damaged_target && damaged_target->GetID() == StoneDoor)
	{
		var observer = damaged_target->FindObject(Find_ID(Clonk), Find_OCF(OCF_Alive), damaged_target->Sort_Distance());
		if (observer)
		{
			Dialogue->MessageBoxAll("$MsgStoneDoorNoDamage$", observer, true);
		}
	}
	return true;
}

