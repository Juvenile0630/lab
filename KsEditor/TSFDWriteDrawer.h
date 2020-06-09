#pragma once
#include <dwrite.h>
#include <d2d1.h>
#include <memory>
#include <wrl.h>

class Line
{
	Microsoft::WRL::ComPtr<ID2D1Factory> m_d2d_factory;
	Microsoft::WRL::ComPtr<ID2D1StrokeStyle> m_dot_style;
	Microsoft::WRL::ComPtr<ID2D1StrokeStyle> m_dash_style;
private:


public:
	Line(Microsoft::WRL::ComPtr<ID2D1Factory> f) :m_d2d_factory(f) {}
	/*
	solid
	*/
	HRESULT DrawSolidLine(ID2D1RenderTarget* target, D2D1_POINT_2F p0, D2D1_POINT_2F p1, ID2D1Brush* brush, FLOAT storokeWidth = (1.0F)) {
		target->DrawLine(p0, p1, brush, storokeWidth, NULL);
		return S_OK;
	}
	/*
	dot
	*/
	HRESULT DrawDOTLine(ID2D1RenderTarget* target, D2D1_POINT_2F p0, D2D1_POINT_2F p1, ID2D1Brush* brush, FLOAT storokeWidth = (1.0F)) {
		if (!m_dot_style && FAILED(InitDOTLineStyle()))
		{
			return E_FAIL;
		}
		target->DrawLine(p0, p1, brush, storokeWidth, m_dot_style.Get());
		return S_OK;
	}
	HRESULT InitDOTLineStyle() {
		D2D1_STROKE_STYLE_PROPERTIES prop{};
		prop.dashStyle = D2D1_DASH_STYLE_DOT;
		prop.dashCap = D2D1_CAP_STYLE_ROUND;
		auto hr = m_d2d_factory->CreateStrokeStyle(prop, NULL, 0, &m_dot_style);
		return hr;
	}
	/*
	dash
	*/
	HRESULT DrawDashLine(ID2D1RenderTarget* target, D2D1_POINT_2F p0, D2D1_POINT_2F p1, ID2D1Brush* brush, FLOAT storokeWidth = (1.0F)) {
		if (!m_dash_style && FAILED(InitDashLineStyle()))
		{
			return E_FAIL;
		}
		target->DrawLine(p0, p1, brush, storokeWidth, m_dash_style.Get());
		return S_OK;
	}
	HRESULT InitDashLineStyle() {
		D2D1_STROKE_STYLE_PROPERTIES prop{};
		prop.dashStyle = D2D1_DASH_STYLE_DASH;
		prop.dashCap = D2D1_CAP_STYLE_ROUND;
		auto hr = m_d2d_factory->CreateStrokeStyle(prop, NULL, 0, &m_dash_style);
		return hr;
	}
};

enum LineStyle {
		LineStyle_Solid, LineStyle_Dot, LineStyle_Dash, LineStyle_Squiggle
};

struct TsfDWriteDrawerEffectUnderline {
	const LineStyle lineStyle;
	const bool boldLine;
	const Microsoft::WRL::ComPtr<ID2D1Brush> lineColor;
	TsfDWriteDrawerEffectUnderline(const LineStyle ls, const bool bold, ID2D1Brush* b) :lineStyle(ls), boldLine(bold), lineColor(b) {}
};

struct TsfDWriteDrawerEffect :IUnknown {
	const Microsoft::WRL::ComPtr<ID2D1Brush> backgroundColor;
	const Microsoft::WRL::ComPtr<ID2D1Brush> textColor;
	std::unique_ptr<TsfDWriteDrawerEffectUnderline> underline;
	TsfDWriteDrawerEffect(ID2D1Brush* bg, ID2D1Brush* fr, std::unique_ptr<TsfDWriteDrawerEffectUnderline> under) :backgroundColor(bg), textColor(fr), underline(std::move(under)), m_ref_cnt(0) {}
	// IUnknown methods
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
		void** ppvObject) override;
private:
	LONG m_ref_cnt;
};

struct TsfDWriteDrawerContext
{
	ID2D1RenderTarget* renderTarget;
	Microsoft::WRL::ComPtr<TsfDWriteDrawerEffect> dafaultEffect;
	TsfDWriteDrawerContext(ID2D1RenderTarget* t, TsfDWriteDrawerEffect* e) :renderTarget(t), dafaultEffect(e) {}
};

class TsfDWriteDrawer : public IDWriteTextRenderer {
private:
	std::unique_ptr<Line> m_line;
	Microsoft::WRL::ComPtr<ID2D1Factory> m_factory;
	TsfDWriteDrawer(ID2D1Factory* factory) : m_line(std::make_unique<Line>(factory)), m_factory(factory) {}
	ULONG m_ref_cnt;
public:
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
	static void Create(ID2D1Factory* factory, TsfDWriteDrawer** render);
	HRESULT STDMETHODCALLTYPE DrawGlyphRun( void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, DWRITE_MEASURING_MODE measuringMode, const DWRITE_GLYPH_RUN* glyphRun, const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription, IUnknown* clientDrawingEffect ) override;
	HRESULT STDMETHODCALLTYPE DrawUnderline( void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, const DWRITE_UNDERLINE* underline, IUnknown* clientDrawingEffect ) override;
	HRESULT STDMETHODCALLTYPE DrawStrikethrough( void* clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, const DWRITE_STRIKETHROUGH* strikethrough, IUnknown* clientDrawingEffect ) override;
	HRESULT STDMETHODCALLTYPE DrawInlineObject( void* clientDrawingContext, FLOAT originX, FLOAT originY, IDWriteInlineObject* inlineObject, BOOL isSideways, BOOL isRightToLeft, IUnknown* clientDrawingEffect ) override;
	HRESULT STDMETHODCALLTYPE GetCurrentTransform( void* clientDrawingContext, DWRITE_MATRIX* transform ) override;
	HRESULT STDMETHODCALLTYPE GetPixelsPerDip( void* clientDrawingContext, FLOAT* pixelsPerDip ) override;
	HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled( void* clientDrawingContext, BOOL* isDisabled )override;
};
