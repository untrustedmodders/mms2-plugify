#include "mm_logger.hpp"

using namespace mm;

Logger::Logger(const char* name, int flags, LoggingVerbosity_t verbosity, const Color& defaultColor) {
	m_channelID = LoggingSystem_RegisterLoggingChannel(name, nullptr, flags, verbosity, defaultColor);
}

bool Logger::IsChannelEnabled(LoggingSeverity_t severity) const {
	return LoggingSystem_IsChannelEnabled(m_channelID, severity);
}

bool Logger::IsChannelEnabled(LoggingVerbosity_t verbosity) const {
	return LoggingSystem_IsChannelEnabled(m_channelID, verbosity);
}

LoggingVerbosity_t Logger::GetChannelVerbosity() const {
	return LoggingSystem_GetChannelVerbosity(m_channelID);
}

Color Logger::GetColor() const {
	Color rgba;
	rgba.SetRawColor(LoggingSystem_GetChannelColor(m_channelID));
	return rgba;
}

LoggingChannelFlags_t Logger::GetFlags() const {
	return LoggingSystem_GetChannelFlags(m_channelID);
}

void Logger::SetSeverity(plugify::Severity severity) {
	m_severity = severity;
}

LoggingResponse_t Logger::Log(LoggingSeverity_t severity, const char* message) const {
	LoggingResponse_t response = LR_ABORT;

	if (IsChannelEnabled(severity)) {
		response = LoggingSystem_LogDirect(m_channelID, severity, message);
	}

	return response;
}

LoggingResponse_t Logger::Log(LoggingSeverity_t severity, const Color& color, const char* message) const {
	LoggingResponse_t response = LR_ABORT;

	if (IsChannelEnabled(severity)) {
		response = LoggingSystem_LogDirect(m_channelID, severity, color, message);
	}

	return response;
}

LoggingResponse_t Logger::Log(LoggingSeverity_t severity, const LeafCodeInfo_t& code, const char* message) const {
	LoggingResponse_t response = LR_ABORT;

	if (IsChannelEnabled(severity)) {
		response = LoggingSystem_LogDirect(m_channelID, severity, code, message);
	}

	return response;
}

LoggingResponse_t Logger::Log(LoggingSeverity_t severity, const LeafCodeInfo_t& code, const Color& color, const char* message) const {
	LoggingResponse_t response = LR_ABORT;

	if (IsChannelEnabled(severity)) {
		response = LoggingSystem_LogDirect(m_channelID, severity, code, color, message);
	}

	return response;
}

void Logger::Log(std::string_view message, plugify::Severity severity) {
	if (severity <= m_severity) {
		std::string sMessage = std::format("{}\n", message);

		switch (severity) {
			case plugify::Severity::None: {
				Log(LS_MESSAGE, Color(255, 255, 255, 255), sMessage.c_str());
				break;
			}

			case plugify::Severity::Fatal: {
				Log(LS_ERROR, Color(255, 0, 255, 255), sMessage.c_str());
				break;
			}

			case plugify::Severity::Error: {
				Log(LS_WARNING, Color(255, 0, 0, 255), sMessage.c_str());
				break;
			}

			case plugify::Severity::Warning: {
				Log(LS_WARNING, Color(255, 127, 0, 255), sMessage.c_str());
				break;
			}

			case plugify::Severity::Info: {
				Log(LS_MESSAGE, Color(255, 255, 0, 255), sMessage.c_str());
				break;
			}

			case plugify::Severity::Debug: {
				Log(LS_MESSAGE, Color(0, 255, 0, 255), sMessage.c_str());
				break;
			}

			case plugify::Severity::Verbose: {
				Log(LS_MESSAGE, Color(255, 255, 255, 255), sMessage.c_str());
				break;
			}

			default: {
				break;
			}
		}
	}
}
