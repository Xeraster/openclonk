/*-- 
		Parkour 
		Authors: Maikel
		
		The goal is to be the first to reach the finish, the team or player to do so wins the round.
		Checkpoints can be added to make the path more interesting and more complex.
		Checkpoints can have different functionalities:
			* Respawn: On/Off - The clonk respawns at the last passed checkpoint.
			* Check: On/Off - The clonk must pass through these checkpoints before being able to finish.
			* Ordered: On/Off - The checkpoints mussed be passed in the order specified.
			* The start and finish are also checkpoints.
			
		TODO: 
			* Update CP Graphics -> looks satisfactory atm but cpu intensive.
			* Add significant message under goal, done.
--*/


#include Library_Goal

local finished; // Whether the goal has been reached by some player.
local cp_list; // List of checkpoints.
local cp_count; // Number of checkpoints.
local respawn_list; // List of last reached respawn CP per player.
local plr_list; // Number of checkpoints the player completed.
local team_list; // Number of checkpoints the team completed.
local time_store; // String for best time storage in player file.

/*-- General --*/

protected func Initialize()
{
	finished = false;
	cp_list = [];
	cp_count = 0;
	respawn_list = [];
	plr_list = [];
	team_list = [];
	// Best time tracking.
	time_store = Format("Parkour_%s_BestTime", GetScenTitle());
	AddEffect("IntBestTime", this, 100, 1, this);
	// Activate restart rule, if there isn't any.
	if (!ObjectCount(Find_ID(Rule_Restart)))
		CreateObject(Rule_Restart, 0, 0, NO_OWNER);
	// Scoreboard.
	InitScoreboard();
	return _inherited(...);
}

/*-- Checkpoint creation --*/

public func SetStartpoint(int x, int y)
{
	var cp = CreateObject(ParkourCheckpoint, x, y, NO_OWNER);
	cp->SetPosition(x, y);
	cp->SetCPMode(PARKOUR_CP_Start);
	cp->SetCPController(this);
	cp_list[0] = cp;
	return;
}

public func SetFinishpoint(int x, int y)
{
	var cp = CreateObject(ParkourCheckpoint, x, y, NO_OWNER);
	cp->SetPosition(x, y);
	cp->SetCPMode(PARKOUR_CP_Finish);
	cp->SetCPController(this);
	cp_count++;
	cp_list[cp_count] = cp;
	return;
}

public func AddCheckpoint(int x, int y, int mode)
{
	var cp = CreateObject(ParkourCheckpoint, x, y, NO_OWNER);
	cp->SetPosition(x, y);
	cp->SetCPMode(mode);
	cp->SetCPController(this);
	if (mode & PARKOUR_CP_Check || mode & PARKOUR_CP_Ordered)
	{
		cp_count++;
		cp_list[cp_count + 1] = cp_list[cp_count]; // Finish 1 place further.
		cp_list[cp_count] = cp;
	}
	return;
}

/*-- Checkpoint interaction --*/

// Called from a finish CP to indicate that plr has reached it.
public func PlrReachedFinishCP(int plr, object cp)
{
	if (finished)
		return;
	var plrid = GetPlayerID(plr);
	plr_list[plrid]++;
	if (GetPlayerTeam(plr))
		team_list[GetPlayerTeam(plr)]++;
	UpdateScoreboard(plr);
	DoBestTime(plr);
	SetEvalData(plr);
	EliminatePlayers(plr);
	finished = true;
	return;
}

// Called from a respawn CP to indicate that plr has reached it.
public func SetPlrRespawnCP(int plr, object cp)
{
	if (respawn_list[plr] == cp)
		return;
	respawn_list[plr] = cp;
	cp->PlayerMessage(plr, "$MsgNewRespawn$");
	return;
}

// Called from a check CP to indicate that plr has cleared it.
public func AddPlrClearedCP(int plr, object cp)
{
	if (finished)
		return;
	var plrid = GetPlayerID(plr);
	var team = GetPlayerTeam(plr);
	plr_list[plrid]++;
	if (team)
		team_list[team]++;
	UpdateScoreboard(plr);
	return;
}

/*-- Goal interface --*/

// Eliminates all players apart from the winner and his team.
private func EliminatePlayers(int winner)
{
	var winteam = GetPlayerTeam(winner);
	for (var i = 0; i < GetPlayerCount(); i++)
	{
		var plr = GetPlayerByIndex(i);
		var team = GetPlayerTeam(plr);
		if (plr == winner) // The winner self.
			continue;
		if (team && team == winteam) // In the same team as the winner.
			continue;
		EliminatePlayer(plr);
	}
	return;
}

public func IsFulfilled()
{
	return finished;
}

public func Activate(int plr)
{
	var team = GetPlayerTeam(plr);
	var msg;
	if (finished)
	{
		if (team)
		{
			if (IsWinner(plr))
				msg = "$MsgRaceWonTeam$";
			else
				msg = "$MsgRaceLostTeam$";
		}
		else
		{
			if (IsWinner(plr))
				msg = "$MsgRaceWon$";
			else
				msg = "$MsgRaceLost$";
		}
	}
	else
		msg = Format("$MsgRace$", cp_count);
	// Show goal message.
	MessageWindow(msg, plr);
	return;
}

public func GetShortDescription(int plr)
{
	var team = GetPlayerTeam(plr);
	var msg, pos;
	if (team)
	{
		pos = GetTeamPosition(team);
		if (pos == 1)
			msg = "$MsgDescTeamFirst$";
		if (pos == 2)
			msg = "$MsgDescTeamSecond$";	
		if (pos == 3)
			msg = "$MsgDescTeamThird$";
		if (pos >= 4)
			msg = Format("$MsgDescTeamNth$", pos);
	}
	else
	{
		pos = GetPlayerPosition(plr);
		if (pos == 1)
			msg = "$MsgDescFirst$";
		if (pos == 2)
			msg = "$MsgDescSecond$";	
		if (pos == 3)
			msg = "$MsgDescThird$";
		if (pos >= 4)
			msg = Format("$MsgDescNth$", pos);	
	}	
	return msg;
}

private func GetPlayerPosition(int plr)
{
	var pos = 1;
	var plrid = GetPlayerID(plr); 
	for (var i = 0; i < GetPlayerCount(); i++)
		if (plr_list[plrid] < plr_list[GetPlayerID(GetPlayerByIndex(i))])
			pos++;
	return pos;
}

private func GetTeamPosition(int team)
{
	var pos = 1;
	for (var i = 0; i < GetTeamCount(); i++)
		if (team_list[team] < team_list[GetTeamByIndex(i)])
			pos++;
	return pos;
}

private func IsWinner(int plr)
{
	var team = GetPlayerTeam(plr);
	var finish = FindObject(Find_ID(ParkourCheckpoint), Find_Func("FindCPMode", PARKOUR_CP_Finish));
	if (!finish)
		return false;
	if (team)
	{
		if (finish->ClearedByTeam(team))
			return true;
	}
	else
	{
		if (finish->ClearedByPlr(plr))
			return true;
	}
	return false;
}

/*-- Player section --*/

protected func InitializePlayer(int plr, int x, int y, object base, int team)
{
	// If the parkour is already finished, then immediately eliminate player.
	if (finished)
		return EliminatePlayer(plr);
	// Remove all hostilities.
	for (var i = 0; i < GetPlayerCount(); i++)
	{
		SetHostility(plr, GetPlayerByIndex(i), false, true);
		SetHostility(GetPlayerByIndex(i), plr, false, true);
	}
	// Init Respawn CP to start CP.
	var plrid = GetPlayerID(plr);
	respawn_list[plr] = cp_list[0];
	plr_list[plrid] = 0;
	if (team)
		if (!team_list[team])
			team_list[team] = 0;
	// Scoreboard.
	InitScoreboard();
	UpdateScoreboard(plr);
	DoScoreboardShow(1, plr + 1);
	JoinPlayer(plr);
	// Scenario script callback.
	GameCall("PlrHasRespawned", plr, respawn_list[plr]);
	return;
}

protected func RelaunchPlayer(int plr)
{
	var clonk = CreateObject(Clonk, 0, 0, plr);
	clonk->MakeCrewMember(plr);
	SetCursor(plr, clonk);
	JoinPlayer(plr);
	// Scenario script callback.
	GameCall("PlrHasRespawned", plr, respawn_list[plr]);
	// Log message.
	Log(RndRespawnMsg(), GetPlayerName(plr));
	return;
}

private func RndRespawnMsg()
{
	return Translate(Format("MsgRespawn%d", Random(4)));
}

protected func JoinPlayer(int plr)
{
	var clonk = GetCrew(plr);
	clonk->DoEnergy(100000);
	var pos = FindRespawnPos(plr);
	clonk->SetPosition(pos[0], pos[1]);
	AddEffect("IntDirNextCP", clonk, 100, 1, this);
	return;
}

private func FindRespawnPos(int plr)
{
	return [respawn_list[plr]->GetX(), respawn_list[plr]->GetY()];
}

protected func RemovePlayer(int plr)
{
	if (!finished)
		AddEvalData(plr);
	return;
}

/*-- Scoreboard --*/

static const SBRD_Checkpoints = 0;
static const SBRD_BestTime = 1;

private func InitScoreboard()
{
	if (cp_count > 0)
		var caption = Format("$MsgCaptionX$", cp_count);
	else
		var caption = "$MsgCaptionNone$";
	// The above row.
	SetScoreboardData(SBRD_Caption, SBRD_Caption, caption, SBRD_Caption);
	SetScoreboardData(SBRD_Caption, SBRD_Checkpoints, Format("{{%i}}", ParkourCheckpoint), SBRD_Caption);
	SetScoreboardData(SBRD_Caption, SBRD_BestTime, "T", SBRD_Caption);
	return;
}

private func UpdateScoreboard(int plr)
{
	if (finished)
		return;
	var plrid = GetPlayerID(plr);
	// The player name.
	SetScoreboardData(plrid, SBRD_Caption, GetTaggedPlayerName(plr), SBRD_Caption);
	// The player scores.
	SetScoreboardData(plrid, SBRD_Checkpoints, Format("%d", plr_list[plrid]), plr_list[plrid]);
	SetScoreboardData(plrid, SBRD_BestTime, TimeToString(GetPlrExtraData(plr, time_store)), GetPlrExtraData(plr, time_store));
	// Sort.
	SortScoreboard(SBRD_BestTime, false);
	SortScoreboard(SBRD_Checkpoints, true);
	return;
}

/*-- Direction indication --*/

// Effect for direction indication for the clonk.
protected func FxIntDirNextCPStart(object target, int fxnum)
{
	var arrow = CreateObject(GUI_GoalArrow, 0, 0, target->GetOwner());
	arrow->SetAction("Show", target);
	EffectVar(0, target, fxnum) = arrow;
	return FX_OK;
}

protected func FxIntDirNextCPTimer(object target, int fxnum)
{
	var plr = target->GetOwner();
	var team = GetPlayerTeam(plr);
	// Find nearest CP.
	var nextcp;
	for (var cp in FindObjects(Find_ID(ParkourCheckpoint), Find_Func("FindCPMode", PARKOUR_CP_Check | PARKOUR_CP_Finish), Sort_Distance(target->GetX() - GetX(), target->GetY() - GetY())))
		if (!cp->ClearedByPlr(plr) && (cp->IsActiveForPlr(plr) || cp->IsActiveForTeam(GetPlayerTeam(plr))))
		{
			nextcp = cp;
			break;
		}	
	if (!nextcp)
		return EffectVar(0, target, fxnum)->SetClrModulation(RGBa(0, 0, 0, 0));
	// Calculate parameters.
	var angle = Angle(target->GetX(), target->GetY(), nextcp->GetX(), nextcp->GetY());
	var dist = Min(510 * ObjectDistance(GetCrew(plr), nextcp) / 400, 510); 
	var red = BoundBy(dist, 0, 255);
	var green = BoundBy(510 - dist, 0, 255);
	var color = RGBa(red, green, 0, 128);
	// Draw arrow.
	EffectVar(0, target, fxnum)->SetR(angle);
	EffectVar(0, target, fxnum)->SetClrModulation(color);
	return FX_OK;
}

protected func FxIntDirNextCPStop(object target, int fxnum)
{
	EffectVar(0, target, fxnum)->RemoveObject();
	return;
}

/*-- Time tracker --*/

private func DoBestTime(int plr)
{
	var effect = GetEffect("IntBestTime", this);
	var time = EffectVar(0, this, effect);
	var rectime = GetPlrExtraData(plr, time_store);
	if (time != 0 && (!rectime || time < rectime))
	{
		SetPlrExtraData(plr, time_store, time);
		Log(Format("$MsgBestTime$", GetPlayerName(plr), TimeToString(time)));
	}
	return;
}

// Starts at goal initialization, should be equivalent to gamestart.
protected func FxIntBestTimeTimer(object pTarget, int iEffectNumber, int iEffectTime)
{
	EffectVar(0, pTarget, iEffectNumber) = iEffectTime;
	return FX_OK;
}

// Returns a best time string.
private func TimeToString(int time)
{
	if (!time) // No time.
		return "N/A";
	if (time > 36 * 60 * 60) // Longer than an hour.
		return Format("%d:%.2d:%.2d.%.1d", (time / 60 / 60 / 36) % 24, (time / 60 / 36) % 60, (time / 36) % 60, (10 * time / 36) % 10);
	if (time > 36 * 60) // Longer than a minute.
		return Format("%d:%.2d.%.1d", (time / 60 / 36) % 60, (time / 36) % 60, (10 * time / 36) % 10);
	else // Only seconds.
		return Format("%d.%.1d", (time / 36) % 60, (10 * time / 36) % 10);
}

/*-- Evaluation data --*/

private func SetEvalData(int winner)
{
	var winteam = GetPlayerTeam(winner);
	var effect = GetEffect("IntBestTime", this);
	var time = EffectVar(0, this, effect);
	var msg;
	// General data.
	if (winteam)
		msg = Format("$MsgEvalTeamWon$", GetTeamName(winteam), TimeToString(time));
	else
		msg = Format("$MsgEvalPlrWon$", GetPlayerName(winner), TimeToString(time));
	AddEvaluationData(msg, 0);
	// Individual data.
	for (var i = 0; i < GetPlayerCount(); i++)
		AddEvalData(GetPlayerByIndex(i));
	// Obviously get rid of settlement score.
	HideSettlementScoreInEvaluation(true); 
	return;
}

private func AddEvalData(int plr)
{
	if (finished)
		return;
	var plrid = GetPlayerID(plr);
	var cps = plr_list[plrid];
	var msg;
	if (cps == cp_count)
		msg = "$MsgEvalPlayerAll$";
	else
		msg = Format("$MsgEvalPlayerX$", cps);
	AddEvaluationData(msg, plrid);
	return;
}

/*-- Proplist --*/

protected func Definition(def)
{
	SetProperty("Name", "$Name$", def);
}
