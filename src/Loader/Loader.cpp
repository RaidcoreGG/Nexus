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
                        AddonDefs[path] = 1;
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
        /*for (std::map<std::filesystem::path, int>::reverse_iterator it = AddonDefs.rbegin(); it != AddonDefs.rend(); ++it)
        {
            bool stillInDir = false;
            for (std::filesystem::path dllPath : dirDLLs)
            {
                if (it->first == dllPath)
                {
                    stillInDir = true;
                    break;
                }
            }

            if (!stillInDir)
            {
                Logger->LogInfo("Unloaded addon: %s", it->first.string().c_str());
                AddonDefs.erase(it->first);
            }
        }*/
        AddonsMutex.unlock();

        prevDirDLLs = dirDLLs;
	}
}