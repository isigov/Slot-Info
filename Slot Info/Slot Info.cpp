// Slot Info.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <Iphlpapi.h>
#include <iostream>
#include <vector>
#include <fstream>

#pragma comment(lib, "Iphlpapi.lib")

using namespace std;

#define AF_INET 2

bool GetDebugPrivileges(HANDLE hProc);
void KickPlayer(char* IP, DWORD port);
vector<MIB_TCPROW_OWNER_PID> GetTcpConnections();
DWORD CloseConnection(MIB_TCPROW_OWNER_PID pointer);

int _tmain(int argc, _TCHAR* argv[])
{
	SetConsoleTitle(L"TinyKick 0.2 Beta");
	printf("TinyKick 0.2 Beta for WC3 TFT 1.26\nCreated by Sixco\nsix_co@live.com\n\nCommands (to be typed in the WC3 chat):\n/kick playerName - disconnects a player from the game\n/exit - exits TinyKick\r\n\n");

	DWORD dwStorm = 0;
	DWORD dwGame = 0;
	DWORD dwStatusAddr = 0;

	if(!GetDebugPrivileges(GetCurrentProcess()))
	{
		printf("couldnt get debug privs\n");
		getchar();
		return 0;
	}

	HWND window = FindWindow(NULL, L"Warcraft III");
	if(window == NULL)
	{
		printf("couldnt find wc3\n");
		getchar();
		return 0;
	}
	DWORD pid;
	GetWindowThreadProcessId(window, &pid);
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	if(hProc == NULL)
	{
		printf("couldnt open wc3\n");
		getchar();
		return 0;
	}

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	MODULEENTRY32 mEntry;
	mEntry.dwSize = sizeof(MODULEENTRY32);
	
	if(Module32First(hSnap, &mEntry))
	{
		while(Module32Next(hSnap, &mEntry))
		{
			if(!wcscmp(mEntry.szModule, L"Storm.dll"))
				dwStorm = (DWORD)mEntry.modBaseAddr;
			else if(!wcscmp(mEntry.szModule, L"Game.dll"))
				dwGame = (DWORD)mEntry.modBaseAddr;
			if(dwStorm && dwGame)
				break;
		}
		CloseHandle(hSnap);
	}
	if(dwStorm == 0 || dwGame == 0)
	{
		printf("couldnt open get dll handles\n");
		getchar();
		return 0;
	}

	//for(unsigned int i = 10; i > 0; i--)
	//{
		//printf("\rHiding window in %d...", i);
		//Sleep(1000);
	//}
	ShowWindow(GetConsoleWindow(), 0);

	LPVOID lpBuffer = malloc(4);
	DWORD dwBase = 0;

	if(!ReadProcessMemory(hProc, (LPCVOID)(dwGame + 0x00ADA7E0), lpBuffer, 4, NULL))
			ExitProcess(0);
	dwBase = (*(DWORD*)lpBuffer) + 0x4AC;
	ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
	dwBase = (*(DWORD*)lpBuffer) + 0x4;
	ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
	dwBase = (*(DWORD*)lpBuffer);
	ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
	dwBase = (*(DWORD*)lpBuffer) + 0x4;
	ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
	dwStatusAddr = (*(DWORD*)lpBuffer) + 0x64;

	while(true)
	{
		Sleep(2000);

		if(!ReadProcessMemory(hProc, (LPCVOID)(dwStatusAddr), lpBuffer, 1, NULL))
			ExitProcess(0);

		char* textString = new char[256];
		char* playerToKick;
		bool Kick = false;
		if(((BYTE*)lpBuffer)[0] == 0x0)
		{
			ReadProcessMemory(hProc, (LPCVOID)(dwStorm + 0x000555BC), lpBuffer, 4, NULL);
			dwBase = (*(DWORD*)lpBuffer) + 0x44;
			ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
			dwBase = (*(DWORD*)lpBuffer) + 0x8;
			ReadProcessMemory(hProc, (LPCVOID)(dwBase), textString, 256, NULL);
		}
		else if(((BYTE*)lpBuffer)[0] == 0x1)
		{
			ReadProcessMemory(hProc, (LPCVOID)(dwStorm + 0x000555BC), lpBuffer, 4, NULL);
			dwBase = (*(DWORD*)lpBuffer);
			ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
			dwBase = (*(DWORD*)lpBuffer);
			ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
			dwBase = (*(DWORD*)lpBuffer) + 0x44;
			ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
			dwBase = (*(DWORD*)lpBuffer) + 0x8;
			ReadProcessMemory(hProc, (LPCVOID)(dwBase), textString, 256, NULL);
		}
		else
			continue;

		if(strstr(textString, "/kick "))
		{
			playerToKick = new char[17];
			strcpy(playerToKick, &textString[6]);
			Kick = true;
		}
		else if(strstr(textString, "/exit"))
				ExitProcess(0);

		if(Kick)
		{
			ReadProcessMemory(hProc, (LPCVOID)(dwStorm + 0x00055438), lpBuffer, 4, NULL);
			dwBase = (*(DWORD*)lpBuffer);
			for(unsigned int z = 0; z < 4; z++)
			{
				ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
				dwBase = (*(DWORD*)lpBuffer);
				BYTE* bTemp = new BYTE[4];
				DWORD dwTemp = (*(DWORD*)lpBuffer) + 0x6D;
				ReadProcessMemory(hProc, (LPCVOID)(dwTemp), bTemp, 4, NULL);
				if(((char*)bTemp)[0] == '\\' && ((char*)bTemp)[1] == 'N')
				{
					dwTemp += 0xD0;
					for(unsigned int i = 0; i < 11; i++)
					{
						char* playerName = new char[17];
						ReadProcessMemory(hProc, (LPVOID)(dwTemp + (0xE8 * i)), (LPVOID)playerName, 17, NULL);
						if(playerName[0] == 0x00)
							break;
					
						if(strstr(playerName, playerToKick))
						{
							BYTE* address = new BYTE[4];
							BYTE* bPort = new BYTE[2];
							ReadProcessMemory(hProc, (LPVOID)(dwTemp + (0xE8 * i) - 0x21), (LPVOID)address, 4, NULL);
							ReadProcessMemory(hProc, (LPVOID)(dwTemp + (0xE8 * i) - 0x21 - 2), (LPVOID)bPort, 2, NULL);
							short port = (bPort[0] << 8 | bPort[1]);
							char* ip = new char[20];
							sprintf(ip, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);
							KickPlayer(ip, port);
						}
					}
				}
			}
		}
			/*if(((char*)lpBuffer)[0] == '\\' && ((char*)lpBuffer)[1] == 'N')
			{
				dwBase += 0xD0;
				for(unsigned int i = 0; i < 11; i++)
				{
					char* playerName = new char[17];
					ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i)), (LPVOID)playerName, 17, NULL);
					if(playerName[0] == 0x00)
						break;
					
					if(strstr(playerName, playerToKick))
					{
						BYTE* address = new BYTE[4];
						BYTE* bPort = new BYTE[2];
						ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i) - 0x21), (LPVOID)address, 4, NULL);
						ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i) - 0x21 - 2), (LPVOID)bPort, 2, NULL);
						short port = (bPort[0] << 8 | bPort[1]);
						char* ip = new char[20];
						sprintf(ip, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);
						KickPlayer(ip, port);
					}
				}
			}
			else
			{
				ReadProcessMemory(hProc, (LPCVOID)(dwStorm + 0x00055438), lpBuffer, 4, NULL);
				dwBase = (*(DWORD*)lpBuffer);
				ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
				dwBase = (*(DWORD*)lpBuffer) + 0x6D;
				ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
				if(((char*)lpBuffer)[0] == '\\' && ((char*)lpBuffer)[1] == 'N')
				{
					dwBase += 0xD0;
					for(unsigned int i = 0; i < 11; i++)
					{
						char* playerName = new char[17];
						ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i)), (LPVOID)playerName, 17, NULL);
						if(playerName[0] == 0x00)
							break;

						if(strstr(playerName, playerToKick))
						{
							BYTE* address = new BYTE[4];
							BYTE* bPort = new BYTE[2];
							ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i) - 0x21), (LPVOID)address, 4, NULL);
							ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i) - 0x21 - 2), (LPVOID)bPort, 2, NULL);
							short port = (bPort[0] << 8 | bPort[1]);
							char* ip = new char[20];
							sprintf(ip, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);
							KickPlayer(ip, port);
						}
					}
				}
				else
				{
					ReadProcessMemory(hProc, (LPCVOID)(dwStorm + 0x00055438), lpBuffer, 4, NULL);
					dwBase = (*(DWORD*)lpBuffer);
					ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
					dwBase = (*(DWORD*)lpBuffer);
					ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
					dwBase = (*(DWORD*)lpBuffer) + 0x6D;
					ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
					if(((char*)lpBuffer)[0] == '\\' && ((char*)lpBuffer)[1] == 'N')
					{
						dwBase += 0xD0;
						for(unsigned int i = 0; i < 11; i++)
						{
							char* playerName = new char[17];
							ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i)), (LPVOID)playerName, 17, NULL);
							if(playerName[0] == 0x00)
								break;
							if(strstr(playerName, playerToKick))
							{
								BYTE* address = new BYTE[4];
								BYTE* bPort = new BYTE[2];
								ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i) - 0x21), (LPVOID)address, 4, NULL);
								ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i) - 0x21 - 2), (LPVOID)bPort, 2, NULL);
								short port = (bPort[0] << 8 | bPort[1]);
								char* ip = new char[20];
								sprintf(ip, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);
								KickPlayer(ip, port);
							}
						}
					}
					else
					{
						ReadProcessMemory(hProc, (LPCVOID)(dwStorm + 0x00055438), lpBuffer, 4, NULL);
						dwBase = (*(DWORD*)lpBuffer);
						ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
						dwBase = (*(DWORD*)lpBuffer);
						ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
						dwBase = (*(DWORD*)lpBuffer);
						ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
						dwBase = (*(DWORD*)lpBuffer) + 0x6D;
						ReadProcessMemory(hProc, (LPCVOID)(dwBase), lpBuffer, 4, NULL);
						if(((char*)lpBuffer)[0] == '\\' && ((char*)lpBuffer)[1] == 'N')
							dwBase += 0xD0;
						for(unsigned int i = 0; i < 11; i++)
						{
							char* playerName = new char[17];
							ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i)), (LPVOID)playerName, 17, NULL);
							if(playerName[0] == 0x00)
								break;
							if(strstr(playerName, playerToKick))
							{
								BYTE* address = new BYTE[4];
								BYTE* bPort = new BYTE[2];
								ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i) - 0x21), (LPVOID)address, 4, NULL);
								ReadProcessMemory(hProc, (LPVOID)(dwBase + (0xE8 * i) - 0x21 - 2), (LPVOID)bPort, 2, NULL);
								short port = (bPort[0] << 8 | bPort[1]);
								char* ip = new char[20];
								sprintf(ip, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);
								KickPlayer(ip, port);
							}
						}
					}
				}
			}*/
	}
	return 0;
}

bool GetDebugPrivileges(HANDLE hProc)
{
	HANDLE hToken;
	if(!OpenProcessToken(hProc, TOKEN_ADJUST_PRIVILEGES, &hToken))
		return false;

	LUID luid;
	if(!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
		return false;

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(hToken, false, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
		return false;

	return true;
}

void KickPlayer(char* IP, DWORD port)
{
	vector<MIB_TCPROW_OWNER_PID> connections = GetTcpConnections();
	for(unsigned int i = 0; i < connections.size(); i++)
	{
		char* strString = new char[20];
		sprintf(strString, "%d.%d.%d.%d", (connections[i].dwRemoteAddr) & 0xFF, (connections[i].dwRemoteAddr >> 8) & 0xFF, (connections[i].dwRemoteAddr >> 16) & 0xFF, (connections[i].dwRemoteAddr >> 24) & 0xFF);
		if(!strcmp(strString, IP))
			CloseConnection(connections[i]);
	}
}

vector<MIB_TCPROW_OWNER_PID> GetTcpConnections()
{
	vector<MIB_TCPROW_OWNER_PID> list;
	MIB_TCPTABLE_OWNER_PID *pointer = (MIB_TCPTABLE_OWNER_PID*)malloc(1);
	DWORD getSize = 1;
	GetExtendedTcpTable(pointer, &getSize, true, AF_INET, TCP_TABLE_OWNER_PID_CONNECTIONS, 0);
	free(pointer);
	pointer = (MIB_TCPTABLE_OWNER_PID*)malloc(getSize);
	GetExtendedTcpTable(pointer, &getSize, true, AF_INET, TCP_TABLE_OWNER_PID_CONNECTIONS, 0);
	for(unsigned int i = 0; i < pointer->dwNumEntries; i++)
		list.push_back(pointer->table[i]);
	free(pointer);
	return list;
}

DWORD CloseConnection(MIB_TCPROW_OWNER_PID pointer)
{
	MIB_TCPROW row;
	row.dwLocalAddr = pointer.dwLocalAddr;
	row.dwLocalPort = pointer.dwLocalPort;
	row.dwRemoteAddr = pointer.dwRemoteAddr;
	row.dwRemotePort = pointer.dwRemotePort;
	row.dwState = MIB_TCP_STATE_DELETE_TCB;
	return (SetTcpEntry(&row));
}