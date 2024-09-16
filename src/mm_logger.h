#pragma once

#include <plugify/log.h>
#include <plugify/compat_format.h>

namespace plugifyMM
{
	class MMLogger final : public plugify::ILogger
	{
	public:
		void Log(std::string_view message, plugify::Severity severity);
		void SetSeverity(plugify::Severity severity);

	private:
		plugify::Severity m_severity{ plugify::Severity::None };
	};
} // namespace plugifyMM
