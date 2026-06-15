#include "Application.h"

#include "../Shell/MainFrame.h"

bool Application::OnInit()
{
	if (not wxApp::OnInit())
	{
		return false;
	}

	MainFrame *frame = new MainFrame();
	frame->Show(true);
	frame->PresentStartDialog();

	return true;
}
