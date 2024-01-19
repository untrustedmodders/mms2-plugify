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
#include <wizard/wizard.h>
#include <wizard/plugin.h>
#include <wizard/module.h>
#include <wizard/plugin_descriptor.h>
#include <wizard/package.h>
#include <wizard/plugin_manager.h>
#include <wizard/package_manager.h>

#include <filesystem>
#include <chrono>

std::string FormatTime(std::string_view format = "%Y-%m-%d %H:%M:%S") {
	auto now = std::chrono::system_clock::now();
	auto timeT = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&timeT), format.data());
	return ss.str();
}

namespace wizardMM {
	IServerGameDLL* server = NULL;
	IServerGameClients* gameclients = NULL;
	IVEngineServer* engine = NULL;
	IGameEventManager2* gameevents = NULL;
	ICvar* icvar = NULL;

	WizardMMPlugin g_Plugin;
	PLUGIN_EXPOSE(WizardMMPlugin, g_Plugin);

	template<typename S, typename T, typename F>
	void Print(const T& t, F& f, std::string_view tab = "  ") {
		std::stringstream ss;
		ss << tab;
		if (t.GetState() != S::Loaded) {
			ss << std::format("[{:02d}] <{}> {}", static_cast<int>(t.GetId()), f(t.GetState()), t.GetFriendlyName());
		} else {
			ss << std::format("[{:02d}] {}", static_cast<int>(t.GetId()), t.GetFriendlyName());
		}
		if (!t.GetDescriptor().versionName.empty()){
			ss << std::format(" ({})", t.GetDescriptor().versionName);
		} else {
			ss << std::format(" (v{})", t.GetDescriptor().version);
		}
		if (!t.GetDescriptor().createdBy.empty()) {
			ss << std::format(" by {}", t.GetDescriptor().createdBy);
		}
		ss << "\n";
		std::string buffer(ss.str());
		META_CONPRINT(buffer.c_str());
	}

	template<typename S, typename T, typename F>
	void Print(std::string_view name, const T& t, F& f) {
		if (t.GetState() == S::Error) {
			META_CONPRINTF("%s has error: %s.\n", name, t.GetError().c_str());
		} else {
			META_CONPRINTF("%s %d is %s.\n", name, static_cast<int>(t.GetId()), f(t.GetState()));
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

	CON_COMMAND_F(wizard, "Wizard control options", FCVAR_NONE) {
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
		
		auto& wizard = g_Plugin._context;
		if (!wizard)
			return; // Should not trigger!
		
		auto packageManager = wizard->GetPackageManager().lock();
		auto pluginManager = wizard->GetPluginManager().lock();
		if (!packageManager || !pluginManager)
			return; // Should not trigger!

		if (arguments.size() > 1) {
			
			if (arguments[1] == "help" || arguments[1] == "-h") {
				META_CONPRINT("Wizard Menu\n");
				META_CONPRINT("(c) untrustedmodders\n");
				META_CONPRINT("https://github.com/untrustedmodders\n");
				META_CONPRINT("usage: wizard <command> [options] [arguments]\n");
				META_CONPRINT("  help           - Show help\n");
				META_CONPRINT("  version        - Version information\n");
				META_CONPRINT("Plugin Manager commands:\n");
				META_CONPRINT("  load           - Load plugin manager\n");
				META_CONPRINT("  unload         - Unload plugin manager\n");
				META_CONPRINT("  modules        - List running modules\n");
				META_CONPRINT("  plugins        - List running plugins\n");
				META_CONPRINT("  plugin <name>  - Show information about a module\n");
				META_CONPRINT("  module <name>  - Show information about a plugin\n");
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
				META_CONPRINT(R"(            .)" "\n");
				META_CONPRINT(R"(           /:\            Wizard v)" WIZARD_PROJECT_VERSION "\n");
				META_CONPRINT(R"(          /;:.\           )");
				META_CONPRINTF("Copyright (C) 2023-%s Untrusted Modders Team\n", __DATE__ + 7);
				META_CONPRINT(R"(         //;:. \)" "\n");
				META_CONPRINT(R"(        ///;:.. \         This program may be freely redistributed under)" "\n");
				META_CONPRINT(R"(  __--"////;:... \"--__   the terms of the GNU General Public License.)" "\n");
				META_CONPRINT(R"(--__   "--_____--"   __--)" "\n");
				META_CONPRINT(R"(    """--_______--"""")" "\n");
			} 
			
			else if (arguments[1] == "load") {
				if (!options.contains("-i") {
					if (packageManager->HasMissedPackages()) {
						META_CONPRINT("Plugin manager has .");
						return;
					}
					if (packageManager->HasConflictedPackages()) {
						META_CONPRINT("Plugin manager has .");
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
					Print<wizard::PluginState>(pluginRef.get(), wizard::PluginStateToString);
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
					Print<wizard::ModuleState>(moduleRef.get(), wizard::ModuleStateToString);
				}
			} 
			
			else if (arguments[1] == "plugin") {
				if (arguments.size() > 2) {
					if (!pluginManager->IsInitialized()) { 
						META_CONPRINT("You must load plugin manager before query any information from it.");
						return;
					}
					auto pluginRef = pluginManager->FindPlugin(arguments[2]);
					if (pluginRef.has_value()) {
						auto& plugin = pluginRef->get();
						Print<wizard::PluginState>("Plugin", plugin, wizard::PluginStateToString);
						META_CONPRINTF("  Language module: %s\n", plugin.GetDescriptor().languageModule.name.c_str());
						META_CONPRINT("  Dependencies: \n");
						for (const auto& reference : plugin.GetDescriptor().dependencies) {
							auto dependencyRef = pluginManager->FindPlugin(reference.name);
							if (dependencyRef.has_value()) {
								Print<wizard::PluginState>(dependencyRef->get(), wizard::PluginStateToString, "    ");
							} else {
								META_CONPRINTF("    %s <Missing> (v%s)", reference.name.c_str(), reference.requestedVersion.has_value() ? std::to_string(*reference.requestedVersion).c_str() : "[latest]");
							}
						}
						META_CONPRINTF("  File: %s\n\n", plugin.GetFilePath().c_str());
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
					auto moduleRef = pluginManager->FindModule(arguments[2]);
					if (moduleRef.has_value()) {
						auto& module = moduleRef->get();
						Print<wizard::ModuleState>("Module", module, wizard::ModuleStateToString);
						META_CONPRINTF("  Language: %s\n", module.GetDescriptor().language.c_str());
						META_CONPRINTF("  File: %s\n\n", module.GetFilePath().c_str());
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
				packageManager->SnapshotPackages(wizard->GetConfig().baseDir / std::format("snapshot_{}.wpackagemanifest", FormatTime("%Y_%m_%d_%H_%M_%S")), true);
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
						packageManager->InstallConflictedPackages();
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
							std::stringstream ss;
							ss << "  Versions: \n";
							ss << package.versions.begin()->version;
							for (auto it = std::next(package.versions.begin()); it != package.versions.end(); ++it) {
								ss << ", " << it->version;
							}
							ss << "\n\n";
							std::string buffer(ss.str());
							META_CONPRINT(buffer.c_str());
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
				META_CONPRINT("usage: wizard <command> [options] [arguments]\n");
				META_CONPRINT("Try wizard help or -h for more information.\n");
			}
		} else {
			META_CONPRINT("usage: wizard <command> [options] [arguments]\n");
			META_CONPRINT("Try wizard help or -h for more information.\n");
		}
	}

	bool WizardMMPlugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late) {
		PLUGIN_SAVEVARS();

		GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
		GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
		GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
		GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
		GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

		//g_SMAPI->AddListener(this, this);

		g_pCVar = icvar;
		ConVar_Register(FCVAR_RELEASE | FCVAR_SERVER_CAN_EXECUTE | FCVAR_GAMEDLL);

		_context = wizard::MakeWizard();

		auto logger = std::make_shared<MMLogger>();
		logger->SetSeverity(wizard::Severity::Info);
		_context->SetLogger(std::move(logger));

		auto result = _context->Initialize(Plat_GetGameDirectory());
		if (result) {
			if (auto packageManager = _context->GetPackageManager().lock()) {
				packageManager->Initialize();
			}
		}
		return result;
	}

	bool WizardMMPlugin::Unload(char* error, size_t maxlen) {
		_context.reset();
		return true;
	}

	void WizardMMPlugin::AllPluginsLoaded() {
	}

	bool WizardMMPlugin::Pause(char* error, size_t maxlen) {
		return true;
	}

	bool WizardMMPlugin::Unpause(char* error, size_t maxlen) {
		return true;
	}

	const char* WizardMMPlugin::GetLicense() {
		return "Public Domain";
	}

	const char* WizardMMPlugin::GetVersion() {
		return WIZARD_PROJECT_VERSION;
	}

	const char* WizardMMPlugin::GetDate() {
		return __DATE__;
	}

	const char* WizardMMPlugin::GetLogTag() {
		return "WIZARD";
	}

	const char* WizardMMPlugin::GetAuthor() {
		return "untrustedmodders";
	}

	const char* WizardMMPlugin::GetDescription() {
		return WIZARD_PROJECT_DESCRIPTION;
	}

	const char* WizardMMPlugin::GetName() {
		return WIZARD_PROJECT_NAME;
	}

	const char* WizardMMPlugin::GetURL() {
		return WIZARD_PROJECT_HOMEPAGE_URL;
	}

}