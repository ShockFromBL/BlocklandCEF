#pragma once

#include <Windows.h>

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include\cef_base.h"

#include <unordered_map>
#include <vector>

#pragma comment(lib, "libcef.lib")
#pragma comment(lib, "libcef_dll_wrapper.lib")

class BLCefApp : public CefApp {

	public:

		BLCefApp() {
			
		};
		// CefBrowserProcessHandler methods:
		//rtual void OnRegisterCustomSchemes() OVERRIDE;

	private:

		// Include the default reference counting implementation.
		IMPLEMENT_REFCOUNTING(BLCefApp);

};

class SubApp : public BLCefApp, public CefRenderProcessHandler {
	
	public:

		virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE {
			return this;
		}

		virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE {
			auto args = message->GetArgumentList();

			return false;
		}

	private:

		IMPLEMENT_REFCOUNTING(SubApp);

};

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	CefMainArgs args(hInstance);
	return CefExecuteProcess(args, new SubApp(), nullptr);
}