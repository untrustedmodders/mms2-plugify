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

#include "mm_plugin.hpp"

#include <igameevents.h>
#include <iserver.h>

#include <plugify/compat_format.hpp>
#include <plugify/plugify.hpp>
#include <plugify/plugin.hpp>
#include <plugify/module.hpp>
#include <plugify/plugin_descriptor.hpp>
#include <plugify/plugin_reference_descriptor.hpp>
#include <plugify/language_module_descriptor.hpp>
#include <plugify/package.hpp>
#include <plugify/plugin_manager.hpp>
#include <plugify/package_manager.hpp>

#include <filesystem>
#include <chrono>
#include <type_traits>

CBaseGameSystemFactory **CBaseGameSystemFactory::sm_pFirst = nullptr;
CBaseGameSystemFactory *sm_Factory = nullptr;
CGameSystemEventDispatcher **g_ppEventDispatcher = nullptr;

namespace mm
{
	PlugifyPlugin g_Plugin;
	PLUGIN_EXPOSE(PlugifyPlugin, g_Plugin);

	#define CONPRINT(x) g_Plugin.m_logger->Log(LS_MESSAGE, x)
	#define CONPRINTE(x) g_Plugin.m_logger->Log(LS_WARNING, x)

	template <typename S, typename T, typename F> requires (std::is_function_v<F>)
	void Print(std::string& out, const T &t, F &f, std::string_view tab = "  ")
	{
		out += tab;
		if (t.GetState() != S::Loaded)
		{
			std::format_to(std::back_inserter(out), "[{:02d}] <{}> {}", t.GetId(), f(t.GetState()), t.GetFriendlyName());
		}
		else
		{
			std::format_to(std::back_inserter(out), "[{:02d}] {}", t.GetId(), t.GetFriendlyName());
		}
		auto descriptor = t.GetDescriptor();
		const auto &versionName = descriptor.GetVersionName();
		if (!versionName.empty())
		{
			std::format_to(std::back_inserter(out), " ({})", versionName);
		}
		else
		{
			std::format_to(std::back_inserter(out), " (v{})", descriptor.GetVersion());
		}
		const auto &createdBy = descriptor.GetCreatedBy();
		if (!createdBy.empty())
		{
			std::format_to(std::back_inserter(out), " by {}", createdBy);
		}
		out += '\n';
	}

	template <typename S, typename T, typename F> requires (std::is_function_v<F>)
	void Print(std::string& out, const char *name, const T &t, F &f)
	{
		if (t.GetState() == S::Error)
		{
			std::format_to(std::back_inserter(out), "{} has error: {}.\n", name, t.GetError());
		}
		else
		{
			std::format_to(std::back_inserter(out), "{} {} is {}.\n", name, t.GetId(), f(t.GetState()));
		}
		auto descriptor = t.GetDescriptor();
		const auto &getCreatedBy = descriptor.GetCreatedBy();
		if (!getCreatedBy.empty())
		{
			std::format_to(std::back_inserter(out), "  Name: \"{}\" by {}\n", t.GetFriendlyName(), getCreatedBy);
		}
		else
		{
			std::format_to(std::back_inserter(out), "  Name: \"{}\"\n", t.GetFriendlyName());
		}
		const auto &versionName = descriptor.GetVersionName();
		if (!versionName.empty())
		{
			std::format_to(std::back_inserter(out), "  Version: {}\n", versionName);
		}
		else
		{
			std::format_to(std::back_inserter(out), "  Version: {}\n", descriptor.GetVersion());
		}
		const auto &description = descriptor.GetDescription();
		if (!description.empty())
		{
			std::format_to(std::back_inserter(out), "  Description: {}\n", description);
		}
		const auto &createdByURL = descriptor.GetCreatedByURL();
		if (!createdByURL.empty())
		{
			std::format_to(std::back_inserter(out), "  URL: {}\n", createdByURL);
		}
		const auto &docsURL = descriptor.GetDocsURL();
		if (!docsURL.empty())
		{
			std::format_to(std::back_inserter(out), "  Docs: {}\n", docsURL);
		}
		const auto &downloadURL = descriptor.GetDownloadURL();
		if (!downloadURL.empty())
		{
			std::format_to(std::back_inserter(out), "  Download: {}\n", downloadURL);
		}
		const auto &updateURL = descriptor.GetUpdateURL();
		if (!updateURL.empty())
		{
			std::format_to(std::back_inserter(out), "  Update: {}\n", updateURL);
		}
	}

	std::string FormatTime(std::string_view format = "%Y-%m-%d %H:%M:%S")
	{
		auto now = std::chrono::system_clock::now();
		auto timeT = std::chrono::system_clock::to_time_t(now);
		std::stringstream ss;
		ss << std::put_time(std::localtime(&timeT), format.data());
		return ss.str();
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
			CONPRINT(std::format("Invalid argument: {}\n", e.what()).c_str());
		}
		catch (const std::out_of_range &e)
		{
			CONPRINT(std::format("Out of range: {}\n", e.what()).c_str());
		}
		catch (const std::exception &e)
		{
			CONPRINT(std::format("Conversion error: {}\n", e.what()).c_str());
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
				CONPRINT("Plugify Menu\n"
				         "(c) untrustedmodders\n"
				         "https://github.com/untrustedmodders\n"
				         "usage: plugify <command> [options] [arguments]\n"
				         "  help           - Show help\n"
				         "  version        - Version information\n"
				         "Plugin Manager commands:\n"
				         "  load           - Load plugin manager\n"
				         "  unload         - Unload plugin manager\n"
				         "  modules        - List running modules\n"
				         "  plugins        - List running plugins\n"
				         "  plugin <name>  - Show information about a module\n"
				         "  module <name>  - Show information about a plugin\n"
				         "Plugin Manager options:\n"
				         "  -h, --help     - Show help\n"
				         "  -u, --uuid     - Use index instead of name\n"
				         "Package Manager commands:\n"
				         "  install <name> - Packages to install (space separated)\n"
				         "  remove <name>  - Packages to remove (space separated)\n"
				         "  update <name>  - Packages to update (space separated)\n"
				         "  list           - Print all local packages\n"
				         "  query          - Print all remote packages\n"
				         "  show  <name>   - Show information about local package\n"
				         "  search <name>  - Search information about remote package\n"
				         "  snapshot       - Snapshot packages into manifest\n"
				         "  repo <url>     - Add repository to config\n"
				         "Package Manager options:\n"
				         "  -h, --help     - Show help\n"
				         "  -a, --all      - Install/remove/update all packages\n"
				         "  -f, --file     - Packages to install (from file manifest)\n"
				         "  -l, --link     - Packages to install (from HTTP manifest)\n"
				         "  -m, --missing  - Install missing packages\n"
				         "  -c, --conflict - Remove conflict packages\n"
				         "  -i, --ignore   - Ignore missing or conflict packages\n");
			}

			else if (arguments[1] == "version" || arguments[1] == "-v")
			{
				CONPRINT(R"(      ____)" "\n"
				         R"( ____|    \         Plugify v)" PLUGIFY_PROJECT_VERSION "\n"
				         R"((____|     `._____  )" "Copyright (C) 2023-" PLUGIFY_PROJECT_YEAR " Untrusted Modders Team\n"
				         R"( ____|       _|___)" "\n"
				         R"((____|     .'       This program may be freely redistributed under)" "\n"
				         R"(     |____/         the terms of the GNU General Public License.)" "\n");
			}

			else if (arguments[1] == "load")
			{
				if (!options.contains("--ignore") && !options.contains("-i"))
				{
					if (packageManager->HasMissedPackages())
					{
						CONPRINT("Plugin manager has missing packages, run 'install --missing' to resolve issues.\n");
						return;
					}
					if (packageManager->HasConflictedPackages())
					{
						CONPRINT("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.\n");
						return;
					}
				}
				if (pluginManager->IsInitialized())
				{
					CONPRINT("Plugin manager already loaded.\n");
				}
				else
				{
					g_Plugin.m_state = PlugifyState::Load;
				}
			}

			else if (arguments[1] == "unload")
			{
				if (!pluginManager->IsInitialized())
				{
					CONPRINT("Plugin manager already unloaded.\n");
				}
				else
				{
					g_Plugin.m_state = PlugifyState::Unload;
				}
			}

			else if (arguments[1] == "plugins")
			{
				if (!pluginManager->IsInitialized())
				{
					CONPRINT("You must load plugin manager before query any information from it.\n");
					return;
				}

				auto count = pluginManager->GetPlugins().size();
				std::string sMessage = count ? std::format("Listing {} plugin{}:\n", static_cast<int>(count), (count > 1) ? "s" : "") : std::string("No plugins loaded.\n");

				for (auto &plugin : pluginManager->GetPlugins())
				{
					Print<plugify::PluginState>(sMessage, plugin, plugify::PluginUtils::ToString);
				}

				CONPRINT(sMessage.c_str());
			}

			else if (arguments[1] == "modules")
			{
				if (!pluginManager->IsInitialized())
				{
					CONPRINT("You must load plugin manager before query any information from it.\n");
					return;
				}
				auto count = pluginManager->GetModules().size();
				std::string sMessage = count ? std::format("Listing {} module{}:\n", static_cast<int>(count), (count > 1) ? "s" : "") : std::string("No modules loaded.\n");
				for (auto &module : pluginManager->GetModules())
				{
					Print<plugify::ModuleState>(sMessage, module, plugify::ModuleUtils::ToString);
				}

				CONPRINT(sMessage.c_str());
			}

			else if (arguments[1] == "plugin")
			{
				if (arguments.size() > 2)
				{
					if (!pluginManager->IsInitialized())
					{
						CONPRINT("You must load plugin manager before query any information from it.\n");
						return;
					}
					auto plugin = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindPluginFromId(FormatInt(arguments[2])) : pluginManager->FindPlugin(arguments[2]);
					if (plugin.has_value())
					{
						std::string sMessage;
						Print<plugify::PluginState>(sMessage, "Plugin", *plugin, plugify::PluginUtils::ToString);
						auto descriptor = plugin->GetDescriptor();
						std::format_to(std::back_inserter(sMessage), "  Language module: {}\n", descriptor.GetLanguageModule());
						sMessage += "  Dependencies: \n";
						for (const auto &reference : descriptor.GetDependencies())
						{
							auto dependency = pluginManager->FindPlugin(reference.GetName());
							if (dependency.has_value())
							{
								Print<plugify::PluginState>(sMessage, *dependency, plugify::PluginUtils::ToString, "    ");
							}
							else
							{
								std::format_to(std::back_inserter(sMessage), "    {} <Missing> (v{})", reference.GetName(), reference.GetRequestedVersion().has_value() ? std::to_string(*reference.GetRequestedVersion()) : "[latest]");
							}
						}
						std::format_to(std::back_inserter(sMessage), "  File: {}\n\n", descriptor.GetEntryPoint());

						CONPRINT(sMessage.c_str());
					}
					else
					{
						CONPRINT(std::format("Plugin {} not found.\n", arguments[2]).c_str());
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
						std::string sMessage;

						Print<plugify::ModuleState>(sMessage, "Module", *module, plugify::ModuleUtils::ToString);
						std::format_to(std::back_inserter(sMessage), "  Language: {}\n", module->GetLanguage());
						std::format_to(std::back_inserter(sMessage), "  File: {}\n\n", std::filesystem::path(module->GetFilePath()).string());

						CONPRINT(sMessage.c_str());
					}
					else
					{
						CONPRINT(std::format("Module {} not found.\n", arguments[2]).c_str());
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
					CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
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
					CONPRINT("You must give at least one repository to add.\n");
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
					CONPRINT(std::format("Listing {} local package{}:\n", static_cast<int>(count), (count > 1) ? "s" : "").c_str());
				}
				for (auto &localPackage : packageManager->GetLocalPackages())
				{
					CONPRINT(std::format("  {} [{}] (v{}) at {}\n", localPackage.name, localPackage.type, localPackage.version, localPackage.path.string()).c_str());
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
				std::string sMessage = count ? std::format("Listing {} remote package{}:\n", static_cast<int>(count), (count > 1) ? "s" : "") : std::string("No remote packages found.\n");
				for (auto &remotePackage : packageManager->GetRemotePackages())
				{
					if (remotePackage.author.empty() || remotePackage.description.empty())
					{
						std::format_to(std::back_inserter(sMessage), "  {} [{}]\n", remotePackage.name, remotePackage.type);
					}
					else
					{
						std::format_to(std::back_inserter(sMessage), "  {} [{}] ({}) by {}\n", remotePackage.name, remotePackage.type, remotePackage.description, remotePackage.author);
					}
				}

				CONPRINT(sMessage.c_str());
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
						CONPRINT(std::format("  Name: {}\n"
						                     "  Type: {}\n"
						                     "  Version: {}\n"
						                     "  File: {}\n\n", package->name, package->type, package->version, package->path.string()).c_str());
					}
					else
					{
						CONPRINT(std::format("Package {} not found.\n", arguments[2]).c_str());
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
						std::string sMessage;

						std::format_to(std::back_inserter(sMessage), "  Name: {}\n", package->name);
						std::format_to(std::back_inserter(sMessage), "  Type: {}\n", package->type);
						if (!package->author.empty())
						{
							std::format_to(std::back_inserter(sMessage), "  Author: {}\n", package->author);
						}
						if (!package->description.empty())
						{
							std::format_to(std::back_inserter(sMessage), "  Description: {}\n", package->description);
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
							sMessage += versions;

							CONPRINT(sMessage.c_str());
						}
						else
						{
							CONPRINT("\n");
						}
					}
					else
					{
						CONPRINT(std::format("Package {} not found.\n", arguments[2]).c_str());
					}
				}
				else
				{
					CONPRINT("You must provide name.\n");
				}
			}

			else
			{
				std::string sMessage = std::format("unknown option: {}\n", arguments[1]);
				sMessage += "usage: plugify <command> [options] [arguments]\n"
				           "Try plugify help or -h for more information.\n";

				CONPRINT(sMessage.c_str());
			}
		}
		else
		{
			CONPRINT("usage: plugify <command> [options] [arguments]\n"
			         "Try plugify help or -h for more information.\n");
		}
	}
	static ConCommand plg_command("plg", plugify_callback, "Plugify control options", 0);

	bool PlugifyPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
	{
		PLUGIN_SAVEVARS();

		GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
		GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
		GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2Server, ISource2Server, SOURCE2SERVER_INTERFACE_VERSION);
		GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

		g_SMAPI->AddListener(this, &m_listener);

		ConVar_Register(FCVAR_RELEASE | FCVAR_SERVER_CAN_EXECUTE | FCVAR_GAMEDLL);

		m_context = plugify::MakePlugify();

		m_logger = std::make_shared<MMLogger>("plugify");
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

	bool PlugifyPlugin::Unload(char *error, size_t maxlen)
	{
		m_context.reset();
		if (sm_Factory)
		{
			sm_Factory->Shutdown();
			{
				const auto *pGameSystem = sm_Factory->GetStaticGameSystem();
				auto **ppDispatcher = g_ppEventDispatcher;
				Assert(ppDispatcher);
				auto *pDispatcher = *ppDispatcher;
				if (pDispatcher)
				{
					auto *funcListeners = pDispatcher->m_funcListeners;
					Assert(funcListeners);
					for (auto &vecListeners : *funcListeners)
					{
						FOR_EACH_VEC(vecListeners, i)
						{
							if (vecListeners[i] == pGameSystem)
							{
								vecListeners.FastRemove(i);
							}
						}
					}
				}
			}
			sm_Factory->DestroyGameSystem(this);
			sm_Factory->Destroy();
			sm_Factory = nullptr;
		}
		return true;
	}

	GS_EVENT_MEMBER(PlugifyPlugin, ServerGamePostSimulate)
	{
		switch (m_state)
		{
			case PlugifyState::Load:
			{
				auto pluginManager = m_context->GetPluginManager().lock();
				if (!pluginManager)
				{
					m_state = PlugifyState::Wait;
					return;
				}

				pluginManager->Initialize();
				CONPRINT("Plugin manager was loaded.\n");
				break;
			}
			case PlugifyState::Unload:
			{
				auto pluginManager = m_context->GetPluginManager().lock();
				if (!pluginManager)
				{
					m_state = PlugifyState::Wait;
					return;
				}

				pluginManager->Terminate();
				CONPRINT("Plugin manager was unloaded.\n");
				break;
			}
			case PlugifyState::Wait:
				return;
		}

		m_state = PlugifyState::Wait;
	}

	void PlugifyPlugin::AllPluginsLoaded()
	{
	}

	bool PlugifyPlugin::Pause(char *error, size_t maxlen)
	{
		return true;
	}

	bool PlugifyPlugin::Unpause(char *error, size_t maxlen)
	{
		return true;
	}

	const char *PlugifyPlugin::GetLicense()
	{
		return "MIT";
	}

	const char *PlugifyPlugin::GetVersion()
	{
		return PLUGIFY_PROJECT_VERSION;
	}

	const char *PlugifyPlugin::GetDate()
	{
		return __DATE__;
	}

	const char *PlugifyPlugin::GetLogTag()
	{
		return "PLUGIFY";
	}

	const char *PlugifyPlugin::GetAuthor()
	{
		return "untrustedmodders";
	}

	const char *PlugifyPlugin::GetDescription()
	{
		return PLUGIFY_PROJECT_DESCRIPTION;
	}

	const char *PlugifyPlugin::GetName()
	{
		return PLUGIFY_PROJECT_NAME;
	}

	const char *PlugifyPlugin::GetURL()
	{
		return PLUGIFY_PROJECT_HOMEPAGE_URL;
	}
} // namespace mm

SMM_API IMetamodListener *Plugify_ImmListener()
{
	return &mm::g_Plugin.m_listener;
}

SMM_API ISmmAPI *Plugify_ISmmAPI()
{
	return mm::g_SMAPI;
}

SMM_API ISmmPlugin *Plugify_ISmmPlugin()
{
	return mm::g_PLAPI;
}

SMM_API PluginId Plugify_Id()
{
	return mm::g_PLID;
}

SMM_API SourceHook::ISourceHook *Plugify_SourceHook()
{
	return mm::g_SHPtr;
}

SMM_API void Plugify_RegisterGameSystem(CBaseGameSystemFactory **ppFirstGameSystem, CGameSystemEventDispatcher** ppEventDispatcher)
{
	CBaseGameSystemFactory::sm_pFirst = ppFirstGameSystem;
	g_ppEventDispatcher = ppEventDispatcher;
	if (sm_Factory == nullptr)
	{
		sm_Factory = new CGameSystemStaticFactory<mm::PlugifyPlugin>("PlugifyGameSystem", &mm::g_Plugin);
	}
}
