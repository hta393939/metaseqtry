
#include "stdafx.h"
#include "stationtry.h"

#define MY_PRODUCT (0xB2B2501D)
#define MY_ID (0x0ED64D3F)
#define MY_PLUGINNAME "Station Try Copyright(C) 2024, hta393939"

static WidgetTestPlugin plugin;


// Constructor
// コンストラクタ
WidgetTestPlugin::WidgetTestPlugin()
{
	m_Window = NULL;
	m_bActivate = false;
}

// Destructor
// デストラクタ
WidgetTestPlugin::~WidgetTestPlugin()
{
	if(m_Window != NULL){
		// Also deletes the window here just to make sure though it has been 
		// deleted in Exit().
		// Exit()で破棄されるはずだが一応念のためここでも
		delete m_Window;
		m_Window = NULL;
	}
}

//---------------------------------------------------------------------------
//  GetPlugInID
//    プラグインIDを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
void WidgetTestPlugin::GetPlugInID(DWORD *Product, DWORD *ID)
{
	// プロダクト名(制作者名)とIDを、全部で64bitの値として返す
	// 値は他と重複しないようなランダムなもので良い
	*Product = MY_PRODUCT;
	*ID      = MY_ID;
}

//---------------------------------------------------------------------------
//  GetPlugInName
//    プラグイン名を返す。
//    この関数は[プラグインについて]表示時に呼び出される。
//---------------------------------------------------------------------------
const char *WidgetTestPlugin::GetPlugInName(void)
{
	return MY_PLUGINNAME;
}

//---------------------------------------------------------------------------
//  EnumString
//    ポップアップメニューに表示される文字列を返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
const wchar_t *WidgetTestPlugin::EnumString(void)
{
	return L"Station Try Widget Test panel";
}


//---------------------------------------------------------------------------
//  Initialize
//    アプリケーションの初期化
//---------------------------------------------------------------------------
BOOL WidgetTestPlugin::Initialize()
{
	if (m_Window == NULL){
		// Create a new window that's parent is the main window.
		// メインウインドウを親にした新しいウィンドウの作成
		MQWindowBase mainwnd = MQWindow::GetMainWindow();
		m_Window = new MainWindow(this, mainwnd);
	}
	return TRUE;
}

//---------------------------------------------------------------------------
//  Exit
//    アプリケーションの終了
//---------------------------------------------------------------------------
void WidgetTestPlugin::Exit()
{
	if (m_Window != NULL){
		delete m_Window;
		m_Window = NULL;
	}
}

//---------------------------------------------------------------------------
//  Activate
//    表示・非表示切り替え要求
//---------------------------------------------------------------------------
BOOL WidgetTestPlugin::Activate(MQDocument doc, BOOL flag)
{
	if (m_Window == NULL)
		return FALSE;

	// Switch showing and hiding the window.
	// ウインドウの表示・非表示切り替え
	m_bActivate = flag ? true : false;
	m_Window->SetVisible(m_bActivate);

	return m_bActivate;
}

//---------------------------------------------------------------------------
//  IsActivated
//    表示・非表示状態の返答
//---------------------------------------------------------------------------
BOOL WidgetTestPlugin::IsActivated(MQDocument doc)
{
	// Return TRUE if a window has been not created yet.
	// ウインドウがまだ生成されていないならTRUE
	if (m_Window == NULL)
		return FALSE;

	return m_bActivate;
}

//---------------------------------------------------------------------------
//  ExecuteCallback
//    コールバックに対する実装部
//---------------------------------------------------------------------------
bool WidgetTestPlugin::ExecuteCallback(MQDocument doc, void *option)
{
	CallbackInfo *info = (CallbackInfo*)option;
	return ((*this).*info->proc)(doc);
}

// コールバックの呼び出し
void WidgetTestPlugin::Execute(ExecuteCallbackProc proc)
{
	CallbackInfo info;
	info.proc = proc;
	BeginCallback(&info);
}

// プラグインのベースクラスを返す
MQBasePlugin *GetPluginClass()
{
	return &plugin;
}

