#include <Windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "c2b.h"

WCHAR BranchName[MAX_PATH];
WCHAR ChangeFilePath[MAX_PATH];
WCHAR WinbuildsPath[MAX_PATH];
BOOLEAN AllBranches;

VOID
PrintHelp (
    VOID
    )
{

    wprintf(
        L"\nUSAGE:"
        L"\n    ChangeToBin2.exe <winbuildspath|branchname> <change file path>"
        L"\n"
        L"\n    ChangeToBin2.exe <change file path>"
        L"\nARGS:"
        L"\n    <branch name>"
        L"\n"
        L"\n            it can be rs5_release_svc_1907b"
        L"\n"
        L"\n    <change file path>"
        L"\n"
        L"\n            Path to the text file which contain function or type name that are"
        L"\n            modified in your change list or PR. One per line."
        L"\n"
        L"\n            The idea is we try to check each function and type listed in this"
        L"\n            file to find which binary is is effected.");
}

DWORD
ParseArguments (
    _In_ INT Argc,
    _In_ WCHAR **Argv
    )
{

    DWORD Status;

    Status = ERROR_SUCCESS;
    AllBranches = FALSE;

    if (Argc < 2) {
        return ERROR_INVALID_PARAMETER;
    }

    if (Argc == 2) {
        StringCchPrintfW(ChangeFilePath, MAX_PATH, Argv[1]);

        //
        // Check if the file can be opened?
        //

        if (FileExists(ChangeFilePath) == FALSE) {
            return ERROR_FILE_NOT_FOUND;
        }

        AllBranches = TRUE;
        return Status;
    }

    if (Argv[1][0] == L'\\' && Argv[1][1] == L'\\') {
        StringCchPrintfW(WinbuildsPath, MAX_PATH, Argv[1]);

    } else {
        StringCchPrintfW(BranchName, MAX_PATH, Argv[1]);
    }

    AllBranches = FALSE;
    StringCchPrintfW(ChangeFilePath, MAX_PATH, Argv[2]);

    return Status;
}

VOID
FreeBinPlaceResourcesFromArtifact (
    _Inout_ PBUILD_ARTIFACT Artifact
    )
{

    // DWORD Index;

    // for (Index = 0; Index < Artifact->NumBinPdbArray; Index++) {
    //     free(Artifact->BinPdbArray[Index].Bin);
    //     free(Artifact->BinPdbArray[Index].Pdb);
    // }

    free(Artifact->BinPdbArray);
}

DWORD
ParseSymbolsInputFile (
    _Inout_ PCONFIG_DATA ConfigData
    )

{
    DWORD Status;
    DWORD LineCount;
    WCHAR Line[MAX_LINE];
    LPWSTR *TempFunctionNames;
    FILE *File;

    Status = ERROR_SUCCESS;
    File = NULL;
    LineCount = 0;

    File = _wfopen(ConfigData->FuncFile, L"r");
    if (File == NULL) {
        Status = ERROR_FILE_NOT_FOUND;
        goto Exit;
    }

    while (fgetws(Line, MAX_LINE, File) != NULL) {
        StrTrimEnd(Line);
        if (StrIsEmpty(Line)) {
            continue;
        }

        LineCount++;
    }

    rewind(File);
    TempFunctionNames = calloc(LineCount + 1, sizeof(LPCWSTR));
    ConfigData->FunctionNames = TempFunctionNames;
    while (fgetws(Line, MAX_LINE, File) != NULL) {
        StrTrimEnd(Line);
        if (StrIsEmpty(Line)) {
            continue;
        }

        *TempFunctionNames = _wcsdup(Line);
        TempFunctionNames++;
    }

    TempFunctionNames = NULL;
    qsort(ConfigData->FunctionNames, LineCount, sizeof(LPCWSTR), StringCompare);
    ConfigData->NumFunctionNames = LineCount;

Exit:
    if (File != NULL) {
        fclose(File);
    }

    return Status;
}

DWORD
InitConfigData (
    _Inout_ PCONFIG_DATA ConfigData
    )
{

    DWORD Status;

    Status = ConfigReadSection(CONFIG_FILE_PATH,
                                DEFAULT_BRANCH_SECTION_NAME,
                                &ConfigData->DefaultBranches,
                                &ConfigData->NumDefaultBranches);

    if (Status != ERROR_SUCCESS) {
        goto Exit;
    }

    Status = ConfigReadSection(CONFIG_FILE_PATH,
                                BLACKLIST_PATHS_SECTION_NAME,
                                &ConfigData->BlackListedPaths,
                                &ConfigData->NumBlackListedPaths);

    if (Status != ERROR_SUCCESS) {
        goto Exit;
    }

    Status = ConfigReadSection(CONFIG_FILE_PATH,
                                VALID_COMPONENT_PATHS_SECTION_NAME,
                                &ConfigData->ValidComponentPaths,
                                &ConfigData->NumValidComponentPaths);

    if (Status != ERROR_SUCCESS) {
        goto Exit;
    }

Exit:
    return Status;
}

INT
wmain (
    _In_ INT Argc,
    _In_ WCHAR **Argv
    )
{

    DWORD Status;
    DWORD Index;
    BUILD_ARTIFACT Artifact;
    CONFIG_DATA ConfigData;

    Artifact.ConfigData = &ConfigData;

    Status = ParseArguments(Argc, Argv);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"Error parsing the arguments %u", Status);
        PrintHelp();
        goto Exit;
    }

    ConfigData.FuncFile = ChangeFilePath;
    Status = ParseSymbolsInputFile(&ConfigData);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"Error parsing %s file", ChangeFilePath);
        PrintHelp();
        goto Exit;
    }

    Status = InitConfigData(&ConfigData);
    if (Status != ERROR_SUCCESS) {
        goto Exit;
    }

    if (AllBranches == FALSE) {
        ConfigData.DefaultBranches[0] = BranchName;
        ConfigData.NumDefaultBranches = 1;
    }

    Status = InitSymbolParser(&ConfigData);
    if (Status != ERROR_SUCCESS) {
        wprintf(L"Dia COM Init failed %u", Status);
        goto Exit;
    }

    for (Index = 0; Index < ConfigData.NumDefaultBranches; Index++) {
        StringCchPrintfW(Artifact.BuildPath,
                        MAX_PATH,
                        L"\\\\winbuilds\\release\\%s",
                        ConfigData.DefaultBranches[Index]);

        wprintf(L"\nBuild Path: %s", Artifact.BuildPath);
        Status = FindGoodBuild(&Artifact);
        if (Status != ERROR_SUCCESS) {
            wprintf(L"\n\tCould not find a valid build for %s : %d",
                    ConfigData.DefaultBranches[Index],
                    Status);

            if (Status == ERROR_FILE_NOT_FOUND) {
                Status = ERROR_SUCCESS;
                continue;
            }
        }

        Status = BinplaceParseLog(&Artifact);
        if (Status != ERROR_SUCCESS) {
            wprintf(L"\n\tError parsing bin place log: %d", Status);
            goto FreeAllocations;
        }

        FindSymbols(&Artifact);

FreeAllocations:

        //
        // Free the BinPdbArray once done
        //
        FreeBinPlaceResourcesFromArtifact(&Artifact);
    }

Exit:
    FreeSymbolParser();
    return Status;
}