#include <Windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "c2b.h"


//#pragma comment(lib, "diaguids.lib")
// Symbols/Functions to search in pdb


//#define PDBUTIL_COMMANDLINE L" pretty -no-compiler-generated -no-system-libs -sym-types=all  <withnames> <pdbpath>";
#define PDBUTIL_COMMANDLINE L" pretty -no-compiler-generated -no-system-libs -sym-types=all  "
#define PDBUTIL_PATH L"llvm\\llvm-pdbutil.exe "

static BOOLEAN LLVMParserInitialized = FALSE;
static LPWSTR PdbCommandLineArguments = NULL;
static size_t PdbCommandLineArgumentsLength = 0;

DWORD
InitSymbolParser (
    PCONFIG_DATA ConfigData
    )
{

    DWORD Status;
    UINT Index;
    UINT PdbCommandLineBufferLength;

    Status = ERROR_SUCCESS;
    PdbCommandLineBufferLength = ConfigData->NumFunctionNames * sizeof(WCHAR)*MAX_PATH;
    Index = 0;

    PdbCommandLineArguments = malloc(PdbCommandLineBufferLength);
    ZeroMemory(PdbCommandLineArguments, PdbCommandLineBufferLength);

    StringCbPrintf(PdbCommandLineArguments,
                   PdbCommandLineBufferLength,
                   PDBUTIL_COMMANDLINE);

    for (Index = 0; Index < ConfigData->NumFunctionNames; Index++) {
        StringCbCat(PdbCommandLineArguments,
                    PdbCommandLineBufferLength,
                    L" -with-name=");

        StringCbCat(PdbCommandLineArguments,
                    PdbCommandLineBufferLength,
                    ConfigData->FunctionNames[Index]);
    }

    PdbCommandLineArgumentsLength = wcslen(PdbCommandLineArguments) * sizeof(WCHAR);
    LLVMParserInitialized = TRUE;
    return Status;
}

VOID
FreeSymbolParser (
    VOID
    )
{

    if (LLVMParserInitialized != FALSE) {
        free(PdbCommandLineArguments);
        LLVMParserInitialized = FALSE;
    }
}

DWORD
SpawnLLVMPdbProcess (
    _In_ LPWSTR PdbCommandLineArgument,
    _Out_ PINT ChildOutputFd
    )
{

    HANDLE ChildOutReadPipe;
    HANDLE ChildOutWritePipe;
    DWORD Status;
    SECURITY_ATTRIBUTES SeAttributes;
    PROCESS_INFORMATION ProcInfo;
    STARTUPINFO StartInfo;
    INT ChildOutFileFd;

    Status = ERROR_SUCCESS;
    ChildOutFileFd = -1;
    ChildOutReadPipe = INVALID_HANDLE_VALUE;
    ChildOutWritePipe = INVALID_HANDLE_VALUE;
    ZeroMemory(&ProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&StartInfo, sizeof(STARTUPINFO));
    StartInfo.cb = sizeof(STARTUPINFO);
    StartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    StartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    StartInfo.dwFlags |= STARTF_USESTDHANDLES;

    SeAttributes.bInheritHandle = TRUE;
    SeAttributes.lpSecurityDescriptor = NULL;
    SeAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);

    Status = CreatePipe(&ChildOutReadPipe, &ChildOutWritePipe, &SeAttributes, 0);
    if (Status == 0) {
        Status = GetLastError();
        goto Exit;
    }

    // Make sure child cannot write to ChildOutReadPipe.
    Status = SetHandleInformation(ChildOutReadPipe, HANDLE_FLAG_INHERIT, 0);
    if (Status == 0) {
        Status = GetLastError();
        goto Exit;
    }

    // Set the output handle of the child process to pipe write handle
    StartInfo.hStdOutput = ChildOutWritePipe;
    Status = CreateProcess(NULL,
                           PdbCommandLineArgument,
                           NULL,
                           NULL,
                           TRUE,
                           0,
                           NULL,
                           NULL,
                           &StartInfo,
                           &ProcInfo);

    if (Status == 0) {
        Status = GetLastError();
        goto Exit;
    }

    //Success
    Status = WaitForSingleObject(ProcInfo.hProcess, INFINITE);
    if (Status == WAIT_FAILED ||
        Status == WAIT_TIMEOUT ||
        Status == WAIT_ABANDONED) {
        goto Exit;
    }

    ChildOutFileFd = _open_osfhandle((intptr_t)ChildOutReadPipe, _O_TEXT);
    if (ChildOutFileFd == -1) {
        Status = ERROR_FILE_INVALID;
        goto Exit;
    }

    *ChildOutputFd = ChildOutFileFd;
    Status = ERROR_SUCCESS;

Exit:
    if (ChildOutWritePipe != INVALID_HANDLE_VALUE) {
        CloseHandle(ChildOutWritePipe);
    }

    if (ProcInfo.hProcess != INVALID_HANDLE_VALUE) {
        CloseHandle(ProcInfo.hProcess);
    }

    if (ProcInfo.hThread != INVALID_HANDLE_VALUE) {
        CloseHandle(ProcInfo.hThread);
    }

    return Status;
}

DWORD
FindSymbolsInternal (
    _In_ PBIN_PDB BinPdb
    )

{
    DWORD Status;
    LPWSTR PdbCommandLine;
    size_t PdbCommandLineLength;
    INT PdbOutputFileFd;
    FILE* PdbOutputFile;
    WCHAR Line[MAX_LINE];
    HRESULT Hr;

    Status = ERROR_SUCCESS;
    PdbOutputFile = NULL;
    PdbCommandLine = NULL;
    PdbOutputFileFd = -1;
    PdbCommandLineLength = (wcslen(PDBUTIL_PATH) * sizeof(WCHAR) +
                           PdbCommandLineArgumentsLength +
                           2 * MAX_PATH * sizeof(WCHAR));

    PdbCommandLine = calloc(1, PdbCommandLineLength);
    GetModuleFileName(NULL, Line, MAX_LINE);
    *wcsrchr(Line, L'\\') = L'\0';
    Hr = StringCbPrintf(PdbCommandLine,
                        PdbCommandLineLength,
                        L"%s\\%s %s %s",
                        Line,
                        PDBUTIL_PATH,
                        PdbCommandLineArguments,
                        BinPdb->Pdb);
    if (FAILED(Hr)) {
        Status = ERROR_INVALID_DATA;
        goto Exit;
    }

    Status = SpawnLLVMPdbProcess(PdbCommandLine, &PdbOutputFileFd);
    if (Status != ERROR_SUCCESS) {
        goto Exit;
    }

    PdbOutputFile = _wfdopen(PdbOutputFileFd, L"r");
    if (PdbOutputFile == NULL) {
        Status = ERROR_FILE_INVALID;
        goto Exit;
    }

    while (!feof(PdbOutputFile)) {
        fgetws(Line, MAX_LINE, PdbOutputFile);
        if (wcsstr(Line, L"occurrences]") != NULL) {
            wprintf(L"\n\tImpacted By: %s", Line);
            BinPdb->Impacted = TRUE;
            break;
        }
    }

Exit:
    if (PdbCommandLine != NULL) {
        free(PdbCommandLine);
    }

    if (PdbOutputFileFd != -1) {
        _close(PdbOutputFileFd);
    }

    return Status;
}

VOID
FindSymbols (
    _In_ PBUILD_ARTIFACT Artifact
    )

{

    DWORD Status;
    DWORD Index;
    PBIN_PDB BinPdb;

    Status = ERROR_SUCCESS;
    BinPdb = Artifact->BinPdbArray;

    for (Index = 0; Index < Artifact->NumBinPdbArray; Index++, BinPdb++) {
        wprintf(L"\n[%d/%d] %s %s",
                Index,
                Artifact->NumBinPdbArray,
                BinPdb->Bin,
                BinPdb->Pdb);

        Status = FindSymbolsInternal(BinPdb);
        if (Status != ERROR_SUCCESS) {
            continue;
        }
    }

    wprintf(L"\nBelow are the list of impacted binaries");
    BinPdb = Artifact->BinPdbArray;
    for (Index = 0; Index < Artifact->NumBinPdbArray; Index++, BinPdb++) {
        if (BinPdb->Impacted != FALSE) {
            wprintf(L"\n%s", BinPdb->Bin);
        }
    }
}