#pragma once

#define MAX_LINE 4000

#define CONFIG_FILE_PATH    L"config.txt"
#define DEFAULT_BRANCH_SECTION_NAME L"[default branches]"
#define BLACKLIST_PATHS_SECTION_NAME L"[blacklisted paths]"
#define VALID_COMPONENT_PATHS_SECTION_NAME L"[valid component paths]"

typedef struct _BIN_PDB {
    WCHAR Bin[MAX_PATH];
    WCHAR Pdb[MAX_PATH];
    BOOLEAN Impacted;
} BIN_PDB, *PBIN_PDB;

typedef struct _CONFIG_DATA {
    //
    // Filter
    //
    LPWSTR *BlackListedPaths;
    DWORD NumBlackListedPaths;
    LPWSTR *ValidComponentPaths;
    DWORD NumValidComponentPaths;

    //
    // Input Functions
    //
    LPWSTR *FunctionNames;
    DWORD NumFunctionNames;
    LPCWSTR FuncFile;

    //
    // Default Branches
    //
    LPCWSTR *DefaultBranches;
    DWORD NumDefaultBranches;
} CONFIG_DATA, *PCONFIG_DATA;

typedef struct _BUILD_ARTIFACT {
    WCHAR BuildPath[MAX_PATH];
    PBIN_PDB BinPdbArray;
    DWORD NumBinPdbArray;

    PCONFIG_DATA ConfigData;
} BUILD_ARTIFACT, *PBUILD_ARTIFACT;

//
// Build share API
//

DWORD
FindGoodBuild (
    _Inout_ PBUILD_ARTIFACT Artifact
    );

//
// Time related API
//

UINT
GetDaysSince2005 (
    _In_ SYSTEMTIME *Date
    );

//
// File Related
//

BOOLEAN
FileExists (
    _In_ LPCWSTR FilePath
    );

//
// Config Parsing external functions
//

DWORD
ConfigReadSection(
    _In_ LPCWSTR ConfigPath,
    _In_ LPCWSTR SectionName,
    _Out_ LPCWSTR** Result,
    _Out_ PDWORD  NumResult
    );

//
// Parse Func.txt file
//

DWORD
ParseSymbolsInputFile (
    _Inout_ PCONFIG_DATA ConfigData
    );

//
// Symbol file processing
//

VOID
FindSymbols (
    _In_ PBUILD_ARTIFACT Artifact
    );

DWORD
InitSymbolParser(
    PCONFIG_DATA ConfigData
    );

VOID
FreeSymbolParser(
    VOID
    );
//
// Binplace Log Parsing external functions
//

DWORD
BinplaceParseLog (
    _Inout_ PBUILD_ARTIFACT Artifact
    );

//
// String Utility APIs
//

LPWSTR
StrTrimEnd (
    _Inout_ LPWSTR String
    );

BOOLEAN
StrEndsWith (
    _In_ LPCWSTR String,
    _In_ LPCWSTR Suffix
    );

BOOLEAN
StrIsEmpty (
    _In_ LPCWSTR String
    );

INT
StringCompare (
    CONST VOID* Ptr1,
    CONST VOID* Ptr2
    );


