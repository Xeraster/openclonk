/*
 * OpenClonk, http://www.openclonk.org
 *
 * Copyright (c) 2001-2009, RedWolf Design GmbH, http://www.clonk.de
 *
 * Portions might be copyrighted by other authors who have contributed
 * to OpenClonk.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * See isc_license.txt for full license and disclaimer.
 *
 * "Clonk" is a registered trademark of Matthes Bender.
 * See clonk_trademark_license.txt for full license.
 */
/* string table: holds all strings used by script engine */

#include <C4Include.h>
#include <C4StringTable.h>

#ifndef BIG_C4INCLUDE
#include <C4Group.h>
#include <C4Components.h>
#include <C4Aul.h>
#include <C4Game.h>
#endif

// *** C4Set
template<> template<>
unsigned int C4Set<C4String *>::Hash<const char *>(const char * s)
	{
	// Fowler/Noll/Vo hash
	unsigned int h = 2166136261u;
	while (*s)
		h = (h ^ *(s++)) * 16777619;
	return h;
	}

template<> template<>
bool C4Set<C4String *>::Equals<const char *>(C4String * a, const char * b)
	{
	return a->GetData() == b;
	}

// *** C4String

C4String::C4String(StdStrBuf strString)
	: iRefCnt(0)
	{
	// take string
	Data.Take(strString);
	Hash = C4Set<C4String*>::Hash(Data.getData());
	// reg
	Game.ScriptEngine.Strings.Set.Add(this);
	}

C4String::~C4String()
	{
	// unreg
	iRefCnt = 1;
	Game.ScriptEngine.Strings.Set.Remove(this);
	}

void C4String::IncRef()
	{
	++iRefCnt;
	}

void C4String::DecRef()
	{
	--iRefCnt;
	if(iRefCnt <= 0)
		delete this;
	}

// *** C4StringTable

C4StringTable::C4StringTable()
	{
	}

C4StringTable::~C4StringTable()
	{
	Clear();
	assert(!Set.GetSize());
	}

void C4StringTable::Clear()
	{
	for (unsigned int i = 0; i < Stringstxt.size(); ++i)
		Stringstxt[i]->DecRef();
	Stringstxt.clear();
	}

C4String *C4StringTable::RegString(StdStrBuf String)
	{
	C4String * s = FindString(String.getData());
	if (s)
		return s;
	else
		return new C4String(String);
	}

C4String *C4StringTable::FindString(const char *strString)
	{
	return Set.Get(strString);
	}

C4String *C4StringTable::FindString(C4String *pString)
	{
	for (C4String * const * i = Set.First(); i; i = Set.Next(i))
		if (*i == pString)
			return pString;
	}

C4String *C4StringTable::FindString(int iEnumID)
	{
	if (iEnumID >= 0 && iEnumID < int(Stringstxt.size()))
		return Stringstxt[iEnumID];
	return NULL;
	}

bool C4StringTable::Load(C4Group& ParentGroup)
	{
	Clear();
	// read data
	char *pData;
	if(!ParentGroup.LoadEntry(C4CFN_Strings, &pData, NULL, 1))
		return false;
	// read all strings
	char strBuf[C4AUL_MAX_String + 1];
	for(int i = 0; SCopySegment(pData, i, strBuf, 0x0A, C4AUL_MAX_String); i++)
		{
		SReplaceChar(strBuf, 0x0D, 0x00);
		// add string to list
		C4String *pnString;
		pnString = RegString(StdStrBuf(strBuf));
		pnString->IncRef();
		Stringstxt.push_back(pnString);
		}
	// delete data
	delete[] pData;
	return true;
	}
