#pragma once

#include <functional>
#include <string>

#include <tier0/dbg.h>
#include <tier0/logging.h>

#include <plugify/log.h>
#include <plugify/compat_format.h>

namespace plugifyMM
{
	class MMLogger final : public plugify::ILogger
	{
	public:
		MMLogger(const char *name, RegisterTagsFunc registerTagsFunc, int flags = 0, LoggingVerbosity_t verbosity = LV_DEFAULT, const Color &defaultColor = UNSPECIFIED_LOGGING_COLOR);
		~MMLogger() override = default;

		bool IsChannelEnabled(LoggingSeverity_t severity) const;
		bool IsChannelEnabled(LoggingVerbosity_t verbosity) const;
		LoggingVerbosity_t GetChannelVerbosity() const;
		Color GetColor() const;
		LoggingChannelFlags_t GetFlags() const;

		LoggingResponse_t Log(LoggingSeverity_t severity, const char *message);
		LoggingResponse_t Log(LoggingSeverity_t severity, const Color &color, const char *message);
		LoggingResponse_t Log(LoggingSeverity_t severity, const LeafCodeInfo_t &code, const char *message);
		LoggingResponse_t Log(LoggingSeverity_t severity, const LeafCodeInfo_t &code, const Color &color, const char *message);

		//LoggingResponse_t LogFormat(LoggingSeverity_t severity, const char *format, ...);
		//LoggingResponse_t LogFormat(LoggingSeverity_t severity, const Color &color, const char *format, ...);
		//LoggingResponse_t LogFormat(LoggingSeverity_t severity, const LeafCodeInfo_t &code, const char *format, ...);
		//LoggingResponse_t LogFormat(LoggingSeverity_t severity, const LeafCodeInfo_t &code, const Color &color, const char *format, ...);

		/*plugify*/
		void Log(std::string_view message, plugify::Severity severity);
		void SetSeverity(plugify::Severity severity);

	private:
		plugify::Severity m_severity{ plugify::Severity::None };
		LoggingChannelID_t m_channelID;
	};
} // namespace plugifyMM
