#include <Windows.h>
#include <TlHelp32.h>
#include <comdef.h>
#include <iostream>
#include <thread>
#include <vector>

struct PModule {
	DWORD dwBase;
	DWORD dwAdress;
};

class memory
{
public:
	inline bool Attach(const char* pName, DWORD dwAccess) {
		HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);

		do
		{
			if (!strcmp(_bstr_t(entry.szExeFile), pName))
			{
				_pId = entry.th32ProcessID;
				CloseHandle(handle);
				_process = OpenProcess(dwAccess, false, _pId);
				return true;
			}

		} while (Process32Next(handle, &entry));
		return false;
	}

	inline PModule GetModule(const char* pModule) {
		HANDLE module = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, _pId);

		MODULEENTRY32 entry;
		entry.dwSize = sizeof(entry);

		do
		{
			if (!strcmp(_bstr_t(entry.szModule), pModule))
			{
				CloseHandle(module);
				return PModule{ reinterpret_cast<DWORD>(entry.hModule), entry.modBaseSize };
			}

		} while (Module32Next(module, &entry));
		return PModule{ 0,0 };

	}

	template <class T>
	T Read(const DWORD dwAddress) {
		T _read;
		ReadProcessMemory(_process, LPVOID(dwAddress), &_read, sizeof(T), NULL);
		return _read;
	}

	template <class T>
	void Write(const DWORD dwAddress, const T value) {
		WriteProcessMemory(_process, LPVOID(dwAddress), &value, sizeof(T), NULL);
	}

	void Exit()
	{
		CloseHandle(_process);
	}



private:
	HANDLE _process;
	DWORD _pId;

};

using namespace std;

memory mem;
PModule bClient;


#define dwLocalPlayer 0xDBA5BC
#define m_iHealth 0x100
#define m_iTeamNum 0xF4
#define m_iCrossHairID 0x11838
#define dwEntityList 0x4DD69DC
#define m_bSpotted  0x93D
#define dwForceJump 0x5280924
#define flags  0x104
#define m_iGlowIndex  0x10488
#define dwGlowObjectManager  0x531F608
#define m_iTeamNum  0xF4

struct MyStruct
{
	bool triggerbot = true;
	bool bunnyhop = true;
	bool radarhack = true;
	bool wallhack = true;

}options;


void inline RadarHack() {
	while (true)
	{

		if (GetAsyncKeyState(VK_NUMPAD2))
		{
			options.radarhack = !options.radarhack;
			cout << "RadarHack is " << options.radarhack << " now" << endl;
			Sleep(500);
		}
	
		if (!options.radarhack) continue;
		DWORD pLocal = mem.Read<DWORD>(bClient.dwBase + dwLocalPlayer);
		DWORD lHealth = mem.Read<DWORD>(pLocal + m_iHealth);
		if (!lHealth) continue;
		DWORD localTeam = mem.Read<DWORD>(pLocal + m_iTeamNum);
		for (DWORD i = 0; i < 64; i++)
		{
			DWORD pEnt = mem.Read<DWORD>(bClient.dwBase + dwEntityList + (i - 1) * 0x10);
			DWORD entHp = mem.Read<DWORD>(pEnt + m_iHealth);
			if (!entHp) continue;
			DWORD entTeam = mem.Read<DWORD>(pEnt + m_iTeamNum);
			if (entTeam == localTeam) continue;
			BOOL entSpooted = mem.Read<DWORD>(pEnt + m_bSpotted);
			if (entSpooted) continue;
			mem.Write(pEnt + m_bSpotted, TRUE);
		}
	}

}



void  TriggerBot() {
	while (1)
	{
		if (GetAsyncKeyState(VK_NUMPAD1))
		{
			options.triggerbot = !options.triggerbot;
			cout << "TrigerBot is " << options.triggerbot << " now" << endl;
			Sleep(500);
		}
		if (!options.triggerbot) continue;
		DWORD pLocal = mem.Read<DWORD>(bClient.dwBase + dwLocalPlayer);
		DWORD lHealth = mem.Read<DWORD>(pLocal + m_iHealth);
		if (!lHealth) continue;
		DWORD lTeamNum = mem.Read<DWORD>(pLocal + m_iTeamNum);
		DWORD lCrosshairId = mem.Read<DWORD>(pLocal + m_iCrossHairID);
		if (!lCrosshairId || lCrosshairId > 64) continue;
		DWORD entityBase = mem.Read<int>(bClient.dwBase + dwEntityList + (lCrosshairId - 1) * 0x10);
		if (!entityBase) continue;
		DWORD entityHp = mem.Read<DWORD>(entityBase + m_iHealth);
		DWORD entityTeam = mem.Read<DWORD>(entityBase + m_iTeamNum);
		if (!entityHp || entityTeam == lTeamNum) continue;

		
		if (GetAsyncKeyState(VK_MBUTTON))
		{
			Sleep(1);
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			Sleep(1);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		}
	}
}

void inline BunnyHop() {


	while (true)
	{
		if (GetAsyncKeyState(VK_NUMPAD3))
		{
			options.bunnyhop = !options.bunnyhop;
			cout << "BunnyHop is " << options.bunnyhop << " now" << endl;
			Sleep(500);
		}
		if (!options.bunnyhop) continue;
		DWORD LocalPlayer = mem.Read<DWORD>(bClient.dwBase + dwLocalPlayer);
		DWORD flag = mem.Read<BYTE>(LocalPlayer + flags);
		if (GetAsyncKeyState(VK_SPACE) && flag & (1 << 0))
		{
			mem.Write<DWORD>(bClient.dwBase + dwForceJump, 6);
			Sleep(1);
		}
	}

}



void Wallhack()
{
	while (true) 
	{
		if (GetAsyncKeyState(VK_NUMPAD4))
		{
			options.wallhack = !options.wallhack;
			cout << "WallHack is " << options.wallhack << " now" << endl;
			Sleep(500);
		}
		if (!options.wallhack) continue;
		
		Sleep(1);
		for (auto i = 0; i < 64; ++i)
		{	
			const auto LocalPlayer = mem.Read<DWORD>(bClient.dwBase + dwLocalPlayer);
			const auto glow = mem.Read<DWORD>(bClient.dwBase + dwGlowObjectManager);
			const auto entity = mem.Read<uintptr_t>(bClient.dwBase + dwEntityList + i * 0x10);
			int LocalTeam = mem.Read<int>(LocalPlayer + m_iTeamNum);
			if (LocalTeam == mem.Read<DWORD>(entity + m_iTeamNum)) continue;
			
			if (entity)
			{
				if (LocalTeam==3)
				{
					const auto glowIndex = mem.Read<int32_t>(entity + m_iGlowIndex);

					mem.Write<float>(glow + (glowIndex * 0x38) + 0x8, 1.f);
					mem.Write<float>(glow + (glowIndex * 0x38) + 0xC, 0.f);
					mem.Write<float>(glow + (glowIndex * 0x38) + 0x10, 0.f);
					mem.Write<float>(glow + (glowIndex * 0x38) + 0x14, 1.f);

					mem.Write<bool>(glow + (glowIndex * 0x38) + 0x27, true);
					mem.Write<bool>(glow + (glowIndex * 0x38) + 0x28, true);
				}
				else if (LocalTeam==2)
				{
					const auto glowIndex = mem.Read<int32_t>(entity + m_iGlowIndex);
					mem.Write<float>(glow + (glowIndex * 0x38) + 0x8, 0.f);
					mem.Write<float>(glow + (glowIndex * 0x38) + 0xC, 0.f);
					mem.Write<float>(glow + (glowIndex * 0x38) + 0x10, 1.f);
					mem.Write<float>(glow + (glowIndex * 0x38) + 0x14, 1.f);

					mem.Write<bool>(glow + (glowIndex * 0x38) + 0x27, true);
					mem.Write<bool>(glow + (glowIndex * 0x38) + 0x28, true);
				}
				
			}
			
		}
	}
}



	

int main() {
	

	cout << "NUMPAD 1 - triggerbot" << endl;
	cout << "NUMPAD 2 - radarhack" << endl;
	cout << "NUMPAD 3 - bunnyhop" << endl;
	cout << "NUMPAD 4 - wallhack" << endl;
	while (!mem.Attach("csgo.exe", PROCESS_ALL_ACCESS))
	{
		cout << "waiting for launch" << endl;

	}
	cout << "Process was found" << endl;
	bClient = mem.GetModule("client.dll");

	std::thread TriggerBotThread(TriggerBot);
	std::thread RadarHackThread(RadarHack);
	std::thread BunnyHopThread(BunnyHop);
	std::thread WallhackThread(Wallhack);
	while (true) {
	
	}
	
	return 0;
}