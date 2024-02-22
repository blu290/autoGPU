#include "SDK/ADLXHelper/Windows/Cpp/ADLXHelper.h"
#include "SDK/Include/IGPUManualFanTuning.h"
#include "SDK/Include/IGPUManualGFXTuning.h"
#include "SDK/Include/IGPUManualPowerTuning.h"
#include "SDK/Include/IGPUTuning.h"
#include <iostream>
#include <vector>
#include <tuple>
#include <chrono>
#include <thread>

using namespace adlx;
static ADLXHelper g_ADLXHelp;
void ShowFrequencyAndVoltageRange(IADLXManualGraphicsTuning1Ptr manualGFXTuning1);
void printStates(IADLXManualGraphicsTuning1Ptr manualGFXTuning1);
std::vector <std::tuple<adlx_int,adlx_int>> getStates(IADLXManualGraphicsTuning1Ptr manualGFXTuning1);
void SetGPUStates(std::vector<std::tuple<adlx_int,adlx_int>> stateMap,IADLXManualGraphicsTuning1Ptr manualGFXTuning1);
IADLXManualGraphicsTuning1Ptr gpuTuningChecks();

//void SetGPUStates(IADLXManualGraphicsTuning1Ptr manualGFXTuning1);
    //ShowFrequencyAndVoltageRange(manualGFXTuning1);
    //printStates(manualGFXTuning1);
    /*auto fuckItWeBall = getStates(manualGFXTuning1);
    for (const auto& a : fuckItWeBall){
        std::cout << "freq : " <<std::get<0>(a) << " volt: " << std::get<1>(a) << "\n";
    }*/

    /*
    settings we want are: (freq(mhz),volt(mv))
    (800,750),(1375,807),(1950,1080)
    for vram we want 1800mhz
    pwr limit +50%
    fan speed 100%*/

int main(){
    ADLX_RESULT res = ADLX_FAIL;
    res = g_ADLXHelp.Initialize();
    if(!ADLX_SUCCEEDED(res)){
        std::cout << "fail initalis adlx womp womp\n";
        return 1;
    }
    auto manualGFXTuning1 = gpuTuningChecks();
    if (!manualGFXTuning1){
        return 0;
    }

    while (true){
        //std::cout << "sleepin\n";
        std::this_thread::sleep_for(std::chrono::minutes(1));
        auto states = getStates(manualGFXTuning1);
        SetGPUStates(states,manualGFXTuning1);
        states.clear();
        }
    //printStates(manualGFXTuning1);
    //std::cout <<"we are so back\n";
    //ShowFrequencyAndVoltageRange(manualGFXTuning1);
    return 0;
}

void ShowFrequencyAndVoltageRange(IADLXManualGraphicsTuning1Ptr manualGFXTuning1)
{
    ADLX_IntRange  freqRange, voltRange;
    ADLX_RESULT  res = manualGFXTuning1->GetGPUTuningRanges(&freqRange, &voltRange);
    std::cout << "Frequency range: (" << freqRange.minValue
              << ", " << freqRange.maxValue  << ")\n";// << ", return code is: "<< res << "(0 means success)" << std::endl;
    std::cout << "Voltage range: (" << voltRange.minValue
              << ", " << voltRange.maxValue  << ")\n";// << ", return code is: "<< res << "(0 means success)" << std::endl;
}

void printStates(IADLXManualGraphicsTuning1Ptr manualGFXTuning1)
{
    IADLXManualTuningStateListPtr states;
    IADLXManualTuningStatePtr oneState;
    ADLX_RESULT  res = manualGFXTuning1->GetGPUTuningStates(&states);
    if (ADLX_SUCCEEDED  (res))
    {
        for (adlx_uint crt = states->Begin(); crt != states->End(); ++crt)
        {
            states->At(crt, &oneState);
            adlx_int freq = 0, volt = 0;
            res = oneState->GetFrequency(&freq);
            std::cout << "state " << crt << ": frequency is " << freq <<"\n"; 
            res = oneState->GetVoltage(&volt);
            std::cout << "state " << crt << ": voltage is " << volt <<"\n"; 
    }
}
}
std::vector <std::tuple<adlx_int,adlx_int>> getStates(IADLXManualGraphicsTuning1Ptr manualGFXTuning1){
    std::vector<std::tuple<adlx_int,adlx_int>> stateMap;
    IADLXManualTuningStateListPtr states;
    IADLXManualTuningStatePtr oneState;
    ADLX_RESULT  res = manualGFXTuning1->GetGPUTuningStates(&states);
    if (ADLX_SUCCEEDED  (res))
    {
        for (adlx_uint crt = states->Begin(); crt != states->End(); ++crt)
        {
            states->At(crt, &oneState);
            adlx_int freq = 0, volt = 0;
            res = oneState->GetFrequency(&freq);
            //std::cout << "\tThe current state " << crt << ": frequency is " << freq <<"\n";
            res = oneState->GetVoltage(&volt);
            //std::cout << "\tThe current state " << crt << ": voltage is " << volt <<"\n"; 
            stateMap.push_back(std::make_tuple(freq,volt));
        }
    }
    return stateMap;
}
IADLXManualGraphicsTuning1Ptr gpuTuningChecks(){
    IADLXGPUTuningServicesPtr gpuTuningService;
    ADLX_RESULT res = ADLX_FAIL;
    res = g_ADLXHelp.GetSystemServices()->GetGPUTuningServices(&gpuTuningService);
    if (ADLX_FAILED  (res)){
            res = g_ADLXHelp.Terminate ();
            std::cout << "Destroy ADLX res: " << res << std::endl;
            return nullptr;
        }

    IADLXGPUListPtr gpus;
    res = g_ADLXHelp.GetSystemServices()->GetGPUs(&gpus);

    IADLXGPUPtr oneGPU;
    res = gpus -> At(0,&oneGPU);
    if (ADLX_FAILED  (res) || oneGPU == nullptr){std::cout <<"guess you don't have a gpu\n";return nullptr;}

    adlx_bool supported = false;
    res = gpuTuningService->IsSupportedManualGFXTuning(oneGPU, &supported);
    if (ADLX_FAILED  (res) || supported == false){std::cout <<"guess your gpu isn't supporting manual tuning\n";return nullptr;}

    IADLXInterfacePtr manualGFXTuningIfc;
    res = gpuTuningService->GetManualGFXTuning(oneGPU, &manualGFXTuningIfc);
    if (ADLX_FAILED  (res) || manualGFXTuningIfc == nullptr)
        {
            // Destroy ADLX
            std::cout << "interface failure:(\n";
            res = g_ADLXHelp.Terminate ();
            return nullptr;
        }
    IADLXManualGraphicsTuning1Ptr manualGFXTuning1(manualGFXTuningIfc);
    //IADLXManualGraphicsTuning2Ptr manualGFXTuning2(manualGFXTuningIfc);   this one isn't needed because this is for POST NAVI even tho i have NAVI WTF IS THIS 
    if (manualGFXTuning1 == nullptr) //&& manualGFXTuning2 == nullptr)
        {
            // Destroy ADLX
            std::cout <<"failed to interface lol \n";
            res = g_ADLXHelp.Terminate ();
            return nullptr;
        }
    return manualGFXTuning1;
}
void SetGPUStates(std::vector<std::tuple<adlx_int,adlx_int>> stateMap,IADLXManualGraphicsTuning1Ptr manualGFXTuning1){
    IADLXManualTuningStateListPtr states;
    IADLXManualTuningStatePtr oneState;
    ADLX_RESULT  res1 = manualGFXTuning1->GetEmptyGPUTuningStates(&states);
    ADLX_RESULT  res;
    ADLX_IntRange  freqRange, voltRange;
    ADLX_RESULT  res2 = manualGFXTuning1->GetGPUTuningRanges(&freqRange, &voltRange);

    if (ADLX_SUCCEEDED  (res1) && ADLX_SUCCEEDED  (res2)){
        //if the states are met
        if (stateMap.size() == 3 &&( std::get<0>(stateMap[0]) == 800 && std::get<1>(stateMap[0]) <= 750) &&( (std::get<0>(stateMap[1]) >= 1400 && std::get<0>(stateMap[1]) <= 1420) && (std::get<1>(stateMap[1]) <= 840) && std::get<1>(stateMap[1]) >= 810)&&( std::get<0>(stateMap[2]) >=2035 && std::get<0>(stateMap[2]) <=2060 && std::get<1>(stateMap[2]) >= 1185 && std::get<1>(stateMap[2]) <= 1200)){
                //we were going to use the vector thing earlier and genuinely that would make more sense but i saw the example here and couldn't get my way working so we move
                for (adlx_uint crt = states->Begin(); crt != states->End(); ++crt){
                    states->At(crt, &oneState);
                    adlx_int freq = 0, volt = 0;
                    if (crt == 0){
                        res=oneState->SetFrequency(800);
                        res=oneState->GetFrequency(&freq);
                        res=oneState->SetVoltage(750);
                        res=oneState->GetVoltage(&volt);
                    }
                    else if (crt == 1){
                        res=oneState->SetFrequency(1375);
                        res=oneState->GetFrequency(&freq);
                        res=oneState->SetVoltage(834);
                        res=oneState->GetVoltage(&volt);
                    }
                    else if (crt == 2){
                        res=oneState->SetFrequency(1950);
                        res=oneState->GetFrequency(&freq);
                        res=oneState->SetVoltage(1080);
                        res=oneState->GetVoltage(&volt);
                    }

                    //std::cout << "Set empty state " << crt << ": frequency is " << freq << ", voltage is " << volt << "\n";
        }
            adlx_int errorIndex;
            ADLX_RESULT  res = manualGFXTuning1->IsValidGPUTuningStates(states, &errorIndex);
            if (ADLX_SUCCEEDED (res)){
                res = manualGFXTuning1->SetGPUTuningStates(states);
                std::cout << "Set GPU states " << (ADLX_SUCCEEDED  (res) ? "succeeded" : "failed") << std::endl;
            }
            //res = manualGFXTuning1->GetGPUTuningStates(&states);

        }
    }

}
