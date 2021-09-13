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

#include "ExtCmd.h"
#include "Main.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TExtCmdDlg::TExtCmdDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Caption = "Asignar Programa";
		CancelBtn->Caption = "Cancelar";
		L1->Caption = "Nombre";
		L2->Caption = "Archivos del programa";
		RefBtn->Caption = "Ref.";
		CBR->Caption = "Eeste programa usar� la misma tarjeta de sonido";
	}
}
//---------------------------------------------------------------------
int __fastcall TExtCmdDlg::Execute(AnsiString &Name, AnsiString &Cmd, int &Susp)
{
	EM->Text = Name;
	EC->Text = Cmd;
	CBR->Checked = Susp;
	if( ShowModal() == IDOK ){
		if( !EC->Text.IsEmpty() ){
			Name = EM->Text;
			Cmd = EC->Text;
			Susp = CBR->Checked;
			return TRUE;
		}
	}
	return FALSE;
}
//---------------------------------------------------------------------
void __fastcall TExtCmdDlg::RefBtnClick(TObject *Sender)
{
	char bf[256];
	SetDirName(bf, AnsiString(EC->Text).c_str());	//ja7ude 0428
	if( !bf[0] ){
		strcpy(bf, "C:\\Program Files");
	}
	TOpenDialog *pDialog = Mmsstv->OpenDialog;
	pDialog->Options >> ofCreatePrompt;
	pDialog->Options << ofFileMustExist;
	if( MsgEng ){
		pDialog->Title = "Archivo de programa";
		pDialog->Filter = "Program files(*.exe;*.lnk)|*.exe;*.lnk|";
	}
	else {
		pDialog->Title = "�v���O�����t�@�C��";
		pDialog->Filter = "�v���O�����t�@�C��(*.exe;*.lnk)|*.exe;*.lnk|";
	}
	pDialog->FileName = "";
	pDialog->DefaultExt = "exe";
	pDialog->InitialDir = bf;
	SetDisPaint();
	NormalWindow(this);
	if( Mmsstv->OpenDialogExecute(FALSE) == TRUE ){
		EC->Text = pDialog->FileName.c_str();
	}
	ResDisPaint();
	TopWindow(this);
}
//---------------------------------------------------------------------------


