#pragma once 

#pragma warning(disable : 4238) // References to temporary classes are okay because they are only used as function parameters.
#define NOMINMAX


#include "d3dx12.h" // The D3D12 Helper Library that you downloaded.
#define DML_TARGET_VERSION_USE_LATEST
#include <DirectML.h> // The DirectML header from the Windows SDK.

#include <d3d11_1.h>
#include <d3dcompiler.h>

inline bool IsDMLAvailable()
{

    try {

        ID3D12Device * d3D12Device;

        D3D_FEATURE_LEVEL  featureLevel = D3D_FEATURE_LEVEL_12_1;
        HRESULT result = D3D12CreateDevice(NULL, featureLevel, __uuidof(ID3D12Device), (void**)&d3D12Device);
        if (FAILED(result))
        {
            //printf("cannot create d12 device!\n");
            return false;
        }



        IDMLDevice * dmlDevice;

        DML_CREATE_DEVICE_FLAGS dmlCreateDeviceFlags = DML_CREATE_DEVICE_FLAG_NONE;

        DMLCreateDevice(
            d3D12Device,
            dmlCreateDeviceFlags,
            __uuidof(dmlDevice), 
            reinterpret_cast<void**>(&dmlDevice));

        dmlDevice->Release();

        d3D12Device->Release();


    }
    catch (...)
    {

       // printf("exception!\n");
        return false;
    }

    return true;
}


typedef struct {

    int nGPUID;
    float  nMemCap; //G
}RP_GPUINFO;

inline std::vector <IDXGIAdapter*> QmEnumerateAdapters(void)
{
    IDXGIAdapter* pAdapter;
    std::vector <IDXGIAdapter*> vAdapters;
    IDXGIFactory* pFactory = NULL;


    // Create a DXGIFactory object.
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory)))
    {
        return vAdapters;
    }


    for (UINT i = 0;  pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND;
        ++i)
    {
        vAdapters.push_back(pAdapter);
    }


    if (pFactory)
    {
        pFactory->Release();
    }

    return vAdapters;

}

inline std::string WStringToString(const std::wstring& wstr)
{
    std::string str(wstr.length(), ' ');
    std::copy(wstr.begin(), wstr.end(), str.begin());
    return str;
}

inline RP_GPUINFO  QmGetCapacitableGPU(std::vector <IDXGIAdapter*> vAdapters)
{

    RP_GPUINFO Info = { -1,0 };
    for (size_t n = 0; n < vAdapters.size(); n++)
    {

     
            DXGI_ADAPTER_DESC adapterDesc;
            vAdapters[n]->GetDesc(&adapterDesc);
           std::wstring aa(adapterDesc.Description);
            std::string bb = WStringToString(aa);
            int nPosIntel = bb.find("Intel(R)");
            int nPostMs = bb.find("Microsoft ");
            bool bNotGoodGPU = nPosIntel  > -1 || nPostMs > -1;

            if (!bNotGoodGPU) // not intel
            {
               
     
             
                Info.nGPUID = n;
                Info.nMemCap = adapterDesc.DedicatedVideoMemory / 1024 / 1024; // M
                return Info;

            }


   
    }

    return Info;
}


inline void  QmFreeAdapter(std::vector <IDXGIAdapter*> vAdapters)
{
    for (size_t n = 0; n < vAdapters.size(); n++)
    {
        vAdapters[n]->Release();
    }


}
inline RP_GPUINFO QmSelectGPU()
{


    auto Result = QmEnumerateAdapters();

   auto Info= QmGetCapacitableGPU(Result);

    QmFreeAdapter(Result);

    return Info;


}