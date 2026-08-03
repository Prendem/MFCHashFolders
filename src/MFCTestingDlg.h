
// MFCTestingDlg.h : header file
//

#pragma once

#include <filesystem>
#include <future>
#include <vector>
#include <string>
#include <span>

#include "FileUtil.hpp"
#include "ThreadManager.hpp"

struct DirectoryObject
{
	std::filesystem::path path;
	bool isDirectory;
	std::future<std::wstring> MD5Job;
	std::future<std::wstring> SHA2Job;
	std::future<std::wstring> SHA3Job;
	std::wstring MD5Hash;
	std::wstring SHA2Hash;
	std::wstring SHA3Hash;

	DirectoryObject(const std::filesystem::path& inPath, const bool inDirectory) : path{ inPath }, isDirectory{ inDirectory } {}
};

// CMFCTestingDlg dialog
class CMFCTestingDlg : public CDialogEx
{
// Construction
public:
	CMFCTestingDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCTESTING_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CImageList m_imageList;
	std::vector<DirectoryObject> m_fileSet;
	ThreadManager m_threadManager;
	uint16_t m_generation = 0;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	void populateListFromPath(std::filesystem::path path);
	afx_msg LRESULT OnHashComplete(WPARAM wParam, LPARAM lParam);

public:
	afx_msg void OnMainSelectdirectory();
	CListCtrl m_fileList;
	afx_msg void OnLvnGetInfoTipList1(NMHDR* pNMHDR, LRESULT* pResult);
	CString m_sidePaneText;
	afx_msg void OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult);
};
