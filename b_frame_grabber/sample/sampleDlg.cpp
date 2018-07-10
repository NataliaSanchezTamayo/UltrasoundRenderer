
#pragma once

#include "stdafx.h"
#include "sample.h"
#include "sampleDlg.h"

#include "DIBSectn.h"

#include <mtype.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//--------------------------------------------------------------------------

void CsampleDlg::CreateUsgControl( IUsgDataView* data_view, const IID& type_id, ULONG scan_mode, ULONG stream_id, void** ctrl )
{
	IUsgControl* ctrl2;
	ctrl2 = NULL;
	if (data_view == NULL) return;
	data_view->GetControlObj( &type_id, scan_mode, stream_id, &ctrl2 );
	if (ctrl2 != NULL)
	{
		HRESULT hr;
		hr = ctrl2->QueryInterface( type_id, (void**)ctrl );
		if (hr != S_OK)
			*ctrl = NULL;
		SAFE_RELEASE(ctrl2);
	}
}

//--------------------------------------------------------------------------

void CsampleDlg::CreateUsgControls()
{
	HRESULT hr = S_OK;
	IUnknown* tmp_obj = NULL;

	do
	{
		// create main Usgfw2 library object
		hr = CoCreateInstance(CLSID_Usgfw2, NULL, CLSCTX_INPROC_SERVER, IID_IUsgfw2,(LPVOID*) &m_usgfw2);
		if (hr != S_OK)
		{
			m_usgfw2 = NULL;
			break;
		}

		IUsgCollection* probes_collection = NULL;
		tmp_obj = NULL;
		// get collection of connected (available) probes
		m_usgfw2->get_ProbesCollection(&tmp_obj);
		if (tmp_obj == NULL)
		{
			probes_collection = NULL;
			break;
		}
		hr = tmp_obj->QueryInterface(IID_IUsgCollection,(void**)&probes_collection);
		SAFE_RELEASE(tmp_obj);
		if ( (hr != S_OK) || (probes_collection == NULL) )
		{
			probes_collection = NULL;
			break;
		}

		LONG probes_count = 0;
		// get the number of connected (available) probes
		probes_collection->get_Count(&probes_count);
		if (probes_count == 0)
		{
			SAFE_RELEASE(probes_collection);
			break;
		}

		tmp_obj = NULL;
		// get first available probe
		probes_collection->Item(0,&tmp_obj);
		SAFE_RELEASE(probes_collection);
		if (tmp_obj == NULL)
		{
			m_probe = NULL;
			break;
		}
		hr = tmp_obj->QueryInterface(IID_IProbe,(void**)&m_probe);
		SAFE_RELEASE(tmp_obj);
		if ( (hr != S_OK) || (m_probe == NULL) )
		{
			m_probe = NULL;
			break;
		}

		// create main ultrasound scanning object (data view) for selected probe
		m_usgfw2->CreateDataView(m_probe, &m_data_view);
		if (m_data_view == NULL)
			break;

		// stop ultrasound scanning
		m_data_view->put_ScanState(SCAN_STATE_STOP);

		IUsgScanMode* mode = NULL;
		m_data_view->GetScanModeObj(SCAN_MODE_B,&mode);
		if (mode == NULL)
			break;
		// get mixer control for B mode
		mode->GetMixerControl(SCAN_MODE_B,0,&m_mixer_control);
		SAFE_RELEASE(mode);
		if (m_mixer_control == NULL)
			break;

		// set B scanning mode
		m_data_view->put_ScanMode(SCAN_MODE_B);

		// set ultrasound output window;
		// if instead of Usgfw2 ultrasound output we want to implement some custom output,
		// then Usgfw2 ultrasound output can be redirected to some hidden window;
		// in any case, software must set both output rectangle and output window hwnd, because
		// they are required for scan-converter programming and streaming to work;
		m_mixer_control->SetOutputWindow((LONG)((HWND)(*GetDlgItem(IDC_HIDDEN_WND))));
		
		tagRECT rect1;
		rect1.left		= 0;
		rect1.top		= 0;
		rect1.right		= rect1.left + 512;
		rect1.bottom	= rect1.top + 512;
		// set ultrasound output rectangle; output rectangle can have any size;
		// scan-converter will automatically create ultrasound image using correct aspect ratio;
		// selected scanning depth will be fit to the height of output rectangle;
		// ultrasound image width will be adjusted proportionally and, if required, the image will be cropped;
		// the image of requested size is calculated NOT by resizing some fixed output image, but
		// during scan-conversion and interpolation process when from scan-lines is created
		// 2D image using information about ultrasound probe geometry;
		// the number of points in one B mode ultrasound beam (scan-line) varies from ~700 to ~1024.
		// the number of beams (scan lines) in one B mode frame may be between 64 and 128
		// depending on the used ultrasound scanner, probe, lines density;
		m_mixer_control->SetOutputRect(&rect1);

		tagPALETTEENTRY clr1;
		clr1.peRed		= 0;
		clr1.peGreen	= 0;
		clr1.peBlue		= 255;
		clr1.peFlags	= 0;
		// set background color that surrounds ultrasound image
		m_mixer_control->put_BkColor(clr1);

		tmp_obj = NULL;
		CreateUsgControl( m_data_view, IID_IUsgGain, SCAN_MODE_B, 0, (void**)&tmp_obj );
		if (tmp_obj != NULL)
		{
			// set some gain value
			((IUsgGain*)tmp_obj)->put_Current(100);
			SAFE_RELEASE(tmp_obj);
		}

		// create scan-converter callback COM object that can be implemented as class like in
		// this sample or can be implemented as a usual COM object that is created using CoCreateInstance
		m_scan_converter_callback = new ScanConverterCallback();
		m_scan_converter_callback->AddRef();
		// set output window where callback will display received ultrasound frames
		m_scan_converter_callback->SetUsgWnd((HWND)(*this), 20, 20);

		// get IUsgScanConverterPlugin for desired ultrasound data *stream*:
		// SCAN_MODE_B, SCAN_MODE_M, SCAN_MODE_PW, SCAN_MODE_CFM, SCAN_MODE_PDI, SCAN_MODE_DPDI
		IUnknown* tmp_obj = NULL;
		CreateUsgControl(m_data_view, IID_IUsgScanConverterPlugin, SCAN_MODE_B, 0, (void**)&tmp_obj);
		if (tmp_obj != NULL)
			scp = (IUsgScanConverterPlugin*)tmp_obj;

		// get IUsgfwScanConverterPlugin interface
		IUnknown* tmp_unk = NULL;
		m_plugin = NULL;
		if (scp != NULL)
			scp->get_ScanConverter(&tmp_unk);
		if (tmp_unk != NULL)
		{
			hr = tmp_unk->QueryInterface(IID_IUsgfwScanConverterPlugin,(void**)&m_plugin);
			if (hr != S_OK)
				m_plugin = NULL;
			SAFE_RELEASE(tmp_unk);
		}

		// register COM callback object and specify method(s) to callback
		if ( (m_plugin != NULL) && (m_scan_converter_callback != NULL) )
		{
			hr = m_plugin->SetCallback(m_scan_converter_callback, USPC_BUFFER_INPUT_OUTPUT);
			m_scan_converter_callback->SetPlugin(m_plugin);
		}

		// start ultrasound scanning (Run)
		m_data_view->put_ScanState(SCAN_STATE_RUN);

		return;
	} while (false);

	ReleaseUsgControls();

} // void CsampleDlg::CreateUsgControls()

//--------------------------------------------------------------------------

void CsampleDlg::ReleaseUsgControls()
{
	if (m_data_view != NULL)
		m_data_view->put_ScanState(SCAN_STATE_STOP);
	if (m_plugin != NULL)
		m_plugin->SetCallback(NULL, USPC_BUFFER_INPUT_OUTPUT); // unregister callback
	SAFE_RELEASE(m_scan_converter_callback);
	SAFE_RELEASE(m_plugin);
	SAFE_RELEASE(scp);
	SAFE_RELEASE(m_mixer_control);
	SAFE_RELEASE(m_data_view);
	SAFE_RELEASE(m_probe);
	SAFE_RELEASE(m_usgfw2);
} // void CsampleDlg::ReleaseUsgControls()

//--------------------------------------------------------------------------

CsampleDlg::CsampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CsampleDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//------------------------------

void CsampleDlg::OnClose()
{
	ReleaseUsgControls();

	__super::OnClose();
}

//------------------------------

CsampleDlg::~CsampleDlg()
{
	CoUninitialize();
}

//------------------------------

void CsampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

//------------------------------

BEGIN_MESSAGE_MAP(CsampleDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_FREEZE_BUTTON, &CsampleDlg::OnBnClickedFreezeButton)
	ON_BN_CLICKED(IDC_RUN_BUTTON, &CsampleDlg::OnBnClickedRunButton)
	ON_WM_CLOSE()
	ON_WM_QUERYENDSESSION()
	ON_WM_ENDSESSION()
END_MESSAGE_MAP()

//------------------------------

BOOL CsampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	CoInitialize(NULL);

	m_usgfw2		= NULL;
	m_data_view		= NULL;
	m_probe			= NULL;
	m_mixer_control = NULL;
	m_plugin		= NULL;
	scp				= NULL;
	m_scan_converter_callback = NULL;

	CreateUsgControls();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//------------------------------

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CsampleDlg::OnPaint()
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
		CDialog::OnPaint();
	}

}

//------------------------------

// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR CsampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//--------------------------------------------------------------------------

void CsampleDlg::OnBnClickedFreezeButton()
{
	if (m_data_view == NULL) return;
	m_data_view->put_ScanState(SCAN_STATE_FREEZE);
}

//------------------------------

void CsampleDlg::OnBnClickedRunButton()
{
	if (m_data_view == NULL) return;
	m_data_view->put_ScanState(SCAN_STATE_RUN);
}

//--------------------------------------------------------------------------

BOOL CsampleDlg::OnQueryEndSession()
{
	if (!__super::OnQueryEndSession())
		return FALSE;

	return TRUE;
}

//------------------------------

void CsampleDlg::OnEndSession(BOOL bEnding)
{
	__super::OnEndSession(bEnding);

	ReleaseUsgControls();
}

//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------

HRESULT __stdcall ScanConverterCallback::QueryInterface( REFIID riid, void **ppObj )
{
	if ( riid == IID_IUnknown || riid == IID_IUsgfwScanConverterPluginCB)
	{
		*ppObj = (void *)(this); 
		AddRef();
		return S_OK;
	}
	*ppObj = NULL;
	return E_NOINTERFACE ;
}

//------------------------------

ULONG STDMETHODCALLTYPE ScanConverterCallback::AddRef( void )
{
	return InterlockedIncrement(&m_nRefCount);
}

//------------------------------

ULONG STDMETHODCALLTYPE ScanConverterCallback::Release( void )
{
	long nRefCount = 0;
	nRefCount = InterlockedDecrement(&m_nRefCount) ;
	if (nRefCount == 0)
		delete this;
	return nRefCount;
}

//------------------------------

ScanConverterCallback::ScanConverterCallback()
{
	m_nRefCount = 0;
	m_plugin = NULL;
	m_usg_wnd = NULL;
	m_usg_x = 0;
	m_usg_y = 0;
}

//------------------------------

ScanConverterCallback::~ScanConverterCallback()
{
}

//------------------------------

void ScanConverterCallback::SetUsgWnd(HWND wnd, int x, int y)
{
	m_usg_wnd = wnd;
	m_usg_x = x;
	m_usg_y = y;
}

//------------------------------

void ScanConverterCallback::SetPlugin(IUsgfwScanConverterPlugin* plugin)
{
	m_plugin = plugin;
}

//------------------------------

// Return the number of bytes used to store a scanline for the DIB.
// Rounded up to the nearest 32-bit boundary.
// ScanBytes(infoHeader.biWidth, infoHeader.biBitCount)
int ScanBytes(long w, int bpp)
{
	return (int)(((w*bpp+31)&(~31))/8);
}

HRESULT ScanConverterCallback::BufferCB (THIS_
	PBYTE pBufferIn,
	int nInBufferLen,
	PBYTE pBufferOut,
	int nOutBufferLen,
	int nOutX1,
	int nOutY1,
	int nOutX2,
	int nOutY2
	)
{
	if ( (m_plugin == NULL) || (m_usg_wnd == NULL) || (pBufferOut == NULL) || (nOutBufferLen == NULL) )
		return S_OK;

	AM_MEDIA_TYPE mtOut;
	VIDEOINFO *pVideoInfo = NULL;

	if (m_plugin->GetConnectedMediaType(PINDIR_OUTPUT, &mtOut) != S_OK)
		return S_OK;

	// get output image format
	pVideoInfo = (VIDEOINFO*)mtOut.pbFormat;
	if (pVideoInfo == NULL)
	{
		FreeMediaType(mtOut);
		return S_OK;
	}

	int output_width, output_height, output_bits_per_pixel, output_pitch;
	output_width			= pVideoInfo->bmiHeader.biWidth;
	output_height			= abs(pVideoInfo->bmiHeader.biHeight);
	output_bits_per_pixel	= pVideoInfo->bmiHeader.biBitCount;
	output_pitch			= ScanBytes(output_width, output_bits_per_pixel);

	if ( IsEqualGUID(mtOut.subtype, MEDIASUBTYPE_RGB555) )
		output_bits_per_pixel = 15;

	FreeMediaType(mtOut);

	// begin: custom output of received frame data

	HBITMAP bmp;
	bmp = DSCreateDIBSection(output_width, output_height, output_bits_per_pixel, NULL, 0);

	LPBYTE src;
	LPBYTE dst;
	src = pBufferOut;
	dst = DSGetPointerToDIBSectionImageBits(bmp);

	// VARIABLE TO SAVE BITMAP TO DESKTOP FILE CALLED BITMAP1.BMP
	LPTSTR FileDir = TEXT("C:\\Users\\natal\\Desktop\\Bitmap1.bmp");

	// flip image up/down and copy ultrasound frame bits to DIB section
	src += (output_pitch*(output_height-1));
	for (int i1=0; i1<output_height; i1++)
	{
		memcpy(dst,src,output_pitch);
		dst += output_pitch;
		src -= output_pitch;
	}
	src = NULL;
	dst = NULL;

	HDC dc;
	dc = ::GetDC(m_usg_wnd);

	RECT r;
	r.left = m_usg_x;
	r.top = m_usg_y;
	r.right = r.left + output_width + 1;
	r.bottom = r.top + output_height + 1;
	DSDrawDIBSectionOnDC(dc, bmp, &r);

	//saves the bitmap to a file in the desktop called Bitmap1.bmp
	DSStoreDIBSectionInBMPFile(FileDir, bmp);

	::ReleaseDC(m_usg_wnd, dc);
	::DeleteObject(bmp);

	// end: custom output of received frame data

	return S_OK;
} // HRESULT ScanConverterCallback::BufferCB

//----------------------------------------------------------------
