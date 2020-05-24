#include <base/color.h>

#include <engine/shared/config.h>
#include <engine/textrender.h>
#include <engine/graphics.h>
#include <engine/storage.h> // entities loading

#include <game/version.h>
// #include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/ui.h>
#include <game/client/teecomp.h>
#include <game/client/components/entities.h>

#include <generated/client_data.h>

#include <fstream> // Loading stats
#include <time.h> // Random

#include "binds.h"
#include "menus.h"
#include "skins.h"
#include "items.h"
#include "sounds.h"

// some legacy UI helpers, no biggie
void CMenus::NewLine(CUIRect *pButton, CUIRect *pView)
{
	pNewLineButton = pButton;
	pNewLineView = pView;
	
	if(pNewLineButton == NULL || pNewLineView == NULL)
		return;
	NewLine();
}

void CMenus::NewLine()
{
	if(pNewLineButton == NULL || pNewLineView == NULL)
		return;
	pNewLineView->HSplitTop(20.0f, pNewLineButton, pNewLineView);
} 

void CMenus::RenderSettingsGamer(CUIRect MainView)
{
	CUIRect Button, Tabbar, BottomView, HeaderFold;
	
	static int s_SettingsPage = 0;
	
	// Tabs (Teecomp pattern)
	MainView.HSplitBottom(80.0f, &MainView, &BottomView);
	if(Client()->State() != IClient::STATE_ONLINE)
		MainView.HSplitTop(20.0f, 0, 0);
	BottomView.HSplitTop(20.f, 0, &BottomView);
	MainView.HSplitTop(10.0f, &HeaderFold, &MainView);
	MainView.HSplitTop(4.0f, 0, &MainView);
	MainView.HSplitTop(24.0f, &Tabbar, &MainView);
	if(Client()->State() == IClient::STATE_ONLINE)
		RenderTools()->DrawUIRect4(&HeaderFold, 
			vec4(0.0f, 0.0f, 0.0f, g_Config.m_ClMenuAlpha/200.0f), 
			vec4(0.0f, 0.0f, 0.0f, g_Config.m_ClMenuAlpha/100.0f), 
			vec4(0.0f, 0.0f, 0.0f, g_Config.m_ClMenuAlpha/200.0f), 
			vec4(0.0f, 0.0f, 0.0f, g_Config.m_ClMenuAlpha/100.0f), 
			Client()->State() == IClient::STATE_OFFLINE ? CUI::CORNER_ALL : CUI::CORNER_B, 5.0f);
	

	const char* pTabs[] = {"General", "Assets"/* , "Stats", "Credits" */};
	int NumTabs = (int)(sizeof(pTabs)/sizeof(*pTabs));
	
	static CButtonContainer s_Buttons[2];
	for(int i=0; i<NumTabs; i++)
	{
		Tabbar.VSplitLeft(10.0f, &Button, &Tabbar);
		Tabbar.VSplitLeft(80.0f, &Button, &Tabbar);
				
		if (DoButton_MenuTabTop(&s_Buttons[i], pTabs[i], s_SettingsPage == i, &Button, 1.0f, 1.0f, CUI::CORNER_T, 5.0f, 0.25f))
			s_SettingsPage = i;
		if(i == 1)
		{	
			CUIRect Label = Button;
			Label.w -= 5.0f;
			TextRender()->TextColor(0.9f, 0.1f, 0.1f, 1.0f);
			UI()->DoLabel(&Label, Localize("NEW!"), 7.0f, CUI::ALIGN_RIGHT);
			TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	if(s_SettingsPage != 1)
		RenderTools()->DrawUIRect(&MainView, vec4(0.0f, 0.0f, 0.0f, 0.5f), CUI::CORNER_ALL, 10.0f);
	else
		RenderTools()->DrawUIRect(&MainView, vec4(0.0f, 0.0f, 0.0f, 0.25f), CUI::CORNER_ALL, 10.0f);
	MainView.Margin(10.0f, &MainView);
	
	if(s_SettingsPage == 0)
		RenderSettingsGamerGeneral(MainView);
	else if(s_SettingsPage == 1)
	{
		RenderSettingsGamerEntities(MainView);
	}
	else if(s_SettingsPage == 2)
	{
		// RenderSettingsGamerStats(MainView);
	}
	else if(s_SettingsPage == 3)
	{
		// RenderSettingsGamerCredits(MainView);
	}
}

void CMenus::RenderSettingsGamerGeneral(CUIRect MainView)
{
	CUIRect Button, LeftView, RightView;
	char aBuf[512];
	
	static int s_FirstTime = 1;
	if(s_FirstTime)
	{
		srand(time(NULL));
		s_FirstTime = 0;
	}
	
	MainView.VSplitLeft(MainView.w/2, &LeftView, &RightView);
		
	// LeftView.HSplitTop(20.0f, &Button, &LeftView);
	NewLine(&Button, &LeftView);
	
	UI()->DoLabel(&Button, Localize("Health and Ammo"), 14.0f, CUI::ALIGN_LEFT);
	NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClGhud, "Gamer HUD", &Button);
		
	NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClNoHud, "Disable HUD", &Button);
		
	NewLine();
	if(DoButton_CheckBox(&g_Config.m_GfxHealthBar, "Display healthbar", g_Config.m_GfxHealthBar, &Button))
		g_Config.m_GfxHealthBar = g_Config.m_GfxHealthBar ? 0 : 3;
	
	if(g_Config.m_GfxHealthBar > 0 && g_Config.m_GfxHealthBar != 3)
	{
		NewLine();
		Button.VSplitLeft(20.0f, 0, &Button);
		DoButton_BinaryCheckBox(&g_Config.m_GfxHealthBarDamagedOnly, "Only display damaged attributes", &Button);

		NewLine();
		Button.VSplitLeft(20.0f, 0, &Button);
		DoButton_BinaryCheckBox(&g_Config.m_GfxArmorUnderHealth, "Render armor under health", &Button);
		
		NewLine();
		Button.VSplitLeft(20.0f, 0, &Button);
		DoButton_BinaryCheckBox(&g_Config.m_GfxHealthBarNumbers, "Render numbers next to the healthbar", &Button);
	}

	// NewLine();
	// DoButton_BinaryCheckBox(&g_Config.m_ClGcolor, "Use Gamer colors", &Button);
	/*
	
	if(g_Config.m_ClAnnouncers)
	{
		// NewLine();
		// DoButton_BinaryCheckBox(&g_Config.m_ClAnnouncersShadows, "Render shadows around the announcers", &Button);
		
		NewLine();
		DoButton_BinaryCheckBox(&g_Config.m_ClAnnouncersLegend, "Render a legend under the announcers", &Button);
	}

	NewLine();
	NewLine();
	UI()->DoLabel(&Button, Localize("Extras"), 14.0f, CUI::ALIGN_LEFT);
	NewLine();
	{
		static float Offset = 0.0f;
		static int s_PlusButton = 0;
		static int s_MinusButton = 0;
		bool Minus, Plus;
		
		str_format(aBuf, sizeof(aBuf), "%d", g_Config.m_ClBroadcastSize);
		UI()->DoLabel(&Button, Localize("Broadcast size (%)"), 12.0f, -1);
		Button.VSplitLeft(120.0f, 0, &Button);
		Button.VSplitLeft(40.0f, &Button, 0);
		DoEditBox(&g_Config.m_ClBroadcastSize, &Button, aBuf, sizeof(aBuf), 14.0f, &Offset);
		
		g_Config.m_ClBroadcastSize =  max(1, str_toint(aBuf));
		
		Button.VSplitLeft(45.0f, 0, &Button);
		Button.VSplitLeft(15.0f, &Button, 0);
		Plus = DoButton_Menu(&s_PlusButton, "+", 0, &Button);
		
		Button.VSplitLeft(16.0f, 0, &Button);
		Button.VSplitLeft(15.0f, &Button, 0);
		Minus = DoButton_Menu(&s_MinusButton, "-", 0, &Button);
		
		g_Config.m_ClBroadcastSize += (int)Plus*10 - (int)Minus*10;
	}*/

	NewLine();
	NewLine();
	UI()->DoLabel(&Button, Localize("Instashield"), 14.0f, CUI::ALIGN_LEFT);
	NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClShieldDisplay, "Display prettier shield graphics", &Button);
	
	NewLine();
	NewLine();
	UI()->DoLabel(&Button, Localize("Announcers"), 14.0f, CUI::ALIGN_LEFT);
	NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClAnnouncers, "Visual announcers", &Button);
	NewLine();
	if(DoButton_CheckBox(&g_Config.m_ClGSound, Localize("Play announcer sounds"), g_Config.m_ClGSound, &Button))
		g_Config.m_ClGSound ^= 1;
		
	Button.VSplitLeft(185.0f, 0, &Button);
	Button.VSplitLeft(140.0f, &Button, 0);
	static CButtonContainer s_TestButton;
	if(DoButton_Menu(&s_TestButton, "Test one!", 0, &Button))
	{
		int sounds[11] = {SOUND_SPREE_HUMILIATION, SOUND_SPREE_KILLING, SOUND_SPREE_RAMPAGE, SOUND_SPREE_DOMINATING, SOUND_SPREE_UNSTOPPABLE, SOUND_SPREE_GODLIKE,
		SOUND_SPREE_WICKEDSICK, SOUND_SPREE_PREPARETOFIGHT, SOUND_SPREE_PREPARETOKILL, SOUND_SPREE_HOLYSHIT, SOUND_SPREE_FIRSTBLOOD};
		m_pClient->m_pSounds->Play(CSounds::CHN_GUI, sounds[rand()%11], 0);
	}	

	NewLine();
	NewLine();
	UI()->DoLabel(&Button, Localize("Misc"), 14.0f, CUI::ALIGN_LEFT);
	NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClPingGraph, "Show network graph next to ping in scoreboard", &Button);
	NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClClientRecognition, "Enable gamer client recognition", &Button);
		
	// ===------------ right side ------------===
	NewLine(&Button, &RightView);
	
	UI()->DoLabel(&Button, Localize("Chat"), 14.0f, CUI::ALIGN_LEFT);
	
	NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClChatSound, "Chat sounds", &Button);

	if(g_Config.m_ClChatSound)
	{
		NewLine();
		DoButton_BinaryCheckBox(&g_Config.m_ClSwapChatSounds, "Swap normal/highlight chat sounds", &Button);
	}
		
	NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClDiscreetWhispers, "Discreet whispering", &Button);

	/*NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClArrows, "Use arrows in place of chat", &Button);
		
	NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClNoCustomForArrows, "Do not use custom team colors for arrows", &Button);
	*/

	NewLine();
	{		
		static float Offset = 0.0f;
		static CButtonContainer s_PlusButton;
		static CButtonContainer s_MinusButton;
		bool Minus, Plus;
		
		str_format(aBuf, sizeof(aBuf), "%d", g_Config.m_ClTextSize);
		UI()->DoLabel(&Button, Localize("Chat size (%)"), 12.0f, CUI::ALIGN_LEFT);
		Button.VSplitLeft(90.0f, 0, &Button);
		Button.VSplitLeft(40.0f, &Button, 0);
		DoEditBox(&g_Config.m_ClTextSize, &Button, aBuf, sizeof(aBuf), 14.0f, &Offset);
		
		Button.VSplitLeft(45.0f, 0, &Button);
		Button.VSplitLeft(15.0f, &Button, 0);
		Plus = DoButton_Menu(&s_PlusButton, "+", 0, &Button);
		
		Button.VSplitLeft(16.0f, 0, &Button);
		Button.VSplitLeft(15.0f, &Button, 0);
		Minus = DoButton_Menu(&s_MinusButton, "-", 0, &Button);
			
		Button.VSplitLeft(45.0f, 0, &Button);
		UI()->DoLabel(&Button, Localize("Dune says : Test !"), 12.0f*clamp(g_Config.m_ClTextSize, 50, 200)/100.0f, CUI::ALIGN_LEFT);
		g_Config.m_ClTextSize =  max(1, str_toint(aBuf));
		
		if(Plus)
			g_Config.m_ClTextSize += 10;
		else if(Minus)
			g_Config.m_ClTextSize -= 10;
	}

	NewLine();
	NewLine();
	UI()->DoLabel(&Button, Localize("Automapper"), 14.0f, CUI::ALIGN_LEFT);
	NewLine();
	DoButton_BinaryCheckBox(&g_Config.m_ClAutomapperMenus, "Show automapper options in the ingame menus", &Button);
	NewLine();
	static int s_ShowGameTiles = 0;
	if(DoButton_CheckBox(&s_ShowGameTiles, "Show the game tiles", g_Config.m_GfxGameTiles > 0, &Button))
	{
		if(g_Config.m_GfxGameTiles)
			g_Config.m_GfxGameTiles = 0;
		else
			g_Config.m_GfxGameTiles = 1;
	}

	RightView.HSplitTop(10.0f, &Button, &RightView);
	NewLine();
	UI()->DoLabel(&Button, Localize("Game entities background"), 14.0f, CUI::ALIGN_LEFT);
	// NewLine();
	// RenderTools()->DrawUIRect(&Button, vec4(0.0f, 0.0f, 0.0f, 0.25f), CUI::CORNER_ALL, 5.0f);
	// static CButtonContainer s_ResetClearButton;
	// if(DoButton_Menu(&s_ResetClearButton, Localize("Reset background color"), 0, &Button))
	// {
	// 	const ivec4 DefaultClearColor = ivec4(149, 255, 53, 255);
	// 	g_Config.m_GfxClearColor = (int(DefaultClearColor.x) << 16) + (int(DefaultClearColor.y) << 8) + int(DefaultClearColor.z);
	// }
	bool Modified;
	static HSLPickerState HSLState;
	ivec4 Hsl = RenderHSLPicker(RightView, g_Config.m_GfxClearColor, false, Modified, HSLState);
	if(Modified)
		g_Config.m_GfxClearColor = (Hsl.x << 16) + (Hsl.y << 8) + Hsl.z;

	NewLine(NULL, NULL);
}

void CMenus::RenderSettingsGamerEntities(CUIRect MainView)
{
	// tabs
	static int s_EntitiesPage = 0;
	CUIRect Tabbar, Button;
	MainView.HSplitTop(/* 1 */4.0f, 0, &MainView);
	MainView.HSplitTop(/* 24 */20.0f, &Tabbar, &MainView);

	const char* pTabs[] = {"Game skin", "Particles", "Cursor", "Emoticons", "Font"};
	int NumTabs = (int)(sizeof(pTabs)/sizeof(*pTabs));
	
	for(int i=0; i<NumTabs; i++)
	{
		Tabbar.VSplitLeft(10.0f, &Button, &Tabbar);
		Tabbar.VSplitLeft(80.0f, &Button, &Tabbar);
				
		static CButtonContainer s_Buttons[5];
		if (DoButton_MenuTabTop(&s_Buttons[i], pTabs[i], s_EntitiesPage == i, &Button, 1.0f, 1.0f, CUI::CORNER_T, 5.0f, 0.25f))
				s_EntitiesPage = i;
	}
	RenderTools()->DrawUIRect(&MainView, vec4(0.0f, 0.0f, 0.0f, 0.25f), CUI::CORNER_ALL, 10.0f);

	// loading
	if(!m_pClient->m_pEntities->IsLoaded())
	{
		char aBuf[512];
		CUIRect Button;
		Button = MainView;
		Button.HMargin(MainView.h/2-24.0f, &Button);
		static int MustLoadCountDown = 5;
		if(MustLoadCountDown) // aesthetics
		{
			str_format(aBuf, sizeof(aBuf), "%s...", Localize("Loading"));
			UI()->DoLabel(&Button, aBuf, 24.0f, CUI::ALIGN_CENTER);
			MustLoadCountDown--;
			if(MustLoadCountDown == 0)
				m_pClient->m_pEntities->LoadEntities();
		}
	}
	else
	{
		if(s_EntitiesPage == 0)
			RenderSettingsGamerEntitiesGameSkin(MainView);
		else if(s_EntitiesPage == 1)
			RenderSettingsGamerEntitiesParticles(MainView);
		else if(s_EntitiesPage == 2)
			RenderSettingsGamerEntitiesCursor(MainView);
		else if(s_EntitiesPage == 3)
			RenderSettingsGamerEntitiesEmoticons(MainView);
		else if(s_EntitiesPage == 4)
			RenderSettingsGamerEntitiesFont(MainView);
	}
}

void CMenus::RenderSettingsGamerEntitiesGameSkin(CUIRect MainView)
{
	RenderSettingsGamerEntitiesGeneric(MainView, &m_pClient->m_pEntities->m_GameSkins, g_Config.m_ClCustomGameskin, "Game skin", g_Config.m_UiWideview ? 4 : 3, 2.0f);
}

void CMenus::RenderSettingsGamerEntitiesParticles(CUIRect MainView)
{
	RenderSettingsGamerEntitiesGeneric(MainView, &m_pClient->m_pEntities->m_Particles, g_Config.m_ClCustomParticles, "Particles", g_Config.m_UiWideview ? 6 : 5, 1.0f);
}

void CMenus::RenderSettingsGamerEntitiesCursor(CUIRect MainView)
{
	RenderSettingsGamerEntitiesGeneric(MainView, &m_pClient->m_pEntities->m_Cursors, g_Config.m_ClCustomCursor, "Cursors", g_Config.m_UiWideview ? 20 : 16, 1.0f);
}

void CMenus::RenderSettingsGamerEntitiesEmoticons(CUIRect MainView)
{
	RenderSettingsGamerEntitiesGeneric(MainView, &m_pClient->m_pEntities->m_Emoticons, g_Config.m_ClCustomEmoticons, "Emoticons", g_Config.m_UiWideview ? 6 : 5, 1.0f);
}

int CMenus::FontsScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CMenus *pSelf = (CMenus *)pUser;
	const char *pSuffix = str_endswith(pName, ".ttf");
	if(IsDir || !str_endswith(pName, ".ttf"))
		return 0;

	char aFontName[128];
	str_truncate(aFontName, sizeof(aFontName), pName, pSuffix - pName);
	pSelf->m_aFonts.add(aFontName);
	return 0;
}

void CMenus::RenderSettingsGamerEntitiesFont(CUIRect MainView)
{
	static int s_FontList = 0;
	static int s_SelectedFont = -1;
	static CListBoxState s_ListBoxState;

	if(m_aFonts.size() == 0) // not loaded yet
	{
		Storage()->ListDirectory(IStorage::TYPE_ALL, "fonts", FontsScan, (CMenus*)this);
		for(int i = 0; i < m_aFonts.size(); i++)
			if(str_comp(m_aFonts[i], g_Config.m_ClFontfile) == 0)
			{
				s_SelectedFont = i;
				break;
			}
	}

	int OldSelected = s_SelectedFont;

	UiDoListboxHeader(&s_ListBoxState, &MainView, Localize("Font"), 20.0f, 2.0f);
	UiDoListboxStart(&s_ListBoxState, &s_FontList, 20.0f, 0, m_aFonts.size(), 1, s_SelectedFont, 0, true);

	for(sorted_array<string>::range r = m_aFonts.all(); !r.empty(); r.pop_front())
	{
		bool IsActive = true;
		CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, &r.front(), false, &IsActive);

		if(Item.m_Visible)
		{
			Item.m_Rect.y += 2.0f;
			Item.m_Rect.x += 10.0f;
			if(s_SelectedFont != -1 && !str_comp(m_aFonts[s_SelectedFont], r.front()))
			{
				TextRender()->TextColor(0.0f, 0.0f, 0.0f, 1.0f);
				TextRender()->TextOutlineColor(1.0f, 1.0f, 1.0f, 0.25f);
				UI()->DoLabel(&Item.m_Rect, r.front(), Item.m_Rect.h*ms_FontmodHeight*0.8f, CUI::ALIGN_LEFT);
				TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
				TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.3f);
			}
			else
				UI()->DoLabel(&Item.m_Rect, r.front(), Item.m_Rect.h*ms_FontmodHeight*0.8f, CUI::ALIGN_LEFT);
		}
	}

	s_SelectedFont = UiDoListboxEnd(&s_ListBoxState, 0);

	if(OldSelected != s_SelectedFont)
	{
		str_format(g_Config.m_ClFontfile, sizeof(g_Config.m_ClFontfile), "%s.ttf", m_aFonts[s_SelectedFont].cstr());
		// reload font
		char aFontName[256];
		str_format(aFontName, sizeof(aFontName), "fonts/%s", g_Config.m_ClFontfile);
		char aFilename[512];
		IOHANDLE File = Storage()->OpenFile(aFontName, IOFLAG_READ, IStorage::TYPE_ALL, aFilename, sizeof(aFilename));
		if(File)
		{
			io_close(File);
			if(TextRender()->LoadFont(aFilename))
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "failed to load font. filename='%s'", aFontName);
				Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", aBuf);
			}
		}
	}
}

void CMenus::RenderSettingsGamerEntitiesGeneric(CUIRect MainView, CEntities::CTextureEntity* pEntities, char* pConfigStr, const char* pLabel, int ItemsPerRow, float Ratio)
{
	char aBuf[512];
	static CListBoxState s_ListBoxState;
	int OldSelected = -1;
	str_format(aBuf, sizeof(aBuf), "%s: %s", pLabel, pConfigStr[0] ? pConfigStr : "default");
	UiDoListboxHeader(&s_ListBoxState, &MainView, aBuf, 20.0f, 2.0f);

	const int Num = pEntities->Num();
	UiDoListboxStart(&s_ListBoxState, &s_ListBoxState, MainView.w/(float)ItemsPerRow/Ratio, 0, Num, ItemsPerRow, OldSelected);

	for(int i = 0; i < Num+1; ++i) // first is default
	{
		if(i == 0)
		{
			if(pConfigStr[0] == '\0')
				OldSelected = i;
		}
		else if(str_comp(pEntities->GetName(i-1), pConfigStr) == 0)
			OldSelected = i;
		static int s_DefaultEntitiyId;
		CListboxItem Item = UiDoListboxNextItem(&s_ListBoxState, i > 0 ? (void*)pEntities->GetName(i-1) : (void*)&s_DefaultEntitiyId, OldSelected == i);
		if(Item.m_Visible)
		{
			CUIRect Pos;
			Item.m_Rect.Margin(5.0f, &Item.m_Rect);
			Item.m_Rect.HSplitBottom(10.0f, &Item.m_Rect, &Pos);

			Item.m_Rect.h = Item.m_Rect.w/Ratio;

			Graphics()->BlendNormal();
			if(i == 0)
				Graphics()->TextureSet(pEntities->GetDefault());
			else
				Graphics()->TextureSet(pEntities->Get(i-1));
			Graphics()->QuadsBegin();
			IGraphics::CQuadItem QuadItem(Item.m_Rect.x, Item.m_Rect.y, Item.m_Rect.w, Item.m_Rect.h);
			Graphics()->QuadsDrawTL(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}
	}

	const int NewSelected = UiDoListboxEnd(&s_ListBoxState, 0);
	if(OldSelected != NewSelected)
	{
		if(NewSelected == 0)
			pConfigStr[0] = '\0';
		else
			str_copy(pConfigStr, pEntities->GetName(NewSelected-1), 255);
		pEntities->Reload(NewSelected - 1); // -1 is default
	}
}

#if 0
#define PRINT_STATS_BEGIN(str) \
	MainView.HSplitTop(20.0f, &Button, &MainView);\
	UI()->DoLabel(&Button, Localize(str), 14.0f, CUI::ALIGN_LEFT);\
	Button.VSplitLeft(115.0f, 0, &Button);\
	
#define PRINT_STATS(str, var) \
	PRINT_STATS_BEGIN(str) \
	str_format(buf, sizeof(buf), "%d", var);\
	UI()->DoLabel(&Button, buf, 12.0f, CUI::ALIGN_LEFT);\

#define PRINT_STATS_MED(str, var, games) \
	PRINT_STATS_BEGIN(str) \
	str_format(buf, sizeof(buf), "%.2f", (float)var/(float)games);\
	UI()->DoLabel(&Button, buf, 12.0f, CUI::ALIGN_LEFT);\

#define PRINT_STATS_STR(str, str2, str3) \
	PRINT_STATS_BEGIN(str) \
	UI()->DoLabel(&Button, str2, 12.0f, CUI::ALIGN_LEFT);\
	Button.VSplitLeft(120.0f, 0, &Button);\
	UI()->DoLabel(&Button, str3, 12.0f, CUI::ALIGN_LEFT);\


void CMenus::RenderSettingsGamerStats(CUIRect MainView)
{
	CUIRect Button, ResetButton, RefreshButton, Footer;
	static PersonalStats stats;
	static int IsLoaded = NOT_LOADED;
	
	if(!IsLoaded)
	{
		char Path[512];
		FILE* File;
		
#if defined(CONF_FAMILY_WINDOWS)
		str_format(Path, sizeof(Path), "%s\\stats.cfg", ClientUserDirectory());
#else
		str_format(Path, sizeof(Path), "%s/stats.cfg", ClientUserDirectory());
#endif
		
		File = fopen(Path, "r");
		if(File)
		{
			int* StatsVar[] = {&stats.games, &stats.frags_with[0], &stats.frags_with[1], &stats.frags_with[2], &stats.frags_with[3], &stats.frags_with[4], &stats.frags_with[5],
			&stats.deaths_from[0], &stats.deaths_from[1], &stats.deaths_from[2], &stats.deaths_from[3], &stats.deaths_from[4], &stats.deaths_from[5], &stats.frags, &stats.deaths,
			&stats.suicides, &stats.flag_grabs, &stats.flag_captures, &stats.carriers_killed, &stats.kills_carrying, &stats.deaths_carrying, &stats.won,};
		
			int NumStatsVar = 22, i;
			
			for(i = 0; i < NumStatsVar; i++)
			{
				if(!fscanf(File, "%d", StatsVar[i]))
				{
					dbg_msg("client/stats", "file is corrupted, cannnot continue reading");
					break;
				}
			}
			
			fclose(File);
			
			IsLoaded = (i == NumStatsVar) ? LOADED : FAILED_LOADING;
		}
		else
			IsLoaded = FAILED_LOADING;
	}
	
	MainView.HSplitTop(3.0f, &MainView, &Button);
	
	UI()->DoLabel(&Button, Localize("Personal statistics"), 20.0f, -1);
	
	MainView.HSplitTop(10.0f, &Button, &MainView);
	// MainView.HSplitTop(20.0f, &Button, &MainView);
	NewLine(&Button, &MainView);
	
	NewLine();
	if(DoButton_CheckBox(&g_Config.m_ClRegisterStats, Localize("Register stats"), g_Config.m_ClRegisterStats, &Button))
		g_Config.m_ClRegisterStats ^= 1;
		
	NewLine();
	if(DoButton_CheckBox(&g_Config.m_ClRegisterStatsPure, Localize("Only register stats from pure games"), g_Config.m_ClRegisterStatsPure, &Button))
		g_Config.m_ClRegisterStatsPure ^= 1;
	
	NewLine();
	
	if(IsLoaded == FAILED_LOADING)
	{
		UI()->DoLabel(&Button, Localize("Cannot load stats: you have never finished a game."), 14.0f, -1);
	}
	else
	{		
		char buf[256];
		char buf2[256];
		PRINT_STATS("Games:", stats.games)
		
		if(stats.games)
			str_format(buf, sizeof(buf), "%.1f%%", 100*(float)stats.won/(float)stats.games);
		else str_format(buf, sizeof(buf), "N/A");
		PRINT_STATS_STR("Won:", buf, "")
		// MainView.HSplitTop(10.0f, &Button, &MainView);

		if(stats.frags > (stats.deaths))
			TextRender()->TextColor(0,1,0,1);
		else if(stats.frags < (stats.deaths))
			TextRender()->TextColor(1,0,0,1);
		else
			TextRender()->TextColor(1,1,0,1);
		if(stats.deaths)
			str_format(buf, sizeof(buf), "%.2f (%d/%d)", (float)stats.frags/(float)(stats.deaths), stats.frags, stats.deaths);
		else str_format(buf, sizeof(buf), "N/A (%d/%d)", stats.frags, stats.deaths);
		PRINT_STATS_STR("K/D Ratio:", buf, "")
		TextRender()->TextColor(1,1,1,1);
		
		MainView.HSplitTop(8.0f, &Button, &MainView);
		NewLine();
		UI()->DoLabel(&Button, Localize("Per game:"), 16.0f, -1);
		if(stats.games)
		{
			PRINT_STATS_MED("Suicides:", stats.suicides, stats.games)
			PRINT_STATS_MED("Flag grabs:", stats.flag_grabs, stats.games)
			PRINT_STATS_MED("Carriers killed:", stats.carriers_killed, stats.games)
			PRINT_STATS_MED("Kills carrying:", stats.kills_carrying, stats.games)
			PRINT_STATS_MED("Deaths carrying:", stats.deaths_carrying, stats.games)
		}
		else
		{
			PRINT_STATS("Suicides:", stats.suicides)
			PRINT_STATS("Flag grabs:", stats.flag_grabs)
			PRINT_STATS("Carriers killed:", stats.carriers_killed)
			PRINT_STATS("Kills carrying:", stats.kills_carrying)
			PRINT_STATS("Deaths carrying:", stats.deaths_carrying)
		}
		
		NewLine();
		MainView.HSplitTop(16.0f, &Button, &MainView);
		
		const char* WeaponsStr[6] = {"      Hammer", "      Pistol", "      Shotgun", "      Rocket", "      Rifle", "      Ninja"};
		Button.VSplitLeft(110.0f, 0, &Button); UI()->DoLabel(&Button, Localize("Frags with:"), 14.0f, -1);
		Button.VSplitLeft(120.0f, 0, &Button); UI()->DoLabel(&Button, Localize("Deaths from:"), 14.0f, -1);
		
		for(int i = 0; i < 6; i++)
		{
			if(!stats.frags_with[i] && !stats.deaths_from[i]) // Unused weapon
				continue;
				
			if(stats.frags)
			{
				if(stats.frags_with[i])
					str_format(buf, sizeof(buf), "%s%.1f%%", 10*stats.frags_with[i] < stats.frags ? "0" : "",
						(float)(100*stats.frags_with[i])/(float)stats.frags);
				else
					str_copy(buf, "    -", sizeof(buf));
			}
			else str_copy(buf, "N/A", sizeof(buf));
			if(stats.deaths)
			{
				if(stats.deaths_from[i])
					str_format(buf2, sizeof(buf2), "%s%.1f%%", 10*stats.deaths_from[i] < stats.deaths ? "0" : "",
						(float)(100*stats.deaths_from[i])/(float)stats.deaths);
				else
					str_copy(buf2, "    -", sizeof(buf2));
			}
			else str_copy(buf2, "N/A", sizeof(buf2));
			
			PRINT_STATS_STR(WeaponsStr[i], buf, buf2)
			
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
			Graphics()->QuadsBegin();
			RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i].m_pSpriteBody, 0);
			if(i == 5)
				RenderTools()->DrawSprite(30.0f, Button.y+10.0f, 27.0f);
			else
				RenderTools()->DrawSprite(35.0f, Button.y+10.0f, 27.0f);
			Graphics()->QuadsEnd();
		}
	}

	// draw footers	
	MainView.HSplitTop(15.0f, &Button, &MainView);
	NewLine();
	
	Button.HSplitTop(20.0f, &RefreshButton, &MainView);
	Button.HSplitTop(20.0f, &ResetButton, &MainView);
	
	RefreshButton.VSplitLeft(20.0f, 0, &RefreshButton);
	RefreshButton.VSplitLeft(150.0f, &RefreshButton, 0);
	static int s_Refresh = 0;
	if(DoButton_Menu(&s_Refresh, Localize("Refresh"), 0, &RefreshButton))
	{
		IsLoaded = NOT_LOADED;
	}

	ResetButton.VSplitLeft(200.0f, 0, &ResetButton);
	ResetButton.VSplitLeft(150.0f, &ResetButton, 0);
	static int s_Reset = 0;
	if(DoButton_Menu(&s_Reset, Localize("Reset stats"), 0, &ResetButton))
	{
		char Path[512];
		FILE* File;
		
#if defined(CONF_FAMILY_WINDOWS)
		str_format(Path, sizeof(Path), "%s\\stats.cfg", ClientUserDirectory());
#else
		str_format(Path, sizeof(Path), "%s/stats.cfg", ClientUserDirectory());
#endif

		File = fopen(Path, "w+");
		if(!File)
			return;
		for(int j = 0; j < 22; j++)
			fputs("0\n", File);
		fclose(File);
		IsLoaded = NOT_LOADED; // We will refresh the stats the next time
	}
}

#endif