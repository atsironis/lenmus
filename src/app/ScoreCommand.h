//--------------------------------------------------------------------------------------
//    LenMus Phonascus: The teacher of music
//    Copyright (c) 2002-2007 Cecilio Salmeron
//
//    This program is free software; you can redistribute it and/or modify it under the
//    terms of the GNU General Public License as published by the Free Software Foundation;
//    either version 2 of the License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//    PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along with this
//    program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street,
//    Fifth Floor, Boston, MA  02110-1301, USA.
//
//    For any comment, suggestion or feature request, please contact the manager of
//    the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LM_SCORECOMMAND_H__        //to avoid nested includes
#define __LM_SCORECOMMAND_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "ScoreCommand.cpp"
#endif

#include "wx/cmdproc.h"

#include "../score/defs.h"
class lmComponentObj;
class lmScoreDocument;

// base abstract class
class lmScoreCommand: public wxCommand
{
public:
	// Commands
	enum lmEScoreCommand
	{
		lmCMD_SelectSingle = 1,
		lmCMD_MoveScoreObj,
        lmCMD_InsertBarline,
        lmCMD_InsertClef,
        lmCMD_InsertNote,
	};

    virtual ~lmScoreCommand();

    virtual bool Do()=0;
    virtual bool Undo()=0;
    virtual lmEScoreCommand GetCommandType()=0;

protected:
    lmScoreCommand(const wxString& name, lmScoreDocument *pDoc);

    lmScoreDocument*    m_pDoc;
	bool				m_fDocModified;

};

// Select object command
//------------------------------------------------------------------------------------
class lmCmdSelectSingle: public lmScoreCommand
{
public:
    lmCmdSelectSingle(const wxString& name, lmScoreDocument *pDoc, lmGMObject* pGMO)
        : lmScoreCommand(name, pDoc)
        {
            m_pGMO = pGMO;
        }

    ~lmCmdSelectSingle() {}

    //overrides of pure virtual methods in base class
    bool Do();
    bool Undo();
    lmEScoreCommand GetCommandType() { return lmCMD_SelectSingle; }


protected:
	bool DoSelectObject();


    lmGMObject*		m_pGMO;

};

// Move object command
//------------------------------------------------------------------------------------
class lmCmdMoveScoreObj: public lmScoreCommand
{
public:
    lmCmdMoveScoreObj(const wxString& name, lmScoreDocument *pDoc, lmScoreObj* pSO,
					   const lmUPoint& uPos);
    ~lmCmdMoveScoreObj() {}

    //overrides of pure virtual methods in base class
    bool Do();
    bool Undo();
    lmEScoreCommand GetCommandType() { return lmCMD_MoveScoreObj; }


protected:
    lmLocation      m_tPos;
    lmLocation		m_tOldPos;        // for Undo
	lmScoreObj*		m_pSO;
};


// Insert barline command
//------------------------------------------------------------------------------------
class lmCmdInsertBarline: public lmScoreCommand
{
public:

    lmCmdInsertBarline(const wxString& name, lmScoreDocument *pDoc, lmStaffObj* pCursorSO,
					   lmEBarline nType);
    ~lmCmdInsertBarline() {}

    //overrides of pure virtual methods in base class
    bool Do();
    bool Undo();
    lmEScoreCommand GetCommandType() { return lmCMD_InsertBarline; }


protected:
    lmEBarline	    m_nBarlineType;
	lmStaffObj*		m_pCursorSO;
};


// Insert clef command
//------------------------------------------------------------------------------------
class lmCmdInsertClef: public lmScoreCommand
{
public:

    lmCmdInsertClef(const wxString& name, lmScoreDocument *pDoc, lmStaffObj* pCursorSO,
					lmEClefType nClefType);
    ~lmCmdInsertClef() {}

    //overrides of pure virtual methods in base class
    bool Do();
    bool Undo();
    lmEScoreCommand GetCommandType() { return lmCMD_InsertClef; }


protected:
    lmEClefType     m_nClefType;
	lmStaffObj*		m_pCursorSO;
};


// Insert note command
//------------------------------------------------------------------------------------
class lmCmdInsertNote: public lmScoreCommand
{
public:

    lmCmdInsertNote(const wxString& name, lmScoreDocument *pDoc, lmStaffObj* pCursorSO,
					lmEPitchType nPitchType, wxString sStep, wxString sOctave, 
					lmENoteType nNoteType, float rDuration);
    ~lmCmdInsertNote() {}

    //overrides of pure virtual methods in base class
    bool Do();
    bool Undo();
    lmEScoreCommand GetCommandType() { return lmCMD_InsertNote; }


protected:
	lmENoteType		m_nNoteType;
	lmEPitchType	m_nPitchType;
	wxString		m_sStep;
	wxString		m_sOctave;
	float			m_rDuration;
	lmStaffObj*		m_pCursorSO;
};

#endif    // __LM_SCORECOMMAND_H__        //to avoid nested includes
