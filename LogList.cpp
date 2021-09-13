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

#include "LogList.h"
#include "QsoDlg.h"
#include "TextEdit.h"
#include "LogConv.h"
#include "ConvDef.h"
#include "LogSet.h"
#include "country.h"
#include "Main.h"
#include "Hamlog5.h"
#include "TH5Len.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TLogListDlg *LogListDlg;
//---------------------------------------------------------------------
__fastcall TLogListDlg::TLogListDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( MsgEng ){
		Grid->Font->Name = ((TForm *)AOwner)->Font->Name;
		Grid->Font->Charset = ((TForm *)AOwner)->Font->Charset;
		KFile->Caption = "Archivo(&F)";
		KEdit->Caption = "Editar(&E)";
		KFind->Caption = "Buscar(&S)";
		KOpt->Caption = "Opciones(&O)";
		KHelp->Caption = "Ayuda(&H)";

		KOpen->Caption = "Abrir Archivo(&O)...";
		KFlush->Caption = "Guardar(&M)";
		KReIndex->Caption = "Hacer �ndice";
		KMTextRead->Caption = "Abrir Archivo de texto MMLOG(&R)...";
		KMTextWrite->Caption = "Guardar rango seleccionado a Archivo de texto MMLOG(&W)...";
		KImport->Caption = "Importar";
		KImportText->Caption = "Archivo de Texto...";
		KImportLog200->Caption = "Archivo Log200...";
		KImportHamlog->Caption = "Archivo Hamlog...";
		KImportADIF->Caption = "Archivo ADIF...";
		KExport->Caption = "Exportar rango seleccionado";
		KExportText->Caption = "Archivo de texto...";
		KExportADIF->Caption = "Archivo ADIF...";
		KExportLog200->Caption = "Archivo Log200...";
		KExportHamlog->Caption = "Archivo Hamlog...";
		KExportCabrillo->Caption = "Archivo Cabrillo...";
		KExit->Caption = "Volver a MMSSTV(&X)";

		KDelCur->Caption = "Cortar";
		KInsCur->Caption = "Isertar";
		KSelAll->Caption = "Seleccionar Todo";
		KDelSel->Caption = "Borrar rango seleccionado";
		KSortDate->Caption = "Ordenar(Fecha/Hora)";

		KTop->Caption = "Mover Arriba";
		KBottom->Caption = "Mover Abajo";
		KFindTop->Caption = "Buscar adelante...";
		KFindBottom->Caption = "Buscar atr�s...";
		KFindConT->Caption = "Nueva b�squeda hacia adelante";
		KFindConB->Caption = "Nueva b�squeda hacia atr�s";

		KLogOpt->Caption = "Configurar...";
	}
	if( !JanHelp.IsEmpty() ) KHlpLog->Caption = "MMSSTV Log";

	int CX = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int CY = ::GetSystemMetrics(SM_CYFULLSCREEN);
	if( (CX < Width)||(CY < Height) ){
		Top = 0;
		Left = 0;
		Width = CX;
		Height = CY;
	}
	m_DateWidth = Grid->ColWidths[0];
	FormCenter(this, CX, CY);
	if( Owner != NULL ){
		WindowState = ((TForm *)Owner)->WindowState;
	}
	SetTimeZone();
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::SetTimeZone(void)
{
	switch(Log.m_LogSet.m_TimeZone){
		case 'I':
			if( lcid == LANG_KOREAN ){
				m_TimeZone = "KST";
			}
			else {
				m_TimeZone = "JST";
			}
			break;
		default:
			m_TimeZone = "UTC";
			break;
	}
	if(Log.m_LogSet.m_DateType & 1){
		Grid->ColWidths[0] = (m_DateWidth * 10) / 8;
	}
	else {
		Grid->ColWidths[0] = m_DateWidth;
	}
}
//---------------------------------------------------------------------------
// �A�C�h������
void __fastcall TLogListDlg::OnIdle(TObject *Sender, bool &Done)
{
	UpdateMenu();
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::UpdateLogCount(int sw)
{
	Grid->RowCount = Log.GetCount() + 2;

	if( sw ){
		Grid->Row = Log.GetCount() + 1;
		AdjustTopRow();
		char bf[256];
		if( Log.IsOpen() ){
			sprintf(bf, MsgEng ? "%s - [%u QSO (s)]" : "%s - [%u���̃f�[�^������܂�]", Log.m_FileName.c_str(), Log.GetCount());
        }
        else {
			sprintf(bf, "%s no esta abierto.", Log.m_FileName.c_str());
        }
		Caption = bf;
	}
	Grid->Col = 0;
	Grid->Invalidate();
}

void __fastcall TLogListDlg::AdjustTopRow(void)
{
	int GridLine = Grid->GridHeight/Grid->RowHeights[1] - 1;
	int Top = Grid->Row - GridLine + 1;
	if( Top < 1 ) Top = 1;
	Grid->TopRow = Top;
	Grid->Invalidate();
}

void __fastcall TLogListDlg::Execute(void)
{
	if( !Log.IsOpen() ){
		Log.Open(NULL, TRUE);
	}
	Application->OnIdle = OnIdle;
	UpdateLogCount(1);
	ShowModal();
	Application->OnIdle = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::GridDrawCell(TObject *Sender, int Col,
	  int Row, TRect &Rect, TGridDrawState State)
{

	char	bf[256];
	SDMMLOG	sd;

	Grid->Canvas->FillRect(Rect);
	int X = Rect.Left + 4;
	int Y = Rect.Top + 2;

	if( Row ){
		Row--;
		bf[0] = 0;
		if( Row < Log.GetCount() ){
			Log.GetData(&sd, Row);
		}
		else {
			memset(&sd, 0, sizeof(SDMMLOG));
		}
		if( Log.m_LogSet.m_TimeZone != 'I' ){
			JSTtoUTC(&sd);
		}
		switch(Col){
			case 0:		// Date
				MultProc();
				strcpy(bf, Log.GetDateString(&sd));
				break;
			case 1:		// Time
				strcpy(bf, Log.GetTimeString(sd.btime));
				break;
			case 2:		// Call
				strcpy(bf, sd.call);
				break;
			case 3:		// M
				bf[0] = sd.cq;
				bf[1] = 0;
				break;
			case 4:		// HisRST
				strcpy(bf, sd.ur);
				break;
			case 5:		// MyRST
				strcpy(bf, sd.my);
				break;
			case 6:		// Band
				strcpy(bf, Log.GetFreqString(sd.band, sd.fq));
				break;
			case 7:		// Mode
				strcpy(bf, Log.GetModeString(sd.mode));
				break;
			case 8:		// Pow
				strcpy(bf, sd.pow);
				break;
			case 9:		// Name
				strcpy(bf, sd.name);
				break;
			case 10:	// QTH
				strcpy(bf, sd.qth);
				break;
			case 11:	// S
				bf[0] = sd.send;
				bf[1] = 0;
				break;
			case 12:	// R
				bf[0] = sd.recv;
				bf[1] = 0;
				break;
			case 13:	// REM
				strcpy(bf, sd.rem);
				break;
			case 14:	// QSL
				strcpy(bf, sd.qsl);
				break;
			case 15:	// etime;
				strcpy(bf, Log.GetTimeString(sd.etime));
				break;
			case 16:	// Env
				if( sd.env ) sprintf(bf, "%u", sd.env);
				break;
			case 17:	// Opt1
				strcpy(bf, sd.opt1);
				break;
			case 18:	// Opt2
				strcpy(bf, sd.opt2);
				break;
			case 19:	// Usr1
				strcpy(bf, Log.GetOptStr(2, &sd));
				break;
			case 20:	// Usr2
				strcpy(bf, Log.GetOptStr(3, &sd));
				break;
		}
		Grid->Canvas->TextRect(Rect, X, Y, bf);
	}
	else {		// �^�C�g��
		LPCSTR	_tt[]={
			"Fecha","JST","Indicativo","M", "Env","Rcb","Banda","Modo","Pow","Nombre","QTH","S","R",
			"Nota","QSL", "Fin", "Env", "Opt1", "Opt2", "Usr1", "Usr2",
		};
		if( Col == 1 ){
			Grid->Canvas->TextRect(Rect, X, Y, m_TimeZone.c_str());
		}
		else {
			Grid->Canvas->TextRect(Rect, X, Y, _tt[Col]);
		}
	}
}
//---------------------------------------------------------------------------
// ���^�[���L�[�̏���
void __fastcall TLogListDlg::GridKeyPress(TObject *Sender, char &Key)
{
	if( Key == 0x0d ){
		GridDblClick(NULL);
		Key = 0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::UpdateMenu(void)
{
	int f = Log.IsOpen();
	KFlush->Enabled = f && Log.IsEdit();
	KDelCur->Enabled = f && (Grid->Row <= Log.GetCount()) && (Grid->Row >= 1);
	KInsCur->Enabled = f && (Grid->Row <= Log.GetCount()) && (Grid->Row >= 1);
	KDelSel->Enabled = f && (Grid->Selection.Top < Grid->Selection.Bottom);
	KTop->Enabled = f && Log.GetCount();
	KBottom->Enabled = f && Log.GetCount();
	KFindTop->Enabled = f && Log.GetCount();
	KFindBottom->Enabled = f && Log.GetCount();
	KFindConT->Enabled = f && Log.GetCount();
	KFindConB->Enabled = f && Log.GetCount();
	KSortDate->Enabled = f && Log.GetCount() && (Grid->Selection.Top < Grid->Selection.Bottom);
	KSelAll->Enabled = f && Log.GetCount();
	KMTextRead->Enabled = f;
	KMTextWrite->Enabled = f && Log.GetCount() && (Grid->Selection.Top <= Log.GetCount());
	KExport->Enabled = f && Log.GetCount() && (Grid->Selection.Top <= Log.GetCount());
	KReIndex->Enabled = f;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KOpenClick(TObject *Sender)
{
#if 1
	Mmsstv->KLogOpenClick(NULL);
	UpdateLogCount(1);
#else
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options >> ofFileMustExist;
	if(MsgEng){
		OpenDialog->Title = "Abrir archivo";
		OpenDialog->Filter = "MMLOG Data File(*.mdt)|*.mdt|";
	}
	else {
		OpenDialog->Title = "���O�t�@�C���̃I�[�v��";
		OpenDialog->Filter = "MMLOG Data File(*.mdt)|*.mdt|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "mdt";
	OpenDialog->InitialDir = MMLogDir;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		Log.Close();
		Log.MakeName(OpenDialog->FileName.c_str());
		Log.Open(NULL, TRUE);
		UpdateLogCount(1);
	}
	TopWindow(this);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KFlushClick(TObject *Sender)
{
	Log.Close();
	Log.Open(NULL, TRUE);
	UpdateLogCount(0);
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KDelCurClick(TObject *Sender)
{
	int n = Grid->Row - 1;
	SDMMLOG sd;
	Log.GetData(&sd, n);
	memcpy(&Log.m_bak, &sd, sizeof(SDMMLOG));
	Log.Delete(n, n);
	UpdateLogCount(0);
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KInsCurClick(TObject *Sender)
{
	int n = Grid->Row - 1;
	Log.Insert(n, &Log.m_bak);
	UpdateLogCount(0);
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KDelSelClick(TObject *Sender)
{
	if( YesNoMB( MsgEng ? "Eliminar selecci�n. (No podr� restaurar) �Est� seguro?":"���ݑI������Ă���͈͂��폜���܂�.�폜�������e�͕����ł��܂���\r\n\r\n��낵���܂����H") == IDYES ){
		int top = Grid->Selection.Top - 1;
		int end = Grid->Selection.Bottom - 1;
		Log.Delete(top, end);
		UpdateLogCount(0);
		Grid->Row = top + 1;
	}
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KExitClick(TObject *Sender)
{
	ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KFindTopClick(TObject *Sender)
{
	if( m_FindCall.IsEmpty() || (Sender == KFindTop) ){
		AnsiString as = m_FindCall;
		if( InputMB(NULL, MsgEng ? "Indicativo":"��������R�[���T�C��", as) == FALSE ) return;
		jstrupr(as.c_str());
		m_FindCall = as;
	}
	int n = Log.Find(m_FindCall.c_str(), Grid->Row, 0);
	if( n >= 0 ){
		Grid->Row = n + 1;
	}
	else {
		WarningMB(MsgEng ? "'%s' no fue encontrado" : "'%s'�͌�����܂���ł���.", m_FindCall.c_str());
	}
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KFindBottomClick(TObject *Sender)
{
	if( m_FindCall.IsEmpty() || (Sender == KFindBottom) ){
		AnsiString as = m_FindCall;
		if( InputMB(NULL, MsgEng ? "Indicativo":"��������R�[���T�C��", as) == FALSE ) return;
		jstrupr(as.c_str());
		m_FindCall = as;
	}
	int n = Log.Find(m_FindCall.c_str(), Grid->Row - 2, 1);
	if( n >= 0 ){
		Grid->Row = n + 1;
	}
	else {
		WarningMB(MsgEng ? "'%s' no fue encontrado" : "'%s'�͌�����܂���ł���.", m_FindCall.c_str());
	}
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KTopClick(TObject *Sender)
{
	Grid->Row = 1;
	Grid->TopRow = 1;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KBottomClick(TObject *Sender)
{
	Grid->Row = Log.GetCount() + 1;
	AdjustTopRow();
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KSortDateClick(TObject *Sender)
{
	if( YesNoMB( MsgEng ? "Puede llevar mucho tiempo. �Est�s seguro?":"���̏����̓��`�����R���Ԃ������邩���m��܂���.\r\n\r\n��낵���܂����H" ) == IDYES ){
		int top = Grid->Selection.Top - 1;
		int end = Grid->Selection.Bottom - 1;
		if( top == end ){
			top = 0;
			end = Log.GetCount() - 1;
		}
		Log.SortDate(top, end);
		Grid->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::GridDblClick(TObject *Sender)
{
	int n = Grid->Row - 1;
	if( (n >= 0) && (n < Log.GetCount()) ){
		TQSODlgBox *pBox = new TQSODlgBox(this);
		SDMMLOG	sd;
		Log.GetData(&sd, n);
		CLogFind	Find;

		Log.FindSet(&Find, sd.call);

		pBox->Execute(&Find, &sd, n);
		Grid->Invalidate();
		delete pBox;
	}
}
//---------------------------------------------------------------------------
// �l�l�k�n�f�e�L�X�g�t�@�C���̃��[�h
void __fastcall TLogListDlg::KMTextReadClick(TObject *Sender)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options << ofFileMustExist;
	if( MsgEng ){
		OpenDialog->Title = "Abrir archivo";
		OpenDialog->Filter = "MMLOG Text Files(*.log)|*.log|";
	}
	else {
		OpenDialog->Title = "MMLOG�e�L�X�g�t�@�C���̃��[�h";
		OpenDialog->Filter = "MMLOG�e�L�X�g�t�@�C��(*.log)|*.log|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "log";
	OpenDialog->InitialDir = MMLogDir;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		LoadMmlogText(AnsiString(OpenDialog->FileName).c_str());	//JA7UDE 0428
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KMTextWriteClick(TObject *Sender)
{
	SaveDialog->Options << ofOverwritePrompt;
	if( MsgEng ){
		SaveDialog->Title = "Guardar archivo";
		SaveDialog->Filter = "MMLOG Text Files(*.log)|*.log|";
	}
	else {
		SaveDialog->Title = "MMLOG�e�L�X�g�t�@�C���̍쐬";
		SaveDialog->Filter = "MMLOG�e�L�X�g�t�@�C��(*.log)|*.log|";
	}
	char bf[256];
	strcpy(bf, Log.GetName());
	SetEXT(bf, ".log");
	SaveDialog->FileName = bf;
	SaveDialog->DefaultExt = "log";
	SaveDialog->InitialDir = MMLogDir;
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		SaveMmlogText(AnsiString(SaveDialog->FileName).c_str());	//JA7UDE 0428
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::SureRead(void)
{
	int r;
	if( MsgEng ){
		r = YesNoCancelMB("MMSSTV ya tiene datos de QSO. �Agregar datos QSO del archivo?\r\n\r\n<Yes>\tA�adido al final\r\n<No>\tSobreinscribir\r\n\t(Todos los datos actuales se eliminan y se reemplazan por los datos le�dos)\r\n<Cancel>\tAbortar");
	}
	else {
		r = YesNoCancelMB("����MMSSTV�Ƀ��O�f�[�^�����݂��܂�. �ǉ��ǂ݂��݂��܂����H\r\n\r\n<�͂�>\t���݂̃��O�f�[�^�̍Ō�ɓǂݍ��񂾃f�[�^���ǉ������.\r\n<������>\t���݂̃��O�f�[�^�͑S�č폜����ǂݍ��񂾃f�[�^�ɒu�������.\r\n<��ݾ�>\t�������~");
	}
	switch(r){
		case IDCANCEL:
			return 0;
		case IDNO:
			if(  MsgEng ){
				r = OkCancelMB("Eliminar todos los datos de QSO ... (no podr� restaurar) �Est� seguro?");
			}
			else {
				r = OkCancelMB("����MMSSTV���ێ����Ă��郍�O�f�[�^��S�Ď����܂�.\r\n\r\n�ق�܂ɂ����ł����H");
			}
			switch(r){
				case IDOK:
					Log.DeleteAll();
					break;
				case IDCANCEL:
					return 0;
			}
			break;
		default:
			break;
	}
	return 1;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::LoadMmlogText(LPCSTR pName)
{
	char bf[1024];

	if( Log.GetCount() ){
		if( !SureRead() ) return;
	}
	FILE *fp = fopen(pName, "rt");
	if( fp != NULL ){
		int r = IDNO;
		if( MsgEng ){
			r = YesNoMB("Esta funci�n existe para la compatibilidad con software antiguo que maneja JST.\r\n\r\n�Los datos grabados son UTC?");
		}
		CWaitCursor w;
		SDMMLOG	sd;
		while(!feof(fp)){
			if( fgets(bf, 1023, fp) != NULL ){
				ClipLF(bf);
				if( bf[0] ){
					if( Log.ReadAscii(&sd, bf) == TRUE ){
						if( r == IDYES ) UTCtoJST(&sd);
						Log.PutData(&sd, Log.GetCount());
					}
				}
			}
		}
		fclose(fp);
	}
	else {
		ErrorMB(MsgEng ? "No se puede abrir '%s'" : "'%s'��������܂���.", pName);
	}
	UpdateLogCount(1);
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::SaveMmlogText(LPCSTR pName)
{
	FILE *fp = fopen(pName, "wt");
	if( fp != NULL ){
		int r = IDNO;
		if( MsgEng ){
			r = YesNoMB("Esta funci�n existe para la compatibilidad con software antiguo que maneja JST.\r\n\r\n�Quieres la hora en UTC?");
		}
		SDMMLOG	sd;
		int b = Grid->Selection.Top - 1;
		int e = Grid->Selection.Bottom - 1;
		if( e >= Log.GetCount() ) e--;
		int i;
		CWaitCursor w;
		for( i = b; i <= e; i++ ){
			Log.GetData(&sd, i);
			if( r == IDYES ) JSTtoUTC(&sd);
			fprintf(fp, "\042%s\042,\042%s.%02u\042,\042%s\042,\042%s\042,\042%s\042,\042%s\042,",
				Log.GetDateString(&sd, 0),
				Log.GetTimeString(sd.btime), (sd.btime % 30 * 2),
				sd.call, sd.ur, sd.my,
				Log.GetFreqString(sd.band, sd.fq)
			);

			fprintf(fp, "\042%s\042,\042%s\042,\042%s\042,\042%s\042,\042%s\042,\042%s\042,",
				Log.GetModeString(sd.mode),
				sd.pow, sd.name, sd.qth, sd.rem, sd.qsl
			);
			fprintf(fp, "\042%s\042,\042%.1s\042,\042%.1s\042,\042%.1s\042,\042%u\042,",
				Log.GetTimeString(sd.etime),
				&sd.send, &sd.recv, &sd.cq, sd.env
			);
			fprintf(fp, "\042%s\042,\042%s\042,", sd.opt1, sd.opt2 );
			fprintf(fp, "\042%s\042,", Log.GetOptStr(2, &sd));
			fprintf(fp, "\042%s\042\n", Log.GetOptStr(3, &sd));
			if( ferror(fp) ) break;
		}
		if( fclose(fp) ){
			ErrorMB( MsgEng ? "No se puede guardar '%s'" : "'%s'���������쐬�ł��܂���ł���.", pName);
		}
	}
	else {
		ErrorMB(MsgEng ? "No se puede guardar '%s'" : "'%s'���쐬�ł��܂���.", pName);
	}
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KSelAllClick(TObject *Sender)
{
	TGridRect gRect;
	gRect.Top = 1;
	gRect.Bottom = Grid->RowCount - 1;
	gRect.Left = 0;
	gRect.Right = 20;

	Grid->Selection = gRect;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KHlpLogClick(TObject *Sender)
{
	ShowHelp(this, MsgEng ? "EMSTVLOG.TXT":"MSTVLOG.TXT");
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KReIndexClick(TObject *Sender)
{
	Log.MakeIndex();
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::LoadText(LPCSTR pName)
{
	TConvDefDlg *pBox = new TConvDefDlg(this);
	int r = pBox->Execute(LogText.m_rConv, LogText.m_Delm, LogText.m_UTC, LogText.m_Double, pName);
	delete pBox;
	if( r == FALSE ) return FALSE;

	if( Log.GetCount() ){
		if( !SureRead() ) return FALSE;
	}

	if( LogText.Open(pName) ){
		SDMMLOG	sd;
		CWaitCursor w;
		while(LogText.Read(&sd)==TRUE){
			Log.PutData(&sd, Log.GetCount());
		}
		LogText.Close();
	}
	else {
		UpdateLogCount(1);
		return FALSE;
	}
	UpdateLogCount(1);
	return TRUE;
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::SaveText(LPCSTR pName)
{
	int b = Grid->Selection.Top - 1;
	int e = Grid->Selection.Bottom - 1;
	if( e >= Log.GetCount() ) e--;

	TConvDefDlg *pBox = new TConvDefDlg(this);
	int r = pBox->Execute(LogText.m_tConv, LogText.m_Delm, LogText.m_UTC, LogText.m_Double, b, e);
	delete pBox;
	if( r == FALSE ) return FALSE;
	if( LogText.Create(pName) ){
		SDMMLOG	sd;
		int i;
		CWaitCursor w;
		for( i = b; i <= e; i++ ){
			Log.GetData(&sd, i);
			if( LogText.Write(&sd) == FALSE ) break;
		}
		LogText.Close();
	}
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KExportTextClick(TObject *Sender)
{
	SaveDialog->Options << ofOverwritePrompt;
	if( MsgEng ){
		SaveDialog->Title = "Guardar archivo";
		SaveDialog->Filter = "Text Files(*.csv;*.txt)|*.csv;*.txt|";
	}
	else {
		SaveDialog->Title = "�e�L�X�g�t�@�C���̍쐬";
		SaveDialog->Filter = "�e�L�X�g�t�@�C��(*.csv;*.txt)|*.csv;*.txt|";
	}
	char bf[256];
	strcpy(bf, Log.GetName());
	SetEXT(bf, ".csv");
	SaveDialog->FileName = bf;
	SaveDialog->DefaultExt = "csv";
	SaveDialog->InitialDir = ExtLogDir;
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		SaveText(AnsiString(SaveDialog->FileName).c_str());	//JA7UDE 0428
		SetDirName(ExtLogDir, AnsiString(SaveDialog->FileName).c_str());	//JA7UDE 0428
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KImportTextClick(TObject *Sender)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options >> ofFileMustExist;
	if( MsgEng ){
		OpenDialog->Title = "Abrir archivo";
		OpenDialog->Filter = "Text File(*.csv;*.txt)|*.csv;*.txt|";
	}
	else {
		OpenDialog->Title = "�e�L�X�g�t�@�C���̃��[�h";
		OpenDialog->Filter = "�e�L�X�g�t�@�C��(*.csv;*.txt)|*.csv;*.txt|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "csv";
	OpenDialog->InitialDir = ExtLogDir;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		LoadText(AnsiString(OpenDialog->FileName).c_str());	//ja7ude 0428
		SetDirName(ExtLogDir, AnsiString(OpenDialog->FileName).c_str());	//ja7ude 0428
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::LoadLog200(LPCSTR pName)
{
	if( Log.GetCount() ){
		if( !SureRead() ) return FALSE;
	}

	CLog200 log200;
	if( log200.Open(pName) ){
		SDMMLOG	sd;
		CWaitCursor w;
		while(log200.Read(&sd)==TRUE){
			Log.PutData(&sd, Log.GetCount());
		}
		log200.Close();
	}
	else {
		UpdateLogCount(1);
		return FALSE;
	}
	UpdateLogCount(1);
	return TRUE;
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::SaveLog200(LPCSTR pName)
{
	int b = Grid->Selection.Top - 1;
	int e = Grid->Selection.Bottom - 1;
	if( e >= Log.GetCount() ) e--;

	CLog200 log200;
	if( log200.Create(pName) ){
		SDMMLOG	sd;
		int i;
		CWaitCursor w;
		for( i = b; i <= e; i++ ){
			Log.GetData(&sd, i);
			if( log200.Write(&sd) == FALSE ) break;
		}
		log200.Close();
	}
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KExportLog200Click(TObject *Sender)
{
	SaveDialog->Options << ofOverwritePrompt;
	if( MsgEng ){
		SaveDialog->Title = "Guardar archivo";
		SaveDialog->Filter = "LOG200 Data File(*.l2)|*.l2|";
	}
	else {
		SaveDialog->Title = "LOG200�f�[�^�t�@�C���̍쐬";
		SaveDialog->Filter = "LOG200�f�[�^�t�@�C��(*.l2)|*.l2|";
	}
	char bf[256];
	strcpy(bf, Log.GetName());
	SetEXT(bf, ".l2");
	SaveDialog->FileName = bf;
	SaveDialog->DefaultExt = "l2";
	SaveDialog->InitialDir = ExtLogDir;
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		SaveLog200(AnsiString(SaveDialog->FileName).c_str());	//ja7ude 0428
		SetDirName(ExtLogDir, AnsiString(SaveDialog->FileName).c_str());	//ja7ude 0428
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KImportLog200Click(TObject *Sender)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options >> ofFileMustExist;
	if( MsgEng ){
		OpenDialog->Title = "Abrir archivo";
		OpenDialog->Filter = "LOG200 Data File(*.l2)|*.l2|";
	}
	else {
		OpenDialog->Title = "LOG200�f�[�^�t�@�C���̃��[�h";
		OpenDialog->Filter = "LOG200�f�[�^�t�@�C��(*.l2)|*.l2|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "l2";
	OpenDialog->InitialDir = ExtLogDir;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		LoadLog200(AnsiString(OpenDialog->FileName).c_str());	//ja7ude 0428
		SetDirName(ExtLogDir, AnsiString(OpenDialog->FileName).c_str());	//ja7ude 0428
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::LoadHamLog(LPCSTR pName)
{
	if( Log.GetCount() ){
		if( !SureRead() ) return FALSE;
	}

	CHamLog HamLog;
	if( HamLog.Open(pName) ){
		SDMMLOG	sd;
		CWaitCursor w;
		while(HamLog.Read(&sd)==TRUE){
			Log.PutData(&sd, Log.GetCount());
		}
		HamLog.Close();
	}
	else {
		UpdateLogCount(1);
		return FALSE;
	}
	UpdateLogCount(1);
	return TRUE;
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::LoadHamLog5(LPCSTR pName)
{
	if( Log.GetCount() ){
		if( !SureRead() ) return FALSE;
	}

	CHamlog5 HamLog;
	if( HamLog.Open(pName, TRUE) ){
		SDMMLOG	sd;
		CWaitCursor w;
        DWORD Pos;
        for( Pos = 0; Pos < HamLog.GetRCount(); Pos++ ){
			if( HamLog.Seek(Pos) ){
            	HamLog.DecodeData(&sd);
				Log.PutData(&sd, Log.GetCount());
            }
        }
		HamLog.Close();
	}
	else {
		UpdateLogCount(1);
		return FALSE;
	}
	UpdateLogCount(1);
	return TRUE;
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::SaveHamLog(LPCSTR pName)
{
	int b = Grid->Selection.Top - 1;
	int e = Grid->Selection.Bottom - 1;
	if( e >= Log.GetCount() ) e--;

	CHamLog HamLog;
	if( HamLog.Create(pName) ){
		SDMMLOG	sd;
		int i;
		CWaitCursor w;
		for( i = b; i <= e; i++ ){
			Log.GetData(&sd, i);
			if( HamLog.Write(&sd) == FALSE ) break;
		}
		HamLog.Close();
	}
	return TRUE;
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::SaveHamLog5(LPCSTR pName)
{
	int b = Grid->Selection.Top - 1;
	int e = Grid->Selection.Bottom - 1;
	if( e >= Log.GetCount() ) e--;


	char master[256];
    SetDirName(master, pName);
    strcat(master, "HAMLOG.HDB");
	CHamlog5 HamLog;
    if( HamLog.Open(master, FALSE) ){	// �t�B�[���h����ǂݍ���
    	HamLog.Close();
    }
    else {								// �t�B�[���h����₢���킹
		TTH5LenDlg *pBox = new TTH5LenDlg(this);
    	if( !pBox->Execute() ){
	        return FALSE;
		}
    }

	if( HamLog.Create(pName) ){
		SDMMLOG	sd;
		int i;
        int n = 0;
		CWaitCursor w;
		for( i = b; i <= e; i++, n++ ){
			Log.GetData(&sd, i);
			HamLog.Seek(n);
			HamLog.EncodeData(&sd);
			if( HamLog.Update() == FALSE ) break;
		}
		HamLog.Close();
	}
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KImportHamlogClick(TObject *Sender)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options >> ofFileMustExist;
	if( MsgEng ){
		OpenDialog->Title = "Abrir archivo";
		OpenDialog->Filter = "HamLog Data File(*.hdb;*.dbs)|*.hdb;*.dbs|";
	}
	else {
		OpenDialog->Title = "HamLog�f�[�^�t�@�C���̃��[�h";
		OpenDialog->Filter = "HamLog�f�[�^�t�@�C��(*.hdb;*.dbs)|*.hdb;*.dbs|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "hdb";
	OpenDialog->InitialDir = ExtLogDir;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		LPCSTR pName = AnsiString(OpenDialog->FileName).c_str();	//ja7ude 0428
		if( !strcmpi(GetEXT(pName), "HDB") ){
			LoadHamLog5(pName);
        }
        else {
			LoadHamLog(pName);
        }
		SetDirName(ExtLogDir, pName);
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KExportHamlogClick(TObject *Sender)
{
	SaveDialog->Options << ofOverwritePrompt;
	if( MsgEng ){
		SaveDialog->Title = "Guardar archivo";
		SaveDialog->Filter = "HAMLOG (Ver5) Data File(*.hdb)|*.hdb|HAMLOG Data File(*.dbs)|*.dbs|";
	}
	else {
		SaveDialog->Title = "HAMLOG�f�[�^�t�@�C���̍쐬";
		SaveDialog->Filter = "HAMLOG (Ver5) �f�[�^�t�@�C��(*.hdb)|*.hdb|HAMLOG�f�[�^�t�@�C��(*.dbs)|*.dbs|";
	}
	char bf[256];
	strcpy(bf, Log.GetName());
	SetEXT(bf, ".hdb");
	SaveDialog->FileName = bf;
	SaveDialog->DefaultExt = "hdb";
	SaveDialog->InitialDir = ExtLogDir;
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		LPCSTR pName = AnsiString(SaveDialog->FileName).c_str();	//ja7ude 0428
		if( !strcmpi(GetEXT(pName), "HDB") ){
			SaveHamLog5(pName);
        }
        else {
			SaveHamLog(pName);
        }
		SetDirName(ExtLogDir, pName);
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KLogOptClick(TObject *Sender)
{
	TLogSetDlg *pBox = new TLogSetDlg(this);
	pBox->Execute();
	delete pBox;
	SetTimeZone();
	Grid->Invalidate();
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::SaveADIF(LPCSTR pName)
{
	int b = Grid->Selection.Top - 1;
	int e = Grid->Selection.Bottom - 1;
	if( e >= Log.GetCount() ) e--;

	CLogADIF logADIF;
	if( logADIF.Create(pName) ){
		SDMMLOG	sd;
		int i;
		CWaitCursor w;
		for( i = b; i <= e; i++ ){
			Log.GetData(&sd, i);
			if( logADIF.Write(&sd) == FALSE ) break;
		}
		logADIF.Close();
	}
	return TRUE;
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::LoadADIF(LPCSTR pName)
{
	if( Log.GetCount() ){
		if( !SureRead() ) return FALSE;
	}

	CLogADIF logADIF;
	if( logADIF.Open(pName) ){
		SDMMLOG	sd;
		memset(&sd, 0, sizeof(sd));
		CWaitCursor w;
		while(logADIF.Read(&sd)==TRUE){
			Log.PutData(&sd, Log.GetCount());
			memset(&sd, 0, sizeof(sd));
		}
		logADIF.Close();
	}
	else {
		UpdateLogCount(1);
		return FALSE;
	}
	UpdateLogCount(1);
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KExportADIFClick(TObject *Sender)
{
	SaveDialog->Options << ofOverwritePrompt;
	if( MsgEng ){
		SaveDialog->Title = "Guardar archivo";
		SaveDialog->Filter = "ADIF Files(*.adi)|*.adi|";
	}
	else {
		SaveDialog->Title = "ADIF �t�@�C���̍쐬";
		SaveDialog->Filter = "ADIF �t�@�C��(*.adi)|*.adi|";
	}
	char bf[256];
	strcpy(bf, Log.GetName());
	SetEXT(bf, ".adi");
	SaveDialog->FileName = bf;
	SaveDialog->DefaultExt = "adi";
	SaveDialog->InitialDir = ExtLogDir;
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		SaveADIF(AnsiString(SaveDialog->FileName).c_str());   //ja7ude 0428
		SetDirName(ExtLogDir, AnsiString(SaveDialog->FileName).c_str());   //ja7ude 0428
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KImportADIFClick(TObject *Sender)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options >> ofFileMustExist;
	if( MsgEng ){
		OpenDialog->Title = "Abrir archivo";
		OpenDialog->Filter = "ADIF Files(*.adi;*.txt)|*.adi;*.txt|";
	}
	else {
		OpenDialog->Title = "ADIF �t�@�C���̃��[�h";
		OpenDialog->Filter = "ADIF �t�@�C��(*.adi;*.txt)|*.adi;*.txt|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "adi";
	OpenDialog->InitialDir = ExtLogDir;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		LoadADIF(AnsiString(OpenDialog->FileName).c_str());
		SetDirName(ExtLogDir, AnsiString(OpenDialog->FileName).c_str());
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
int __fastcall TLogListDlg::SaveCabrillo(LPCSTR pName)
{
	int b = Grid->Selection.Top - 1;
	int e = Grid->Selection.Bottom - 1;
	if( e >= Log.GetCount() ) e--;

	CLogCabrillo logCabrillo;
	if( logCabrillo.Create(pName) ){
		SDMMLOG	sd;
		int i;
		CWaitCursor w;
		for( i = b; i <= e; i++ ){
			Log.GetData(&sd, i);
			if( logCabrillo.Write(&sd) == FALSE ) break;
		}
		logCabrillo.Close();
	}
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TLogListDlg::KExportCabrilloClick(TObject *Sender)
{
	SaveDialog->Options << ofOverwritePrompt;
	if( MsgEng ){
		SaveDialog->Title = "Guardar archivo";
		SaveDialog->Filter = "Cabrillo Files(*.txt)|*.txt|";
	}
	else {
		SaveDialog->Title = "Cabrillo �t�@�C���̍쐬";
		SaveDialog->Filter = "Cabrillo �t�@�C��(*.txt)|*.txt|";
	}
	char bf[256];
	strcpy(bf, Log.GetName());
	SetEXT(bf, ".txt");
	SaveDialog->FileName = bf;
	SaveDialog->DefaultExt = "txt";
	SaveDialog->InitialDir = ExtLogDir;
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		SaveCabrillo(AnsiString(SaveDialog->FileName).c_str());             //ja7ude 0428
		SetDirName(ExtLogDir, AnsiString(SaveDialog->FileName).c_str());	//ja7ude 0428
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------


