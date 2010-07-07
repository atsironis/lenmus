//--------------------------------------------------------------------------------------
//  LenMus Library
//  Copyright (c) 2010 LenMus project
//
//  This program is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License along
//  with this library; if not, see <http://www.gnu.org/licenses/> or write to the
//  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//  MA  02111-1307,  USA.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#include "lenmus_document.h"
#include "lenmus_document_cursor.h"
#include "lenmus_elements.h"
#include "lenmus_score_iterator.h"
#include "lenmus_internal_model.h"
#include "lenmus_time.h"

using namespace std;

namespace lenmus
{

//-------------------------------------------------------------------------------------
// DocCursor implementation
//-------------------------------------------------------------------------------------

DocCursor::DocCursor(Document* pDoc)
    : m_pDoc(pDoc)
    , m_it(pDoc)
    , m_pCursor(NULL)
{
    m_it.start_of_content();
    m_pFirst = *m_it;
} 

DocCursor::~DocCursor()
{
    if (m_pCursor)
        delete m_pCursor;
}

DocCursor::DocCursor(const DocCursor& cursor)
    : m_pDoc(cursor.m_pDoc)
    , m_it(m_pDoc)
{
    m_it = cursor.m_it;
    m_pFirst = cursor.m_pFirst;
    if (cursor.m_pCursor == NULL)
        m_pCursor = NULL;
    else
    {
        start_delegation();
        DocCursor &cRef = const_cast<DocCursor&>(cursor);
        m_pCursor->point_to( cRef.get_pointee() );
    }
}

DocCursor& DocCursor::operator= (DocCursor const& cursor)
{
    if (this != &cursor)
    {
        assert(m_pDoc == cursor.m_pDoc);
        m_it = cursor.m_it;
        m_pFirst = cursor.m_pFirst;
        if (cursor.m_pCursor == NULL)
            m_pCursor = NULL;
        else
        {
            start_delegation();
            DocCursor &cRef = const_cast<DocCursor&>(cursor);
            m_pCursor->point_to( cRef.get_pointee() );
        }
    }
    return *this;
}

LdpElement* DocCursor::get_pointee()
{
	return (is_delegating() ? m_pCursor->get_pointee() : *m_it);
}

void DocCursor::enter_element()
{
    if (!is_delegating())
        start_delegation();
}

void DocCursor::start_delegation()
{
    //Factory method to create delegate cursors
    if ((*m_it)->is_type(k_score))
        m_pCursor = new ScoreCursor(m_pDoc, *m_it);
}

void DocCursor::stop_delegation()
{
    delete m_pCursor;
    m_pCursor = NULL;
}

void DocCursor::next()
{
    if (is_at_end_of_child())
    {
        stop_delegation();
        ++m_it;
    }
    else if (is_delegating())
	{
		m_pCursor->next();
	}
	else
		++m_it;
}

void DocCursor::prev()
{
    if (is_delegating())
    {
        m_pCursor->prev();
        if (m_pCursor->get_pointee() == NULL)
        {
            stop_delegation();
            //m_it.exit_element();
        }
        else
            --m_it;
    }
    else if (*m_it != m_pFirst)
    {
        if (*m_it != NULL)
            --m_it;
        else
            m_it.last_of_content();
    }
}

void DocCursor::point_to(long nId)
{
    if (is_delegating())
		m_pCursor->point_to(nId);
	else
    {
        m_it.start_of_content();
        while (*m_it != NULL && (*m_it)->get_id() != nId) 
            ++m_it;
    }
}



//-------------------------------------------------------------------------------------
// ScoreCursor implementation
//-------------------------------------------------------------------------------------

ScoreCursor::ScoreCursor(Document* pDoc, LdpElement* pScoreElm)
    : m_pDoc(pDoc)
    , m_pScore( dynamic_cast<ImScore*>(pScoreElm->get_imobj()) )
    , m_pColStaffObjs( m_pScore->get_staffobjs_table() )
{
    start();
} 

ScoreCursor::~ScoreCursor()
{
}

void ScoreCursor::next() 
{
    if (m_it == m_pColStaffObjs->end())
        m_it = m_pColStaffObjs->begin();
    else
        ++m_it;
}

void ScoreCursor::prev() 
{
    if (m_it == m_pColStaffObjs->begin())
        m_it = m_pColStaffObjs->end();
    else
        --m_it;
}

void ScoreCursor::start() 
{
    m_it = m_pColStaffObjs->begin();
    m_nInstr = 0;
    m_nStaff = 0;
    m_rTime = 0.0f;
    m_nSegment = 0;
}

LdpElement* ScoreCursor::get_pointee() 
{ 
    if (m_it != m_pColStaffObjs->end())
        return (*m_it)->element();
    else
        return NULL;
}

void ScoreCursor::point_to(LdpElement* pElm)
{
    m_it = m_pColStaffObjs->begin();
    for (; m_it != m_pColStaffObjs->end() && (*m_it)->element() != pElm; ++m_it);
    update_state();
}

void ScoreCursor::point_to(long nId)
{
    m_it = m_pColStaffObjs->begin();
    for (; m_it != m_pColStaffObjs->end() && (*m_it)->element_id() != nId; ++m_it);
    update_state();
}

void ScoreCursor::point_to_barline(long nId, int nStaff)
{
    point_to(nId);
    m_nStaff = nStaff;
}

void ScoreCursor::to_state(int nInstr, int nSegment, int nStaff, float rTime)
{
    m_it = m_pColStaffObjs->begin();
    forward_to_state(nInstr, nSegment, nStaff, rTime);
}

void ScoreCursor::forward_to_state(int nInstr, int nSegment, int nStaff, float rTime)
{
    m_nInstr = nInstr;
    m_nSegment = nSegment;
    m_nStaff = nStaff;

    forward_to_instr_segment_with_time_not_lower_than(rTime);
    m_rTime = rTime;

    forward_to_current_staff();
}

void ScoreCursor::move_next()
{
    //Implements user expectations when pressing 'cursor right' key: move cursor to
    //next timepos in current instrument. Cursor remains in current staff even if
    //the timepos is not occupied in that staff. When in last timepos, moves to next
    //logical timepos (current timepos + object duration). When end of staff is
    //reached:
    // - if current instrument has more staves,
    //   advance to next staff, to first object in first segment.
    // - else to first staff of next instrument.
    // - If no more instruments, remains at end of score

    if (is_at_end_of_staff())
    {
        if (more_staves_in_instrument())
            to_start_of_next_staff();
        else if (more_instruments())
            to_start_of_next_instrument();
        else     //we are at end of score
            return;     //remain there
    }
    else if (try_next_at_same_time())
        return;
    else
        forward_to_next_time();
}

bool ScoreCursor::more_staves_in_instrument()
{
	ImInstrument* pInstr = m_pScore->get_instrument(m_nInstr);
    int numStaves = pInstr->get_num_staves();
	return (m_nStaff < numStaves - 1);
}

void ScoreCursor::to_start_of_next_staff()
{
    m_nSegment = 0;
    m_rTime = 0.0f;
    m_nStaff++;

    to_state(m_nInstr, m_nSegment, m_nStaff, m_rTime);
}

bool ScoreCursor::more_instruments()
{
	int numInstruments = m_pScore->get_num_instruments();
    return m_nInstr < numInstruments - 1;
}

void ScoreCursor::to_start_of_next_instrument()
{
    m_nInstr++;
    m_nSegment = 0;
    m_nStaff = 0;
    m_rTime = 0.0f;

    to_state(m_nInstr, m_nSegment, m_nStaff, m_rTime);
}

void ScoreCursor::forward_to_next_time()
{
    float rTargetTime = determine_next_target_time();
    int nTargetSegment = determine_next_target_segment();

    forward_to_state(m_nInstr, nTargetSegment, m_nStaff, rTargetTime);
}

void ScoreCursor::forward_to_current_staff()
{
    if (!is_at_end_of_staff() && !ref_object_is_on_staff(m_nStaff))
    {
        ColStaffObjs::iterator  itLast = m_it;          //save current ref.object
        if ( !find_current_staff_at_current_ref_object_time() )
            m_it = itLast;      //not found. Go back to previous ref.object
    }
}

void ScoreCursor::update_state()
{
    if (there_is_ref_object())
    {
        m_nInstr = (*m_it)->num_instrument();
        m_nStaff = (*m_it)->staff();
        m_rTime = (*m_it)->time();
        m_nSegment = (*m_it)->segment();
    }
}

bool ScoreCursor::try_next_at_same_time()
{
    //try to move to a possible next object at same time

    next();
    if ( there_is_ref_object()
         && ref_object_is_on_instrument(m_nInstr)
         && ref_object_is_on_staff(m_nStaff)
         && is_equal_time(ref_object_time(), m_rTime) )
    {
        return true;     //next object found. done
    }
    prev();
    return false;   
}

float ScoreCursor::determine_next_target_time()
{
    if (is_pointing_object())
    {
        if (ref_object_is_barline())
            return 0.0f;
        else
            return ref_object_time() + ref_object_duration();
    }
    else
    {
        if ( is_equal_time(m_rTime, ref_object_time()) )
            return ref_object_time() + ref_object_duration();
        else
            return ref_object_time();
    }
}

int ScoreCursor::determine_next_target_segment()
{
    if (is_pointing_barline())
        return ref_object_segment() + 1;
    else
        return m_nSegment;
}

void ScoreCursor::forward_to_instr_segment_with_time_not_lower_than(float rTargetTime)
{
    while (there_is_ref_object()
           && ( !ref_object_is_on_instrument(m_nInstr)
                || !ref_object_is_on_segment(m_nSegment)
                || is_greater_time(rTargetTime, ref_object_time()) ))
    {
        next();
    }
}

bool ScoreCursor::find_current_staff_at_current_ref_object_time()
{
    float rCurTime = ref_object_time();
    while (there_is_ref_object()
            && is_equal_time(rCurTime, ref_object_time())
            && ( !ref_object_is_on_instrument(m_nInstr)
                || !ref_object_is_on_staff(m_nStaff) ))
    {
        next();
    }

    return there_is_ref_object()
           && ref_object_is_on_instrument(m_nInstr)
           && is_equal_time(rCurTime, ref_object_time())
           && ref_object_is_on_staff(m_nStaff);
}

void ScoreCursor::move_next_new_time()
{
    // move to next object but with different time than current one
    //Behaviour is as move_next but repeats while new time is equal than current time.
}

void ScoreCursor::move_prev()
{
    //Implements user expectations when pressing 'cursor left' key: move cursor to
    //previous time in current instrument. 
    //Cursor will always stop in each measure at timepos 0 (even if no objects 
    //there) and then move to prev measure and stop before barline.
    //If cursor is at start of score will remain there.
    //When cursor is at start of staff:
    // - if current instrument has more staves,
    //   goes back to end of previous staff.
    // - else to end of last staff of previous instrument.

    if (is_at_start_of_score())
        return;         //remain there
    else if (try_prev_at_same_time())
        return;         //prev object found. done
    else if (is_at_start_of_staff())
    {
        if (!is_first_staff_of_current_instrument())
            to_end_of_prev_staff();
        else if (!is_first_instrument())
            to_end_of_prev_instrument();
    }
    else
        backward_to_prev_time();
}

void ScoreCursor::backward_to_prev_time()
{
    //back_to_different_time
    prev();
    while ( there_is_ref_object() && is_equal_time(ref_object_time(), m_rTime) )
        prev();

    //change segment if we are going to cross a barline
    if (is_equal_time(m_rTime, 0.0f))
        m_nSegment--;

    //back to current instrument
    while ( there_is_ref_object() 
            && !ref_object_is_on_instrument(m_nInstr) )
        prev();

    //set new current time
    m_rTime = ref_object_time();

    //back to start of current time
    while ( there_is_ref_object()
            && ref_object_is_on_instrument(m_nInstr)
            && is_equal_time(ref_object_time(), m_rTime) )
    {
        prev();
    }
    next();

    //forward to right staff
    forward_to_current_staff();

    //forward to last object in target staff & time
    if (there_is_ref_object())
    {
        next();
        while ( there_is_ref_object()
                && ref_object_is_on_instrument(m_nInstr)
                && ref_object_is_on_staff(m_nStaff)
                && is_equal_time(ref_object_time(), m_rTime) )
        {
            next();
        }
        prev();
    }
}

bool ScoreCursor::try_prev_at_same_time()
{
    //try to move to a possible prev object at same time

    prev();
    if ( there_is_ref_object()
         && ref_object_is_on_instrument(m_nInstr)
         && ref_object_is_on_staff(m_nStaff)
         && is_equal_time(ref_object_time(), m_rTime) )
    {
        return true;     //prev object found. done
    }
    next();
    return false;   
}

bool ScoreCursor::is_at_start_of_staff()
{
    return m_nSegment == 0 && is_equal_time(m_rTime, 0.0f);
}

void ScoreCursor::to_end_of_prev_staff()
{
    m_rTime = 0.0f;
    m_nStaff--;

    to_end_of_staff();
}

void ScoreCursor::to_end_of_prev_instrument()
{
    m_rTime = 0.0f;
    m_nInstr--;

    //determine last staff
	ImInstrument* pInstr = m_pScore->get_instrument(m_nInstr);
    m_nStaff = pInstr->get_num_staves() - 1;

    to_end_of_staff();
}

void ScoreCursor::to_end_of_staff()
{
    //determine last segment
    m_it = m_pColStaffObjs->end();
    prev();
    if (there_is_ref_object())
    {
        while ( there_is_ref_object()
                && !ref_object_is_on_instrument(m_nInstr) )
        {
            prev();
        }
        if (there_is_ref_object())
            m_nSegment = ref_object_segment() + 1;
        else
            m_nSegment = 0;
    }
    else
        m_nSegment = 0; 

    //move to end of staff
    m_it = m_pColStaffObjs->end();
}

void ScoreCursor::move_prev_new_time()
{
    // move to prev object but with dfferent time than current one.
    //Behaviour is as move_prev but repeats while new time is equal than current time.
}

void ScoreCursor::to_start_of_instrument(int nInstr)
{
    //to first obj in instr nInstr
    //Moves cursor to instrument nInstr (1..n), at first object.
    //[at timepos 0 after prolog] ?
}

void ScoreCursor::to_start_of_segment(int nSegment, int nStaff)
{
    //to first obj in specified segment and staff
    //Limited to current instrument. Move cursor to start of segment, 
    //that is to first SO and timepos 0. Set staff. Then, if fSkipClef, 
    //advances after last clef in this segment, if any. And then, if 
    //fSkipKey, advances after last key, if any.
}

void ScoreCursor::skip_clef_key_time()
{
    //while pointing clef, key or time, move next    
}

float ScoreCursor::ref_object_duration()
{ 
    ImStaffObj* pISO = dynamic_cast<ImStaffObj*>((*m_it)->element()->get_imobj());
    return pISO->get_duration();
}

bool ScoreCursor::ref_object_is_barline()
{
    ImBarline* pIBL = dynamic_cast<ImBarline*>((*m_it)->element()->get_imobj());
    return (pIBL != NULL);
}


}  //namespace lenmus