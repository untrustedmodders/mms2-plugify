#include "mm_logger.h"
#include "mm_plugin.h"

using namespace plugifyMM;

MMLogger::MMLogger(const char *pszName, RegisterTagsFunc pfnRegisterTagsFunc, int iFlags, LoggingVerbosity_t eVerbosity, const Color &aDefault)
{
	this->m_nChannelID = LoggingSystem_RegisterLoggingChannel(pszName, pfnRegisterTagsFunc, iFlags, eVerbosity, aDefault);
}

bool MMLogger::IsChannelEnabled(LoggingSeverity_t eSeverity) const
{
	return LoggingSystem_IsChannelEnabled(this->m_nChannelID, eSeverity);
}

bool MMLogger::IsChannelEnabled(LoggingVerbosity_t eVerbosity) const
{
	return LoggingSystem_IsChannelEnabled(this->m_nChannelID, eVerbosity);
}

LoggingVerbosity_t MMLogger::GetChannelVerbosity() const
{
	return LoggingSystem_GetChannelVerbosity(this->m_nChannelID);
}

Color MMLogger::GetColor() const
{
	Color rgba;
	rgba.SetRawColor(LoggingSystem_GetChannelColor(this->m_nChannelID));
	return rgba;
}

LoggingChannelFlags_t MMLogger::GetFlags() const
{
	return LoggingSystem_GetChannelFlags(this->m_nChannelID);
}

void MMLogger::SetSeverity(plugify::Severity severity)
{
	m_nSeverity = severity;
}

LoggingResponse_t MMLogger::InternalMessage(LoggingSeverity_t eSeverity, const char *pszContent)
{
	LoggingResponse_t eResponse = LR_ABORT;

	if (this->IsChannelEnabled(eSeverity))
	{
		eResponse = LoggingSystem_LogDirect(this->m_nChannelID, eSeverity, pszContent);
	}

	return eResponse;
}

LoggingResponse_t MMLogger::InternalMessage(LoggingSeverity_t eSeverity, const Color &aColor, const char *pszContent)
{
	LoggingResponse_t eResponse = LR_ABORT;

	if (this->IsChannelEnabled(eSeverity))
	{
		eResponse = LoggingSystem_LogDirect(this->m_nChannelID, eSeverity, aColor, pszContent);
	}

	return eResponse;
}

LoggingResponse_t MMLogger::InternalMessage(LoggingSeverity_t eSeverity, const LeafCodeInfo_t &aCode, const char *pszContent)
{
	LoggingResponse_t eResponse = LR_ABORT;

	if (this->IsChannelEnabled(eSeverity))
	{
		eResponse = LoggingSystem_LogDirect(this->m_nChannelID, eSeverity, aCode, pszContent);
	}

	return eResponse;
}

LoggingResponse_t MMLogger::InternalMessage(LoggingSeverity_t eSeverity, const LeafCodeInfo_t &aCode, const Color &aColor, const char *pszContent)
{
	LoggingResponse_t eResponse = LR_ABORT;

	if (this->IsChannelEnabled(eSeverity))
	{
		eResponse = LoggingSystem_LogDirect(this->m_nChannelID, eSeverity, aCode, aColor, pszContent);
	}

	return eResponse;
}

LoggingResponse_t MMLogger::InternalMessageFormat(LoggingSeverity_t eSeverity, const char *pszFormat, ...)
{
	LoggingResponse_t eResponse = LR_ABORT;

	if (this->IsChannelEnabled(eSeverity))
	{
		char sBuffer[1024];

		va_list aParams;

		va_start(aParams, pszFormat);
		V_vsnprintf((char *)sBuffer, sizeof(sBuffer), pszFormat, aParams);
		va_end(aParams);

		eResponse = this->InternalMessage(eSeverity, (const char *)sBuffer);
	}

	return eResponse;
}

LoggingResponse_t MMLogger::InternalMessageFormat(LoggingSeverity_t eSeverity, const Color &aColor, const char *pszFormat, ...)
{
	LoggingResponse_t eResponse = LR_ABORT;

	if (this->IsChannelEnabled(eSeverity))
	{
		char sBuffer[1024];

		va_list aParams;

		va_start(aParams, pszFormat);
		V_vsnprintf((char *)sBuffer, sizeof(sBuffer), pszFormat, aParams);
		va_end(aParams);

		eResponse = this->InternalMessage(eSeverity, aColor, (const char *)sBuffer);
	}

	return eResponse;
}

LoggingResponse_t MMLogger::InternalMessageFormat(LoggingSeverity_t eSeverity, const LeafCodeInfo_t &aCode, const char *pszFormat, ...)
{
	LoggingResponse_t eResponse = LR_ABORT;

	if (this->IsChannelEnabled(eSeverity))
	{
		char sBuffer[1024];

		va_list aParams;

		va_start(aParams, pszFormat);
		V_vsnprintf((char *)sBuffer, sizeof(sBuffer), pszFormat, aParams);
		va_end(aParams);

		eResponse = this->InternalMessage(eSeverity, aCode, (const char *)sBuffer);
	}

	return eResponse;
}

LoggingResponse_t MMLogger::InternalMessageFormat(LoggingSeverity_t eSeverity, const LeafCodeInfo_t &aCode, const Color &aColor, const char *pszFormat, ...)
{
	LoggingResponse_t eResponse = LR_ABORT;

	if (this->IsChannelEnabled(eSeverity))
	{
		char sBuffer[1024];

		va_list aParams;

		va_start(aParams, pszFormat);
		V_vsnprintf((char *)sBuffer, sizeof(sBuffer), pszFormat, aParams);
		va_end(aParams);

		eResponse = this->InternalMessage(eSeverity, aCode, aColor, (const char *)sBuffer);
	}

	return eResponse;
}

void MMLogger::Log(const std::string &message, plugify::Severity severity)
{
	if (severity <= m_nSeverity)
	{
		switch (severity)
		{
			case plugify::Severity::Fatal:
			{
				// this->ThrowAssertFormat({__FILE__, __LINE__, __FUNCTION__}, "%s\n", message.c_str());
				this->WarningFormat(Color(255, 0, 255, 255), "%s\n", message.c_str());
				break;
			}

			case plugify::Severity::Error:
			{
				// this->ErrorFormat( Color( 255, 0, 0, 255 ), "%s\n", message.c_str());
				this->WarningFormat(Color(255, 0, 0, 255), "%s\n", message.c_str());
				break;
			}

			case plugify::Severity::Warning:
			{
				this->WarningFormat(Color(255, 127, 0, 255), "%s\n", message.c_str());
				break;
			}

			case plugify::Severity::Info:
			{
				this->MessageFormat(Color(255, 255, 0, 255), "%s\n", message.c_str());
				break;
			}

			case plugify::Severity::Debug:
			{
				this->MessageFormat(Color(0, 255, 0, 255), "%s\n", message.c_str());
				break;
			}

			case plugify::Severity::Verbose:
			{
				this->MessageFormat(Color(255, 255, 255, 255), "%s\n", message.c_str());
				break;
			}

			case plugify::Severity::None:
			{
				break;
			}
		}
	}
}
