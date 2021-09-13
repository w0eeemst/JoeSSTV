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

#include "RMenuDlg.h"
#include "Main.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//TRMenuDialog *RMenuDialog;
static LPCSTR g_help={
"�R�}���h\r\n"
"\r\n"
"\\$##...	##=00-FF,  ���M���镡���o�C�g��16�i���Œ�`����\r\n"
"		(��:\\$FE55AA  -> FEh,55h,AAh)\r\n"
"\\x##	##=00-FF,  ���M����P�o�C�g��16�i���Œ�`����\r\n"
"		(��:\\xFE\\x55\\xAA  -> FEh, 55h, AAh)\r\n"
"\\w##	##=00-99,  �f�B���C���Ԃ�10ms�P�ʂ�10�i���Œ�`����\r\n"
"		(��:\\w05  -> wait 50ms)\r\n"
"\\r	�L�����b�W���^�[���𑗐M����\r\n"
"\\n	���C���t�B�[�h�𑗐M����\r\n"
"\\c....	�ȍ~�͂��ׂăR�����g\r\n"
"\\\\	'\\'�����𑗐M����\r\n"
"���̑�	���̕�����𑗐M����\r\n"
"\r\n"
"##�̑����xx���L�q����ƁA���̕�����xx���̓{�b�N�X�Œ�`����16�i���ɒu�������܂��B\r\n"
"���̕ϊ���ICOM�̋@��A�h���X���w�肷��ꍇ�Ɏg�p���܂��B\r\n"
"		��:\\$FEFExxE01C0001FD	�ixx�̕�������`���e�ɒu�������j"
};
static LPCSTR g_helpeng={
" Este comando env�a un comando de radio a la plataforma. Funcionar� para cualquier plataforma. Use la siguiente palabra clave para especificar datos de bytes en n�mero hexadecimal.\r\n"
"\r\n"
"\\$##...		##=00-FF, Especifique la cadena de datos de bytes en formato hexadecimal\r\n"
"			(Example: \\$FE55AA  -> $FE,$55,$AA)\r\n"
"			ICOM CI-V direcci�n puede ser expresada por xx\r\n"
"\\x##		##=00-FF,Especifique la cadena de datos de bytes en formato hexadecimal\r\n"
" 			(Example: \\xFE\\x55\\xAA  -> $FE, $55, $AA)\r\n"
"\\w##		##=00-99, Especifique el tiempo de retardo\r\n"
"			(Example: \\w05  -> espera 50ms)\r\n"
"\\r		Enviar retorno\r\n"
"\\n		Enviar un avance de l�nea\r\n"
"\\c....		Comentario\r\n"
"\\\\		'\\' enviar caracter\r\n"
"Otros		esvia caracter sin modificar"
};

//---------------------------------------------------------------------
__fastcall TRMenuDialog::TRMenuDialog(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;

	if( MsgEng ){
		LH->Font->Name = "Arial";
		LH->Font->Charset = ((TForm *)AOwner)->Font->Charset;

		L1->Caption = "Subt�tulo";
		L2->Caption = "Comando";
		Caption = "Editar man�";
		SBDel->Caption = "Borrar este men�";
		SBIns->Caption = "Insertar este men�";
		CancelBtn->Caption = "Cancelar";
		LH->Caption = g_helpeng;
    }
    else {
		LH->Caption = g_help;
    }
}
//---------------------------------------------------------------------
void __fastcall TRMenuDialog::UpdateUI(void)
{
	if( SBIns->Visible ){
		SBIns->Enabled = Mmsstv->m_nRadioMenu < RADIOMENUMAX;
    }
}
//---------------------------------------------------------------------
int __fastcall TRMenuDialog::Execute(AnsiString &strTTL, AnsiString &strCMD, int fAdd)
{
	if( fAdd ){
		Caption = MsgEng ? "Agregar men�" : "���j���[�̒ǉ�";
		SBDel->Visible = FALSE;
		SBIns->Visible = FALSE;
    }
	ETTL->Text = strTTL;
    ECMD->Text = strCMD;
    UpdateUI();
    int r = ShowModal();
	strTTL = ETTL->Text;
    strCMD = ECMD->Text;
	return r;
}
//---------------------------------------------------------------------
void __fastcall TRMenuDialog::SBDelClick(TObject *Sender)
{
	ModalResult = 1024;
}
//---------------------------------------------------------------------------
void __fastcall TRMenuDialog::SBInsClick(TObject *Sender)
{
	ModalResult = 1025;
}
//---------------------------------------------------------------------------
void __fastcall TRMenuDialog::SBRefClick(TObject *Sender)
{
	TOpenDialog *pBox = new TOpenDialog(this);
	pBox->Options >> ofCreatePrompt;
	pBox->Options << ofFileMustExist;
	if( MsgEng ){
		pBox->Title = "Abrir archivo";
		pBox->Filter = "Command files(*.rcm)|*.rcm|";
	}
	else {
		pBox->Title = "�R�}���h��`�t�@�C�����J��";
		pBox->Filter = "�R�}���h��`�t�@�C��(*.rcm)|*.rcm|";
	}
	pBox->FileName = "";
	pBox->DefaultExt = "rcm";
	pBox->InitialDir = BgnDir;
	if( pBox->Execute() == TRUE ){
		ECMD->Text = pBox->FileName.c_str();
	}
    delete pBox;
}
//---------------------------------------------------------------------------


