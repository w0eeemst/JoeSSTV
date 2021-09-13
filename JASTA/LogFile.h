//---------------------------------------------------------------------------
#ifndef LogFileH
#define LogFileH

#include "ComLib.h"
//---------------------------------------------------------------------------
#define	MMLOGID	"MMLOG DATA Ver1.00\032"

extern const char	*_band[];

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

typedef struct {	// Log�̓���I�v�V����
	char	m_TimeZone;				// �\���p�^�C���]�[��

	int		m_UpperName;			// �啶���ւ̕ϊ�
	int		m_UpperQTH;				// �啶���ւ̕ϊ�
	int		m_UpperREM;				// �啶���ւ̕ϊ�
	int		m_UpperQSL;				// �啶���ւ̕ϊ�

	int		m_DefMyRST;				// 0-OFF, 1-ON

	int		m_CopyFreq;				// 0-Band, 1-Freq
	int		m_CopyHis;				// 0-599, 1-599001, 2-599UTC
	int		m_CopyName;				// 0-OFF, 1-ON
	int		m_CopyQTH;				// 0-OFF, 1-ON
	int		m_CopyREM;				// 0-OFF, 1-ON
	int		m_CopyQSL;				// 0-OFF, 1-ON

	int		m_AutoSave;				// �����Z�[�u

	int		m_QSOMacroFlag;			// 0-OFF, 1-ON
	int		m_CheckBand;			// ����o���h�ł̏d�����`�F�b�N
	int		m_QSOMacro[5];			// 0-Run 1st, 1-Run 2nd, 2-Run Dupe, 3-S&P 1st, 4-S&P 2nd
	WORD	m_QSOMacroKey[5];
	AnsiString	m_QSOMacroStr[5];

	AnsiString	m_THRTTY;
	AnsiString	m_THSSTV;
	int     m_THTZ;
	int		m_ClipRSTADIF;
	int		m_DateType;
}LOGSET;

#define	MLCALL	16		/* ��ٻ�݂̒���		*/
#define	MLRST	20		/* RST�i���o�̒���	*/
#define	MLREM	56		/* �L���̒���		*/
#define	MLNAME	16		/* ���O�̒���		*/
#define	MLQTH	28		/* �p�s�g�̒���		*/
#define	MLQSL	54		/* �p�r�k�̒���		*/
#define	MLPOW	4		/* �d��				*/
#define	MLOPT	8		/* �I�v�V����		*/

#define MODEMAX	48

#define	YEAR(c)	(((c)<50)?(2000+(c)):(1900+(c)))

#pragma option -a-	// �p�b�N�̎w��
typedef struct {			/* �l�l�k�n�f�f�[�^�`��	*/
	BYTE	year;		/* �N		*/
	WORD	date;			/* ���t		*/
	WORD	btime;			/* �J�n����	*/
	WORD	etime;			/* �I������	*/
	char	call[MLCALL+1];	/* ��ٻ��	*/
	char	ur[MLRST+1];	/* T ��߰�	*/
	char	my[MLRST+1];	/* R ��߰�	*/
	BYTE	band;		/* �����	*/
	SHORT	fq;				/* ���g��	*/
	BYTE	mode;		/* Ӱ��		*/
	char	pow[MLPOW+1];	/* �d��		*/
	char	name[MLNAME+1];	/* ���O		*/
	char	qth[MLQTH+1];	/* QTH		*/
	char	qsl[MLQSL+1];	/* QSL		*/
	char	send;			/* QSL ���M	*/
	char	recv;			/* QSL ��M	*/
	char	cq;				/* cq/call	*/
	WORD	env;			/* ���Ǌ�	*/
	char	rem[MLREM+1];	/* �L��		*/
	char	opt1[MLOPT+1];	/* ��߼��	*/
	char	opt2[MLOPT+1];	/* ��߼��2	*/
}SDMMLOG;

#define	FHDOFF	256			/* �擪�̃w�b�_�̃I�t�Z�b�g		*/
#define	UMODEMAX	32
typedef struct {
	char	id[20];			/* �t�@�C���o�[�W����			*/
	char	dmy[4];			/* �_�~�[�̈�					*/
	USHORT	mlt;			/* �}���`���̃T�C�Y			*/
	char	td;				/* �����R�[�h					*/
	char	hash;			/* �n�b�V���f�[�^�i�[�t���O		*/
	long	size;			/* �f�[�^�T�C�Y					*/
	char	master;			/* �}�X�^�[�t�@�C���t���O		*/
	char	dm2[15];		/* �_�~�[�Q						*/
	char	mode[UMODEMAX][6];	/* ���[�U��`���[�h				*/
}FHD;
#pragma option -a.	// �p�b�N�����̎w��

class CIndex
{
private:
	int		m_IndexMax;		// �m�ے��̃C���f�b�N�X�̐�
	int		m_IndexCnt;		// ���݂̃C���f�b�N�X�̐�
	LPSTR	pIndex;			// �C���f�b�N�X�̈�̃|�C���^(16�o�C�g�C���f�b�N�X)
	BYTE	*pMult;			// �}���`���̃|�C���^
public:
	CIndex();
	~CIndex();
	void AllocIndex(int n);
	void ClearIndex(void);
	void WriteIndex(LPCSTR pCall, int n);
	void ReadIndex(LPSTR pCall, int n);
	LPSTR PointIndex(int n);
	void AddBlock(LPCSTR p, int len);
	int ReadIndex(int handle, FHD *hp);
	void MakeIndex(int handle, FHD *hp);
	int WriteIndex(int handle, FHD *hp);


};

#define	FINDMAX		32768
class CLogFind
{
public:
	int			m_FindCnt;
	int			m_FindCmp1Max;
	int			m_FindCmp2Max;
	int			m_FindStr1Max;
	int			m_FindStr2Max;

	int			*pFindTbl;
	AnsiString	m_FindStr;
public:
	CLogFind();
	~CLogFind();
	inline int GetCount(void){return m_FindCnt;};
	inline void Clear(void){
		m_FindCnt = m_FindCmp1Max = m_FindCmp2Max = m_FindStr1Max = m_FindStr2Max = 0;
	};
	int Add(int n);
	void Ins(int n);

	inline void Write(int n){
		pFindTbl[m_FindCnt] = n;
		m_FindCnt++;
	};
	inline void SetText(LPCSTR p){
		if( p != m_FindStr.c_str() ){
			m_FindStr = p;
		}
	};
	inline LPCSTR GetText(void){
		return m_FindStr.c_str();
	};
	inline void ClearText(void){
		m_FindStr = "";
	};
};

class CLogFile
{
private:
	int		m_Open;			// �I�[�v���t���O
	int     m_ReadOnly;
	int		m_EditFlag;		// �ҏW�t���O
	int		m_Handle;		// �t�@�C���n���h��
	CIndex	m_Index;		// ���݃I�[�v�����̃C���f�b�N�X
	FHD		m_fhd;			// ���݃I�[�v�����̃t�@�C���w�b�_
	AnsiString	m_Name;		// ���O�t�@�C���̖��O
	char	m_modebuf[8];
	char    LogDir[256];
public:
	AnsiString	m_FileName;	// ���O�t�@�C���̖��O�i�t���p�X�j

	int		m_CurNo;
	int		m_CurChg;
	SDMMLOG	m_sd;
	SDMMLOG	m_bak;
	SDMMLOG	m_asd;

	CLogFind	m_Find;		// �J�����g�����f�[�^
	LOGSET		m_LogSet;
private:
	void SetHisRST(SDMMLOG *sp);

public:
	CLogFile();
	~CLogFile();

	int Open(LPCSTR pName, int enbwrt);
	int Close(void);

	void ReadIniFile(LPCSTR pKey, TIniFile *pIniFile);
	void WriteIniFile(LPCSTR pKey, TIniFile *pIniFile);

	void MakeIndex(void);

	inline IsEdit(void){return m_EditFlag;};
	void MakePathName(LPCSTR pName);
	void MakeName(LPCSTR pName);
	inline LPCSTR GetName(void){return m_Name.c_str();};

	inline int GetCount(void){return m_fhd.size;};
	inline int IsOpen(void){return m_Open;};
	int GetData(SDMMLOG *sp, int n);
	int PutData(SDMMLOG *sp, int n);

	LPCSTR GetDateString(SDMMLOG *sp, int sw);
	inline LPCSTR GetDateString(SDMMLOG *sp){ return GetDateString(sp, m_LogSet.m_DateType); };
	LPCSTR GetTimeString(WORD d);


	LPCSTR GetModeString(BYTE m);
	void SetMode(SDMMLOG *sp, LPCSTR s);

	LPCSTR GetFreqString(BYTE b, short fq);
	void SetFreq(SDMMLOG *sp, LPCSTR p);
	LPCSTR GetOptStr(int n, SDMMLOG *sp);
	void SetOptStr(int n, SDMMLOG *sp, LPCSTR pOpt);

	void CopyAF(void);

	void SortDate(int bb, int eb);
	int FindSameBand(void);
	int FindSameDate(SYSTEMTIME *pUTC);

	void FindStrSet(CLogFind *fp, LPCSTR pCall);
	int FindCmpSet(CLogFind *fp, LPCSTR pCall);
	int FindClipSet(CLogFind *fp, LPCSTR pCall);
	int FindSet(CLogFind *fp, LPCSTR pCall);
	int Find(LPCSTR pCall, int b, int dir);
	int IsAlready(LPCSTR pCall);
	void DeleteAll(void);
	void DeleteLast(void);
	void Delete(int top, int end);
	void Insert(int n, SDMMLOG *sp);

	void InitCur(void);
	void SetLastPos(void);

	int ReadAscii(SDMMLOG *sp, LPSTR p);
	CIndex *GetIndex(void){return &m_Index;};
};

void JSTtoUTC(int &Year, int &Mon, int &Day, int &Hour);
void JSTtoUTC(SDMMLOG *sp);
void UTCtoJST(int &Year, int &Mon, int &Day, int &Hour);
void UTCtoJST(SDMMLOG *sp);
void UTCtoJST(SYSTEMTIME *tp);
void mBandToBand(SDMMLOG *sp, LPCSTR p);
LPCSTR FreqTomBand(SDMMLOG *sp);
int GetLMode(BYTE m);

extern	CLogFile	Log;
extern	const char	MONN[];
extern	const char	MONU[];
#endif

