
// AC64Dlg.h: Headerdatei
//

#pragma once
#include "afxwin.h"

struct vm_context;

// CAC64Dlg-Dialogfeld
class CAC64Dlg : public CDialogEx
{
// Konstruktion
public:
	CAC64Dlg(CWnd* pParent = NULL);	// Standardkonstruktor

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AC64_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung


// Implementierung
protected:
	HICON m_hIcon;

	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit _asmCode;
	CEdit _memoryView;
	afx_msg void OnBnClickedDumpMemory();
private:
	vm_context* _ctx;
	int hex2int(const CString& txt);
	void dumpMemory();
	int _memoryAddress;
	CStatusBarCtrl _statucBarCtrl;
	void updateCPUState();
	int compile();
public:
	CEdit _memAddress;
	afx_msg void OnBnClickedRun();
	afx_msg void OnBnClickedPreviousMem();
	afx_msg void OnBnClickedNextMem();
private:
	CEdit _regA;
	CEdit _regX;
	CEdit _regY;
	CEdit _programCounter;
	CEdit _stackPointer;
public:
	afx_msg void OnBnClickedLoadCode();
	afx_msg void OnBnClickedResetPc();
	afx_msg void OnBnClickedStep();
private:
	CEdit _cpuFlags;
public:
	afx_msg void OnBnClickedLoadBin();
	afx_msg void OnBnClickedSaveBin();
	afx_msg void OnBnClickedSaveText();
private:
	CEdit _numCommands;
	CEdit _numBytes;
public:
	afx_msg void OnBnClickedCompile();
	afx_msg void OnEnChangeRegX();
	afx_msg void OnBnClickedDisassemble();
};
