#ifndef _WIDGET_TEST_H_
#define _WIDGET_TEST_H_

#include "MQBasePlugin.h"
#include "mainwin.h"


class WidgetTestPlugin : public MQStationPlugin
{
	friend class MainWindow;

public:
	WidgetTestPlugin();
	virtual ~WidgetTestPlugin();

	// プラグインIDを返す。
	virtual void GetPlugInID(DWORD *Product, DWORD *ID);
	// プラグイン名を返す。
	virtual const char *GetPlugInName(void);
	// ポップアップメニューに表示される文字列を返す。
	virtual const wchar_t *EnumString(void);

	// アプリケーションの初期化
	virtual BOOL Initialize();
	// アプリケーションの終了
	virtual void Exit();

	// 表示・非表示切り替え要求
	virtual BOOL Activate(MQDocument doc, BOOL flag);
	// 表示・非表示状態の返答
	virtual BOOL IsActivated(MQDocument doc);


	typedef bool (WidgetTestPlugin::*ExecuteCallbackProc)(MQDocument doc);

	void Execute(ExecuteCallbackProc proc);

protected:
	struct CallbackInfo {
		ExecuteCallbackProc proc;
	};

	// コールバックに対する実装部
	virtual bool ExecuteCallback(MQDocument doc, void *option);

private:
	MainWindow *m_Window;

	bool m_bActivate;
};

#endif _WIDGET_TEST_H_