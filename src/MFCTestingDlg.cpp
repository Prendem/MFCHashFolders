
// MFCTestingDlg.cpp : implementation file
//

#include "framework.h"
#include "MFCTesting.h"
#include "MFCTestingDlg.h"
#include "afxdialogex.h"
#include "MD5Hasher.hpp"
#include "SHA2_256Hasher.hpp"
#include "SHA3_256Hasher.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCTestingDlg dialog



CMFCTestingDlg::CMFCTestingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCTESTING_DIALOG, pParent)
	, m_sidePaneText(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCTestingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_fileList);
	DDX_Text(pDX, IDC_SIDE_PANE, m_sidePaneText);
}

BEGIN_MESSAGE_MAP(CMFCTestingDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_MAIN_SELECTDIRECTORY, &CMFCTestingDlg::OnMainSelectdirectory)
	ON_NOTIFY(LVN_GETINFOTIP, IDC_LIST1, &CMFCTestingDlg::OnLvnGetInfoTipList1)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CMFCTestingDlg::OnNMClickList1)
	ON_MESSAGE(WM_HASH_COMPLETE, &CMFCTestingDlg::OnHashComplete)
END_MESSAGE_MAP()


// CMFCTestingDlg message handlers

BOOL CMFCTestingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// sets the spacing on the icons in the list view
	m_fileList.SetIconSpacing(CSize(200, 200));

	// loads the image list for list view icons
	m_imageList.Create(48, 48, ILC_COLOR32 | ILC_MASK, 0, 1);
	m_imageList.Add(AfxGetApp()->LoadIconW(IDI_FILE));
	m_imageList.Add(AfxGetApp()->LoadIconW(IDI_FOLDER));
	m_fileList.SetImageList(&m_imageList, LVSIL_NORMAL);

	// enable mouseover tooltips on the list view
	m_fileList.SetExtendedStyle(m_fileList.GetExtendedStyle() | LVS_EX_INFOTIP);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCTestingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCTestingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCTestingDlg::populateListFromPath(std::filesystem::path path)
{
	m_fileList.DeleteAllItems();
	m_fileSet.clear();
	m_threadManager.purgeQueue();
	++m_generation;

	for (const auto& entry : std::filesystem::directory_iterator(path))
		m_fileSet.emplace_back(entry, entry.is_directory());

	std::sort(m_fileSet.begin(), m_fileSet.end(),
		[](const auto& a, const auto& b)
		{
			if (a.isDirectory != b.isDirectory)
				return a.isDirectory;

			return StrCmpLogicalW(a.path.filename().c_str(), b.path.filename().c_str()) < 0;
		});

	for (int index{ 0 }; index < m_fileSet.size(); ++index)
	{
		try
		{
			if (!m_fileSet[index].isDirectory)
			{
				m_fileSet[index].MD5Hash = L"Calculating...";
				m_fileSet[index].SHA2Hash = L"Calculating...";
				m_fileSet[index].SHA3Hash = L"Calculating...";
				std::packaged_task<std::wstring()> MD5{ [p = m_fileSet[index].path]() {return toHexString(computeFileHash<Crypto::MD5Hasher>(p)); } };
				std::packaged_task<std::wstring()> SHA2{ [p = m_fileSet[index].path]() {return toHexString(computeFileHash<Crypto::SHA2_256Hasher>(p)); } };
				std::packaged_task<std::wstring()> SHA3{ [p = m_fileSet[index].path]() {return toHexString(computeFileHash<Crypto::SHA3_256Hasher>(p)); } };
				m_fileSet[index].MD5Job = MD5.get_future();
				m_fileSet[index].SHA2Job = SHA2.get_future();
				m_fileSet[index].SHA3Job = SHA3.get_future();
				m_threadManager.enqueueTask(std::move(MD5), m_hWnd, index, m_generation, HashType::MD5);
				m_threadManager.enqueueTask(std::move(SHA2), m_hWnd, index, m_generation, HashType::SHA2);
				m_threadManager.enqueueTask(std::move(SHA3), m_hWnd, index, m_generation, HashType::SHA3);
			}
		}

		catch (const std::exception& ex)
		{
			MessageBoxA(nullptr, ex.what(), "Error", MB_OK | MB_ICONERROR);
		}

		LVITEM lvItem;
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
		lvItem.iItem = index;
		lvItem.iSubItem = 0;
		CString fileName{ m_fileSet[index].path.filename().c_str()};
		lvItem.pszText = fileName.GetBuffer();
		if (m_fileSet[index].isDirectory)
			lvItem.iImage = 1;
		else
			lvItem.iImage = 0;
		m_fileList.InsertItem(&lvItem);
	}
}

LRESULT CMFCTestingDlg::OnHashComplete(WPARAM wParam, LPARAM lParam)
{
	uint16_t generation{ static_cast<uint16_t>(LOWORD(lParam)) };
	
	if (generation != m_generation)
		return 0; // stale data, ignore

	size_t index{ static_cast<size_t>(wParam) };
	HashType hashType{ static_cast<HashType>(HIWORD(lParam)) };

	switch (hashType)
	{
	case HashType::MD5:
		try
		{
			m_fileSet[index].MD5Hash = m_fileSet[index].MD5Job.get();
		}

		catch(const std::exception& e)
		{
			std::string narrow_str(e.what());
			std::wstring str(narrow_str.begin(), narrow_str.end());
			m_fileSet[index].MD5Hash = L"Error computing hash -> ";
			m_fileSet[index].MD5Hash += str;
		}

		break;
	
	case HashType::SHA2:
		try
		{
			m_fileSet[index].SHA2Hash = m_fileSet[index].SHA2Job.get();
		}

		catch (const std::exception& e)
		{
			m_fileSet[index].SHA2Hash = L"Error computing hash";
		}

		break;

	case HashType::SHA3:
		try
		{
			m_fileSet[index].SHA3Hash = m_fileSet[index].SHA3Job.get();
		}

		catch (const std::exception& e)
		{
			m_fileSet[index].SHA3Hash = L"Error computing hash";
		}

		break;
	}

	int selectedIndex = m_fileList.GetNextItem(-1, LVNI_SELECTED);
	if (selectedIndex == index)
	{
		m_sidePaneText.Empty();

		m_sidePaneText.Append((m_fileSet[index].isDirectory ? L"Directory Name: " : L"File Name: "));
		m_sidePaneText.Append(m_fileSet[index].path.filename().c_str());
		m_sidePaneText.Append(L"\r\nPath: ");
		m_sidePaneText.Append(m_fileSet[index].path.c_str());
		m_sidePaneText.Append(L"\r\n\r\n");

		if (m_fileSet[index].isDirectory)
		{
			m_sidePaneText.Append(L"Double Click to open as a new file set");
			UpdateData(FALSE);
		}

		m_sidePaneText.Append(L"MD5: ");
		m_sidePaneText.Append(m_fileSet[index].MD5Hash.c_str());
		m_sidePaneText.Append(L"\r\nSHA2: ");
		m_sidePaneText.Append(m_fileSet[index].SHA2Hash.c_str());
		m_sidePaneText.Append(L"\r\nSHA3: ");
		m_sidePaneText.Append(m_fileSet[index].SHA3Hash.c_str());

		UpdateData(FALSE);
	}

	return 0;
}


void CMFCTestingDlg::OnMainSelectdirectory()
{
	CFolderPickerDialog dlgFolder;
	if (dlgFolder.DoModal() == IDOK)
	{
		CString strFolderPath = dlgFolder.GetPathName();
		populateListFromPath(strFolderPath.GetString());
	}
}

void CMFCTestingDlg::OnLvnGetInfoTipList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	
	int index = pGetInfoTip->iItem;
	if (index >= 0)
	{
		std::wstring text{ std::format(L"I am index {}", index) };
		CString tooltipText = text.c_str();
		wcsncpy_s(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, tooltipText, _TRUNCATE);
	}

	*pResult = 0;
}

void CMFCTestingDlg::OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	int index = pNMItemActivate->iItem;
	m_sidePaneText.Empty();
	if (index >= 0 && index < m_fileSet.size())
	{
		m_sidePaneText.Append((m_fileSet[index].isDirectory ? L"Directory Name: " : L"File Name: "));
		m_sidePaneText.Append(m_fileSet[index].path.filename().c_str());
		m_sidePaneText.Append(L"\r\nPath: ");
		m_sidePaneText.Append(m_fileSet[index].path.c_str());
		m_sidePaneText.Append(L"\r\n\r\n");
		
		if (m_fileSet[index].isDirectory)
		{
			m_sidePaneText.Append(L"Double Click to open as a new file set");
			UpdateData(FALSE);
			*pResult = 0;
			return;
		}

		m_sidePaneText.Append(L"MD5: ");
		m_sidePaneText.Append(m_fileSet[index].MD5Hash.c_str());
		m_sidePaneText.Append(L"\r\nSHA2: ");
		m_sidePaneText.Append(m_fileSet[index].SHA2Hash.c_str());
		m_sidePaneText.Append(L"\r\nSHA3: ");
		m_sidePaneText.Append(m_fileSet[index].SHA3Hash.c_str());
	}
	UpdateData(FALSE);
	*pResult = 0;
}
