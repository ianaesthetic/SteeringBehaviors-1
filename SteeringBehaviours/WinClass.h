#pragma once

#include "Util.h"
#define ReleaseCOM(x) {if(x) x->Release();}

template <class DERIVED_TYPE>
class BaseWindow
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_TYPE *pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else
		{
			pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	BaseWindow() : m_hwnd(NULL) { }

	BOOL Create(
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		HWND hWndParent = 0,
		HMENU hMenu = 0
	)
	{
		WNDCLASS wc = { 0 };

		wc.lpfnWndProc = DERIVED_TYPE::WindowProc;
		wc.hInstance = GetModuleHandle(NULL);
		wc.lpszClassName = ClassName();

		RegisterClass(&wc);

		m_hwnd = CreateWindowEx(
			dwExStyle, ClassName(), lpWindowName, dwStyle, x, y,
			nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
		);

		return (m_hwnd ? TRUE : FALSE);
	}

	HWND Window() const { return m_hwnd; }

protected:

	virtual PCWSTR  ClassName() const = 0;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	HWND m_hwnd;
};

class Direct2d {
public:

	Direct2d() :
		pFactory(NULL), 
		pRenderTarget(NULL),
		pRect(NULL), 
		pRound(NULL){
		displayCount = 0;
		timeSpan = 0;
		isStop = false;
	}

	~Direct2d() {
		ReleaseCOM(pRenderTarget);
		ReleaseCOM(pFactory);
		for (int i = 0; i < 4; ++i)
			ReleaseCOM(pBrushes[i]);
		//ReleaseCOM(pRect);
		//if (pRound) int x = pRound->Release();
		ReleaseCOM(pTextFormat);
		ReleaseCOM(pTextFactory);
	}

	bool Init(HWND hwnd) {
		HRESULT hr = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			&pFactory
		);
		if (FAILED(hr)) return 0;
		GetClientRect(hwnd, &rect);

		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				hwnd, 
				D2D1::SizeU(
					rect.right - rect.left,
					rect.bottom - rect.top
				)
			),
			&pRenderTarget
		);

		if (FAILED(hr)) return 0;

		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(pTextFactory),
			reinterpret_cast<IUnknown **>(&pTextFactory));
		if (FAILED(hr)) return 0; 

		hr = pTextFactory->CreateTextFormat(
			fontName,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fontSize,
			L"",
			&pTextFormat
		);
		if (FAILED(hr)) return 0;
		pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
		pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), pBrushes);
		if (FAILED(hr)) return 0; 
		hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), pBrushes + 1);
		if (FAILED(hr)) return 0;
		hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), pBrushes + 2);
		if (FAILED(hr)) return 0;
		hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), pBrushes + 3);
		if (FAILED(hr)) return 0;
		return 1;
	}

	void DrawRectangle(FXMVECTOR _p0, FXMVECTOR _p1, FXMVECTOR _p2, FXMVECTOR _p3, int f) {
		XMFLOAT2 p[4];
		XMStoreFloat2(&p[0], _p0);
		XMStoreFloat2(&p[1], _p1);
		XMStoreFloat2(&p[2], _p2);
		XMStoreFloat2(&p[3], _p3);
		for (int i = 0; i < 3; ++i)
			pRenderTarget->DrawLine(
				D2D1::Point2F(p[i].x, p[i].y),
				D2D1::Point2F(p[i + 1].x, p[i + 1].y),
				pBrushes[f]
			);
		pRenderTarget->DrawLine(
			D2D1::Point2F(p[3].x, p[3].y),
			D2D1::Point2F(p[0].x, p[0].y),
			pBrushes[f]
		);
	}
		
	void DrawRound(FXMVECTOR _point, float radius, int f) {
		XMFLOAT2 origin;
		XMStoreFloat2(&origin, _point);
		HRESULT hr; 
		hr = pFactory->CreateEllipseGeometry(
			D2D1::Ellipse(D2D1::Point2F(origin.x, origin.y), radius, radius),
			&pRound
		);
		if (FAILED(hr)) return;
		pRenderTarget->DrawGeometry(pRound, pBrushes[f]);
		pRound->Release();
		//ReleaseCOM(pRound);
	}

	void DrawTriangle(FXMVECTOR _p0, FXMVECTOR _p1, FXMVECTOR _p2, int f) {
		XMFLOAT2 p0, p1, p2; 
		XMStoreFloat2(&p0, _p0);
		XMStoreFloat2(&p1, _p1); 
		XMStoreFloat2(&p2, _p2);
		pRenderTarget->DrawLine(
			D2D1::Point2F(p0.x, p0.y),
			D2D1::Point2F(p1.x, p1.y),
			pBrushes[f]
		);
		pRenderTarget->DrawLine(
			D2D1::Point2F(p1.x, p1.y),
			D2D1::Point2F(p2.x, p2.y),
			pBrushes[f]
		);
		pRenderTarget->DrawLine(
			D2D1::Point2F(p0.x, p0.y),
			D2D1::Point2F(p2.x, p2.y),
			pBrushes[f]
		);
	}
	
	void StringOutput(char* _str, int length, int k) {
		WCHAR str[30]; 
		for (int i = 0; i < length; ++i)
			str[i] = _str[i]; 

		pRenderTarget->DrawTextW(
			str,
			length,
			pTextFormat,
			D2D1::RectF(0, k * fontSize, pRenderTarget->GetSize().width, pRenderTarget->GetSize().height),
			pBrushes[0]
		);
	}

	void DisplayFloat(float x) {
		char s[30];
		sprintf_s(s, "%f", x); 
		StringOutput(s, min(strlen(s), 10), displayCount++);
	}


	/* test */

	void TimeSpanAdd(float dt) {
		timeSpan += dt; 
	}

	void CountReset() {
		if (!isStop) {
			outputCnt = 0;
		}
	}
	void Add(float t) {
		if(!isStop) 
		output[outputCnt ++] = t; 
	}
	void DisplayInfo() {
		if (!isStop) isStop = true; 
		for (int i = 0; i < outputCnt; ++i) DisplayFloat(output[i]);
		if (timeSpan > 0.06f) {
			timeSpan -= 0.06f;
			isStop = false; 
		}
	}

	/* render */ 
	void Begin() {
		displayCount = 0;
		pRenderTarget->BeginDraw();
		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
	}

	void End() {
		pRenderTarget->EndDraw();
	}

protected:
	ID2D1Factory* pFactory; 
	ID2D1HwndRenderTarget* pRenderTarget; 
	ID2D1SolidColorBrush* pBrushes[4];
	ID2D1RectangleGeometry* pRect; 
	ID2D1EllipseGeometry* pRound; 
	RECT rect; 

	IDWriteTextFormat* pTextFormat; 
	IDWriteFactory* pTextFactory; 

	float output[10];
	int outputCnt;
	int displayCount;

	float timeSpan;
	bool isStop;
};
