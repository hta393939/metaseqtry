
#pragma once

#include "MQWidget.h"

class WidgetTestPlugin;


class MainWindow : public MQWindow
{
public:
	MQTab *m_Tab;
	MQFrame *m_Frame[12];
	MQLabel *m_BottomLabel;

	MQButton *m_Buttons[9];
	MQButton *m_ToggleButton;
	MQButton *m_RepeatButton;
	MQButton *m_MenuButton;

	MQCheckBox *m_Check;
	MQRadioButton *m_Radio[3];

	MQComboBox *m_Combo;
	MQButton *m_ComboAdd;
	MQButton *m_ComboDelete;
	MQButton *m_ComboClear;

	MQListBox *m_List;
	MQButton *m_ListAdd;
	MQButton *m_ListDelete;
	MQButton *m_ListClear;

	MQCheckListBox *m_CheckList;

	MQTreeListBox *m_TreeList;

	MQCheckBox *m_ReadOnlyCheck;
	MQEdit *m_ReadOnlyEdit;
	MQCheckBox *m_PasswordCheck;
	MQEdit *m_PasswordEdit;
	MQMemo *m_Memo;
	MQButton *m_MemoAddButton;
	MQButton *m_MemoClearButton;

	MQSpinBox *m_VisibleColumnSpin;
	MQSpinBox *m_DigitSpin;
	MQDoubleSpinBox *m_DigitValueSpin;

	/// <summary>
	/// スライダーその1
	/// </summary>
	MQSlider *m_Slider;

	MQScrollBar *m_ScrollBar;
	MQSpinBox *m_ScrollMinSpin;
	MQSpinBox *m_ScrollMaxSpin;
	MQSpinBox *m_ScrollPageSpin;
	MQSpinBox *m_ScrollIncSpin;

	MQColorPanel *m_ColorPanel;

	MQPaintBox *m_PaintFrame;

	WidgetTestPlugin *m_pPlugin;

	MainWindow(WidgetTestPlugin *plugin, MQWindowBase& parent);

	/// <summary>
	/// ボーンを変更する
	/// </summary>
	/// <returns></returns>
	int changeBone(MQDocument doc);

private:
	MQFrame *CreateButtonFrame(MQWidgetBase *parent);
	MQFrame *CreateCheckFrame(MQWidgetBase *parent);
	MQFrame *CreateComboFrame(MQWidgetBase *parent);
	MQFrame *CreateListFrame(MQWidgetBase *parent);
	MQFrame *CreateCheckListFrame(MQWidgetBase *parent);
	MQFrame *CreateTreeListFrame(MQWidgetBase *parent);
	MQFrame *CreateLabelFrame(MQWidgetBase *parent);
	MQFrame *CreateEditFrame(MQWidgetBase *parent);
	MQFrame *CreateSpinBoxFrame(MQWidgetBase *parent);
	MQFrame *CreateSliderFrame(MQWidgetBase *parent);
	MQFrame *CreateColorFrame(MQWidgetBase *parent);
	MQFrame *CreatePaintFrame(MQWidgetBase *parent);

	BOOL OnHide(MQWidgetBase *sender, MQDocument doc);
	BOOL TabChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL ButtonClick(MQWidgetBase *sender, MQDocument doc);
	BOOL ToggleButtonClick(MQWidgetBase *sender, MQDocument doc);
	BOOL RepeatButtonClick(MQWidgetBase *sender, MQDocument doc);
	BOOL RepeatButtonRepeat(MQWidgetBase *sender, MQDocument doc);
	BOOL MenuButtonClick(MQWidgetBase *sender, MQDocument doc);
	BOOL MenuClick(MQWidgetBase *sender, MQDocument doc);
	BOOL OpenFileDialog(MQWidgetBase *sender, MQDocument doc);
	BOOL SaveFileDialog(MQWidgetBase *sender, MQDocument doc);
	BOOL CheckChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL RadioChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL ComboChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL ComboAddClick(MQWidgetBase *sender, MQDocument doc);
	BOOL ComboDeleteClick(MQWidgetBase *sender, MQDocument doc);
	BOOL ComboClearClick(MQWidgetBase *sender, MQDocument doc);
	BOOL ListChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL ListDrawItem(MQWidgetBase *sender, MQDocument doc, MQListBoxDrawItemParam& param);
	BOOL ListAddClick(MQWidgetBase *sender, MQDocument doc);
	BOOL ListDeleteClick(MQWidgetBase *sender, MQDocument doc);
	BOOL ListClearClick(MQWidgetBase *sender, MQDocument doc);
	BOOL EditChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL EditChanging(MQWidgetBase *sender, MQDocument doc);
	BOOL ReadOnlyCheckChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL PasswordCheckChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL MemoChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL MemoChanging(MQWidgetBase *sender, MQDocument doc);
	BOOL MemoAddClick(MQWidgetBase *sender, MQDocument doc);
	BOOL MemoClearClick(MQWidgetBase *sender, MQDocument doc);
	BOOL SpinBoxChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL SpinBoxChanging(MQWidgetBase *sender, MQDocument doc);
	BOOL VisibleColumnSpinChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL DigitSpinChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL DigitValueChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL DSpinBoxChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL DSpinBoxChanging(MQWidgetBase *sender, MQDocument doc);
	BOOL SliderChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL SliderChanging(MQWidgetBase *sender, MQDocument doc);
	BOOL ScrollBarChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL ScrollBarChanging(MQWidgetBase *sender, MQDocument doc);
	BOOL ScrollSpinChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL ColorPanelChanged(MQWidgetBase *sender, MQDocument doc);
	BOOL ColorPanelChanging(MQWidgetBase *sender, MQDocument doc);
	BOOL PaintFramePaint(MQWidgetBase *sender, MQDocument doc, MQWidgetPaintParam& param);
	BOOL PaintFrameMouseMove(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);

};

