
#include "stdafx.h"
#include "mainwin.h"
#include "stationtry.h"

#include "MQBoneManager.h"


MainWindow::MainWindow(WidgetTestPlugin *plugin, MQWindowBase& parent) : MQWindow(parent)
{
	m_pPlugin = plugin;

	SetTitle(L"Station Try Widget Test");
	SetOutSpace(0.4);

	m_Tab = CreateTab(this);
	m_Tab->AddChangedEvent(this, &MainWindow::TabChanged);

	m_Frame[0] = CreateButtonFrame(m_Tab);
	m_Tab->SetTabTitle(0, L"Button");

	m_Frame[1] = CreateCheckFrame(m_Tab);
	m_Tab->SetTabTitle(1, L"Check");

	m_Frame[2] = CreateComboFrame(m_Tab);
	m_Tab->SetTabTitle(2, L"Combo");

	m_Frame[3] = CreateListFrame(m_Tab);
	m_Tab->SetTabTitle(3, L"List");

	m_Frame[4] = CreateCheckListFrame(m_Tab);
	m_Tab->SetTabTitle(4, L"CheckList");

	m_Frame[5] = CreateTreeListFrame(m_Tab);
	m_Tab->SetTabTitle(5, L"TreeList");

	m_Frame[6] = CreateLabelFrame(m_Tab);
	m_Tab->SetTabTitle(6, L"Label");

	m_Frame[7] = CreateEditFrame(m_Tab);
	m_Tab->SetTabTitle(7, L"Edit");

	m_Frame[8] = CreateSpinBoxFrame(m_Tab);
	m_Tab->SetTabTitle(8, L"SpinBox");

	m_Frame[9] = CreateSliderFrame(m_Tab);
	m_Tab->SetTabTitle(9, L"Slider/ScrollBar");

	m_Frame[10] = CreateColorFrame(m_Tab);
	m_Tab->SetTabTitle(10, L"Color");

	m_Frame[11] = CreatePaintFrame(m_Tab);
	m_Tab->SetTabTitle(11, L"Paint");

	m_BottomLabel = CreateLabel(this);
	m_BottomLabel->SetHintSizeRateY(4);
	m_BottomLabel->SetFrame(true);

	this->AddHideEvent(this, &MainWindow::OnHide);
}

MQFrame *MainWindow::CreateButtonFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);

	MQFrame *mtx_frame = CreateHorizontalFrame(root);
	mtx_frame->SetMatrixColumn(3);
	mtx_frame->SetInSpace(0);
	for(int i=0; i<9; i++){
		wchar_t name[16];
		swprintf_s(name, L"Button %d", i+1);

		m_Buttons[i] = CreateButton(mtx_frame);
		m_Buttons[i]->SetText(name);
		//m_Buttons[i]->SetGroup(true);
		m_Buttons[i]->AddClickEvent(this, &MainWindow::ButtonClick);
	}

	m_ToggleButton = CreateButton(root, L"Toggle");
	m_ToggleButton->SetToggle(true);
	m_ToggleButton->SetFontScale(1.5);
	m_ToggleButton->AddClickEvent(this, &MainWindow::ToggleButtonClick);

	m_RepeatButton = CreateButton(root, L"Repeat");
	m_RepeatButton->SetRepeat(true);
	m_RepeatButton->SetFontColor(255, 0, 64, 160);
	m_RepeatButton->AddClickEvent(this, &MainWindow::RepeatButtonClick);
	m_RepeatButton->AddRepeatEvent(this, &MainWindow::RepeatButtonRepeat);

	m_MenuButton = CreateButton(root, L"Menu");
	m_MenuButton->AddClickEvent(this, &MainWindow::MenuButtonClick);

	MQButton *btn = CreateButton(root, L"OpenFileDialog");
	btn->SetFontName(L"Courier New");
	btn->AddClickEvent(this, &MainWindow::OpenFileDialog);

	btn = CreateButton(root, L"SaveFileDialog");
	btn->SetFontBold(true);
	btn->AddClickEvent(this, &MainWindow::SaveFileDialog);

	return root;
}

MQFrame *MainWindow::CreateCheckFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);

	m_Check = CreateCheckBox(root);
	m_Check->SetText(L"Check1");
	m_Check->AddChangedEvent(this, &MainWindow::CheckChanged);

	for(int i=0; i<3; i++){
		wchar_t name[16];
		swprintf_s(name, L"Radio %d", i+1);

		m_Radio[i] = CreateRadioButton(root);
		m_Radio[i]->SetText(name);
		m_Radio[i]->AddChangedEvent(this, &MainWindow::RadioChanged);
	}
	m_Radio[0]->SetChecked(true);

	return root;
}

MQFrame *MainWindow::CreateComboFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);

	m_Combo = CreateComboBox(root);
	m_Combo->AddItem(L"Default1", 1);
	m_Combo->AddItem(L"Default2", 2);
	m_Combo->AddItem(L"Default3", 3);
	m_Combo->AddChangedEvent(this, &MainWindow::ComboChanged);

	MQFrame *frame = CreateHorizontalFrame(root);
	frame->SetUniformSize(true);
	m_ComboAdd = CreateButton(frame, L"Add");
	m_ComboAdd->AddClickEvent(this, &MainWindow::ComboAddClick);
	m_ComboDelete = CreateButton(frame, L"Delete");
	m_ComboDelete->AddClickEvent(this, &MainWindow::ComboDeleteClick);
	m_ComboClear = CreateButton(frame, L"Clear");
	m_ComboClear->AddClickEvent(this, &MainWindow::ComboClearClick);

	return root;
}

MQFrame *MainWindow::CreateListFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);

	m_List = CreateListBox(root);
	m_List->AddItem(L"Default1", 1);
	m_List->AddItem(L"Default2", 2);
	m_List->AddItem(L"Default3", 3);
	m_List->AddChangedEvent(this, &MainWindow::ListChanged);
	m_List->AddDrawItemEvent(this, &MainWindow::ListDrawItem);

	MQFrame *frame = CreateHorizontalFrame(root);
	frame->SetUniformSize(true);
	m_ListAdd = CreateButton(frame, L"Add");
	m_ListAdd->AddClickEvent(this, &MainWindow::ListAddClick);
	m_ListDelete = CreateButton(frame, L"Delete");
	m_ListDelete->AddClickEvent(this, &MainWindow::ListDeleteClick);
	m_ListClear = CreateButton(frame, L"Clear");
	m_ListClear->AddClickEvent(this, &MainWindow::ListClearClick);

	return root;
}

MQFrame *MainWindow::CreateCheckListFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);

	m_CheckList = CreateCheckListBox(root);
	m_CheckList->AddItem(L"Default1", 1);
	m_CheckList->AddItem(L"Default2", 2);
	m_CheckList->AddItem(L"Default3", 3);
	m_CheckList->SetItemChecked(1, true);

	return root;
}

MQFrame *MainWindow::CreateTreeListFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);

	m_TreeList = CreateTreeListBox(root);
	int id1 = m_TreeList->AddItem(L"Item1", 1);
	int id2 = m_TreeList->AddItem(id1, L"Item2", 2);
	int id3 = m_TreeList->AddItem(id2, L"Item3", 3);
	int id4 = m_TreeList->AddItem(id1, L"Item4", 4);
	int id5 = m_TreeList->AddItem(id4, L"Item5", 5);
	m_TreeList->SetItemCollapsed(id2, true);

	return root;
}

MQFrame *MainWindow::CreateLabelFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);

	MQLabel *label;
	label = CreateLabel(root, L"Left");
	label->SetAlignment(MQLabel::ALIGN_LEFT);

	label = CreateLabel(root, L"Center");
	label->SetAlignment(MQLabel::ALIGN_CENTER);

	label = CreateLabel(root, L"Right");
	label->SetAlignment(MQLabel::ALIGN_RIGHT);

	label = CreateLabel(root, L"Frame");
	label->SetFrame(true);

	label = CreateLabel(root, L"Vertical");
	label->SetVerticalText(true);

	label = CreateLabel(root);
	label->SetText(L"WordWrap WordWrap WordWrap WordWrap WordWrap WordWrap WordWrap WordWrap WordWrap WordWrap");
	label->SetWordWrap(true);
	label->SetHintSizeRateX(12);
	label->SetHintSizeRateY(4);

	return root;
}

MQFrame *MainWindow::CreateEditFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);

	MQEdit *edit;
	MQFrame *frame;
	edit = CreateEdit(root, L"Normal");
	edit->AddChangedEvent(this, &MainWindow::EditChanged);
	edit->AddChangingEvent(this, &MainWindow::EditChanging);

	frame = CreateHorizontalFrame(root);
	m_ReadOnlyCheck = CreateCheckBox(frame, L"ReadOnly");
	m_ReadOnlyCheck->SetChecked(true);
	m_ReadOnlyCheck->AddChangedEvent(this, &MainWindow::ReadOnlyCheckChanged);
	m_ReadOnlyEdit = CreateEdit(frame);
	m_ReadOnlyEdit->SetText(L"ReadOnly");
	m_ReadOnlyEdit->SetReadOnly(true);
	m_ReadOnlyEdit->SetHorzLayout(MQWidgetBase::LAYOUT_FILL);
	m_ReadOnlyEdit->AddChangedEvent(this, &MainWindow::EditChanged);
	m_ReadOnlyEdit->AddChangingEvent(this, &MainWindow::EditChanging);

	frame = CreateHorizontalFrame(root);
	m_PasswordCheck = CreateCheckBox(frame, L"Password");
	m_PasswordCheck->SetChecked(true);
	m_PasswordCheck->AddChangedEvent(this, &MainWindow::PasswordCheckChanged);
	m_PasswordEdit = CreateEdit(frame);
	m_PasswordEdit->SetText(L"Password");
	m_PasswordEdit->SetPassword(true);
	m_PasswordEdit->SetHorzLayout(MQWidgetBase::LAYOUT_FILL);
	m_PasswordEdit->AddChangedEvent(this, &MainWindow::EditChanged);
	m_PasswordEdit->AddChangingEvent(this, &MainWindow::EditChanging);

	frame = CreateHorizontalFrame(root);
	CreateLabel(frame, L"Int");
	edit = CreateEdit(frame);
	edit->SetText(L"0");
	edit->SetAlignment(MQEdit::ALIGN_LEFT);
	edit->SetNumeric(MQEdit::NUMERIC_INT);
	edit->AddChangedEvent(this, &MainWindow::EditChanged);
	edit->AddChangingEvent(this, &MainWindow::EditChanging);

	CreateLabel(frame, L"Double");
	edit = CreateEdit(frame);
	edit->SetText(L"0.0");
	edit->SetNumeric(MQEdit::NUMERIC_DOUBLE);
	edit->SetAlignment(MQEdit::ALIGN_RIGHT);
	edit->AddChangedEvent(this, &MainWindow::EditChanged);
	edit->AddChangingEvent(this, &MainWindow::EditChanging);

	frame = CreateHorizontalFrame(root);
	edit = CreateEdit(frame, L"Column 8");
	edit->SetVisibleColumn(8);
	edit = CreateEdit(frame, L"Column 16");
	edit->SetVisibleColumn(16);

	frame = CreateHorizontalFrame(root);
	m_Memo = CreateMemo(frame);
	m_Memo->SetHintSizeRateY(6);
	m_Memo->SetHorzLayout(MQWidgetBase::LAYOUT_FILL);
	m_Memo->AddChangedEvent(this, &MainWindow::MemoChanged);
	m_Memo->AddChangingEvent(this, &MainWindow::MemoChanging);
	frame = CreateVerticalFrame(frame);
	m_MemoAddButton = CreateButton(frame, L"Add");
	m_MemoAddButton->AddClickEvent(this, &MainWindow::MemoAddClick);
	m_MemoClearButton = CreateButton(frame, L"Clear");
	m_MemoClearButton->AddClickEvent(this, &MainWindow::MemoClearClick);

	return root;
}

MQFrame *MainWindow::CreateSpinBoxFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);

	MQSpinBox *spin;
	spin = CreateSpinBox(root);
	spin->SetMin(10);
	spin->SetMax(100);
	spin->SetPosition(50);
	spin->SetIncrement(5);
	spin->AddChangedEvent(this, &MainWindow::SpinBoxChanged);
	spin->AddChangingEvent(this, &MainWindow::SpinBoxChanging);

	MQFrame *frame = CreateHorizontalFrame(root);
	CreateLabel(frame, L"VisibleColumn");
	m_VisibleColumnSpin = CreateSpinBox(frame);
	m_VisibleColumnSpin->SetPosition(8);
	m_VisibleColumnSpin->SetVisibleColumn(8);
	m_VisibleColumnSpin->AddChangedEvent(this, &MainWindow::VisibleColumnSpinChanged);
	m_VisibleColumnSpin->AddChangingEvent(this, &MainWindow::VisibleColumnSpinChanged);

	int r,g,b,a;
	MQWidgetBase::GetDefaultFrameColor(r,g,b,a);
	frame = CreateHorizontalFrame(root);
	frame->SetBackColor(r,g,b,a);
	frame->SetHintSizeRateY(0.05);

	frame = CreateHorizontalFrame(root);
	CreateLabel(frame, L"DecimalDigit");
	CreateLabel(frame, L"Digit");
	m_DigitSpin = CreateSpinBox(frame);
	m_DigitSpin->SetMin(0);
	m_DigitSpin->SetMax(20);
	m_DigitSpin->SetPosition(3);
	m_DigitSpin->AddChangedEvent(this, &MainWindow::DigitSpinChanged);
	m_DigitSpin->AddChangingEvent(this, &MainWindow::DigitSpinChanged);
	CreateLabel(frame, L"Value");
	m_DigitValueSpin = CreateDoubleSpinBox(frame);
	m_DigitValueSpin->SetMin(0.5);
	m_DigitValueSpin->SetMax(50);
	m_DigitValueSpin->SetPosition(0.12345678);
	m_DigitValueSpin->SetDecimalDigit(3);
	m_DigitValueSpin->SetIncrement(0.5);
	m_DigitValueSpin->AddChangedEvent(this, &MainWindow::DigitValueChanged);

	MQDoubleSpinBox *dspin;
	frame = CreateHorizontalFrame(root);
	CreateLabel(frame, L"Exponential");
	dspin = CreateDoubleSpinBox(frame);
	dspin->SetExponential(true);
	dspin->SetMin(0.0001);
	dspin->SetMax(10000);
	dspin->SetPosition(1);
	dspin->SetIncrement(1.1);
	dspin->AddChangedEvent(this, &MainWindow::DSpinBoxChanged);
	dspin->AddChangingEvent(this, &MainWindow::DSpinBoxChanging);

	frame = CreateHorizontalFrame(root);
	dspin = CreateDoubleSpinBox(frame);
	dspin->SetVisibleColumn(8);
	dspin = CreateDoubleSpinBox(frame);
	dspin->SetVisibleColumn(16);

	return root;
}

MQFrame *MainWindow::CreateSliderFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);

	m_Slider = CreateSlider(root);
	m_Slider->SetMin(10);
	m_Slider->SetMax(100);
	m_Slider->SetPosition(50);
	// NOTE: ここで登録している
	m_Slider->AddChangedEvent(this, &MainWindow::SliderChanged);
	m_Slider->AddChangingEvent(this, &MainWindow::SliderChanging);

	MQFrame *frame;
	MQLabel *label;
	int r,g,b,a;
	MQWidgetBase::GetDefaultFrameColor(r,g,b,a);
	frame = CreateHorizontalFrame(root);
	frame->SetBackColor(r,g,b,a);
	frame->SetHintSizeRateY(0.05);

	m_ScrollBar = CreateScrollBar(root);
	m_ScrollBar->SetMin(10);
	m_ScrollBar->SetMax(100);
	m_ScrollBar->SetPosition(50);
	m_ScrollBar->AddChangedEvent(this, &MainWindow::ScrollBarChanged);
	m_ScrollBar->AddChangingEvent(this, &MainWindow::ScrollBarChanging);

	frame = CreateHorizontalFrame(root);
	CreateLabel(frame, L"Min");
	m_ScrollMinSpin = CreateSpinBox(frame);
	m_ScrollMinSpin->SetMin(-1000);
	m_ScrollMinSpin->SetMax(1000);
	m_ScrollMinSpin->SetPosition(m_ScrollBar->GetMin());
	m_ScrollMinSpin->AddChangedEvent(this, &MainWindow::ScrollSpinChanged);
	m_ScrollMinSpin->AddChangingEvent(this, &MainWindow::ScrollSpinChanged);
	label = CreateLabel(frame, L"Max");
	label->SetFillBeforeRate(1.0);
	m_ScrollMaxSpin = CreateSpinBox(frame);
	m_ScrollMaxSpin->SetMin(-1000);
	m_ScrollMaxSpin->SetMax(1000);
	m_ScrollMaxSpin->SetPosition(m_ScrollBar->GetMax());
	m_ScrollMaxSpin->AddChangedEvent(this, &MainWindow::ScrollSpinChanged);
	m_ScrollMaxSpin->AddChangingEvent(this, &MainWindow::ScrollSpinChanged);
	frame = CreateHorizontalFrame(root);
	CreateLabel(frame, L"Page");
	m_ScrollPageSpin = CreateSpinBox(frame);
	m_ScrollPageSpin->SetMin(1);
	m_ScrollPageSpin->SetMax(1000);
	m_ScrollPageSpin->SetPosition(m_ScrollBar->GetPage());
	m_ScrollPageSpin->AddChangedEvent(this, &MainWindow::ScrollSpinChanged);
	m_ScrollPageSpin->AddChangingEvent(this, &MainWindow::ScrollSpinChanged);
	CreateLabel(frame, L"Increment");
	m_ScrollIncSpin = CreateSpinBox(frame);
	m_ScrollIncSpin->SetMin(1);
	m_ScrollIncSpin->SetMax(1000);
	m_ScrollIncSpin->SetPosition(m_ScrollBar->GetIncrement());
	m_ScrollIncSpin->AddChangedEvent(this, &MainWindow::ScrollSpinChanged);
	m_ScrollIncSpin->AddChangingEvent(this, &MainWindow::ScrollSpinChanged);

	return root;
}

MQFrame *MainWindow::CreateColorFrame(MQWidgetBase *parent)
{
	/*
	MQFrame *root = CreateVerticalFrame(parent);

	m_ColorPanel = CreateColorPanel(root);
	m_ColorPanel->SetColor(0, 128, 255);
	m_ColorPanel->AddChangedEvent(this, &MainWindow::ColorPanelChanged);
	m_ColorPanel->AddChangingEvent(this, &MainWindow::ColorPanelChanging);
	*/
	MQScrollBox *box = CreateScrollBox(parent);
	box->SetAutoWidgetScroll(TRUE);
	MQFrame *root = CreateVerticalFrame(box);
	root->SetHorzLayout(MQWidgetBase::LAYOUT_FILL);

	m_ColorPanel = CreateColorPanel(root);
	m_ColorPanel->SetColor(0, 128, 255);
	m_ColorPanel->AddChangedEvent(this, &MainWindow::ColorPanelChanged);
	m_ColorPanel->AddChangingEvent(this, &MainWindow::ColorPanelChanging);
	m_ColorPanel->SetHorzLayout(MQWidgetBase::LAYOUT_FILL);
	
	CreateButton(root, L"test");
	return root;
}

MQFrame *MainWindow::CreatePaintFrame(MQWidgetBase *parent)
{
	MQFrame *root = CreateVerticalFrame(parent);
	root->SetVertLayout(MQWidgetBase::LAYOUT_FILL);

	m_PaintFrame = CreatePaintBox(root);
	m_PaintFrame->SetVertLayout(MQWidgetBase::LAYOUT_FILL);
	m_PaintFrame->AddPaintEvent(this, &MainWindow::PaintFramePaint);
	m_PaintFrame->AddMouseMoveEvent(this, &MainWindow::PaintFrameMouseMove);

	return root;
}


BOOL MainWindow::OnHide(MQWidgetBase *sender, MQDocument doc)
{
	if(m_pPlugin->m_bActivate){
		m_pPlugin->m_bActivate = false;
		m_pPlugin->WindowClose();
	}
	return FALSE;
}

BOOL MainWindow::TabChanged(MQWidgetBase *sender, MQDocument doc)
{
	wchar_t text[64];
	swprintf_s(text, L"Tab changed %d", m_Tab->GetCurrentPage());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::ButtonClick(MQWidgetBase *sender, MQDocument doc)
{
	for(int i=0; i<9; i++){
		m_Buttons[i]->SetDown(m_Buttons[i] == sender);
	}
	wchar_t text[64];
	swprintf_s(text, L"%s clicked", static_cast<MQButton*>(sender)->GetText().c_str());
	m_BottomLabel->SetText(text);
	return TRUE;
}

BOOL MainWindow::ToggleButtonClick(MQWidgetBase *sender, MQDocument doc)
{
	static int count = 0;
	wchar_t text[64];
	swprintf_s(text, L"Toggle button clicked %s %d", m_ToggleButton->GetDown() ? L"down" : L"up", ++count);
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::RepeatButtonClick(MQWidgetBase *sender, MQDocument doc)
{
	m_BottomLabel->SetText(L"Repeat button clicked");
	return FALSE;
}

BOOL MainWindow::RepeatButtonRepeat(MQWidgetBase *sender, MQDocument doc)
{
	static int count = 0;
	wchar_t text[64];
	swprintf_s(text, L"Repeat button repeat %d", ++count);
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::MenuButtonClick(MQWidgetBase *sender, MQDocument doc)
{
	MQPopup popup(*this);
	MQMenuItem *menu1 = popup.CreateMenuItem(L"Menu1");
	menu1->AddClickEvent(this, &MainWindow::MenuClick);
	MQMenuItem *menu2 = popup.CreateMenuItem(L"Menu2");
	menu2->SetChecked(true);
	menu2->AddClickEvent(this, &MainWindow::MenuClick);
	MQMenuItem *menu3 = popup.CreateMenuItem(L"");
	menu3->SetSeparator(true);
	MQMenuItem *menu4 = popup.CreateMenuItem(L"MenuToggle");
	menu4->SetToggle(true);
	menu4->SetClickClose(false);
	MQMenuItem *menu5 = popup.CreateMenuItem(L"CascadeMenu");
	MQMenuItem *menu6 = popup.CreateSubMenuItem(menu5, L"SubMenu1");
	menu6->AddClickEvent(this, &MainWindow::MenuClick);
	MQMenuItem *menu7 = popup.CreateSubMenuItem(menu5, L"SubMenu2");
	menu7->AddClickEvent(this, &MainWindow::MenuClick);
	int x,y,w,h;
	popup.GetPreferredSidePosition(x,y,w,h,m_MenuButton,false);
	popup.ShowPopup(x,y);
	return FALSE;
}

BOOL MainWindow::MenuClick(MQWidgetBase *sender, MQDocument doc)
{
	MQMenuItem *menu = dynamic_cast<MQMenuItem*>(sender);
	if(menu != NULL){
		MQDialog::MessageInformationBox(*this, menu->GetText(), L"Menu clicked");
	}
	return FALSE;
}

BOOL MainWindow::OpenFileDialog(MQWidgetBase *sender, MQDocument doc)
{
	MQOpenFileDialog dlg(*this);
	dlg.AddFilter(L"Text file (*.txt)|*.txt");
	dlg.AddFilter(L"Bitmap file (*.bmp)|*.bmp");
	dlg.AddFilter(L"All files (*.*)|*.*");
	if(dlg.Execute()){
		std::wstring fn = L"File name: " + dlg.GetFileName();
		m_BottomLabel->SetText(fn);
	}
	return FALSE;
}

BOOL MainWindow::SaveFileDialog(MQWidgetBase *sender, MQDocument doc)
{
	MQSaveFileDialog dlg(*this);
	dlg.AddFilter(L"Text file (*.txt)|*.txt");
	dlg.AddFilter(L"Bitmap file (*.bmp)|*.bmp");
	dlg.SetDefaultExt(L"txt");
	if(dlg.Execute()){
		std::wstring fn = L"File name: " + dlg.GetFileName();
		m_BottomLabel->SetText(fn);
	}
	return FALSE;
}

BOOL MainWindow::CheckChanged(MQWidgetBase *sender, MQDocument doc)
{
	wchar_t text[64];
	swprintf_s(text, L"Check changed %s", m_Check->GetChecked() ? L"1" : L"0");
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::RadioChanged(MQWidgetBase *sender, MQDocument doc)
{
	for(int i=0; i<3; i++){
		m_Radio[i]->SetChecked(m_Radio[i] == sender);
	}
	return FALSE;
}

BOOL MainWindow::ComboChanged(MQWidgetBase *sender, MQDocument doc)
{
	wchar_t text[64];
	swprintf_s(text, L"Combo changed %d", m_Combo->GetCurrentIndex());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::ComboAddClick(MQWidgetBase *sender, MQDocument doc)
{
	static int count = 0;
	wchar_t name[64];
	swprintf_s(name, L"Add %d", ++count);
	int index = m_Combo->AddItem(name);
	m_Combo->SetCurrentIndex(index);
	return FALSE;
}

BOOL MainWindow::ComboDeleteClick(MQWidgetBase *sender, MQDocument doc)
{
	int index = m_Combo->GetCurrentIndex();
	if(index >= 0 && index < m_Combo->GetItemCount()){
		m_Combo->DeleteItem(index);
	}
	return FALSE;
}

BOOL MainWindow::ComboClearClick(MQWidgetBase *sender, MQDocument doc)
{
	m_Combo->ClearItems();
	return FALSE;
}

BOOL MainWindow::ListChanged(MQWidgetBase *sender, MQDocument doc)
{
	wchar_t text[64];
	swprintf_s(text, L"List changed %d", m_List->GetCurrentIndex());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::ListDrawItem(MQWidgetBase *sender, MQDocument doc, MQListBoxDrawItemParam& param)
{
	int pad = (int)ceil(param.Height * 0.2);
	int size = param.Height - pad*2;
	param.Canvas->SetColor(255,0,0,255);
	param.Canvas->FillRect(param.X + pad, param.Y + pad, size, size);
	param.Canvas->SetColor(0,0,0,255);
	param.Canvas->DrawRect(param.X + pad, param.Y + pad, size, size);

	param.X += size + pad*2;
	param.Width -= size + pad*2;
	return FALSE; // Continue to draw a text.
}

BOOL MainWindow::ListAddClick(MQWidgetBase *sender, MQDocument doc)
{
	static int count = 0;
	wchar_t name[64];
	swprintf_s(name, L"Add %d", ++count);
	int index = m_List->AddItem(name);
	m_List->SetCurrentIndex(index);
	return FALSE;
}

BOOL MainWindow::ListDeleteClick(MQWidgetBase *sender, MQDocument doc)
{
	int index = m_List->GetCurrentIndex();
	if(index >= 0 && index < m_List->GetItemCount()){
		m_List->DeleteItem(index);
	}
	return FALSE;
}

BOOL MainWindow::ListClearClick(MQWidgetBase *sender, MQDocument doc)
{
	m_List->ClearItems();
	return FALSE;
}

BOOL MainWindow::EditChanged(MQWidgetBase *sender, MQDocument doc)
{
	MQEdit *edit = static_cast<MQEdit*>(sender);
	std::wstring text = edit->GetText();
	text = L"Edit changed " + text;
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::EditChanging(MQWidgetBase *sender, MQDocument doc)
{
	MQEdit *edit = static_cast<MQEdit*>(sender);
	std::wstring text = edit->GetText();
	text = L"Edit changing " + text;
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::ReadOnlyCheckChanged(MQWidgetBase *sender, MQDocument doc)
{
	m_ReadOnlyEdit->SetReadOnly(m_ReadOnlyCheck->GetChecked());

	wchar_t text[64];
	swprintf_s(text, L"Edit read only %d", m_ReadOnlyEdit->GetReadOnly() ? 1 : 0);
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::PasswordCheckChanged(MQWidgetBase *sender, MQDocument doc)
{
	m_PasswordEdit->SetPassword(m_PasswordCheck->GetChecked());

	wchar_t text[64];
	swprintf_s(text, L"Edit password %d", m_PasswordEdit->GetPassword() ? 1 : 0);
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::MemoChanged(MQWidgetBase *sender, MQDocument doc)
{
	MQMemo *memo = static_cast<MQMemo*>(sender);
	std::wstring text = memo->GetText();
	text = L"Memo changed " + text;
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::MemoChanging(MQWidgetBase *sender, MQDocument doc)
{
	MQMemo *memo = static_cast<MQMemo*>(sender);
	std::wstring text = memo->GetText();
	text = L"Memo changing " + text;
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::MemoAddClick(MQWidgetBase *sender, MQDocument doc)
{
	m_Memo->AddLine(L"AddLine");
	return FALSE;
}

BOOL MainWindow::MemoClearClick(MQWidgetBase *sender, MQDocument doc)
{
	m_Memo->SetText(L"");
	return FALSE;
}

BOOL MainWindow::SpinBoxChanged(MQWidgetBase *sender, MQDocument doc)
{
	MQSpinBox *spin = static_cast<MQSpinBox*>(sender);
	wchar_t text[64];
	swprintf_s(text, L"SpinBox changed %d", spin->GetPosition());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::SpinBoxChanging(MQWidgetBase *sender, MQDocument doc)
{
	MQSpinBox *spin = static_cast<MQSpinBox*>(sender);
	wchar_t text[64];
	swprintf_s(text, L"SpinBox changeing %d", spin->GetPosition());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::VisibleColumnSpinChanged(MQWidgetBase *sender, MQDocument doc)
{
	m_VisibleColumnSpin->SetVisibleColumn(m_VisibleColumnSpin->GetPosition());

	wchar_t text[64];
	swprintf_s(text, L"SpinBox visible column %d", m_VisibleColumnSpin->GetPosition());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::DigitSpinChanged(MQWidgetBase *sender, MQDocument doc)
{
	m_DigitValueSpin->SetDecimalDigit(m_DigitSpin->GetPosition());

	wchar_t text[64];
	swprintf_s(text, L"DoubleSpinBox decimal digit %d", m_DigitSpin->GetPosition());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::DigitValueChanged(MQWidgetBase *sender, MQDocument doc)
{
	m_DigitSpin->SetPosition(m_DigitValueSpin->GetDecimalDigit());

	wchar_t text[64];
	swprintf_s(text, L"DoubleSpinBox decimal digit %d", m_DigitValueSpin->GetDecimalDigit());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::DSpinBoxChanged(MQWidgetBase *sender, MQDocument doc)
{
	MQDoubleSpinBox *spin = static_cast<MQDoubleSpinBox*>(sender);
	wchar_t text[64];
	swprintf_s(text, L"DoubleSpinBox changed %f", spin->GetPosition());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::DSpinBoxChanging(MQWidgetBase *sender, MQDocument doc)
{
	MQDoubleSpinBox *spin = static_cast<MQDoubleSpinBox*>(sender);
	wchar_t text[64];
	swprintf_s(text, L"DoubleSpinBox changing %f", spin->GetPosition());
	m_BottomLabel->SetText(text);
	return FALSE;
}


int MainWindow::changeBone(MQDocument doc, BONESINFO& boneInfo)
{
	MQBoneManager bone_manager(this->m_pPlugin, doc);
	int bone_num = bone_manager.GetBoneNum();
	if (bone_num == 0) {
		return 0;
	}

	// Enum bones ボーンのIDリスト
	std::vector<UINT> bone_id;
	//std::vector<GPBBoneParam> bone_param;

	bone_id.resize(bone_num); // 領域確保する
	bone_manager.EnumBoneID(bone_id); // IDリストを取得

	for (int i = 0; i < bone_num; ++i) {
		std::wstring name;
		MQPoint translate;
		translate.x = 10.0f * (float)i;
		translate.y = (float)(i * i);
		translate.z = 0.0f;
		bone_manager.SetDeformTranslate(bone_id[i], translate);
		/*
		// ボーンID指定して受け取り変数を指定する
		bone_manager.GetParent(bone_id[i], bone_param[i].parent);
		// 子ボーン個数
		bone_manager.GetChildNum(bone_id[i], bone_param[i].child_num);
		bone_manager.GetBasePos(bone_id[i], bone_param[i].org_pos);
		bone_manager.GetDeformPos(bone_id[i], bone_param[i].def_pos);
		bone_manager.GetDeformMatrix(bone_id[i], bone_param[i].mtx);
		bone_manager.GetBaseMatrix(bone_id[i], bone_param[i].base_mtx);
		//bone_manager.GetDeformScale(bone_id[i], bone_param[i].scale);
		bone_manager.GetName(bone_id[i], name);
		bone_manager.GetDummy(bone_id[i], bone_param[i].dummy);
		*/
	}
	bone_manager.Update();
	return 1;
}

/// <summary>
/// スライダー変化時の処理か
/// </summary>
/// <param name="sender"></param>
/// <param name="doc"></param>
/// <returns></returns>
BOOL MainWindow::SliderChanged(MQWidgetBase *sender, MQDocument doc)
{
	wchar_t text[64];
	swprintf_s(text, L"Slider changed %f", m_Slider->GetPosition());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::SliderChanging(MQWidgetBase *sender, MQDocument doc)
{
	wchar_t text[64];
	auto pos = m_Slider->GetPosition();
	auto ipos = (int)floorf(pos);
	swprintf_s(text, L"Slider changing %d %f", ipos, pos);

	m_BottomLabel->SetText(text);
	BONESINFO bonesInfo;
	this->changeBone(doc, bonesInfo);
	return FALSE;
}

BOOL MainWindow::ScrollBarChanged(MQWidgetBase *sender, MQDocument doc)
{
	wchar_t text[64];
	swprintf_s(text, L"ScrollBar changed %d", m_ScrollBar->GetPosition());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::ScrollBarChanging(MQWidgetBase *sender, MQDocument doc)
{
	wchar_t text[64];
	swprintf_s(text, L"ScrollBar changing %d", m_ScrollBar->GetPosition());
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::ScrollSpinChanged(MQWidgetBase *sender, MQDocument doc)
{
	m_ScrollBar->SetMin(m_ScrollMinSpin->GetPosition());
	m_ScrollBar->SetMax(m_ScrollMaxSpin->GetPosition());
	m_ScrollBar->SetPage(m_ScrollPageSpin->GetPosition());
	m_ScrollBar->SetIncrement(m_ScrollIncSpin->GetPosition());
	return FALSE;
}

BOOL MainWindow::ColorPanelChanged(MQWidgetBase *sender, MQDocument doc)
{
	int r,g,b;
	m_ColorPanel->GetColor(r,g,b);
	double h,s,v;
	m_ColorPanel->GetHSV(h,s,v);

	wchar_t text[64];
	swprintf_s(text, L"ColorPanel changed RGB(%d %d %d) HSV(%.0lf %.2lf %.2lf)", r,g,b, h,s,v);
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::ColorPanelChanging(MQWidgetBase *sender, MQDocument doc)
{
	int r,g,b;
	m_ColorPanel->GetColor(r,g,b);
	double h,s,v;
	m_ColorPanel->GetHSV(h,s,v);

	wchar_t text[64];
	swprintf_s(text, L"ColorPanel changing RGB(%d %d %d) HSV(%.0lf %.2lf %.2lf)", r,g,b, h,s,v);
	m_BottomLabel->SetText(text);
	return FALSE;
}

BOOL MainWindow::PaintFramePaint(MQWidgetBase *sender, MQDocument doc, MQWidgetPaintParam& param)
{
	int w = m_PaintFrame->GetWidth();
	int h = m_PaintFrame->GetHeight();

	param.Canvas->SetAntiAlias(false);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->SetStrokeWidth(2.5f);
	param.Canvas->DrawLine(10, 10, 50, 30);

	param.Canvas->SetAntiAlias(true);
	param.Canvas->DrawLine(10.5f, 30.5f, 50.5f, 10.5f);

	param.Canvas->SetColor(0, 0, 255, 255);
	param.Canvas->FillCircle(20, 50, 8);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->DrawCircle(20, 50, 8);

	param.Canvas->SetColor(0, 0, 255, 255);
	param.Canvas->FillCircle(45.0f, 50.0f, 8.0f);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->DrawCircle(45.0f, 50.0f, 8.0f);

	param.Canvas->SetColor(0, 0, 255, 255);
	param.Canvas->FillEllipse(20, 70, 8, 6);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->DrawEllipse(20, 70, 8, 6);

	param.Canvas->SetColor(0, 0, 255, 255);
	param.Canvas->FillEllipse(45.0f, 70.0f, 8.0f, 6.0f);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->DrawEllipse(45.0f, 70.0f, 8.0f, 6.0f);

	std::vector<MQCanvasColor> colors(2);
	std::vector<float> segments(2);
	colors[0] = MQCanvasColor(0, 0, 255, 255);
	colors[1] = MQCanvasColor(0, 255, 255, 128);
	segments[0] = 0;
	segments[1] = 1;
	param.Canvas->SetGradientColor(10, 90, 30, 130, colors, segments);
	param.Canvas->FillRect(10, 90, 30, 40);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->DrawRect(10, 90, 30, 40);

	param.Canvas->SetGradientColor(50.f, 90.f, 80.f, 130.f, colors, segments);
	param.Canvas->FillRect(50.f, 90.f, 30.f, 40.f);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->DrawRect(50.f, 90.f, 30.f, 40.f);

	param.Canvas->SetColor(0, 0, 255, 255);
	param.Canvas->FillRoundRect(10, 140, 30, 40, 12, 6);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->DrawRoundRect(10, 140, 30, 40, 12, 6);

	param.Canvas->SetColor(0, 0, 255, 255);
	param.Canvas->FillRoundRect(50.f, 140.f, 30.f, 40.f, 12.f, 6.f);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->DrawRoundRect(50.f, 140.f, 30.f, 40.f, 12.f, 6.f);

	POINT pts_i[3];
	pts_i[0].x=10; pts_i[0].y=190;
	pts_i[1].x=10; pts_i[1].y=210;
	pts_i[2].x=50; pts_i[2].y=190;
	param.Canvas->SetColor(0, 0, 255, 255);
	param.Canvas->FillPolygon(pts_i, 3);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->DrawPolygon(pts_i, 3);

	MQCanvasPoint pts_f[3];
	pts_f[0].x=70; pts_f[0].y=190;
	pts_f[1].x=70; pts_f[1].y=210;
	pts_f[2].x=30; pts_f[2].y=210;
	param.Canvas->SetColor(0, 0, 255, 255);
	param.Canvas->FillPolygon(pts_f, 3);
	param.Canvas->SetColor(255, 0, 0, 255);
	param.Canvas->DrawPolygon(pts_f, 3);

	MQCanvasPoint polyline[3];
	polyline[0] = MQCanvasPoint(140,180);
	polyline[1] = MQCanvasPoint(180,120);
	polyline[2] = MQCanvasPoint(220,180);
	param.Canvas->SetStrokeCap(MQCanvas::CAP_BUTT);
	param.Canvas->SetStrokeJoin(MQCanvas::JOIN_BEVEL);
	param.Canvas->SetStrokeWidth(8);
	param.Canvas->SetColor(0, 0, 0, 255);
	param.Canvas->DrawPolyline(polyline, 3);
	param.Canvas->SetStrokeWidth(1);
	param.Canvas->SetColor(255, 255, 255, 255);
	param.Canvas->DrawPolyline(polyline, 3);
	
	polyline[0] = MQCanvasPoint(140,210);
	polyline[1] = MQCanvasPoint(180,150);
	polyline[2] = MQCanvasPoint(220,210);
	param.Canvas->SetStrokeCap(MQCanvas::CAP_ROUND);
	param.Canvas->SetStrokeJoin(MQCanvas::JOIN_ROUND);
	param.Canvas->SetStrokeWidth(8);
	param.Canvas->SetColor(0, 0, 0, 255);
	param.Canvas->DrawPolyline(polyline, 3);
	param.Canvas->SetStrokeWidth(1);
	param.Canvas->SetColor(255, 255, 255, 255);
	param.Canvas->DrawPolyline(polyline, 3);
	
	polyline[0] = MQCanvasPoint(140,240);
	polyline[1] = MQCanvasPoint(180,180);
	polyline[2] = MQCanvasPoint(220,240);
	param.Canvas->SetStrokeCap(MQCanvas::CAP_SQUARE);
	param.Canvas->SetStrokeJoin(MQCanvas::JOIN_MITER);
	param.Canvas->SetStrokeWidth(8);
	param.Canvas->SetColor(0, 0, 0, 255);
	param.Canvas->DrawPolyline(polyline, 3);
	param.Canvas->SetStrokeWidth(1);
	param.Canvas->SetColor(255, 255, 255, 255);
	param.Canvas->DrawPolyline(polyline, 3);

	polyline[0] = MQCanvasPoint(140,270);
	polyline[1] = MQCanvasPoint(180,210);
	polyline[2] = MQCanvasPoint(220,270);
	param.Canvas->SetStrokeMiterLimit(1.8f);
	param.Canvas->SetStrokeWidth(8);
	param.Canvas->SetColor(0, 0, 0, 255);
	param.Canvas->DrawPolyline(polyline, 3);
	param.Canvas->SetStrokeWidth(1);
	param.Canvas->SetColor(255, 255, 255, 255);
	param.Canvas->DrawPolyline(polyline, 3);


	param.Canvas->SetColor(0, 0, 0, 255);
	POINT size;
	int y = 20;
	param.Canvas->SetFont(L"Courier New", true);
	size = param.Canvas->MeasureText(L"Left");
	param.Canvas->DrawText(L"Left", 200, y, 160, size.y, false, false);
	y += size.y + 5;
	
	param.Canvas->SetFontSize(20);
	param.Canvas->SetFont(L"Times New Roman", true);
	size = param.Canvas->MeasureText(L"Middle");
	param.Canvas->DrawText(L"Middle", 200, y, 160, size.y, true, false);
	y += size.y + 5;
	
	param.Canvas->SetFontRateSize(2.5);
	param.Canvas->SetFont(L"Arial", true);
	size = param.Canvas->MeasureText(L"Right");
	param.Canvas->DrawText(L"Right", 360-size.x, y, size.x, size.y, true, false);
	y += size.y;

	param.Canvas->SetColor(0, 128, 128, 255);
	param.Canvas->DrawRect(200, 20, 160, y-20);


	return FALSE;
}

BOOL MainWindow::PaintFrameMouseMove(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
	int screen_x, screen_y;
	m_PaintFrame->ClientToScreen(param.ClientPos.x, param.ClientPos.y, &screen_x, &screen_y);
	wchar_t text[64];
	swprintf_s(text, L"client(%d %d) screen(%d %d)", param.ClientPos.x, param.ClientPos.y, screen_x, screen_y);
	m_BottomLabel->SetText(text);
	return FALSE;
}

