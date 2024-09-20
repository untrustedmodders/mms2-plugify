#pragma once

#include <functional>
#include <string>

#include <tier0/dbg.h>
#include <tier0/logging.h>

#include <plugify/log.h>
#include <plugify/compat_format.h>

#include <logger.hpp>

namespace plugifyMM
{
	using MMLoggerBase = Logger;

	class MMLogger final : public MMLoggerBase, public plugify::ILogger
	{
	public:
		using Base = MMLoggerBase;
		using Base::Base;

		~MMLogger() override = default;

		void SetSeverity(plugify::Severity severity);

	public: // plugify::ILogger
		void Log(std::string_view message, plugify::Severity severity);

	private:
		plugify::Severity m_severity { plugify::Severity::None };
	};
} // namespace plugifyMM
