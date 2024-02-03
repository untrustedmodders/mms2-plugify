#include "mm_logger.h"
#include "mm_plugin.h"

namespace plugifyMM {
	void MMLogger::Log(const std::string& message, plugify::Severity severity)  {
		if (severity <= _severity) {
			switch (severity) {
				case plugify::Severity::Fatal: {
					Log_Error(LOG_GENERAL, "%s\n", message.c_str());
					break;
				}
				case plugify::Severity::Error: {
					Log_Warning(LOG_GENERAL, Color( 255, 0, 0, 255 ), "%s\n", message.c_str());
					break;
				}
				case plugify::Severity::Warning: {
					Log_Warning(LOG_GENERAL, Color( 255, 127, 0, 255 ), "%s\n", message.c_str());
					break;
				}
				case plugify::Severity::Info: {
					Log_Msg(LOG_GENERAL, Color( 255, 255, 0, 255 ), "%s\n", message.c_str());
					break;
				}
				case plugify::Severity::Debug: {
					Log_Msg(LOG_GENERAL, Color( 0, 255, 0, 255 ), "%s\n", message.c_str());
					break;
				}
				case plugify::Severity::Verbose:  {
					Log_Msg(LOG_GENERAL, Color( 255, 255, 255, 255 ), "%s\n", message.c_str());
					break;
				}
				case plugify::Severity::None:
					break;
			}
		}
	}
}
