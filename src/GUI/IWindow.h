#ifndef GUI_IWINDOW_H
#define GUI_IWINDOW_H

namespace GUI
{
	class IWindow
	{
	public:
		bool Visible = false;

		IWindow() = default;
		~IWindow() = default;

		virtual void Render() = 0;
		virtual void MenuOption(int aCategory) = 0; /* aCategory: 0 = Main ; 1 = Debug ; 2 = Info */
	};
}

#endif