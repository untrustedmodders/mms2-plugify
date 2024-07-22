#include "mm_logger.h"
#include "mm_plugin.h"

using namespace plugifyMM;

MMLogger::MMLogger(const char *name, RegisterTagsFunc registerTagsFunc, int flags, LoggingVerbosity_t verbosity, const Color &defaultColor)
{
	m_channelID = LoggingSystem_RegisterLoggingChannel(name, registerTagsFunc, flags, verbosity, defaultColor);
}

bool MMLogger::IsChannelEnabled(LoggingSeverity_t severity) const
{
	return LoggingSystem_IsChannelEnabled(m_channelID, severity);
}

bool MMLogger::IsChannelEnabled(LoggingVerbosity_t verbosity) const
{
	return LoggingSystem_IsChannelEnabled(m_channelID, verbosity);
}

LoggingVerbosity_t MMLogger::GetChannelVerbosity() const
{
	return LoggingSystem_GetChannelVerbosity(m_channelID);
}

Color MMLogger::GetColor() const
{
	Color rgba;
	rgba.SetRawColor(LoggingSystem_GetChannelColor(m_channelID));
	return rgba;
}

LoggingChannelFlags_t MMLogger::GetFlags() const
{
	return LoggingSystem_GetChannelFlags(m_channelID);
}

void MMLogger::SetSeverity(plugify::Severity severity)
{
	m_severity = severity;
}

LoggingResponse_t MMLogger::Log(LoggingSeverity_t severity, const char *message)
{
	LoggingResponse_t response = LR_ABORT;

	if (IsChannelEnabled(severity))
	{
		response = LoggingSystem_LogDirect(m_channelID, severity, message);
	}

	return response;
}

LoggingResponse_t MMLogger::Log(LoggingSeverity_t severity, const Color &color, const char *message)
{
	LoggingResponse_t response = LR_ABORT;

	if (IsChannelEnabled(severity))
	{
		response = LoggingSystem_LogDirect(m_channelID, severity, color, message);
	}

	return response;
}

LoggingResponse_t MMLogger::Log(LoggingSeverity_t severity, const LeafCodeInfo_t &code, const char *message)
{
	LoggingResponse_t response = LR_ABORT;

	if (IsChannelEnabled(severity))
	{
		response = LoggingSystem_LogDirect(m_channelID, severity, code, message);
	}

	return response;
}

LoggingResponse_t MMLogger::Log(LoggingSeverity_t severity, const LeafCodeInfo_t &code, const Color &color, const char *message)
{
	LoggingResponse_t response = LR_ABORT;

	if (IsChannelEnabled(severity))
	{
		response = LoggingSystem_LogDirect(m_channelID, severity, code, color, message);
	}

	return response;
}

LoggingResponse_t MMLogger::LogFormat(LoggingSeverity_t severity, const char *format, ...)
{
	char buffer[MAX_LOGGING_MESSAGE_LENGTH];
	
	va_list params;

	va_start(params, format);
	V_vsnprintf((char *)buffer, sizeof(buffer), format, params);
	va_end(params);

	return Log(severity, buffer);
}

LoggingResponse_t MMLogger::LogFormat(LoggingSeverity_t severity, const Color &color, const char *format, ...)
{
	LoggingResponse_t response = LR_ABORT;
	
	if (IsChannelEnabled(severity))
	{
		char buffer[1024];

		va_list params;

		va_start(params, format);
		V_vsnprintf((char *)buffer, sizeof(buffer), format, params);
		va_end(params);

		response = Log(severity, color, buffer);
	}

	return response;
}

LoggingResponse_t MMLogger::LogFormat(LoggingSeverity_t severity, const LeafCodeInfo_t &code, const char *format, ...)
{
	LoggingResponse_t response = LR_ABORT;

	if (IsChannelEnabled(severity))
	{
		char buffer[1024];

		va_list params;

		va_start(params, format);
		V_vsnprintf((char *)buffer, sizeof(buffer), format, params);
		va_end(params);

		response = Log(severity, code, buffer);
	}

	return response;
}

LoggingResponse_t MMLogger::LogFormat(LoggingSeverity_t severity, const LeafCodeInfo_t &code, const Color &color, const char *format, ...)
{
	LoggingResponse_t response = LR_ABORT;

	if (IsChannelEnabled(severity))
	{
		char buffer[1024];

		va_list params;

		va_start(params, format);
		V_vsnprintf((char *)buffer, sizeof(buffer), format, params);
		va_end(params);

		response = Log(severity, code, color, buffer);
	}

	return response;
}

void MMLogger::Log(std::string_view message, plugify::Severity severity)
{
	if (severity <= m_severity)
	{
		switch (severity)
		{
			case plugify::Severity::Fatal:
			{
				LogFormat(LS_ERROR, Color(255, 0, 255, 255), "%s\n", message.data());
				break;
			}

			case plugify::Severity::Error:
			{
				LogFormat(LS_WARNING, Color(255, 0, 0, 255), "%s\n", message.data());
				break;
			}

			case plugify::Severity::Warning:
			{
				LogFormat(LS_WARNING, Color(255, 127, 0, 255), "%s\n", message.data());
				break;
			}

			case plugify::Severity::Info:
			{
				LogFormat(LS_MESSAGE, Color(255, 255, 0, 255), "%s\n", message.data());
				break;
			}

			case plugify::Severity::Debug:
			{
				LogFormat(LS_MESSAGE, Color(0, 255, 0, 255), "%s\n", message.data());
				break;
			}

			case plugify::Severity::Verbose:
			{
				LogFormat(LS_MESSAGE, Color(255, 255, 255, 255), "%s\n", message.data());
				break;
			}

			case plugify::Severity::None:
			{
				break;
			}
		}
	}
}
