#include "persistence.h"
#include "utils.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

static fs::path CreateTaskXml() {
    fs::path tempPath = fs::temp_directory_path() / "PM_Restore.xml";
    std::ofstream ofs(tempPath);

    // 注意：保持原逻辑中的 XML 格式
    ofs << L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n"
        << L"<Task version=\"1.2\">\n"
        << L" <Triggers><BootTrigger><Enabled>true</Enabled></BootTrigger></Triggers>\n"
        << L" <Principals><Principal><RunLevel>HighestAvailable</RunLevel></Principal></Principals>\n"
        << L" <Settings><StartWhenAvailable>true</StartWhenAvailable></Settings>\n"
        << L" <Actions><Exec>\n"
        << L" <Command>cmd.exe</Command>\n"
        << L" <Arguments>/c if not exist \"%~dp0process_runtime.txt\" copy /y \"C:\\ProgramData\\ProcessMonitor\\process_runtime.bak\" \"%~dp0process_runtime.txt\"</Arguments>\n"
        << L" </Exec></Actions>\n"
        << L"</Task>";
    return tempPath;
}

void Persistence::InstallBootTask() {
    fs::path xmlPath = CreateTaskXml();
    std::wstring cmd = L"schtasks.exe /create /tn \"ProcessMonitorBootRestore\" /xml \"" + xmlPath.wstring() + L"\" /f";

    SHELLEXECUTEINFOW sei{ sizeof(sei) };
    sei.lpVerb = L"runas";
    sei.lpFile = L"schtasks.exe";
    sei.lpParameters = cmd.substr(cmd.find(L" /create")).c_str();
    sei.nShow = SW_HIDE;

    if (ShellExecuteExW(&sei)) {
        Utils::Log(u8"开机还原任务注册成功");
    } else {
        Utils::Log(u8"开机还原任务注册失败");
    }
    fs::remove(xmlPath);
}
