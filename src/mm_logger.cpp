#include "mm_logger.h"
#include "mm_plugin.h"

namespace wizardMM {
	void MMLogger::Log(const std::string& message, wizard::Severity severity)  {
		if (severity <= _severity) {
			switch (severity) {
				case wizard::Severity::Fatal: {
					Log_Assert("%s\n", message.c_str());
					break;
				}
				case wizard::Severity::Error: {
					Log_Error(LOG_GENERAL, "%s\n", message.c_str());
					break;
				}
				case wizard::Severity::Warning: {
					Log_Warning(LOG_GENERAL, "%s\n", message.c_str());
					break;
				}
				case wizard::Severity::Info: {
					Log_Msg(LOG_GENERAL, Color( 255, 255, 0, 255 ), "%s\n", message.c_str());
					break;
				}
				case wizard::Severity::Debug: {
					Log_Msg(LOG_GENERAL, Color( 0, 255, 0, 255 ), "%s\n", message.c_str());
					break;
				}
				case wizard::Severity::Verbose:  {
					Log_Msg(LOG_GENERAL, Color( 255, 255, 255, 255 ), "%s\n", message.c_str());
					break;
				}
				case wizard::Severity::None:
					break;
			}
		}
	}
}
