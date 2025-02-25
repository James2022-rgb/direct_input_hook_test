
#include <iostream>

#include <Windows.h>

#include <detours.h>

int main(int argc, char* argv[]) {
  LPCSTR kTargetPath = "direct_input_example.exe";
  LPCSTR kDllPath = "hook.dll";

  STARTUPINFOA si = { sizeof(STARTUPINFOA) };
  PROCESS_INFORMATION pi = { 0 };

  // Launch the target EXE in a suspended state, with our DLL injected.
  BOOL result = DetourCreateProcessWithDllsA(
    kTargetPath,      // lpApplicationName
    nullptr,          // lpCommandLine
    nullptr,          // lpProcessAttributes
    nullptr,          // lpThreadAttributes
    FALSE,            // bInheritHandles
    CREATE_SUSPENDED, // dwCreationFlags
    nullptr,          // lpEnvironment
    nullptr,          // lpCurrentDirectory
    &si,              // lpStartupInfo
    &pi,              // lpProcessInformation
    1,                // nDlls
    &kDllPath,        // rlpDlls
    nullptr           // pfCreateProcessA
  );
  if (!result) {
    std::cerr << "DetourCreateProcessWithDllsA failed: " << result << std::endl;
    return 1;
  }

  // Resume the target process, and wait for it to finish.
  ResumeThread(pi.hThread);
  WaitForSingleObject(pi.hProcess, INFINITE);

  return 0;
}
