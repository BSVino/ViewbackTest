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

#include "cvar.h"

#include <viewback.h>

#include <strutils.h>
#include <vector>

CCommand::CCommand(std::string sName, CommandCallback pfnCallback)
{
	m_sName = sName;
	m_pfnCallback = pfnCallback;

	RegisterCommand(this);
}

void CCommand::Run(std::string sCommand)
{
	std::vector<std::string> asTokens;
	strtok(sCommand, asTokens);

	if (asTokens.size() == 0)
		return;

	std::map<std::string, CCommand*>::iterator it = GetCommands().find(asTokens[0]);
	if (it == GetCommands().end())
	{
		vb_console_append("Unrecognized command.\n");
		return;
	}

	CCommand* pCommand = it->second;
	pCommand->m_pfnCallback(pCommand, asTokens, sCommand);
}

std::vector<std::string> CCommand::GetCommandsBeginningWith(std::string sFragment)
{
	std::vector<std::string> sResults;

	size_t iFragLength = sFragment.length();

	std::map<std::string, CCommand*>& sCommands = GetCommands();
	for (std::map<std::string, CCommand*>::iterator it = sCommands.begin(); it != sCommands.end(); it++)
	{
		if (it->first.substr(0, iFragLength) == sFragment)
			sResults.push_back(it->first);
	}

	return sResults;
}

void CCommand::RegisterCommand(CCommand* pCommand)
{
	GetCommands()[pCommand->m_sName] = pCommand;
}

void SetCVar(CCommand* pCommand, std::vector<std::string>& asTokens, const std::string& /*sCommand*/)
{
	CVar* pCVar = dynamic_cast<CVar*>(pCommand);
	TAssert(pCVar);
	if (!pCVar)
		return;

	if (asTokens.size() > 1)
		pCVar->SetValue(asTokens[1]);

	vb_console_append(tsprintf("%s = %s\n", pCVar->GetName().c_str(), pCVar->GetValue().c_str()).c_str());
}

CVar::CVar(std::string sName, std::string sValue)
	: CCommand(sName, ::SetCVar)
{
	m_sValue = sValue;

	m_bDirtyValues = true;
}

void CVar::SetValue(std::string sValue)
{
	m_sValue = sValue;

	m_bDirtyValues = true;
}

void CVar::SetValue(int iValue)
{
	m_sValue = tsprintf("%d", iValue);

	m_bDirtyValues = true;
}

void CVar::SetValue(float flValue)
{
	m_sValue = tsprintf("%f", flValue);

	m_bDirtyValues = true;
}

bool CVar::GetBool()
{
	if (m_bDirtyValues)
		CalculateValues();

	return m_bValue;
}

int CVar::GetInt()
{
	if (m_bDirtyValues)
		CalculateValues();

	return m_iValue;
}

float CVar::GetFloat()
{
	if (m_bDirtyValues)
		CalculateValues();

	return m_flValue;
}

void CVar::CalculateValues()
{
	if (!m_bDirtyValues)
		return;

	m_flValue = (float)atof(m_sValue.c_str());
	m_iValue = atoi(m_sValue.c_str());
	m_bValue = (m_sValue.compare("yes") == 0 ||
		m_sValue.compare("true") == 0 ||
		m_sValue.compare("on") == 0 ||
		fabs(atof(m_sValue.c_str())) > 0);

	m_bDirtyValues = false;
}

CVar* CVar::FindCVar(std::string sName)
{
	std::map<std::string, CCommand*>::iterator it = GetCommands().find(sName);
	if (it == GetCommands().end())
		return NULL;

	CVar* pVar = dynamic_cast<CVar*>(it->second);
	return pVar;
}

void CVar::SetCVar(std::string sName, std::string sValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(sValue);
}

void CVar::SetCVar(std::string sName, int iValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(iValue);
}

void CVar::SetCVar(std::string sName, float flValue)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return;

	pVar->SetValue(flValue);
}

std::string CVar::GetCVarValue(std::string sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return "";

	return pVar->GetValue();
}

bool CVar::GetCVarBool(std::string sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return false;

	return pVar->GetBool();
}

int CVar::GetCVarInt(std::string sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return 0;

	return pVar->GetInt();
}

float CVar::GetCVarFloat(std::string sName)
{
	CVar* pVar = FindCVar(sName);
	if (!pVar)
		return 0;

	return pVar->GetFloat();
}
