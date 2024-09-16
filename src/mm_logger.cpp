#include "mm_logger.h"
#include "mm_plugin.h"

using namespace plugifyMM;

void MMLogger::Log(std::string_view message, plugify::Severity severity) {
	if (severity <= m_severity) {
		switch (severity) {
			case plugify::Severity::Fatal:
				META_CONPRINT(std::format("[@] Fatal: {}\n", message).c_str());
				break;
			case plugify::Severity::Error:
				META_CONPRINT(std::format("[#] Error: {}\n", message).c_str());
				break;
			case plugify::Severity::Warning:
				META_CONPRINT(std::format("[!] Warning: {}\n", message).c_str());
				break;
			case plugify::Severity::Info:
				META_CONPRINT(std::format("[+] Info: {}\n", message).c_str());
				break;
			case plugify::Severity::Debug:
				META_CONPRINT(std::format("[~] Debug: {}\n", message).c_str());
				break;
			case plugify::Severity::Verbose:
				META_CONPRINT(std::format("[*] Verbose: {}\n", message).c_str());
				break;
			default:
				META_CONPRINT(std::format("Unsupported error message logged {}\n", message).c_str());
				break;
		}
	}
}

void MMLogger::SetSeverity(plugify::Severity severity) {
	m_severity = severity;
}