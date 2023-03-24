#define COBJMACROS
#define CINTERFACE
#include <Windows.h>
#include <strsafe.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <dia2.h>
#include "c2b.h"

//#pragma comment(lib, "diaguids.lib")
// Symbols/Functions to search in pdb

#if 0
//#define PDBUTIL_COMMANDLINE L" pretty -no-compiler-generated -no-system-libs -sym-types=all  <withnames> <pdbpath>";
#define PDBUTIL_COMMANDLINE L" pretty -no-compiler-generated -no-system-libs -sym-types=all  ";
#endif

IDiaDataSource *DiaDataSource = NULL;
static BOOLEAN DiaCOMInitialized = FALSE;

DWORD
InitSymbolParser (
    PCONFIG_DATA ConfigData
)
{

    DWORD Status;

    Status = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER(ConfigData);
    if (FAILED(CoInitialize(NULL))) {
        Status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    DiaCOMInitialized = TRUE;

Exit:
    return Status;
}

VOID
FreeSymbolParser (
    VOID
    )
{

    if (DiaCOMInitialized != FALSE) {
        CoUninitialize();
        DiaCOMInitialized = FALSE;
    }
}

DWORD
FindSymbolsInternal (
    _In_ PBIN_PDB BinPdb,
    _In_ LPCWSTR* FunctionNames,
    _In_ DWORD NumFunctionNames
    )

{
    //DWORD tag;
    BSTR Name;
    HRESULT Hr;
    IDiaEnumSymbols* DiaEnumSymbols;
    IDiaSymbol* DiaSymbol;
    ULONG Retrived;
    DWORD Status;
    IDiaSession *DiaSession;
    IDiaSymbol *DiaGlobalSymbol;
    PVOID Found;

    Status = ERROR_SUCCESS;
    DiaSession = NULL;
    DiaGlobalSymbol = NULL;
    DiaEnumSymbols = NULL;
    DiaSymbol = NULL;

    Hr = CoCreateInstance(&CLSID_DiaSource,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_IDiaDataSource,
                          (void **)&DiaDataSource);

    if (FAILED(Hr)) {
        Status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    Hr = IDiaDataSource_loadDataFromPdb(DiaDataSource, BinPdb->Pdb);
    //Hr = DiaDataSource->loadDataFromPdb(BinPdb->Pdb);
    if (FAILED(Hr)) {
        Status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    // Open a session for querying symbols
    Hr = IDiaDataSource_openSession(DiaDataSource, &DiaSession);
    if (FAILED(Hr)) {
        Status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    // Retrieve a reference to the global scope
    Hr = IDiaSession_get_globalScope(DiaSession, &DiaGlobalSymbol);
    if (FAILED(Hr)) {
        Status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    // Find all children symbols in the global scope
    Hr = IDiaSymbol_findChildren(DiaGlobalSymbol,
                                SymTagNull,
                                NULL,
                                nsNone,
                                &DiaEnumSymbols);

    if (FAILED(Hr)) {
        Status = ERROR_INVALID_FUNCTION;
        goto Exit;
    }

    BinPdb->Impacted = FALSE;
    while ((IDiaEnumSymbols_Next(DiaEnumSymbols, 1, &DiaSymbol, &Retrived) == S_OK) &&
            (Retrived == 1)) {

        //DiaSymbol->get_symTag(&tag);
        if (SUCCEEDED(IDiaSymbol_get_name(DiaSymbol, &Name))) {
            if (Name != NULL) {
                //wprintf(L"\nSymbol name: %s", Name);
                Found = bsearch(&Name,
                                FunctionNames,
                                NumFunctionNames,
                                sizeof(LPCWSTR),
                                StringCompare);

                if (Found != NULL) {
                    wprintf(L"\n%s", Name);
                    BinPdb->Impacted = TRUE;
                }

                SysFreeString(Name);
                Name = NULL;

                if (BinPdb->Impacted) {
                    break;
                }
            }
        }

        IDiaSymbol_Release(DiaSymbol);
        DiaSymbol = NULL;
        Retrived = 0;
    }

Exit:
    if (DiaEnumSymbols != NULL) {
        IDiaEnumSymbols_Release(DiaEnumSymbols);
        DiaEnumSymbols = NULL;
    }

    if (DiaGlobalSymbol != NULL) {
        IDiaSymbol_Release(DiaGlobalSymbol);
        DiaGlobalSymbol = NULL;
    }

    if (DiaSession != NULL) {
        IDiaSession_Release(DiaSession);
        DiaSession = NULL;
    }

    if (DiaDataSource) {
        IDiaDataSource_Release(DiaDataSource);
        DiaDataSource = NULL;
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

        Status = FindSymbolsInternal(BinPdb,
                                    Artifact->ConfigData->FunctionNames,
                                    Artifact->ConfigData->NumFunctionNames);

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

