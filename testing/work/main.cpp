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
	auto test = sgui::text::make("Test", arial);
	
	auto button = sgui::button::make(sgui::vec2{}, sgui::vec2{ 50, 50 }, sgui::vec4{ 1, 0, 0, 1 });
	button->set_enter_callback([&button] { button->set_color({ 0, 1, 0, 1 }); });
	button->set_exit_callback([&button] { button->set_color({ 1, 0, 0, 1 }); });

	/*sgui::button sub_button({}, { 10, 10 }, { 1, 1, 0, 1 });
	sub_button.set_enter_callback([&sub_button] { sub_button.set_color({ 0, 0, 1, 1 }); });
	sub_button.set_exit_callback([&sub_button] { sub_button.set_color({ 1, 1, 0, 1 }); });
	sub_button.attach(button, sgui::attach_flags::centered);*/

	button->add_child(test, sgui::attach_flags::centered);
	window.add_child(button, sgui::attach_flags::centered);

	auto rounded_button = sgui::rounded_button::make(sgui::vec2{ 100, 100 }, sgui::vec2{ 300, 300 }, 20, sgui::vec4{ 1, 0, 0, 1 });
	rounded_button->set_enter_callback([&rounded_button] { rounded_button->set_color({ 0, 1, 0, 1 }); });
	rounded_button->set_exit_callback([&rounded_button] { rounded_button->set_color({ 1, 0, 0, 1 }); });
	window.add_child(rounded_button);

	return app.run();
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