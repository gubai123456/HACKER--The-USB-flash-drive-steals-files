#include<windows.h>
#include<Dbt.h>//
#include<string.h>
#include <direct.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//从逻辑单元获取盘符名称
char FirstDriveFromMask(ULONG unitMask);
//copy file
DWORD WINAPI CopyProc(LPVOID lPvoid);
//复制
void Copy(TCHAR* szRoot,TCHAR* szDest);

//U盘根目录
TCHAR g_szRoot[MAX_PATH];
//U盘复制目的地
TCHAR g_szDest[MAX_PATH]=	TEXT("E:/COPY");

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, PSTR szCmdLine, int iCmdShow)
{
	//做一个窗口
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
		switch (wParam)//设备状态存在这里
		{
		case DBT_DEVICEARRIVAL:
			lpdb = (DEV_BROADCAST_HDR*)lParam;
			lpdbv = (DEV_BROADCAST_VOLUME*)lpdb;

		 //获取盘符名称
			 ch=FirstDriveFromMask(lpdbv->dbcv_unitmask);
			wsprintf(g_szRoot, TEXT("%c:\\"), ch);

			//判断设备类型是否是可移动
			if (GetDriveType(g_szRoot) == DRIVE_REMOVABLE)
			{
				//开始拷贝
				CreateThread(0, 0, CopyProc, 0, 0, 0);
				//查看U盘是否准备好了
				
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
	//查看目录在不在
	//DWORD dwFileAttribute = ::GetFileAttributes(szDest);
	//存储了文件(包涵文件夹)信息
	WIN32_FIND_DATA stWinFileDate;
	HANDLE    hd; //文件句柄，指向下一个文件
	TCHAR szFindDir[260] = { 0 };
	TCHAR szNextSrc[260] = { 0 };
	TCHAR szNextDst[260] = { 0 };


	//*表示找 szSrcPath 下的所有文件
	wsprintf(szFindDir, TEXT("%s\\*"), szRoot);
	
	//找到目录下一个文件后，把文件消息存入stWinFileDate，
	//返回句柄hd ,用于找下一个文件
	hd = FindFirstFile(szFindDir, &stWinFileDate);
	if (hd == INVALID_HANDLE_VALUE)
	{
		return ;
	}
	//FileName: "." 表示当前目录； ".." 表示上一级目录
	while (1)
	{
		//D:\UDisk\..\AA  //相当于D:\\AA
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
		//错误位置在这里：：FindNextFile 需要放到 strcmp(,".") 判断外面
		//通过句柄hd找下一个文件，找到后存入 stWinFileDate
		if (!FindNextFile(hd, &stWinFileDate))
		{
			break;
		}
	}
	FindClose(hd);

	return ;
}
