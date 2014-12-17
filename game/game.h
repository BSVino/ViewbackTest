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

#pragma once

#include <vector>
#include <deque>

#include <common/common.h>

#include <math/frustum.h>
#include <math/graph.h>

#include <renderer/application.h>

#include "handle.h"

using std::vector;

#define MAX_CHARACTERS 1000

// CGame is the "application" class. It creates the window and handles user input.
// It extends CApplication, which does all of the dirty work. All we have to do
// is override functions like KeyPress and KeyRelease, and CApplication will call
// those functions when the player presses a key on the keyboard.
class CGame : public CApplication
{
	DECLARE_CLASS(CGame, CApplication);

	// A puff of "smoke" as if from a bullet hitting something
	class CPuff
	{
	public:
		Point vecOrigin;
		float flTimeCreated;
	};

	// A bullet tracer so we can see where the bullet goes
	class CBulletTracer
	{
	public:
		Point vecStart;
		Point vecEnd;
		float flTimeCreated;
	};

public:
	CGame(int argc, char** argv);

public:
	void Load();

	virtual bool KeyPress(int c);
	virtual void KeyRelease(int c);
	virtual void MouseMotion(int x, int y);
	virtual bool MouseInput(int iButton, tinker_mouse_state_t iState);

	bool TraceLine(const Vector& v0, const Vector& v1, Vector& vecIntersection, class CCharacter*& pHit);

	void MakePuff(const Point& vecPuff);
	const vector<CPuff>& GetPuffs() const { return m_aPuffs; }

	void MakeBulletTracer(const Point& vecStart, const Point& vecEnd);
	const vector<CBulletTracer>& GetTracers() const { return m_aTracers; }

	void Update(float dt);
	void Draw();
	void DrawCharacters(const std::vector<CCharacter*>& apRenderList, bool bTransparent);
	void MergeSortTransparentRenderList();
	void GameLoop();

	CCharacter* CreateCharacter();
	void        RemoveCharacter(CCharacter* pCharacter);
	CCharacter* GetCharacterIndex(size_t i);

	size_t      GetMonsterTexture() { return m_iMonsterTexture; }

private:
	int m_iLastMouseX;
	int m_iLastMouseY;

	CFrustum m_oFrameFrustum;

	vector<CPuff> m_aPuffs;
	vector<CBulletTracer> m_aTracers;

	size_t m_iMonsterTexture;
	size_t m_iCrateTexture;

	CCharacter*              m_apEntityList[MAX_CHARACTERS];
	std::vector<CCharacter*> m_apRenderOpaqueList;
	std::vector<CCharacter*> m_apRenderTransparentList;

	// This is the player character
	CHandle m_hPlayer;

	size_t m_iMeshVB;
	size_t m_iMeshSize;
};

inline CGame* Game()
{
	return static_cast<CGame*>(Application());
}
