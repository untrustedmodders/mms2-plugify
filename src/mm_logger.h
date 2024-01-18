#pragma once

#include <wizard/log.h>

namespace wizardMM {
	class MMLogger final : public wizard::ILogger {
	public:
		MMLogger() = default;
		~MMLogger() override = default;

		void SetSeverity(wizard::Severity severity) {
			_severity = severity;
		}

		void Log(const std::string& message, wizard::Severity severity) override;

	private:
		wizard::Severity _severity{ wizard::Severity::None };
	};
}
