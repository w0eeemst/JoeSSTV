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



//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "ZoomView.h"
#include "ComLib.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TZoomViewDlg::TZoomViewDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	pBitmap = NULL;
	pBitmapS = NULL;

	m_MaxX = ::GetSystemMetrics(SM_CXFULLSCREEN);
	m_MaxY = ::GetSystemMetrics(SM_CYFULLSCREEN);
	m_DisEvent = 0;
}
//---------------------------------------------------------------------
__fastcall TZoomViewDlg::~TZoomViewDlg()
{
	if( pBitmapS != NULL ){
		delete pBitmapS;
	}
}
//---------------------------------------------------------------------
void __fastcall TZoomViewDlg::SetInitSize(int xw, int yw)
{
	m_DisEvent++;
	ClientWidth = xw;
	ClientHeight = yw;
	m_DisEvent--;
}
//---------------------------------------------------------------------
void __fastcall TZoomViewDlg::UpdateTitle(void)
{
	if( pBitmap == NULL ) return;

	char bf[256];

	strcpy(bf, MsgEng ? "Visor de im�genes" : "�摜�\��");

	char bbf[64];
	sprintf(bbf, " - Fuente:%dx%d", pBitmap->Width, pBitmap->Height);
	strcat(bf, bbf);
	if( (pBitmap->Width != m_ViewXW) || (pBitmap->Height != m_ViewYW) ){
		sprintf(bbf, ", Ver:%dx%d", m_ViewXW, m_ViewYW);
		strcat(bf, bbf);
		if( pBitmapS != NULL ) strcat(bf, " (Suavizado)");
	}
	Caption = bf;
}
//---------------------------------------------------------------------
void __fastcall TZoomViewDlg::Execute(Graphics::TBitmap *pbmp, int sw)
{
	pBitmap = pbmp;
	if( pbmp->Width != 320 ){
		int xw = pbmp->Width;
		int yw = pbmp->Height;
		if( (xw > m_MaxX) || (yw > m_MaxY) ){
			if( xw > m_MaxX ) xw = m_MaxX;
			if( yw > m_MaxY ) yw = m_MaxY;
			KeepAspect(xw, yw, pbmp->Width, pbmp->Height);
		}
		m_DisEvent++;
		ClientWidth = xw;
		ClientHeight = yw;
		m_DisEvent--;
//        ClientWidth = pbmp->Width;
//        ClientHeight = pbmp->Height;
	}
	Timer->Enabled = sw;
	UpdateViewSize();
	UpdateTitle();
	ShowModal();
	Timer->Enabled = FALSE;
}
//---------------------------------------------------------------------
void __fastcall TZoomViewDlg::FormPaint(TObject *Sender)
{
	if( pBitmap == NULL ) return;
	if( pBitmapS != NULL ){
    	int x = (ClientWidth - pBitmapS->Width)/2;
    	int y = (ClientHeight - pBitmapS->Height)/2;
		Canvas->Draw(x, y, pBitmapS);
	}
	else {
    	UpdateViewSize();
		TRect rc;
		rc.Left = (ClientWidth - m_ViewXW)/2;
		rc.Top = (ClientHeight - m_ViewYW)/2;
		rc.Right = rc.Left + m_ViewXW;
		rc.Bottom = rc.Top + m_ViewYW;
		::SetStretchBltMode(Canvas->Handle, HALFTONE);
		Canvas->StretchDraw(rc, (TGraphic*)pBitmap);
	}
#if 0
	char bf[256];
	sprintf(bf, "%ux%u", ClientWidth, ClientHeight);
	Canvas->Font->Color = clBlack;
	Canvas->TextOut(0, 0, bf);
#endif
}
//---------------------------------------------------------------------
void __fastcall TZoomViewDlg::TimerTimer(TObject *Sender)
{
	if( (Visible == TRUE) && (pBitmapS == NULL) ) FormPaint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TZoomViewDlg::FormResize(TObject *Sender)
{
	if( m_DisEvent ) return;
	m_DisEvent++;
	if( ClientHeight < 100 ) ClientHeight = 100;
	if( ClientWidth < 100 ) ClientWidth = 100;

	if( (ClientWidth != pBitmap->Width) || (ClientHeight != pBitmap->Height) ){
		double XW = pBitmap->Width;
		double YW = pBitmap->Height;
		double xw = ClientWidth;
		double yw = ClientHeight;
		if( Width > m_MaxX ) xw = m_MaxX;
		if( Height > m_MaxY ) yw = m_MaxY;

		double xv = (xw / Width) * XW;
		double yv = (yw / Height) * YW;
		if( yv < xv ){
			yw = xw * YW / XW;
		}
		else {
			xw = yw * XW / YW;
		}
		if( ABS(xw - ClientWidth) >= 2 ) ClientWidth = xw;
		if( ABS(yw - ClientHeight) >= 2 ) ClientHeight = yw;
    	UpdateViewSize();
	}
	else {
		delete pBitmapS;
		pBitmapS = NULL;
		UpdateTitle();
	}
	if( pBitmapS != NULL ){
		StretchCopy(pBitmap);
	}
	Invalidate();
	m_DisEvent--;
	UpdateTitle();
}
//---------------------------------------------------------------------------
// ���x�ȐL�k�R�s�[
void __fastcall TZoomViewDlg::StretchCopy(Graphics::TBitmap *pbmp)
{
	if( pBitmapS == NULL ){
		pBitmapS = new Graphics::TBitmap;
	}
	pBitmapS->PixelFormat = pf24bit;
	pBitmapS->Width = m_ViewXW;
	pBitmapS->Height = m_ViewYW;
	::StretchCopy(pBitmapS, NULL, pbmp, NULL);
}
//---------------------------------------------------------------------------
void __fastcall TZoomViewDlg::FormClick(TObject *Sender)
{
	if( (pBitmap->Width == ClientWidth) && (pBitmap->Height == ClientHeight) ) return;
	if( pBitmapS != NULL ){
		delete pBitmapS;
		pBitmapS = NULL;
	}
	else {
		StretchCopy(pBitmap);
	}
	UpdateTitle();
	Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TZoomViewDlg::UpdateViewSize(void)
{
	m_ViewXW = ClientWidth;
    m_ViewYW = ClientHeight;
	if( WindowState == wsMaximized ){
		KeepAspect(m_ViewXW, m_ViewYW, pBitmap->Width, pBitmap->Height);
    }
}


