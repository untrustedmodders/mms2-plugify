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
		META_CONPRINTF("Sample command called by %d. Command: %s\n", context.GetPlayerSlot(), args.GetCommandString());
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
		return "1.0.0.0";
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
		return "Package & Plugin Manager";
	}

	const char* WizardMMPlugin::GetName() {
		return "CS2 Wizard";
	}

	const char* WizardMMPlugin::GetURL() {
		return "https://github.com/untrustedmodders/cs2wizard";
	}

}