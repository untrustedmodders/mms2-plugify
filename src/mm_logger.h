#pragma once

#include <plugify/log.h>

namespace plugifyMM {
	class MMLogger final : public plugify::ILogger {
	public:
		MMLogger() = default;
		~MMLogger() override = default;

		void SetSeverity(plugify::Severity severity) {
			_severity = severity;
		}

		void Log(const std::string& message, plugify::Severity severity) override;

	private:
		plugify::Severity _severity{ plugify::Severity::None };
	};
}
