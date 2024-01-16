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

namespace cs2wizard {
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
			META_CONPRINTF("  Version: %u\n", t.GetDescriptor().version);
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
		std::span view(args.ArgV(), args.ArgC());
		arguments.reserve(view.size());
		for (auto arg : view) {
			arguments.emplace_back(arg);
		}

		if (arguments.size() > 1) {
			auto& wizard = g_Plugin._context;
			if (arguments[1] == "help") {
				META_CONPRINTF("Wizard Menu\n");
				META_CONPRINTF("(c) untrustedmodders\n");
				META_CONPRINTF("https://github.com/untrustedmodders\n");
				META_CONPRINTF("usage: wizard <command> [arguments]\n");
				META_CONPRINTF("  version       - Version information\n");
				META_CONPRINTF("Plugin Manager commands:\n");
				META_CONPRINTF("  load          - Load plugin manager\n");
				META_CONPRINTF("  unload        - Unload plugin manager\n");
				META_CONPRINTF("  modules       - List running modules\n");
				META_CONPRINTF("  plugins       - List running plugins\n");
				META_CONPRINTF("  plugin <name> - Show information about a module\n");
				META_CONPRINTF("  module <name> - Show information about a plugin\n");
				META_CONPRINTF("Package Manager commands:\n");
				META_CONPRINTF("  -S   <name>   - Packages to install (space separated)\n");
				META_CONPRINTF("  -Sf  <file>   - Packages to install (from file manifest)\n");
				META_CONPRINTF("  -Sw  <link>   - Packages to install (from HTTP manifest)\n");
				META_CONPRINTF("  -R   <name>   - Packages to uninstall (space separated)\n");
				META_CONPRINTF("  -Ra           - Uninstall all packages\n");
				META_CONPRINTF("  -U   <name>   - Packages to update (space separated)\n");
				META_CONPRINTF("  -Ua           - Update all packages\n");
				META_CONPRINTF("  -Q            - Print all local packages\n");
				META_CONPRINTF("  -Qr           - Print all remote packages\n");
				META_CONPRINTF("  -Qi  <name>   - Show information about local package\n");
				META_CONPRINTF("  -Qri <name>   - Show information about remote package\n");
				META_CONPRINTF("  -Qd           - Snapshot packages into manifest\n");
			} else if (arguments[1] == "version") {
				META_CONPRINTF(" Wizard %s\n", wizard->GetVersion());
			} else if (arguments[1] == "load") {
				if (auto packageManager = wizard->GetPackageManager().lock()) {
					if (packageManager->HasMissedPackages()) {
						packageManager->InstallMissedPackages();
						return;
					}
				}
				if (auto pluginManager = wizard->GetPluginManager().lock()) {
					pluginManager->Initialize();
				}
			} else if (arguments[1] == "unload") {
				if (auto pluginManager = wizard->GetPluginManager().lock()) {
					pluginManager->Terminate();
				}
			} else if (arguments[1] == "plugins") {
				if (auto pluginManager = wizard->GetPluginManager().lock()) {
					auto count = pluginManager->GetPlugins().size();
					if (!count) {
						META_CONPRINTF("No plugins loaded.\n");
					} else {
						META_CONPRINTF("Listing %d plugin%s:\n", static_cast<int>(count), (count > 1) ? "s" : "");
					}
					for (auto& pluginRef : pluginManager->GetPlugins()) {
						Print<wizard::PluginState>(pluginRef.get(), wizard::PluginStateToString);
					}
				}
			} else if (arguments[1] == "modules") {
				if (auto pluginManager = wizard->GetPluginManager().lock()) {
					auto count = pluginManager->GetModules().size();
					if (!count) {
						META_CONPRINTF("No modules loaded.\n");
					} else {
						META_CONPRINTF("Listing %d module%s:\n", static_cast<int>(count), (count > 1) ? "s" : "");
					}
					for (auto& moduleRef : pluginManager->GetModules()) {
						Print<wizard::ModuleState>(moduleRef.get(), wizard::ModuleStateToString);
					}
				}
			} else if (arguments[1] == "plugin" && arguments.size() > 2) {
				if (auto pluginManager = wizard->GetPluginManager().lock()) {
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
								// TODO: Print missing dependency
							}
						}
						META_CONPRINTF("  File: %s\n\n", plugin.GetFilePath().c_str());
					} else {
						META_CONPRINTF("Plugin %s not found.\n", arguments[2].c_str());
					}
				}
			} else if (arguments[1] == "module" && arguments.size() > 2) {
				if (auto pluginManager = wizard->GetPluginManager().lock()) {
					auto moduleRef = pluginManager->FindModule(arguments[2]);
					if (moduleRef.has_value()) {
						auto& module = moduleRef->get();
						Print<wizard::ModuleState>("Module", module, wizard::ModuleStateToString);
						META_CONPRINTF("  Language: %s\n", module.GetDescriptor().language.c_str());
						META_CONPRINTF("  File: %s\n\n", module.GetFilePath().c_str());
					} else {
						META_CONPRINTF("Module %s not found.\n", arguments[2].c_str());
					}
				}
			} else if (arguments[1] == "-Qd") {
				if (auto packageManager = wizard->GetPackageManager().lock()) {
					packageManager->SnapshotPackages(wizard->GetConfig().baseDir / std::format("snapshot_{}.wpackagemanifest", FormatTime("%Y_%m_%d_%H_%M_%S")), true);
				}
			} else if (arguments[1] == "-S" && arguments.size() > 2) {
				if (auto packageManager = wizard->GetPackageManager().lock()) {
					packageManager->InstallPackages(std::span(arguments.begin() + 2, arguments.size() - 2));
				}
			} else if (arguments[1] == "-Sf" && arguments.size() > 2) {
				if (auto packageManager = wizard->GetPackageManager().lock()) {
					packageManager->InstallAllPackages(std::filesystem::path{arguments[2]}, arguments.size() > 3);
				}
			} else if (arguments[1] == "-Sw" && arguments.size() > 2) {
				if (auto packageManager = wizard->GetPackageManager().lock()) {
					packageManager->InstallAllPackages(arguments[2], arguments.size() > 3);
				}
			} else if (arguments[1] == "-R" && arguments.size() > 2) {
				if (auto packageManager = wizard->GetPackageManager().lock()) {
					packageManager->UninstallPackages(std::span(arguments.begin() + 2, arguments.size() - 2));
				}
			} else if (arguments[1] == "-Ra") {
				if (auto packageManager = wizard->GetPackageManager().lock()) {
					packageManager->UninstallAllPackages();
				}
			} else if (arguments[1] == "-U" && arguments.size() > 2) {
				if (auto packageManager = wizard->GetPackageManager().lock()) {
					packageManager->UpdatePackages(std::span(arguments.begin() + 2, arguments.size() - 2));
				}
			} else if (arguments[1] == "-Ua") {
				if (auto packageManager = wizard->GetPackageManager().lock()) {
					packageManager->UpdateAllPackages();
				}
			} else if (arguments[1] == "-Q") {
				return; // TODO:
			} else if (arguments[1] == "-Qr") {
				return; // TODO:
			} else if (arguments[1] == "-Qi" && arguments.size() > 2) {
				return; // TODO:
			} else if (arguments[1] == "-Qri" && arguments.size() > 2) {
				return; // TODO:
			} else {
				META_CONPRINTF("unknown option: %s\n", arguments[1].c_str());
				META_CONPRINTF("usage: wizard <command> [arguments]\n");
				META_CONPRINTF("Try wizard help' for more information.\n");
			}
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
		return CS2WIZARD_PROJECT_VERSION;
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
		return CS2WIZARD_PROJECT_DESCRIPTION;
	}

	const char* WizardMMPlugin::GetName() {
		return CS2WIZARD_PROJECT_NAME;
	}

	const char* WizardMMPlugin::GetURL() {
		return CS2WIZARD_PROJECT_HOMEPAGE_URL;
	}

}