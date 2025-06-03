
// CxImageTestDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "CxImageTest.h"
#include "CxImageTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CCxImageTestDlg 대화 상자



CCxImageTestDlg::CCxImageTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CXIMAGETEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCxImageTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCxImageTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CCxImageTestDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCxImageTestDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CCxImageTestDlg 메시지 처리기

BOOL CCxImageTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	std::vector<BYTE> vecResource;
	core::ReadFileContents(TEXT("../../Build/Test/XLS.bmp"), vecResource);

	core::ST_BMP_INFO_HEADER info = { 0, };
	memcpy(&info, &vecResource[0], sizeof(info));

	info.Height /= 2;
	
	const DWORD dwHeaderSize = sizeof(info);
	const DWORD dwColorCount = (0x01 << info.BitCount);
	const DWORD dwPalletteSize = 
		(info.BitCount == 1 || info.BitCount == 4 || info.BitCount == 8) ?
		(dwColorCount * 4) : 0;
	const DWORD dwPixelDataSize = info.Width * info.Height * info.BitCount / 8;
	const DWORD dwTotalSize = dwHeaderSize + dwPalletteSize + dwPixelDataSize;
	if (dwTotalSize <= vecResource.size())
	{
		m_Image.CreateFromArray(&vecResource[dwHeaderSize + dwPalletteSize]
			, info.Width, info.Height, info.BitCount, info.Width * info.BitCount / 8, false);

		if (dwPalletteSize)
			m_Image.SetPalette((RGBQUAD*)&vecResource[dwHeaderSize], dwColorCount);
	}

	const DWORD dwTransparentMaskSize = info.Width * info.Height / 8;
	if ((dwTotalSize + dwTransparentMaskSize) <= vecResource.size())
	{
		LPCBYTE pAlphaMask = &vecResource[dwTotalSize];
		m_Image.AlphaCreate();
		for (int y = 0; y < info.Height; y++)
		{
			for (int x = 0; x < info.Width; x++)
			{
				const DWORD dwIndex = x + y * info.Height;
				const DWORD dwByteIndex = dwIndex / 8;
				const DWORD dwBitIndex = 7 - (dwIndex % 8);
				const bool bTransparent = (pAlphaMask[dwByteIndex] & (0x01 << dwBitIndex)) != 0;
				m_Image.AlphaSet(x, y, bTransparent ? 0 : 255);
			}
		}
	}

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CCxImageTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CCxImageTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.
		m_Image.Draw(dc.GetSafeHdc(), 0, 0, -1, -1);

		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CCxImageTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCxImageTestDlg::OnBnClickedOk()
{
	CDialogEx::OnOK();
}


void CCxImageTestDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}
