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
#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "ScoreView.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "wx/scrolbar.h"

#include "global.h"
#include "TheApp.h"
#include "MainFrame.h"
#include "ScoreDoc.h"
#include "ScoreView.h"
#include "ScoreCanvas.h"
#include "EditFrame.h"
#include "../widgets/Ruler.h"
#include "FontManager.h"
#include "ArtProvider.h"
#include "../graphic/BoxSlice.h"


// access to main frame
extern lmMainFrame* GetMainFrame();

// IDs for events, windows, etc.
enum
{
	// windows
    lmID_HSCROLL = 1000,
    lmID_VSCROLL,

	// cursos
	lmID_TIMER_CURSOR,
};

// global variables
bool gfDrawSelRec;        //draw selection rectangles around staff objects


// Dragging states
enum
{
	lmDRAG_NONE = 0,
	lmDRAG_START,
	lmDRAG_DRAGGING,
};

#define lmCURSOR_BLINKING_RATE  750		//cursor blinking rate = 750ms

IMPLEMENT_DYNAMIC_CLASS(lmScoreView, lmView)

BEGIN_EVENT_TABLE(lmScoreView, lmView)
    EVT_COMMAND_SCROLL	(lmID_HSCROLL, lmScoreView::OnScroll)
    EVT_COMMAND_SCROLL	(lmID_VSCROLL, lmScoreView::OnScroll)
    EVT_MOUSEWHEEL		(lmScoreView::OnMouseWheel)
	EVT_TIMER			(lmID_TIMER_CURSOR, lmScoreView::OnCursorTimer)

END_EVENT_TABLE()



lmScoreView::lmScoreView()
{
    m_pFrame = (lmEditFrame*) NULL;
    m_pCanvas = (lmScoreCanvas*) NULL;
    m_pDoc = (lmScoreDocument*) NULL;
    m_pHRuler = (lmRuler*) NULL;
    m_pVRuler = (lmRuler*) NULL;
    m_pHScroll = (wxScrollBar*) NULL;
    m_pVScroll = (wxScrollBar*) NULL;
    m_xScrollPosition = 0;
    m_yScrollPosition = 0;
    m_rScale = 1.0 * lmSCALE;

    // drag state control initializations
    m_dragState = lmDRAG_NONE;
    m_pDragImage = (wxDragImage*) NULL;

    //options
    m_fRulers = false;

    // view layout
    m_xBorder = 13;
    m_yBorder = 13;
    m_yInterpageGap = 17;

    // initializations
    m_numPages = 0;            // no pages yet
    m_xDisplayPixelsPerLU = 1.0;
    m_yDisplayPixelsPerLU = 1.0;
    m_xScrollPosition = 0;
    m_yScrollPosition = 0;

	//cursor initializations
	m_oCursorTimer.SetOwner(this, lmID_TIMER_CURSOR);
	m_pCursorInstr = (lmInstrument*)NULL;
	m_nCursorStaff = 1;
	m_nCursorMeasure = 1;
	m_nCursorTime = 0;
	m_pCursorSO = (lmStaffObj*)NULL;
    m_nCursorIdSO = -1;
    m_fCursorEnabled = true;
    m_fCursorShown = false;
    m_pCursorIT = (lmStaffObjIterator*)NULL;

}

lmScoreView::~lmScoreView()
{
    if (m_pCursorIT)
        delete m_pCursorIT;
}

bool lmScoreView::OnCreate(wxDocument* doc, long WXUNUSED(flags) )
{
    // The OnCreate function, called when the window is created
    // When a view is created (via main menu 'file > new'  or 'file > open') class wxDocTemplate
    // invokes ::CreateDocument and ::CreateView. This last one invokes ::OnCreate
    // In this method a child MDI frame is created, populated with the
    // needed controls and shown.

    //save the document
    m_pDoc = (lmScoreDocument*)doc;

    // create the frame and set its icon and default title
    m_pFrame = new lmEditFrame(doc, this, GetMainFrame());
    m_pFrame->SetIcon( wxArtProvider::GetIcon(_T("app_score"), wxART_TOOLBAR, wxSize(16,16)) );

    // Set frame title: the score title
    lmScore* pScore = m_pDoc->GetScore();
    if (pScore)
        m_pFrame->SetTitle( pScore->GetScoreName() );
    else
        m_pFrame->SetTitle(_T("New score"));

    //m_colorBg = wxColour(10,36,106);        //deep blue
    //wxColour colorBg(200, 200, 200);    // light grey
    m_colorBg = wxColour(127, 127, 127);    // dark grey

    //m_pFrame->SetBackgroundColour(wxColour(10,36,106));

    //rulers
    m_fRulers = GetMainFrame()->ShowRulers();



#ifdef __X__
    // X seems to require a forced resize
    int x, y;
    m_pFrame->GetSize(&x, &y);
    m_pFrame->SetSize(-1, -1, x, y);
#endif

    //-- Create the controls on the frame's window

    // create the two rulers, and hide them if user has not selected rulers
    m_pHRuler = new lmRuler(m_pFrame, this, -1, wxHORIZONTAL, wxPoint(0,0), 300);
    m_pVRuler = new lmRuler(m_pFrame, this, -1, wxVERTICAL, wxPoint(0,0), 300);
    if (!m_fRulers) {
        m_pHRuler->Hide();
        m_pVRuler->Hide();
    }

    // add an offset of 2 pixels to the rulers to take into account the canvas border
    m_pHRuler->SetOffset(2 - m_xBorder);
    m_pVRuler->SetOffset(2);

    // create the canvas for the score to edit
    m_pCanvas = new lmScoreCanvas(this, m_pFrame, m_pDoc, wxPoint(0, 0), m_pFrame->GetSize(),
                        wxNO_BORDER, m_colorBg );

    // create the scrollbars
    m_pHScroll = new wxScrollBar(m_pFrame, lmID_HSCROLL, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL);
    m_pVScroll = new wxScrollBar(m_pFrame, lmID_VSCROLL, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);

    SetScale(1.0);            // to create the font and resize controls and scrollbars

    //show the frame
    m_oCursorTimer.Start(lmCURSOR_BLINKING_RATE, wxTIMER_CONTINUOUS);
	m_pFrame->Show(true);
    Activate(true);

    return true;
}

void lmScoreView::ResizeControls()
{
    // Get client area
    lmPixels dxFrame, dyFrame;
    m_pFrame->GetClientSize(&dxFrame, &dyFrame);
    lmPixels dxFree = dxFrame,
             dyFree = dyFrame;


    // Discount scrollbars
    lmPixels dxVScroll = m_pVScroll->GetSize().GetWidth(),
             dyHScroll = m_pHScroll->GetSize().GetHeight();

    dxFree -= dxVScroll;
    dyFree -= dyHScroll;

    // Discount rulers width plus 2 pixels for separation
    lmPixels dxVRuler = 0, dyHRuler = 0;
    if (m_fRulers) {
        wxASSERT(m_pHRuler && m_pVRuler);
        dxVRuler = m_pVRuler->GetWidth() + 2;
        dyHRuler = m_pHRuler->GetWidth() + 2;
        dxFree -= dxVRuler;
        dyFree -= dyHRuler;
    }

    // Compute available space for the view and set view margin so that
    // the view is centered
    dxFree -= m_xPageSizeD;
    m_xBorder = (dxFree > 0 ? dxFree/2 : 13);


    // Move controls to the computed positions
    lmPixels xLeft = 0, yTop = 3;
    lmPixels dyCanvas = dyFrame - yTop - dyHRuler - dyHScroll;
    lmPixels dxCanvas = dxFrame - xLeft - dxVRuler - dxVScroll;
    if (m_fRulers) {
        lmPixels dxHRuler = m_xPageSizeD;
        m_pHRuler->NewSize(xLeft + dxVRuler + m_xBorder, yTop, dxHRuler);
        m_pVRuler->NewSize(xLeft, yTop + dyHRuler + m_yBorder, dyCanvas - m_yBorder);
    }
    m_pCanvas->SetSize(xLeft + dxVRuler, yTop + dyHRuler, dxCanvas, dyCanvas);
    m_pHScroll->SetSize(xLeft, yTop + dyHRuler + dyCanvas, dxFrame - xLeft - dxVScroll, dyHScroll);
    m_pVScroll->SetSize(xLeft + dxVRuler + dxCanvas, yTop, dxVScroll, dyCanvas + dyHRuler);




    //// Compute available space for the canvas
    //lmPixels cvMaxDx = Min(m_xPageSizeD+m_xBorder+m_xBorder, dxFree),
    //         cvMaxDy = Min(m_yPageSizeD, dyFree);

    //// Compute view origin, to center everything
    //dxFree -= cvMaxDx;
    //lmPixels left = (dxFree > 0 ? dxFree/2 : 0);

    //// Move controls to the computed positions
    //if (m_fRulers) {
    //    lmPixels dxHR = (cvMaxDx >= m_xPageSizeD+m_xBorder ? m_xPageSizeD : cvMaxDx-m_xBorder);
    //    m_pHRuler->NewSize(left+dxVRuler+m_xBorder, 0, dxHR);
    //    m_pVRuler->NewSize(left, dyHRuler, cvMaxDy);
    //}
    //m_pCanvas->SetSize(left+dxVRuler, dyHRuler, cvMaxDx, dyFrame - dyHScroll); 	//cvMaxDy);
    //m_pHScroll->SetSize(0, dyFrame - dyHScroll, dxFrame - dxVScroll, dyHScroll);
    //m_pVScroll->SetSize(dxFrame - dxVScroll, 0, dxVScroll, dyFrame - dyHScroll);

    //wxLogStatus(_T("Frame size(%d,%d) dxVScroll=%d, dyHScroll=%d"),
    //    dxFrame, dyFrame,
    //    dxVScroll, dyHScroll);

    // compute new scrollbars
    AdjustScrollbars();
    m_pCanvas->Refresh();

}

void lmScoreView::AdjustScrollbars()
{
    if (!m_pCanvas) return;

    // Lets verify if scrollbars are needed
    int dxCanvas, dyCanvas;
    m_pCanvas->GetSize(&dxCanvas, &dyCanvas);
    //bool fHScroll = (dxCanvas < m_xPageSizeD),
    //     fVScroll = (dyCanvas < m_yPageSizeD);

    // scroll step size will be 5 mm . transform into device units (pixels)
    m_pixelsPerStepX = (int)wxMax(lmToLogicalUnits(5, lmMILLIMETERS) * m_xDisplayPixelsPerLU, 1);
    m_pixelsPerStepY = (int)wxMax(lmToLogicalUnits(5, lmMILLIMETERS) * m_yDisplayPixelsPerLU, 1);

    // compute height and width of the whole view (all pages in the view plus margins)
    lmPixels xViewD = m_xPageSizeD + 2 * m_xBorder,
            yViewD = m_numPages * (m_yPageSizeD + m_yInterpageGap)- m_yInterpageGap + 2 * m_yBorder;

    // The range is the total number of units associated with the view represented by the scrollbar.
    // the scrollable space is the whole view minus the visible area (the canvas window)
    // Let's compute how many steps are required
    m_xMaxScrollSteps = 1 + xViewD / m_pixelsPerStepX;        // add 1 to take into account rounding errors
    m_yMaxScrollSteps = 1 + yViewD / m_pixelsPerStepY;

    // compute how many steps are required to scroll one page and the thumb size.
    // The thumb size is the number of units that are currently visible
    if (dxCanvas >= xViewD) {
        // full view visible on current canvas size. No X scrolling needed
        m_xScrollStepsPerPage = m_xMaxScrollSteps;
        m_thumbX = m_xScrollStepsPerPage;
    } else {
        m_thumbX = dxCanvas / m_pixelsPerStepX;
        m_xScrollStepsPerPage = wxMin( m_xPageSizeD / m_pixelsPerStepX , m_xMaxScrollSteps);
    }
    if (dyCanvas >= yViewD) {
        // full view visible on current canvas size. No X scrolling needed
        m_yScrollStepsPerPage = m_yMaxScrollSteps;
        m_thumbY = m_yScrollStepsPerPage;
    } else {
        // full view visible on current canvas size. No X scrolling needed
        m_yScrollStepsPerPage = wxMin((m_yPageSizeD + m_yInterpageGap) / m_pixelsPerStepY , m_yMaxScrollSteps);
        m_thumbY = dyCanvas / m_pixelsPerStepY;
    }

    // Finally, lets set up the scroll bars according to previous calculations
    m_pHScroll->SetScrollbar(m_xScrollPosition, m_thumbX, m_xMaxScrollSteps, m_pixelsPerStepX);
    m_pVScroll->SetScrollbar(m_yScrollPosition, m_thumbY, m_yMaxScrollSteps, m_pixelsPerStepY);

    ////DEBUG
    //wxLogStatus(_T("nPag=%d,lmPage(%d,%d),Px/Step(%d,%d),View(%d,%d),MaxSteps(%d,%d),StepsPage(%d,%d),Canvas(%d,%d)"),
    //    m_numPages, m_xPageSizeD, m_yPageSizeD, m_pixelsPerStepX, m_pixelsPerStepY,
    //    xViewD, yViewD,
    //    m_xMaxScrollSteps, m_yMaxScrollSteps,
    //    m_xScrollStepsPerPage, m_yScrollStepsPerPage,
    //    dxCanvas, dyCanvas);


}

void lmScoreView::SetRulersVisible(bool fVisible)
{
    m_fRulers = fVisible;
    if (m_pFrame) {
        wxASSERT(m_pVRuler && m_pHRuler);
        m_pHRuler->Show(m_fRulers);
        m_pVRuler->Show(m_fRulers);
        ResizeControls();
    }

}

void lmScoreView::OnDraw(wxDC* pDC)
{
    // OnDraw is a mandatory override of wxView. So we must define an OnDraw method. But the
    // repaint behaviour is controled by the OnPaint event on lmScoreCanvas and is redirected
    // to lmScoreView.RepaintScoreRectangle().
    // So OnDraw is empty. It is only invoked by the print/preview architecture, for print/preview
    // the document.

    wxLogMessage(wxT("Error: llega a lmScoreView.OnDraw()"));
}

void lmScoreView::GetPageInfo(int* pMinPage, int* pMaxPage, int* pSelPageFrom, int* pSelPageTo)
{
    // This method is only invoked for print and print-preview. It is invoked from
    // lmPrintout to get the number of pages needed to print the score

    wxMemoryDC mDC;
    //m_Paper.SetDC(&mDC);           //the layout phase requires a DC
    m_Paper.SetDrawer(new lmDirectDrawer(&mDC));
    lmScore* pScore = ((lmScoreDocument*)GetDocument())->GetScore();
    m_graphMngr.Prepare(pScore, m_xPageSizeD, m_yPageSizeD, m_rScale, &m_Paper);
    int nTotalPages = m_graphMngr.GetNumPages();

    *pMinPage = 1;
    *pMaxPage = nTotalPages;
    *pSelPageFrom = 1;
    *pSelPageTo = nTotalPages;
}

void lmScoreView::DrawPage(wxDC* pDC, int nPage, lmPrintout* pPrintout)
{
    // This method is only invoked for print and print-preview. It is invoked from
    // lmPrintout. It is responsible of drawing the requested page.
    // In some displays, the preview at 100% scale does not have the real size.
    // After several attempts and research I discovered that this is due to the
    // fact that changing display resolution doesn't change reported ppi
    // resolution. The effect also affects to other programs (i.e. Adobe Acrobat
    // Reader) that presents the same behaviour that LenMus.


    // Get paper size and real usable size of printer paper (in world units)
    wxSize uPaperSize = m_Paper.GetPaperSize();     // in lmLUnits
    int printerWidthMM, printerHeightMM;            // in millimeters
    pPrintout->GetPageSizeMM(&printerWidthMM, &printerHeightMM);
    lmLUnits uPrinterSizeX = lmToLogicalUnits(printerWidthMM, lmMILLIMETERS);
    lmLUnits uPrinterSizeY = lmToLogicalUnits(printerHeightMM, lmMILLIMETERS);

    // Get DC size in pixels
    int nDCPixelsW, nDCPixelsH;
    pDC->GetSize(&nDCPixelsW, &nDCPixelsH);

    // Calculate the scaling factor to fit score page in printer paper
    // It should be 1.00 if printer paper is the same size than the
    // intended score paper, but in practise it will be a litle less
    // than 1.00 as wxPrintout reports paper size discounting printer
    // margings
    double marginScaleX = (double)uPrinterSizeX / (double)uPaperSize.GetWidth();
    double marginScaleY = (double)uPrinterSizeY / (double)uPaperSize.GetHeight();


    bool fPreview = pPrintout->IsPreview();
    double overallScale;

    if (fPreview) {
        // We have to compute the scaling factor between the preview DC size and
        // the current view size
        double xScale = (double)(nDCPixelsW/(double)m_xPageSizeD) * m_rScale / marginScaleX;
        double yScale = (double)(nDCPixelsH/(double)m_yPageSizeD) * m_rScale / marginScaleY;
        overallScale = wxMin(xScale, yScale);
    }
    else {
        // For printing, as the DC is set in LOMETRIC mode, all scaling
        // is autoatically handled by the DC
        overallScale = lmSCALE;
    }

    pDC->SetUserScale(overallScale, overallScale);
    pDC->SetMapMode(lmDC_MODE);

    lmScore* pScore = ((lmScoreDocument*)GetDocument())->GetScore();
    if (fPreview) {
        // use anti-aliasing
        wxMemoryDC memoryDC;
        m_Paper.SetDrawer(new lmDirectDrawer(&memoryDC));
        m_graphMngr.Prepare(pScore, nDCPixelsW, nDCPixelsH, (double)overallScale, &m_Paper);
        wxBitmap* pPageBitmap = m_graphMngr.Render(lmUSE_BITMAPS, nPage);
        wxASSERT(pPageBitmap && pPageBitmap->Ok());
        memoryDC.SelectObject(*pPageBitmap);
        pDC->SetUserScale(1.0, 1.0);
        pDC->SetMapMode(wxMM_TEXT);
        pDC->Blit(0, 0, nDCPixelsW, nDCPixelsH, &memoryDC, 0, 0);
        memoryDC.SelectObject(wxNullBitmap);
    }
    else {
        //Direct renderization on printer DC
        m_Paper.SetDrawer(new lmDirectDrawer(pDC));
        m_graphMngr.Prepare(pScore, nDCPixelsW, nDCPixelsH, (double)overallScale, &m_Paper);
        m_graphMngr.Render(lmNO_BITMAPS, nPage);
    }

}

void lmScoreView::OnUpdate(wxView* sender, wxObject* hint)
{
    // Called from the document when an update is needed. i.e. when UpdateAllViews()
    // has been invoked

	WXUNUSED(sender)
	WXUNUSED(hint)

    if (m_pFrame) {
        m_pCanvas->Refresh();
        ResizeControls();
        lmScore* pScore = ((lmScoreDocument*)GetDocument())->GetScore();
        m_pFrame->SetTitle( pScore->GetScoreName() );
    }

}

bool lmScoreView::OnClose(bool deleteWindow)
{
    // Clean up all windows used for displaying this view.

    if (!GetDocument()->Close()) return false;

    Activate(false);

    if (deleteWindow) {
        if (m_pFrame) {
            delete m_pFrame;
            m_pFrame = (lmEditFrame*) NULL;
        }
        return true;
    }
    return true;
}

void lmScoreView::SetScale(double rScale)
{
    wxASSERT(rScale > 0);
    m_rScale = rScale * lmSCALE;

    if (m_pCanvas) {
        // compute new paper size in pixels
        wxClientDC dc(m_pCanvas);
        dc.SetMapMode(lmDC_MODE);
        dc.SetUserScale( m_rScale, m_rScale );
        wxSize uPageSize = m_Paper.GetPaperSize();
        m_xPageSizeD = dc.LogicalToDeviceXRel(uPageSize.GetWidth());
        m_yPageSizeD = dc.LogicalToDeviceYRel(uPageSize.GetHeight());

        // ----------------------------------------------------------------------------
        // This commented out code produces the same results than the
        // following code. The problem for not geting real size (1 : 1) on
        // screen is due to ppi resolution doesn't change when pixels
        // resolution is changed.

        //// Get the logical pixels per inch of screen
        //wxSize ppiScreen = dc.GetPPI();
        //wxLogMessage(_T("[lmScoreView::SetScale] ppiScreenX=%f, ppiScreenY=%f"),
        //    ppiScreen.GetWidth(), ppiScreen.GetHeight() );

        //// There are approx. 25.4 mm to the inch. There are ppi
        //// device units to the inch. Therefore 1 mm corresponds to
        //// ppi/25.4 device units.
        //lmLUnits oneMM = lmToLogicalUnits(1, lmMILLIMETERS);
        //m_xDisplayPixelsPerLU = (double)ppiScreen.x / (25.4 * (double)oneMM);
        //m_yDisplayPixelsPerLU = (double)ppiScreen.y / (25.4 * (double)oneMM);

        //-----------------------------------------------------------------------------

        // store new conversion factors
        m_xDisplayPixelsPerLU = (double)dc.LogicalToDeviceXRel(100000) / 100000.0;
        m_yDisplayPixelsPerLU = (double)dc.LogicalToDeviceYRel(100000) / 100000.0;

        //reposition controls
        ResizeControls();

        //wxLogMessage(_T("[lmScoreView::SetScale] scale=%f, m_rScale=%f, DisplayPixelsPerLU=(%f, %f), pageSize LU(%d, %d), pageSize pixels(%d, %d)"),
        //    rScale, m_rScale, m_xDisplayPixelsPerLU, m_yDisplayPixelsPerLU,
        //    uPageSize.GetWidth(), uPageSize.GetHeight(),
        //    m_xPageSizeD, m_yPageSizeD);
    }

    m_pCanvas->Refresh(true);    //erase background

}

void lmScoreView::SetScaleFitWidth()
{
    if (!m_pCanvas) return;

    int xScreen, yScreen;
    m_pFrame->GetClientSize(&xScreen, &yScreen);

    double xScale = m_rScale * (double)(xScreen-50) / (double)m_xPageSizeD;

    wxLogMessage(_T("[] xScreen=%d, xPageSizeD=%d, rScale=%f, scale=%f"),
            xScreen, m_xPageSizeD, m_rScale, xScale );

    SetScale(xScale / lmSCALE);

}

void lmScoreView::SetScaleFitFull()
{
    if (!m_pCanvas) return;

    int xScreen, yScreen;
    m_pFrame->GetClientSize(&xScreen, &yScreen);

    double xScale = m_rScale * (double)(xScreen-50) / (double)m_xPageSizeD;
    double yScale = m_rScale * (double)(yScreen-20) / (double)m_yPageSizeD;
    double rScale = wxMin(xScale, yScale) / lmSCALE;
    SetScale(rScale);

}

void lmScoreView::OnVisualHighlight(lmScoreHighlightEvent& event)
{
	lmScore* pScore = m_pDoc->GetScore();
    EHighlightType nHighlightType = event.GetHighlightType();
    switch (nHighlightType) {
        case ePrepareForHighlight:
        {
            m_graphMngr.PrepareForHighlight();
            return;
        }
        break;

        case eRemoveAllHighlight:
        {
			pScore->RemoveAllHighlight((wxWindow*)m_pCanvas);
			return;
        }
        break;

        case eVisualOff:
        case eVisualOn:
        {
        }
        break;

        default:
            wxASSERT(false);
    }

    //AWARE: Only eVisualOff and eVisualOn events reach this point

    //prepare paper DC
    wxClientDC dc(m_pCanvas);
    dc.SetMapMode(lmDC_MODE);
    dc.SetUserScale( m_rScale, m_rScale );
    //m_Paper.SetDC(&dc);
    m_Paper.SetDrawer(new lmDirectDrawer(&dc));

	//Obtain the StaffObject
	//For events of type eRemoveAllHighlight the pSO is NULL
	lmStaffObj* pSO = event.GetStaffObj();
	int nNumPage = pSO->GetPageNumber();        // nNumPage = 1..n
	//position DC origing according to current scrolling and page position
	wxPoint org = GetDCOriginForPage(nNumPage);
	dc.SetDeviceOrigin(org.x, org.y);

	//do the requested action:  highlight (eVisualOn) / unhighlight (eVisualOff)
	pScore->ScoreHighlight(pSO, &m_Paper, nHighlightType);

}

wxPoint lmScoreView::GetDCOriginForPage(int nNumPage)
{
    // Pages measure (m_xPageSizeD, m_yPageSizeD) pixels.
    // There is a gap between pages of  m_yInterpageGap  pixels.
    // There is a left margin:  m_xBorder  pixels
    // And there is a top margin before the first page:  m_yBorder  pixels

    //First page at (m_xBorder, m_yBorder), size (m_xPageSizeD, m_yPageSizeD)
    //Second page at (m_xBorder, m_yBorder+m_yPageSizeD+m_yInterpageGap)
    //...
    //Page n (1..n) at (m_xBorder, m_yBorder + (n-1) * (m_yPageSizeD+m_yInterpageGap))
    // all this coordinates are referred to view origin (0,0), a virtual infinite
    // paper on which all pages are rendered one after the other.

    lmPixels xPage = m_xBorder;
    lmPixels yPage = m_yBorder + (nNumPage-1) * (m_yPageSizeD + m_yInterpageGap);

    //position DC origing according to current scrolling and page position
    int dx = xPage - m_xScrollPosition* m_pixelsPerStepX;
    int dy = yPage - m_yScrollPosition* m_pixelsPerStepY;
	return wxPoint(dx, dy);
}

void lmScoreView::DeviceToLogical(lmDPoint& posDevice, lmUPoint& posLogical,
                            lmDPoint* pPageNPosD, lmDPoint* pPageNOrgD,
                            lmDPoint* pOffsetD, int* pNumPage, bool* pfInInterpageGap)
{
	//converts a device position (pixels), referred to the lmScoreCanvas window,
	//to logical position (lmLUnits) referred to current page origin.
    //Optionally (if not null pointers) returns:
    // pPageNPosD - device position (lmPixels) referred to current page origin.
    // pPageNOrgD - origing of current page (lmPixels) referred to view origin.
    // pNumPage - number of current page (1..n)
    // pfInInterpageGap - flag informing if position is out of page

    // Set DC in logical units and scaled, so that
    // transformations logical/device and viceversa can be computed
    wxClientDC dc(m_pCanvas);
    dc.SetMapMode(lmDC_MODE);
    dc.SetUserScale( m_rScale, m_rScale );

    // We need to know how much the window has been scrolled (in pixels)
    int xScrollUnits, yScrollUnits, xOrg, yOrg;
    GetViewStart(&xOrg, &yOrg);
    GetScrollPixelsPerUnit(&xScrollUnits, &yScrollUnits);
    xOrg *= xScrollUnits;
    yOrg *= yScrollUnits;
    lmDPoint canvasOrgD(xOrg, yOrg);

    //Pages measure (m_xPageSizeD, m_yPageSizeD) pixels.
    //There is a gap between pages of  m_yInterpageGap  pixels.
    //There is a left margin:  m_xBorder  pixels
    //And there is a top margin before the first page:  m_yBorder  pixels
    //Therefore, first page is at (m_xBorder, m_yBorder), size (m_xPageSizeD, m_yPageSizeD)
    //Second page at (m_xBorder, m_yBorder+m_yPageSizeD+m_yInterpageGap)
    //...
    //Page n (1..n) at (m_xBorder, m_yBorder + (n-1) * (m_yPageSizeD+m_yInterpageGap))
    //All this coordinates are referred to view origin (0,0), a virtual infinite
    //paper on which all pages are rendered one after the other.

	lmPixels yPage = posDevice.y + canvasOrgD.y;
	int nNumPage = ((yPage - m_yBorder) / (m_yPageSizeD + m_yInterpageGap)) + 1;
    lmPixels yStartPage = m_yBorder + (nNumPage-1) * (m_yPageSizeD + m_yInterpageGap);
    lmPixels yEndPage = yStartPage + m_yPageSizeD;
	bool fInInterpageGap = (yPage < m_yBorder || yPage > yEndPage);

	// the origin of current page is at (pixels)
    lmDPoint pageNOrgD(m_xBorder, yStartPage);

    // let's compute the position (pixels and logical) referred to current page origin
    lmDPoint pageNPosD(posDevice.x + canvasOrgD.x - pageNOrgD.x,
                     posDevice.y + canvasOrgD.y - pageNOrgD.y);
    lmUPoint pageNPosL(dc.DeviceToLogicalXRel(pageNPosD.x),
                     dc.DeviceToLogicalYRel(pageNPosD.y));


	//move requested answers
    posLogical.x = pageNPosL.x;
    posLogical.y = pageNPosL.y;

    if (pPageNPosD)
        *pPageNPosD = pageNPosD;

    if (pNumPage)
        *pNumPage = nNumPage;

    if (pfInInterpageGap)
        *pfInInterpageGap = fInInterpageGap;

    if (pPageNOrgD) {
        (*pPageNOrgD).x = pageNOrgD.x;
        (*pPageNOrgD).y = pageNOrgD.y;
    }

    if (pOffsetD) {
        (*pOffsetD).x = pageNOrgD.x - canvasOrgD.x;
        (*pOffsetD).y = pageNOrgD.y - canvasOrgD.y;
    }

  //  wxLogMessage(_T("[lmScoreView::DeviceToLogical] coverting canvas point (%d, %d) pixels\n")
		//		 _T("     Point referred to first paper page origin (%d, %d) pixels\n")
		//		 _T("     Point referred to first paper page origin (%.2f, %.2f) lmLUnits\n")
		//		 _T("     Point referred to this paper page origin (%.2f, %.2f) lmLUnits\n")
		//		 _T("     Point is at page %d (yStartPage=%d, yPage=%d, yEndPage=%d) %s"),
  //      posDevice.x, posDevice.y, pageNPosD.x, pageNPosD.y, posLogical.x, pagePosL.y,
		//pageNPosL.x, pageNPosL.y,
		//nNumPage, yStartPage, yPage, yEndPage, (fInInterpageGap ? _T("in gap between pages") : _T("")) );



}

lmDPoint lmScoreView::GetPageOffset(int nNumPage)
{
	// Returns the offset to add to a display point (that is, a point in pixels referred to
	// CanvasOrg) to convert it, so that it become referred to current PageOrg

    // Set DC in logical units and scaled, so that
    // transformations logical/device and viceversa can be computed
    wxClientDC dc(m_pCanvas);
    dc.SetMapMode(lmDC_MODE);
    dc.SetUserScale( m_rScale, m_rScale );

    // We need to know how much the window has been scrolled (in pixels)
    lmDPoint canvasOrgD = GetScrollOffset();

    //Pages measure (m_xPageSizeD, m_yPageSizeD) pixels.
    //There is a gap between pages of  m_yInterpageGap  pixels.
    //There is a left margin:  m_xBorder  pixels
    //And there is a top margin before the first page:  m_yBorder  pixels
    //Therefore, first page is at (m_xBorder, m_yBorder), size (m_xPageSizeD, m_yPageSizeD)
    //Second page at (m_xBorder, m_yBorder+m_yPageSizeD+m_yInterpageGap)
    //...
    //Page n (1..n) at (m_xBorder, m_yBorder + (n-1) * (m_yPageSizeD+m_yInterpageGap))
    //All this coordinates are referred to view origin (0,0), a virtual infinite
    //paper on which all pages are rendered one after the other.

    lmPixels xStartPage = m_xBorder;
    lmPixels yStartPage = m_yBorder + (nNumPage-1) * (m_yPageSizeD + m_yInterpageGap);

	// the origin of current page is at (pixels)
    lmDPoint pageNOrgD(xStartPage, yStartPage);

    //terefore the offset is
    return lmDPoint(pageNOrgD.x - canvasOrgD.x, pageNOrgD.y - canvasOrgD.y);

}

lmDPoint lmScoreView::GetScrollOffset()
{
	// Returns the offset to add to a display point (that is, a point in pixels referred to
	// CanvasOrg) to convert it, so that it become referred to ViewOrg

    // We need to know how much the window has been scrolled (in pixels)
    int xScrollUnits, yScrollUnits, xOrg, yOrg;
    GetViewStart(&xOrg, &yOrg);
    GetScrollPixelsPerUnit(&xScrollUnits, &yScrollUnits);
    xOrg *= xScrollUnits;
    yOrg *= yScrollUnits;
    return lmDPoint(xOrg, yOrg);
}

void lmScoreView::LogicalToDevice(lmUPoint& posLogical, lmDPoint& posDevice)
{
	//converts a logical position (lmLUnits), referred to current page origin to
	//a device position (pixels), referred to the lmScoreCanvas window,

    // Set DC in logical units and scaled, so that
    // transformations logical/device and viceversa can be computed
    wxClientDC dc(m_pCanvas);
    dc.SetMapMode(lmDC_MODE);
    dc.SetUserScale( m_rScale, m_rScale );

    // We need to know how much the window has been scrolled (in pixels)
    lmDPoint canvasOrgD = GetScrollOffset();

	//convert logical point to pixels, referred to start of first page origin
	lmDPoint pointRelD(dc.LogicalToDeviceXRel((int)posLogical.x),
                     dc.LogicalToDeviceYRel((int)posLogical.y));

	// the origin of first page is at (pixels)
    lmDPoint pageOrgD(m_xBorder, m_yBorder);

	//therefore the point, referred to canvas origin is at
	lmDPoint pointAbsD(pointRelD.x + pageOrgD.x, pointRelD.y + pageOrgD.y);

	//now lets discount canvas scrolling
	posDevice.x = pointAbsD.x - canvasOrgD.x;
	posDevice.y = pointAbsD.y - canvasOrgD.y;


    //// For transformations from page to canvas and viceversa we need to combine both origins
    //lmDPoint offsetD(pageOrgD.x - canvasOrgD.x, pageOrgD.y - canvasOrgD.y);

  //  wxLogMessage(_T("[lmScoreView::LogicalToDevice] coverting logical point (%.2f, %.2f) lmLUnits\n")
		//		 _T("     Point referred to first paper page origin (%d, %d) pixels\n")
		//		 _T("     Point referred to view origin (%d, %d) pixels\n")
		//		 _T("     Point referred to canvas origin (%d, %d) pixels\n"),
  //      posLogical.x, posLogical.y, pointRelD.x, pointRelD.y, pointAbsD.x, pointAbsD.y,
		//posDevice.x, posDevice.y );



}

void lmScoreView::OnMouseEvent(wxMouseEvent& event, wxDC* pDC)
{

    // get logical coordinates of point pointed by mouse

    // as wxDragImage works with unscrolled device coordinates, we need current position
    // in device units. All device coordinates are referred to the lmScoreCanvas window
    lmDPoint canvasPosD(event.GetPosition());

    // Leave DC in logical units and scaled, so that
    // transformations logical/device and viceversa can be computed
    pDC->SetMapMode(lmDC_MODE);
    pDC->SetUserScale( m_rScale, m_rScale );

    //compute click point in logical units. Get also different origins and values
    lmDPoint pageNOrgD;     //the origin (pixels) of this page
    lmDPoint pageNPosD;      //the position (pixels) referred to current page origin
    lmDPoint offsetD;       //offset from canvas origin
	lmUPoint pageNPosL;     //the position (logical) referred to current page origin
    int nNumPage;           //the score num page
    bool fInInterpageGap;   //mouse click out of page
	DeviceToLogical(canvasPosD, pageNPosL, &pageNPosD, &pageNOrgD, &offsetD,
                    &nNumPage, &fInInterpageGap);

	////for testing and debugging methods DeviceToLogical [ok] and LogicalToDevice [ok]
	//lmDPoint tempPagePosD;
	//LogicalToDevice(tempPagePosL, tempPagePosD);

    // draw markers on the rulers
    if (m_fRulers) {
        //lmDPoint ptR(pDC->LogicalToDeviceX(pt.x), pDC->LogicalToDeviceY(pt.y));
        if (m_pHRuler) m_pHRuler->ShowPosition(pageNPosD);
        if (m_pVRuler) m_pVRuler->ShowPosition(pageNPosD);
    }

    // check if a key is pressed
    enum {
        lmKEY_ALT = 0x0001,
        lmKEY_CTRL = 0x0002,
        lmKEY_SHIFT = 0x0004,
    };

    int nKeysPressed = 0;
    if (event.ShiftDown())
        nKeysPressed = nKeysPressed | lmKEY_SHIFT;
    if (event.ControlDown())
        nKeysPressed = nKeysPressed | lmKEY_CTRL;
    if (event.AltDown())
        nKeysPressed = nKeysPressed | lmKEY_ALT;


    if (event.LeftDClick() ) {
        // mouse left double click: Select/deselect the object pointed by mouse
        //--------------------------------------------------------------------------

		//ScoreObjs and other score objects (lmBoxXXXX) has all its measurements
		//relative to each page start position

        // locate the object
        lmScoreObj* pScO = m_pDoc->FindSelectableObject(pageNPosL);

        if (pScO)
        {
            // we've got a valid object: select/deselect it.
			m_pCanvas->SelectObject(pScO);
        }
        else
        {
            // no object. Perhaps it is a doble click on the staff.
            lmBoxScore* pBScore = m_graphMngr.GetBoxScore();
            lmBoxPage* pBPage = pBScore->GetPage(nNumPage);
            lmBoxSlice* pBSlice = pBPage->FindStaffAtPosition(pageNPosL);
			if (pBSlice)
			{
				//prepare paper DC
				wxClientDC dc(m_pCanvas);
				dc.SetMapMode(lmDC_MODE);
				dc.SetUserScale( m_rScale, m_rScale );
				//position DC origing at current page origin
				wxPoint org = GetDCOriginForPage(nNumPage);
				dc.SetDeviceOrigin(org.x, org.y);
				//set paper and draw selection rectangle
				m_Paper.SetDrawer(new lmDirectDrawer(&dc));
				pBSlice->DrawSelRectangle(&m_Paper);
			}
            else
                wxMessageBox( wxString::Format( _T("Page %d"), nNumPage ));
        }


    } else if (event.LeftDown() ) {
        // mouse left button down: if pointing an object posible start of dragging.
        // ---------------------------------------------------------------------------

        // locate the object
        lmScoreObj* pScO = m_pDoc->FindSelectableObject(pageNPosL);
        if (pScO)
        {
            //valid object pointed.
            if (pScO->IsDraggable())
            {
                //Is a draggable object. Tentatively start dragging
                m_pSoDrag = pScO;
                m_dragState = lmDRAG_START;
                m_dragStartPosL = pageNPosL;        // save mouse position (page logical coordinates)
                // compute the location of the drag position relative to the upper-left
                // corner of the image (pixels)
                lmUPoint hotSpot = pageNPosL - pScO->GetGlyphPosition();
                m_dragHotSpot.x = pDC->LogicalToDeviceXRel((int)hotSpot.x);
                m_dragHotSpot.y = pDC->LogicalToDeviceYRel((int)hotSpot.y);
            }
            else
            {
                //Non-draggable object. Possible selection?
            }
       }
       else
       {
           //no object pointed. Possible tool box insert command
       }

    }
    else if ((event.LeftUp() && m_dragState != lmDRAG_NONE )) {
        // Left up & dragging: Finish dragging
        //---------------------------------------------------

        m_dragState = lmDRAG_NONE;

        if (!m_pSoDrag || !m_pDragImage) return;

        // delete the image used for dragging
        m_pDragImage->Hide();
        m_pDragImage->EndDrag();
        delete m_pDragImage;
        m_pDragImage = (wxDragImage*) NULL;

        // Generate move command to move lmStaffObj and update the document
        //lmScoreDocument* doc = (lmScoreDocument*)GetDocument();
        //wxCommandProcessor* pCP = doc->GetCommandProcessor();
        //pCP->Submit(new lmScoreCommandMove(_T("Move object"), doc, m_pSoDrag, finalPos));
        lmUPoint finalPos = m_pSoDrag->GetGlyphPosition() + pageNPosL - m_dragStartPosL;
		m_pCanvas->MoveObject(m_pSoDrag, finalPos);

        m_pSoDrag = (lmScoreObj*) NULL;

    }

    else if (event.Dragging() && (m_dragState == lmDRAG_START)) {
        // The mouse was clicked and now has started to drag
        //-----------------------------------------------------------

        m_dragState = lmDRAG_DRAGGING;

        // prepare the image to drag
        if (m_pDragImage) delete m_pDragImage;
        wxBitmap* pBitmap = m_pSoDrag->GetBitmap(m_rScale);
        if (!pBitmap) {
            wxLogMessage(wxString::Format(_T("No drag image for object type %d"),
                m_pSoDrag->GetType() ));
            m_dragState = lmDRAG_NONE;
            return;
        }
        m_pDragImage = new wxDragImage(*pBitmap);    //, wxCursor(wxCURSOR_HAND));
        delete pBitmap;

        // show drag image
        bool fOK = m_pDragImage->BeginDrag(m_dragHotSpot, m_pCanvas);
        if (!fOK) {
            delete m_pDragImage;
            m_pDragImage = (wxDragImage*) NULL;
            m_dragState = lmDRAG_NONE;

        } else {
            //drag image started OK. Move image to current cursor position
            //and show it (was hidden until now)
            lmDPoint offset(offsetD.x + m_dragHotSpot.x, offsetD.y + m_dragHotSpot.y);
            //m_Paper.SetDC(pDC);
            m_Paper.SetDrawer(new lmDirectDrawer(pDC));
            m_pSoDrag->MoveDragImage(&m_Paper, m_pDragImage, offset, pageNPosL, m_dragStartPosL, canvasPosD);
            m_pDragImage->Show();
        }

    }
    else if (event.Dragging() && (m_dragState == lmDRAG_DRAGGING)) {
        // We're currently dragging. Move the image
        //------------------------------------------------------
        if (!m_pDragImage) return;

        // If mouse outside of canvas window let's force autoscrolling.
        bool fDoScroll = false;
        wxSize canvasSize = m_pCanvas->GetSize();
        int nUnits=0, orientation=0;

        if (canvasPosD.x < 0) {
            fDoScroll = true;
            nUnits = -1;
            orientation = wxHORIZONTAL;
        } else if (canvasPosD.y < 0) {
            fDoScroll = true;
            nUnits = -1;
            orientation = wxVERTICAL;
        } else if (canvasPosD.x > canvasSize.GetWidth()) {
            fDoScroll = true;
            nUnits = 1;
            orientation = wxHORIZONTAL;
        } else if (canvasPosD.y > canvasSize.GetHeight()) {
            fDoScroll = true;
            nUnits = 1;
            orientation = wxVERTICAL;
        }

        if (fDoScroll) {
            m_pDragImage->Hide();
            DoScroll(orientation, nUnits);
            m_pDragImage->Show();
            //wxLogStatus(_T("Scrolling(%d), canvasPosD=(%d, %d), canvasSize=(%d, %d)"),
            //    nUnits, canvasPosD.x, canvasPosD.y,
            //    canvasSize.GetWidth(), canvasSize.GetHeight());

            // The user could held the mouse outside the window for a few seconds to force
            // to scroll several pages. As the mouse is not getting moved no new events
            // would be generated. Therefore we must generate them.
            wxPoint oldPos = ::wxGetMousePosition();    // get mouse position (screen origin)
            ::wxMilliSleep(100);                        // wait 100ms to slow autoscrolling
            wxPoint newPos = ::wxGetMousePosition();    // get new position
            if (oldPos.x == newPos.x && oldPos.y==newPos.y) {
                // mouse is held outside the window. Generate a new mouse drag event
                wxEvtHandler* pEvtH = m_pCanvas->GetEventHandler();
                pEvtH->AddPendingEvent(event);
            }

        } else {
            // just move the image
            lmDPoint offset(offsetD.x + m_dragHotSpot.x, offsetD.y + m_dragHotSpot.y);
            //m_Paper.SetDC(pDC);
            m_Paper.SetDrawer(new lmDirectDrawer(pDC));
            m_pSoDrag->MoveDragImage(&m_Paper, m_pDragImage, offset, pageNPosL, m_dragStartPosL, canvasPosD);
        }

    }

    else if (event.GetEventType() == wxEVT_MOUSEWHEEL ) {
        OnMouseWheel(event);
    }

}

void lmScoreView::OnMouseWheel(wxMouseEvent& event)
{

    int nWheelRotation = event.GetWheelRotation();
    int lines = nWheelRotation / event.GetWheelDelta();
    nWheelRotation -= lines * event.GetWheelDelta();

    if (lines != 0) {

        wxScrollEvent newEvent;

        newEvent.SetPosition(0);
        newEvent.SetOrientation(wxVERTICAL);
        newEvent.SetEventObject(m_pCanvas);

        if (event.IsPageScroll())
        {
            if (lines > 0)
                newEvent.SetEventType(wxEVT_SCROLL_PAGEUP);
            else
                newEvent.SetEventType(wxEVT_SCROLL_PAGEDOWN);

            OnScroll(newEvent);
        }
        else
        {
            lines *= event.GetLinesPerAction();
            if (lines > 0)
                newEvent.SetEventType(wxEVT_SCROLL_LINEUP);
            else
                newEvent.SetEventType(wxEVT_SCROLL_LINEDOWN);

            int times = abs(lines);
            for (; times > 0; times--)
                OnScroll(newEvent);
        }
    }

}

void lmScoreView::OnScroll(wxScrollEvent& event)
{
    int nScrollSteps = CalcScrollInc(event);
    if ( nScrollSteps == 0 ) return;        // can't scroll further
    DoScroll(event.GetOrientation(), nScrollSteps);
}

void lmScoreView::DoScroll(int orientation, int nScrollSteps)
{
    // verify limits
    int newPos;
    int xMaxSteps = m_xMaxScrollSteps - m_thumbX,
            yMaxSteps = m_yMaxScrollSteps - m_thumbY;

    if (orientation == wxHORIZONTAL) {
        newPos = nScrollSteps + m_xScrollPosition;
        nScrollSteps = (newPos < 0 ? -m_xScrollPosition :
            ((newPos > xMaxSteps) ? (xMaxSteps - m_xScrollPosition) : nScrollSteps) );
    } else {
        newPos = nScrollSteps + m_yScrollPosition;
        nScrollSteps = (newPos < 0 ? -m_yScrollPosition :
            ((newPos > yMaxSteps) ? (yMaxSteps - m_yScrollPosition) : nScrollSteps) );
    }

    if (nScrollSteps == 0) return;        // can't scroll further


    // save data and transform steps into pixels
    if (orientation == wxHORIZONTAL) {
        m_xScrollPosition += nScrollSteps;
    } else {
        m_yScrollPosition += nScrollSteps;
    }

    // compute scroll displacement in pixels
    int dx = 0, dy = 0;
    if (orientation == wxHORIZONTAL) {
        dx = -m_pixelsPerStepX * nScrollSteps;
    } else {
        dy = -m_pixelsPerStepY * nScrollSteps;
    }

    // reposition scrollbars
    m_pHScroll->SetThumbPosition(m_xScrollPosition);
    m_pVScroll->SetThumbPosition(m_yScrollPosition);

    // proceed to do the scrolling
    m_pCanvas->ScrollWindow(dx, dy, (wxRect*)NULL);    // we have to scroll all the window
    if (m_fRulers) {
        if (m_pHRuler) m_pHRuler->ScrollWindow(dx, 0, (wxRect*)NULL);    //rect );
        if (m_pVRuler) m_pVRuler->ScrollWindow(0, dy, (wxRect*)NULL);    //rect );
    }

#ifdef __WXMAC__
    m_pCanvas->MacUpdateImmediately() ;
#endif

}

//------------------------------------------------------------------------------------------
// Helper methods for scrolling
//------------------------------------------------------------------------------------------

// Where the current view starts from
void lmScoreView::GetViewStart (int* x, int* y) const
{
    if ( x )
        *x = m_xScrollPosition;
    if ( y )
        *y = m_yScrollPosition;
}

void lmScoreView::GetScrollPixelsPerUnit (int* x_unit, int* y_unit) const
{
    if ( x_unit )
        *x_unit = m_pixelsPerStepX;
    if ( y_unit )
        *y_unit = m_pixelsPerStepY;
}

int lmScoreView::CalcScrollInc(wxScrollEvent& event)
{
    int pos = event.GetPosition();
    int orient = event.GetOrientation();
    int xMaxSteps = m_xMaxScrollSteps - m_thumbX,
        yMaxSteps = m_yMaxScrollSteps - m_thumbY;

    int nScrollInc = 0;
    if (event.GetEventType() == wxEVT_SCROLL_TOP) {
        if (orient == wxHORIZONTAL)
            nScrollInc = - m_xScrollPosition;        // so that total scroll becomes zero
        else
            nScrollInc = - m_yScrollPosition;

    } else if (event.GetEventType() == wxEVT_SCROLL_BOTTOM) {
        if (orient == wxHORIZONTAL)
            nScrollInc = xMaxSteps - m_xScrollPosition;
        else
            nScrollInc = yMaxSteps - m_yScrollPosition;

    } else if (event.GetEventType() == wxEVT_SCROLL_LINEUP) {
        nScrollInc = -1;

    } else if (event.GetEventType() == wxEVT_SCROLL_LINEDOWN) {
        nScrollInc = 1;

    } else if (event.GetEventType() == wxEVT_SCROLL_PAGEUP) {
            if (orient == wxHORIZONTAL)
                nScrollInc = -m_xScrollStepsPerPage;
            else
                nScrollInc = -m_yScrollStepsPerPage;

    } else if (event.GetEventType() == wxEVT_SCROLL_PAGEDOWN) {
            if (orient == wxHORIZONTAL)
                nScrollInc = m_xScrollStepsPerPage;
            else
                nScrollInc = m_yScrollStepsPerPage;

    } else if ((event.GetEventType() == wxEVT_SCROLL_THUMBTRACK) ||
        (event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE))
    {
        if (orient == wxHORIZONTAL)
            nScrollInc = pos - m_xScrollPosition;
        else
            nScrollInc = pos - m_yScrollPosition;

    } else if (event.GetEventType() == wxEVT_SCROLL_ENDSCROLL) {
        // Move focus to canvas
        m_pCanvas->SetFocus();
    }

    return nScrollInc;
}

void lmScoreView::RepaintScoreRectangle(wxDC* pDC, wxRect& repaintRect)
{
    // This method is invoked by lmScoreCanvas::OnPaint to repaint a rectangle of the score
    // The DC is not scrolled.
    // The rectangle to redraw is in pixels and unscrolled

    // To draw a cast shadow for each page we need the shadow sizes
    lmPixels nRightShadowWidth = 3;      //pixels
    lmPixels nBottomShadowHeight = 3;      //pixels

    // Here in OnPaint we want to know which page
    // to redraw so that we prevent redrawing pages that don't
    // need to get redrawn.

    // Pages measure (m_xPageSizeD, m_yPageSizeD) pixels.
    // There is a gap between pages of  m_yInterpageGap  pixels.
    // There is a left margin:  m_xBorder  pixels
    // And there is a top margin before the first page:  m_yBorder pixels

    //First page at (m_xBorder, m_yBorder), size (m_xPageSizeD, m_yPageSizeD)
    //Second page at (m_xBorder, m_yBorder + m_yPageSizeD + m_yInterpageGap)
    //...
    //Page n+1 at (m_xBorder, m_yBorder + n * (m_yPageSizeD + m_yInterpageGap))
    // all this coordinates are referred to view origin (0,0), a virtual infinite
    // paper on which all pages are rendered one after the other.

    // We need to know how much the window has been scrolled (in pixels)
    wxPoint canvasOffset = GetScrollOffset();

    // allocate a DC in memory for using the offscreen bitmaps
    wxMemoryDC memoryDC;

    // Following code initializes cursor position if not yet initialized.
    // Next code has nothing to do with repainting but I didn't find a better place to
    // include it. And when repainting it is necessary to have cursor initialized.
    lmScore* pScore = m_pDoc->GetScore();
    if (!pScore) return;
	if (m_pCursorInstr == (lmInstrument*)NULL)
	{
		// Set initial cursor position
		m_pCursorInstr = pScore->GetFirstInstrument();	//first instrument
		m_nCursorStaff = 1;								//staff = 1
		m_nCursorMeasure = 1;							//measure = 1
		m_nCursorTime = 0;
	}


    // inform the paper that we are going to use it, and get the number of
    // pages needed to draw the score
    //m_Paper.SetDC(&memoryDC);           //the layout phase requires a DC
    m_Paper.SetDrawer(new lmDirectDrawer(&memoryDC));
    m_graphMngr.Prepare(pScore, m_xPageSizeD, m_yPageSizeD, m_rScale, &m_Paper);
    int nTotalPages = m_graphMngr.GetNumPages();

    if (nTotalPages != m_numPages) {
        // number of pages has changed. Adjust scrollbars
        m_numPages = nTotalPages;
        AdjustScrollbars();
    }

    // the repaintRect is referred to canvas window origin and is unscrolled.
    // To refer it to view origin it is necessary to add scrolling origin
    wxRect drawRect(repaintRect.x + canvasOffset.x, repaintRect.y + canvasOffset.y,
                    repaintRect.width, repaintRect.height );

    wxLogMessage(_T("Repainting rectangle (%d, %d, %d, %d), drawRect=(%d, %d, %d, %d)"),
        repaintRect.x, repaintRect.y, repaintRect.width, repaintRect.height,
        drawRect.x, drawRect.y, drawRect.width, drawRect.height );
    
    //verify if left background needs repaint
    if (drawRect.x < m_xBorder)
    {
        //compute left background rectangle
        wxRect bgRect(repaintRect.x, repaintRect.y,
                      m_xBorder - repaintRect.x - canvasOffset.x, repaintRect.height );
        wxBrush brush(m_colorBg, wxSOLID);
        wxPen pen(m_colorBg);
        pDC->SetBrush(brush);
        pDC->SetPen(pen);
        pDC->DrawRectangle(bgRect);
    }

    //verify if rigth background needs repaint
    lmPixels xRight = m_xBorder + m_xPageSizeD;
    if (drawRect.x + drawRect.width  > xRight)
    {
        //compute right background rectangle
        wxRect bgRect(xRight - canvasOffset.x, repaintRect.y,
                      repaintRect.width + repaintRect.x - xRight + canvasOffset.x,
					  repaintRect.height );
		bgRect.Intersect(drawRect);
        wxBrush brush(m_colorBg, wxSOLID);
        wxPen pen(m_colorBg);
        pDC->SetBrush(brush);
        pDC->SetPen(pen);
        pDC->DrawRectangle(bgRect);
    }

    //verify if top margin needs repaint
    //top margin rectangle, referred to ViewOrg, is at:
    wxRect bgTopMargin(m_xBorder, 0, m_xPageSizeD, m_yBorder);
	bgTopMargin.Intersect(drawRect);
    if (bgTopMargin.width > 0 && bgTopMargin.height > 0)
    {
        //rectangle, referred to CanvasOrg is at:
        wxRect bgRect(bgTopMargin.x - canvasOffset.x, bgTopMargin.y - canvasOffset.y, bgTopMargin.width, bgTopMargin.height);
        wxBrush brush(m_colorBg, wxSOLID);
        wxPen pen(m_colorBg);
        pDC->SetBrush(brush);
        pDC->SetPen(pen);
        pDC->DrawRectangle(bgRect);
    }

    // loop to verify if page nPag (0..n-1) is visible and needs redrawing.
    // To optimize, the loop is exited as soon as we find a non-visible page after
    // a visible one.
    bool fPreviousPageVisible = false;
    int nPag=0;
    for (nPag=0; nPag < m_numPages; nPag++) {
        // Let's compute this page rectangle, referred to view origin (0,0)
        wxRect pageRect(m_xBorder,
                        m_yBorder + nPag * (m_yPageSizeD+m_yInterpageGap),
                        m_xPageSizeD,
                        m_yPageSizeD);
        //wxLogStatus(wxT("pageRect(%d,%d, %d, %d)"),
        //    pageRect.x, pageRect.y, pageRect.width, pageRect.height);

        // Lets intersect pageRect with drawRect to verify if this page is affected
        wxRect interRect(pageRect.x,
                         pageRect.y,
                         pageRect.width + nRightShadowWidth,
                         pageRect.height + nBottomShadowHeight);
        interRect.Intersect(drawRect);

        // if intersection is not null this page needs repainting.
        if (interRect.width > 0 && interRect.height > 0) {

            // ask paper for the offscreen bitmap of this page
            wxBitmap* pPageBitmap = m_graphMngr.Render(lmUSE_BITMAPS, nPag+1);
            wxASSERT(pPageBitmap && pPageBitmap->Ok());
            memoryDC.SelectObject(*pPageBitmap);

            // Intersection rectangle is referred to view origin. To refer it
            // to bitmap coordinates we need to substract page origin
            int xBitmap = interRect.x - pageRect.x,
                yBitmap = interRect.y - pageRect.y;
            // and to refer it to canvas window coordinates we need to
            // substract scroll origin
            int xCanvas = interRect.x - canvasOffset.x,
                yCanvas = interRect.y - canvasOffset.y;

            //wxLogMessage(_T("nPag=%d, canvasOrg (%d,%d), bitmapOrg (%d, %d), interRec (%d, %d, %d, %d), pageRect (%d, %d, %d, %d)"),
            //    nPag, xCanvas, yCanvas, xBitmap, yBitmap,
            //    interRect.x, interRect.y, interRect.width, interRect.height,
            //    pageRect.x, pageRect.y, pageRect.width, pageRect.height);

            // Copy the damaged rectangle onto the device DC
            pDC->Blit(xCanvas, yCanvas, interRect.width, interRect.height,
                        &memoryDC, xBitmap, yBitmap);

            // deselect the bitmap
            memoryDC.SelectObject(wxNullBitmap);

            ////DEBUG: draw red rectangle to show updated rectangle
            //pDC->SetBrush(*wxTRANSPARENT_BRUSH);
            //pDC->SetPen(*wxRED_PEN);
            //pDC->DrawRectangle(xCanvas, yCanvas, interRect.width, interRect.height);

            //paint backgroud in the remaining area
            //TODO

            //draw page cast shadow
            // to refer page rectangle to canvas window coordinates we need to
            // substract scroll origin
            int xRight = pageRect.x + pageRect.width - canvasOffset.x,
                yTop = pageRect.y - canvasOffset.y,
                xLeft = pageRect.x - canvasOffset.x,
                yBottom = pageRect.y + pageRect.height - canvasOffset.y;
            pDC->SetBrush(*wxBLACK_BRUSH);
            pDC->SetPen(*wxBLACK_PEN);
            pDC->DrawRectangle(xRight, yTop + nBottomShadowHeight,
                               nRightShadowWidth, pageRect.height);
            pDC->DrawRectangle(xLeft + nRightShadowWidth, yBottom,
                               pageRect.width, nBottomShadowHeight);

        }
        else
        {
            //intersection is null. Just repaint background
            //TODO
        }

		//verify if intergap needs repaint
		//intergap rectangle, referred to ViewOrg, is at:
        wxRect bgIntergap(m_xBorder,
						  m_yBorder + nPag * (m_yPageSizeD+m_yInterpageGap) + m_yPageSizeD,
                          m_xPageSizeD,
                          m_yInterpageGap);
		bgIntergap.Intersect(drawRect);
		if (bgIntergap.width > 0 && bgIntergap.height > 0)
		{
			//rectangle, referred to CanvasOrg is at:
			wxRect bgRect(bgIntergap.x - canvasOffset.x, bgIntergap.y - canvasOffset.y, bgIntergap.width, bgIntergap.height);
			wxBrush brush(m_colorBg, wxSOLID);
			wxPen pen(m_colorBg);
			pDC->SetBrush(brush);
			pDC->SetPen(pen);
			pDC->DrawRectangle(bgRect);

			////DEBUG: Draw marks
			//int nPoint = 0;
			//lmPixels yPos = bgIntergap.y - canvasOffset.y;
			//wxColour yellow(255, 255, 0);    //yellow
			//wxPen penY(yellow);
			//pDC->SetPen(*wxRED_PEN);
			//pDC->DrawPoint(bgIntergap.x - canvasOffset.x, yPos++);
			//pDC->SetPen(penY);
			//pDC->DrawPoint(bgIntergap.x - canvasOffset.x, yPos++);
			//nPoint += 2;
			//while(nPoint < m_yBorder)
			//{
			//    pDC->SetPen((nPoint % 2 == 0 ? *wxGREEN_PEN : penY));
			//    pDC->DrawPoint(bgIntergap.x - canvasOffset.x, yPos++);
			//    nPoint++;
			//}
			//pDC->SetPen(*wxRED_PEN);
			//pDC->DrawPoint(bgIntergap.x - canvasOffset.x, bgIntergap.y - canvasOffset.y + bgIntergap.height - 1);
			//wxLogMessage(_T("ScoreView: drawRect=(%d, %d, %d, %d), bgRect=(%d, %d, %d, %d), xBorder=%d, yIntergap=%d, canvasOffset=(%d, %d), yPos=%d"),
			//    drawRect.x, drawRect.y, drawRect.width, drawRect.height,
			//    bgRect.x, bgRect.y, bgRect.width, bgRect.height,
			//    m_xBorder, m_yInterpageGap, canvasOffset.x, canvasOffset.y, yPos-1 );

		}

        //verify if we should finish the loop
        if (fPreviousPageVisible) break;
    }

    ////DEBUG: draw cyan rectangle to show updated rectangle
    //pDC->SetBrush(*wxTRANSPARENT_BRUSH);
    //pDC->SetPen(*wxCYAN_PEN);
    //pDC->DrawRectangle(repaintRect);


}

void lmScoreView::SaveAsImage(wxString& sFilename, wxString& sExt, int nImgType)
{
    //compute required screen size (pixels) for 1:1 renderization
    wxClientDC dc(m_pCanvas);
    dc.SetMapMode(lmDC_MODE);
    dc.SetUserScale( 1.0, 1.0 );
    wxSize pageSize = m_Paper.GetPaperSize();
    int paperWidth = dc.LogicalToDeviceXRel(pageSize.GetWidth());
    int paperHeight = dc.LogicalToDeviceYRel(pageSize.GetHeight());

    //Prepare the GraphicManager
    //m_Paper.SetDC(&dc);           //the layout phase requires a DC
    m_Paper.SetDrawer(new lmDirectDrawer(&dc));
    lmScore* pScore = ((lmScoreDocument*)GetDocument())->GetScore();
    m_graphMngr.Prepare(pScore, paperWidth, paperHeight, 1.0, &m_Paper);

    //Now proceed to export images
    m_graphMngr.ExportAsImage(sFilename, sExt, nImgType);

}

void lmScoreView::DumpBitmaps()
{
    wxString sFilename = _T("lenmus_bitmap");
    wxString sExt = _T("jpg");
    m_graphMngr.BitmapsToFile(sFilename, sExt, wxBITMAP_TYPE_JPEG);
}

void lmScoreView::DrawCursor()
{
    if (!m_pCanvas) return;

	//get pointed object
	if (m_pCursorSO == (lmStaffObj*)NULL)
	{
        SetInitialCursorPosition();
	    if (m_pCursorSO == (lmStaffObj*)NULL) return;
    }


	//get cursor position
    //if we are going to show cursor. Get new position
    //else, if we are going to hide cursor,  use current cursor pos.
    if (!m_fCursorShown)
    {
        //we are going to show cursor. Get new position if object changed
        if (m_pCursorSO->GetID() != m_nCursorIdSO)
        {
	        lmVStaff* pVStaff = m_pCursorInstr->GetVStaff(m_nCursorStaff);
	        m_oCursorPos = m_pCursorSO->GetOrigin();
	        m_oCursorPos.y -= pVStaff->TenthsToLogical(10, m_nCursorStaff);
	        m_udyLength = pVStaff->TenthsToLogical(60, m_nCursorStaff);
	        m_udxSegment = pVStaff->TenthsToLogical(5, m_nCursorStaff);
            m_nCursorIdSO = m_pCursorSO->GetID();
        }
    }
    m_fCursorShown = !m_fCursorShown;       //toggle status

	// prepare DC
    wxClientDC dc(m_pCanvas);
	dc.SetBrush(*wxBLUE_BRUSH);
	int vxlineWidth = 1;
	wxColour color(255,255,0);		//XOR transforms it in the complementary: blue
	wxPen pen(color, vxlineWidth);
	dc.SetPen(pen);
	dc.SetLogicalFunction(wxXOR);

	//cursor geometry
	lmDPoint pointD;
	LogicalToDevice(lmUPoint(m_oCursorPos.x, m_oCursorPos.y), pointD);
	lmPixels vxLine = pointD.x;
	lmPixels vyTop = pointD.y;

    dc.SetMapMode(lmDC_MODE);
    dc.SetUserScale( m_rScale, m_rScale );
	lmPixels vyBottom = vyTop + dc.LogicalToDeviceYRel(m_udyLength);
	lmPixels vdxSegment = dc.LogicalToDeviceYRel(m_udxSegment);
    dc.SetMapMode(wxMM_TEXT);
    dc.SetUserScale(1.0, 1.0);

	//draw vertical line
	dc.DrawLine(vxLine, vyTop, vxLine, vyBottom);

	//draw horizontal segments
	dc.DrawLine(vxLine-vdxSegment, vyTop-1, vxLine+vdxSegment+1, vyTop-1);
	dc.DrawLine(vxLine-vdxSegment, vyBottom, vxLine+vdxSegment+1, vyBottom);

}

void lmScoreView::OnCursorTimer(wxTimerEvent& event)
{
    if (m_fCursorEnabled)
    	DrawCursor();
}

void lmScoreView::UpdateCursor()
{
    //for inmediate visual feedback, after changing cursor position,
    //we are going to hide current cursor and to display the new one

    //hide old cursor
    if (m_fCursorShown)
        DrawCursor();

    //show new cursor
    DrawCursor();

    //restart timer
    m_oCursorTimer.Start(lmCURSOR_BLINKING_RATE, wxTIMER_CONTINUOUS);

}

void lmScoreView::SetInitialCursorPosition()
{
    if (m_pCursorIT) return;
	if (m_pCursorSO == (lmStaffObj*)NULL)
	{
		if (m_pCursorInstr == (lmInstrument*)NULL) return;
		lmVStaff* pVStaff = m_pCursorInstr->GetVStaff(m_nCursorStaff);

		//loop to process all StaffObjs in this measure
		m_pCursorIT = pVStaff->CreateIterator(eTR_ByTime);
		if (m_pCursorIT->EndOfList())
		{
			//the score is empty. Place cursor at start of first staff
		}
		else
		{
			//place cursor at first StaffObj
			m_pCursorSO = m_pCursorIT->GetCurrent();
		}
	}

}

void lmScoreView::EnableCursor(bool fEnable)
{
    //TODO
}

void lmScoreView::CursorRight()
{
    if (!m_pCanvas) return;
	if (m_pCursorSO == (lmStaffObj*)NULL) return;
	if (m_pCursorInstr == (lmInstrument*)NULL) return;
    if (m_pCursorIT == (lmStaffObjIterator*)NULL) return;

    //stopt cursor timer
    m_oCursorTimer.Stop();

	//advance to next staff obj.
	if (!m_pCursorIT->EndOfList()) {
        m_pCursorIT->MoveNext();
	    if (m_pCursorIT->EndOfList()) {
            //continue in last item
            m_pCursorIT->MoveLast();
        }
        m_pCursorSO = m_pCursorIT->GetCurrent();
    }

    //for inmediate visual feedback, hide current cursor and display new one
    UpdateCursor();

}

void lmScoreView::CursorLeft()
{
    if (!m_pCanvas) return;
	if (m_pCursorSO == (lmStaffObj*)NULL) return;
	if (m_pCursorInstr == (lmInstrument*)NULL) return;
    if (m_pCursorIT == (lmStaffObjIterator*)NULL) return;

    //stopt cursor timer
    m_oCursorTimer.Stop();

	//go back to previous staff obj.
	if (!m_pCursorIT->EndOfList()) {
        m_pCursorIT->MovePrev();
	    if (m_pCursorIT->EndOfList()) {
            //continue in first item
            m_pCursorIT->MoveFirst();
        }
        m_pCursorSO = m_pCursorIT->GetCurrent();
    }

    //for inmediate visual feedback, hide current cursor and display new one
    UpdateCursor();

}

void lmScoreView::CursorUp()
{
	wxMessageBox(_T("lmScoreView::CursorUp"));
}

void lmScoreView::CursorDown()
{
    lmBoxScore* pBS = m_graphMngr.GetBoxScore();
    lmBoxPage* pBP = pBS->GetPage(1);
	wxMessageBox(_T("lmScoreView::CursorDown"));
}

void lmScoreView::CursorAtPoint(lmUPoint& point)
{
	wxMessageBox(_T("lmScoreView::CurserAtPoint"));
}