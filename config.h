/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>
#include <stdbool.h>

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;     /* 0 means no systray */
static const int showbar            = 1;     /* 0 means no bar */
static const int topbar             = 1;     /* 0 means bottom bar */
static int floatposgrid_x           = 5;        /* float grid columns */
static int floatposgrid_y           = 5;        /* float grid rows */
static const char *fonts[]          = { "monospace:size=10" };
static const char dmenufont[]       = "monospace:size=10";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char col1[]            = "#ffffff";
static const char col2[]            = "#ffffff";
static const char col3[]            = "#ffffff";
static const char col4[]            = "#ffffff";
static const char col5[]            = "#ffffff";
static const char col6[]            = "#ffffff";

enum { SchemeNorm, SchemeCol1, SchemeCol2, SchemeCol3, SchemeCol4,
       SchemeCol5, SchemeCol6, SchemeSel }; /* color schemes */

static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm]  = { col_gray3, col_gray1, col_gray2 },
	[SchemeCol1]  = { col1,      col_gray1, col_gray2 },
	[SchemeCol2]  = { col2,      col_gray1, col_gray2 },
	[SchemeCol3]  = { col3,      col_gray1, col_gray2 },
	[SchemeCol4]  = { col4,      col_gray1, col_gray2 },
	[SchemeCol5]  = { col5,      col_gray1, col_gray2 },
	[SchemeCol6]  = { col6,      col_gray1, col_gray2 },
	[SchemeSel]   = { col_gray4, col_cyan,  col_cyan  },
};

static const char *const autostart[] = {
	"kitty", NULL,
	"/home/derpy/.local/share/firefoxpwa/runtime/firefox", NULL,
	"dwmblocks", NULL,
	"/home/derpy/.local/bin/vicinae", "server", NULL,
	NULL /* terminate */
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   floatpos   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           NULL,        -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           NULL,        -1 },
	{ "XVkbd",    NULL, "xvkbd - Virtual Keyboard (United Kingdom)", 0, true, "0x 0y 400W 130H", -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */
static const int refreshrate = 60;  /* refresh rate (per second) for client move/resize */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} }, \
	{ MODKEY,                      XK_s,      togglesticky,   {0} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "kitty", NULL };
static const char *appcmd[]   = { "rofi", "-show", "drun", NULL };
static const char *pavucmd[]   = { "pavucontrol", NULL };
static const char *firefuck[] = { "firefox-bin", NULL };
// for pulse compatible //
static const char *upvol[] = { "amixer", "-q", "sset", "Master", "5%+", NULL };
static const char *downvol[] = { "amixer", "-q", "sset", "Master", "5%-", NULL };
static const char *mute[] = { "amixer", "-q", "-D", "pulse", "sset", "Master", "toggle", NULL };
static const char *brighter[] = {"brightnessctl", "s", "+2%", NULL };
// static const char *brightless[] = {"brightnessctl", "s", "2%-", NULL }; //
static const char *catpiss[] = {"/home/derpy/.local/bin/vicinae", "toggle", NULL };
static const char *flumshots[] = {"flameshot", "gui", NULL };
static const char *croakapp[] = {"xkill", NULL };

static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ 0,              XF86XK_AudioRaiseVolume, spawn,          {.v = upvol } },
        { 0,              XF86XK_AudioLowerVolume, spawn,          {.v = downvol } },
        { 0,              XF86XK_AudioMute,        spawn,          {.v = mute } },
        { 0,              XF86XK_MonBrightnessUp,   spawn,          {.v = brighter } },
        { 0,              XF86XK_MonBrightnessDown, spawn,          {.v = croakapp } },
	{ MODKEY|ShiftMask,             XK_w,      spawn,          {.v = firefuck } },
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_d,      spawn,          {.v = appcmd } },
	{ MODKEY,                       XK_v,      spawn,          {.v = pavucmd } },
	{ MODKEY,                       XK_d,      spawn,          {.v = catpiss } },
	{ MODKEY|ShiftMask,             XK_l,      spawn,          {.v = flumshots } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_g,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefullscr,  {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	/* Client position is limited to monitor window area */
	{ Mod4Mask,                     XK_u,      floatpos,       {.v = "-26x -26y" } }, // ↖
	{ Mod4Mask,                     XK_i,      floatpos,       {.v = "  0x -26y" } }, // ↑
	{ Mod4Mask,                     XK_o,      floatpos,       {.v = " 26x -26y" } }, // ↗
	{ Mod4Mask,                     XK_j,      floatpos,       {.v = "-26x   0y" } }, // ←
	{ Mod4Mask,                     XK_l,      floatpos,       {.v = " 26x   0y" } }, // →
	{ Mod4Mask,                     XK_m,      floatpos,       {.v = "-26x  26y" } }, // ↙
	{ Mod4Mask,                     XK_comma,  floatpos,       {.v = "  0x  26y" } }, // ↓
	{ Mod4Mask,                     XK_period, floatpos,       {.v = " 26x  26y" } }, // ↘
	/* Absolute positioning (allows moving windows between monitors) */
	{ Mod4Mask|ControlMask,         XK_u,      floatpos,       {.v = "-26a -26a" } }, // ↖
	{ Mod4Mask|ControlMask,         XK_i,      floatpos,       {.v = "  0a -26a" } }, // ↑
	{ Mod4Mask|ControlMask,         XK_o,      floatpos,       {.v = " 26a -26a" } }, // ↗
	{ Mod4Mask|ControlMask,         XK_j,      floatpos,       {.v = "-26a   0a" } }, // ←
	{ Mod4Mask|ControlMask,         XK_l,      floatpos,       {.v = " 26a   0a" } }, // →
	{ Mod4Mask|ControlMask,         XK_m,      floatpos,       {.v = "-26a  26a" } }, // ↙
	{ Mod4Mask|ControlMask,         XK_comma,  floatpos,       {.v = "  0a  26a" } }, // ↓
	{ Mod4Mask|ControlMask,         XK_period, floatpos,       {.v = " 26a  26a" } }, // ↘
	/* Resize client, client center position is fixed which means that client expands in all directions */
	{ Mod4Mask|ShiftMask,           XK_u,      floatpos,       {.v = "-26w -26h" } }, // ↖
	{ Mod4Mask|ShiftMask,           XK_i,      floatpos,       {.v = "  0w -26h" } }, // ↑
	{ Mod4Mask|ShiftMask,           XK_o,      floatpos,       {.v = " 26w -26h" } }, // ↗
	{ Mod4Mask|ShiftMask,           XK_j,      floatpos,       {.v = "-26w   0h" } }, // ←
	{ Mod4Mask|ShiftMask,           XK_k,      floatpos,       {.v = "800W 800H" } }, // ·
	{ Mod4Mask|ShiftMask,           XK_l,      floatpos,       {.v = " 26w   0h" } }, // →
	{ Mod4Mask|ShiftMask,           XK_m,      floatpos,       {.v = "-26w  26h" } }, // ↙
	{ Mod4Mask|ShiftMask,           XK_comma,  floatpos,       {.v = "  0w  26h" } }, // ↓
	{ Mod4Mask|ShiftMask,           XK_period, floatpos,       {.v = " 26w  26h" } }, // ↘
	/* Client is positioned in a floating grid, movement is relative to client's current position */
	{ Mod4Mask|Mod1Mask,            XK_u,      floatpos,       {.v = "-1p -1p" } }, // ↖
	{ Mod4Mask|Mod1Mask,            XK_i,      floatpos,       {.v = " 0p -1p" } }, // ↑
	{ Mod4Mask|Mod1Mask,            XK_o,      floatpos,       {.v = " 1p -1p" } }, // ↗
	{ Mod4Mask|Mod1Mask,            XK_j,      floatpos,       {.v = "-1p  0p" } }, // ←
	{ Mod4Mask|Mod1Mask,            XK_k,      floatpos,       {.v = " 0p  0p" } }, // ·
	{ Mod4Mask|Mod1Mask,            XK_l,      floatpos,       {.v = " 1p  0p" } }, // →
	{ Mod4Mask|Mod1Mask,            XK_m,      floatpos,       {.v = "-1p  1p" } }, // ↙
	{ Mod4Mask|Mod1Mask,            XK_comma,  floatpos,       {.v = " 0p  1p" } }, // ↓
	{ Mod4Mask|Mod1Mask,            XK_period, floatpos,       {.v = " 1p  1p" } }, // ↘
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
	{ MODKEY,                       XK_minus, scratchpad_show, {0} },
	{ MODKEY|ShiftMask,             XK_minus, scratchpad_hide, {0} },
	{ MODKEY,                       XK_equal,scratchpad_remove,{0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button1,        sigdwmblocks,   {.i = 1} },
	{ ClkStatusText,        0,              Button2,        sigdwmblocks,   {.i = 2} },
	{ ClkStatusText,        0,              Button3,        sigdwmblocks,   {.i = 3} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

