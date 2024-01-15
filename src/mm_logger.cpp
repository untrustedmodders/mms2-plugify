#include "mm_logger.h"
#include "mm_plugin.h"

namespace cs2wizard {
	void MMLogger::Log(const std::string& message, wizard::Severity severity)  {
		if (severity <= _severity) {
			switch (severity) {
				case wizard::Severity::Fatal: {
					META_CONPRINTF("[@] Fatal: %s\n", message.c_str());
					break;
				}
				case wizard::Severity::Error: {
					META_CONPRINTF("[#] Error: %s\n", message.c_str());
					break;
				}
				case wizard::Severity::Warning: {
					META_CONPRINTF("[!] Warning: %s\n", message.c_str());
					break;
				}
				case wizard::Severity::Info: {
					META_CONPRINTF("[+] Info: %s\n", message.c_str());
					break;
				}
				case wizard::Severity::Debug: {
					META_CONPRINTF("[~] Debug: %s\n", message.c_str());
					break;
				}
				case wizard::Severity::Verbose:  {
					META_CONPRINTF("[*] Verbose: %s\n", message.c_str());
					break;
				}
				case wizard::Severity::None:
					META_CONPRINT("Unsupported error message logged\n");
					break;
			}
		}
	}
}
