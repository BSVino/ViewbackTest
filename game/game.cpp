/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "game.h"

#include <cstring>

#include <algorithm>

#include <viewback.h>
#include <viewback_util.h>

#include <common_platform.h>

#include <mtrand.h>
#include <math/collision.h>
#include <math/frustum.h>
#include <maths.h>
#include <math/quaternion.h>
#include <math/physics.h>

#include <renderer/cvar.h>
#include <renderer/renderer.h>
#include <renderer/renderingcontext.h>

#include "character.h"

CGame::CGame(int argc, char** argv)
	: CApplication(argc, argv)
{
	m_hPlayer = nullptr;

	m_iLastMouseX = m_iLastMouseY = -1;

	memset(m_apEntityList, 0, sizeof(m_apEntityList));

	InitializeWin32SocketsBullshit();

	mtsrand(0);
}

void vb_command(const char* text)
{
	CCommand::Run(text);
}

void vb_player_speed(float value)
{
	CVar::SetCVar("player_speed", value);
}

void CGame::Load()
{
	m_iMonsterTexture = GetRenderer()->LoadTextureIntoGL("monster.png");
	m_iCrateTexture = GetRenderer()->LoadTextureIntoGL("crate.png");

	vb_util_add_channel("Player speed", VB_DATATYPE_FLOAT, NULL);
	vb_util_set_range_s("Player speed", 0, 400);

	vb_util_add_control_slider_float("Player speed", 0, 20, 0, vb_player_speed);
	vb_util_set_control_slider_float_value("Player speed", CVar::GetCVarFloat("player_speed"));

	vb_util_set_command_callback(vb_command);

	vb_util_server_create("Roguelike Test");
}

CVar player_speed("player_speed", "15");

// This method gets called when the user presses a key
bool CGame::KeyPress(int c)
{
	if (c == 'W')
	{
		m_hPlayer->m_vecMovementGoal.x = player_speed.GetFloat();
		return true;
	}
	else if (c == 'A')
	{
		m_hPlayer->m_vecMovementGoal.z = player_speed.GetFloat();
		return true;
	}
	else if (c == 'S')
	{
		m_hPlayer->m_vecMovementGoal.x = -player_speed.GetFloat();
		return true;
	}
	else if (c == 'D')
	{
		m_hPlayer->m_vecMovementGoal.z = -player_speed.GetFloat();
		return true;
	}
	else if (c == ' ')
	{
		m_hPlayer->m_vecVelocity.y = 7;
		return true;
	}
	else if (c == 256)
	{
		SetMouseCursorEnabled(!m_bMouseEnabled);
		return true;
	}
	else
		return CApplication::KeyPress(c);
}

// This method gets called when the player releases a key.
void CGame::KeyRelease(int c)
{
	if (c == 'W')
	{
		m_hPlayer->m_vecMovementGoal.x = 0;
	}
	else if (c == 'A')
	{
		m_hPlayer->m_vecMovementGoal.z = 0;
	}
	else if (c == 'S')
	{
		m_hPlayer->m_vecMovementGoal.x = 0;
	}
	else if (c == 'D')
	{
		m_hPlayer->m_vecMovementGoal.z = 0;
	}
	else
		CApplication::KeyPress(c);
}

// In this Update() function we need to update all of our characters. Move them around or whatever we want to do.
// http://www.youtube.com/watch?v=c4b9lCfSDQM
void CGame::Update(float dt)
{
	Vector x0 = m_hPlayer->GetGlobalOrigin();

	// The approach function http://www.youtube.com/watch?v=qJq7I2DLGzI
	m_hPlayer->m_vecMovement.x = Approach(m_hPlayer->m_vecMovementGoal.x, m_hPlayer->m_vecMovement.x, dt * 65);
	m_hPlayer->m_vecMovement.z = Approach(m_hPlayer->m_vecMovementGoal.z, m_hPlayer->m_vecMovement.z, dt * 65);

	Vector vecForward = m_hPlayer->GetGlobalView();
	vecForward.y = 0;
	vecForward.Normalize();

	Vector vecUp(0, 1, 0);

	// Cross product http://www.youtube.com/watch?v=FT7MShdqK6w
	Vector vecRight = vecUp.Cross(vecForward);

	float flSaveY = m_hPlayer->m_vecVelocity.y;
	m_hPlayer->m_vecVelocity = vecForward * m_hPlayer->m_vecMovement.x + vecRight * m_hPlayer->m_vecMovement.z;
	m_hPlayer->m_vecVelocity.y = flSaveY;

	// Update position and vecMovement. http://www.youtube.com/watch?v=c4b9lCfSDQM
	m_hPlayer->SetTranslation(m_hPlayer->GetGlobalOrigin() + m_hPlayer->m_vecVelocity * dt);
	m_hPlayer->m_vecVelocity = m_hPlayer->m_vecVelocity + m_hPlayer->m_vecGravity * dt;

	// Make sure the player doesn't fall through the floor. The y dimension is up/down, and the floor is at 0.
	Vector vecTranslation = m_hPlayer->GetGlobalOrigin();
	if (vecTranslation.y < 0)
		m_hPlayer->SetTranslation(Vector(vecTranslation.x, 0, vecTranslation.z));

	// Grab the player's translation and make a translation only matrix. http://www.youtube.com/watch?v=iCazI3nKBf0
	Vector vecPosition = m_hPlayer->GetGlobalOrigin();
	Matrix4x4 mPlayerTranslation;
	mPlayerTranslation.SetTranslation(vecPosition);

	// Create a set of basis vectors that do what we need.
	vecForward = m_hPlayer->GetGlobalView();
	vecForward.y = 0;       // Flatten the angles so that the box doesn't rotate up and down as the player does.
	vecForward.Normalize(); // Re-normalize, we need all of our basis vectors to be normal vectors (unit-length)
	vecUp = Vector(0, 1, 0);  // The global up vector
	vecRight = -vecUp.Cross(vecForward).Normalized(); // Cross-product: https://www.youtube.com/watch?v=FT7MShdqK6w

	// Use these basis vectors to make a matrix that will transform the player-box the way we want it.
	// http://youtu.be/8sqv11x10lc
	Matrix4x4 mPlayerRotation(vecForward, vecUp, vecRight);

	Matrix4x4 mPlayerScaling = Matrix4x4();

	// Produce a transformation matrix from our three TRS matrices.
	// Order matters! http://youtu.be/7pe1xYzFCvA
	m_hPlayer->SetGlobalTransform(mPlayerTranslation * mPlayerRotation * mPlayerScaling);

	vb_data_send_float_s("Player speed", m_hPlayer->m_vecVelocity.Length2D());
	//vb_data_set_control_slider_float_value("Player speed", player_speed.GetFloat());

	float flMonsterSpeed = 0.5f;
	for (size_t i = 0; i < MAX_CHARACTERS; i++)
	{
		CCharacter* pCharacter = GetCharacterIndex(i);
		if (!pCharacter)
			continue;

		if (!pCharacter->m_bEnemyAI)
			continue;

		// Update position and movement. http://www.youtube.com/watch?v=c4b9lCfSDQM
		if ((m_hPlayer->GetGlobalOrigin() - pCharacter->GetGlobalOrigin()).Length() < 1)
			continue;

		pCharacter->m_vecVelocity = (m_hPlayer->GetGlobalOrigin() - pCharacter->GetGlobalOrigin()).Normalized() * flMonsterSpeed;

		pCharacter->SetTranslation(pCharacter->GetGlobalOrigin() + pCharacter->m_vecVelocity * dt);
	}
}

// The Game Loop http://www.youtube.com/watch?v=c4b9lCfSDQM
void CGame::GameLoop()
{
	m_hPlayer = CreateCharacter();

	// Initialize the box's position etc
	m_hPlayer->SetGlobalOrigin(Point(0, 0, 0));
	m_hPlayer->m_vecMovement = Vector(0, 0, 0);
	m_hPlayer->m_vecMovementGoal = Vector(0, 0, 0);
	m_hPlayer->m_vecVelocity = Vector(0, 0, 0);
	m_hPlayer->m_vecGravity = Vector(0, -10, 0);
	m_hPlayer->m_clrRender = Color(0.8f, 0.4f, 0.2f, 1.0f);
	m_hPlayer->m_bHitByTraces = false;
	m_hPlayer->m_aabbSize = AABB(-Vector(0.5f, 0, 0.5f), Vector(0.5f, 2, 0.5f));
	m_hPlayer->m_bTakesDamage = true;

	Vector vecMonsterMin = Vector(-1, 0, -1);
	Vector vecMonsterMax = Vector(1, 2, 1);

	CCharacter* pTarget1 = CreateCharacter();
	pTarget1->SetTransform(Vector(1, 1, 1), 0, Vector(0, 1, 0), Vector(6, 0, 6));
	pTarget1->m_aabbSize.vecMin = vecMonsterMin;
	pTarget1->m_aabbSize.vecMax = vecMonsterMax;
	pTarget1->m_iBillboardTexture = m_iMonsterTexture;
	pTarget1->m_bEnemyAI = true;
	pTarget1->m_bTakesDamage = true;

	CCharacter* pTarget2 = CreateCharacter();
	pTarget2->SetTransform(Vector(1, 1, 1), 0, Vector(0, 1, 0), Vector(6, 0, -6));
	pTarget2->m_aabbSize.vecMin = vecMonsterMin;
	pTarget2->m_aabbSize.vecMax = vecMonsterMax;
	pTarget2->m_iBillboardTexture = m_iMonsterTexture;
	pTarget2->m_bEnemyAI = true;
	pTarget2->m_bTakesDamage = true;

	CCharacter* pTarget3 = CreateCharacter();
	pTarget3->SetTransform(Vector(1, 1, 1), 0, Vector(0, 1, 0), Vector(-6, 0, 8));
	pTarget3->m_aabbSize.vecMin = vecMonsterMin;
	pTarget3->m_aabbSize.vecMax = vecMonsterMax;
	pTarget3->m_iBillboardTexture = m_iMonsterTexture;
	pTarget3->m_bEnemyAI = true;
	pTarget3->m_bTakesDamage = true;

	Vector vecPropMin = Vector(-1, 0, -1);
	Vector vecPropMax = Vector(1, 2, 1);

	for (int i = 0; i < 8; i++)
	{
		float rand1 = (float)(mtrand()%1000)/1000; // [0, 1]
		float rand2 = (float)(mtrand()%1000)/1000; // [0, 1]

		float theta = rand1 * 2.0f * (float)M_PI;
		float radius = sqrt(rand2);

		Vector position = Vector(radius * cos(theta), 0, radius * sin(theta));
		position = position * 50;

		CCharacter* pProp = CreateCharacter();
		pProp->SetTransform(Vector(1, 1, 1), 0, Vector(0, 1, 0), position);
		pProp->m_aabbSize.vecMin = vecPropMin;
		pProp->m_aabbSize.vecMax = vecPropMax;
		pProp->m_clrRender = Color(0.4f, 0.8f, 0.2f, 1.0f);
		pProp->m_iTexture = m_iCrateTexture;
	}

	CRenderingContext c(GetRenderer());
	c.RenderBox(Vector(-1, 0, -1), Vector(1, 2, 1));
	c.CreateVBO(m_iMeshVB, m_iMeshSize);

	float flPreviousTime = 0;
	float flCurrentTime = Application()->GetTime();

	float frame_end_time = 0;
	float frame_start_time = 0;

	while (true)
	{
		frame_end_time = GetTime();

		{
			double next_frame_time = frame_start_time + (1.0f / 30);
			double time_to_sleep_seconds = next_frame_time - frame_end_time;
			if (time_to_sleep_seconds > 0.001)
				SleepMS((size_t)(time_to_sleep_seconds * 1000));
		}

		frame_start_time = GetTime();

		// flCurrentTime will be lying around from last frame. It's now the previous time.
		flPreviousTime = flCurrentTime;
		flCurrentTime = Application()->GetTime();

		vb_server_update((vb_uint64)(flCurrentTime * 1000));

		float dt = flCurrentTime - flPreviousTime;

		// Keep dt from growing too large.
		if (dt > 0.15f)
			dt = 0.15f;

		Update(dt);

		Draw();
	}
}

