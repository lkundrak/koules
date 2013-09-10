/***********************************************************
*                      K O U L E S                         *
*----------------------------------------------------------*
*  C1995 JAHUSOFT                                          *
*        Jan Hubicka                                       *
*        Dukelskych Bujuvniku 1944                         *
*        390 03 Tabor                                      *
*        Czech Republic                                    *
*        Phone: 0041-361-32613                             *
*        eMail: hubicka@limax.paru.cas.cz                  *
*----------------------------------------------------------*
*   Copyright(c)1995,1996 by Jan Hubicka.See README for    *
*                    licence details.			   *
*----------------------------------------------------------*
*  text.h starwars scroller text                           *
*  revised by Thomas Marsh				   *
***********************************************************/

#define KOULESLINE 11
#define PLAYERLINE 34
#define D1LINE 44
#define D2LINE 46
#define BLINE 57
static char    *text[] =
{
  "Somewhere in the galaxy",
  "...",
  "Once upon a time",
  "the evil",
  "KOULES",
  "exhausted all the resources",
  "of their home planet",
  "...",
  "Their emperor",
  "THE DARK APPLEPOLISHER",
  "deployed his brutal",
  "hyperspace commandos",
  "to wipe out",
  "a human civilization",
  "and claim Earth's",
  "resources for his",
  "realms"
  "...",
  "The nations of Earth",
  "chose their best warriors",
  "to defend our planet",
  "...",
  "These champions were",
  "biologically mutated",
  "into special creatures.",
  "Their entities were reduced",
  "to mere chests and eyes",
  "...",
  "These qualities allow them",
  "operate in enemies' domain",
  "...",
  "Your battle for a new world",
  "begins",
  "",
  "NOW",
  "...",
  "Your mission is to",
  "bounce all",
  "KOULES fighters",
  "from your sector",
  "...",
  "Some of them are",
  "deserters",
  "The green ones give you",
  "acceleration",
  "the blue ones give you",
  "more weight",
  "...",
  "Look out",
  "for",
  "mighty",
  "super-spherical",
  "megaheavy",
  "ultra green",
  "B_BALL",
  "...",
  "",
  "",
  "KOULES TEAM",
  "...",
  "Programming by Jan Hubicka",
  "...",
  "OS/2 version by Thomas A. K. Kjaer",
  "...",
  "Script by Kamil Toman",
  "and Jan Hubicka",
  "...",
  "Spelling check by Thomas Marsh",
  "...",
  "Graphics by Jan Hubicka",
  "and Kamil Toman",
  "...",
  "Sounds by Jan Hubicka",
  "...",
  "HP-UX sound by Lutz Vieweg",
  "...",
  "SGI sound by Rick Sayre",
  "...",
  "Story by Kamil Toman",
  "and Jan Hubicka",
  "...",
  "Sound server by",
  "Joe Rumsey",
  "and S. M. Patel",
  "...",
  "Joystick support by",
  "Ludvik Tesar",
  "...",
  "Greetings",
  "...",
  "Linus Torvalds",
  "Have a nice Linux",
  "...",
  "Ondrej Stasek",
  "alias KOTELNIK",
  "from RDOS",
  "Hello you ___",
  "nice man",
  "...",
  "Jaroslav Kysela",
  "debugged on your GUS driver",
  "...",
  "Mojmir Svoboda",
  "No chance to run it",
  "on your machine...",
  "It doesn't matter",
  "you've bought better",
  "computer equipment",
  "...",
  "Roman Zenka",
  "You lucky man",
  "you ARE in credits",
  "...",
  "Aragorn",
  "Thanks for an account",
  "...",
  "Jaroslav Pech",
  "When could I hope you learn",
  "Linux",
  "...",
  "Roman Dolejsi",
  "I hope your game",
  "will be better",
  "...",
  "Boris Letocha",
  "Write me in credits in",
  "your game",
  "...",
  "...",
  "HEY WAKE UP",
  "and",
  "press any key",
  "excluding",
  "power",
  "reset",
  "turbo",
#ifndef XSUPPORT
  "CTRL+C",
  "CTRL+Z",
  "ALT F1",
  "ALT F2",
  "ALT F3",
  "ALT F4",
  "ALT F5",
  "ALT F6",
  "ALT F7",
  "ALT F8",
  "ALT F9",
  "ALT F10",
  "ALT F11",
  "ALT F12",
  "ALT F13",
  "( huh )",
  "...",
  "ALT LEFT ARROW",
  "ALT RIGHT ARROW",
#else
#ifdef linux
  "CONTROL ALT F1",
  "CONTROL ALT F2",
  "CONTROL ALT F3",
  "CONTROL ALT F4",
  "CONTROL ALT F5",
  "CONTROL ALT F6",
  "CONTROL ALT F7",
  "CONTROL ALT F8",
  "CONTROL ALT F9",
  "CONTROL ALT F10",
  "CONTROL ALT F11",
  "CONTROL ALT F12",
  "CONTROL ALT F13",
  "( huh )",
  "...",
  "CONTROL ALT PLUS",
  "CONTROL ALT MINUS",
  "CONTROL ALT BACKSPACE",
#endif
#endif
  "ESC",
  "and some other",
  "keys",
  "( no key except A is safe )"
  "...",
  "Copyright(c)1995-1996",
  "Jan Hubicka",
  "hubicka@paru.cas.cz",
};
static int      TEXTSIZE = (sizeof (text) / sizeof (char *));

static char    *text1[] =
{
  "Very well",
  "...",
  "You have destroyed most",
  "of the Koules fighters",
  "...",
  "Now you are",
  "in the main base",
  "Your goal",
  "is to destroy the",
  "Mighty",
  "Mega",
  "Super",
  "Ultra",
  "HEAVY",
  "DARK APPLEPOLISHER"};
static int      TEXTSIZE1 = (sizeof (text1) / sizeof (char *));
static char    *text2[] =
{
  "Congratulations",
  "your mission is complete",
  "...",
  "The death of",
  "DARK APPLEPOLISHER",
  "caused the end",
  "of the whole",
  "KOULES civilization",
  "...",
  "Millions of",
  "innocent KOULES",
  "died in chaos",
  "caused by death",
  "of DARK APPLEPOLISHER",
  "...",
  "You have killed",
  "innocent KOULES",
  "You killer",
  "You murderer",
  "You...you..you..",
  "...",
  "And now",
  "Wait for",
  "The greatest",
  "Game",
  "For",
  "Linux",
  "...",
  "KOULES II",
  "...",
  ""};
static int      TEXTSIZE2 = (sizeof (text2) / sizeof (char *));
#define CONTLINE 16
#define UDIVLINE 10
static char    *bballtext[] =
{
  "The DARK APPLEPOLISHER",
  "has heard about you",
  "You are famous",
  "This has one disadvantage",
  "B_BALLS are going to",
  "kick you out",
  "of the game",
  "We believe in your",
  "abilities",
  "Yet no one had survived",
  "the ... meeting ...",
  "before"};
static int      BBALLSIZE = (sizeof (bballtext) / sizeof (char *));
static char    *bbballtext[] =
{
  "You entered the",
  "PRIMARY ALARM ZONE",
  "New B_BALLS have been",
  "sent to protect the leader",
  "...",
  "do not",
  "turn the power off",
  "you are near",
  "your goal",
  "NOW you can finish the game"};
static int      BBBALLSIZE = (sizeof (bbballtext) / sizeof (char *));
static char    *introtext[] =
{
  "The best way",
  "to play KOULES",
  "is to let you play",
  "without instructions",
  "...",
  "We can only advise you",
  "...",
  "(connection failed)"};
static int      INTROSIZE = (sizeof (introtext) / sizeof (char *));
static char    *holetext[] =
{
  "Our scientists have",
  "developed a great",
  "weapon",
  "so powerful",
  "that even you can",
  "be destroyed by it",
  "...",
  "They have designed",
  "special black holes",
  "which can be moved",
  "through the hyperspace",
  "Your advantage is",
  "that black holes cannot",
  "be seen by KOULES"};
static int      HOLESIZE = (sizeof (holetext) / sizeof (char *));
static char    *inspectortext[] =
{
  "Standard KOULES",
  "inspectors were sent",
  "to monitor your actions",
  "They can not be destroyed",
  "directly",
  "...",
  "Good luck",
  "..."};
static int      INSPECTORSIZE = (sizeof (inspectortext) / sizeof (char *));
static char    *magholetext[] =
{
  "KOULES are not stupid",
  "They have developed a new",
  "funny weapon",
  "which can be called",
  "the MAGNETIC HOLE",
  "..."};
static int      MAGSIZE = (sizeof (magholetext) / sizeof (char *));
static char    *springtext[] =
{
  "Koules have",
  "more hidden qualities",
  "...",
  "When they're in a danger",
  "they spit on you",
  "...",
  "their spits sticks to you",
  "...",
  "Do not puke on the keyboard",
  "..."};
static int      SPRINGTSIZE = (sizeof (springtext) / sizeof (char *));
static char    *thieftext[] =
{
  "During the war",
  "the Earth's power",
  "resources have been",
  "almost exhausted",
  "...",
  "An underground guerilla",
  "organisation called SUBWAY",
  "was established",
  "to take care",
  "of this problem",
  "...",
  "SUBWAY squads",
  "known as Thieves",
  "managed to infiltrate",
  "war teritories",
  "..."};

static int      THIEFSIZE = (sizeof (thieftext) / sizeof (char *));
static char    *findertext[] =
{
  "Their competitors,",
  "the on-the-ground",
  "organisation SURFACE",
  "developed a special",
  "persuaduing method",
  "which makes",
  "a Thief become",
  "a complete boob",
  "which returns to you",
  "everything he has stolen",
  "...",
  "Technical note",
  "This technology",
  "is based on UNIX",
  "lost+found directory"};

static int      FINDERSIZE = (sizeof (findertext) / sizeof (char *));
static char    *ttooltext[] =
{
  "After intensive",
  "research our scientists",
  "have finally stolen",
  "SUBWAY's technologies",
  "...",
  "If you get a special",
  "chip you will be able",
  "to steal energy",
  "from other ships or B_BALLS",
};
static int      TTOOLSIZE = (sizeof (ttooltext) / sizeof (char *));
static char    *lunatictext[] =
{
  " DANGER ",
  "Our scientists",
  "have been working",
  "on very dangerous",
  "virus",
  "...",
  "By a small mistake",
  "they left the",
  "laboratory",
  "open and have",
  "accidentally released",
  "the LUNATICS.",
  "...",
  "Beware, for Lunatics",
  "are extremely",
  "dangerous."
};
static int      LUNATICSIZE = (sizeof (lunatictext) / sizeof (char *));
