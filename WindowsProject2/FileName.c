/*

��ü �ڵ� ���� �ۼ�(������ �����Լ�, ������ ���ν���, ���̾�α� ���ν���)

����� �ڵ� ���� ����(����ȭ) ���� ���Դϴ�.


WinMain()
{
	������ ����ü ���� �� ��� : WNDclassEx() ��� ���� ��
								 RegisterClassEx()���� ���

	������ ����/ ��� : HWND hWnd�� ������ ���� ��Ÿ�� �� �ڵ� ����
						ShowWindow�� �޸𸮿� �����ϴ� ��
						UpdateWindow�� ��ũ���� ����ϴ� ��

	�޼��� ����(�ݺ�? -> �� �� ���� ó���� �ϰ� ���ĺ��� �̰͸� �ؼ� �׷� ��)
					  : GetMassage()�Լ��� WndProc�� ����

}
WinProc()
{
	�޼��� ó�� ����
	switch(message)
	{
	case WM_...:
		break;
	case WM_...:
		break

	default:
		DefWndProc(); (�׻� ����Ʈ�� �� �־�� ��)
	}

}
**
	�޼��� ������ WndProc()�� ����
 GetMssage(&msg,NULL,0,0)
 {
	TranslateMessage(&msg);
	DispatchMessge(&msg);-> �޼����� WndProc�� ����-> WndProc���� �޼��� ���� �� ��ȯ
 }
 GetMessage()-> DispatchMessage()-> WnProc() -> CALLBACK(?)


 WndProc() /DlgProc() : WndProc()���� DlgProc()�� ȣ���Ͽ� ��� ����

*/

//���� ������ : ���Ǵ��б� ���޻�� ��Ȳ�� ��µǴ� ������
//���̾�α�1 : �ܰ����� ���� ������
//���̾�α�2 : ���޻�� ������ ǥ�õǴ� ������(���� ���� ��� ��� X)

#include <Windows.h>
#include <wchar.h> //�����ڵ� �� �� �ʿ�
#include "Resource.h" //�ۼ� ��α� 20211827
#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <locale.h>

//�����ͺ��̽� ���� �κ�
#define HOST "ssu-info.civydey9zqjl.us-east-2.rds.amazonaws.com"
#define USER "ssu_01"
#define PASSWORD "soongsil1"
#define DATABASE "ssu_info"

MYSQL_RES* res;

//�Լ� ���� (������Ÿ��)
void MyConvertToWideString(const char* narrowString, wchar_t* wideString, size_t size);
MYSQL* ConnectToDatabase();
void PopulateListBox(HWND hwndListBox);
void AddDataToListBox(HWND hwndListBox, const wchar_t* data);
void finish_with_error(MYSQL* con);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); 
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam); //�ۼ� ��α� 20211726
INT_PTR CALLBACK DlgProc2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam); 

//�������� ����

HINSTANCE hInst;//�ν��Ͻ� �ڵ�
HWND hWnd;//���� ������ �ڵ�
HWND g_hModelessWnd; //�ܰ� ���� ���� ������ �ڵ�
HWND childWindow;// ��ư Ŭ�� �� ������ ������ �ڵ�
HWND versionWnd;//�޴����� ������ ���� �� ������ ������ �ڵ�


//���� �Լ�
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow
)
{

	//InitializeMySQLLibrary();
	//1. ������ Ŭ���� (������ ����ü) ���� [���� ������]
	WNDCLASSEX wcex; //WNDCLASSEX�� �� �� ������ ������ Ŭ������ pure C������ ��밡��
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;//������ �̵� �Ǵ� �ø��� ���̱� ����
	wcex.lpfnWndProc = WndProc; //�Լ� ������ -> �Լ� ��ü�� ����Ŵ
	wcex.cbClsExtra = 0;//���� ��� X -> ��κ� 0���� �ʱ�ȭ
	wcex.cbWndExtra = 0;//���� ��� X -> ��κ� 0���� �ʱ�ȭ
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));//��ũ�� �Լ� ���ҽ� ���̵� ������ ��ȯ�Ͽ� ���(�ڵ�)
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //�� ����
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); //�޴�1 ���ҽ� ���
	wcex.lpszClassName = L"Basic"; //Ŭ���� �̸�: �Ʒ� ������ ���� ��(CreateWindow) �����ϱ⸸ �ϸ� ū ��� X
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON2)); //���� ����ȭ ��

	//2. ������ Ŭ���� (������ ����ü) ���
	RegisterClassEx(&wcex);

	//3. ������ ����
	hWnd = CreateWindow(L"Basic", L"SSU",//����ü��, Ÿ��Ʋ��(������ â ��)
		WS_OVERLAPPEDWINDOW,//������ ����
		CW_USEDEFAULT,//��� X ��ǥ �����찡 ��Ÿ���� ��ġ
		CW_USEDEFAULT,//��� Y ��ǥ �����찡 ��Ÿ���� ��ġ
		400, 300, NULL, NULL,//������ �ʺ�, ����, �θ� ������, �޴��� �ڵ�
		hInstance, NULL);//�ν��Ͻ�, ����(NULL)

	//4. ������ ���(�޸� �� ��ũ��)
	ShowWindow(hWnd, nCmdShow); //WM_PAINT�� ���õ� 
	UpdateWindow(hWnd);

	//5. �޼��� ����
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))//����ȭ �Ǿ��ִ� ����
	{
		TranslateMessage(&msg);//���� Ű �Ǵ� �Է¿� ���� �޼��� ��ȯ
		DispatchMessage(&msg); //WndProc()�Լ� ȣ�� ������� ��� WndProc()�� �޼��� ���� 
	}
	return 0; //Winmain()�� ����� �� �޼��� ������ ���� ����� 
}
//������ ���ν��� (��ü ������ �Լ� �޼��� ó��)
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)//�޼����� ���� case ����
	{
	case WM_CREATE: //���α׷� ���� �� �ٷ� ���̾�αװ� ��Ÿ��
		g_hModelessWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, DlgProc); //���̾�α� ���ν��� �Լ� ���� (resource �� IDD_DIALOG1�� ������) 
		if (g_hModelessWnd == NULL) //���̾�α� ���� ���� ���� �߻� ��
		{
			MessageBox(hWnd, L"Dialog creation failed!", L"Error", MB_OK | MB_ICONERROR);
			return -1;
		}
		MYSQL* conn = ConnectToDatabase();
		if (conn == NULL)
		{
			// �����ͺ��̽� ���� ���� ó��
			DestroyWindow(hWnd);
			return -1;
		}
	case WM_COMMAND:// �޴� ����(���α׷� ���� �� ���� ������ ��� �� �ٷ� �Ʒ� ��) [����] -> [����],[����]
	{
		wmId = LOWORD(wParam);
		switch (wmId) // ���� �ʿ�
		{
		case IDOK:
			DestroyWindow(hWnd);
			break;
		case ID_32771:// **���� �ʿ� ���� ���ŵ� �������� ������ ���� ����** ���� ������ �޴� ��[����]-> [����]
			MessageBox(hWnd, L"�ֱ� ������: 2023 11 20", L"Version", MB_OK | MB_ICONINFORMATION);
			break;
		case ID_32772: //���� ������ �޴� ��[����]-> [����]
			g_hModelessWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, DlgProc); //���̾�α� ���ν��� �Լ� ���� (resource �� IDD_DIALOG1�� ������) 
			if (g_hModelessWnd == NULL) //���̾�α� ���� ���� ���� �߻� ��
			{
				MessageBox(hWnd, L"Dialog creation failed!", L"Error", MB_OK | MB_ICONERROR);
				return -1;
			}
			break;
		case ID_32773:
			DestroyWindow(g_hModelessWnd); //���̾�α� ����
			DestroyWindow(hWnd); //������ ���� ==  ���α׷� ����
			break;

		default: //���� �۾��� ���� 
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}

	case WM_PAINT: //�����쿡 �׷��� ��� 
		hdc = BeginPaint(hWnd, &ps);
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		DrawText(hdc, L"���Ǵ��б� ���޻�� ��Ȳ", -1, &clientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY: //�����ٴ� �޼��� ���ν����� ����
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		DestroyWindow(g_hModelessWnd);
		DestroyWindow(hWnd);
		break;
	default: //���� �۾��� ���� 
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	//DeinitializeMySQLLibrary();
	return 0;
}

//���̾�α� ���ν���
//�ۼ� ��α� 20211726

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)//�ۼ� ��α� 20211726
{
	UNREFERENCED_PARAMETER(lParam); //�⺻������ ���� ��
	HDC hdc; //dc�ڵ� �׷��� �۾� �ҿ� �� �ʿ��� �ڵ�
	PAINTSTRUCT ps;//WM_PAINT�� �ʿ��� ����ü

	switch (message)//case�� ���� ���� 
	{

	case WM_CLOSE:
		DestroyWindow(hDlg);//���̾�α� ����
		return (INT_PTR)TRUE;

	case IDOK: //Ȯ�� ��ư = IDOK (���� �ʿ�?)
		EndDialog(hDlg, 0);
		PostQuitMessage(0);
		return (INT_PTR)TRUE;

	case WM_COMMAND: //��� case �з�
		switch (LOWORD(wParam))//�⺻������ ���� ��
		{
		case IDC_BUTTON1: //�������� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, DlgProc2); //DlgProc ���� �����Ǵ� ���̾�α��� �ڵ� childWindow�� ����
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
			return (INT_PTR)TRUE;
		case IDC_BUTTON2:  //IT���� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), childWindow, DlgProc2);
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
		case IDC_BUTTON3: //�ڿ����д��� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), childWindow, DlgProc2); 
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
		case IDC_BUTTON4: //��Ư�������� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), childWindow, DlgProc2); 
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
		case IDC_BUTTON5: //�ݵ�ü���� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), childWindow, DlgProc2); 
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
			return (INT_PTR)TRUE;
		case IDC_BUTTON6: //����� ������� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), childWindow, DlgProc2);
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
			return (INT_PTR)TRUE;
		case IDC_BUTTON7: //�ι����� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), childWindow, DlgProc2); 
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
			return (INT_PTR)TRUE;
		case IDC_BUTTON8: //�������� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), childWindow, DlgProc2);
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
			return (INT_PTR)TRUE;
		case IDC_BUTTON9: //��ȸ���д��� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), childWindow, DlgProc2); 
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
			return (INT_PTR)TRUE;
		case IDC_BUTTON10: //���������� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), childWindow, DlgProc2); 
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
			return (INT_PTR)TRUE;
		case IDC_BUTTON11: //�濵���� ��ư ID
			childWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG2), childWindow, DlgProc2); 
			if (childWindow != NULL) {
				// ���� ȭ�� ũ�� �� �۾� ���� ũ�� ��������
				RECT desktopRect, workAreaRect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
				GetWindowRect(GetDesktopWindow(), &desktopRect);

				// ���̾�α� ũ�� ��������
				RECT dialogRect;
				GetWindowRect(childWindow, &dialogRect);
				int dialogWidth = dialogRect.right - dialogRect.left;
				int dialogHeight = dialogRect.bottom - dialogRect.top;

				// ȭ�� �߾ӿ� ��ġ ���
				int centerX = (desktopRect.right - dialogWidth) / 2;
				int centerY = (desktopRect.bottom - dialogHeight) / 2;

				// ���̾�α׸� ȭ�� �߾����� �̵�
				SetWindowPos(childWindow, NULL, centerX, centerY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

				// ���̾�α׸� ������
				ShowWindow(childWindow, SW_SHOWNORMAL);
			}
			else {
				MessageBox(hWnd, L"Dialog creation failed", L"Error", MB_OK | MB_ICONERROR);
			}
			return (INT_PTR)TRUE;
		case IDOK://Ȯ�� ��ư
			EndDialog(hDlg, IDOK); //���̾�α� ����
			break;
		}break;
	default: FALSE;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK DlgProc2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_CLOSE:
		DestroyWindow(hDlg);
		return (INT_PTR)TRUE;
	//case WM_PAINT:
		//hdc = BeginPaint(hDlg, &ps);
		//EndPaint(hDlg, &ps);
		//return (INT_PTR)TRUE;
	case IDOK:
		EndDialog(hDlg, 0);
		return (INT_PTR)TRUE;
	case WM_INITDIALOG:
		// ���⿡ ����Ʈ �ڽ� �ʱ�ȭ �ڵ� �߰�
		HWND hwndListBox = GetDlgItem(hDlg, IDC_LIST1);  // IDC_LIST2�� ����Ʈ �ڽ��� ID�Դϴ�.
		if (hwndListBox != NULL)
		{
			// ListBox �ʱ�ȭ
			SendMessage(hwndListBox, LB_RESETCONTENT, 0, 0);

			// ListBox�� ä��� �Լ� ȣ��
			PopulateListBox(hwndListBox);
		}
		return (INT_PTR)TRUE;
		break;
	}

	return (INT_PTR)FALSE;
}



void AddDataToListBox(HWND hwndListBox, const wchar_t* data)
{
	SendMessage(hwndListBox, LB_ADDSTRING, 0, (LPARAM)data);
}


void InitializeMySQLLibrary() {
	if (mysql_library_init(0, NULL, NULL)) {
		fprintf(stderr, "mysql_library_init() failed\n");
		exit(1);
	}
}

void DeinitializeMySQLLibrary() {
	mysql_library_end();
}

// MySQL ���� �Լ�
MYSQL* ConnectToDatabase() {
	setlocale(LC_ALL, "en_US.UTF-8");
	MYSQL* conn = mysql_init(NULL);
	if (conn == NULL) {
		fprintf(stderr, "mysql_init() failed\n");
		return NULL;
	}

	if (mysql_real_connect(conn, "ssu-info.civydey9zqjl.us-east-2.rds.amazonaws.com", "ssu_01", "soongsil1", "ssu_info", 3306, NULL, 0) == NULL) {
		finish_with_error(conn);
		return NULL;
	}
	return conn;
}

// �����ͺ��̽����� ������ �������� �� ����Ʈ �ڽ��� �߰��ϴ� �Լ�
void PopulateListBox(HWND hwndListBox) {
	MYSQL* conn = ConnectToDatabase();
	if (conn != NULL) {
		// �����ͺ��̽����� ������ �������� ����
		if (mysql_query(conn, "SELECT * FROM categories")) {
			finish_with_error(conn);
		}

		res = mysql_store_result(conn);

		if (res == NULL) {
			finish_with_error(conn);
		}

		int num_fields = mysql_num_fields(res);
		MYSQL_ROW row;

		while ((row = mysql_fetch_row(res))) {
					// �� ���� id�� name�� �����ͼ� ���
					wchar_t wideId[1024];  // ������ ���̷� �����ϼ���.
					wchar_t wideName[1024];  // ������ ���̷� �����ϼ���.

					MyConvertToWideString(row[0] ? row[0] : "NULL", wideId, sizeof(wideId) / sizeof(wchar_t));
					MyConvertToWideString(row[1] ? row[1] : "NULL", wideName, sizeof(wideName) / sizeof(wchar_t));

					// �� ������ ���
					wchar_t wideRow[2024];  // id�� name�� ���ļ� ����� ����
					swprintf(wideRow, sizeof(wideRow) / sizeof(wchar_t), L"%s %s", wideId, wideName);

					// ���
					SendMessage(hwndListBox, LB_INSERTSTRING, -1, (LPARAM)wideRow);
			}
			mysql_free_result(res);
	}
	mysql_close(conn);
}

void finish_with_error(MYSQL* con)
{
	fprintf(stderr, "MySQL error: %s\n", mysql_error(con));
	mysql_close(con);
	exit(1);
}
void MyConvertToWideString(const char* narrowString, wchar_t* wideString, size_t size) {
	MultiByteToWideChar(CP_UTF8, 0, narrowString, -1, wideString, size);
}