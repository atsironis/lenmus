//--------------------------------------------------------------------------------------
//    LenMus Phonascus: The teacher of music
//    Copyright (c) 2002-2008 Cecilio Salmeron
//
//    This program is free software; you can redistribute it and/or modify it under the
//    terms of the GNU General Public License as published by the Free Software Foundation,
//    either version 3 of the License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but WITHOUT ANY
//    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//    PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along with this
//    program. If not, see <http://www.gnu.org/licenses/>.
//
//    For any comment, suggestion or feature request, please contact the manager of
//    the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "StaffObj.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <math.h>
#include "wx/image.h"
#include "Score.h"
#include "Staff.h"
#include "VStaff.h"
#include "Context.h"
#include "ObjOptions.h"
#include "UndoRedo.h"
#include "Text.h"
#include "../app/ArtProvider.h"
#include "../graphic/GMObject.h"
#include "../graphic/Shapes.h"
#include "../app/ScoreCanvas.h"
#include "properties/DlgProperties.h"
#include "properties/GeneralProperties.h"


//-------------------------------------------------------------------------------------
// lmScoreObj implementation
//-------------------------------------------------------------------------------------

lmScoreObj::lmScoreObj(lmScoreObj* pParent)
{
    m_pParent = pParent;
    m_pObjOptions = (lmObjOptions*)NULL;
    m_pAuxObjs = (lmAuxObjsCol*)NULL;

    // initializations: positioning related info
    m_uPaperPos.y = 0.0f,   m_uPaperPos.x = 0.0f;
	m_tSrcPos = g_tDefaultPos;
    m_uComputedPos.x = 0.0f,   m_uComputedPos.y = 0.0f;
	m_fModified = false;
    m_pShapesMngr = new lmShapesMngr();     //default behaviour: only one shape
}

lmScoreObj::~lmScoreObj()
{
    if (m_pObjOptions) delete m_pObjOptions;

    //delete the attached AuxObjs
    if (m_pAuxObjs)
    {
        for (int i=0; i < (int)m_pAuxObjs->size(); i++)
        {
            delete (*m_pAuxObjs)[i];
        }
        m_pAuxObjs->clear();
        delete m_pAuxObjs;
    }

    if (m_pShapesMngr) delete m_pShapesMngr;

}

void lmScoreObj::DefineAsMultiShaped()
{
    delete m_pShapesMngr;
    m_pShapesMngr = new lmMultiShapesMngr();
}


lmObjOptions* lmScoreObj::GetCurrentObjOptions()
{
    //recurse in the parents chain to find the first non-null CtxObject
    //and return it
    if (m_pObjOptions) return m_pObjOptions;
    if (m_pParent) return m_pParent->GetCurrentObjOptions();
    return (lmObjOptions*)NULL;
}


//Set value for option in this object context. If no context exist, create it

void lmScoreObj::SetOption(wxString sName, long nLongValue)
{
    if (!m_pObjOptions) m_pObjOptions = new lmObjOptions();
    m_pObjOptions->SetOption(sName, nLongValue);
}

void lmScoreObj::SetOption(wxString sName, wxString sStringValue)
{
    if (!m_pObjOptions) m_pObjOptions = new lmObjOptions();
    m_pObjOptions->SetOption(sName, sStringValue);
}

void lmScoreObj::SetOption(wxString sName, double nDoubleValue)
{
    if (!m_pObjOptions) m_pObjOptions = new lmObjOptions();
    m_pObjOptions->SetOption(sName, nDoubleValue);
}

void lmScoreObj::SetOption(wxString sName, bool fBoolValue)
{
    if (!m_pObjOptions) m_pObjOptions = new lmObjOptions();
    m_pObjOptions->SetOption(sName, fBoolValue);
}

//Look for the value of an option. A method for each supported data type.
//Recursive search throug the ObjOptions chain

long lmScoreObj::GetOptionLong(wxString sOptName)
{
    return GetCurrentObjOptions()->GetOptionLong(sOptName);
}

double lmScoreObj::GetOptionDouble(wxString sOptName)
{
    return GetCurrentObjOptions()->GetOptionDouble(sOptName);
}

bool lmScoreObj::GetOptionBool(wxString sOptName)
{
    return GetCurrentObjOptions()->GetOptionBool(sOptName);
}

wxString lmScoreObj::GetOptionString(wxString sOptName)
{
    return GetCurrentObjOptions()->GetOptionString(sOptName);
}


int lmScoreObj::AttachAuxObj(lmAuxObj* pAO, int nIndex)
{
    // Add an AuxObj to the collection of attached AuxObjs. Index is the position
	// that the addedAuxObj must occupy (0..n). If -1, it will be added at the end.

	//add the AuxObj
    if (!m_pAuxObjs)
	{
		wxASSERT(nIndex == -1 || nIndex == 0);
		m_pAuxObjs = new lmAuxObjsCol();
		m_pAuxObjs->push_back(pAO);
		nIndex = 0;
	}
	else if (nIndex == -1 || nIndex == (int)m_pAuxObjs->size())
	{
		m_pAuxObjs->push_back(pAO);
		nIndex = (int)m_pAuxObjs->size() - 1;
	}
	else
	{
		int iN;
		std::vector<lmAuxObj*>::iterator it;
		for (iN=0, it=m_pAuxObjs->begin(); it != m_pAuxObjs->end(); ++it, ++iN)
		{
			if (iN == nIndex)
			{
				//insert before current item
				m_pAuxObjs->insert(it, pAO);
				break;
			}
		}
	}

	//set owner and return index to attached object
    pAO->SetOwner(this);
	return nIndex;
}

int lmScoreObj::DetachAuxObj(lmAuxObj* pAO)
{
    //Remove object from collection. Return index to position it occupied

	int nIdx = 0;
    std::vector<lmAuxObj*>::iterator it;
    for (it = m_pAuxObjs->begin(); it != m_pAuxObjs->end(); ++it, ++nIdx)
	{
		if (*it == pAO) break;
	}
    wxASSERT(it != m_pAuxObjs->end());
    m_pAuxObjs->erase(it);
	return nIdx;
}

lmLocation lmScoreObj::SetUserLocation(lmLocation tPos, int nShapeIdx)
{
	//convert location to logical units
	if (tPos.xUnits == lmTENTHS)
		tPos.x = TenthsToLogical(tPos.x);
	if (tPos.yUnits == lmTENTHS)
		tPos.y = TenthsToLogical(tPos.y);

    lmGMObject* pGMObj = GetGraphicObject(nShapeIdx);
    if (pGMObj)
    {
		//interactive edition: user is moving an object
	    lmUPoint uShapePos = pGMObj->GetBounds().GetTopLeft();
        lmUPoint uShift(tPos.x - uShapePos.x, tPos.y - uShapePos.y);
        lmUPoint uUserShift = GetUserShift(nShapeIdx);
		uUserShift.x += uShift.x;
		uUserShift.y += uShift.y;
        this->SaveUserLocation(uUserShift.x, uUserShift.y, nShapeIdx);

		////DBG--------------------------------------------------------------------------------
		//if (GetScoreObjType()==lmSOT_ComponentObj) // && ((lmComponentObj*)this)->GetID()==4)
		//{
		//    lmUPoint uShapePos = pGMObj->GetBounds().GetTopLeft();
		//    lmUPoint uShapeOrg = pGMObj->GetOrigin();
		//	  wxLogMessage(_T("[lmScoreObj::SetUserLocation] UserPos=(%.2f, %.2f), ShapeOrg=(%.2f, %.2f), ShapePos=(%.2f, %.2f), old UserShift=(%.2f, %.2f), new UserShift=(%.2f, %.2f)"),
		//				tPos.x, tPos.y, uShapeOrg.x, uShapeOrg.y,
		//				uShapePos.x, uShapePos.y, uShift.x, uShift.y,
		//				m_uUserShift.x, m_uUserShift.y );
		//}
		////END DBG----------------------------------------------------------------------------

		//Move also attached AuxObjs to this ScoreObj
		if (m_pAuxObjs && IsMainShape(nShapeIdx))
		{
			for (int i=0; i < (int)m_pAuxObjs->size(); i++)
			{
				(*m_pAuxObjs)[i]->OnParentMoved(uShift.x, uShift.y);
			}
		}
    }
	else
	{
		//loading the score from a file. Not yet layouted
        this->SaveUserLocation(tPos.x, tPos.y, nShapeIdx);
	}

	m_tSrcPos = m_tPos;
	//m_tPos = tPos;
	return m_tSrcPos;
}

void lmScoreObj::ResetObjectLocation()
{
	m_tPos = m_tSrcPos;

 //   // X position
 //   wxString sType = _T("");
	//switch (m_tPos.xType) {
	//	case lmLOCATION_DEFAULT:		sType = _T("LOCATION_DEFAULT"); break;
	//	case lmLOCATION_COMPUTED:		sType = _T("LOCATION_COMPUTED"); break;
	//	case lmLOCATION_USER_RELATIVE:	sType = _T("LOCATION_USER_RELATIVE"); break;
	//	case lmLOCATION_USER_ABSOLUTE:	sType = _T("LOCATION_USER_ABSOLUTE"); break;
	//	default:
	//		sType = wxString::Format(_T("LOCATION %d"), m_tPos.xType);
	//}

	//wxString sSource = wxString::Format(_T("x: %.2f %.2f - "), m_tPos.x, m_tPos.xUnits);
	//sSource += sType;

	//// Y position
 //   sType = _T("");
	//switch (m_tPos.yType) {
	//	case lmLOCATION_DEFAULT:		sType = _T("LOCATION_DEFAULT"); break;
	//	case lmLOCATION_COMPUTED:		sType = _T("LOCATION_COMPUTED"); break;
	//	case lmLOCATION_USER_RELATIVE:	sType = _T("LOCATION_USER_RELATIVE"); break;
	//	case lmLOCATION_USER_ABSOLUTE:	sType = _T("LOCATION_USER_ABSOLUTE"); break;
	//	default:
	//		sType = wxString::Format(_T("LOCATION %d"), m_tPos.yType);
	//}

	//sSource += wxString::Format(_T("  /  y: %.2f %.2f - %s\n"), m_tPos.y, m_tPos.yUnits, sType.c_str());

	//wxLogMessage(sSource);
}

void lmScoreObj::StoreOriginAndShiftShapes(lmLUnits uxShift, int nShapeIdx)
{
    //This method is invoked only from TimeposTable module, from methods 
    //lmTimeLine::ShiftEntries() and lmTimeLine::Reposition(), during auto-layout
    //computations.
    //By invoking this method, the auto-layout algorithm is informing about a change in
    //the computed final position for this ScoreObj.
    //Be aware of the fact that this method can be invoked several times for the
    //same ScoreObj, when the auto-layout algorithm refines the final position.

    lmGMObject* pGMObj = GetGraphicObject(nShapeIdx);
    if (pGMObj)
    {
		////DBG--------------------------------------------------------------------------------
		////if (GetScoreObjType()==lmSOT_ComponentObj && ((lmComponentObj*)this)->GetID()==1)
		//if (GetScoreObjType()==lmSOT_ComponentObj
  //          && ((lmComponentObj*)this)->GetType()==lm_eStaffObj
  //          && ((lmStaffObj*)this)->IsClef() )
		//{
		//	lmUPoint uNewOrg = m_uComputedPos + m_uUserShift;
		//	wxLogMessage(_T("[lmScoreObj::StoreOriginAndShiftShapes] uxShift=%.2f, ShapeIdx=%d"),
		//				uxShift, nShapeIdx );
		//}
		////END DBG----------------------------------------------------------------------------
        
        pGMObj->Shift(uxShift, 0.0f);
        pGMObj->ApplyUserShift( this->GetUserShift(nShapeIdx) );
    }

	// inform about the change to AuxObjs attached to this StaffObj
    if (m_pAuxObjs && this->IsMainShape(nShapeIdx))
    {
        for (int i=0; i < (int)m_pAuxObjs->size(); i++)
        {
            (*m_pAuxObjs)[i]->OnParentComputedPositionShifted(uxShift, 0.0f);
        }
    }

}

void lmScoreObj::PopupMenu(lmController* pCanvas, lmGMObject* pGMO, const lmDPoint& vPos)
{
	wxMenu* pMenu = pCanvas->GetContextualMenu();
	if (!pMenu)
		wxMessageBox(pGMO->Dump(0));
	else
	{
		CustomizeContextualMenu(pMenu, pGMO);
		pCanvas->ShowContextualMenu(this, pGMO, pMenu, vPos.x, vPos.y);
	}
}

void lmScoreObj::CustomizeContextualMenu(wxMenu* pMenu, lmGMObject* pGMO)
{
    // Add 'Attach text' item if it is a StaffObj or a AuxObj
    if (this->IsComponentObj())
    {
#if defined(__WXMSW__) || defined(__WXGTK__)

		wxMenuItem* pItem;
		wxSize nIconSize(16, 16);

        pMenu->AppendSeparator();

		pItem = new wxMenuItem(pMenu, lmPOPUP_AttachText, _("Attach text"));
		pItem->SetBitmap( wxArtProvider::GetBitmap(_T("tool_add_text"), wxART_TOOLBAR, nIconSize) );
		pMenu->Append(pItem);

#else
        pMenu->AppendSeparator();
        pMenu->Append(lmPOPUP_AttachText, _("Attach text"));

#endif
    }
}

void lmScoreObj::OnProperties(lmController* pController, lmGMObject* pGMO)
{
	if(!pGMO)
		wxMessageBox(_T("Nothing selected!"));
	else
	{
		lmDlgProperties dlg(pController);

		//give opportunity to derived classes to add specific panels
		OnEditProperties(&dlg);

		//prepare dialog and show it
		dlg.Layout();
		dlg.ShowModal();
	}
}

void lmScoreObj::OnEditProperties(lmDlgProperties* pDlg, const wxString& sTabName)
{
	pDlg->AddPanel( new lmGeneralProperties(pDlg->GetNotebook(), this, this->GetScore()),
				_("General"));
}

int lmScoreObj::GetPageNumber()
{
    //For visual highlight we need to know the page in wich the StaffObj to highlight
    //is located. To get it we are going to access this object main shape.
	//Returns the page number in whith the shape for this ScoreObj is rendered
	//if no shape returns 0

    lmShape* pGMObj = GetShape();
	if (!pGMObj) return 0;
	return pGMObj->GetPageNumber();
}

lmShape* lmScoreObj::GetShape(int nStaff)
{
    //Single shape ScoreObjs use a simple shape manager. Idx is ignored as there is
    //only one shape.
    //There are two behaviours for multi-shape ScoreObjs (clefs, time & key signatures). 
    //For clefs, there is a shape for each system. Real object corresponds to Idx=0
    //For key signatures there is a shape for each staff and system. Indexes are 
    //computed as nIdx = (nSystem -1) * numStaves + nStaff - 1
    //For time signatures there is a shape only for each staff in first system. Indexes
    //are computed as nIdx = nStaff - 1

    //This is the basic implementation, is valid for single shape ScoreObjs and for
    //time & key signatures. Therfore, it is overriden by Clefs

    return (lmShape*)m_pShapesMngr->GetGraphicObject(nStaff - 1);
}

wxFont* lmScoreObj::GetSuitableFont(lmPaper* pPaper)
{
	//returns the font to use to render this ScoreObj
	return (wxFont*)NULL;
}

lmUPoint lmScoreObj::SetReferencePos(lmPaper* pPaper)
{
    m_uPaperPos.x = pPaper->GetCursorX();
    m_uPaperPos.y = pPaper->GetCursorY();
	return m_uPaperPos;
}

void lmScoreObj::SetReferencePos(lmUPoint& uPos)
{
    m_uPaperPos = uPos;
}

void lmScoreObj::RecordHistory(lmUndoData* pUndoData) 
{
	if (!m_fModified) return;

	//once the changes are logged, consolidate new state
	AcceptChanges();
	m_fModified = false;
}

void lmScoreObj::AcceptChanges() 
{
	m_fModified = false;
}

wxString lmScoreObj::Dump()
{
	wxString sDump = _T("");

    ////position info
    //sDump += wxString::Format(_T(", ComputedPos=(%.2f, %.2f), UserShift=(%.2f, %.2f)"),
    //                m_uComputedPos.x, m_uComputedPos.y, m_uUserShift.x, m_uUserShift.y );
	return sDump;
}

wxString lmScoreObj::SourceLDP(int nIndent)
{
	wxString sSource = _T("");

    //TODO: Code for multi-shaped objects

	//location
    lmUPoint uUserShift = this->GetUserShift(0);
    if (uUserShift.x != 0.0f)
		sSource += wxString::Format(_T(" dx:%s"),
					DoubleToStr((double)m_pParent->LogicalToTenths(uUserShift.x), 4).c_str() );

	if (uUserShift.y != 0.0f)
		sSource += wxString::Format(_T(" dy:%s"),
					DoubleToStr((double)m_pParent->LogicalToTenths(uUserShift.y), 4).c_str() );

	return sSource;
}

wxString lmScoreObj::SourceXML(int nIndent)
{
    //TODO
	wxString sSource = _T("");
	return sSource;
}




//-------------------------------------------------------------------------------------------------
// lmComponentObj implementation
//-------------------------------------------------------------------------------------------------

//implementation of the StaffObjs List
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(StaffObjsList);

//implementation of the AuxObjs List
WX_DEFINE_LIST(AuxObjsList);

static int m_IdCounter = 0;        //to assign unique IDs to ComponentObjs

lmComponentObj::lmComponentObj(lmScoreObj* pParent, lmEComponentObjType nType, lmLocation* pPos,
                               bool fIsDraggable)
    : lmScoreObj(pParent)
{
    m_nId = m_IdCounter++;      // give it an ID
    m_nType = nType;            // save type
    m_tPos = *pPos;

    // behaviour
    m_fIsDraggable = fIsDraggable;
}

lmComponentObj::~lmComponentObj()
{
}

//wxString lmComponentObj::SourceLDP_Location(lmUPoint uPaperPos)
//{
//    wxString sSource = _T("");
//
  //  // X position
  //  wxString sPosX = _T("");
  //  if (m_tPos.xType != lmLOCATION_DEFAULT && m_tPos.xType != lmLOCATION_COMPUTED)
  //  {
  //      //value
  //      if (m_tPos.xType == lmLOCATION_USER_RELATIVE)
		//{
		//	if (m_tPos.xUnits == lmLUNITS)
		//		sPosX = wxString::Format(_T("dx:%s"),
		//					DoubleToStr((double)LogicalToTenths(m_tPos.x), 4).c_str() );
		//	else if (m_tPos.xUnits == lmTENTHS)
		//		sPosX = wxString::Format(_T("dx:%s"), DoubleToStr((double)m_tPos.x, 4).c_str() );
		//}
  //      else
		//{
		//	//absolute. Convert to relative
		//	if (m_tPos.xUnits == lmLUNITS)
		//		sPosX = wxString::Format(_T("dx:%s"),
		//					DoubleToStr((double)LogicalToTenths(m_tPos.x - uPaperPos.x), 4).c_str() );
		//	else if (m_tPos.xUnits == lmTENTHS)
		//		sPosX = wxString::Format(_T("dx:%.4f"),
		//					DoubleToStr((double)(m_tPos.x - LogicalToTenths(uPaperPos.x)), 4).c_str() );
		//}

  //      //units
  //      wxString sUnits = _T("");
  //      if (m_tPos.xUnits != lmTENTHS)
  //      {
  //      }
  //      sPosX += sUnits;
  //  }

  //  if (sPosX != _T(""))
  //  {
  //      sSource += _T(" ");
  //      sSource += sPosX;
  //      sSource += _T("");
  //  }

  //  // Y position
  //  wxString sPosY = _T("");
  //  if (m_tPos.yType != lmLOCATION_DEFAULT && m_tPos.yType != lmLOCATION_COMPUTED)
  //  {
  //      //value
  //      if (m_tPos.yType == lmLOCATION_USER_RELATIVE)
		//{
		//	if (m_tPos.xUnits == lmLUNITS)
		//		sPosY = wxString::Format(_T("dy:%s"),
		//						DoubleToStr((double)LogicalToTenths(m_tPos.y), 4).c_str() );
		//	else if (m_tPos.xUnits == lmTENTHS)
		//		sPosY = wxString::Format(_T("dy:%s"), DoubleToStr((double)m_tPos.y, 4).c_str() );
		//}
  //      else
		//{
		//	//absolute. Convert to relative
		//	if (m_tPos.yUnits == lmLUNITS)
		//		sPosY = wxString::Format(_T("dy:%s"),
		//					DoubleToStr((double)LogicalToTenths(m_tPos.y - uPaperPos.y), 4).c_str() );
		//	else if (m_tPos.yUnits == lmTENTHS)
		//		sPosY = wxString::Format(_T("dy:%s"),
		//					DoubleToStr((double)(m_tPos.y - LogicalToTenths(uPaperPos.y)), 4).c_str() );
		//}

  //      //units
  //      wxString sUnits = _T("");
  //      if (m_tPos.yUnits != lmTENTHS)
  //      {
  //      }
  //      sPosY += sUnits;
  //  }

  //  if (sPosY != _T(""))
  //  {
  //      sSource += _T(" ");
  //      sSource += sPosY;
  //      sSource += _T("");
  //  }
//
//    return sSource;
//}

lmUPoint lmComponentObj::ComputeObjectLocation(lmPaper* pPaper)
{
	lmUPoint uPos = GetReferencePaperPos();

#if 1
	return ComputeBestLocation(uPos, pPaper);

#else

	////if default location, ask derived object to compute the best position for itself
 //   if (m_tPos.xType == lmLOCATION_DEFAULT || m_tPos.yType == lmLOCATION_DEFAULT)
	//	uPos = ComputeBestLocation(uPos, pPaper);
	//else if (m_tPos.xType == lmLOCATION_COMPUTED || m_tPos.yType == lmLOCATION_COMPUTED)
	//{
	//	m_tPos = m_tSrcPos;
	//	uPos = ComputeBestLocation(uPos, pPaper);
	//}


 //   if (m_tPos.xType == lmLOCATION_DEFAULT)
	//{
	//	//use the computed best location
	//	m_tPos.x = uPos.x;
	//	m_tPos.xType = lmLOCATION_COMPUTED;
	//	m_tPos.xUnits = lmLUNITS;
 //   }

	//else if (m_tPos.xType == lmLOCATION_COMPUTED)
	//{
	//	//the default position was computed in a previous invocation. Use it
	//	//The computed location is always absolute, in tenths
	//	uPos.x = m_tPos.x;
 //   }

	//else if (m_tPos.xType == lmLOCATION_USER_ABSOLUTE)
	//{
	//	//the position was fixed by user (either in source file or by dragging object)
	//	//Use it
	//	if (m_tPos.xUnits == lmLUNITS)
	//		uPos.x = m_tPos.x;
	//	else if (m_tPos.xUnits == lmTENTHS)
	//		uPos.x = TenthsToLogical( m_tPos.x );
	//}

	//else if (m_tPos.xType == lmLOCATION_USER_RELATIVE)
	//{
	//	//the position was fixed by user (either in source file or by dragging object)
	//	//Use it
	//	if (m_tPos.xUnits == lmLUNITS)
	//		uPos.x += m_tPos.x;
	//	else if (m_tPos.xUnits == lmTENTHS)
	//		uPos.x += TenthsToLogical( m_tPos.x );
	//}
	//else
	//	wxASSERT(false);


 //   if (m_tPos.yType == lmLOCATION_DEFAULT)
	//{
	//	//use the computed best location
	//	m_tPos.y = uPos.y;
	//	m_tPos.yType = lmLOCATION_COMPUTED;
	//	m_tPos.yUnits = lmLUNITS;
 //   }

	//else if (m_tPos.yType == lmLOCATION_COMPUTED)
	//{
	//	//the position was computed in a previous invocation or was fixed by user.
	//	//Use it
	//	//The computed location is always absolute, in tenths
	//	uPos.y = m_tPos.y;
 //   }

	//else if (m_tPos.yType == lmLOCATION_USER_ABSOLUTE)
	//{
	//	//the position was fixed by user (either in source file or by dragging object)
	//	//Use it
	//	if (m_tPos.yUnits == lmLUNITS)
	//		uPos.y = m_tPos.y;
	//	else if (m_tPos.yUnits == lmTENTHS)
	//		uPos.y = TenthsToLogical( m_tPos.y );
	//}

	//else if (m_tPos.yType == lmLOCATION_USER_RELATIVE)
	//{
	//	//the position was fixed by user (either in source file or by dragging object)
	//	//Use it
	//	if (m_tPos.yUnits == lmLUNITS)
	//		uPos.y += m_tPos.y;
	//	else if (m_tPos.xUnits == lmTENTHS)
	//		uPos.y += TenthsToLogical( m_tPos.y );
	//}
	//else
	//	wxASSERT(false);

	//return uPos;
#endif

}



//-------------------------------------------------------------------------------------------------
// lmStaffObj implementation
//-------------------------------------------------------------------------------------------------

lmStaffObj::lmStaffObj(lmScoreObj* pParent, EStaffObjType nType, lmVStaff* pStaff, int nStaff,
                   bool fVisible, bool fIsDraggable) :
    lmComponentObj(pParent, lm_eStaffObj, &g_tDefaultPos, fIsDraggable)
{
    // store parameters
    m_fVisible = fVisible;
    m_nClass = nType;

    // initializations: staff ownership info
    m_pVStaff = pStaff;
    m_nStaffNum = (pStaff ? nStaff : 0);

}

lmStaffObj::~lmStaffObj()
{
}

lmScore* lmStaffObj::GetScore()
{ 
    return m_pVStaff->GetScore(); 
}

void lmStaffObj::Layout(lmBox* pBox, lmPaper* pPaper, wxColour colorC, bool fHighlight)
{
    PrepareToCreateShapes();

	lmUPoint uOrg = SetReferencePos(pPaper);
	m_uComputedPos = ComputeObjectLocation(pPaper);			// compute location

	lmLUnits uWidth;
    if (m_fVisible)
	{
		//SetFont(pPaper);										// set the font
		// layout derived object
        uWidth = LayoutObject(pBox, pPaper, m_uComputedPos, colorC);
	}
	else
	{
		//Create an invisible shape, to store the StaffObj position
		lmShapeInvisible* pShape = new lmShapeInvisible(this, uOrg, lmUSize(0.0, 0.0) );
		pBox->AddShape(pShape);
		StoreShape(pShape);
		uWidth = 0;
	}

	////if user defined position shift the shape
	//if (m_pGMObj && m_uUserShift.x != 0.0f || m_uUserShift.y != 0.0f)
	//	m_pGMObj->Shift(m_uUserShift.x, m_uUserShift.y);

	// layout AuxObjs attached to this StaffObj
    if (m_pAuxObjs)
    {
	    for (int i=0; i < (int)m_pAuxObjs->size(); i++)
	    {
		    //assign m_uComputedPos as paper pos. for this AuxObj
		    pPaper->SetCursorX(m_uComputedPos.x);
		    pPaper->SetCursorY(m_uComputedPos.y);

		    (*m_pAuxObjs)[i]->Layout(pBox, pPaper, colorC, fHighlight);
	    }
    }

    // update paper cursor position
    pPaper->SetCursorX(uOrg.x + uWidth);
}

wxFont* lmStaffObj::GetSuitableFont(lmPaper* pPaper)
{
	WXUNUSED(pPaper);
    wxASSERT(m_pVStaff);
    wxASSERT(m_nStaffNum > 0);
    lmStaff* pStaff = m_pVStaff->GetStaff(m_nStaffNum);
    return pStaff->GetFontDraw();
}

lmLUnits lmStaffObj::TenthsToLogical(lmTenths nTenths)
{
    return m_pVStaff->TenthsToLogical(nTenths, m_nStaffNum);
}

lmTenths lmStaffObj::LogicalToTenths(lmLUnits uUnits)
{
    return m_pVStaff->LogicalToTenths(uUnits, m_nStaffNum);
}

lmContext* lmStaffObj::GetCurrentContext()
{
	// Returns the context that is applicable to the this StaffObj.
	// AWARE: Only Clef, key signature and time signature are updated. To get
	//	applicable accidentals use NewUpdatedContext() instead.
	return m_pVStaff->GetCurrentContext(this);
}

lmContext* lmStaffObj::NewUpdatedContext()
{
	//returns the applicable context for this StaffObj, updated with all
	//accidentals introduced by previous notes
	return m_pVStaff->NewUpdatedContext(this);
}

lmClef* lmStaffObj::GetApplicableClef()
{
    //returns the TS applicable to this staffobj, if any

    lmContext* pContext = GetCurrentContext();
    if (pContext)
        return pContext->GetClef();
    else
        return (lmClef*)NULL;
}

lmTimeSignature* lmStaffObj::GetApplicableTimeSignature()
{
    //returns the TS applicable to this staffobj, if any

    lmContext* pContext = GetCurrentContext();
    if (pContext)
        return pContext->GetTime();
    else
        return (lmTimeSignature*)NULL;
}

lmKeySignature* lmStaffObj::GetApplicableKeySignature()
{
    //returns the Key Signature applicable to this staffobj, if any

    lmContext* pContext = GetCurrentContext();
    if (pContext)
        return pContext->GetKey();
    else
        return (lmKeySignature*)NULL;
}


wxString lmStaffObj::SourceLDP(int nIndent)
{
	wxString sSource = _T("");

    //staff num
    if (m_pVStaff->GetNumStaves() > 1
        && !IsKeySignature()
        && !IsTimeSignature())
    {
        sSource += wxString::Format(_T(" p%d"), m_nStaffNum);
    }
    
    //visible?
    if (!m_fVisible) { sSource += _T(" noVisible"); }

    // Generate source code for AuxObjs attached to this StaffObj
    if (m_pAuxObjs)
    {
		nIndent++;
        for (int i=0; i < (int)m_pAuxObjs->size(); i++)
        {
            sSource += (*m_pAuxObjs)[i]->SourceLDP(nIndent);
        }
		nIndent--;
    }

    //base class info
    sSource += lmScoreObj::SourceLDP(nIndent);
	return sSource;
}

wxString lmStaffObj::SourceXML(int nIndent)
{
    // Generate source code for AuxObjs attached to this StaffObj
	wxString sSource = _T("");
    if (m_pAuxObjs)
    {
        for (int i=0; i < (int)m_pAuxObjs->size(); i++)
        {
            sSource += (*m_pAuxObjs)[i]->SourceXML(nIndent);
        }
    }
	return sSource;
}

wxString lmStaffObj::Dump()
{
    //staff
    wxString sDump = wxString::Format(_T(", staff=%d"), m_nStaffNum);

    //base class info
	sDump += lmScoreObj::Dump();

    // Dump AuxObjs attached to this StaffObj
    if (m_pAuxObjs)
    {
        for (int i=0; i < (int)m_pAuxObjs->size(); i++)
        {
            sDump += (*m_pAuxObjs)[i]->Dump();
        }
    }
	return sDump;
}




//-------------------------------------------------------------------------------------------------
// lmShapesMngr implementation
//-------------------------------------------------------------------------------------------------

lmShapesMngr::lmShapesMngr()
{
    m_uUserShift = lmUPoint(0.0f, 0.0f);
    m_pGMObj = (lmShape*)NULL;
}

lmShapesMngr::~lmShapesMngr()
{
}

lmGMObject* lmShapesMngr::GetGraphicObject(int nShapeIdx)
{
    //default implementation assumes that an ScoreObj only has
    //one associated grapic object, ant it is pointed by m_pGMObj

    WXUNUSED(nShapeIdx);
    return m_pGMObj;
}

void lmShapesMngr::SaveUserLocation(lmLUnits xPos, lmLUnits yPos, int nShapeIdx)
{
    //default implementation for virtual method.
    //It assumes that ScoreObj only has a shape. 

    WXUNUSED(nShapeIdx);
	m_uUserShift.x = xPos;
	m_uUserShift.y = yPos;
}


lmUPoint lmShapesMngr::GetUserShift(int nShapeIdx)
{
    //default implementation for virtual method.
    //It assumes that ScoreObj only has a shape. 

    WXUNUSED(nShapeIdx);
    return m_uUserShift;
}





//-------------------------------------------------------------------------------------------------
// lmMultiShapesMngr implementation
//-------------------------------------------------------------------------------------------------

lmMultiShapesMngr::lmMultiShapesMngr()
{
    Init();
}

lmMultiShapesMngr::~lmMultiShapesMngr()
{
    std::vector<lmShapeInfo*>::iterator it = m_ShapesInfo.begin();
    while (it != m_ShapesInfo.end())
    {
        delete *it;
        ++it;
    }
}

void lmMultiShapesMngr::Init()
{
    m_nNextIdx = 0;

    //remove pointers to already deleted shapes
    std::vector<lmShapeInfo*>::iterator it = m_ShapesInfo.begin();
    while (it != m_ShapesInfo.end())
    {
        (*it)->pGMObj = (lmGMObject*)NULL;
        ++it;
    }
}

void lmMultiShapesMngr::StoreShape(lmGMObject* pGMObj)
{
    int nIdx = pGMObj->GetOwnerIDX();
    if (nIdx == (int)m_ShapesInfo.size())
    {
        lmShapeInfo* pShapeInfo = new lmShapeInfo;
        pShapeInfo->pGMObj = pGMObj;
        pShapeInfo->uUserShift = lmUPoint(0.0f, 0.0f);
        m_ShapesInfo.push_back(pShapeInfo);
    }
    else
    {
        wxASSERT(m_ShapesInfo[nIdx]->pGMObj == (lmGMObject*)NULL); 
        m_ShapesInfo[nIdx]->pGMObj = pGMObj;
    }
}

lmGMObject* lmMultiShapesMngr::GetGraphicObject(int nShapeIdx)
{
    //For KeySignatures shape index is staff number (1..n) minus 1

    if (m_ShapesInfo.size() == 0) return (lmGMObject*)NULL;

    wxASSERT(nShapeIdx < (int)m_ShapesInfo.size());
    return m_ShapesInfo[nShapeIdx]->pGMObj;
}

void lmMultiShapesMngr::SaveUserLocation(lmLUnits xPos, lmLUnits yPos, int nShapeIdx)
{
    //if necessary, create empty shapes info entries
    int nToAdd = nShapeIdx - (int)m_ShapesInfo.size() + 1;
    for (int i=0; i < nToAdd; ++i)
    {
        lmShapeInfo* pShapeInfo = new lmShapeInfo;
        pShapeInfo->pGMObj = (lmGMObject*)NULL;
        pShapeInfo->uUserShift = lmUPoint(0.0f, 0.0f);
        m_ShapesInfo.push_back(pShapeInfo);
    }

    //save new user position
    m_ShapesInfo[nShapeIdx]->uUserShift = lmUPoint(xPos, yPos);
}

lmUPoint lmMultiShapesMngr::GetUserShift(int nShapeIdx)
{
    wxASSERT(nShapeIdx < (int)m_ShapesInfo.size());
    return m_ShapesInfo[nShapeIdx]->uUserShift;
}
