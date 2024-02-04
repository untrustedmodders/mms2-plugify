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
#include "mm_logger.h"
#include <igameevents.h>
#include <iserver.h>
#include <plugify/plugify.h>
#include <plugify/plugin.h>
#include <plugify/module.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/package.h>
#include <plugify/plugin_manager.h>
#include <plugify/package_manager.h>

#include <filesystem>
#include <chrono>

std::string FormatTime(std::string_view format = "%Y-%m-%d %H:%M:%S") {
	auto now = std::chrono::system_clock::now();
	auto timeT = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&timeT), format.data());
	return ss.str();
}

namespace plugifyMM {
	IServerGameDLL* server = NULL;
	IServerGameClients* gameclients = NULL;
	IVEngineServer* engine = NULL;
	IGameEventManager2* gameevents = NULL;
	ICvar* icvar = NULL;

	PlugifyMMPlugin g_Plugin;
	PLUGIN_EXPOSE(PlugifyMMPlugin, g_Plugin);

	template<typename S, typename T, typename F>
	void Print(const T& t, F& f, std::string_view tab = "  ") {
		std::string result(tab);
		if (t.GetState() != S::Loaded) {
			std::format_to(std::back_inserter(result), "[{:02d}] <{}> {}", static_cast<int>(t.GetId()), f(t.GetState()), t.GetFriendlyName());
		} else {
			std::format_to(std::back_inserter(result), "[{:02d}] {}", static_cast<int>(t.GetId()), t.GetFriendlyName());
		}
		if (!t.GetDescriptor().versionName.empty()){
			std::format_to(std::back_inserter(result), " ({})", t.GetDescriptor().versionName);
		} else {
			std::format_to(std::back_inserter(result), " (v{})", t.GetDescriptor().version);
		}
		if (!t.GetDescriptor().createdBy.empty()) {
			std::format_to(std::back_inserter(result), " by {}", t.GetDescriptor().createdBy);
		}
		std::format_to(std::back_inserter(result), "\n");
		META_CONPRINT(result.c_str());
	}

	template<typename S, typename T, typename F>
	void Print(const char* name, const T& t, F& f) {
		if (t.GetState() == S::Error) {
			META_CONPRINTF("%s has error: %s.\n", name, t.GetError().c_str());
		} else {
			META_CONPRINTF("%s %d is %s.\n", name, static_cast<int>(t.GetId()), f(t.GetState()).data());
		}
		if (!t.GetDescriptor().createdBy.empty()) {
			META_CONPRINTF("  Name: \"%s\" by %s\n", t.GetFriendlyName().c_str(), t.GetDescriptor().createdBy.c_str());
		} else {
			META_CONPRINTF("  Name: \"%s\"\n", t.GetFriendlyName().c_str());
		}
		if (!t.GetDescriptor().versionName.empty()) {
			META_CONPRINTF("  Version: %s\n", t.GetDescriptor().versionName.c_str());
		} else {
			META_CONPRINTF("  Version: %d\n", t.GetDescriptor().version);
		}
		if (!t.GetDescriptor().description.empty()) {
			META_CONPRINTF("  Description: %s\n", t.GetDescriptor().description.c_str());
		}
		if (!t.GetDescriptor().createdByURL.empty()) {
			META_CONPRINTF("  URL: %s\n", t.GetDescriptor().createdByURL.c_str());
		}
		if (!t.GetDescriptor().docsURL.empty()) {
			META_CONPRINTF("  Docs: %s\n", t.GetDescriptor().docsURL.c_str());
		}
		if (!t.GetDescriptor().downloadURL.empty()) {
			META_CONPRINTF("  Download: %s\n", t.GetDescriptor().downloadURL.c_str());
		}
		if (!t.GetDescriptor().updateURL.empty()) {
			META_CONPRINTF("  Update: %s\n", t.GetDescriptor().updateURL.c_str());
		}
	}

	uintmax_t FormatInt(const std::string& str) {
		try {
			size_t pos;
			uintmax_t result = std::stoull(str, &pos);
			if (pos != str.length()) {
				throw std::invalid_argument("Trailing characters after the valid part");
			}
			return result;
		} catch (const std::invalid_argument& e) {
			META_CONPRINTF("Invalid argument: %s", e.what());
		} catch (const std::out_of_range& e) {
			META_CONPRINTF("Out of range: %s", e.what());
		} catch (const std::exception& e) {
			META_CONPRINTF("Conversion error: %s", e.what());
		}

		return uintmax_t(-1);
	}

	CON_COMMAND_F(plugify, "Plugify control options", FCVAR_NONE) {
		std::vector<std::string> arguments;
		std::unordered_set<std::string> options;
		std::span view(args.ArgV(), args.ArgC());
		arguments.reserve(view.size());
		for (size_t i = 0; i < view.size(); i++) {
			std::string str(view[i]);
			if (i > 1 && str.starts_with("-")) {
				options.emplace(std::move(str));
			} else {
				arguments.emplace_back(std::move(str));
			}
		}
		
		auto& plugify = g_Plugin.m_context;
		if (!plugify)
			return; // Should not trigger!
		
		auto packageManager = plugify->GetPackageManager().lock();
		auto pluginManager = plugify->GetPluginManager().lock();
		if (!packageManager || !pluginManager)
			return; // Should not trigger!

		if (arguments.size() > 1) {
			
			if (arguments[1] == "help" || arguments[1] == "-h") {
				META_CONPRINT("Plugify Menu\n");
				META_CONPRINT("(c) untrustedmodders\n");
				META_CONPRINT("https://github.com/untrustedmodders\n");
				META_CONPRINT("usage: plugify <command> [options] [arguments]\n");
				META_CONPRINT("  help           - Show help\n");
				META_CONPRINT("  version        - Version information\n");
				META_CONPRINT("Plugin Manager commands:\n");
				META_CONPRINT("  load           - Load plugin manager\n");
				META_CONPRINT("  unload         - Unload plugin manager\n");
				META_CONPRINT("  modules        - List running modules\n");
				META_CONPRINT("  plugins        - List running plugins\n");
				META_CONPRINT("  plugin <name>  - Show information about a module\n");
				META_CONPRINT("  module <name>  - Show information about a plugin\n");
				META_CONPRINT("Plugin Manager options:\n");
				META_CONPRINT("  -h, --help     - Show help\n");
				META_CONPRINT("  -u, --uuid     - Use index instead of name\n");
				META_CONPRINT("Package Manager commands:\n");
				META_CONPRINT("  install <name> - Packages to install (space separated)\n");
				META_CONPRINT("  remove <name>  - Packages to remove (space separated)\n");
				META_CONPRINT("  update <name>  - Packages to update (space separated)\n");
				META_CONPRINT("  list           - Print all local packages\n");
				META_CONPRINT("  query          - Print all remote packages\n");
				META_CONPRINT("  show  <name>   - Show information about local package\n");
				META_CONPRINT("  search <name>  - Search information about remote package\n");
				META_CONPRINT("  snapshot       - Snapshot packages into manifest\n");
				META_CONPRINT("Package Manager options:\n");
				META_CONPRINT("  -h, --help     - Show help\n");
				META_CONPRINT("  -a, --all      - Install/remove/update all packages\n");
				META_CONPRINT("  -f, --file     - Packages to install (from file manifest)\n");
				META_CONPRINT("  -l, --link     - Packages to install (from HTTP manifest)\n");
				META_CONPRINT("  -m, --missing  - Install missing packages\n");
				META_CONPRINT("  -c, --conflict - Remove conflict packages\n");
				META_CONPRINT("  -i, --ignore   - Ignore missing or conflict packages\n");
			} 
			
			else if (arguments[1] == "version" || arguments[1] == "-v") {
				static std::string copyright = std::format("Copyright (C) 2023-{} Untrusted Modders Team\n", __DATE__ + 7);
				META_CONPRINT(R"(      ____)" "\n");
				META_CONPRINT(R"( ____|    \         Plugify v)" PLUGIFY_PROJECT_VERSION "\n");
				META_CONPRINT(R"((____|     `._____  )");
				META_CONPRINTF("%s\n", copyright.c_str());
				META_CONPRINT(R"( ____|       _|___)" "\n");
				META_CONPRINT(R"((____|     .'       This program may be freely redistributed under)" "\n");
				META_CONPRINT(R"(     |____/         the terms of the GNU General Public License.)" "\n");
			} 
			
			else if (arguments[1] == "load") {
				if (!options.contains("--ignore") && !options.contains("-i")) {
					if (packageManager->HasMissedPackages()) {
						META_CONPRINT("Plugin manager has missing packages, run 'install --missing' to resolve issues.");
						return;
					}
					if (packageManager->HasConflictedPackages()) {
						META_CONPRINT("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
						return;
					}
				}
				if (pluginManager->IsInitialized()) {
					META_CONPRINT("Plugin manager already loaded.");
				} else {
					pluginManager->Initialize();
					META_CONPRINT("Plugin manager was loaded.");
				}
			} 
			
			else if (arguments[1] == "unload") {
				if (!pluginManager->IsInitialized()) { 
					META_CONPRINT("Plugin manager already unloaded.");
				} else {
					pluginManager->Terminate();
					META_CONPRINT("Plugin manager was unloaded.");
				}
			} 
			
			else if (arguments[1] == "plugins") {
				if (!pluginManager->IsInitialized()) { 
					META_CONPRINT("You must load plugin manager before query any information from it.");
					return;
				}
				auto count = pluginManager->GetPlugins().size();
				if (!count) {
					META_CONPRINT("No plugins loaded.\n");
				} else {
					META_CONPRINTF("Listing %d plugin%s:\n", static_cast<int>(count), (count > 1) ? "s" : "");
				}
				for (auto& pluginRef : pluginManager->GetPlugins()) {
					Print<plugify::PluginState>(pluginRef.get(), plugify::PluginStateToString);
				}
			} 
			
			else if (arguments[1] == "modules") {
				if (!pluginManager->IsInitialized()) { 
					META_CONPRINT("You must load plugin manager before query any information from it.");
					return;
				}
				auto count = pluginManager->GetModules().size();
				if (!count) {
					META_CONPRINT("No modules loaded.\n");
				} else {
					META_CONPRINTF("Listing %d module%s:\n", static_cast<int>(count), (count > 1) ? "s" : "");
				}
				for (auto& moduleRef : pluginManager->GetModules()) {
					Print<plugify::ModuleState>(moduleRef.get(), plugify::ModuleStateToString);
				}
			}
			
			else if (arguments[1] == "plugin") {
				if (arguments.size() > 2) {
					if (!pluginManager->IsInitialized()) { 
						META_CONPRINT("You must load plugin manager before query any information from it.");
						return;
					}
					auto pluginRef = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindPluginFromId(FormatInt(arguments[2])) : pluginManager->FindPlugin(arguments[2]);
					if (pluginRef.has_value()) {
						auto& plugin = pluginRef->get();
						Print<plugify::PluginState>("Plugin", plugin, plugify::PluginStateToString);
						META_CONPRINTF("  Language module: %s\n", plugin.GetDescriptor().languageModule.name.c_str());
						META_CONPRINT("  Dependencies: \n");
						for (const auto& reference : plugin.GetDescriptor().dependencies) {
							auto dependencyRef = pluginManager->FindPlugin(reference.name);
							if (dependencyRef.has_value()) {
								Print<plugify::PluginState>(dependencyRef->get(), plugify::PluginStateToString, "    ");
							} else {
								META_CONPRINTF("    %s <Missing> (v%s)", reference.name.c_str(), reference.requestedVersion.has_value() ? std::to_string(*reference.requestedVersion).c_str() : "[latest]");
							}
						}
						META_CONPRINTF("  File: %s\n\n", plugin.GetDescriptor().entryPoint.c_str());
					} else {
						META_CONPRINTF("Plugin %s not found.\n", arguments[2].c_str());
					}
				} else {
					META_CONPRINT("You must provide name.\n");
				}
			} 
			
			else if (arguments[1] == "module") {
				if (arguments.size() > 2) {
					if (!pluginManager->IsInitialized()) { 
						META_CONPRINT("You must load plugin manager before query any information from it.");
						return;
					}
					auto moduleRef = options.contains("--uuid") || options.contains("-u") ? pluginManager->FindModuleFromId(FormatInt(arguments[2])) : pluginManager->FindModule(arguments[2]);
					if (moduleRef.has_value()) {
						auto& module = moduleRef->get();
						Print<plugify::ModuleState>("Module", module, plugify::ModuleStateToString);
						META_CONPRINTF("  Language: %s\n", module.GetDescriptor().language.c_str());
						META_CONPRINTF("  File: %s\n\n", module.GetFilePath().string().c_str());
					} else {
						META_CONPRINTF("Module %s not found.\n", arguments[2].c_str());
					}
				} else {
					META_CONPRINT("You must provide name.\n");
				}
			} 
			
			else if (arguments[1] == "snapshot") {
				if (pluginManager->IsInitialized()) { 
					META_CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				packageManager->SnapshotPackages(plugify->GetConfig().baseDir / std::format("snapshot_{}.wpackagemanifest", FormatTime("%Y_%m_%d_%H_%M_%S")), true);
			} 
			
			else if (arguments[1] == "install") {
				if (pluginManager->IsInitialized()) {
					META_CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				if (options.contains("--missing") || options.contains("-m")) {
					if (packageManager->HasMissedPackages()) {
						packageManager->InstallMissedPackages();
					} else {
						META_CONPRINT("No missing packages were found.\n");
					}
				} else {
					if (arguments.size() > 2) {
						if (options.contains("--link") || options.contains("-l")) {
							packageManager->InstallAllPackages(arguments[2], arguments.size() > 3);
						} else if (options.contains("--file") || options.contains("-f")) {
							packageManager->InstallAllPackages(std::filesystem::path{arguments[2]}, arguments.size() > 3);
						} else {
							packageManager->InstallPackages(std::span(arguments.begin() + 2, arguments.size() - 2));
						}
					} else {
						META_CONPRINT("You must give at least one requirement to install.\n");
					}
				}
			} 
			
			else if (arguments[1] == "remove") {
				if (pluginManager->IsInitialized()) {
					META_CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				if (options.contains("--all") || options.contains("-a")) {
					packageManager->UninstallAllPackages();
				} else if (options.contains("--conflict") || options.contains("-c")) {
					if (packageManager->HasConflictedPackages()) {
						packageManager->UninstallConflictedPackages();
					} else {
						META_CONPRINT("No conflicted packages were found.\n");
					}
				} else {
					if (arguments.size() > 2) {
						packageManager->UninstallPackages(std::span(arguments.begin() + 2, arguments.size() - 2));
					} else {
						META_CONPRINT("You must give at least one requirement to remove.\n");
					}
				}
			}
			
			else if (arguments[1] == "update") {
				if (pluginManager->IsInitialized()) {
					META_CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				if (options.contains("--all") || options.contains("-a")) {
					packageManager->UpdateAllPackages();
				} else {
					if (arguments.size() > 2) {
						packageManager->UpdatePackages(std::span(arguments.begin() + 2, arguments.size() - 2));
					} else {
						META_CONPRINT("You must give at least one requirement to update.\n");
					}
				}
			} 
			
			else if (arguments[1] == "list") {
				if (pluginManager->IsInitialized()) {
					META_CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				auto count = packageManager->GetLocalPackages().size();
				if (!count) {
					META_CONPRINT("No local packages found.\n");
				} else {
					META_CONPRINTF("Listing %d local package%s:\n", static_cast<int>(count), (count > 1) ? "s" : "");
				}
				for (auto& localPackageRef : packageManager->GetLocalPackages()) {
					auto& localPackage = localPackageRef.get();
					META_CONPRINTF("  %s [%s] (v%d) at %s\n", localPackage.name.c_str(), localPackage.type.c_str(), localPackage.version, localPackage.path.string().c_str());
				}
			} 
			
			else if (arguments[1] == "query") {
				if (pluginManager->IsInitialized()) {
					META_CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				auto count = packageManager->GetRemotePackages().size();
				if (!count) {
					META_CONPRINT("No remote packages found.\n");
				} else {
					META_CONPRINTF("Listing %d remote package%s:\n", static_cast<int>(count), (count > 1) ? "s" : "");
				}
				for (auto& remotePackageRef : packageManager->GetRemotePackages()) {
					auto& remotePackage = remotePackageRef.get();
					if (remotePackage.author.empty() || remotePackage.description.empty()) {
						META_CONPRINTF("  %s [%s]\n", remotePackage.name.c_str(), remotePackage.type.c_str());
					} else {
						META_CONPRINTF("  %s [%s] (%s) by %s\n", remotePackage.name.c_str(), remotePackage.type.c_str(), remotePackage.description.c_str(), remotePackage.author.c_str());
					}
				}
			} 
			
			else if (arguments[1] == "show") {
				if (pluginManager->IsInitialized()) {
					META_CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				if (arguments.size() > 2) {
					auto packageRef = packageManager->FindLocalPackage(arguments[2]);
					if (packageRef.has_value()) {
						auto& package = packageRef->get();
						META_CONPRINTF("  Name: %s\n", package.name.c_str());
						META_CONPRINTF("  Type: %s\n", package.type.c_str());
						META_CONPRINTF("  Version: %s\n", package.version);
						META_CONPRINTF("  File: %s\n\n", package.path.string().c_str());

					} else {
						META_CONPRINTF("Package %s not found.\n", arguments[2].c_str());
					}
				} else {
					META_CONPRINT("You must provide name.\n");
				}
			} 
			
			else if (arguments[1] == "search") {
				if (pluginManager->IsInitialized()) {
					META_CONPRINT("You must unload plugin manager before bring any change with package manager.\n");
					return;
				}
				if (arguments.size() > 2) {
					auto packageRef = packageManager->FindRemotePackage(arguments[2]);
					if (packageRef.has_value()) {
						auto& package = packageRef->get();
						META_CONPRINTF("  Name: %s\n", package.name.c_str());
						META_CONPRINTF("  Type: %s\n", package.type.c_str());
						if (!package.author.empty()) {
							META_CONPRINTF("  Author: %s\n", package.author.c_str());
						}
						if (!package.description.empty()) {
							META_CONPRINTF("  Description: %s\n", package.description.c_str());
						}
						if (!package.versions.empty()) {
							std::string versions("  Versions: ");
							std::format_to(std::back_inserter(versions), "{}", package.versions.begin()->version);
							for (auto it = std::next(package.versions.begin()); it != package.versions.end(); ++it) {
								std::format_to(std::back_inserter(versions), ", {}", it->version);
							}
							std::format_to(std::back_inserter(versions), "\n\n");
							META_CONPRINT(versions.c_str());
						} else {
							META_CONPRINT("\n");
						}
					} else {
						META_CONPRINTF("Package %s not found.\n", arguments[2].c_str());
					}
				} else {
					META_CONPRINT("You must provide name.\n");
				}
			}
			
			else {
				META_CONPRINTF("unknown option: %s\n", arguments[1].c_str());
				META_CONPRINT("usage: plugify <command> [options] [arguments]\n");
				META_CONPRINT("Try plugify help or -h for more information.\n");
			}
		} else {
			META_CONPRINT("usage: plugify <command> [options] [arguments]\n");
			META_CONPRINT("Try plugify help or -h for more information.\n");
		}
	}

	bool PlugifyMMPlugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late) {
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

		auto logger = std::make_shared<MMLogger>();
		logger->SetSeverity(plugify::Severity::Info);
		m_context->SetLogger(std::move(logger));

		auto result = m_context->Initialize(Plat_GetGameDirectory());
		if (result) {
			if (auto packageManager = m_context->GetPackageManager().lock()) {
				packageManager->Initialize();

				if (packageManager->HasMissedPackages()) {
					META_CONPRINT("Plugin manager has missing packages, run 'update --missing' to resolve issues.");
					return true;
				}
				if (packageManager->HasConflictedPackages()) {
					META_CONPRINT("Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
					return true;
				}
			}

			if (auto pluginManager = m_context->GetPluginManager().lock()) {
				pluginManager->Initialize();
			}
		}

		return result;
	}

	bool PlugifyMMPlugin::Unload(char* error, size_t maxlen) {
		m_context.reset();
		return true;
	}

	void PlugifyMMPlugin::AllPluginsLoaded() {
	}

	bool PlugifyMMPlugin::Pause(char* error, size_t maxlen) {
		return true;
	}

	bool PlugifyMMPlugin::Unpause(char* error, size_t maxlen) {
		return true;
	}

	const char* PlugifyMMPlugin::GetLicense() {
		return "Public Domain";
	}

	const char* PlugifyMMPlugin::GetVersion() {
		return PLUGIFY_PROJECT_VERSION;
	}

	const char* PlugifyMMPlugin::GetDate() {
		return __DATE__;
	}

	const char* PlugifyMMPlugin::GetLogTag() {
		return "PLUGIFY";
	}

	const char* PlugifyMMPlugin::GetAuthor() {
		return "untrustedmodders";
	}

	const char* PlugifyMMPlugin::GetDescription() {
		return PLUGIFY_PROJECT_DESCRIPTION;
	}

	const char* PlugifyMMPlugin::GetName() {
		return PLUGIFY_PROJECT_NAME;
	}

	const char* PlugifyMMPlugin::GetURL() {
		return PLUGIFY_PROJECT_HOMEPAGE_URL;
	}
}

SMM_API IMetamodListener* Plugify_ImmListener() {
	return &plugifyMM::g_Plugin.m_listener;
}

SMM_API ISmmAPI* Plugify_ISmmAPI() {
	return plugifyMM::g_SMAPI;
}

SMM_API ISmmPlugin* Plugify_ISmmPlugin() {
	return plugifyMM::g_PLAPI;
}

SMM_API PluginId Plugify_Id() {
	return plugifyMM::g_PLID;
}

SMM_API SourceHook::ISourceHook* Plugify_SourceHook() {
	return plugifyMM::g_SHPtr;
}
