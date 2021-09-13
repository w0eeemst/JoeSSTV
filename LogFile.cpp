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
#include <vcl.h>	//ja7ude 0521
#pragma hdrstop

#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include "LogFile.h"
#include "Main.h"
#include "Hamlog5.h"
CLogFile	Log;


//---------------------------------------------------------------------------
// MMLOG Constant
//
enum BD {			/* �o���h�̔ԍ�						*/
	B_NULL,
	B_19,
	B_35,
	B_38,
	B_7,
	B_10,
	B_14,
	B_18,
	B_21,
	B_24,
	B_28,
	B_50,
	B_144,
	B_430,
	B_1200,
	B_2400,
	B_5600,
	B_101G,
	B_104G,
	B_24G,
	B_47G,
	B_75G,
	B_142G,
	B_248G,
	B_4630,
	B_220,
	B_SAT,
};

const char	*_mBand[]={
	"","160","80","75","40","30","20","17","15","12","10","6",
	"2","430","1200","2400","5600","10.1G","10.4G","24G","47G",
	"75G","142G","248G","4630","220",NULL,
};
const char	*_band[]={
	"","1.9","3.5","3.8","7","10","14","18","21","24","28","50",
	"144","430","1200","2400","5600","10.1G","10.4G","24G","47G",
	"75G","142G","248G","4630","220",NULL,
};
static const char	*_mode[]={
	"",
	"CW","SSB","AM","FM","RTTY","PAC","FAX","SSTV","ATV","TV","FSTV",
	"A1","A2","A3","A3A","A3H","A3J","A4","A5","A5C","A5J",
	"A9","A9C","F1","F2","F3","F4","F5","F9","P0","P1",
	"P2D","P2E","P2F","P3D","P3E","P3F","P9",
	"U1","U2","U3","U4",
	"PSK","BPSK","QPSK","HELL","MFSK",
	NULL,
};
const char	lmode[]={
	3, 3, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 2, 2, 2, 2, 3, 3, 3, 3,
	3, 3, 3, 3, 2, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3,
	3, 3, 3, 3, 3,
};
const	char	MONN[]={31,31,28,31,30,31,30,31,31,30,31,30,31};
const	char	MONU[]={31,31,29,31,30,31,30,31,31,30,31,30,31};
//---------------------------------------------------------------------------
// CIndex�N���X
int GetLMode(BYTE m)
{
	if( m < MODEMAX ){
		return lmode[m];
	}
	else {
		return 3;
	}
}
//---------------------------------------------------------------------------
// CIndex�N���X
CIndex::CIndex()
{
	m_IndexMax = 0;		// �m�ے��̃C���f�b�N�X�̐�
	m_IndexCnt = 0;		// ���݂̃C���f�b�N�X�̐�
	pIndex = NULL;		// �C���f�b�N�X�̈�̃|�C���^(16�o�C�g�C���f�b�N�X)
	pMult = NULL;
}
CIndex::~CIndex()
{
	if( pIndex != NULL ){
		delete pIndex;
		pIndex = NULL;
	}
	if( pMult != NULL ){
		delete pMult;
		pMult = NULL;
	}
}
void CIndex::AllocIndex(int n)
{
	if( n >= m_IndexMax ){
		int max = m_IndexMax ? (m_IndexMax * 2) : 32768;
		LPSTR np = new char[16*max];
		if( pIndex != NULL ){
			memcpy(np, pIndex, m_IndexCnt*16);
			delete pIndex;
		}
		pIndex = np;
		m_IndexMax = max;
	}
}

void CIndex::ClearIndex(void)
{
	m_IndexCnt = 0;
}

void CIndex::WriteIndex(LPCSTR pCall, int n)
{
	AllocIndex(n);
	memcpy(&pIndex[n*16], pCall, 16);
	if( n >= m_IndexCnt ) m_IndexCnt = n + 1;
}

void CIndex::ReadIndex(LPSTR pCall, int n)
{
	memcpy(pCall, &pIndex[n*16], 16);
	pCall[16] = 0;
}

LPSTR CIndex::PointIndex(int n)
{
	return &pIndex[n*16];
}

void CIndex::AddBlock(LPCSTR p, int len)
{
	int n = len/16;
	AllocIndex(n + m_IndexCnt);
	memcpy(&pIndex[m_IndexCnt*16], p, len);
	m_IndexCnt += n;
}
//---------------------------------------------------------------------------
int CIndex::ReadIndex(int handle, FHD *hp)
{
	int		r = TRUE;
	int		BUFSIZE=32768;

	CWaitCursor w;
	if( lseek(handle, (hp->size * (ULONG)sizeof(SDMMLOG)) + FHDOFF, SEEK_SET) != -1L ){
		if( hp->hash != 2 ){			// 16�o�C�g�C���f�b�N�X�ƈقȂ�
			if( hp->hash ){					// 2�o�C�g�C���f�b�N�X�̃X�L�b�v
				lseek(handle, hp->size * 2, SEEK_CUR);
			}
			r = FALSE;
		}
		else {							// 16�o�C�g�C���f�b�N�X
			LPSTR bp = new char[BUFSIZE];
			int len = hp->size * 16;
			int rlen, wlen;
			while(len){
				rlen = (len > BUFSIZE) ? BUFSIZE : len;
				wlen = read(handle, bp, rlen);
				if( rlen != wlen ){
					r = FALSE;
					break;
				}
				AddBlock(bp, wlen);
				len -= wlen;
			}
			delete bp;
		}
		if( pMult == NULL ) pMult = new BYTE[32768];
		read(handle, pMult, hp->mlt);
	}
	else {
		r = FALSE;
	}
	return r;
}

void CIndex::MakeIndex(int handle, FHD *hp)
{
	CWaitCursor w;

	lseek(handle, (ULONG)FHDOFF, SEEK_SET);
	ClearIndex();

	FHD	fh;
	memset(&fh, 0, sizeof(fh));

	int i, rlen;
	SDMMLOG	sd;
	for( i = 0; i < hp->size; i++ ){
		rlen = read(handle, &sd, sizeof(sd));
		if( rlen != (int)sizeof(sd) ) break;	//ja7ude 0525
		WriteIndex(sd.call, i);
		if( sd.mode >= MODEMAX ){
			int m = sd.mode - MODEMAX;
			int f = 0;
			int n;
			char sbf[7];
			StrCopy(sbf, hp->mode[m], 6);
			for( n = 0; (n < UMODEMAX) && fh.mode[n][0] ; n++ ){
				char tbf[7];
				StrCopy(tbf, fh.mode[n], 6);
				if( !strcmp(sbf, tbf) ){
					if( m != n ){
						f++;
						sd.mode = BYTE(n + MODEMAX);
						break;
					}
				}
			}
			if( !fh.mode[n][0] ){
				StrCopy(fh.mode[n], sbf, 6);
				if( m != n ){
					sd.mode = BYTE(n + MODEMAX);
					f++;
				}
			}
			if( f ){
				lseek( handle, -long(sizeof(sd)), SEEK_CUR);
				write(handle, &sd, sizeof(sd));
			}
		}
	}
	memcpy(hp->mode, fh.mode, sizeof(fh.mode));
	lseek( handle, (hp->size * (ULONG)sizeof(SDMMLOG)) + FHDOFF, SEEK_SET);
	hp->hash = 2;	// 16�o�C�g�C���f�b�N�X
}

//---------------------------------------------------------------------------
int CIndex::WriteIndex(int handle, FHD *hp)
{
	int		r = TRUE;
	int		BUFSIZE=32768;

	CWaitCursor w;

	if( lseek(handle, (hp->size * (ULONG)sizeof(SDMMLOG)) + FHDOFF, SEEK_SET) != -1L ){
		hp->hash = 2;
		int len = hp->size * 16;
		int vlen, wlen;
		LPSTR cp = pIndex;
		while(len){
			wlen = (len > BUFSIZE) ? BUFSIZE : len;
			vlen = write(handle, cp, wlen);
			if( wlen != vlen ) r = FALSE;
			len -= vlen;
			cp += vlen;
		}
		if( pMult != NULL ){
			write(handle, pMult, hp->mlt);
		}
	}
	else {
		r = FALSE;
	}
	return r;
}

//---------------------------------------------------------------------------
// CLogFind�N���X
CLogFind::CLogFind()
{
	m_FindCnt = 0;
	pFindTbl = new int[FINDMAX];
}
//---------------------------------------------------------------------------
CLogFind::~CLogFind()
{
	delete pFindTbl;
	pFindTbl = NULL;
}
//---------------------------------------------------------------------------
int CLogFind::Add(int n)
{
	if( m_FindCnt >= FINDMAX ) return 0;

	for( int i = 0; i < m_FindCnt; i++ ){
		if( pFindTbl[i] == n ) return 0;
	}
	pFindTbl[m_FindCnt] = n;
	m_FindCnt++;
	return 1;
}
//---------------------------------------------------------------------------
void CLogFind::Ins(int n)
{
	if( m_FindCnt >= FINDMAX ) return;

	int i;
	for( i = 0; i < m_FindCnt; i++ ){
		if( pFindTbl[i] == n ) return;
	}
	for( i = m_FindCnt - 1; i > 0; i-- ){
		pFindTbl[i] = pFindTbl[i-1];
	}
	pFindTbl[0] = n;
	m_FindCnt++;
	m_FindCmp1Max++;
	m_FindCmp2Max++;
	m_FindStr1Max++;
	m_FindStr2Max++;
}
//---------------------------------------------------------------------------
// CLogFile�N���X
CLogFile::CLogFile()
{
	memset(&m_sd, 0, sizeof(SDMMLOG));
	memset(&m_bak, 0, sizeof(SDMMLOG));

	m_Open = 0;							// �I�[�v���t���O
	m_EditFlag = 0;
	m_Handle = 0;
	memset(&m_fhd, 0, sizeof(m_fhd));	// ���݃I�[�v�����̃t�@�C���w�b�_
	MakePathName("TEMP.MDT");
	m_CurNo = 0;

	// LogSet �̃f�t�H���g

	m_LogSet.m_TimeZone = 'I';			// JA

	m_LogSet.m_UpperName = 0;			// �啶���ւ̕ϊ�
	m_LogSet.m_UpperQTH = 0;			// �啶���ւ̕ϊ�
	m_LogSet.m_UpperREM = 0;			// �啶���ւ̕ϊ�
	m_LogSet.m_UpperQSL = 0;			// �啶���ւ̕ϊ�

	m_LogSet.m_DefMyRST = 0;			// 0-OFF, 1-ON

	m_LogSet.m_CopyFreq = 1;			// 0-Band, 1-Freq
	m_LogSet.m_CopyHis = 0;				// 0-599, 1-599001, 2-599UTC
	m_LogSet.m_CopyName = 1;			// 0-OFF, 1-ON
	m_LogSet.m_CopyQTH = 1;				// 0-OFF, 1-ON
	m_LogSet.m_CopyREM = 0;				// 0-OFF, 1-ON
	m_LogSet.m_CopyQSL = 0;				// 0-OFF, 1-ON
	m_LogSet.m_CopyREMB4 = 0;			// 0-OFF, 1-ON

	m_LogSet.m_AutoSave = 1;
	m_LogSet.m_CheckBand = 1;

	m_LogSet.m_THRTTY = "RTY";
	m_LogSet.m_THSSTV = "STV";
	m_LogSet.m_THTZ = 0;
	m_LogSet.m_ClipRSTADIF = 1;
	m_LogSet.m_DateType = 0;

    m_LogSet.m_FSKNR = 1;
    m_LogSet.m_Backup = 1;
	memcpy(m_LogSet.m_Hamlog5Len, g_Hamlog5Len, 17);
}

CLogFile::~CLogFile()
{
	Close();
}

void CLogFile::ReadIniFile(LPCSTR pKey, TMemIniFile *pIniFile)
{
	m_LogSet.m_DateType = pIniFile->ReadInteger(pKey, "DateType", m_LogSet.m_DateType);
	m_LogSet.m_TimeZone = (char)pIniFile->ReadInteger(pKey, "TimeZone", m_LogSet.m_TimeZone);

	m_LogSet.m_UpperName = pIniFile->ReadInteger(pKey, "UpperName", m_LogSet.m_UpperName);
	m_LogSet.m_UpperQTH = pIniFile->ReadInteger(pKey, "UpperQTH", m_LogSet.m_UpperQTH);
	m_LogSet.m_UpperREM = pIniFile->ReadInteger(pKey, "UpperREM", m_LogSet.m_UpperREM);
	m_LogSet.m_UpperQSL = pIniFile->ReadInteger(pKey, "UpperQSL", m_LogSet.m_UpperQSL);

	m_LogSet.m_DefMyRST = pIniFile->ReadInteger(pKey, "DefMyRST", m_LogSet.m_DefMyRST);

	m_LogSet.m_CopyFreq = pIniFile->ReadInteger(pKey, "CopyFreq", m_LogSet.m_CopyFreq);
	m_LogSet.m_CopyHis = pIniFile->ReadInteger(pKey, "CopyHis", m_LogSet.m_CopyHis);
	m_LogSet.m_CopyName = pIniFile->ReadInteger(pKey, "CopyName", m_LogSet.m_CopyName);
	m_LogSet.m_CopyQTH = pIniFile->ReadInteger(pKey, "CopyQTH", m_LogSet.m_CopyQTH);
	m_LogSet.m_CopyREM = pIniFile->ReadInteger(pKey, "CopyREM", m_LogSet.m_CopyREM);
	m_LogSet.m_CopyQSL = pIniFile->ReadInteger(pKey, "CopyQSL", m_LogSet.m_CopyQSL);
	m_LogSet.m_CopyREMB4 = pIniFile->ReadInteger(pKey, "CopyREMB4", m_LogSet.m_CopyREMB4);

	m_LogSet.m_CheckBand = pIniFile->ReadInteger(pKey, "CheckBand", m_LogSet.m_CheckBand);

	m_LogSet.m_AutoSave = pIniFile->ReadInteger(pKey, "AutoSave", m_LogSet.m_AutoSave);
	m_LogSet.m_THRTTY = pIniFile->ReadString(pKey, "THRTTY", m_LogSet.m_THRTTY);
	m_LogSet.m_THSSTV = pIniFile->ReadString(pKey, "THSSTV", m_LogSet.m_THSSTV);
	m_LogSet.m_THTZ = pIniFile->ReadInteger(pKey, "THTZ", m_LogSet.m_THTZ);
	m_LogSet.m_ClipRSTADIF = pIniFile->ReadInteger(pKey, "ClipRSTADIF", m_LogSet.m_ClipRSTADIF);

	m_LogSet.m_FSKNR = pIniFile->ReadInteger(pKey, "FSKNR", m_LogSet.m_FSKNR);
	m_LogSet.m_Backup = pIniFile->ReadInteger(pKey, "Backup", m_LogSet.m_Backup);
	AnsiString ss;
    GetHamlog5FieldsLen(ss);
    AnsiString as = pIniFile->ReadString(pKey, "TH5Fields", ss);
    SetHamlog5FieldsLen(as);
}

void CLogFile::WriteIniFile(LPCSTR pKey, TMemIniFile *pIniFile)
{
	pIniFile->WriteInteger(pKey, "TimeZone", m_LogSet.m_TimeZone);
	pIniFile->WriteInteger(pKey, "DateType", m_LogSet.m_DateType);

	pIniFile->WriteInteger(pKey, "UpperName", m_LogSet.m_UpperName);
	pIniFile->WriteInteger(pKey, "UpperQTH", m_LogSet.m_UpperQTH);
	pIniFile->WriteInteger(pKey, "UpperREM", m_LogSet.m_UpperREM);
	pIniFile->WriteInteger(pKey, "UpperQSL", m_LogSet.m_UpperQSL);

	pIniFile->WriteInteger(pKey, "DefMyRST", m_LogSet.m_DefMyRST);

	pIniFile->WriteInteger(pKey, "CopyFreq", m_LogSet.m_CopyFreq);
	pIniFile->WriteInteger(pKey, "CopyHis", m_LogSet.m_CopyHis);
	pIniFile->WriteInteger(pKey, "CopyName", m_LogSet.m_CopyName);
	pIniFile->WriteInteger(pKey, "CopyQTH", m_LogSet.m_CopyQTH);
	pIniFile->WriteInteger(pKey, "CopyREM", m_LogSet.m_CopyREM);
	pIniFile->WriteInteger(pKey, "CopyQSL", m_LogSet.m_CopyQSL);
	pIniFile->WriteInteger(pKey, "CopyREMB4", m_LogSet.m_CopyREMB4);

	pIniFile->WriteInteger(pKey, "CheckBand", m_LogSet.m_CheckBand);

	pIniFile->WriteInteger(pKey, "AutoSave", m_LogSet.m_AutoSave);
	pIniFile->WriteString(pKey, "THRTTY", m_LogSet.m_THRTTY);
	pIniFile->WriteString(pKey, "THSSTV", m_LogSet.m_THSSTV);
	pIniFile->WriteInteger(pKey, "THTZ", m_LogSet.m_THTZ);
	pIniFile->WriteInteger(pKey, "ClipRSTADIF", m_LogSet.m_ClipRSTADIF);

	pIniFile->WriteInteger(pKey, "FSKNR", m_LogSet.m_FSKNR);
	pIniFile->WriteInteger(pKey, "Backup", m_LogSet.m_Backup);
	AnsiString ss;
    GetHamlog5FieldsLen(ss);
    pIniFile->WriteString(pKey, "TH5Fields", ss);
}

void CLogFile::MakeIndex(void)
{
	if( !m_Open ) return;

	m_Index.MakeIndex(m_Handle, &m_fhd);
	m_EditFlag = 1;
}

void CLogFile::MakePathName(LPCSTR pName)
{
	char bf[256];

	m_Name = pName;
	sprintf(bf, "%s%s", MMLogDir, pName);
	SetEXT(bf, ".MDT");
	m_FileName = bf;
}

void CLogFile::MakeName(LPCSTR pPathName)
{
	if( pPathName != m_FileName.c_str() ) m_FileName = pPathName;

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char name[_MAX_FNAME];
	char ext[_MAX_EXT];
	AnsiString	Dir;

	::_splitpath( pPathName, drive, dir, name, ext );
	Dir = drive;
	Dir += dir;
	m_Name = name;
	m_Name += ext;
	if( Dir.IsEmpty() ){
		if( !MMLogDir[0] ) strcpy(MMLogDir, BgnDir);
		MakePathName(m_Name.c_str());
    }
    else {
		strncpy(MMLogDir, Dir.c_str(), 128);
    }
}

void CLogFile::DoBackup(void)
{
	if( !Log.m_LogSet.m_Backup ) return;
	if( m_FileName.IsEmpty() ) return;

	FILE *sfp = fopen(m_FileName.c_str(), "rb");
	if( sfp == NULL ) return;

	char tname[256];
    strcpy(tname, m_FileName.c_str());
	SetEXT(tname, "_BAK.MDT");
    FILE *tfp = fopen(tname, "rb");
    if( tfp != NULL ){
        long slen = filelength(fileno(sfp));
		long tlen = filelength(fileno(tfp));
        fclose(tfp);
        if( slen <= tlen ){
			fclose(sfp);
            return;
        }
    }
    fclose(sfp);
    ::CopyFile(m_FileName.c_str(), tname, FALSE);
}

int CLogFile::Close(void)
{
	int r = TRUE;
	if( m_Open ){
		if( m_EditFlag ){
			if( m_fhd.size ){
				SDMMLOG	sd;

				GetData(&sd, m_fhd.size - 1);
				if( !sd.date || !sd.btime ){
					DeleteLast();
				}
			}
			ULONG size = (m_fhd.size * (ULONG)sizeof(SDMMLOG))+(ULONG)FHDOFF;
			if( chsize(m_Handle, size) ){
				ErrorFWrite(m_FileName.c_str());
				r = FALSE;
				lseek(m_Handle, size, SEEK_SET);
            }
			m_Index.WriteIndex(m_Handle, &m_fhd);	// �C���f�b�N�X�̊i�[
			lseek(m_Handle, 0L, SEEK_SET);
			if( write( m_Handle, &m_fhd, (int)sizeof(m_fhd) ) != (int)sizeof(m_fhd) ){
				ErrorFWrite(m_FileName.c_str());
				r = FALSE;
			}
		}
		if( close(m_Handle) ){
			if( r != FALSE ){
				ErrorFWrite(m_FileName.c_str());
				r = FALSE;
			}
		}
//		if( !m_fhd.size ) unlink(m_FileName.c_str());	// 0���̎��͏���
		m_Open = 0;
	}
	m_EditFlag = 0;
	m_Index.ClearIndex();
	return r;
}

void CLogFile::InitHeader(void)
{
	memset(&m_fhd, 0, sizeof(m_fhd));
	strcpy(m_fhd.id, MMLOGID);
	m_fhd.size = 0L;
	m_fhd.mlt = 0;
}

int CLogFile::Open(LPCSTR pName, BOOL fNew)
{
	CWaitCursor w;
	Close();
	InitHeader();

	if( pName == NULL ) pName = m_FileName.c_str();
	int omode = O_RDWR|O_BINARY;		// ����͐V�K�쐬�Ȃ��ŃI�[�v��
	int n = 0;
	while(1){
		if( (m_Handle = open(pName, omode, S_IREAD|S_IWRITE)) > 0 ){
			if( read(m_Handle, &m_fhd, (int)sizeof(m_fhd)) == (int)sizeof(m_fhd) ){	//ja7ude 0525
				if( !strcmp(m_fhd.id, MMLOGID) ){
					if( m_Index.ReadIndex(m_Handle, &m_fhd) != TRUE ){
						m_Index.MakeIndex(m_Handle, &m_fhd);
					}
					m_Open = 1;
					MakeName(pName);
					m_Find.Clear();
					SetLastPos();
					memcpy(&m_asd, &m_sd, sizeof(m_asd));
					return TRUE;
                }
				else {
					close(m_Handle);
					ErrorMB("'%s' is not a correct format.", pName);
				}
			}
			else if( omode & O_CREAT ){		// �V�K�t�@�C���@2��ڂ̃p�X�̂�
				InitHeader();
				if( write(m_Handle, &m_fhd, (int)sizeof(m_fhd)) != (int)sizeof(m_fhd) ){	//ja7ude 0525
					close(m_Handle);
					ErrorMB("Cannot open '%s'.", pName);
				}
				else {
					m_Open = 1;
					MakeName(pName);
					m_Index.ClearIndex();
					m_Find.Clear();
					SetLastPos();
					memcpy(&m_asd, &m_sd, sizeof(m_asd));
					return TRUE;
				}
            }
			else {       // �����̃t�B�A��
				close(m_Handle);
				ErrorMB("Cannot open '%s'.", pName);
			}
        	return FALSE;
        }
		// �I�[�v���G���[�̏ꍇ
		if( !(omode & O_CREAT) && (errno == ENOENT) ){	// �P��ڂŃt�@�C�������݂��Ȃ��ꍇ
			n++;
            if( n >= 2 ){
			    if( fNew ){
					if( YesNoMB(MsgEng ? "El archivo para guardar el log (%s) no existe, desea crearlo ?" : "���O�t�@�C��(%s)��������܂���. �V�K�ɍ쐬���܂��� ?", pName) != IDYES ) return FALSE;
   				}
        		omode |= O_CREAT;		// 2��ڂ͐V�K�쐬�t���ŃI�[�v��
            }
            else {
				::Sleep(1000);
            }
        }
		else if( IsFile(pName) && (GetFileAttributes(pName) & FILE_ATTRIBUTE_READONLY) ){
			ErrorMB("'%s' is read-only.", pName);
            return FALSE;
		}
		else {
			ErrorMB("Cannot open '%s'.", pName);
            return FALSE;
		}
	}
}

void CLogFile::InitCur(void)
{
	SetLastPos();
}

void CLogFile::SetLastPos(void)
{
	m_CurNo = m_fhd.size;
	memset(&m_sd, 0, sizeof(SDMMLOG));

	int n = m_CurNo - 1;
	if( n >= 0 ){
		SDMMLOG	sd;

		GetData(&sd, n);
		if( !sd.etime ){
			m_CurNo = n;
			memcpy(&m_sd, &sd, sizeof(SDMMLOG));
		}
	}
	else {
#if 0
		if( lcid != LANG_JAPANESE ){
			m_sd.band = B_14;
			m_sd.fq = 230;
		}
		else {
			m_sd.band = B_7;
			m_sd.fq = 33;
		}
#else
		m_sd.band = B_14;
		m_sd.fq = 230;
#endif
		m_sd.mode = 8;
		m_sd.env = 1;
	}
	CopyAF();
	m_CurChg = 0;
}

int CLogFile::GetData(SDMMLOG *sp, int n)
{
	if( !m_Open || (n >= m_fhd.size) ){
		memset(sp, 0, sizeof(SDMMLOG));
		if( !n ){
			m_sd.band = B_14;
			m_sd.mode = 5;
		}
		return FALSE;
	}
	if( lseek(m_Handle, (ULONG)FHDOFF + ((ULONG)n * (ULONG)sizeof(SDMMLOG)), SEEK_SET) == -1L ){
		memset(sp, 0, sizeof(SDMMLOG));
		return FALSE;
	}
	if( read( m_Handle, sp, (int)sizeof(SDMMLOG) ) != (int)sizeof(SDMMLOG) ){	//ja7ude 0525
		memset(sp, 0, sizeof(SDMMLOG));
		return FALSE;
	}
	else {
		return TRUE;
	}
}

int CLogFile::PutData(SDMMLOG *sp, int n)
{
	if( !m_Open ){
		if( m_FileName.IsEmpty() ){
			ErrorMB("���O�t�@�C���̖��O����`����Ă��܂���.");
			return FALSE;
		}
		if( Open(m_FileName.c_str(), TRUE) != TRUE ) return FALSE;
	}
	if( lseek(m_Handle, (ULONG)FHDOFF + ((ULONG)n * (ULONG)sizeof(SDMMLOG)), SEEK_SET) == -1L ){
		ErrorFWrite(m_FileName.c_str());
		return FALSE;
	}
	if( write( m_Handle, sp, (int)sizeof(SDMMLOG) ) != (int)sizeof(SDMMLOG) ){	//ja7ude 0525
		ErrorFWrite(m_FileName.c_str());
		return FALSE;
	}
	else {
		m_EditFlag = 1;
		if( m_CurNo == n ) m_CurChg = 1;
		if( n >= m_fhd.size ) m_fhd.size = n + 1;
		m_Index.WriteIndex(sp->call, n);
	}
	return TRUE;
}

LPCSTR CLogFile::GetOptStr(int n, SDMMLOG *sp)
{
	switch(n){
		case 0:
			return sp->opt1;
		case 1:
			return sp->opt2;
		case 2:
			if( !sp->ur[MLRST-MLOPT-1] && sp->ur[MLRST-MLOPT] ){
				return &sp->ur[MLRST-MLOPT];
			}
			else {
				return "";
			}
		case 3:
			if( !sp->my[MLRST-MLOPT-1] && sp->my[MLRST-MLOPT] ){
				return &sp->my[MLRST-MLOPT];
			}
			else {
				return "";
			}
		default:
			return "";
	}
}

void CLogFile::SetOptStr(int n, SDMMLOG *sp, LPCSTR pOpt)
{
	switch(n){
		case 0:
			StrCopy(sp->opt1, pOpt, MLOPT);
			jstrupr(sp->opt1);
			break;
		case 1:
			StrCopy(sp->opt2, pOpt, MLOPT);
			jstrupr(sp->opt2);
			break;
		case 2:
			if( *pOpt ){
				StrCopy(&sp->ur[MLRST-MLOPT], pOpt, MLOPT);
				jstrupr(&sp->ur[MLRST-MLOPT]);
				sp->ur[MLRST-MLOPT-1] = 0;
			}
			else if( !sp->ur[MLRST-MLOPT-1] ){
				sp->ur[MLRST-MLOPT] = 0;
			}
			break;
		case 3:
			if( *pOpt ){
				StrCopy(&sp->my[MLRST-MLOPT], pOpt, MLOPT);
				jstrupr(&sp->my[MLRST-MLOPT]);
				sp->my[MLRST-MLOPT-1] = 0;
			}
			else if( !sp->my[MLRST-MLOPT-1] ){
				sp->my[MLRST-MLOPT] = 0;
			}
			break;
		default:
			break;
	}
}

void CLogFile::SetHisRST(SDMMLOG *sp)
{
	int n, tim;

	switch(m_LogSet.m_CopyHis){
		case 0:
			if( sp == NULL ){
				strcpy(m_sd.ur, "595");
			}
			else {
				strcpy(m_sd.ur, sp->ur);
				memcpy(m_sd.ur, "595", 3);
			}
			break;
		case 1:
			if( sp == NULL ){
				strcpy(m_sd.ur, "595001");
			}
			else {
				if( sp->ur[0] ){
					n = atoin(&sp->ur[3], -1);
				}
				else {
					n = 0;
				}
				n++;
				if( n >= 10000 ){
					sprintf(m_sd.ur, "595%04u", n);
				}
				else {
					sprintf(m_sd.ur, "595%03u", n);
				}
			}
			break;
		case 2:
			if( m_sd.btime ){
				tim = m_sd.btime / 30;
				if( tim >= (9 * 60) ){
					tim -= 9 * 60;
				}
				else {
					tim += 15 * 60;
				}
				sprintf(m_sd.ur, "595%02u%02u", tim/60, tim%60);
			}
			break;
	}
}

void CLogFile::CopyAF(void)
{
	int n = m_CurNo - 1;
	if( n >= 0 ){
		SDMMLOG sd;
		GetData(&sd, n);
		if( !m_sd.band ){
			m_sd.band = sd.band;
			if( m_LogSet.m_CopyFreq ){
				m_sd.fq = sd.fq;
			}
			else {
				m_sd.fq = 0;
			}
		}
		if( !m_sd.env ){
			m_sd.env = sd.env;
		}
		if( !m_sd.pow[0] ){
			strcpy(m_sd.pow, sd.pow);
		}
		if( !m_sd.ur[0] ){
			SetHisRST(&sd);
		}
		if( m_LogSet.m_CopyREMB4 ){
			strcpy(m_sd.rem, sd.rem);
		}
	}
	else {
		if( !m_sd.band ){
			m_sd.band = B_14;
			m_sd.fq = 0;
		}
		if( !m_sd.env ){
			m_sd.env = 1;
		}
		if( !m_sd.ur[0] ){
			SetHisRST(NULL);
		}
	}
	if( !m_sd.mode ) m_sd.mode = 8;	// SSTV
	if( m_LogSet.m_DefMyRST ){
		if( !m_sd.my[0] ){
			strcpy(m_sd.my, "595");
		}
	}
}

static int _cmpdate(SDMMLOG *s, SDMMLOG *t)
{
	int r = YEAR(s->year) - YEAR(t->year);
	if( r ) return r;
	r = s->date - t->date;
	if( r ) return r;
	return s->btime - t->btime;
}

void CLogFile::SortDate(int bb, int eb)
{
	int		gap, i, j, em;
	SDMMLOG	sd1, sd2;

	CWaitCursor w;
	if( bb != eb ){
		em = eb - bb;
		for( gap = (em + 1)/2; gap > 0; gap /= 2 ){
			for( i = gap; i <= em; i++ ){
				for( j = i - gap; j >= 0; j -= gap ){
					GetData(&sd1, bb+j);
					GetData(&sd2, bb+j+gap);
					if( _cmpdate(&sd1, &sd2) <= 0 ) break;
					PutData(&sd1, bb+j+gap);
					PutData(&sd2, bb+j);
				}
			}
		}
	}
	m_EditFlag = 1;
}

int CLogFile::FindSameBand(void)
{
	if( !m_sd.call[0] ) return 0;
	char	call[MLCALL+1];
	SDMMLOG	sd;

	MultProc();
	int i;
	for( i = 0; i < m_Find.GetCount(); i++ ){
		m_Index.ReadIndex(call, m_Find.pFindTbl[i]);
		if( !strcmp(call, m_sd.call) ){
			GetData(&sd, m_Find.pFindTbl[i]);
			MultProc();
			if( !sd.etime && (i == (m_Find.GetCount()-1)) ) break; 
			if( (sd.band == m_sd.band)&&(sd.mode == m_sd.mode) ) return 1;
		}
	}
	return 0;
}

int CLogFile::FindSameDate(void)
{
	if( !m_sd.call[0] ) return 0;
	char	call[MLCALL+1];
	SDMMLOG	sd;

	SYSTEMTIME utc;
	GetUTC(&utc);
	MultProc();
	int i;
	for( i = 0; i < m_Find.GetCount(); i++ ){
		m_Index.ReadIndex(call, m_Find.pFindTbl[i]);
		if( !strcmp(call, m_sd.call) ){
			GetData(&sd, m_Find.pFindTbl[i]);
			MultProc();
			if( !sd.etime && (i == (m_Find.GetCount()-1)) ) break; 
			WORD btime = sd.btime;
			JSTtoUTC(&sd);
			int yy = sd.year + 2000;
			int mm = sd.date / 100;
			int dd = sd.date % 100;
			if( (sd.mode == 8) && (btime != m_sd.btime) && (yy == utc.wYear) && (mm == utc.wMonth) && (dd == utc.wDay) ) return 1;
		}
	}
	return 0;
}

void CLogFile::FindStrSet(CLogFind *fp, LPCSTR pCall)
{
	if( !*pCall ) return;
	if( fp->GetCount() >= FINDMAX ) return;
	char	call[MLCALL+1];

	MultProc();
	int i;
	for( i = m_fhd.size - 1; i >= 0; i-- ){
		m_Index.ReadIndex(call, i);
		if( strstr(call, pCall)!=NULL ){
			fp->Add(i);
			if( fp->GetCount() >= FINDMAX ) break;
		}
	}
}

int CLogFile::FindCmpSet(CLogFind *fp, LPCSTR pCall)
{
	if( fp->GetCount() >= FINDMAX ) return 0;
	char	call[MLCALL+1];

	MultProc();
	int i;
	for( i = m_fhd.size - 1; i >= 0; i-- ){
		if( *pCall ){
			m_Index.ReadIndex(call, i);
			if( !strcmp(call, pCall) ){
				fp->Write(i);
			}
		}
		else {
			fp->Write(i);
		}
		if( fp->GetCount() >= FINDMAX ) break;
	}
	return fp->GetCount();
}

int CLogFile::FindClipSet(CLogFind *fp, LPCSTR pCall)
{
	if( !*pCall ) return 0;
	if( fp->GetCount() >= FINDMAX ) return 0;
	char	call[MLCALL+1];

	MultProc();
	int i;
	for( i = m_fhd.size - 1; i >= 0; i-- ){
		m_Index.ReadIndex(call, i);
		if( !strcmp(ClipCall(call), pCall) ){
			fp->Add(i);
		}
		if( fp->GetCount() >= FINDMAX ) break;
	}
	return fp->GetCount();
}

int CLogFile::FindSet(CLogFind *fp, LPCSTR pCall)
{
	fp->SetText(pCall);
	fp->Clear();
	int r = FindCmpSet(fp, pCall);
	fp->m_FindCmp1Max = fp->m_FindCnt;
	char	clipcall[MLCALL+1];
	strcpy(clipcall, ClipCall(pCall));
	if( FindClipSet(fp, clipcall) ){
		r = 1;
	}
	fp->m_FindCmp2Max = fp->m_FindCnt;
	FindStrSet(fp, pCall);
	fp->m_FindStr1Max = fp->m_FindStr2Max = fp->m_FindCnt;
	if( strcmp(pCall, clipcall) ){
		FindStrSet(fp, clipcall);
		fp->m_FindStr2Max = fp->m_FindCnt;
	}
	return r;
}

int CLogFile::Find(LPCSTR pCall, int b, int dir)
{
	if( !m_fhd.size ) return -1;
	char	call[MLCALL+1];

	MultProc();
	int i;
	if( dir ){
		for( i = b; i >= 0; i-- ){
			m_Index.ReadIndex(call, i);
			if( strstr(call, pCall) != NULL ) return i;
		}
	}
	else {
		for( i = b; i < m_fhd.size; i++ ){
			m_Index.ReadIndex(call, i);
			if( strstr(call, pCall) != NULL ) return i;
		}
	}
	return -1;
}

int CLogFile::IsAlready(LPCSTR pCall)
{
	if( !m_fhd.size ) return -1;
	char	call[MLCALL+1];

	for( int i = m_CurNo; i >= 0; i-- ){
		m_Index.ReadIndex(call, i);
		if( !strcmp(call, pCall) ) return i;
	}
	return -1;
}

void CLogFile::DeleteAll(void)
{
	m_fhd.size = 0;
	m_CurNo = m_fhd.size;
	m_CurChg = 1;
	m_EditFlag = 1;
}

void CLogFile::DeleteLast(void)
{
	if( m_fhd.size ){
		m_fhd.size--;
	}
	m_CurNo = m_fhd.size;
	m_CurChg = 1;
	m_EditFlag = 1;
}

void CLogFile::Delete(int top, int end)
{
	if( !m_fhd.size ) return;

	if( end >= (m_fhd.size - 1) ){		// �Ō�܂ō폜����ꍇ
		m_fhd.size = top;
		m_CurNo = m_fhd.size;
	}
	else {								// �r�����폜����ꍇ
		int i, j;
		SDMMLOG	sd;
		for( i = top, j = end + 1; j < m_fhd.size; i++, j++ ){
			GetData(&sd, j);
			PutData(&sd, i);
		}
		m_fhd.size -= end - top + 1;
		m_CurNo = m_fhd.size;
	}
	SetLastPos();
	m_CurChg = 1;
	m_EditFlag = 1;
}

void CLogFile::Insert(int n, SDMMLOG *sp)
{
	if( n >= m_fhd.size ){				// �ŏI�ʒu�̏ꍇ�͒ǉ��Ɠ���
		PutData(sp, n);
		m_CurNo++;
	}
	else {								// �r�����폜����ꍇ
		int i, j;
		SDMMLOG	sd;
		i = m_fhd.size;
		j = i - 1;
		for( ; i > n; i--, j-- ){
			GetData(&sd, j);
			PutData(&sd, i);
		}
		PutData(sp, n);
		m_CurNo++;
		m_CurChg = 0;
	}
	m_EditFlag = 1;
}

LPCSTR CLogFile::GetDateString(SDMMLOG *sp, int sw)
{
	static char bf[12];

	if( sp->date ){
		switch(sw){
			case 1:			// yyyy-mm-dd
				sprintf(bf, "%04u.%02u.%02u", YEAR(sp->year), sp->date/100, sp->date%100);
				break;
			case 2:			// dd-mm-yy
				sprintf(bf, "%02u.%02u.%02u", sp->date%100, sp->date/100, sp->year);
				break;
			case 3:			// dd-mm-yyyy
				sprintf(bf, "%02u.%02u.%04u", sp->date%100, sp->date/100, YEAR(sp->year));
				break;
			case 4:			// mm-dd-yy
				sprintf(bf, "%02u.%02u.%02u", sp->date/100, sp->date%100, sp->year);
				break;
			case 5:			// mm-dd-yyyy
				sprintf(bf, "%02u.%02u.%04u", sp->date/100, sp->date%100, YEAR(sp->year));
				break;
			default:		// yy-mm-dd
				sprintf(bf, "%02u.%02u.%02u", sp->year, sp->date/100, sp->date%100);
				break;
		}
	}
	else {
		bf[0] = 0;
	}
	return bf;
}

LPCSTR CLogFile::GetTimeString(WORD d)
{
	static char bf[8];

	if( d ){
		d = WORD(d / 30);
		sprintf(bf, "%02u%02u", d / 60, d % 60);
	}
	else {
		bf[0] = 0;
	}
	return bf;
}

LPCSTR CLogFile::GetModeString(BYTE m)
{
	if( m < MODEMAX ){
		return _mode[m];
	}
	else {
		m -= BYTE(MODEMAX);
		StrCopy(m_modebuf, m_fhd.mode[m], 6);
		return m_modebuf;
	}
}

void CLogFile::SetMode(SDMMLOG *sp, LPCSTR s)
{
	if( !*s ){
		sp->mode = 0;
		return;
	}

	int n;
	const char	**t;
	char bf[7];
	StrCopy(bf, s, 6);
	jstrupr(bf);
	s = bf;
	int len = strlen(s);
	for( n = 0, t = _mode; *t != NULL; t++, n++ ){
		if( !strcmp(*t, s) ){
			sp->mode = BYTE(n);
			return;
		}
	}
	for( n = 0; (n < UMODEMAX) && m_fhd.mode[n][0] ; n++ ){
		char vbf[7];
		StrCopy(vbf, m_fhd.mode[n], 6);
		if( !strcmp(vbf, s) ){
			sp->mode = BYTE(n + MODEMAX);
			return;
		}
	}
	if( n < UMODEMAX ){
		if( len < 6 ){
			strcpy(m_fhd.mode[n], s);
		}
		else {
			memcpy(m_fhd.mode[n], s, 6);
		}
		sp->mode = BYTE(n + MODEMAX);
	}
	else {
		sp->mode = 0;
	}
}

LPCSTR CLogFile::GetFreqString(BYTE b, short fq)
{
	static char	bf[12];
	int			d;

	if( fq < 0 ){
		sprintf( bf, "%s/%s", _band[b], _band[-fq]);
		return(bf);
	}
	else if( fq ){
		switch(b){
			case 0:
				return "";
			case B_19:	/* 1.9	*/
				if( fq == 800 ){
					strcpy(bf, "1.8");
				}
				else {
					sprintf( bf, "1.%03u", fq );
				}
				break;
			case B_35:	/* 3.5	*/
				sprintf( bf, "3.%03u", fq );
				break;
			case B_38:	/* 3.8	*/
				sprintf( bf, "3.%03u", fq );
				break;
			case B_1200:
				d = 1240 + (fq / 100);
				sprintf( bf, "%u.%02u", d, fq % 100 );
				break;
			case B_2400:
				d = 2350 + (fq / 100);
				sprintf( bf, "%u.%02u", d, fq % 100 );
				break;
			case B_220:
				d = 220 + (fq / 1000);
				sprintf( bf, "%u.%03u", d, fq % 1000 );
				break;
			default:
				if( b <= B_24 ){
					sprintf( bf, "%s.%03u", _band[b], fq );
				}
				else if( b <= B_430 ){
					d = atoin(_band[b], -1) + (fq / 1000);
					sprintf( bf, "%u.%03u", d, fq % 1000 );
				}
				else {
					return(_band[b]);
				}
				break;
		}
		return(bf);
	}
	else {
		return(_band[b]);
	}
}

/*#$%
===============================================================
	�o���h�ԍ��𓾂�
---------------------------------------------------------------
	p : ������̃|�C���^
---------------------------------------------------------------
---------------------------------------------------------------
===============================================================
*/
static BYTE _bandno_(LPCSTR p)
{
	char	n;
	const char **t;

	for( n = 0, t = _band; *t != NULL; t++, n++ ){
		if( !strcmp(*t, p) ) return(n);
	}
	return(0);
}

/*#$%
===============================================================
	�o���h�ԍ��𓾂�
---------------------------------------------------------------
	p : ������̃|�C���^
---------------------------------------------------------------
---------------------------------------------------------------
===============================================================
*/
void CLogFile::SetFreq(SDMMLOG *sp, LPCSTR p)
{
	BYTE	n;
	char	ub[16], db[16];
	int		ud, d;

	sp->fq = 0;
	if( strchr(p, '/' )!=NULL ){
		p = _strdmcpy(ub, p, '/');
		if( (n = _bandno_(ub))!= 0 ){
			if( (sp->fq = _bandno_(p))!= 0 ){
				sp->fq = SHORT(-sp->fq);
				sp->band = n;
				return;
			}
		}
		sp->band = 0;
		return;
	}
	if( (n = _bandno_(p))!= 0 ){
		sp->band = n;
		return;
	}
	if( strchr(p, '.')!=NULL ){
		p = _strdmcpy(ub, p, '.');
		memset(db, 0, 4);
		strcpy(db, p);
		db[3] = 0;
		for( n = 0; n < 3; n++ ){
			if( !db[n] ) db[n] = '0';
		}
		ud = atoin(ub, -1);
		d = atoin(db, -1);
	}
	else {
		ud = atoin(p, -1);
		d = 0;
	}
	if( ud == 1 ){
		sp->fq = SHORT(d);
		sp->band = B_19;
	}
	else if( ud == 3 ){
		sp->fq = SHORT(d);
		if( d >= 700 ){
			n = B_38;
		}
		else {
			n = B_35;
		}
		sp->band = n;
	}
	else if( (ud >= 7) && (ud <= 24) ){
		sp->fq = SHORT(d);
		if( (n = _bandno_(ub))!= 0 ){
			sp->band = n;
			return;
		}
		sp->fq = 0;
		sp->band = 0;
	}
	else if( (ud >= 28)&&(ud <= 29) ){
		sp->fq = SHORT(((ud - 28) * 1000) + d);
		sp->band = B_28;
	}
	else if( (ud >= 50)&&(ud <= 54) ){
		sp->fq = SHORT(((ud - 50) * 1000) + d);
		sp->band = B_50;
	}
	else if( (ud >= 144) && (ud <= 147) ){
		sp->fq = SHORT(((ud - 144) * 1000) + d);
		sp->band = B_144;
	}
	else if( (ud >= 430) && (ud <= 440) ){
		sp->fq = SHORT(((ud - 430) * 1000) + d);
		sp->band = B_430;
	}
	else if( (ud >= 220) && (ud <= 225) ){
		sp->fq = SHORT(((ud - 220) * 1000) + d);
		sp->band = B_220;
	}
	else if( (ud >= 1240) && (ud <= 1300) ){
		sp->fq = SHORT(((ud - 1240) * 100) + (d/10));
		sp->band = B_1200;
	}
	else if( (ud >= 2350) && (ud <= 2450) ){
		sp->fq = SHORT(((ud - 2350) * 100) + (d/10));
		sp->band = B_2400;
	}
	else {
		sp->band = 0;
	}
}

int CLogFile::ReadAscii(SDMMLOG *sp, LPSTR p)
{
	LPSTR	t;
	int	y, m, d;

	memset(sp, 0, sizeof(SDMMLOG));
	p = StrDlm(t, p);		/* DATE	*/
	if( sscanf(t, "%u.%u.%u", &y, &m, &d )!=3 ) return FALSE;
	sp->year = BYTE(y % 100);
	sp->date = WORD((m * 100) + d);
	p = StrDlm(t, p);		/* BGN	*/
	if( sscanf(t, "%u.%u", &d, &y) != 2 ){
		d = atoin(t, -1);
		y = 0;
	}
	m = d / 100;
	d = d % 100;
	y /= 2;
	sp->btime = WORD((((m * 60) + d) * 30) + y);
	p = StrDlm(t, p);		/* CALL	*/
	StrCopy(sp->call, t, MLCALL);
	p = StrDlm(t, p);		/* UR	*/
	StrCopy(sp->ur, t, MLRST);
	p = StrDlm(t, p);		/* MY	*/
	StrCopy(sp->my, t, MLRST);
	p = StrDlm(t, p);		/* BAND	*/
	SetFreq(sp, t);
	p = StrDlm(t, p);		/* MODE	*/
	SetMode(sp, t);
	p = StrDlm(t, p);		/* POW	*/
	StrCopy(sp->pow, t, MLPOW);
	p = StrDlm(t, p);		/* NAME	*/
	StrCopy(sp->name, t, MLNAME);
	p = StrDlm(t, p);		/* QTH	*/
	StrCopy(sp->qth, t, MLQTH);
	p = StrDlm(t, p);		/* REM	*/
	StrCopy(sp->rem, t, MLREM);
	p = StrDlm(t, p);		/* QSL	*/
	StrCopy(sp->qsl, t, MLQSL);
	p = StrDlm(t, p);		/* ETIME*/
	d = atoin(t, -1);
	m = d / 100;
	d = d % 100;
	sp->etime = WORD(((m * 60) + d) * 30);
	p = StrDlm(t, p);		/* S	*/
	sp->send = *t;
	p = StrDlm(t, p);		/* R	*/
	sp->recv = *t;
	p = StrDlm(t, p);		/* M	*/
	sp->cq = *t;
	p = StrDlm(t, p);		/* ENV	*/
	sp->env = WORD(atoin(t, -1));
	p = StrDlm(t, p);		/* OPT1	*/
	StrCopy(sp->opt1, t, MLOPT);
	p = StrDlm(t, p);		/* OPT2	*/
	StrCopy(sp->opt2, t, MLOPT);
	p = StrDlm(t, p);		/* USR1	*/
	if( *t ) SetOptStr(2, sp, t);
	StrDlm(t, p);		/* USR2	*/
	if( *t ) SetOptStr(3, sp, t);
	return TRUE;
}

void JSTtoUTC(int &Year, int &Mon, int &Day, int &Hour)
{

	Hour -= 9;
	if( Hour < 0 ){
		Hour += 24;
		Day--;
		if( Day < 1 ){
			Mon--;
			if( Mon < 1 ){
				Mon = 12;
				if( Year ){
					Year--;
				}
				else {
					Year = 99;
				}
			}
			if( Year % 4 ){
				Day = MONN[Mon];
			}
			else {
				Day = MONU[Mon];
			}
		}
	}
}

void JSTtoUTC(SDMMLOG *sp)
{
	int Year = sp->year;
	int Mon = sp->date / 100;
	int Day = sp->date % 100;
	int	Hour = sp->btime / (60*30);
	if( sp->date || sp->btime ){
		JSTtoUTC(Year, Mon, Day, Hour);
		sp->year = BYTE(Year);
		sp->date = WORD(Mon * 100 + Day);
		sp->btime = WORD((sp->btime % 1800) + (Hour * 1800));
	}
	if( sp->etime ){
		// JST to UTC
		if( sp->etime >= (9*60*30) ){
			sp->etime -= WORD(9 * 60 * 30);
		}
		else {
			sp->etime += WORD(15 * 60 * 30);
		}
		if( !sp->etime ) sp->etime++;
	}
}

void UTCtoJST(int &Year, int &Mon, int &Day, int &Hour)
{
	LPCSTR	mt;

	Hour += 9;
	if( Hour >= 24 ){
		Hour -= 24;
		Day++;
		if( Year % 4 ){
			mt = MONN;
		}
		else {
			mt = MONU;
		}
		if( Day > mt[Mon] ){
			Day = 1;
			Mon++;
			if( Mon > 12 ){
				Mon = 1;
				Year++;
				if( Year >= 100 ) Year = 0;
			}
		}
	}
}

void UTCtoJST(SDMMLOG *sp)
{
	int Year = sp->year;
	int Mon = sp->date / 100;
	int Day = sp->date % 100;
	int	Hour = sp->btime / (60*30);
	if( sp->date || sp->btime ){
		UTCtoJST(Year, Mon, Day, Hour);
		sp->year = BYTE(Year);
		sp->date = WORD(Mon * 100 + Day);
		sp->btime = WORD((sp->btime % 1800) + (Hour * 1800));
	}
	if( sp->etime ){
		// UTC to JST
		if( sp->etime >= (15*60*30) ){
			sp->etime -= WORD(15 * 60 * 30);
		}
		else {
			sp->etime += WORD(9 * 60 * 30);
		}
		if( !sp->etime ) sp->etime++;
	}
}

void UTCtoJST(SYSTEMTIME *tp)
{
	int y = tp->wYear;
	int m = tp->wMonth;
	int d = tp->wDay;
	int h = tp->wHour;
	UTCtoJST(y, m, d, h);
	tp->wYear = WORD(y);
	tp->wMonth = WORD(m);
	tp->wDay = WORD(d);
	tp->wHour = WORD(h);
}

void mBandToBand(SDMMLOG *sp, LPCSTR p)
{
	int		m;

	sp->fq = 0;
	if( !strcmpi(p, "SAT") ){
		sp->band = 0;
	}
	else if( sscanf(p, "%u", &m) == 1 ){
		switch(m){
		case 2:
			sp->band = B_144;
			break;
		case 6:
			sp->band = B_50;
			break;
		case 10:
			sp->band = B_28;
			break;
		case 12:
			sp->band = B_24;
			break;
		case 15:
			sp->band = B_21;
			break;
		case 17:
			sp->band = B_18;
			break;
		case 20:
			sp->band = B_14;
			break;
		case 30:
			sp->band = B_10;
			break;
		case 40:
			sp->band = B_7;
			break;
		case 75:
			sp->band = B_38;
			break;
		case 80:
			sp->band = B_35;
			break;
		case 160:
			sp->band = B_19;
			sp->fq = 800;
			break;
		}
	}
}

LPCSTR FreqTomBand(SDMMLOG *sp)
{
	return _mBand[sp->band];
}


