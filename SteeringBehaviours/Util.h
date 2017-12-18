#pragma once
#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <DirectXMath.h>
#include <dwrite.h>

#include <ctime>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
using namespace DirectX;

#define pi 3.141592f
#define sqrt2 1.414213f
#define sqrt3 1.732050f
#define inf 10000000.0f

static XMVECTORF32 zero = {0.0f, 0.0f, 0.0f, 0.0f};
static int outTime = 0;

static float Repeat(float x, float l, float r) {
	if (l > r) std::swap(l, r); 
	float interval = r - l;
	while (x < l) x += interval;
	while (x > r) x -= interval; 
	return x; 
};

static XMVECTOR randomVector(float ceil) {
	float ret = static_cast<float>(rand() % (int)(2 * ceil * 1000 + 1));
	ret /= 1000;
	ret -= ceil;
	ceil = sqrt(ceil * ceil - ret * ret);
	float other = static_cast<float>(rand() % (int)(2 * ceil * 1000 + 1));
	other /= 1000; 
	other -= ceil;
	return XMVectorSet(ret, other, 0, 0);
}

static XMVECTOR LocalToWorld(FXMVECTOR t, FXMVECTOR x, FXMVECTOR y, CXMVECTOR worldOffset) {
	XMMATRIX transform = {
		XMVectorGetX(x), XMVectorGetY(x), 0, 0, 
		XMVectorGetX(y), XMVectorGetY(y), 0, 0, 
		XMVectorGetX(worldOffset), XMVectorGetY(worldOffset), 1, 0, 
		0, 0, 0, 1
	};
	return XMVector3Transform(t, transform);
}

static XMVECTOR WorldToLocal(FXMVECTOR t, FXMVECTOR x, FXMVECTOR y, CXMVECTOR worldOffset) {
	XMMATRIX rotate, translation; 
	translation = XMMATRIX{
		1, 0, 0, 0,
		0, 1, 0, 0, 
		-XMVectorGetX(worldOffset), -XMVectorGetY(worldOffset), 1, 0, 
		0, 0, 0, 1
	};
	rotate = XMMATRIX(
		XMVectorGetX(x), XMVectorGetX(y), 0, 0, 
		XMVectorGetY(x), XMVectorGetY(y), 0, 0, 
		0, 0, 1, 0, 
		0, 0, 0, 1
	);
	return XMVector3Transform(t, translation * rotate);
};

static float Distance(FXMVECTOR a, FXMVECTOR b) {
	return (XMVectorGetX(b) - XMVectorGetX(a)) * (XMVectorGetX(b) - XMVectorGetX(a)) 
		+ (XMVectorGetY(b) - XMVectorGetY(a)) * (XMVectorGetY(b) - XMVectorGetY(a));
}

static bool InterSect(FXMVECTOR l, FXMVECTOR r, FXMVECTOR o, float radius) {
	FXMVECTOR line = r - l; 
	float t = XMVectorGetX(XMVector2Dot(line, o) - XMVector2Dot(line, l)) / XMVectorGetX(XMVector2Dot(line, line));
	float h;
	if(t >= 0 && t <= 1) h = XMVectorGetX(XMVector2LengthSq(o - l - t * line));
	else if (t < 0) h = XMVectorGetX(XMVector2LengthSq(o - l)); 
	else h = XMVectorGetX(XMVector2LengthSq(o - r));
	if (radius * radius > h) return true; 
	else return false; 
}

const WCHAR fontName[] = L"Verdana";
const float fontSize = 30; 

