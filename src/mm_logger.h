#pragma once

#include <wizard/log.h>

namespace wizard {
	class MMLogger final : public wizard::ILogger {
	public:
		MMLogger() = default;
		~MMLogger() override = default;

		void SetSeverity(wizard::Severity severity) {
			_severity = severity;
		}

		void Log(const std::string& message, wizard::Severity severity) override {
			if (severity <= _severity) {
				switch (severity) {
					case wizard::Severity::Fatal: {
						std::string msg{"[@] Fatal: " + message};
						META_LOG(&g_Plugin, msg.c_str());
						break;
					}
					case wizard::Severity::Error: {
						std::string msg{"[#] Error: " + message};
						META_LOG(&g_Plugin, msg.c_str());
						break;
					}
					case wizard::Severity::Warning: {
						std::string msg{"[!] Warning: " + message};
						META_LOG(&g_Plugin, msg.c_str());
						break;
					}
					case wizard::Severity::Info: {
						std::string msg{"[+] Info: " + message};
						META_LOG(&g_Plugin, msg.c_str());
						break;
					}
					case wizard::Severity::Debug: {
						std::string msg{"[~] Debug: " + message};
						META_LOG(&g_Plugin, msg.c_str());
						break;
					}
					case wizard::Severity::Verbose:  {
						std::string msg{"[*] Verbose: " + message};
						META_LOG(&g_Plugin, msg.c_str());
						break;
					}
					case Severity::None:
						META_CONPRINTF("Unsupported error message logged\n");
						break;
				}
			}
		}

	private:
		wizard::Severity _severity{ wizard::Severity::None };
	};
}
