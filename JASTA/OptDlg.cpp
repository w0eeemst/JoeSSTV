//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "OptDlg.h"
#include "Country.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TOptDlgBox *OptDlgBox;
//---------------------------------------------------------------------
__fastcall TOptDlgBox::TOptDlgBox(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall TOptDlgBox::SetLanguage(void)
{
	if( RGL->ItemIndex ){
		Caption = "Setup";
		Font->Name = "Arial";
		Font->Charset = ANSI_CHARSET;

		RGL->Caption = "Language";
		RGL->Items->Strings[0] = "Japanese";
		RGL->Items->Strings[1] = "English";
		L1->Caption = "Callsign";
		L2->Caption = "Year";
		RGC->Caption = "Entry Section";
		RGC->Items->Strings[0] = "J (Japan)";
		RGC->Items->Strings[1] = "S";
		RGF->Caption = "First time entry";
		RGF->Items->Strings[0] = "No";
		RGF->Items->Strings[1] = "Yes";
		L3->Caption = "ZIP";
		L4->Caption = "Address";
//        L5->Caption = "E-Mail";
		L6->Caption = "Name";
		L7->Caption = "License";
		L9->Caption = "Size of T-shirts as an Activity Premium";

		CancelBtn->Caption = "Cancel";
	}
	else {
		Caption = "�Z�b�g�A�b�v";
		Font->Name = "�l�r �o�S�V�b�N";
		Font->Charset = SHIFTJIS_CHARSET;

		RGL->Caption = "���� (Language)";
		RGL->Items->Strings[0] = "���{��(Japanese)";
		RGL->Items->Strings[1] = "�p��(English)";
		L1->Caption = "��ٻ��";
		L2->Caption = "�W�v�N�x";
		RGC->Caption = "�Q������";
		RGC->Items->Strings[0] = "J (���{����)";
		RGC->Items->Strings[1] = "S (�C�O)";
		RGF->Caption = "���Q��";
		RGF->Items->Strings[0] = "�ߋ��ɂ��Q������";
		RGF->Items->Strings[1] = "���Q��";
		L3->Caption = "�X�֔ԍ�";
		L4->Caption = "�Z��";
//        L5->Caption = "E-Mail";
		L6->Caption = "����";
		L7->Caption = "���i";
		L9->Caption = "��è�ޏ܂̂s�|����I���̊�]����";
		CancelBtn->Caption = "��ݾ�";
	}
}
//---------------------------------------------------------------------
int __fastcall TOptDlgBox::Execute(void)
{
	RGL->ItemIndex = MsgEng;
	SetLanguage();

	EditCall->Text = sys.m_Call;
	EditYear->Text = sys.m_Year;
	RGC->ItemIndex = sys.m_Category;
	RGF->ItemIndex = sys.m_First;
	EditZip->Text = sys.m_Zip;
	EditAdr->Text = sys.m_Adr;
	EditMail->Text = sys.m_Mail;
	EditName->Text = sys.m_Name;
	EditLicense->Text = sys.m_License;
	EditPow->Text = sys.m_Pow;
    CBSex->ItemIndex = sys.m_Sexual;
	CBTSize->ItemIndex = sys.m_TSize;

	if( ShowModal() == IDOK ){
		sys.m_Call = EditCall->Text.c_str();
		jstrupr(sys.m_Call.c_str());
		sscanf(AnsiString(EditYear->Text).c_str(), "%u", &sys.m_Year);

		sys.m_Category = RGC->ItemIndex;
		sys.m_First = RGF->ItemIndex;
		sys.m_Zip = EditZip->Text;
		sys.m_Adr = EditAdr->Text;
		sys.m_Mail = EditMail->Text;
		sys.m_Name = EditName->Text;
		sys.m_License = EditLicense->Text;
		sys.m_Pow = EditPow->Text;
	    sys.m_Sexual = CBSex->ItemIndex;
		sys.m_TSize = CBTSize->ItemIndex;

		MsgEng = RGL->ItemIndex;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
void __fastcall TOptDlgBox::RGLClick(TObject *Sender)
{
	SetLanguage();
}
//---------------------------------------------------------------------------

