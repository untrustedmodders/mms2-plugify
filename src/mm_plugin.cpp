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

#include <stdio.h>
#include "mm_plugin.h"
#include "mm_logger.h"
#include <igameevents.h>
#include <iserver.h>
#include <wizard/wizard.h>
#include <filesystem>

namespace cs2wizard {
	IServerGameDLL* server = NULL;
	IServerGameClients* gameclients = NULL;
	IVEngineServer* engine = NULL;
	IGameEventManager2* gameevents = NULL;
	ICvar* icvar = NULL;

	WizardMMPlugin g_Plugin;
	PLUGIN_EXPOSE(WizardMMPlugin, g_Plugin);

	CON_COMMAND_F(wizard, "Wizard control options", FCVAR_NONE) {
		std::span arguments(args.ArgV(), args.ArgC());

		if (args.ArgC() > 1) {
			if (!strcmp(arguments[1], "version")) {
				META_CONPRINTF(" Wizard %s\n", g_Plugin._context->GetVersion());
				return;
			} else if (!strcmp(arguments[1], "load")) {
				return;
			} else if (!strcmp(arguments[1], "unload")) {
				return;
			} else if (!strcmp(arguments[1], "modules")) {
				return;
			} else if (!strcmp(arguments[1], "plugins")) {
				return;
			} else if (!strcmp(arguments[1], "plugin")) {
				return;
			} else if (!strcmp(arguments[1], "module")) {
				return;
			}
		}

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

		return _context->Initialize(Plat_GetGameDirectory());
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