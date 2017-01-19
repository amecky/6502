
// AC64Dlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "..\6502.h"
#include "AC64.h"
#include "AC64Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg-Dialogfeld für Anwendungsbefehl "Info"

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

// Implementierung
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAC64Dlg-Dialogfeld



CAC64Dlg::CAC64Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_AC64_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	_ctx = vm_create();
}

void CAC64Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ASMCODE, _asmCode);
	DDX_Control(pDX, IDC_MEMORY_DUMP, _memoryView);
	DDX_Control(pDX, IDC_MEM_ADR, _memAddress);
	DDX_Control(pDX, IDC_REG_A, _regA);
	DDX_Control(pDX, IDC_REG_X, _regX);
	DDX_Control(pDX, IDC_REG_A3, _regY);
	DDX_Control(pDX, IDC_REG_A2, _programCounter);
	DDX_Control(pDX, IDC_REG_A4, _stackPointer);
	DDX_Control(pDX, IDC_EDIT1, _cpuFlags);
}

BEGIN_MESSAGE_MAP(CAC64Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DUMP_MEMORY, &CAC64Dlg::OnBnClickedDumpMemory)
	ON_BN_CLICKED(IDC_RUN, &CAC64Dlg::OnBnClickedRun)
	ON_BN_CLICKED(IDC_PREVIOUS_MEM, &CAC64Dlg::OnBnClickedPreviousMem)
	ON_BN_CLICKED(IDC_NEXT_MEM, &CAC64Dlg::OnBnClickedNextMem)
	ON_BN_CLICKED(IDC_LOAD_CODE, &CAC64Dlg::OnBnClickedLoadCode)
	ON_BN_CLICKED(IDC_RESET_PC, &CAC64Dlg::OnBnClickedResetPc)
	ON_BN_CLICKED(IDC_STEP, &CAC64Dlg::OnBnClickedStep)
END_MESSAGE_MAP()


// CAC64Dlg-Meldungshandler

BOOL CAC64Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Hinzufügen des Menübefehls "Info..." zum Systemmenü.

	// IDM_ABOUTBOX muss sich im Bereich der Systembefehle befinden.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Symbol für dieses Dialogfeld festlegen.  Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	// TODO: Hier zusätzliche Initialisierung einfügen
	_memAddress.SetWindowTextW(_T("0x600"));
	_memoryAddress = 0x600;
	dumpMemory();
	RECT r;
	GetClientRect(&r);
	r.top = r.bottom - 20;
	_statucBarCtrl.Create(WS_BORDER | WS_VISIBLE | CBRS_BOTTOM, r, this, 3);
	_statucBarCtrl.SetText(_T("Hello world"), 0, 0);
	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void CAC64Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// Wenn Sie dem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie
//  den nachstehenden Code, um das Symbol zu zeichnen.  Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch ausgeführt.

void CAC64Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Die System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CAC64Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ----------------------------------------------------
// dump memory
// ----------------------------------------------------
void CAC64Dlg::OnBnClickedDumpMemory() {	
	CString txt = _T("");
	_memAddress.GetWindowTextW(txt);
	_memoryAddress = hex2int(txt);
	dumpMemory();
}

// ----------------------------------------------------
// internal dump memory
// ----------------------------------------------------
void CAC64Dlg::dumpMemory() {
	CString result = _T("");
	int num = 256;
	for (size_t i = 0; i < num; ++i) {
		if (i % 16 == 0) {
			int adr = _memoryAddress + i;
			if (i > 0) {
				result.Append(_T("\r\n"));
			}
			result.AppendFormat(_T("%04X : "), adr);
		}
		result.AppendFormat(_T("%02X "), _ctx->read(_memoryAddress + i));
	}
	_memoryView.SetWindowTextW(result);
	updateCPUState();
}

// ----------------------------------------------------
// Update CPU state
// ----------------------------------------------------
void CAC64Dlg::updateCPUState() {
	CString result = _T("");
	result.Format(_T("%02X"), _ctx->registers[vm_registers::A]);
	_regA.SetWindowTextW(result);
	result.Format(_T("%02X"), _ctx->registers[vm_registers::X]);
	_regX.SetWindowTextW(result);
	result.Format(_T("%02X"), _ctx->registers[vm_registers::Y]);
	_regY.SetWindowTextW(result);
	result.Format(_T("%04X"), _ctx->programCounter);
	_programCounter.SetWindowTextW(result);
	result.Format(_T("%02X"), _ctx->sp);
	_stackPointer.SetWindowTextW(result);
	// FIXME: update CPU flags
	result = _T("CZIDBVN\r\n");
	for (int i = 1; i < 8; ++i) {
		if (_internal_ctx->isSet(i)) {
			result.AppendChar('1');
		}
		else {
			result.AppendChar('0');
		}
	}
	_cpuFlags.SetWindowTextW(result);
}

// ----------------------------------------------------
// Compile code from asm code edit box
// ----------------------------------------------------
int CAC64Dlg::compile() {
	CString txt = _T("");
	_asmCode.GetWindowTextW(txt);
	CT2A ascii(txt);
	int nc = 0;
	return vm_assemble(ascii.m_psz);
}

// ----------------------------------------------------
// convert HEX to INT
// ----------------------------------------------------
int CAC64Dlg::hex2int(const CString & txt) {
	CT2A ascii(txt);
	const char* hex = ascii.m_psz;
	if (hex[0] == '0' && hex[1] == 'x') {
		hex += 2;
	}
	int val = 0;
	while ((*hex >= '0' && *hex <= '9') || (*hex >= 'a' && *hex <= 'f') || (*hex >= 'A' && *hex <= 'F')) {
		// get current character then increment
		char byte = *hex++;
		// transform hex character to the 4bit equivalent number, using the ascii table indexes
		if (byte >= '0' && byte <= '9') byte = byte - '0';
		else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
		// shift 4 to make space for new digit, and add the 4 bits of the new digit 
		val = (val << 4) | (byte & 0xF);
	}
	return val;
}

// ----------------------------------------------------
// Run code
// ----------------------------------------------------
void CAC64Dlg::OnBnClickedRun() {
	_statucBarCtrl.SetText(_T(""), 0, 0);
	int n = compile();
	vm_run();
	dumpMemory();
	_statucBarCtrl.SetText(_T("Program executed"), 0, 0);
}

// ----------------------------------------------------
// Previous memory block
// ----------------------------------------------------
void CAC64Dlg::OnBnClickedPreviousMem() {
	_memoryAddress -= 0x100;
	if (_memoryAddress < 0) {
		_memoryAddress = 0;
	}
	CString adr = _T("");
	adr.AppendFormat(_T("0x%04X"), _memoryAddress);
	_memAddress.SetWindowTextW(adr);
	dumpMemory();
}

// ----------------------------------------------------
// Next memory block
// ----------------------------------------------------
void CAC64Dlg::OnBnClickedNextMem() {
	_memoryAddress += 0x100;
	if (_memoryAddress > 0xff00) {
		_memoryAddress = 0xff00;
	}
	CString adr = _T("");
	adr.AppendFormat(_T("0x%04X"), _memoryAddress);
	_memAddress.SetWindowTextW(adr);
	dumpMemory();
}

// ----------------------------------------------------
// Load text file into asm code edit box
// ----------------------------------------------------
void CAC64Dlg::OnBnClickedLoadCode() {
	CFileDialog fileDlg(TRUE, _T("txt"), _T("*.txt"),
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("Text Files(*.txt) | *.txt | All Files(*.*) | *.* || "), this);
	CString filename;
	if (fileDlg.DoModal() == IDOK) {
		filename = fileDlg.GetPathName(); 
		CT2A ascii(filename);
		FILE *fp = fopen(ascii.m_szBuffer, "r");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			int sz = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			char* data = new char[sz + 1];
			fread(data, 1, sz, fp);
			data[sz] = '\0';
			fclose(fp);
			char* p = data;
			CString tmp;
			while (*p != 0) {
				if (*p != '\n') {
					tmp.AppendChar(*p);					
				}
				else {
					tmp.Append(_T("\r\n"));
				}
				++p;
			}
			_asmCode.SetWindowTextW(tmp);
			compile();
			delete[] data;
			_statucBarCtrl.SetText(_T("Program loaded and compiled"), 0, 0);
		}
	}
}

// ----------------------------------------------------
// Reset context
// ----------------------------------------------------
void CAC64Dlg::OnBnClickedResetPc() {
	vm_reset();
	updateCPUState();
	_statucBarCtrl.SetText(_T("Context resetted"), 0, 0);
}

// ----------------------------------------------------
// Single step execution
// ----------------------------------------------------
void CAC64Dlg::OnBnClickedStep() {
	vm_step();
	dumpMemory();
	updateCPUState();
	CString str(_ctx->debug);
	_statucBarCtrl.SetText(str, 0, 0);
}
