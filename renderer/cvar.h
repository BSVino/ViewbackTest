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

#ifndef LW_TINKER_CVAR
#define LW_TINKER_CVAR

#include <map>

#include <common.h>
#include <string>
#include <vector>

typedef void(*CommandCallback)(class CCommand* pCommand, std::vector<std::string>& asTokens, const std::string& sCommand);

class CCommand
{
public:
	CCommand(std::string sName, CommandCallback pfnCallback);

public:
	static void			Run(std::string sCommand);

	std::string				GetName() { return m_sName; };

	virtual void		MakeMePolymorphic() {};	// Can delete if another virtual function is added

	static std::vector<std::string> GetCommandsBeginningWith(std::string sFragment);

protected:
	std::string				m_sName;
	CommandCallback		m_pfnCallback;

	static void			RegisterCommand(CCommand* pCommand);

protected:
	static std::map<std::string, CCommand*>& GetCommands()
	{
		static std::map<std::string, CCommand*> aCommands;
		return aCommands;
	}
};

class CVar : public CCommand
{
	DECLARE_CLASS(CVar, CCommand);

public:
	CVar(std::string sName, std::string sValue);

public:
	void				SetValue(std::string sValue);
	void				SetValue(int iValue);
	void				SetValue(float flValue);

	std::string				GetValue() { return m_sValue; };
	bool				GetBool();
	int					GetInt();
	float				GetFloat();

	void				CalculateValues();

	static CVar*		FindCVar(std::string sName);

	static void			SetCVar(std::string sName, std::string sValue);
	static void			SetCVar(std::string sName, int iValue);
	static void			SetCVar(std::string sName, float flValue);

	static std::string		GetCVarValue(std::string sName);
	static bool			GetCVarBool(std::string sName);
	static int			GetCVarInt(std::string sName);
	static float		GetCVarFloat(std::string sName);

protected:
	std::string				m_sValue;

	bool				m_bDirtyValues;
	bool				m_bValue;
	int					m_iValue;
	float				m_flValue;
};

#endif
