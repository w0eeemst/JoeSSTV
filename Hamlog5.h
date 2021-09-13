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



#ifndef Hamlog5H
#define	Hamlog5H

// MMQSL�pHamlog Ver5�̃h���C�o

#include "ComLib.h"
#include "LogFile.h"
extern const LPCSTR g_Hamlog5Key[];
extern const BYTE g_Hamlog5Len[];

typedef enum {
	itemhamlog5CALLS,
	itemhamlog5IGN,
	itemhamlog5DATE,
	itemhamlog5TIME,
	itemhamlog5CODE,
	itemhamlog5GL,
	itemhamlog5QSL,
	itemhamlog5FLAG,
	itemhamlog5HIS,
	itemhamlog5MY,
	itemhamlog5FREQ,
	itemhamlog5MODE,
	itemhamlog5NAME,
	itemhamlog5QTH,
	itemhamlog5RMK1,
	itemhamlog5RMK2,
}HAMLOG5_ITEMS;

#define	bithamlog5DX		8
#define	bithamlog5NorOdr	1
#define	bithamlog5RevOdr	2

#pragma pack(1)
typedef struct {
	BYTE	Type;
	BYTE	Year;
	BYTE	Mon;
	BYTE	Day;
	ULONG	RecMax;
	USHORT	FilOff;
	USHORT	RecWidth;
	BYTE	dm1[20];
}hamlog5DBHD;
typedef struct {
	char	Name[11];
	BYTE	Type;
	BYTE	dm1[4];
	BYTE	Len;
	BYTE	dm2[15];
}hamlog5DBRHD;
#pragma pack()

#define	HamlogDBMAX		16
class CHamlog5
{
private:
	BOOL	m_fCreate;
	hamlog5DBHD	m_Head;				// �w�b�_���
	ULONG	m_RecMax;			// ���R�[�h��
	USHORT	m_FilMax;			// �t�B�[���h��
	USHORT	m_RecWidth;			// ���R�[�h�̕�
	USHORT	m_FilOff;			// �w�b�_�I�t�Z�b�g
	FILE	*m_fp;
	AnsiString	m_StrTable[HamlogDBMAX];	// �e�[�u�����̔z��
	USHORT	m_PosTable[HamlogDBMAX];	// �t�B�[���h�ʒu�̔z��
	USHORT	m_LenTable[HamlogDBMAX];	// �t�B�[���h�����̔z��
	char	m_TypeTable[HamlogDBMAX];	// �^���̔z��
	BOOL	m_OpenFlag;			// �t�@�C���I�[�v���t���O

	ULONG	m_Index;			// �V�[�N���̃C���f�b�N�X
	ULONG	m_Pos;				// �V�[�N���̃t�@�C���ʒu
	LPSTR	m_bp;				// �o�b�t�@�̃|�C���^
	BOOL	m_WriteFlag;		// �������݃t���O
	BOOL	SetupHeader(void);	// �w�b�_�Z�b�g�A�b�v
	BOOL	MakeHeader(const LPCSTR _NT[], const BYTE _LT[]);
public:
	CHamlog5();
	~CHamlog5();
	BOOL Open(LPCSTR Name, BOOL fMsg);
	BOOL Create(LPCSTR Name);
	void Close(void);
	inline ULONG GetRCount(void){return m_RecMax;};
	inline USHORT GetFCount(void){return m_FilMax;};
	AnsiString	*GetStrBase(void){return m_StrTable;};
	BOOL Seek(ULONG Index);
	BOOL IsData(void);
	BOOL GetData(USHORT SubIndex, AnsiString &cs);
	BOOL GetData(USHORT SubIndex, LPSTR pStore);
	BOOL SetData(USHORT SubIndex, LPCSTR p);
	BOOL SetBinary(USHORT SubIndex, LPBYTE p);
	int GetData(USHORT SubIndex, LPBYTE pData, int len);
	BOOL Update(void);
	void DecodeData(SDMMLOG *sp);
	void EncodeData(SDMMLOG *sp);
};

void __fastcall GetHamlog5FieldsLen(AnsiString &as);
void __fastcall SetHamlog5FieldsLen(AnsiString &as);
#endif
