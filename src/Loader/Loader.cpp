#include "Loader.h"

#include "../State.h"
#include "../Shared.h"
#include "../Paths.h"

#include <vector>
#include <thread>

namespace Loader
{
    std::mutex AddonsMutex;
    std::map<std::filesystem::path, AddonDef> AddonDefs;
    std::thread UpdateThread;

	void Initialize()
	{
        State::AddonHost = ggState::ADDONS_LOAD;
        /* load some addons */
        State::AddonHost = ggState::ADDONS_READY;
        UpdateThread = std::thread(Update);
        UpdateThread.detach();
	}

    void Shutdown()
    {
        State::AddonHost = ggState::SHUTDOWN;
    }

    void LoadAddon(const char* aPath)
    {
        Logger->LogInfo("Loaded addon: %s", aPath);
        AddonDef addon = AddonDef{};
        addon.Author = L"[[AUTHOR]]";
        addon.Description = L"[[DESCRIPTION]]";
        addon.Name = L"[[NAME]]";
        addon.Signature = rand() % 0xFFFFFFFF;
        addon.Version = L"[[VERSION]]";
        AddonDefs.insert({ aPath, addon });
    }

    void UnloadAddon(const char* aPath)
    {
        Logger->LogInfo("Unloaded addon: %s", aPath);
        AddonDefs.erase(aPath);
    }

	void Update()
	{
        for (;;)
        {
            if (State::AddonHost == ggState::SHUTDOWN) { return; }

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
                            LoadAddon(path);
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
                        UnloadAddon(dllPath.string().c_str());
                    }
                }
            }
            AddonsMutex.unlock();

            prevDirDLLs = dirDLLs;

            Sleep(1000);
        }
	}
}