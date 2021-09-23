#include <windows.h> // підключення бібліотеки з функціями API
#include <string> // підключення бібліотеки строк
#include <fstream> // підключення бібліотеки строк
#include <vector> 
#include <ctime> 
#include "resource.h"



// Глобальні змінні:
HINSTANCE hInst; 	//Дескриптор програми	
LPCTSTR szWindowClass = "Rykov";
LPCTSTR szTitle = " ";

// Попередній опис функцій

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//Створення вспливаючого вікна
LRESULT CALLBACK WndWL(HWND, UINT, WPARAM, LPARAM);
ATOM RegisterClassChild(HINSTANCE hInstance);
LPCTSTR WinClassWinName = ("ChildClass");
HWND hWL;
HWND hWnd;

// Основна програма 

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG msg;
	// Реєстрація класу вікна 
	MyRegisterClass(hInstance);
	RegisterClassChild(hInstance);
	// Створення вікна програми
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	// Цикл обробки повідомлень
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE; 		//стиль вікна
	wcex.lpfnWndProc = (WNDPROC)WndProc; 		//віконна процедура
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; 			//дескриптор програми
	wcex.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1)); 		//визначення іконки
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW); 	//визначення курсору
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW); //установка фону
	wcex.lpszMenuName = NULL; 				//визначення меню
	wcex.lpszClassName = szWindowClass; 		//ім’я класу
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex); 			//реєстрація класу вікна
}
//Реєструємо вспливаюче вікно
ATOM RegisterClassChild(HINSTANCE hInstance)
{
	WNDCLASSEX wch = { 0 };
	// Определение класса окна
	wch.cbSize = sizeof(WNDCLASSEX);
	wch.style = CS_HREDRAW | CS_VREDRAW ;
	wch.lpfnWndProc = WndWL;
	wch.hInstance = hInst;
	wch.hCursor = LoadCursor(NULL, IDC_CROSS);
	wch.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(200, 200, 240));
	wch.lpszClassName = WinClassWinName;
	wch.hIconSm = LoadIcon(hInst, IDI_ASTERISK);
	return RegisterClassEx(&wch);
}

// FUNCTION: InitInstance (HANDLE, int)
// Створює вікно програми і зберігає дескриптор програми в змінній hInst

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; //зберігає дескриптор додатка в змінній hInst
	hWnd = CreateWindow(szWindowClass, 	// ім’я класу вікна
		NULL, 				// назва програми
		WS_CAPTION ,			// стиль вікна
		600, 			// положення по Х	
		400,			// положення по Y	
		750, 			// розмір по Х
		400, 			// розмір по Y
		NULL, 					// дескриптор батьківського вікна	
		NULL, 					// дескриптор меню вікна
		hInstance, 				// дескриптор програми
		NULL); 				// параметри створення.

	if (!hWnd) 	//Якщо вікно не творилось, функція повертає FALSE
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow); 		//Показати вікно
	UpdateWindow(hWnd); 				//Оновити вікно
	return TRUE;
}

//Структура для кнопок
struct CONTROLS
{
	HWND hControl; //Хендл дочірного вікна
	int id;	//ітидефікатор вікна (його айді) 
};
const int buttonsinprogramm = 2; //Кількість кнопок у програмі
CONTROLS myControls[buttonsinprogramm]; // Маси полів

//Робота із файлами
static OPENFILENAME WordTuple;//Структура із параметрами файлу
static TCHAR FilePath[256] ;//Шлях файлу
static TCHAR FileName[50] ;//НазваСаомогофайлу

static OPENFILENAME ScoreFile;//Структура із параметрами файлу
static TCHAR ScoreFileName[256];//Шлях файлу
static HANDLE hFile;//Хендл файлу
static DWORD nCharRead = 500;//Покажчик на змінну, яка отримує число записаних байтів

static std::ifstream in; //Для зчитування
static std::string  WordFileBuff;//Буфер для зчитування(для слів)
static std::vector <std::string> AllWordFormFile;//Вектор слів
static char ScoreFileBuff[500];//Вектор слів

static bool guess_the_word;//Потрібно для вікна-дитина. Перевірка на виграш / поразку
int score = 0;//Підрахунок результату
static char word[35];//Слово, яке ми вгадуємо

static bool StartGame = true;//Перевірка на початок гри

//Поля виводу
RECT gallowrect;//Шибиниця
RECT editrect; //Розташовуня окна редагування
RECT textrect;//Поле виводу тексту
RECT scorerect;//Поле виводу score
RECT mistakerect[8];//Поля помилок
int mistakes = 0;//Підрахунок помилок

//Функція створення новго слова
void ResetWord(char outtext[], char word[], bool IsNewFile)
{	
	//Умова почтку гри
	if (StartGame)
	{	
		EnableWindow(myControls[0].hControl, 0);
		AllWordFormFile.push_back("Выберите словарь");
		for (int i = 0; i < strlen(AllWordFormFile[0].c_str()); i++)
			outtext[i] = AllWordFormFile[0][i];
		return;
	}

	if (IsNewFile)//Перезаписуємо масив, якщо юзер зменив набір слів
	{
		std::srand(std::time(0));
		//Рандомізуємо
		in.open(FilePath);//Відкриваємо файл на читання
		if (in.is_open()) //Перевірка "Вдалого відкриття файлу"
		{
				AllWordFormFile.clear();//Очищаємо 
				while (!in.eof())
				{
					getline(in, WordFileBuff);
					AllWordFormFile.push_back(WordFileBuff);//Записуємо
				}
		}
		in.close();//Закриваємо файл
	}


	int random = rand() % AllWordFormFile.size();//Вибираємо нове слово
	
	//Очищуємо буфери
	for (int i = 0; i < 35; i++)
	{
		outtext[i] = '\0';
		word[i] = '\0';
	}

	//Зберігаємо слово
	for (int i = 0; i < AllWordFormFile[random].size(); i++)
		word[i] = AllWordFormFile[random][i];

	//Виводимо слово у потрібному руслі. В певному буфері
	for (int i = 0; i < strlen(word); i++)
	{
		if ((i > 0 && i < strlen(word) - 1) && word[i] != ' ')
		{
			if (word[i] == word[strlen(word) - 1] || word[0] == word[i])
				outtext[i] = word[i];
			else
				outtext[i] = '-';
		}
		else
			outtext[i] = word[i];
	}
}

//Функція обробки виграшу/поразки
void ResetGame(HWND hWnd, char outtext[], char word[], bool const win, int& score)
{
	guess_the_word = NULL;
	EnableWindow(hWnd, 0);

	if (win)//Обробка виграшу
	{
		guess_the_word = true;

		hWL = CreateWindow(WinClassWinName, "You WIN <(^--^)>",
			WS_SYSMENU | WS_EX_TOPMOST | WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION,
			830, 500, 300, 200, hWnd, 0, hInst, NULL);
		ResetWord(outtext, word, 0);

	}
	if (!win)//Обробка поразки
	{
		//Вікно поразки
		guess_the_word = false;

		hWL = CreateWindow(WinClassWinName, "You Lose (>___<)",
			WS_SYSMENU | WS_EX_TOPMOST | WS_POPUPWINDOW | WS_VISIBLE |  WS_CAPTION,
			830, 500, 300, 200, hWnd, 0, hInst, word);
		ResetWord(outtext, word, 0);
		score = 0;

	}
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	PAINTSTRUCT ps;

	static HPEN GallowPen, MyBlackPen;//Пера для малювання
	static HFONT MainFont, TitleFont, EditFont, MistakeFont, ScoreFont;//Шрифти
	static HBRUSH TextBg;//Кисть

	//Бітмапи основного меню
	static HBITMAP GallowBG,Background, Door, D_Door, Question, D_Question, D_Dowload, Dowload, Upload, D_Upload, Theme, D_Theme;
	static BITMAP bm; //дескриптор загруженого зображення
	static HDC memBit; //контекст пристрою
	
	static const int MenuButton = 5; //Кількість кнопок у меню
	static int xPos, yPos; // Позиція миші

	//Поля виводу
	static RECT menurectangles[MenuButton];//Позиція меню
	static RECT title;//Шакпка програми
	static RECT categoryrect;//Поле виводу категорії

	//Буфери
	static char buff[2];//Буфер для зберегання літер з дочірнього вікна "edit"
	static char MistakeMass[8];//Буфер для зберігання помилок
	static char textbuff[35];//Буфер для тексту
	static char outtext[35];//Відображаємий текст
	static char filenamebuff[35];//Буфер назви файлу
	static char scorebuff[5];//Буфер для підрахунку

	static bool SymInWord;//Відображаємий текст
	switch (message)
	{
	//Обробка не клієнтською області вікна
	case WM_NCCALCSIZE:
		break;

		//Пересування миші
	case WM_MOUSEMOVE:
		{
		xPos = LOWORD(lParam);
		yPos = HIWORD(lParam);
		hdc = GetDC(hWnd);
		/*Функціонал меню (зміна бітмапів при наведенні)*/
		for (int i = 0; i < MenuButton; i++)
		{
			if (PtInRect(&menurectangles[i], { xPos, yPos }))
			{
				switch (i)
				{
				case 0:
					SelectObject(memBit, D_Dowload);
					break;

				case 1:
					SelectObject(memBit, D_Upload);
					break;

				case 2:
					SelectObject(memBit, D_Theme);
					break;

				case 3:
					SelectObject(memBit, D_Question);
					break;

				case 4:
					SelectObject(memBit, D_Door);
					break;
				}
				BitBlt(hdc, menurectangles[i].left + 1, menurectangles[i].top + 1, bm.bmWidth, bm.bmHeight, memBit, 0, 0, SRCCOPY);
			}
			else
			{
				InvalidateRect(hWnd, &menurectangles[i], 0);
			}
		}
		ReleaseDC(hWnd, hdc);
		}
		break;

		//Повідомлення приходить при створенні вікна
	case WM_CREATE: 				
		{
		//Очистка структури фийлу
		ZeroMemory(&WordTuple, sizeof(WordTuple));
		//Заповнення стурктури зчитумуєго файлу
		WordTuple.lStructSize = sizeof(OPENFILENAME);
		WordTuple.hInstance = hInst;
		WordTuple.lpstrFilter = ("Text\0*.txt");
		WordTuple.lpstrFile = FilePath;
		WordTuple.lpstrFileTitle = FileName;
		WordTuple.nMaxFileTitle = 50;
		WordTuple.nMaxFile = 256;
		WordTuple.lpstrInitialDir = (".\\");
		WordTuple.lpstrDefExt = ("txt");

		//Очистка структури фийлу
		ZeroMemory(&ScoreFile, sizeof(ScoreFile));
		//Заповнення стурктури файлу очків
		ScoreFile.lStructSize = sizeof(OPENFILENAME);
		ScoreFile.hInstance = hInst;
		ScoreFile.lpstrFilter = ("Score\0*.score");
		ScoreFile.lpstrFile = ScoreFileName;
		ScoreFile.nMaxFile = 256;
		ScoreFile.lpstrInitialDir = (".\\");
		ScoreFile.lpstrDefExt = ("score");

		//Створення дочірніх кнопок
			//Присвоєння для кожної своє айді
		for (int i = 0; i < buttonsinprogramm; i++)
			myControls[i].id = i;

		//Поле редагування
		myControls[0].hControl = CreateWindow(
			"edit", "",
			WS_CHILD | WS_VISIBLE | ES_CENTER | ES_MULTILINE,
			255, 250, 400, 100,
			hWnd, (HMENU)myControls[0].id,
			hInst, NULL);

		//Кнопка для перевірки
		myControls[1].hControl = CreateWindow(
			"button", "Перевірити",
			BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
			255, 350, 402, 40,
			hWnd, (HMENU)myControls[1].id,
			hInst, NULL);

		//Створюємо шрифт
		MainFont = CreateFont(55, 18, 0, 0, 500, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Times New Roman");
		MistakeFont = CreateFont(40, 14, 0, 0, 500, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Avenir Next LT Pro");
		TitleFont = CreateFont(50, 20, 0, 0, 1200, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Monotype Corsiva");
		EditFont = CreateFont(100, 40, 0, 0, 700, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe Print");
		ScoreFont = CreateFont(43, 17, 0, 0, 500, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Times New Roman");
		//Створюємо бітмапи
		//Фон
		Background = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP11));

		//Вихід
		D_Door = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		Door = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));

		//Правила
		Question = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3));
		D_Question = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4));

		//Тема
		Theme = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP5));
		D_Theme = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP6));

		//Збереження результату
		Dowload = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP7));
		D_Dowload = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP8));

		//Завантеження результату
		Upload = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP9));
		D_Upload = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP10));

		//Шибиниця
		GallowBG = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP12));
		GallowPen = CreatePen(BS_SOLID, 6, (COLORREF)RGB(0, 250, 250));
		MyBlackPen = CreatePen(BS_SOLID, 10, (COLORREF)RGB(0, 0, 0));
		gallowrect = { 10, 10, 240, 390 };//Поле виводу

		//Створюмо контекст виводу для бітмап
		memBit = CreateCompatibleDC(hdc);

		//Встаномлюэмо шрифт у вiкнi edit
		SendMessage(myControls[0].hControl, WM_SETFONT, (WPARAM)EditFont, 0);

		//Поля іконків меню
		for (int i = 0; i < MenuButton; i++)
			menurectangles[i] = { 670, 0 + 80 * i, 750, 80 + 80 * i };

		//Вивід помилок
		for (int i = 0; i < 8; i++)
			mistakerect[i] = { 260 + i * 50, 200, 300 + i * 50, 240 };

		//Поле виводу назви
		title = { 280,0,620,40 };

		//Поле виводу очкків
		scorerect = { 250, 85, 660, 140 };

		//Поле виводу категорії
		categoryrect = { 250, 45, 660, 110 };

		//Поле виводу тексту
		textrect = { 250, 130, 660, 190 };
		TextBg = CreateHatchBrush(HS_CROSS, RGB(0, 200, 200));//Фон

		//Поле виводу окна редагування
		editrect = { 255, 250, 655, 350 };

		//Створення слова
		ResetWord(outtext, word, 0);
		//Іконка додатку
		LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
		SetClassLong(hWnd, GCL_HICON, (LONG)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)));
		}
		break;

		//Перемалювати вікно
	case WM_PAINT: 				
		{
		hdc = BeginPaint(hWnd, &ps); 	//Почати графічний вивід	

		//Фон
		GetObject(Background, sizeof(bm), &bm);//Отримуємо дані бітмапу
		SelectObject(memBit, Background);//Вибір бітмап
		BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, memBit, 0, 0, SRCCOPY);//Вивід бітмапу

		//Вивід тексту
		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, MainFont);
		SelectObject(hdc, TextBg);
		FillRect(hdc, &textrect, TextBg);
		FrameRect(hdc, &textrect, HBRUSH(GetStockObject(BLACK_BRUSH)));
		DrawText(hdc, textbuff, wsprintf(textbuff, "%s", outtext), &textrect, DT_CENTER | DT_WORD_ELLIPSIS | DT_SINGLELINE);

		//Підрахунок результату
		SelectObject(hdc, ScoreFont);
		SetTextColor(hdc, RGB(100, 250, 200));
		DrawText(hdc, scorebuff, wsprintf(scorebuff, "Your Score: %d", score), &scorerect, DT_LEFT | DT_SINGLELINE | DT_WORD_ELLIPSIS);

		//Виводимо категорію
		DrawText(hdc, filenamebuff, wsprintf(filenamebuff, "Category: %s", FileName), &categoryrect, DT_LEFT | DT_SINGLELINE | DT_WORD_ELLIPSIS);

		//Вивід помилок
		SelectObject(hdc, GetStockObject(GRAY_BRUSH));
		SelectObject(hdc, (GallowPen));
		SelectObject(hdc, (MistakeFont));
		SetTextColor(hdc, COLORREF(RGB(250, 250, 250)));
		for (int i = 0; i < mistakes; i++)
		{
			Rectangle(hdc, mistakerect[i].left, mistakerect[i].top, mistakerect[i].right, mistakerect[i].bottom);
			if (MistakeMass[i] != NULL)
			{
				//Переводимо малі літери в великі
				if ((int('а') <= MistakeMass[i] && MistakeMass[i] <= int('я')))
					MistakeMass[i] -= 32;

				DrawText(hdc, (LPCSTR)&MistakeMass[i], 1, &mistakerect[i], DT_CENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			}
		}
	
		//Малюємо шибиницю
		GetObject(GallowBG, sizeof(bm), &bm);
		SelectObject(memBit, GallowBG);//Вибір бітмап
		SelectObject(hdc, (HBRUSH)GetStockObject(BLACK_BRUSH));
		SelectObject(hdc, (HPEN)GetStockObject(BLACK_PEN));

		Rectangle(hdc, 10, 10, 240, 390);//Рамка
		StretchBlt(hdc, 15, 15, 220, 370, memBit, 10, 10, 220, 370, SRCCOPY);//Вивід бітмапу

		SelectObject(hdc, GallowPen);

		for (int i = 0; i < mistakes; i++)
		{
			//Малюємо шибиницю
			switch (i)
			{
			case 0:
				MoveToEx(hdc, 200, 360, 0);
				LineTo(hdc, 200, 200);
				break;
			case 1://Верт. палку
				MoveToEx(hdc, 225, 200, 0);
				LineTo(hdc, 120, 200);
				break;
			case 2://Гори
				MoveToEx(hdc, 140, 200, 0);
				LineTo(hdc, 140, 230);
				break;
			case 3://Петлю
				SelectObject(hdc, GetStockObject(NULL_BRUSH));
				SelectObject(hdc, (MyBlackPen));
				Ellipse(hdc, 125, 230, 155, 280);
				break;
			case 4://Голову
				SelectObject(hdc, GetStockObject(GRAY_BRUSH));
				SelectObject(hdc, (GallowPen));
				Ellipse(hdc, 120, 230, 160, 270);

				MoveToEx(hdc, 135, 240, 0);
				LineTo(hdc, 135, 250);

				MoveToEx(hdc, 145, 240, 0);
				LineTo(hdc, 145, 250);

				MoveToEx(hdc, 135, 258, 0);
				LineTo(hdc, 145, 258);

				break;
			case 5://Тіло
				Ellipse(hdc, 130, 270, 150, 330);

				SelectObject(hdc, (MyBlackPen));
				Arc(hdc, 125, 230, 155, 280, 130, 270, 155, 280);
				break;
			case 6://Руки
				SelectObject(hdc, (GallowPen));

				MoveToEx(hdc, 130, 290, 0);
				LineTo(hdc, 120, 270);

				MoveToEx(hdc, 150, 290, 0);
				LineTo(hdc, 160, 270);
				break;
			case 7://Ноги
				MoveToEx(hdc, 130, 330, 0);
				LineTo(hdc, 120, 350);

				MoveToEx(hdc, 150, 330, 0);
				LineTo(hdc, 160, 350);

				ResetGame(hWnd, outtext, word, false, score);

				break;
			}
		}
		//Назва
		SetTextColor(hdc, RGB(0, 250, 250));
		SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, TitleFont);
		TextOut(hdc, 340, -8, (LPCSTR)"Шибиниця", 8);

		//Малюэмо меню 
		GetObject(Door, sizeof(bm), &bm);
		SelectObject(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
		SetDCPenColor(hdc, RGB(0, 250, 250));
		SelectObject(hdc, (HBRUSH)GetStockObject(DC_PEN));

		for (int i = 0; i < MenuButton; i++)
		{
			Rectangle(hdc, menurectangles[i].left, menurectangles[i].top, menurectangles[i].left + 80, menurectangles[i].top + 80);
			switch (i)
			{
			case 0:
				SelectObject(memBit, Dowload);
				break;
			case 1:
				SelectObject(memBit, Upload);
				break;
			case 2:
				SelectObject(memBit, Theme);
				break;
			case 3:
				SelectObject(memBit, Question);
				break;
			case 4:
				SelectObject(memBit, Door);
				break;
			}
			BitBlt(hdc, menurectangles[i].left + 1, menurectangles[i].top + 1, bm.bmWidth, bm.bmHeight, memBit, 0, 0, SRCCOPY);
		}

		//Умова виграшу
		if (strcmp(outtext, word) == 0 && StartGame == false)
			ResetGame(hWnd, outtext, word, true, score);

		EndPaint(hWnd, &ps); 		//Закінчити графічний вивід	
		}
		break;

		//Обробка дочірніх вікон
	case WM_COMMAND:
		{
		switch (LOWORD(wParam))
		{
		case 0:
		case EN_SETFOCUS:
			if (GetAsyncKeyState(VK_RETURN))
				SendMessage(hWnd, WM_COMMAND, 1, 0);
			break;
		break;
		case 1://Обробка кнопки "Перевірка"

			//Зчитуємо символ у буфер
			GetWindowText((HWND)myControls[0].hControl, buff, 2);

			//Перевід у велику літеру 
			if ((int('а') <= *buff && *buff <= int('я')))
				*buff -= 32;

			//Перевірка на те, що введена літера. Та вона не повторюється
			for (int i = 0; i < mistakes; i++)
			{
				if ((MistakeMass[i] == *buff))
				{
					return 0;
				}
			}

			//Перевірка на наявності літери у слові
			for (int i = 0; i < strlen(word); i++)
			{
				if (outtext[i] == *buff)
					return 0;

				if (word[i] == *buff)
				{
					SymInWord = true;
					break;
				}
				else
					SymInWord = false;
			}

			//Умови помилки та вгадування
			if (SymInWord == true) //Вгадування
			{
				for (int i = 0; i < strlen(word); i++)
				{
					if (word[i] == *buff)
					{
						if (outtext[i] == '-')
							score += 10;

						outtext[i] = word[i];
					}
				}
				InvalidateRect(hWnd, &textrect, 0);
				InvalidateRect(hWnd, &scorerect, 0);
				//Очищуємо ввод
				SendMessage((HWND)myControls[0].hControl, EM_SETSEL, 0, -1);
				SendMessage((HWND)myControls[0].hControl, WM_CLEAR, 0, 0);
				SetFocus(HWND(myControls[0].hControl));

			}
			if (SymInWord == false) // Помилка
			{
				//Якщо буква відсутня у слові
				if ((*buff >= 'а' && *buff <= 'я') || (*buff >= 'А' && *buff <= 'Я'))
					MistakeMass[mistakes++] = *buff;

				//Оновлюємо вікна
				InvalidateRect(hWnd, &gallowrect, 0);
				for (int i = 0; i < mistakes; i++)
					InvalidateRect(hWnd, &mistakerect[i], 0);
				SetFocus(HWND(myControls[0].hControl));
				//Знову встановлюємо фокус
			}
			break;
			}
		}
		break;

	case WM_CTLCOLORBTN:
		if ((HWND)lParam == GetDlgItem(hWnd, myControls[1].id)) {
			SelectObject(HDC(wParam), TitleFont);
		}
		break;

		//Параметри edit
	case WM_CTLCOLOREDIT:
		{
			// Ограничение количества вводимых символов
			SendMessage((HWND)lParam, EM_LIMITTEXT, 1, 0); 
			if ( (HWND)lParam == GetDlgItem(hWnd, myControls[0].id)) {
				SetTextColor((HDC)wParam, RGB(65, 250, 225));
				SetBkColor((HDC)wParam, RGB(0, 100, 250));
				//HideCaret((HWND)myControls[0].id);
				return 0;
			}
		}
		break;

		//Функціональність меню
	case WM_LBUTTONUP:
		{
		for (int i = 0; i < MenuButton; i++)
		{
			if (StartGame == true && (PtInRect(&menurectangles[0], { xPos, yPos }) || PtInRect(&menurectangles[1], { xPos, yPos })))
			{
				MessageBox(hWnd,"Ви не можете зберігати та завантажувати гру, поки не вбрали словар слів\n3-ій пункт меню","Попередження",MB_OK);
				break;
			}
			if (PtInRect(&menurectangles[i], { xPos, yPos }))
			{
				switch (i)
				{
				case 0://Збереження гри

					EnableWindow(hWnd, false);
					ScoreFile.lpstrTitle = ("Открыть файл для записи");
					ScoreFile.Flags = OFN_NOTESTFILECREATE;
					if (GetSaveFileName(&ScoreFile) == TRUE)
					{
						//Створюємо буфер запису
						//Очистка буфера
						for (int i = 0; i < 500; i++)
							ScoreFileBuff[i] = '\0';

						//Загадане слово
						for (int i = 0; i < strlen(word); i++)
							ScoreFileBuff[i] = word[i];
						ScoreFileBuff[strlen(word)] = '\n';

						//Очки
						for (int koef = 100000, i = strlen(word) + 1; i < strlen(word) + 6; i++, koef /= 10)
							ScoreFileBuff[i] = (score) % koef / (koef / 10) + '0';
						ScoreFileBuff[strlen(word) + 6] = '\n';

						//Помилки
						for (int i = strlen(word) + 7; i < strlen(word) + mistakes + 7; i++)
							ScoreFileBuff[i] = MistakeMass[i - strlen(word) - 7];
						ScoreFileBuff[strlen(word) + mistakes + 7] = '\n';

						//Що вгадано зараз
						for (int i = strlen(word) + 8 + mistakes; i < strlen(word) + mistakes + 8 + strlen(outtext); i++)
							ScoreFileBuff[i] = outtext[i - strlen(word) - mistakes - 8];

						hFile = CreateFile(ScoreFileName, GENERIC_WRITE, 0, NULL,
							CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						WriteFile(hFile, ScoreFileBuff, strlen(ScoreFileBuff), &nCharRead, NULL);
						CloseHandle(hFile);
					}
					ShowWindow(hWnd, SW_SHOWNA);
					EnableWindow(hWnd, 1);
					break;

				case 1://Зчитування гри
					EnableWindow(hWnd, false);

					ScoreFile.lpstrTitle = ("Открыть файл для чтения");
					ScoreFile.Flags = OFN_HIDEREADONLY;
					if (GetOpenFileName(&ScoreFile) == TRUE)
					{
						//Очищуэммо буфер
						for (int i = 0; i < 500; i++)
							ScoreFileBuff[i] = '\0';
						hFile = CreateFile(ScoreFileName, GENERIC_READ, 0, NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						ReadFile(hFile, ScoreFileBuff, 500, &nCharRead, NULL);
						CloseHandle(hFile);

						//Оновлюєммо данні
						score = 0;
						mistakes = 0;
						for (int i = 0; i < 35; i++)
						{
							outtext[i] = '\0';
							word[i] = '\0';
						}

						//Розбиваємо буфер на потрібні данні 
						for (int koef = 10,f = 0,pos = 0, i = 0; i < strlen(ScoreFileBuff); i++, f++)
						{
							if (ScoreFileBuff[i] == '\n')
							{
								pos++; f = 0; i++;
							}
							switch (pos)
							{
							case 0:word[f] = ScoreFileBuff[i]; //слово
								break;
							case 1:score = atoi(&ScoreFileBuff[i]); i += 4;//Рахунок
								break;
							case 2:
								if (ScoreFileBuff[i] == '\n')
								{
									pos++; f = -1;
								}
								else
								{
									MistakeMass[f] = ScoreFileBuff[i];//Помилки
									mistakes++;
								}
								break;
							case 3:outtext[f] = ScoreFileBuff[i];//стан слова
								break;
							}
						}
						//Оновлюємо поле
						InvalidateRect(hWnd, &scorerect, 0);//Слова
						InvalidateRect(hWnd, &gallowrect, 1);
						InvalidateRect(hWnd, &textrect, 1);
						for (int i = 0; i < 8; i++)
							InvalidateRect(hWnd, &mistakerect[i], 1);
					}
					EnableWindow(hWnd, 1);
					ShowWindow(hWnd, SW_SHOWNA);
					break;
				case 2://Зміння словника 
					EnableWindow(hWnd, false);
					WordTuple.lpstrTitle = ("Открыть файл для чтения");
					WordTuple.Flags = OFN_HIDEREADONLY;

					//if (!StartGame)
					//	MessageBox(hWnd, "УВАГА! \nПоточна гра буди онволена. Збережіть гру", "Увага!!!", MB_OK);

					if (GetOpenFileName(&WordTuple) == TRUE)//Відкриваємо файл
					{
						//Гра роспочалася
						if (StartGame)
						{
							EnableWindow(myControls[0].hControl, 1);
							StartGame = false;
						}
						

						ResetWord(outtext, word, 1);
						score = 0;//Онволюємо результат

						//Преводимо назву файлу у потрібний вид
						for (int i = strlen(FileName); i >= 0; i--)
						{
							if (FileName[i] == '.')
							{
								FileName[i] = '\0';
								break;
							}
							FileName[i] = '\0';
						}

						InvalidateRect(hWnd, &scorerect, 0);//Слова
						InvalidateRect(hWnd, &gallowrect, 1);
						InvalidateRect(hWnd, &textrect, 1);
						InvalidateRect(hWnd, &categoryrect, 1);
						for (int i = 0; i < 8; i++)
							InvalidateRect(hWnd, &mistakerect[i], 1);
						mistakes = 0;
					}
					EnableWindow(hWnd, 1);
					ShowWindow(hWnd, SW_SHOWNA);
					break;
				case 3://інфо
					MessageBox(hWnd, "Шибениця - гра про відгалування слів. \nІгроку загадується слово, яке він повинен вгадати, використовуючи букви алфавіту і можливість зробити обмежену кількість помилок(в данній реалізації тільки 8).\n\nМеню:\n1. Зберегти гру\n2. Загрузити гру\n3. Вибрати набір слів для гри\n4. Інформація\n5. Вихід \n\nСтворена: студентом групи КІУКІ-19-7, Риковом Владиславом ", "Довідка", MB_OK);
					break;
				case 4://Вихід
					SendMessage(hWnd, WM_DESTROY, 0, 0);
					break;
				}
			}
		}
		}
		break;

		//Переміщення вікна
	case WM_LBUTTONDOWN:
		{
		//Переміщення вікна рпи зажаття миші
			if (PtInRect(&title, { xPos, yPos }))
				SendMessage(hWnd, WM_SYSCOMMAND, SC_SIZE | HTMAXBUTTON, 0);
		//Убирання фокуусу з "edit", якщо клацаємо поза окном
			if (!PtInRect(&editrect, { xPos, yPos }))
			{
				SetFocus(hWnd);
				SendMessage((HWND)myControls[0].hControl, EM_SETSEL, 0, -1);
				SendMessage((HWND)myControls[0].hControl, WM_CLEAR, 0, 0);
			}
		}
		break;

		//Горячі клавіші
	case WM_KEYDOWN:
		{
		switch (wParam)
		{
		case VK_F1:
			MessageBox(hWnd, "Шибениця - гра про відгалування слів. \nІгроку загадується слово, яке він повинен вгадати, використовуючи букви алфавіту і можливість зробити обмежену кількість помилок(в данній реалізації тільки 8).\n\nМеню:\n1. Зберегти гру\n2. Загрузити гру\n3. Вибрати набір слів для гри\n4. Інформація\n5. Вихід \n\nСтворена: студентом групи КІУКІ-19-7, Риковом Владиславом ", "Довідка", MB_OK);
			break;
		case VK_ESCAPE:
			SendMessage(hWnd, WM_DESTROY, 0, 0);
			break;
		}
		}
		break;

		//Завершення роботи та видалення об'єктів із пам'яті
	case WM_DESTROY: 
		{
		DeleteObject(memBit);
		DeleteObject(&bm);
		//Бітмапів
		DeleteObject(Background);
		DeleteObject(GallowBG);
		DeleteObject(Door);
		DeleteObject(D_Door);
		DeleteObject(Question);
		DeleteObject(D_Question);
		DeleteObject(Dowload);
		DeleteObject(D_Dowload);
		DeleteObject(Upload);
		DeleteObject(D_Upload);
		DeleteObject(Theme);
		DeleteObject(D_Theme);
		//Шрифтів
		DeleteObject(MainFont);
		DeleteObject(TitleFont);
		DeleteObject(EditFont);
		DeleteObject(MistakeFont);
		//Кисть / перо
		DeleteObject(GallowPen);
		DeleteObject(MyBlackPen);
		DeleteObject(TextBg);
		//Файлів
		DeleteObject(&ScoreFile);
		DeleteObject(&WordTuple);
		PostQuitMessage(0);			
		}
		break;
		
	default:
		//Обробка повідомлень, які не оброблені користувачем
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//Вікно яке виводиться при виграші чи проіграші. 
LRESULT CALLBACK WndWL(HWND hWL, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	static RECT rt;
	static int sx, sy;
	static char buff[35];//Буфер для слова
	static char buff1[200];//Буфер для висловів
	static HFONT MainFont1;//Шрифт виводу
	static HWND butt;//Кнопка
	switch (message)
	{
	case WM_CREATE: 				//Повідомлення приходить при створенні вікна
		{
		SetFocus(hWL);//Встановлюємо фокус
		for (int i = 0; i < 35; i++)//Заповнюємо буфер
			buff[i] = word[i];

		//Створюємо шрифт
		MainFont1 = CreateFont(30, 11, 0, 0, 700, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Times New Roman");

		//Створюєммо кнопку
		butt = CreateWindow(
			"button", "OK",
			BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
			110, 110, 60, 30,
			hWL, (HMENU)5,
			hInst, NULL);
		rt = { 10,10,270,200 };//Поле виводу
		}
		break;

		//Обробка нажаття клавіш
	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_RETURN:
			SendMessage(hWL, WM_CLOSE, 0, 0);//Посилаємо вікну функцію завершення роботи
			break;
		case VK_ESCAPE:
			SendMessage(hWL, WM_CLOSE, 0, 0);
			break;
		}
		break;

		//Обробка кнопок
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case 5:
			SendMessage(hWL, WM_CLOSE, 0, 0);
			break;
		}
		break;

	case WM_PAINT: 				//Перемалювати вікно
		{
		hdc = BeginPaint(hWL, &ps); 	//Почати графічний вивід	
		SelectObject(hdc, MainFont1);
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(0, 0, 0));
		if (guess_the_word)
			DrawText(hdc, buff1, wsprintf(buff1, "Ви вгадали слово :)\nНа вашому рахунку \n%d очків", score), &rt, DT_CENTER | DT_VCENTER | DT_WORD_ELLIPSIS);
		else
			DrawText(hdc, buff1, wsprintf(buff1, "Ви програли :(\nЗагадане слово:\n| %s |", buff), &rt, DT_CENTER | DT_VCENTER | DT_WORD_ELLIPSIS);
		EndPaint(hWL, &ps); 		//Закінчити графічний вивід	
		}
		break;

		//Дії при закритті вікна
	case WM_CLOSE:
		{
		ShowWindow(hWnd, SW_RESTORE);//Востановлюємо вікно
		EnableWindow(hWnd, 1);
		SetFocus(myControls[0].hControl);
		DestroyWindow(hWL);

		//Оновлюємо стандартне вікно
		for (int i = 0; i < mistakes; i++)//Помилки
			InvalidateRect(hWnd, &mistakerect[i], 0);
		//Оновлюємо данні
		InvalidateRect(hWnd, &gallowrect, 0);//Шибиницю
		InvalidateRect(hWnd, &textrect, 0);//Слова
		InvalidateRect(hWnd, &scorerect, 0);//Очки

		mistakes = 0;//Оновлюємо помилки

		//Очищуємо edit
		SendMessage((HWND)myControls[0].hControl, EM_SETSEL, 0, -1);
		SendMessage((HWND)myControls[0].hControl, WM_CLEAR, 0, 0);

		return 0;
		}
		break;

	default:
		//Обробка повідомлень, які не оброблені користувачем
		return DefWindowProc(hWL, message, wParam, lParam);
	}
	return 0;
}