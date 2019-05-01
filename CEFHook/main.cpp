#pragma once

#include <windows.h>
#include <stdio.h>
#include "include\cef_base.h"
#include "torque.h"

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/cef_base.h"
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"

#include <stdlib.h>
#include "glapi.h"

#include "MologieDetours/detours.h"

#include <gl\GL.h>
#include "glext.h"

#pragma comment(lib, "libcef.lib")
#pragma comment(lib, "libcef_dll_wrapper.lib")

#define CEFHOOK_VERS_MAJ "0"
#define CEFHOOK_VERS_MIN "1"
#define CEFHOOK_VERS_REV "2"

struct TextureObject {
	TextureObject *next;
	TextureObject *prev;
	TextureObject *hashNext;
	unsigned int texGLName;
	unsigned int smallTexGLName;
	const char *texFileName;
	DWORD *type_GBitmap_bitmap;
	unsigned int texWidth;
	unsigned int texHeight;
	unsigned int bitmapWidth;
	unsigned int bitmapHeight;
	unsigned int downloadedWidth;
	unsigned int downloadedHeight;
	unsigned int enum_TextureHandleType_type;
	bool filterNearest;
	bool clamp;
	bool holding;
	int refCount;
};

CefRefPtr<CefBrowser> BLCefInstance;

typedef int(*swapBuffersFn)();
typedef cef_paint_element_type_t PaintElementType;

static int textureID = 0; // Texture ID that we bind to with OpenGL.
static char* textureFile = "Add-Ons/Print_Screen_Cinema/prints/Cinema.png"; // Default texture file to look for when binding.

static bool debug = false; // Is debug mode enabled?
static bool dirty = false; // Do we need to render the texture?

static int global_ww = 1024; // Width.
static int global_hh = 768; // Height, both are used in swapBuffers to determine what should be copied over.

MologieDetours::Detour<swapBuffersFn>* swapBuffers_detour;
GLuint* texBuffer;

HANDLE cefThread;
HANDLE cefStopEvent;

// The detour so we can draw our stuff before Torque can.
int __fastcall swapBuffers_hook() {
	if (textureID != 0 && dirty) {
		BL_glBindTexture(GL_TEXTURE_2D, textureID);
		BL_glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, global_ww, global_hh, GL_BGRA_EXT, GL_UNSIGNED_BYTE, texBuffer);

		BL_glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		BL_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		dirty = false;
	}

	int ret = swapBuffers_detour->GetOriginalFunction()();

	return ret;
}

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

class BLCefRenderer : public CefRenderHandler {

	public:

		BLCefRenderer(int w, int h) : height(h), width(w) {
			if (!BL_glGenBuffers) {
				if (!BL_glGenBuffersARB) {
					texBuffer = (GLuint*)malloc(2048 * 2048 * 4);
					memset((void*)texBuffer, 0, 2048 * 2048 * 4);
				} else {
					BL_glGenBuffersARB(1, &*texBuffer);
					BL_glBindBufferARB(GL_TEXTURE_BUFFER, *texBuffer);
				}
			}
		};

		~BLCefRenderer() {
			if (BL_glDeleteBuffersARB) {
				BL_glDeleteBuffersARB(1, &*texBuffer);
			} else {
				delete texBuffer;
			}
		};

		CefRefPtr<CefAccessibilityHandler> GetAccessibilityHandler() {
			return nullptr;
		}

		bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
			rect = CefRect(0, 0, width, height);
			return true;
		}

		bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
			rect = CefRect(0, 0, width, height);
			return true;
		}

		bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) {
			screen_info.rect = CefRect(0, 0, width, height);
			screen_info.device_scale_factor = 1.0;
			return true;
		}

		void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CefRenderHandler::CursorType type, const CefCursorInfo& custom_cursor_info) {

		}

		void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser, const CefRange& selected_range, const CefRenderHandler::RectList& character_bounds) {

		}

		void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList& dirtyRects, const void* buffer, int w, int h) {
			if (textureID != 0) {
				if (BL_glBindBufferARB) {
					BL_glBindBufferARB(GL_TEXTURE_BUFFER, *texBuffer);
					BL_glBufferDataARB(GL_TEXTURE_BUFFER, width * height * 4, buffer, GL_DYNAMIC_DRAW);
				} else {
					memcpy(texBuffer, buffer, w * h * 4);
				}

				dirty = true;
			}
		}

		void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser, double x, double y) {

		}

		bool StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr< CefDragData > drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y) {
			return false;
		}

		void UpdateDragCursor(CefRefPtr<CefBrowser> browser, CefRenderHandler::DragOperation operation) {

		}

		void UpdateResolution(int hh, int ww) {
			if (hh * ww * 4 > 16777215) {
				Printf("CEF resolution too high.");
				return;
			}
		
			memset(texBuffer, 0, 2048 * 2048 * 4);
			height = hh;
			width = ww;
			global_ww = width;
			global_hh = height;
		}

	private:

		IMPLEMENT_REFCOUNTING(BLCefRenderer);
		int height, width;

};

CefRefPtr<BLCefRenderer> renderHandler;

class BLCefClient : public CefClient, public CefLifeSpanHandler, public CefLoadHandler {

	public:

		BLCefClient(CefRefPtr<CefRenderHandler> ptr) : renderHandler(ptr) {

		}

		virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() {
			return this;
		}

		virtual CefRefPtr<CefLoadHandler> GetLoadHandler() {
			return this;
		}

		virtual CefRefPtr<CefRenderHandler> GetRenderHandler() {
			return renderHandler;
		}

		// CefLifeSpanHandler methods.
		void OnAfterCreated(CefRefPtr<CefBrowser> browser) {
			// Must be executed on the UI thread.
			//CEF_REQUIRE_UI_THREAD();

			if (browser_id == 0)
				browser_id = browser->GetIdentifier();
		}

		bool DoClose(CefRefPtr<CefBrowser> browser) {
			// Must be executed on the UI thread.
			//CEF_REQUIRE_UI_THREAD();

			// Closing the main window requires special handling. See the DoClose()
			// documentation in the CEF header for a detailed description of this
			// process.
			if (browser->GetIdentifier() == browser_id) {
				// Set a flag to indicate that the window close should be allowed.
				closing = true;
			}

			// Allow the close. For windowed browsers this will result in the OS close
			// event being sent.
			return false;
		}

		void OnBeforeClose(CefRefPtr<CefBrowser> browser) {

		}

		// Popup mitigation.
		bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access) {
			//if (browser->GetMainFrame()->GetIdentifier() == frame->GetIdentifier())
			//	frame->LoadURL(target_url);

			return true;
		}

		void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) {
			loaded = true;
		}

		bool OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString & failedUrl, CefString & errorText) {
			loaded = true;
		}

		void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) {

		}

		void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) {

		}

		bool closeAllowed() const {
			return closing;
		}

		bool isLoaded() const {
			return loaded;
		}

	private:

		int browser_id = 0;
		bool closing = false;
		bool loaded = false;
		CefRefPtr<CefRenderHandler> renderHandler;

		IMPLEMENT_REFCOUNTING(BLCefClient);

};

// Bind to the texture representing a CEF screen.
bool bindTextureID(const char* file) {
	TextureObject* texture;
	textureID = 0;

	for (texture = (TextureObject*)0x7868E0; texture; texture = texture->next) {
		if (texture->texFileName != 0 && _stricmp(texture->texFileName, file) == 0) {
			textureID = texture->texGLName;

			BLCefInstance->GetHost()->Invalidate(PaintElementType::PET_VIEW);

			dirty = true;

			if (debug)
				Printf("BT succ %s", file);

			return true;
		}
	}

	if (debug)
		Printf("BT fail %s", file);

	return false;
}

// Visit a URL.
void ts_goToURL(SimObject* this_, int argc, const char* argv[]) {
	if (debug)
		Printf("URL %s", argv[1]);

	if (BLCefInstance.get() != nullptr) {
		BLCefInstance->GetMainFrame()->LoadURL(CefString(argv[1]));
	} else {
		Printf("CEF instance does not exist.");
	}
}

// Bind to the texture representing a CEF screen.
bool ts_bindTexture(SimObject* this_, int argc, const char* argv[]) {
	return bindTextureID(textureFile);
}

// Set to a texture representing a CEF screen.
void ts_setTexture(SimObject* this_, int argc, const char* argv[]) {
	if (argv[1] != nullptr) {
		char* tf = _strdup(textureFile);
		textureFile = tf;
	} else {
		textureFile = "Add-Ons/Print_Screen_Cinema/prints/Cinema.png";
	}

	if (debug)
		Printf("ST %s", textureFile);
}

// Get the texture currently representing a CEF screen.
const char* ts_getTexture(SimObject* this_, int argc, const char* argv[]) {
	return textureFile;
}

// Resize the CEF window, reallocating the texture buffer.
void ts_setResolution(SimObject* this_, int argc, const char* argv[]) {
	if (debug)
		Printf("SR h:%s w:%s", argv[1], argv[2]);

	if (BLCefInstance.get() != nullptr) {
		int width = atoi(argv[1]);
		int height = atoi(argv[2]);

		renderHandler->UpdateResolution(width, height);

		BLCefInstance->GetHost()->WasResized();
	} else {
		Printf("CEF instance does not exist.");
	}
}

// Execute a line of JavaScript.
void ts_execJS(SimObject* this_, int argc, const char* argv[]) {
	if (debug)
		Printf("JS %s", argv[1]);

	if (BLCefInstance.get() != nullptr) {
		CefString js = CefString(argv[1]);

		BLCefInstance->GetMainFrame()->ExecuteJavaScript(js, "", 0);
	}
	else {
		Printf("CEF instance does not exist.");
	}
}

// Move the mouse to a new position.
void ts_mouseMove(SimObject* this_, int argc, const char* argv[]) {
	if (debug)
		Printf("MM x:%s y:%s", argv[1], argv[2]);

	if (BLCefInstance.get() != nullptr) {
		CefMouseEvent* mouseEvent = new CefMouseEvent();

		mouseEvent->x = atoi(argv[1]);
		mouseEvent->y = atoi(argv[2]);

		BLCefInstance->GetHost()->SendMouseMoveEvent(*mouseEvent, false);

		delete mouseEvent;
	} else {
		Printf("CEF instance does not exist.");
	}
}

// Send a click event on the specified co-ordinates.
void ts_mouseClick(SimObject* this_, int argc, const char* argv[]) {
	if (debug)
		Printf("MC x:%s y:%s ct:%s", argv[1], argv[2], argv[3]);

	if (BLCefInstance.get() != nullptr) {
		CefMouseEvent* mouseEvent = new CefMouseEvent();

		mouseEvent->x = atoi(argv[1]);
		mouseEvent->y = atoi(argv[2]);
	
		int clickType = atoi(argv[3]);
		BLCefInstance->GetHost()->SendMouseClickEvent(*mouseEvent, (cef_mouse_button_type_t)clickType, false, 1);
		BLCefInstance->GetHost()->SendMouseClickEvent(*mouseEvent, (cef_mouse_button_type_t)clickType, true, 1);

		delete mouseEvent;
	} else {
		Printf("CEF instance does not exist.");
	}
}

// Send a mousewheel event at the specified co-ordinates.
void ts_mouseWheel(SimObject* this_, int argc, const char* argv[]) {
	if (debug)
		Printf("MW x:%s y:%s", argv[1], argv[2]);

	if (BLCefInstance.get() != nullptr) {
		CefMouseEvent* mouseEvent = new CefMouseEvent();

		mouseEvent->x = atoi(argv[1]);
		mouseEvent->y = atoi(argv[2]);

		int deltaX = atoi(argv[3]);
		int deltaY = atoi(argv[4]);
		BLCefInstance->GetHost()->SendMouseWheelEvent(*mouseEvent, deltaX, deltaY);

		delete mouseEvent;
	} else {
		Printf("CEF instance does not exist.");
	}
}

// Send a keyboard event.
void ts_keyboardEvent(SimObject* this_, int argc, const char* argv[]) {
	if (debug)
		Printf("KBE chr:%s mod:%s", argv[1][0], argv[2]);

	if (BLCefInstance.get() != nullptr) {
		CefKeyEvent* keyEvent = new CefKeyEvent();

		keyEvent->character = argv[1][0];
		keyEvent->modifiers = atoi(argv[2]);
		BLCefInstance->GetHost()->SendKeyEvent(*keyEvent);

		delete keyEvent;
	} else {
		Printf("CEF instance does not exist.");
	}
}

// Turns debug mode on or off.
void ts_debug(SimObject* this_, int argc, const char* argv[]) {
	if (atoi(argv[1]) == 1) {
		debug = true;

		Printf("CEF debugging enabled.");
	} else {
		debug = false;

		Printf("CEF debugging disabled.");
	}
}

bool attach();
bool detach();

// Initialize Blockland CEF.
bool ts_attach(SimObject* this_, int argc, const char* argv[]) {
	return attach();
}

// De-initialize Blockland CEF.
bool ts_detach(SimObject* this_, int argc, const char* argv[]) {
	return detach();
}

static bool* isRunning = new bool(false);

// Main CEF work loop.
unsigned long __stdcall mainLoop(LPVOID lpParam) {
	cefStopEvent = (HANDLE)lpParam;

	CefMainArgs args;

	CefSettings settings;
	settings.single_process = false;
	settings.command_line_args_disabled = true;
	settings.no_sandbox = true;
	settings.windowless_rendering_enabled = true;
	settings.persist_user_preferences = false;
	settings.log_severity = cef_log_severity_t::LOGSEVERITY_WARNING;
	//settings.remote_debugging_port = 9222;
	//CefString(&settings.resources_dir_path).FromASCII("./CEF");
	//CefString(&settings.locales_dir_path).FromASCII("./CEF/locales");
	//CefString(&settings.log_file).FromASCII("./CEF/debug.log");
	CefString(&settings.browser_subprocess_path).FromASCII("BlocklandCEFSubProcess.exe");
	CefString(&settings.user_agent).FromASCII("Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.146 Safari/537.36 Blockland/r1997 (Torque Game Engine/1.3)");
	CefString(&settings.cache_path).FromASCII("");

	if (!CefInitialize(args, settings, new BLCefApp(), nullptr)) {
		Printf("Initialization of CEF failed.");
		return false;
	} else {
		renderHandler = new BLCefRenderer(global_ww, global_hh);
		CefBrowserSettings browser_settings;
		CefWindowInfo window_info;
		CefRefPtr<BLCefClient> browserClient;
		browserClient = new BLCefClient(renderHandler);
		browser_settings.background_color = CefColorSetARGB(255, 255, 255, 255);
		browser_settings.windowless_frame_rate = 60;
		browser_settings.javascript_access_clipboard = STATE_DISABLED;
		//browser_settings.local_storage = STATE_DISABLED;
		browser_settings.file_access_from_file_urls = STATE_DISABLED;
		browser_settings.universal_access_from_file_urls = STATE_DISABLED;
		window_info.SetAsWindowless(0);
		BLCefInstance = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), "about:blank", browser_settings, NULL);

		Printf("Initialization of CEF complete, CEF is now running.");

		*isRunning = true;

		unsigned long cefSignal;

		while (true) {
			cefSignal = WaitForSingleObject(cefStopEvent, 0);

			if (cefSignal == WAIT_OBJECT_0) {
				Printf("CEF is shutting down...");
				break;
			}

			CefDoMessageLoopWork();

			Sleep(1);
		}
	}

	BLCefInstance->GetHost()->CloseBrowser(true);
	CefShutdown();

	Printf("CEF has shutdown.");

	*isRunning = false;

	return true;
}

static bool* hasInit = new bool(false);

// Pre-initialize Blockland CEF.
bool init() {
	if (!torque_init())
		return false;

	if (*hasInit)
		return false;

	Printf("Pre-initialization of CEF has started...");

	ConsoleFunction(0, "CEF_attach", ts_attach, "() - (Re-)initialize CEF.", 1, 1);
	ConsoleFunction(0, "CEF_detach", ts_detach, "() - De-initialize CEF.", 1, 1);

	ConsoleFunction(0, "CEF_debug", ts_debug, "(bool enabled) - Turn debugging on or off.", 2, 2);

	ConsoleFunction(0, "CEF_goToURL", ts_goToURL, "(string url) - Visit a URL.", 2, 2);
	ConsoleFunction(0, "CEF_setTexture", ts_setTexture, "(string file) - Set to a texture representing a CEF screen.", 1, 2);
	ConsoleFunction(0, "CEF_getTexture", ts_getTexture, "() - Get the texture currently representing a CEF screen.", 1, 1);
	ConsoleFunction(0, "CEF_bindTexture", ts_bindTexture, "() - Bind to the texture representing a CEF screen.", 1, 1);
	ConsoleFunction(0, "CEF_setResolution", ts_setResolution, "(int width, int height) - Resize the CEF window, reallocating the texture buffer.", 3, 3);

	ConsoleFunction(0, "CEF_execJS", ts_execJS, "(string text) - Execute a line of JavaScript.", 2, 2);

	ConsoleFunction(0, "CEF_mouseMove", ts_mouseMove, "(int x, int y) - Move the mouse to a new position.", 3, 3);
	ConsoleFunction(0, "CEF_mouseClick", ts_mouseClick, "(int x, int y, int clickType) - Send a click event on the specified co-ordinates.", 4, 4);
	ConsoleFunction(0, "CEF_mouseWheel", ts_mouseWheel, "(int x, int y, int deltaX, int deltaY) - Send a mousewheel event at the specified co-ordinates.", 5, 5);
	ConsoleFunction(0, "CEF_keyboardEvent", ts_keyboardEvent, "(char key, int modifiers) - Send a keyboard event.", 3, 3);

	SetGlobalVariable("CEFHOOK::MIN", CEFHOOK_VERS_MIN);
	SetGlobalVariable("CEFHOOK::MAJ", CEFHOOK_VERS_MAJ);
	SetGlobalVariable("CEFHOOK::REV", CEFHOOK_VERS_REV);

	Eval("function clientCmdCEF_Version(){commandToServer('CEF_Version', $CEFHOOK::MAJ, $CEFHOOK::MIN, $CEFHOOK::REV);}");

	Eval("function clientCmdCEF_goToURL(%a){CEF_goToURL(%a);}");
	Eval("function clientCmdCEF_mouseMove(%a,%b){CEF_mouseMove(%a,%b);}");
	Eval("function clientCmdCEF_mouseClick(%a,%b,%c){CEF_mouseClick(%a,%b,%c);}");
	Eval("function clientCmdCEF_mouseWheel(%a,%b,%c,%d){CEF_mouseWheel(%a,%b,%c,%d);}");

	const char* cefPackage =
		"package CEFPackage {"
		"function clientCmdMissionStartPhase3(%a,%b,%c) {"
		"Parent::clientCmdMissionStartPhase3(%a,%b,%c);CEF_bindTexture();"
		"}"
		"function connectToServer(%a,%b,%c,%d) {"
		"CEF_goToURL(\"about:blank\");Parent::connectToServer(%a,%b,%c,%d);"
		"}"
		"function disconnect(%a) {"
		"CEF_goToURL(\"about:blank\");Parent::disconnect(%a);"
		"}"
		"function disconnectedCleanup(%a) {"
		"CEF_goToURL(\"about:blank\");Parent::disconnectedCleanup(%a);"
		"}"
		"function onExit() {"
		"CEF_detach();Parent::onExit();"
		"}"
		"function optionsDlg::applyGraphics(%a) {"
		"parent::applyGraphics(%a);schedule(500,ServerConnection,CEF_bindTexture);"
		"}"
		"};";

	Eval(cefPackage);

	initGL();

	*hasInit = true;

	Printf("Pre-initialization of CEF complete.");

	Eval("activatePackage(\"CEFPackage\");");

	return attach();
}

// Initialize Blockland CEF.
bool attach() {
	if (*isRunning)
		return false;

	if (!torque_init())
		return false;

	if (!*hasInit)
		return init();

	Printf("Initialization of CEF has started...");

	swapBuffers_detour = new MologieDetours::Detour<swapBuffersFn>((swapBuffersFn)0x4237D0, (swapBuffersFn)swapBuffers_hook);

	cefStopEvent = CreateEvent(0, true, false, 0);
	cefThread = CreateThread(0, 0, mainLoop, (LPVOID)cefStopEvent, 0, 0);

	return true;
}

// De-initialize Blockland CEF.
bool detach() {
	if (!*isRunning)
		return false;

	Printf("De-initialization of CEF in progress...");

	SetEvent(cefStopEvent);

	WaitForSingleObject(cefThread, INFINITE);

	ResetEvent(cefStopEvent);

	CloseHandle(cefThread);
	CloseHandle(cefStopEvent);

	if (swapBuffers_detour != nullptr) {
		delete swapBuffers_detour;
		swapBuffers_detour = nullptr;
	}

	free(texBuffer);

	debug = false;

	Printf("De-initialization of CEF complete, CEF is no longer running.");

	return true;
}

// Entry point for Blockland Loader.
int __stdcall DllMain(HINSTANCE instance, unsigned long reason, void* reserved) {
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			return attach();
		case DLL_PROCESS_DETACH:
			// Unable to detach Blockland CEF without crashing the game and/or leaving subprocesses open on exit due to a lack of functionality from Blockland Loader.
			// DLL detaching is therefore handled by Blockland's onExit() function in the "CEFPackage" package.
			return true;
		default:
			return true;
	}
}