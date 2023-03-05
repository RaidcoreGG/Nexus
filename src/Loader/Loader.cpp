#include "Loader.h"

namespace Loader
{
    std::mutex Mutex;
    std::map<std::filesystem::path, LoadedAddon> AddonDefs;
    AddonAPI APIDef{};

    std::thread UpdateThread;

    std::set<std::filesystem::path> ExistingLibs;
    std::set<std::filesystem::path> Blacklist;

	void Initialize()
	{
        State::AddonHost = ggState::ADDONS_LOAD;

        /* setup APIDefs */
        APIDef.SwapChain = Renderer::SwapChain;
        APIDef.ImguiContext = Renderer::GuiContext;
        APIDef.WindowWidth = &Renderer::Width;
        APIDef.WindowHeight = &Renderer::Height;

        APIDef.CreateHook = MH_CreateHook;
        APIDef.RemoveHook = MH_RemoveHook;
        APIDef.EnableHook = MH_EnableHook;
        APIDef.DisableHook = MH_DisableHook;

        APIDef.Log = LogMessageA;
        APIDef.RegisterLogger = RegisterLogger;
        APIDef.UnregisterLogger = UnregisterLogger;

        APIDef.RaiseEvent = Events::Raise;
        APIDef.SubscribeEvent = Events::Subscribe;
        APIDef.UnsubscribeEvent = Events::Unsubscribe;

        APIDef.RegisterKeybind = Keybinds::Register;
        APIDef.UnregisterKeybind = Keybinds::Unregister;

        APIDef.GetResource = DataLink::GetResource;
        APIDef.ShareResource = DataLink::ShareResource;

        APIDef.GetTexture = TextureLoader::Get;
        APIDef.LoadTextureFromFile = TextureLoader::LoadFromFile;
        APIDef.LoadTextureFromResource = TextureLoader::LoadFromResource;

        State::AddonHost = ggState::ADDONS_READY;

        UpdateThread = std::thread(Update);
        UpdateThread.detach();
	}

    void Shutdown()
    {
        State::AddonHost = ggState::ADDONS_SHUTDOWN;
    }

    void LoadAddon(std::filesystem::path aPath)
    {
        std::string pathStr = aPath.string();
        const char* path = pathStr.c_str();
        GETADDONDEF getAddonDef = 0;
        HMODULE hMod = LoadLibraryA(path);

        /* lib load failed */
        if (!hMod)
        {
            Blacklist.insert(aPath);
            return;
        }

        getAddonDef = (GETADDONDEF)GetProcAddress(hMod, "GetAddonDef");
        if (getAddonDef == 0)
        {
            Blacklist.insert(aPath);
            FreeLibrary(hMod);
            return;
        }

        AddonDefinition* defs = getAddonDef();
        if (hMod && !defs->HasMinimumRequirements())
        {
            LogWarning("Addon loading cancelled. %s does not fulfill minimum requirements. At least define Name, Version, Author, Description as well as Load and Unload functions.", path);

            Blacklist.insert(aPath);
            FreeLibrary(hMod);
            return;
        }

        LoadedAddon addon{ hMod, defs };

        AddonDefs.insert({ aPath, addon });

        addon.Definitions->Load(APIDef);

        LogInfo("Loaded addon: %s", path);
    }

    void UnloadAddon(std::filesystem::path aPath)
    {
        std::string pathStr = aPath.string();
        const char* path = pathStr.c_str();

        AddonDefs[aPath].Definitions->Unload();

        if (AddonDefs[aPath].Module)
        {
            if (!FreeLibrary(AddonDefs[aPath].Module))
            {
                LogWarning("Couldn't unload %s ", path);
                return;
            }
        }

        AddonDefs[aPath].Definitions = nullptr;
        AddonDefs[aPath].Module = nullptr;
        AddonDefs[aPath] = {};

        AddonDefs.erase(aPath);

        LogInfo("Unloaded addon: %s", path);
    }

	void Update()
	{
        for (;;)
        {
            if (State::AddonHost == ggState::ADDONS_SHUTDOWN) { return; }

            std::set<std::filesystem::path> currentLibs;

            for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(Path::D_GW2_ADDONS))
            {
                if (entry.is_regular_file())
                {
                    std::filesystem::path fsPath = entry.path();
                    std::string pathStr = fsPath.string();
                    std::string dll = ".dll";

                    /* ends with .dll */
                    if (pathStr.size() >= dll.size() && 0 == pathStr.compare(pathStr.size() - dll.size(), dll.size(), dll))
                    {
                        Mutex.lock();
                        if (AddonDefs.find(fsPath) == AddonDefs.end())
                        {
                            /* reload if not blacklisted */
                            if (Blacklist.find(fsPath) == Blacklist.end())
                            {
                                LoadAddon(fsPath);
                            }
                        }
                        Mutex.unlock();
                        currentLibs.insert(fsPath);
                    }
                }
            }

            Mutex.lock();
            if (currentLibs != ExistingLibs)
            {
                for (std::filesystem::path dllPath : ExistingLibs)
                {
                    if (currentLibs.find(dllPath) == currentLibs.end())
                    {
                        if (AddonDefs.find(dllPath) != AddonDefs.end())
                        {
                            UnloadAddon(dllPath);
                        }
                    }
                }
            }
            Mutex.unlock();

            ExistingLibs = currentLibs;

            Sleep(5000);
        }
	}
}