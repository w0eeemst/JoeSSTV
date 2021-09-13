//Copyright+LGPL

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Copyright 2000-2013 Makoto Mori, Nobuyuki Oba
//-----------------------------------------------------------------------------------------------------------------------------------------------
// This file is part of MMSSTV.

// MMSSTV is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// MMSSTV is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License along with MMTTY.  If not, see 
// <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------
#ifndef LogLinkH
#define LogLinkH

#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>

#include "ComLib.h"
#include "LogConv.h"
#include "MMLink.h"

#define	LINKINTERVAL	200

//------------ Command of Hamlog -------------
#define  THW_ENTER          0x10000
#define  THW_FOCUS          0x20000
#define  THW_SAVEBOX_ON     0x40000
#define  THW_SAVEBOX_OFF    0x80000
#define  THW_APPLIHWND      0x100000

//---------------------------------------------------------------------------
// CLogLink�N���X
class CLogLink
{
private:
	int     m_Enabled;      // �����N�̋���
	int     m_Polling;      // ���g���|�[�����O
	BOOL	m_PTTEnabled;	// PTT�R���g���[��
	BOOL	m_1stSession;

	HWND	m_hLog;			// Hamlog�v���O�����̃n���h��
	HWND	m_hLogIn;		// Hamlog���̓E�C���h�E�̃n���h��

	COPYDATASTRUCT  m_cds;	// �C���^�[�t�F�[�X�\����
	int		m_Count;		// �Ď��J�E���^
	int     m_CStat;        // ��ԕύX�Z�}�t�H
	int     m_Count2;       // �|�[�����O�J�E���^

	HWND    m_hMain;        // ���C����ʂ̃E�C���h�E�n���h��
	HWND    m_hApp;         // �A�v���P�[�V�����̃E�C���h�E�n���h��
	BOOL	m_fHLV5;		// Hamlog Ver 5.0

	int     m_Error;

	AnsiString	m_ItemName;
	AnsiString	m_QueryCall;
    AnsiString	m_AppName;
    int		m_AppVer;
	int		m_MMStat;
	int		m_AStat;
	UINT	m_uMsg;

    char		m_strMode[16];
public:
	CMMLink	*m_pLink;
private:
	void __fastcall CloseMMLink(void);

public:
	CLogLink();
	~CLogLink();
	void SetHandle(HWND hMain, UINT uMsg);
	BOOL IsEnabled(void){return m_Enabled;};
	void UpdateLink(int sw);
	void Foreground(void);
	BOOL IsError(void){return m_Error;};
	void ClearError(void){m_Error = 0;};
	BOOL __fastcall IsLink(void);
	LPCSTR __fastcall GetSessionName(void);

	BOOL CheckLink(void);
	BOOL TimerLogLink(void);
	void Clear(void);
	void SetFocus(void);
	void SetCall(LPSTR pCall, int sw);
	void FindCall(LPSTR pCall);
	void Write(SDMMLOG *sp, int sw);
	int AnaData(SDMMLOG *sp, COPYDATASTRUCT *cp);

	BOOL IsPolling(void){return m_Polling;};
	void SetPolling(int sw){m_Polling = sw;};
	void EventGetFreq(void);
	void SetFreq(LPSTR pFreq);
	void SetMode(LPCSTR pMode);
	void __fastcall SetPTT(int ptt);

	void __fastcall SetTime(SYSTEMTIME *tp, int sw);

public:
	void __fastcall LoadMMLink(TMemIniFile *pIni);
	void __fastcall SaveMMLink(TMemIniFile *pIni);
	void __fastcall NotifySession(LPCSTR pSession);
	int __fastcall QReturn(SDMMLOG *sp, const mmLOGDATA *pLog);
	LPCSTR __fastcall GetItemName(void);
	void __fastcall SetItemName(LPCSTR pName);
	BOOL __fastcall IsCopyData(void);

    inline BOOL __fastcall GetPTTEnabled(void){return m_PTTEnabled;};
    inline void __fastcall SetPTTEnabled(BOOL e){m_PTTEnabled = e;};
};
extern  CLogLink    LogLink;
#endif

