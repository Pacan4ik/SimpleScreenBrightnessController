#include <windows.h>
#include <highlevelmonitorconfigurationapi.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "Dxva2.lib")

struct MonitorInfo {
    HMONITOR hMonitor;
    HANDLE hPhysicalMonitor;
};

bool SetMonitorBrightnessN(HANDLE hPhysicalMonitor, DWORD brightness) {
    std::cout << "[DEBUG] Setting brightness to: " << brightness << std::endl;
    if (!SetMonitorBrightness(hPhysicalMonitor, brightness)) {
        std::cerr << "[ERROR] Failed to set brightness. Error code: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);

    DWORD cPhysicalMonitors;
    if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &cPhysicalMonitors)) {
        std::cerr << "[ERROR] Failed to get number of physical monitors for HMONITOR. Error code: " << GetLastError() << std::endl;
        return TRUE;
    }

    LPPHYSICAL_MONITOR pPhysicalMonitors = new PHYSICAL_MONITOR[cPhysicalMonitors];
    if (!GetPhysicalMonitorsFromHMONITOR(hMonitor, cPhysicalMonitors, pPhysicalMonitors)) {
        std::cerr << "[ERROR] Failed to get physical monitors. Error code: " << GetLastError() << std::endl;
        delete[] pPhysicalMonitors;
        return TRUE;
    }

    for (DWORD i = 0; i < cPhysicalMonitors; ++i) {
        MonitorInfo info = { hMonitor, pPhysicalMonitors[i].hPhysicalMonitor };
        monitors->emplace_back(info);
    }

    delete[] pPhysicalMonitors;
    return TRUE;
}

int main() {
    DWORD brightness;
    std::cout << "Enter brightness (0-100): ";
    std::cin >> brightness;
    int exitCode = 0;

    if (brightness > 100) {
        std::cerr << "[ERROR] Invalid brightness level. Use values from 0 to 100." << std::endl;
        exitCode = 2;
    }

    std::vector<MonitorInfo> monitors;
    if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors))) {
        std::cerr << "[ERROR] Failed to enumerate monitors. Error code: " << GetLastError() << std::endl;
        exitCode = 1;
    }

    if (monitors.empty()) {
        std::cerr << "[ERROR] No monitors found." << std::endl;
        exitCode = 1;
    }

    size_t monitorIndex;
    std::cout << "Select monitor (1-" << monitors.size() << "): ";
    std::cin >> monitorIndex;

    if (monitorIndex < 1 || monitorIndex > monitors.size()) {
        std::cerr << "[ERROR] Invalid monitor selection." << std::endl;
        exitCode = 1;
    }

    auto& selectedMonitor = monitors[monitorIndex - 1];
    if (!SetMonitorBrightness(selectedMonitor.hPhysicalMonitor, brightness)) {
        std::cerr << "[ERROR] Failed to adjust brightness for selected monitor." << std::endl;
        exitCode = 1;
    }

    if (!exitCode) {
        std::cout << "[INFO] Brightness set successfully!" << std::endl;
    }

    for (auto& monitor : monitors) {
        PHYSICAL_MONITOR tempMonitor = { monitor.hPhysicalMonitor, L"" };
        DestroyPhysicalMonitors(1, &tempMonitor);
    }
    system("pause");
    return exitCode;
}