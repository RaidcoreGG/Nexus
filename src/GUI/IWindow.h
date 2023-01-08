#ifndef GUI_IWINDOW_H
#define GUI_IWINDOW_H

namespace GUI
{
	class IWindow
	{
	public:
		static bool Visible;

		IWindow() = default;
		~IWindow() = default;

		virtual void Show() = 0;
	};
}

#endif