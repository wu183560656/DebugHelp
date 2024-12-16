/*
* 替换待启动的目标应用程序
*/
#include <Windows.h>
#include <iostream>
#include  <direct.h>
#include <stdio.h>
#include <fstream>

std::string IniFileName()
{
    char   buffer[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, buffer, sizeof(buffer));
    return std::string(buffer) + ".ini";
}

std::string GetGameExePath()
{
    char   buffer[MAX_PATH];
    GetPrivateProfileStringA("app", "path", "", buffer, sizeof(buffer), IniFileName().c_str());
    return buffer;
}

std::string GetExtraParamCommandLine()
{
    char   buffer[MAX_PATH];
    GetPrivateProfileStringA("app", "cmd", "", buffer, sizeof(buffer), IniFileName().c_str());
    return buffer;
}

bool IsSuspendedWaitInput()
{
    char buffer[MAX_PATH] = { 0 };
    return GetPrivateProfileIntA("app", "suspended", 0, IniFileName().c_str()) != 0;
}

int main(int argc, char** argv)
{
    std::string iniFile = IniFileName();
    do
    {
        if (!std::ifstream(iniFile).good())
        {
            std::cout << "配置文件:" << iniFile << " 不存在!";
            break;
        }
        STARTUPINFOA processStartupInfo{ 0 };
        processStartupInfo.cb = sizeof(processStartupInfo); // setup size of strcture in bytes
        PROCESS_INFORMATION processInfo{ nullptr };
        bool isSuspend = IsSuspendedWaitInput();
        //进程名
        auto commandLineToExecute = "\"" + GetGameExePath() + "\"";
        //原程序参数
        for (int i = 1; i < argc; i++)
        {
            commandLineToExecute = commandLineToExecute.append(" ");
            commandLineToExecute = commandLineToExecute.append(argv[i]);
        }
        //附加参数
        auto ExtraParamCommandLine = GetExtraParamCommandLine();
        if (ExtraParamCommandLine.length() != 0)
        {
            commandLineToExecute += " ";
            commandLineToExecute += ExtraParamCommandLine;
        }
        std::cout << commandLineToExecute << std::endl;

        auto result = CreateProcessA(nullptr, (LPSTR)commandLineToExecute.c_str(), nullptr, nullptr, false,
            isSuspend ? CREATE_SUSPENDED : 0,
            nullptr,
            nullptr,
            &processStartupInfo, &processInfo);

        auto  getlasterror = GetLastError();
        std::cout << "CreateProcessA: " << result << std::endl;
        std::cout << "GetLastError: " << getlasterror << std::endl;
        if (!result)
        {
            std::cout << "应用程序启动失败!!!" << std::endl;
            break;
        }
        std::cout << "进程启动成功!!!" << std::endl;
        if (isSuspend)
        {
            std::cout << "进程已暂停,输入r继续运行" << std::endl;
            while (std::getchar() != 'r');
            ResumeThread(processInfo.hThread);
        }
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    } while (false);
    system("pause");
    return 0;
}