#include "mm_logger.h"

using namespace plugifyMM;

void MMLogger::SetSeverity(plugify::Severity severity)
{
	m_severity = severity;
}

void MMLogger::Log(std::string_view message, plugify::Severity severity)
{
	if (severity <= m_severity)
	{
		std::string sMessage = std::format("{}\n", message);

		const char *pszMessage = sMessage.c_str();

		switch (severity)
		{
			case plugify::Severity::Fatal:
			{
				Error({255, 0, 255, 255}, pszMessage);

				break;
			}

			case plugify::Severity::Error:
			{
				Warning({255, 0, 0, 255}, pszMessage);

				break;
			}

			case plugify::Severity::Warning:
			{
				Warning({255, 127, 0, 255}, pszMessage);

				break;
			}

			case plugify::Severity::Info:
			{
				Message({255, 255, 0, 255}, pszMessage);

				break;
			}

			case plugify::Severity::Debug:
			{
				Detailed({0, 255, 255, 255}, pszMessage);

				break;
			}

			case plugify::Severity::Verbose:
			{
				Detailed({255, 255, 255, 255}, pszMessage);

				break;
			}

			case plugify::Severity::None:
			{
				break;
			}
		}
	}
}
