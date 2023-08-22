#include <gui/application.h>
#include <gui/widget.h>
#include <gui/text.h>

#include <iostream>

int main(int argc, char *argv[])
{
	sgui::application app;

	sgui::window window("testing", { 960, 540 });
	app.set_window(window);

	sgui::font arial("arial.ttf", 128);
	sgui::text test("Test", arial);

	sgui::button button({}, { 50, 50 }, { 1, 0, 0, 1 });
	button.set_enter_callback([&button] { button.set_color({ 0, 1, 0, 1 }); });
	button.set_exit_callback([&button] { button.set_color({ 1, 0, 0, 1 }); });

	/*sgui::button sub_button({}, { 10, 10 }, { 1, 1, 0, 1 });
	sub_button.set_enter_callback([&sub_button] { sub_button.set_color({ 0, 0, 1, 1 }); });
	sub_button.set_exit_callback([&sub_button] { sub_button.set_color({ 1, 1, 0, 1 }); });
	sub_button.attach(button, sgui::attach_flags::centered);*/

	button.attach(window, sgui::attach_flags::centered);

	test.attach(button, sgui::attach_flags::centered);

	app.run();
}

#ifdef _WIN32 && WINDOWED_APP == 1
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int argc = __argc;
	char **argv = __argv;

	return main(argc, argv);
}
#endif