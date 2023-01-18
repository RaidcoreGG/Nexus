#include "Loader.h"

#include "../State.h"
#include "../Shared.h"
#include "../Paths.h"

#include <vector>
//#include <iostream>
//#include <string>
//#include <sys/stat.h>

namespace Loader
{
    std::mutex AddonsMutex;
    std::map<std::filesystem::path, AddonDef> AddonDefs;

	void Initialize()
	{
        State::AddonHost = ggState::ADDONS_LOAD;
        State::AddonHost = ggState::ADDONS_READY;
	}

	void Update()
	{
        static std::vector<std::filesystem::path> prevDirDLLs;
        std::vector<std::filesystem::path> dirDLLs;

        for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Path::D_GW2_ADDONS))
        {
            if (entry.is_regular_file())
            {
                std::filesystem::path fsPath = entry.path();
                std::string pathStr = fsPath.string();
                std::string dll = ".dll";
                const char* path = pathStr.c_str();

                /* ends with .dll */
                if (pathStr.size() >= dll.size() && 0 == pathStr.compare(pathStr.size() - dll.size(), dll.size(), dll))
                {
                    AddonsMutex.lock();
                    if (AddonDefs.find(fsPath) == AddonDefs.end())
                    {
                        Logger->LogInfo("Loaded addon: %s", path);
                        AddonDef addon = AddonDef{};
                        addon.Author = L"[[AUTHOR]]";
                        addon.Description = L"[[DESCRIPTION]]";
                        addon.Name = L"[[NAME]]";
                        addon.Signature = rand() % 0xFFFFFFFF;
                        addon.Version = L"[[VERSION]]";
                        AddonDefs.insert({ path, addon });

                        Logger->LogDebug(L"%d", addon.Signature);
                    }
                    AddonsMutex.unlock();
                    dirDLLs.push_back(fsPath);
                }
            }
        }

        AddonsMutex.lock();
        if (dirDLLs != prevDirDLLs)
        {
            for (std::filesystem::path dllPath : prevDirDLLs)
            {
                if (std::find(dirDLLs.begin(), dirDLLs.end(), dllPath) == dirDLLs.end())
                {
                    Logger->LogInfo("Unloaded addon: %s", dllPath.string().c_str());
                    AddonDefs.erase(dllPath.string().c_str());
                }
            }
        }
        AddonsMutex.unlock();

        prevDirDLLs = dirDLLs;
	}
}