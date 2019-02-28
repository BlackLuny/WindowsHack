#include "stdafx.h"
#include "ScreenCapture.h"


BOOL PaintMouse(HDC hdc)
{
	HDC bufdc = NULL;
	CURSORINFO cursorInfo = { 0 };
	ICONINFO iconInfo = { 0 };
	HBITMAP bmpOldMask = NULL;

	bufdc = ::CreateCompatibleDC(hdc);
	::RtlZeroMemory(&iconInfo, sizeof(iconInfo));
	cursorInfo.cbSize = sizeof(cursorInfo);
	// ��ȡ�����Ϣ
	::GetCursorInfo(&cursorInfo);
	// ��ȡ���ͼ����Ϣ
	::GetIconInfo(cursorInfo.hCursor, &iconInfo);
	// ���� �׵׺����(AND)
	bmpOldMask = (HBITMAP)::SelectObject(bufdc, iconInfo.hbmMask);     
	::BitBlt(hdc, cursorInfo.ptScreenPos.x, cursorInfo.ptScreenPos.y, 20, 20,
		bufdc, 0, 0, SRCAND);
	// ���� �ڵײ�ɫ���(OR)
	::SelectObject(bufdc, iconInfo.hbmColor);
	::BitBlt(hdc, cursorInfo.ptScreenPos.x, cursorInfo.ptScreenPos.y, 20, 20,
		bufdc, 0, 0, SRCPAINT);

	// �ͷ���Դ
	::SelectObject(bufdc, bmpOldMask);
	::DeleteObject(iconInfo.hbmColor);
	::DeleteObject(iconInfo.hbmMask);
	::DeleteDC(bufdc);
	return TRUE;
}


HBITMAP GetScreenBmp()
{
	// ��ȡ��Ļ����
	// ��ȡ���洰�ھ��
	HWND hDesktop = ::GetDesktopWindow();
	// ��ȡ���洰��DC
	HDC hdc = ::GetDC(hDesktop);
	// ��������DC
	HDC mdc = ::CreateCompatibleDC(hdc);
	// ��ȡ�������Ļ�Ŀ�͸�
	DWORD dwWidth = ::GetSystemMetrics(SM_CXSCREEN);
	DWORD dwHeight = ::GetSystemMetrics(SM_CYSCREEN);
	// ��������λͼ
	HBITMAP bmp = ::CreateCompatibleBitmap(hdc, dwWidth, dwHeight);
	// ѡ��λͼ
	HBITMAP holdbmp = (HBITMAP)::SelectObject(mdc, bmp);
	// ���������ݻ��Ƶ�λͼ��
	::BitBlt(mdc, 0, 0, dwWidth, dwHeight, hdc, 0, 0, SRCCOPY);

	// �������
	PaintMouse(mdc);

	::SelectObject(mdc, holdbmp);

	// �ͷ��ڴ�
	::DeleteDC(mdc);
	::ReleaseDC(hDesktop, hdc);

	return bmp;
}


BOOL ScreenCapture()
{
	// ��ȡ��Ļ����λͼ���
	HBITMAP hBmp = GetScreenBmp();
	if (NULL == hBmp)
	{
		return FALSE;
	}
	// ����ΪͼƬ
	SaveBmp(hBmp);

	// �ͷ�λͼ���
	::DeleteObject(hBmp);

	return TRUE;
}