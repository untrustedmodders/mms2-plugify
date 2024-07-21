/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Sample Plugin
 * Written by AlliedModders LLC.
 * ======================================================
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from 
 * the use of this software.
 *
 * This sample plugin is public domain.
 */

#include "mm_plugin.h"

#include <igameevents.h>
#include <iserver.h>

#include <plugify/compat_format.h>
#include <plugify/plugify.h>
#include <plugify/plugin.h>
#include <plugify/module.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/plugin_reference_descriptor.h>
#include <plugify/language_module_descriptor.h>
#include <plugify/package.h>
#include <plugify/plugin_manager.h>
#include <plugify/package_manager.h>

#include <filesystem>
#include <chrono>

std::string FormatTime(std::string_view format = "%Y-%m-%d %H:%M:%S")
{
	auto now = std::chrono::system_clock::now();
	auto timeT = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&timeT), format.data());
	return ss.str();
}

void RegisterTags(LoggingChannelID_t channelID)
{
}

namespace plugifyMM
{
	IServerGameDLL *server = NULL;
	IServerGameClients *gameclients = NULL;
	IVEngineServer *engine = NULL;
	IGameEventManager2 *gameevents = NULL;
	ICvar *icvar = NULL;

	PlugifyMMPlugin g_Plugin;
	PLUGIN_EXPOSE(PlugifyMMPlugin, g_Plugin);

	#define CONPRINT(x) g_Plugin.m_logger->Message(x)
	#define CONPRINTE(x) g_Plugin.m_logger->Warning(x)
	#define CONPRINTF(...) g_Plugin.m_logger->MessageFormat(__VA_ARGS__)

	template <typename S, typename T, typename F>
	void Print(const T &t, F &f, std::string_view tab = "  ")
	{
		std::string result(tab);
		if (t.GetState() != S::Loaded)
		{
			std::format_to(std::back_inserter(result), "[{:02d}] <{}> {}", t.GetId(), f(t.GetState()), t.GetFriendlyName());
		}
		else
		{
			std::format_to(std::back_inserter(result), "[{:02d}] {}", t.GetId(), t.GetFriendlyName());
		}
		auto versionName = t.GetDescriptor().GetVersionName();
		if (!versionName.empty())
		{
			std::format_to(std::back_inserter(result), " ({})", versionName);
		}
		else
		{
			std::format_to(std::back_inserter(result), " (v{})", t.GetDescriptor().GetVersion());
		}
		auto createdBy = t.GetDescriptor().GetCreatedBy();
		if (!createdBy.empty())
		{
			std::format_to(std::back_inserter(result), " by {}", createdBy);
		}
		std::format_to(std::back_inserter(result), "\n");
		CONPRINT(result.c_str());
	}

	template <typename S, typename T, typename F>
	void Print(const char *name, const T &t, F &f)
	{
		if (t.GetState() == S::Error)
		{
			CONPRINTF("%s has error: %s.\n", name, t.GetError().data());
		}
		else
		{
			CONPRINTF("%s %zd is %s.\n", name, t.GetId(), f(t.GetState()).data());
		}
		auto getCreatedBy = t.GetDescriptor().GetCreatedBy();
		if (!getCreatedBy.empty())
		{
			CONPRINTF("  Name: \"%s\" by %s\n", t.GetFriendlyName().data(), getCreatedBy.data());
		}
		else
		{
			CONPRINTF("  Name: \"%s\"\n", t.GetFriendlyName().data());
		}
		auto versionName = t.GetDescriptor().GetVersionName();
		if (!versionName.empty())
		{
			CONPRINTF("  Version: %s\n", versionName.data());
		}
		else
		{
			CONPRINTF("  Version: %d\n", t.GetDescriptor().GetVersion());
		}
		auto description = t.GetDescriptor().GetDescription();
		if (!description.empty())
		{
			CONPRINTF("  Description: %s\n", description.data());
		}
		auto createdByURL = t.GetDescriptor().GetCreatedByURL();
		if (!createdByURL.empty())
		{
			CONPRINTF("  URL: %s\n", createdByURL.data());
		}
		auto docsURL = t.GetDescriptor().GetDocsURL();
		if (!docsURL.empty())
		{
			CONPRINTF("  Docs: %s\n", docsURL.data());
		}
		auto downloadURL = t.GetDescriptor().GetDownloadURL();
		if (!downloadURL.empty())
		{
			CONPRINTF("  Download: %s\n", downloadURL.data());
		}
		auto updateURL = t.GetDescriptor().GetUpdateURL();
		if (!updateURL.empty())
		{
			CONPRINTF("  Update: %s\n", updateURL.data());
		}
	}

	ptrdiff_t FormatInt(const std::string &str)
	{
		try
		{
			size_t pos;
			ptrdiff_t result = std::stoul(str, &pos);
			if (pos != str.length())
			{
				throw std::invalid_argument("Trailing characters after the valid part");
			}
			return result;
		}
		catch (const std::invalid_argument &e)
		{
			CONPRINTF("Invalid argument: %s", e.what());
		}
		catch (const std::out_of_range &e)
		{
			CONPRINTF("Out of range: %s", e.what());
		}
		catch (const std::exception &e)
		{
			CONPRINTF("Conversion error: %s", e.what());
		}

		return ptrdiff_t(-1);
	}

	CON_COMMAND_F(plugify, "Plugify control options", FCVAR_NONE)
	{
		std::vector<std::string> arguments;
		std::unordered_set<std::string> options;
		std::span view(args.ArgV(), args.ArgC());
		arguments.reserve(view.size());
		for (size_t i = 0; i < view.size(); i++)
		{
			std::string str(view[i]);
			if (i > 1 && str.starts_with("-"))
			{
				options.emplace(std::move(str));
			}
			else
			{
				arguments.emplace_back(std::move(str));
			}
		}

		auto &plugify = g_Plugin.m_context;
		if (!plugify)
			return; // Should not trigger!

		auto packageManager = plugify->GetPackageManager().lock();
		auto pluginManager = plugify->GetPluginManager().lock();
		if (!packageManager || !pluginManager)
			return; // Should not trigger!

		if (arguments.size() > 1)
		{

			if (arguments[1] == "help" || arguments[1] == "-h")
			{
				CONPRINT("Plugify Menu\n");
				CONPRINT("(c) untrustedmodders\n");
				CONPRINT("https://github.com/untrustedmodders\n");
				CONPRINT("usage: plugify <command> [options] [arguments]\n");
				CONPRINT("  help           - Show help\n");
				CONPRINT("  version        - Version information\n");
				CONPRINT("Plugin Manager commands:\n");
				CONPRINT("  load           - Load plugin manager\n");
				CONPRINT("  unload         - Unload plugin manager\n");
				CONPRINT("  modules        - List running modules\n");
				CONPRINT("  plugins        - List running plugins\n");
				CONPRINT("  plugin <name>  - Show information about a module\n");
				CONPRINT("  module <name>  - Show information about a plugin\n");
				CONPRINT("Plugin Manager options:\n");
				CONPRINT("  -h, --help     - Show help\n");
				CONPRINT("  -u, --uuid     - Use index instead of name\n");
				CONPRINT("Package Manager commands:\n");
				CONPRINT("  install <name> - Packages to install (space separated)\n");
				CONPRINT("  remove <name>  - Packages to remove (space separated)\n");
				CONPRINT("  update <name>  - Packages to update (space separated)\n");
				CONPRINT("  list           - Print all local packages\n");
				CONPRINT("  query          - Print all remote packages\n");
				CONPRINT("  show  <name>   - Show information about local package\n");
				CONPRINT("  search <name>  - Search information about remote package\n");
				CONPRINT("  snapshot       - Snapshot packages into manifest\n");
				CONPRINT("  repo <url>     - Add repository to config\n");
				CONPRINT("Package Manager options:\n");
				CONPRINT("  -h, --help     - Show help\n");
				CONPRINT("  -a, --all      - Install/remove/update all packages\n");
				CONPRINT("  -f, --file     - Packages to install (from file manifest)\n");
				CONPRINT("  -l, --link     - Packages to install (from HTTP manifest)\n");
				CONPRINT("  -m, --missing  - Install missing packages\n");
				CONPRINT("  -c, --conflict - Remove conflict packages\n");
				CONPRINT("  -i, --ignore   - Ignore missing or conflict packages\n");
			}

			else if (arguments[1] == "version" || arguments[1] == "-v")
			{
				static std::string copyright = std::format("Copyright (C) 2023-{}{}{}{} Untrusted Modders Team\n", __DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10]);
				CONPRINT(R"(      ____)" "\n");
				CONPRINT(R"( ____|    \         Plugify v)" PLUGIFY_PROJECT_VERSION "\n");
				CONPRINT(R"((____|     `._____  )");
				CONPRINTF("%s\n", copyright.c_str());
				CONPRINT(R"( ____|       _|___)" "\n");
				CONPRINT(R"((____|     .'       This program may be freely redistributed under)" "\n");
				CONPRINT(R"(     |____/         the terms of the GNU General Public License.)" "\n");
			}

			else if (arguments[1] == "load")
			{
				if (!options.contains("--ignore") && !options.contains("-i"))
				{
					if (packageManager->HasMissedPackages())
					{
						CONPRINT("Plugin manager has missing packages, run 'install --missing' to resolve issues.");
						return;
					}
					if (packageManager->HasConflictedPackages())
					{
						CONPRINT("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
						return;
					}
				}
				if (pluginManager->IsInitialized())
				{
					CONPRINT("Plugin manager already loaded.");
				}
				else
				{
					pluginManager->Initialize();
					CONPRINT("Plugin manager was loaded.");
				}
			}

			else if (arguments[1] == "unload")
			{
				if (!pluginManager->IsInitialized())
				{
					CONPRINT("Plugin manager already unloaded.");
				}
				else
				{
					pluginManager->Terminate();
					CONPRINT("Plugin manager was unloaded.");
				}
			}

			else if (arguments[1] == "plugins")
			{
				if (!pluginManager->IsInitialized())
				{
					CONPRINT("You must load plugin manager before query any information from it.");
					return;
				}
				auto count = pluginManager->GetPlugins().size();
				if (!count)
				{
					CONPRINT("No plugins loaded.\n");
				}
				else
				{
					CONPRINTF("Listing %d plugin%s:\n", static_cast<int>(count), (count > 1) ? "s" : "");
				}
				for (auto &plugin : pluginManager->GetPlugins())
				{
					Print<plugify::PluginState>(plugin, plugify::PluginUtils::ToString);
				}
			}

			else if (arguments[1] == "modules")
			{
				if (!pluginManager->IsInitialized())
				{
					CONPRINT("You must load plugin manager before query any information from it.");
					return;
				}
				auto count = pluginManager->GetModules().size();
				if (!count)
				{
					CONPRINT("No modules loaded.\n");
				}
				else
				{
					CONPRINTF("Listing %d module%s:\n", static_cast<int>(count), (count > 1) ? "s" : "");
				}
				for (auto &module : pluginManager->GetModules())
				{
					Print<plugify::ModuleState>(module, plugify::ModuleUtils::ToString);
				}
			}

			else if (arguments[1] == "plugin")
			{
				if (arguments.size() > 2)
				{
					if (!pluginManager->IsInitialized())
					{
						CONPRINT("You must load plugin manager before query any information from it.");
						return;
					}
					auto plugin = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindPluginFromId(FormatInt(arguments[2])) : pluginManager->FindPlugin(arguments[2]);
					if (plugin.has_value())
					{
						Print<plugify::PluginState>("Plugin", *plugin, plugify::PluginUtils::ToString);
						CONPRINTF("  Language module: %s\n", plugin->GetDescriptor().GetLanguageModule().data());
						CONPRINT("  Dependencies: \n");
						for (const auto &reference : plugin->GetDescriptor().GetDependencies())
						{
							auto dependency = pluginManager->FindPlugin(reference.GetName());
							if (dependency.has_value())
							{
								Print<plugify::PluginState>(*dependency, plugify::PluginUtils::ToString, "    ");
							}
							else
							{
								CONPRINTF("    %s <Missing> (v%s)", reference.GetName().data(), reference.GetRequestedVersion().has_value() ? std::to_string(*reference.GetRequestedVersion()).c_str() : "[latest]");
							}
						}
						CONPRINTF("  File: %s\n\n", plugin->GetDescriptor().GetEntryPoint().data());
					}
					else
					{
						CONPRINTF("Plugin %s not found.\n", arguments[2].c_str());
					}
				}
				else
				{
					CONPRINT("You must provide name.\n");
				}
			}

			else if (arguments[1] == "module")
			{
				if (arguments.size() > 2)
				{
					if (!pluginManager->IsInitialized())
					{
						CONPRINT("You must load plugin manager before query any information from it.");
						return;
					}
					auto module = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindModuleFromId(FormatInt(arguments[2])) : pluginManager->FindModule(arguments[2]);
					if (module.has_value())
					{
						Print<plugify::ModuleState>("Module", *module, plugify::ModuleUtils::ToString);
						CONPRINTF("  Language: %s\n", module->GetDescriptor().GetLanguage().data());
						CONPRINTF("  File: %s\n\n", module->GetFilePath().string().c_str());
					}
					else
					{
						CONPRINTF("Module %s not found.\n", arguments[2].c_str());
					}
				}
				else
				{
					CONPRINT("You must provide name.\n");
				}
			}

			else if (arguments[1] == "snapshot")
			{
				if (pluginManager->IsInitialized())
				{
					CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				packageManager->SnapshotPackages(plugify->GetConfig().baseDir / std::format("snapshot_{}.wpackagemanifest", FormatTime("%Y_%m_%d_%H_%M_%S")), true);
			}

			else if (arguments[1] == "repo")
			{
				if (pluginManager->IsInitialized())
				{
					CONPRINT("You must unload plugin manager before bring any change with package manager.");
					return;
				}

				if (arguments.size() > 2)
				{
					bool success = false;
					for (const auto &repository : std::span(arguments.begin() + 2, arguments.size() - 2))
					{
						success |= plugify->AddRepository(repository);
					}
					if (success)
					{
						packageManager->Reload();
					}
				}
				else
				{
					CONPRINT("You must give at least one repository to add.");
				}
			}

			else if (arguments[1] == "install")
			{
				if (pluginManager->IsInitialized())
				{
					CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				if (options.contains("--missing") || options.contains("-m"))
				{
					if (packageManager->HasMissedPackages())
					{
						packageManager->InstallMissedPackages();
					}
					else
					{
						CONPRINT("No missing packages were found.\n");
					}
				}
				else
				{
					if (arguments.size() > 2)
					{
						if (options.contains("--link") || options.contains("-l"))
						{
							packageManager->InstallAllPackages(arguments[2], arguments.size() > 3);
						}
						else if (options.contains("--file") || options.contains("-f"))
						{
							packageManager->InstallAllPackages(std::filesystem::path{ arguments[2] }, arguments.size() > 3);
						}
						else
						{
							packageManager->InstallPackages(std::span(arguments.begin() + 2, arguments.size() - 2));
						}
					}
					else
					{
						CONPRINT("You must give at least one requirement to install.\n");
					}
				}
			}

			else if (arguments[1] == "remove")
			{
				if (pluginManager->IsInitialized())
				{
					CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				if (options.contains("--all") || options.contains("-a"))
				{
					packageManager->UninstallAllPackages();
				}
				else if (options.contains("--conflict") || options.contains("-c"))
				{
					if (packageManager->HasConflictedPackages())
					{
						packageManager->UninstallConflictedPackages();
					}
					else
					{
						CONPRINT("No conflicted packages were found.\n");
					}
				}
				else
				{
					if (arguments.size() > 2)
					{
						packageManager->UninstallPackages(std::span(arguments.begin() + 2, arguments.size() - 2));
					}
					else
					{
						CONPRINT("You must give at least one requirement to remove.\n");
					}
				}
			}

			else if (arguments[1] == "update")
			{
				if (pluginManager->IsInitialized())
				{
					CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				if (options.contains("--all") || options.contains("-a"))
				{
					packageManager->UpdateAllPackages();
				}
				else
				{
					if (arguments.size() > 2)
					{
						packageManager->UpdatePackages(std::span(arguments.begin() + 2, arguments.size() - 2));
					}
					else
					{
						CONPRINT("You must give at least one requirement to update.\n");
					}
				}
			}

			else if (arguments[1] == "list")
			{
				if (pluginManager->IsInitialized())
				{
					CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				auto count = packageManager->GetLocalPackages().size();
				if (!count)
				{
					CONPRINT("No local packages found.\n");
				}
				else
				{
					CONPRINTF("Listing %d local package%s:\n", static_cast<int>(count), (count > 1) ? "s" : "");
				}
				for (auto &localPackage : packageManager->GetLocalPackages())
				{
					CONPRINTF("  %s [%s] (v%d) at %s\n", localPackage.name.c_str(), localPackage.type.c_str(), localPackage.version, localPackage.path.string().c_str());
				}
			}

			else if (arguments[1] == "query")
			{
				if (pluginManager->IsInitialized())
				{
					CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				auto count = packageManager->GetRemotePackages().size();
				if (!count)
				{
					CONPRINT("No remote packages found.\n");
				}
				else
				{
					CONPRINTF("Listing %d remote package%s:\n", static_cast<int>(count), (count > 1) ? "s" : "");
				}
				for (auto &remotePackage : packageManager->GetRemotePackages())
				{
					if (remotePackage.author.empty() || remotePackage.description.empty())
					{
						CONPRINTF("  %s [%s]\n", remotePackage.name.c_str(), remotePackage.type.c_str());
					}
					else
					{
						CONPRINTF("  %s [%s] (%s) by %s\n", remotePackage.name.c_str(), remotePackage.type.c_str(), remotePackage.description.c_str(), remotePackage.author.c_str());
					}
				}
			}

			else if (arguments[1] == "show")
			{
				if (pluginManager->IsInitialized())
				{
					CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				if (arguments.size() > 2)
				{
					auto package = packageManager->FindLocalPackage(arguments[2]);
					if (package.has_value())
					{
						CONPRINTF("  Name: %s\n", package->name.c_str());
						CONPRINTF("  Type: %s\n", package->type.c_str());
						CONPRINTF("  Version: %d\n", package->version);
						CONPRINTF("  File: %s\n\n", package->path.string().c_str());
					}
					else
					{
						CONPRINTF("Package %s not found.\n", arguments[2].c_str());
					}
				}
				else
				{
					CONPRINT("You must provide name.\n");
				}
			}

			else if (arguments[1] == "search")
			{
				if (pluginManager->IsInitialized())
				{
					CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				if (arguments.size() > 2)
				{
					auto package = packageManager->FindRemotePackage(arguments[2]);
					if (package.has_value())
					{
						CONPRINTF("  Name: %s\n", package->name.c_str());
						CONPRINTF("  Type: %s\n", package->type.c_str());
						if (!package->author.empty())
						{
							CONPRINTF("  Author: %s\n", package->author.c_str());
						}
						if (!package->description.empty())
						{
							CONPRINTF("  Description: %s\n", package->description.c_str());
						}
						if (!package->versions.empty())
						{
							std::string versions("  Versions: ");
							std::format_to(std::back_inserter(versions), "{}", package->versions.begin()->version);
							for (auto it = std::next(package->versions.begin()); it != package->versions.end(); ++it)
							{
								std::format_to(std::back_inserter(versions), ", {}", it->version);
							}
							std::format_to(std::back_inserter(versions), "\n\n");
							CONPRINT(versions.c_str());
						}
						else
						{
							CONPRINT("\n");
						}
					}
					else
					{
						CONPRINTF("Package %s not found.\n", arguments[2].c_str());
					}
				}
				else
				{
					CONPRINT("You must provide name.\n");
				}
			}

			else
			{
				CONPRINTF("unknown option: %s\n", arguments[1].c_str());
				CONPRINT("usage: plugify <command> [options] [arguments]\n");
				CONPRINT("Try plugify help or -h for more information.\n");
			}
		}
		else
		{
			CONPRINT("usage: plugify <command> [options] [arguments]\n");
			CONPRINT("Try plugify help or -h for more information.\n");
		}
	}

	bool PlugifyMMPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
	{
		PLUGIN_SAVEVARS();

		GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
		GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
		GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
		GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
		GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

		g_SMAPI->AddListener(this, &m_listener);

		g_pCVar = icvar;
		ConVar_Register(FCVAR_RELEASE | FCVAR_SERVER_CAN_EXECUTE | FCVAR_GAMEDLL);

		m_context = plugify::MakePlugify();

		m_logger = std::make_shared<MMLogger>("plugify", &RegisterTags);
		m_logger->SetSeverity(plugify::Severity::Info);
		m_context->SetLogger(m_logger);

		std::filesystem::path rootDir(Plat_GetGameDirectory());
		auto result = m_context->Initialize(rootDir / "csgo");
		if (result)
		{
			m_logger->SetSeverity(m_context->GetConfig().logSeverity);

			if (auto packageManager = m_context->GetPackageManager().lock())
			{
				packageManager->Initialize();

				if (packageManager->HasMissedPackages())
				{
					CONPRINT("Plugin manager has missing packages, run 'update --missing' to resolve issues.");
					return true;
				}
				if (packageManager->HasConflictedPackages())
				{
					CONPRINT("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
					return true;
				}
			}

			if (auto pluginManager = m_context->GetPluginManager().lock())
			{
				pluginManager->Initialize();
			}
		}

		return result;
	}

	bool PlugifyMMPlugin::Unload(char *error, size_t maxlen)
	{
		m_context.reset();
		return true;
	}

	void PlugifyMMPlugin::AllPluginsLoaded()
	{
	}

	bool PlugifyMMPlugin::Pause(char *error, size_t maxlen)
	{
		return true;
	}

	bool PlugifyMMPlugin::Unpause(char *error, size_t maxlen)
	{
		return true;
	}

	const char *PlugifyMMPlugin::GetLicense()
	{
		return "Public Domain";
	}

	const char *PlugifyMMPlugin::GetVersion()
	{
		return PLUGIFY_PROJECT_VERSION;
	}

	const char *PlugifyMMPlugin::GetDate()
	{
		return __DATE__;
	}

	const char *PlugifyMMPlugin::GetLogTag()
	{
		return "PLUGIFY";
	}

	const char *PlugifyMMPlugin::GetAuthor()
	{
		return "untrustedmodders";
	}

	const char *PlugifyMMPlugin::GetDescription()
	{
		return PLUGIFY_PROJECT_DESCRIPTION;
	}

	const char *PlugifyMMPlugin::GetName()
	{
		return PLUGIFY_PROJECT_NAME;
	}

	const char *PlugifyMMPlugin::GetURL()
	{
		return PLUGIFY_PROJECT_HOMEPAGE_URL;
	}
} // namespace plugifyMM

SMM_API IMetamodListener *Plugify_ImmListener()
{
	return &plugifyMM::g_Plugin.m_listener;
}

SMM_API ISmmAPI *Plugify_ISmmAPI()
{
	return plugifyMM::g_SMAPI;
}

SMM_API ISmmPlugin *Plugify_ISmmPlugin()
{
	return plugifyMM::g_PLAPI;
}

SMM_API PluginId Plugify_Id()
{
	return plugifyMM::g_PLID;
}

SMM_API SourceHook::ISourceHook *Plugify_SourceHook()
{
	return plugifyMM::g_SHPtr;
}
