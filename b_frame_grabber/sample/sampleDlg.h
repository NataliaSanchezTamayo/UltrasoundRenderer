
#pragma once

#include <initguid.h>
#include <dshow.h>

#include "strmif.h"
#include "usgfw.h"
#include "Usgfw2_h.h"

//--------------------------------------------------------------------------

class ScanConverterCallback : public IUsgfwScanConverterPluginCB
{
public:
	ScanConverterCallback();
	~ScanConverterCallback();
	void SetUsgWnd(HWND wnd, int x, int y);
	void SetPlugin(IUsgfwScanConverterPlugin* plugin);

private:
	IUsgfwScanConverterPlugin* m_plugin;
	HWND m_usg_wnd;
	int m_usg_x;
	int m_usg_y;

public:

// begin: IUnknown
	virtual HRESULT __stdcall QueryInterface( REFIID riid, void **ppObj);
	virtual ULONG STDMETHODCALLTYPE AddRef( void );
	virtual ULONG STDMETHODCALLTYPE Release( void );
	long m_nRefCount;
// end: IUnknown

// begin: IUsgfwScanConverterPluginCB

	// receives pointers to input and output media samples
	STDMETHOD(SampleCB) (THIS_
	IMediaSample *pSampleIn,
	IMediaSample *pSampleOut,
	int nOutX1,
	int nOutY1,
	int nOutX2,
	int nOutY2
	) { return S_OK; }

	// receives pointers to input and output sample buffers
	STDMETHOD(BufferCB) (THIS_
	PBYTE pBufferIn,
	int nInBufferLen,
	PBYTE pBufferOut,
	int nOutBufferLen,
	int nOutX1,
	int nOutY1,
	int nOutX2,
	int nOutY2
	);

	// receives pointers to input and intermediate sample buffers
	STDMETHOD(InInterimBufferCB) (THIS_
	PBYTE pBufferIn,
	int nInBufferLen,
	PBYTE pBufferInterim,
	int nInterimBufferLen,
	int nOutX1,
	int nOutY1,
	int nOutX2,
	int nOutY2
	) { return S_OK; }

	// receves pointers to input media sample and intermediatesample buffer
	STDMETHOD(InInterimSampleCB) (THIS_
	IMediaSample *pSampleIn,
	PBYTE pBufferInterim,
	int nInterimBufferLen,
	int nOutX1,
	int nOutY1,
	int nOutX2,
	int nOutY2
	) { return S_OK; }

	// receives pointers to output and intermediate sample buffers
	STDMETHOD(InterimOutBufferCB) (THIS_
	PBYTE pBufferInterim,
	int nInterimBufferLen,
	PBYTE pBufferOut,
	int nOutBufferLen,
	int nOutX1,
	int nOutY1,
	int nOutX2,
	int nOutY2
	) { return S_OK; }

	// receives pointers to output media sample and intermediate sample buffer
	STDMETHOD(InterimOutSampleCB) (THIS_
	PBYTE pBufferInterim,
	int nInterimBufferLen,
	IMediaSample *pSampleIn,
	int nOutX1,
	int nOutY1,
	int nOutX2,
	int nOutY2
	) { return S_OK; }

	// receives conversion parameter change pin index
	// if parameter is negative parameter was changed by some filter interface
	STDMETHOD(ParameterCB) (THIS_
	int nPin
	) { return S_OK; }

// end: IUsgfwScanConverterPluginCB

}; // class ScanConverterCallback

//--------------------------------------------------------------------------

class CsampleDlg : public CDialog
{
public:
	CsampleDlg(CWnd* pParent = NULL);	// standard constructor
	~CsampleDlg();

// Dialog Data
	enum { IDD = IDD_SAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedFreezeButton();
	afx_msg void OnBnClickedRunButton();

private:
	IUsgfw2* m_usgfw2;
	IUsgDataView* m_data_view;
	IProbe* m_probe;
	IUsgMixerControl* m_mixer_control;
	IUsgScanConverterPlugin* scp;
	IUsgfwScanConverterPlugin* m_plugin;
	ScanConverterCallback* m_scan_converter_callback;

	void CreateUsgControl( IUsgDataView* data_view, const IID& type_id, ULONG scan_mode, ULONG stream_id, void** ctrl );
	void CreateUsgControls();
	void ReleaseUsgControls();

public:
	afx_msg void OnClose();
public:
	afx_msg BOOL OnQueryEndSession();
public:
	afx_msg void OnEndSession(BOOL bEnding);

}; // class CsampleDlg

//--------------------------------------------------------------------------

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

//--------------------------------------------------------------------------
