#ifndef GUI_IWINDOW_H
#define GUI_IWINDOW_H

#include <string>

namespace GUI
{
	class IWindow
	{
	public:
		bool Visible = false;
		std::string Name;

		IWindow() = default;
		~IWindow() = default;

		virtual void Render() = 0;
	};
}

#endif