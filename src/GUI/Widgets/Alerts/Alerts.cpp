#include "Alerts.h"

#include "Shared.h"
#include "Renderer.h"

namespace GUI
{
	namespace Alerts
	{
		float						Opacity			= 1.0f;

		std::mutex					Mutex;
		std::vector<std::string>	QueuedAlerts;

		std::thread					AnimationThread;
		bool						IsAnimating;

		void Fade()
		{
			Opacity = 1.0f;

			Sleep(5000); /* show for 5 seconds before starting the fadeout */

			while (Opacity > 0)
			{
				Opacity -= 0.05f;
				Sleep(35);
			}

			if (QueuedAlerts.size() > 0) // sanity check
			{
				/* erase current message if it is finally hidden */
				QueuedAlerts.erase(QueuedAlerts.begin());
			}

			IsAnimating = false;
		}

		void Render()
		{
			if (QueuedAlerts.size() > 0)
			{
				std::string& message = QueuedAlerts.front();

				ImGui::PushFont(FontBig);
				float width = ImGui::CalcTextSize(message.c_str()).x;

				/* center horizontally */
				ImGui::SetNextWindowPos(ImVec2((Renderer::Width - width) / 2.0f, 230.0f * Renderer::Scaling));
				if (ImGui::Begin("##alerts", (bool*)0, WindowFlags_Overlay | ImGuiWindowFlags_NoBackground))
				{
					ImGui::TextColoredOutlined(ImVec4(1.0f, 1.0f, 0, Opacity), "%s", message.c_str());

					if (!IsAnimating)
					{
						// reset opacity here due to the thread taking longer to start than the pop message blow being reached
						IsAnimating = true;
						AnimationThread = std::thread(Fade);
						AnimationThread.detach();
					}
				}
				ImGui::End();

				ImGui::PopFont();
			}
		}

		void Notify(const char* aMessage)
		{
			std::string message = aMessage;

			const std::lock_guard<std::mutex> lock(Mutex);
			/* reset fade out if it's the same message */
			if (QueuedAlerts.size() > 0 && QueuedAlerts.front() == message)
			{
				Opacity = 1.0f;
			}
			else /* otherwise add it to the queue */
			{
				QueuedAlerts.push_back(message);
			}
		}
	}
}
