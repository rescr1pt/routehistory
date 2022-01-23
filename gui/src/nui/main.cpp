//  This file is part of the project "routehistory". The license agreement is described in the "LICENSE" file.

#include "MainForm.h"

#include <windows.h>

#include "GuiApi.h"
#include "FakePC.h"
#include <iostream>
#include <assert.h>

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
    HANDLE duplicateInstanceMutex = CreateMutex(NULL, TRUE, L"gui");
    if (duplicateInstanceMutex == NULL || ERROR_ALREADY_EXISTS == GetLastError()) {
        return 1;
    }

    std::string workingPath = "./";

#ifdef WIN32
    if (__argc == 2) {
        workingPath = __argv[1];
    }
#else 
#error To do support to other OS
#endif 

    srand((unsigned)time(NULL));

    api::GuiPC pc(workingPath);
    // api::FakePC pc;


    nui::MainForm form(pc);

    form.show();
    nana::exec();
    

    ReleaseMutex(duplicateInstanceMutex);
    CloseHandle(duplicateInstanceMutex);

    return 0;
}
