#include "StdAfx.h"
#include "TrayIcon.h"

namespace DuiLib
{

	CDuiTrayIcon::CDuiTrayIcon(void)
	{
		memset(&m_trayData, 0, sizeof(m_trayData));
		m_bEnabled		= false;
		m_bVisible		= false;
		m_bTwinkling	= false;
		m_hWnd			= NULL;
		pIDuiTimer		= NULL;
		m_uMessage		= UIEVENT_TRAYICON;
	}


	CDuiTrayIcon::~CDuiTrayIcon(void)
	{
		DeleteTrayIcon();
	}

	//************************************
	// 函数名称: CreateTrayIcon
	// 返回类型: void
	// 参数信息: HWND _RecvHwnd
	// 参数信息: LPCTSTR _ToolTipText
	// 参数信息: UINT _Message
	// 参数信息: HICON _DefaultIcon
	// 函数说明: 
	//************************************
	void CDuiTrayIcon::CreateTrayIcon( HWND _RecvHwnd,UINT _IconIDResource,LPCTSTR _ToolTipText /*= NULL*/,UINT _Message /*= UIEVENT_TRAYICON*/)
	{
		if(_Message == NULL)
			_Message = UIEVENT_TRAYICON;

		if(!_RecvHwnd || _IconIDResource <= 0 || _Message < 0)
			return;

		m_hIcon = LoadIcon(CPaintManagerUI::GetInstance(),MAKEINTRESOURCE(_IconIDResource));

		m_trayData.cbSize = sizeof(NOTIFYICONDATA);
		m_trayData.hWnd	 = _RecvHwnd;
		m_trayData.uID	 = _IconIDResource;
		m_trayData.hIcon = m_hIcon;
		m_trayData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		m_trayData.uCallbackMessage = _Message;
		if(_ToolTipText)
		{
#ifndef UNICODE
			strcpy(m_trayData.szTip,_ToolTipText);
#else
			wcscpy(m_trayData.szTip,_ToolTipText);
#endif
		}

		Shell_NotifyIcon(NIM_ADD,&m_trayData);
		m_bEnabled = true;
	}
	
	//************************************
	// 函数名称: DeleteTrayIcon
	// 返回类型: void
	// 函数说明: 
	//************************************
	void CDuiTrayIcon::DeleteTrayIcon()
	{
		if(pIDuiTimer)
			StopTwinkling();

		Shell_NotifyIcon(NIM_DELETE,&m_trayData);
		m_bEnabled		= false;
		m_bVisible		= false;
		m_bTwinkling	= false;
		m_hWnd			= NULL;
		m_uMessage		= UIEVENT_TRAYICON;
	}

	//************************************
	// 函数名称: SetTooltipText
	// 返回类型: bool
	// 参数信息: LPCTSTR _ToolTipText
	// 函数说明: 
	//************************************
	bool CDuiTrayIcon::SetTooltipText( LPCTSTR _ToolTipText )
	{
		if(_ToolTipText)
		{
#ifndef UNICODE
			strcpy(m_trayData.szTip,_ToolTipText);
#else
			wcscpy(m_trayData.szTip,_ToolTipText);
#endif
		}

		if (!m_bEnabled) return FALSE;
		m_trayData.uFlags = NIF_TIP;
		return Shell_NotifyIcon(NIM_MODIFY, &m_trayData) == TRUE;
	}

	//************************************
	// 函数名称: SetTooltipText
	// 返回类型: bool
	// 参数信息: UINT _IDResource
	// 函数说明: 
	//************************************
	bool CDuiTrayIcon::SetTooltipText( UINT _IDResource )
	{
		TCHAR mbuf[64];
		LoadString(CPaintManagerUI::GetInstance(),_IDResource,mbuf,64);

		return SetTooltipText((LPCTSTR)mbuf);
	}

	//************************************
	// 函数名称: GetTooltipText
	// 返回类型: UiLib::CDuiString
	// 函数说明: 
	//************************************
	DuiLib::CDuiString CDuiTrayIcon::GetTooltipText() const
	{
		return m_trayData.szTip;
	}

	//************************************
	// 函数名称: SetIcon
	// 返回类型: bool
	// 参数信息: HICON _Hicon
	// 函数说明: 
	//************************************
	bool CDuiTrayIcon::SetIcon( HICON _Hicon )
	{
		if(_Hicon)
			m_hIcon = _Hicon;

		m_trayData.uFlags = NIF_ICON;
		m_trayData.hIcon = _Hicon;
		
		if (!m_bEnabled) return FALSE;
		return Shell_NotifyIcon(NIM_MODIFY, &m_trayData) == TRUE;

		return false;
	}

	//************************************
	// 函数名称: SetIcon
	// 返回类型: bool
	// 参数信息: LPCTSTR _IconFile
	// 函数说明: 
	//************************************
	bool CDuiTrayIcon::SetIcon( LPCTSTR _IconFile )
	{
		HICON hIcon = LoadIcon(CPaintManagerUI::GetInstance(),_IconFile);
		return SetIcon(hIcon);
	}

	//************************************
	// 函数名称: SetIcon
	// 返回类型: bool
	// 参数信息: UINT _IDResource
	// 函数说明: 
	//************************************
	bool CDuiTrayIcon::SetIcon( UINT _IDResource )
	{
		HICON hIcon = LoadIcon(CPaintManagerUI::GetInstance(),MAKEINTRESOURCE(_IDResource));
		return SetIcon(hIcon);
	}

	//************************************
	// 函数名称: GetIcon
	// 返回类型: HICON
	// 函数说明: 
	//************************************
	HICON CDuiTrayIcon::GetIcon() const
	{
		HICON hIcon = NULL;
		hIcon = m_trayData.hIcon;
		return hIcon;
	}

	//************************************
	// 函数名称: SetHideIcon
	// 返回类型: void
	// 函数说明: 
	//************************************
	void CDuiTrayIcon::SetHideIcon()
	{
		if (IsVisible()) {
			SetIcon((HICON)NULL);
			m_bVisible = TRUE;
		}
	}

	//************************************
	// 函数名称: SetShowIcon
	// 返回类型: void
	// 函数说明: 
	//************************************
	void CDuiTrayIcon::SetShowIcon()
	{
		if (!IsVisible()) {
			SetIcon(m_hIcon);
			m_bVisible = FALSE;
		}
	}

	//************************************
	// 函数名称: RemoveIcon
	// 返回类型: void
	// 函数说明: 
	//************************************
	void CDuiTrayIcon::RemoveIcon()
	{
		m_trayData.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE, &m_trayData);
		m_bEnabled = FALSE;
	}

	//************************************
	// 函数名称: StartTwinkling
	// 返回类型: bool
	// 函数说明: 
	//************************************
	bool CDuiTrayIcon::StartTwinkling()
	{
		if(m_bTwinkling || !m_bEnabled || pIDuiTimer)
			return false;

		pIDuiTimer = MakeDuiTimer(this,&CDuiTrayIcon::OnTimer,400);
		pIDuiTimer->SetDuiTimer();
		m_bTwinkling = true;
		return true;
	}

	//************************************
	// 函数名称: StopTwinkling
	// 返回类型: void
	// 函数说明: 
	//************************************
	void CDuiTrayIcon::StopTwinkling()
	{
		if(pIDuiTimer){
			pIDuiTimer->KillDuiTimer();
			delete pIDuiTimer;
			pIDuiTimer = NULL;
		}
		m_bTwinkling = false;
		SetShowIcon();
	}

	//************************************
	// 函数名称: OnTimer
	// 返回类型: void
	// 参数信息: IDuiTimer * pTimer
	// 函数说明: 
	//************************************
	void CDuiTrayIcon::OnTimer( IDuiTimer* pTimer )
	{
		IsVisible()?SetHideIcon():SetShowIcon();
	}
}
