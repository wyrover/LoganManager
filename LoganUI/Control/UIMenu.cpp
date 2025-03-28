#include "StdAfx.h"
#include "UIMenu.h"



namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
ObserverImpl s_context_menu_observer;

CMenuUI::CMenuUI()
{
	if (GetHeader() != NULL)
		GetHeader()->SetVisible(false);
}

CMenuUI::~CMenuUI()
{}

LPCTSTR CMenuUI::GetClass() const
{
    return _T("MenuUI");
}

LPVOID CMenuUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, _T("Menu")) == 0 ) return static_cast<CMenuUI*>(this);
    return CListUI::GetInterface(pstrName);
}

void CMenuUI::DoEvent(TEventUI& event)
{
	return __super::DoEvent(event);
}

bool CMenuUI::Add(CControlUI* pControl)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
	if (pMenuItem == NULL)
		return false;

	for (int i = 0; i < pMenuItem->GetCount(); ++i)
	{
		if (pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL)
		{
			(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(false);
		}
	}
	return CListUI::Add(pControl);
}

bool CMenuUI::AddAt(CControlUI* pControl, int iIndex)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
	if (pMenuItem == NULL)
		return false;

	for (int i = 0; i < pMenuItem->GetCount(); ++i)
	{
		if (pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL)
		{
			(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(false);
		}
	}
	return CListUI::AddAt(pControl, iIndex);
}

int CMenuUI::GetItemIndex(CControlUI* pControl) const
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
	if (pMenuItem == NULL)
		return -1;

	return __super::GetItemIndex(pControl);
}

bool CMenuUI::SetItemIndex(CControlUI* pControl, int iIndex)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
	if (pMenuItem == NULL)
		return false;

	return __super::SetItemIndex(pControl, iIndex);
}

bool CMenuUI::Remove(CControlUI* pControl)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
	if (pMenuItem == NULL)
		return false;

	return __super::Remove(pControl);
}

SIZE CMenuUI::EstimateSize(SIZE szAvailable)
{
	int cxFixed = 0;
    int cyFixed = 0;
    for( int it = 0; it < GetCount(); it++ ) {
        CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
        if( !pControl->IsVisible() ) continue;
        SIZE sz = pControl->EstimateSize(szAvailable);
        cyFixed += sz.cy;
		if( cxFixed < sz.cx )
			cxFixed = sz.cx;
    }
    return CSize(cxFixed, cyFixed);
}

void CMenuUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	CListUI::SetAttribute(pstrName, pstrValue);
}

/////////////////////////////////////////////////////////////////////////////////////
//

CMenuWnd::CMenuWnd(HWND hParent):
m_hParent(hParent),
m_pOwner(NULL),
m_pLayout(),
m_xml(_T("")),
m_bShowOnTop(false),
m_bFirstEnterSub(false)
{

}

CMenuWnd::~CMenuWnd()
{

}

BOOL CMenuWnd::Receive(ContextMenuParam param)
{
	switch (param.wParam)
	{
	case 1:
		Close();
		break;
	case 2:
		{
			HWND hParent = GetParent(m_hWnd);
			while (hParent != NULL)
			{
				if (hParent == param.hWnd)
				{
					Close();
					break;
				}
				hParent = GetParent(hParent);
			}
		}
		break;
	default:
		break;
	}

	return TRUE;
}

void CMenuWnd::Init(CMenuElementUI* pOwner, STRINGorID xml, POINT point, 
					CPaintManagerUI* pMainPaintManager/* = NULL*/, map<CDuiString,bool>* pMenuCheckInfo/* = NULL*/, bool bShowOnTop/* = false*/)
{
	m_BasedPoint = point;
    m_pOwner = pOwner;
    m_pLayout = NULL;
	m_bShowOnTop = bShowOnTop;
	m_xml = xml;

	if (pMainPaintManager != NULL)
		s_context_menu_observer.m_pMainWndPaintManager = pMainPaintManager;
	if (pMenuCheckInfo != NULL)
		s_context_menu_observer.m_pMenuCheckInfo = pMenuCheckInfo;
	s_context_menu_observer.AddReceiver(this);

	Create((m_pOwner == NULL) ? m_hParent : m_pOwner->GetManager()->GetPaintWindow(), NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST, CDuiRect());
    // HACK: Don't deselect the parent's caption
    HWND hWndParent = m_hWnd;
    while( ::GetParent(hWndParent) != NULL ) hWndParent = ::GetParent(hWndParent);

    ::ShowWindow(m_hWnd, SW_SHOW);
#if defined(WIN32) && !defined(UNDER_CE)
    ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
#endif	
}

LPCTSTR CMenuWnd::GetWindowClassName() const
{
    return _T("MenuWnd");
}


void CMenuWnd::Notify(TNotifyUI& msg)
{
	if( s_context_menu_observer.m_pMainWndPaintManager ) 
	{
		if( msg.sType == _T("click") || msg.sType == _T("valuechanged") ) 
		{
			s_context_menu_observer.m_pMainWndPaintManager->SendNotify(msg, false);
		}
	}

}

CControlUI* CMenuWnd::CreateControl( LPCTSTR pstrClassName )
{
	if (_tcsicmp(pstrClassName, _T("Menu")) == 0)
	{
		return new CMenuUI();
	}
	else if (_tcsicmp(pstrClassName, _T("MenuElement")) == 0)
	{
		return new CMenuElementUI();
	}
	return NULL;
}


void CMenuWnd::OnFinalMessage(HWND hWnd)
{
	RemoveObserver();
	if( m_pOwner != NULL ) {
		for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
			if( static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(_T("MenuElement"))) != NULL ) {
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pOwner->GetParent());
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetVisible(false);
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(false);
			}
		}
		m_pOwner->m_pWindow = NULL;
		m_pOwner->m_uButtonState &= ~ UISTATE_PUSHED;
		m_pOwner->Invalidate();
	}
	m_pOwner = NULL;
	m_pLayout = NULL;
    delete this;
}

LRESULT CMenuWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if( m_pOwner != NULL) {
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		RECT rcClient;
		::GetClientRect(*this, &rcClient);
		::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
			rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

		m_pm.Init(m_hWnd);
		// The trick is to add the items to the new container. Their owner gets
		// reassigned by this operation - which is why it is important to reassign
		// the items back to the righfull owner/manager when the window closes.
		m_pLayout = new CMenuUI();
		m_pm.UseParentResource(m_pOwner->GetManager());
		m_pLayout->SetManager(&m_pm, NULL, true);
		LPCTSTR pDefaultAttributes = m_pOwner->GetManager()->GetDefaultAttributeList(_T("Menu"));
		if( pDefaultAttributes ) {
			m_pLayout->ApplyAttributeList(pDefaultAttributes);
		}
		m_pLayout->GetList()->SetAutoDestroy(false);

		for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
			if(m_pOwner->GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL ){
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pLayout);
				m_pLayout->Add(static_cast<CControlUI*>(m_pOwner->GetItemAt(i)));
			}
		}
		m_pm.AttachDialog(m_pLayout);
		m_pm.AddNotifier(this);
		// Position the popup window in absolute space
		RECT rcOwner = m_pOwner->GetPos();
		RECT rc = rcOwner;

		int cxFixed = 0;
		int cyFixed = 0;

#if defined(WIN32) && !defined(UNDER_CE)
		MONITORINFO oMonitor = {}; 
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
#else
		CDuiRect rcWork;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
		SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };

		for( int it = 0; it < m_pOwner->GetCount(); it++ ) {
			if(m_pOwner->GetItemAt(it)->GetInterface(_T("MenuElement")) != NULL ){
				CControlUI* pControl = static_cast<CControlUI*>(m_pOwner->GetItemAt(it));
				SIZE sz = pControl->EstimateSize(szAvailable);
				cyFixed += sz.cy;

				if( cxFixed < sz.cx )
					cxFixed = sz.cx;
			}
		}
		cyFixed += 4;
		cxFixed += 4;

		RECT rcWindow;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWindow);

		rc.top = rcOwner.top;
		rc.bottom = rc.top + cyFixed;
		::MapWindowRect(m_pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
		rc.left = rcWindow.right;
		rc.right = rc.left + cxFixed;
		rc.right += 2;

		bool bReachBottom = false;
		bool bReachRight = false;
		LONG chRightAlgin = 0;
		LONG chBottomAlgin = 0;

		RECT rcPreWindow = {0};
		ObserverImpl::Iterator iterator(s_context_menu_observer);
		ReceiverImplBase* pReceiver = iterator.next();
		while( pReceiver != NULL ) {
			CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
			if( pContextMenu != NULL ) {
				GetWindowRect(pContextMenu->GetHWND(), &rcPreWindow);

				bReachRight = rcPreWindow.left >= rcWindow.right;
				bReachBottom = rcPreWindow.top >= rcWindow.bottom;
				if( pContextMenu->GetHWND() == m_pOwner->GetManager()->GetPaintWindow() 
					||  bReachBottom || bReachRight )
					break;
			}
			pReceiver = iterator.next();
		}

		if (bReachBottom)
		{
			rc.bottom = rcWindow.top;
			rc.top = rc.bottom - cyFixed;
		}

		if (bReachRight)
		{
			rc.right = rcWindow.left;
			rc.left = rc.right - cxFixed;
		}

		if( rc.bottom > rcWork.bottom )
		{
			rc.bottom = rc.top;
			rc.top = rc.bottom - cyFixed;
		}

		if (rc.right > rcWork.right)
		{
			rc.right = rcWindow.left;
			rc.left = rc.right - cxFixed;

//			rc.top = rcWindow.bottom;
//			rc.bottom = rc.top + cyFixed;
		}

		if( rc.top < rcWork.top )
		{
			rc.top = rcOwner.top;
			rc.bottom = rc.top + cyFixed;
		}

		if (rc.left < rcWork.left)
		{
			rc.left = rcWindow.right;
			rc.right = rc.left + cxFixed;
		}

		MoveWindow(m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
		
	}
	else {
		m_pm.Init(m_hWnd);

		CDialogBuilder builder;

		CControlUI* pRoot = builder.Create(m_xml,UINT(0), this, &m_pm);
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);

#if defined(WIN32) && !defined(UNDER_CE)
		MONITORINFO oMonitor = {}; 
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
#else
		CDuiRect rcWork;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
		SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };
		szAvailable = pRoot->EstimateSize(szAvailable);
		m_pm.SetInitSize(szAvailable.cx, szAvailable.cy);

		DWORD dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top;

		SIZE szInit = m_pm.GetInitSize();
		CDuiRect rc;
		CPoint point = m_BasedPoint;

		if(m_bShowOnTop)
		{
			rc.left = point.x - szInit.cx;
			rc.top = point.y - szInit.cy;
			rc.right = point.x;
			rc.bottom = point.y;
		}
		else
		{
			rc.left = point.x;
			rc.top = point.y;
			rc.right = rc.left + szInit.cx;
			rc.bottom = rc.top + szInit.cy;
		}
		
		

		int nWidth = rc.GetWidth();
		int nHeight = rc.GetHeight();

		if (dwAlignment & eMenuAlignment_Right)
		{
			rc.right = point.x;
			rc.left = rc.right - nWidth;
		}

		if (dwAlignment & eMenuAlignment_Bottom)
		{
			rc.bottom = point.y;
			rc.top = rc.bottom - nHeight;
		}

		SetForegroundWindow(m_hWnd);
		MoveWindow(m_hWnd, rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), FALSE);
		SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, rc.GetWidth(), rc.GetHeight()+8, SWP_SHOWWINDOW);

		m_pm.SetCapture(); 
	}
	return 0;
}

LRESULT CMenuWnd::OnButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL bInMenuWindowList = FALSE;
	ContextMenuParam param;
	param.hWnd = GetHWND();

	ObserverImpl::Iterator iterator(s_context_menu_observer);
	ReceiverImplBase* pReceiver = iterator.next();
	while( pReceiver != NULL ) {
		CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
		if( pContextMenu != NULL ) {
			POINT pt;
			GetCursorPos(&pt);
			if ( WindowFromPoint(pt)== pContextMenu->GetHWND())
			{
				bInMenuWindowList = TRUE;
				break;
			}			
		}
		pReceiver = iterator.next();
	}

	if( !bInMenuWindowList ) {
		param.wParam = 1;
		s_context_menu_observer.RBroadcast(param);

		return 0;
	}
	return 0;
}

LRESULT CMenuWnd::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt;
	GetCursorPos(&pt);
	HWND hCurWnd =  WindowFromPoint(pt);

	ObserverImpl::Iterator iterator(s_context_menu_observer);
	ReceiverImplBase* pReceiver = iterator.next();
	while( pReceiver != NULL ) {
		CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
		if( pContextMenu != NULL ) {
			if ( hCurWnd == pContextMenu->GetHWND())
			{
				::SetCapture(pContextMenu->GetHWND());
				break;
			}			
		}
		pReceiver = iterator.next();
	}
	
	return 0;
}

LRESULT CMenuWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_pm.GetRoundCorner();
	if( !::IsIconic(*this) ) {
		CDuiRect rcWnd;
		::GetWindowRect(*this, &rcWnd);
		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
		rcWnd.right++; rcWnd.bottom++;
		HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
		::SetWindowRgn(*this, hRgn, TRUE);
		::DeleteObject(hRgn);
	}
	bHandled = FALSE;
	return 0;
}

LRESULT CMenuWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch( uMsg )
	{
	case WM_CREATE:       
		lRes = OnCreate(uMsg, wParam, lParam, bHandled); 
		break;
	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE || wParam == VK_LEFT)
			Close();
		break;
	case WM_SIZE:
		lRes = OnSize(uMsg, wParam, lParam, bHandled);
		break;
	case WM_CLOSE:
		if( m_pOwner != NULL )
		{
			m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
			m_pOwner->SetPos(m_pOwner->GetPos());
			m_pOwner->SetFocus();
		}
		break;
	case WM_MOUSEMOVE:
		lRes = OnMouseMove(uMsg, wParam, lParam, bHandled);
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		lRes = OnButtonDown(uMsg, wParam, lParam, bHandled);
		break;	
	case WM_CONTEXTMENU:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		return 0L;
		break;
	default:
		bHandled = FALSE;
		break;
	}

    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) 
		return lRes;
    return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

#pragma endregion
/////////////////////////////////////////////////////////////////////////////////////
//

CMenuElementUI::CMenuElementUI():
m_pWindow(NULL),
m_bDrawLine(false),
m_dwLineColor(DEFAULT_LINE_COLOR),
m_bCheckItem(false),
m_bShowExplandIcon(false)
{
	m_cxyFixed.cy = ITEM_DEFAULT_HEIGHT;
	m_cxyFixed.cx = ITEM_DEFAULT_WIDTH;
	m_szIconSize.cy = ITEM_DEFAULT_ICON_SIZE;
	m_szIconSize.cx = ITEM_DEFAULT_ICON_SIZE;
	m_bMouseChildEnabled = true;
}

CMenuElementUI::~CMenuElementUI()
{}

LPCTSTR CMenuElementUI::GetClass() const
{
	return _T("MenuElementUI");
}

LPVOID CMenuElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, _T("MenuElement")) == 0 ) return static_cast<CMenuElementUI*>(this);    
    return CListContainerElementUI::GetInterface(pstrName);
}

void CMenuElementUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;

	if(m_bDrawLine)
	{
		RECT rcLine = { m_rcItem.left +  DEFAULT_LINE_LEFT_WIDTH, m_rcItem.bottom - 5, m_rcItem.right - DEFAULT_LINE_LEFT_WIDTH, m_rcItem.bottom - 5 };
		CRenderEngine::DrawLine(hDC, rcLine, 1, m_dwLineColor);
	}
	else
	{
		CMenuElementUI::DrawItemBk(hDC, m_rcItem);
		DrawItemText(hDC, m_rcItem);
		DrawItemIcon(hDC, m_rcItem);
		DrawItemExpland(hDC, m_rcItem);
		for (int i = 0; i < GetCount(); ++i)
		{
			if (GetItemAt(i)->GetInterface(_T("MenuElement")) == NULL)
				GetItemAt(i)->DoPaint(hDC, rcPaint);
		}
	}
}

void CMenuElementUI::DrawItemIcon(HDC hDC, const RECT& rcItem)
{
	if ( m_strIcon != _T("") )
	{
		if (!(m_bCheckItem && !GetChecked()))
		{
			CDuiString pStrImage;
			pStrImage.Format(_T("file='%s' dest='%d,%d,%d,%d'"), m_strIcon.GetData(), (ITEM_DEFAULT_ICON_WIDTH - m_szIconSize.cx)/2,
				(m_cxyFixed.cy - m_szIconSize.cy)/2,
				(ITEM_DEFAULT_ICON_WIDTH - m_szIconSize.cx)/2 + m_szIconSize.cx,
				(m_cxyFixed.cy - m_szIconSize.cy)/2 + m_szIconSize.cy);
			CRenderEngine::DrawImageString(hDC, m_pManager, m_rcItem, m_rcPaint, pStrImage, _T(""));
		}			
	}
}

void CMenuElementUI::DrawItemExpland(HDC hDC, const RECT& rcItem)
{
	if (m_bShowExplandIcon)
	{
		CDuiString strExplandIcon;
		strExplandIcon = GetManager()->GetDefaultAttributeList(_T("ExplandIcon"));
		CDuiString strBkImage;
		SIZE szMenu = GetManager()->GetInitSize();
		strBkImage.Format(_T("file='%s' dest='%d,%d,%d,%d'"), strExplandIcon.GetData(), 
			szMenu.cx - ITEM_DEFAULT_EXPLAND_ICON_WIDTH + (ITEM_DEFAULT_EXPLAND_ICON_WIDTH - ITEM_DEFAULT_EXPLAND_ICON_SIZE)/2,
			(m_cxyFixed.cy - ITEM_DEFAULT_EXPLAND_ICON_SIZE)/2,
			szMenu.cx - ITEM_DEFAULT_EXPLAND_ICON_WIDTH + (ITEM_DEFAULT_EXPLAND_ICON_WIDTH - ITEM_DEFAULT_EXPLAND_ICON_SIZE)/2 + ITEM_DEFAULT_EXPLAND_ICON_SIZE,
			(m_cxyFixed.cy - ITEM_DEFAULT_EXPLAND_ICON_SIZE)/2 + ITEM_DEFAULT_EXPLAND_ICON_SIZE);

		CRenderEngine::DrawImageString(hDC, m_pManager, m_rcItem, m_rcPaint, strBkImage, _T(""));
	}
}


void CMenuElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
    if( m_sText.IsEmpty() ) return;

    if( m_pOwner == NULL ) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    DWORD iTextColor = pInfo->dwTextColor;
    if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        iTextColor = pInfo->dwHotTextColor;
    }
    if( IsSelected() ) {
        iTextColor = pInfo->dwSelectedTextColor;
    }
    if( !IsEnabled() ) {
        iTextColor = pInfo->dwDisabledTextColor;
    }
    int nLinks = 0;
    RECT rcText = rcItem;
    rcText.left += pInfo->rcTextPadding.left;
    rcText.right -= pInfo->rcTextPadding.right;
    rcText.top += pInfo->rcTextPadding.top;
    rcText.bottom -= pInfo->rcTextPadding.bottom;

    if( pInfo->bShowHtml )
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, m_sText, iTextColor, \
        NULL, NULL, nLinks, DT_SINGLELINE | pInfo->uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rcText, m_sText, iTextColor, \
        pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
}


SIZE CMenuElementUI::EstimateSize(SIZE szAvailable)
{
	SIZE cXY = {0};
	for( int it = 0; it < GetCount(); it++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
		if( !pControl->IsVisible() ) continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		cXY.cy += sz.cy;
		if( cXY.cx < sz.cx )
			cXY.cx = sz.cx;
	}
	if(cXY.cy == 0) {
		TListInfoUI* pInfo = m_pOwner->GetListInfo();

		DWORD iTextColor = pInfo->dwTextColor;
		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			iTextColor = pInfo->dwHotTextColor;
		}
		if( IsSelected() ) {
			iTextColor = pInfo->dwSelectedTextColor;
		}
		if( !IsEnabled() ) {
			iTextColor = pInfo->dwDisabledTextColor;
		}

		RECT rcText = { 0, 0, MAX(szAvailable.cx, m_cxyFixed.cx), 9999 };
		rcText.left += pInfo->rcTextPadding.left;
		rcText.right -= pInfo->rcTextPadding.right;
		if( pInfo->bShowHtml ) {   
			int nLinks = 0;
			CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, iTextColor, NULL, NULL, nLinks, DT_CALCRECT | pInfo->uTextStyle);
		}
		else {
			CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, iTextColor, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle);
		}
		cXY.cx = rcText.right - rcText.left + pInfo->rcTextPadding.left + pInfo->rcTextPadding.right + 20;
		cXY.cy = rcText.bottom - rcText.top + pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
	}

	if( m_cxyFixed.cy != 0 ) cXY.cy = m_cxyFixed.cy;
	if ( cXY.cx < m_cxyFixed.cx )
		cXY.cx =  m_cxyFixed.cx;

	m_cxyFixed.cy = cXY.cy;
	m_cxyFixed.cx = cXY.cx;
	return cXY;
}

void CMenuElementUI::DoEvent(TEventUI& event)
{

	if( event.Type == UIEVENT_MOUSEENTER )
	{
		CListContainerElementUI::DoEvent(event);
		if( m_pWindow ) return;
		bool hasSubMenu = false;
		for( int i = 0; i < GetCount(); ++i )
		{
			if( GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL )
			{
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);

				hasSubMenu = true;
			}
		}
		if( hasSubMenu )
		{
			m_pOwner->SelectItem(GetIndex(), true);
			CreateMenuWnd();
		}
		else
		{
			ContextMenuParam param;
			param.hWnd = m_pManager->GetPaintWindow();
			param.wParam = 2;
			s_context_menu_observer.RBroadcast(param);
			m_pOwner->SelectItem(GetIndex(), true);
		}
		return;
	}

	if( event.Type == UIEVENT_BUTTONUP )
	{
		if( IsEnabled() ){
			CListContainerElementUI::DoEvent(event);

			if( m_pWindow ) return;

			bool hasSubMenu = false;
			for( int i = 0; i < GetCount(); ++i ) {
				if( GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL ) {
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);

					hasSubMenu = true;
				}
			}
			if( hasSubMenu )
			{
				CreateMenuWnd();
			}
			else
			{
				SetChecked(!GetChecked());
				if (s_context_menu_observer.m_pMainWndPaintManager != NULL)
					PostMessage(s_context_menu_observer.m_pMainWndPaintManager->GetPaintWindow(), WM_MENU, (WPARAM)(new CDuiString(GetName().GetData())), (LPARAM)GetChecked());
				ContextMenuParam param;
				param.hWnd = m_pManager->GetPaintWindow();
				param.wParam = 1;
				s_context_menu_observer.RBroadcast(param);
			}
        }

        return;
    }

	if ( event.Type == UIEVENT_KEYDOWN && event.chKey == VK_RIGHT )
	{
		if( m_pWindow ) return;
		bool hasSubMenu = false;
		for( int i = 0; i < GetCount(); ++i )
		{
			if( GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL )
			{
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);

				hasSubMenu = true;
			}
		}
		if( hasSubMenu )
		{
			m_pOwner->SelectItem(GetIndex(), true);
			CreateMenuWnd();
		}
		else
		{
			ContextMenuParam param;
			param.hWnd = m_pManager->GetPaintWindow();
			param.wParam = 2;
			s_context_menu_observer.RBroadcast(param);
			m_pOwner->SelectItem(GetIndex(), true);
		}

		return;
	}

    CListContainerElementUI::DoEvent(event);
}

bool CMenuElementUI::Activate()
{
	if (CListContainerElementUI::Activate() && m_bSelected)
	{
		if( m_pWindow ) return true;
		bool hasSubMenu = false;
		for (int i = 0; i < GetCount(); ++i)
		{
			if (GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL)
			{
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);

				hasSubMenu = true;
			}
		}
		if (hasSubMenu)
		{
			CreateMenuWnd();
		}
		else
		{
			SetChecked(!GetChecked());
			if (s_context_menu_observer.m_pMainWndPaintManager != NULL)
				PostMessage(s_context_menu_observer.m_pMainWndPaintManager->GetPaintWindow(), WM_MENU, (WPARAM)(new CDuiString(GetName().GetData())), (LPARAM)GetChecked());
			ContextMenuParam param;
			param.hWnd = m_pManager->GetPaintWindow();
			param.wParam = 1;
			s_context_menu_observer.RBroadcast(param);
		}

		return true;
	}
	return false;
}

CMenuWnd* CMenuElementUI::GetMenuWnd()
{
	return m_pWindow;
}

void CMenuElementUI::CreateMenuWnd()
{
	if( m_pWindow ) return;

	m_pWindow = new CMenuWnd(m_pManager->GetPaintWindow());
	ASSERT(m_pWindow);

	ContextMenuParam param;
	param.hWnd = m_pManager->GetPaintWindow();
	param.wParam = 2;
	s_context_menu_observer.RBroadcast(param);


	m_pWindow->Init(static_cast<CMenuElementUI*>(this), _T(""), CPoint());
}

void CMenuElementUI::SetLineType()
{
	m_bDrawLine = true;
	SetFixedHeight(DEFAULT_LINE_HEIGHT);
	SetMouseChildEnabled(false);
	SetMouseEnabled(false);
	SetEnabled(false);
}

void CMenuElementUI::SetLineColor(DWORD color)
{
	m_dwLineColor = color;
}

void CMenuElementUI::SetIcon(LPCTSTR strIcon)
{
	if ( strIcon != _T("") )
		m_strIcon = strIcon;
}

void CMenuElementUI::SetIconSize(LONG cx, LONG cy)
{
	m_szIconSize.cx = cx;
	m_szIconSize.cy = cy;
}

void CMenuElementUI::SetChecked(bool bCheck/* = true*/)
{
	if (!m_bCheckItem || s_context_menu_observer.m_pMenuCheckInfo == NULL )
		return;
	map<CDuiString,bool>::iterator it = s_context_menu_observer.m_pMenuCheckInfo->find(GetName());
	if (it == s_context_menu_observer.m_pMenuCheckInfo->end())
		s_context_menu_observer.m_pMenuCheckInfo->insert(map<CDuiString,bool>::value_type(GetName(),bCheck));
	else
		it->second = bCheck;

}

bool CMenuElementUI::GetChecked() const
{
	if (!m_bCheckItem || s_context_menu_observer.m_pMenuCheckInfo == NULL  || s_context_menu_observer.m_pMenuCheckInfo->size() == 0)
		return false;

	map<CDuiString,bool>::iterator it = s_context_menu_observer.m_pMenuCheckInfo->find(GetName());
	if (it != s_context_menu_observer.m_pMenuCheckInfo->end())
	{
		return it->second;
	}
	return false;
	
}

void CMenuElementUI::SetCheckItem(bool bCheckItem/* = false*/)
{
	m_bCheckItem = bCheckItem;
}

bool CMenuElementUI::GetCheckItem() const
{
	return m_bCheckItem;
}

void CMenuElementUI::SetShowExplandIcon(bool bShow)
{
	m_bShowExplandIcon = bShow;
}
void CMenuElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName, _T("icon")) == 0){
		SetIcon(pstrValue);
	}
	else if( _tcscmp(pstrName, _T("iconsize")) == 0 ) {
		LPTSTR pstr = NULL;
		LONG cx = 0, cy = 0;
		cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);   
		SetIconSize(cx, cy);
	}
	else if( _tcscmp(pstrName, _T("ischeck")) == 0 ) {		
		if (s_context_menu_observer.m_pMenuCheckInfo != NULL && s_context_menu_observer.m_pMenuCheckInfo->find(GetName()) == s_context_menu_observer.m_pMenuCheckInfo->end())
		{
			SetChecked(_tcscmp(pstrValue, _T("true")) == 0 ? true : false);
		}	
	}
	else if( _tcscmp(pstrName, _T("checkitem")) == 0 ) {		
			SetCheckItem(_tcscmp(pstrValue, _T("true")) == 0 ? true : false);		
	}
	else if( _tcscmp(pstrName, _T("type")) == 0){
		if (_tcscmp(pstrValue, _T("line")) == 0)
			SetLineType();
	}
	else if( _tcscmp(pstrName, _T("expland")) == 0 ) {
		SetShowExplandIcon(_tcscmp(pstrValue, _T("true")) == 0 ? true : false);
	}
	else if( _tcscmp(pstrName, _T("linecolor")) == 0){
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		SetLineColor(_tcstoul(pstrValue, &pstr, 16));
	}
	else if	( _tcscmp(pstrName, _T("height")) == 0){
		SetFixedHeight(_ttoi(pstrValue));
	}
	else
		CListContainerElementUI::SetAttribute(pstrName, pstrValue);
}

} // namespace DuiLib
