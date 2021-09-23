#include <windows.h> // ���������� �������� � ��������� API
#include <string> // ���������� �������� �����
#include <fstream> // ���������� �������� �����
#include <vector> 
#include <ctime> 
#include "resource.h"



// �������� ����:
HINSTANCE hInst; 	//���������� ��������	
LPCTSTR szWindowClass = "Rykov";
LPCTSTR szTitle = " ";

// ��������� ���� �������

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//��������� ������������ ����
LRESULT CALLBACK WndWL(HWND, UINT, WPARAM, LPARAM);
ATOM RegisterClassChild(HINSTANCE hInstance);
LPCTSTR WinClassWinName = ("ChildClass");
HWND hWL;
HWND hWnd;

// ������� �������� 

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG msg;
	// ��������� ����� ���� 
	MyRegisterClass(hInstance);
	RegisterClassChild(hInstance);
	// ��������� ���� ��������
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	// ���� ������� ����������
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
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE; 		//����� ����
	wcex.lpfnWndProc = (WNDPROC)WndProc; 		//������ ���������
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; 			//���������� ��������
	wcex.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1)); 		//���������� ������
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW); 	//���������� �������
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW); //��������� ����
	wcex.lpszMenuName = NULL; 				//���������� ����
	wcex.lpszClassName = szWindowClass; 		//��� �����
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex); 			//��������� ����� ����
}
//�������� ���������� ����
ATOM RegisterClassChild(HINSTANCE hInstance)
{
	WNDCLASSEX wch = { 0 };
	// ����������� ������ ����
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
// ������� ���� �������� � ������ ���������� �������� � ����� hInst

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; //������ ���������� ������� � ����� hInst
	hWnd = CreateWindow(szWindowClass, 	// ��� ����� ����
		NULL, 				// ����� ��������
		WS_CAPTION ,			// ����� ����
		600, 			// ��������� �� �	
		400,			// ��������� �� Y	
		750, 			// ����� �� �
		400, 			// ����� �� Y
		NULL, 					// ���������� ������������ ����	
		NULL, 					// ���������� ���� ����
		hInstance, 				// ���������� ��������
		NULL); 				// ��������� ���������.

	if (!hWnd) 	//���� ���� �� ���������, ������� ������� FALSE
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow); 		//�������� ����
	UpdateWindow(hWnd); 				//������� ����
	return TRUE;
}

//��������� ��� ������
struct CONTROLS
{
	HWND hControl; //����� ��������� ����
	int id;	//������������ ���� (���� ���) 
};
const int buttonsinprogramm = 2; //ʳ������ ������ � �������
CONTROLS myControls[buttonsinprogramm]; // ���� ����

//������ �� �������
static OPENFILENAME WordTuple;//��������� �� ����������� �����
static TCHAR FilePath[256] ;//���� �����
static TCHAR FileName[50] ;//�����������������

static OPENFILENAME ScoreFile;//��������� �� ����������� �����
static TCHAR ScoreFileName[256];//���� �����
static HANDLE hFile;//����� �����
static DWORD nCharRead = 500;//�������� �� �����, ��� ������ ����� ��������� �����

static std::ifstream in; //��� ����������
static std::string  WordFileBuff;//����� ��� ����������(��� ���)
static std::vector <std::string> AllWordFormFile;//������ ���
static char ScoreFileBuff[500];//������ ���

static bool guess_the_word;//������� ��� ����-������. �������� �� ������ / �������
int score = 0;//ϳ�������� ����������
static char word[35];//�����, ��� �� �������

static bool StartGame = true;//�������� �� ������� ���

//���� ������
RECT gallowrect;//��������
RECT editrect; //����������� ���� �����������
RECT textrect;//���� ������ ������
RECT scorerect;//���� ������ score
RECT mistakerect[8];//���� �������
int mistakes = 0;//ϳ�������� �������

//������� ��������� ����� �����
void ResetWord(char outtext[], char word[], bool IsNewFile)
{	
	//����� ������ ���
	if (StartGame)
	{	
		EnableWindow(myControls[0].hControl, 0);
		AllWordFormFile.push_back("�������� �������");
		for (int i = 0; i < strlen(AllWordFormFile[0].c_str()); i++)
			outtext[i] = AllWordFormFile[0][i];
		return;
	}

	if (IsNewFile)//������������ �����, ���� ���� ������ ���� ���
	{
		std::srand(std::time(0));
		//����������
		in.open(FilePath);//³�������� ���� �� �������
		if (in.is_open()) //�������� "������� �������� �����"
		{
				AllWordFormFile.clear();//������� 
				while (!in.eof())
				{
					getline(in, WordFileBuff);
					AllWordFormFile.push_back(WordFileBuff);//��������
				}
		}
		in.close();//��������� ����
	}


	int random = rand() % AllWordFormFile.size();//�������� ���� �����
	
	//������� ������
	for (int i = 0; i < 35; i++)
	{
		outtext[i] = '\0';
		word[i] = '\0';
	}

	//�������� �����
	for (int i = 0; i < AllWordFormFile[random].size(); i++)
		word[i] = AllWordFormFile[random][i];

	//�������� ����� � ��������� ����. � ������� �����
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

//������� ������� �������/�������
void ResetGame(HWND hWnd, char outtext[], char word[], bool const win, int& score)
{
	guess_the_word = NULL;
	EnableWindow(hWnd, 0);

	if (win)//������� �������
	{
		guess_the_word = true;

		hWL = CreateWindow(WinClassWinName, "You WIN <(^--^)>",
			WS_SYSMENU | WS_EX_TOPMOST | WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION,
			830, 500, 300, 200, hWnd, 0, hInst, NULL);
		ResetWord(outtext, word, 0);

	}
	if (!win)//������� �������
	{
		//³��� �������
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

	static HPEN GallowPen, MyBlackPen;//���� ��� ���������
	static HFONT MainFont, TitleFont, EditFont, MistakeFont, ScoreFont;//������
	static HBRUSH TextBg;//�����

	//������� ��������� ����
	static HBITMAP GallowBG,Background, Door, D_Door, Question, D_Question, D_Dowload, Dowload, Upload, D_Upload, Theme, D_Theme;
	static BITMAP bm; //���������� ����������� ����������
	static HDC memBit; //�������� ��������
	
	static const int MenuButton = 5; //ʳ������ ������ � ����
	static int xPos, yPos; // ������� ����

	//���� ������
	static RECT menurectangles[MenuButton];//������� ����
	static RECT title;//������ ��������
	static RECT categoryrect;//���� ������ �������

	//������
	static char buff[2];//����� ��� ���������� ���� � ���������� ���� "edit"
	static char MistakeMass[8];//����� ��� ��������� �������
	static char textbuff[35];//����� ��� ������
	static char outtext[35];//³���������� �����
	static char filenamebuff[35];//����� ����� �����
	static char scorebuff[5];//����� ��� ���������

	static bool SymInWord;//³���������� �����
	switch (message)
	{
	//������� �� �볺������� ������ ����
	case WM_NCCALCSIZE:
		break;

		//����������� ����
	case WM_MOUSEMOVE:
		{
		xPos = LOWORD(lParam);
		yPos = HIWORD(lParam);
		hdc = GetDC(hWnd);
		/*���������� ���� (���� ������ ��� ��������)*/
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

		//����������� ��������� ��� �������� ����
	case WM_CREATE: 				
		{
		//������� ��������� �����
		ZeroMemory(&WordTuple, sizeof(WordTuple));
		//���������� ��������� ��������� �����
		WordTuple.lStructSize = sizeof(OPENFILENAME);
		WordTuple.hInstance = hInst;
		WordTuple.lpstrFilter = ("Text\0*.txt");
		WordTuple.lpstrFile = FilePath;
		WordTuple.lpstrFileTitle = FileName;
		WordTuple.nMaxFileTitle = 50;
		WordTuple.nMaxFile = 256;
		WordTuple.lpstrInitialDir = (".\\");
		WordTuple.lpstrDefExt = ("txt");

		//������� ��������� �����
		ZeroMemory(&ScoreFile, sizeof(ScoreFile));
		//���������� ��������� ����� ����
		ScoreFile.lStructSize = sizeof(OPENFILENAME);
		ScoreFile.hInstance = hInst;
		ScoreFile.lpstrFilter = ("Score\0*.score");
		ScoreFile.lpstrFile = ScoreFileName;
		ScoreFile.nMaxFile = 256;
		ScoreFile.lpstrInitialDir = (".\\");
		ScoreFile.lpstrDefExt = ("score");

		//��������� ������� ������
			//��������� ��� ����� ��� ���
		for (int i = 0; i < buttonsinprogramm; i++)
			myControls[i].id = i;

		//���� �����������
		myControls[0].hControl = CreateWindow(
			"edit", "",
			WS_CHILD | WS_VISIBLE | ES_CENTER | ES_MULTILINE,
			255, 250, 400, 100,
			hWnd, (HMENU)myControls[0].id,
			hInst, NULL);

		//������ ��� ��������
		myControls[1].hControl = CreateWindow(
			"button", "���������",
			BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
			255, 350, 402, 40,
			hWnd, (HMENU)myControls[1].id,
			hInst, NULL);

		//��������� �����
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
		//��������� ������
		//���
		Background = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP11));

		//�����
		D_Door = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		Door = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));

		//�������
		Question = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3));
		D_Question = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4));

		//����
		Theme = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP5));
		D_Theme = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP6));

		//���������� ����������
		Dowload = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP7));
		D_Dowload = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP8));

		//������������ ����������
		Upload = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP9));
		D_Upload = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP10));

		//��������
		GallowBG = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP12));
		GallowPen = CreatePen(BS_SOLID, 6, (COLORREF)RGB(0, 250, 250));
		MyBlackPen = CreatePen(BS_SOLID, 10, (COLORREF)RGB(0, 0, 0));
		gallowrect = { 10, 10, 240, 390 };//���� ������

		//�������� �������� ������ ��� �����
		memBit = CreateCompatibleDC(hdc);

		//������������ ����� � �i��i edit
		SendMessage(myControls[0].hControl, WM_SETFONT, (WPARAM)EditFont, 0);

		//���� ������ ����
		for (int i = 0; i < MenuButton; i++)
			menurectangles[i] = { 670, 0 + 80 * i, 750, 80 + 80 * i };

		//���� �������
		for (int i = 0; i < 8; i++)
			mistakerect[i] = { 260 + i * 50, 200, 300 + i * 50, 240 };

		//���� ������ �����
		title = { 280,0,620,40 };

		//���� ������ �����
		scorerect = { 250, 85, 660, 140 };

		//���� ������ �������
		categoryrect = { 250, 45, 660, 110 };

		//���� ������ ������
		textrect = { 250, 130, 660, 190 };
		TextBg = CreateHatchBrush(HS_CROSS, RGB(0, 200, 200));//���

		//���� ������ ���� �����������
		editrect = { 255, 250, 655, 350 };

		//��������� �����
		ResetWord(outtext, word, 0);
		//������ �������
		LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
		SetClassLong(hWnd, GCL_HICON, (LONG)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)));
		}
		break;

		//������������ ����
	case WM_PAINT: 				
		{
		hdc = BeginPaint(hWnd, &ps); 	//������ ��������� ����	

		//���
		GetObject(Background, sizeof(bm), &bm);//�������� ��� ������
		SelectObject(memBit, Background);//���� �����
		BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, memBit, 0, 0, SRCCOPY);//���� ������

		//���� ������
		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, MainFont);
		SelectObject(hdc, TextBg);
		FillRect(hdc, &textrect, TextBg);
		FrameRect(hdc, &textrect, HBRUSH(GetStockObject(BLACK_BRUSH)));
		DrawText(hdc, textbuff, wsprintf(textbuff, "%s", outtext), &textrect, DT_CENTER | DT_WORD_ELLIPSIS | DT_SINGLELINE);

		//ϳ�������� ����������
		SelectObject(hdc, ScoreFont);
		SetTextColor(hdc, RGB(100, 250, 200));
		DrawText(hdc, scorebuff, wsprintf(scorebuff, "Your Score: %d", score), &scorerect, DT_LEFT | DT_SINGLELINE | DT_WORD_ELLIPSIS);

		//�������� ��������
		DrawText(hdc, filenamebuff, wsprintf(filenamebuff, "Category: %s", FileName), &categoryrect, DT_LEFT | DT_SINGLELINE | DT_WORD_ELLIPSIS);

		//���� �������
		SelectObject(hdc, GetStockObject(GRAY_BRUSH));
		SelectObject(hdc, (GallowPen));
		SelectObject(hdc, (MistakeFont));
		SetTextColor(hdc, COLORREF(RGB(250, 250, 250)));
		for (int i = 0; i < mistakes; i++)
		{
			Rectangle(hdc, mistakerect[i].left, mistakerect[i].top, mistakerect[i].right, mistakerect[i].bottom);
			if (MistakeMass[i] != NULL)
			{
				//���������� ��� ����� � �����
				if ((int('�') <= MistakeMass[i] && MistakeMass[i] <= int('�')))
					MistakeMass[i] -= 32;

				DrawText(hdc, (LPCSTR)&MistakeMass[i], 1, &mistakerect[i], DT_CENTER | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			}
		}
	
		//������� ��������
		GetObject(GallowBG, sizeof(bm), &bm);
		SelectObject(memBit, GallowBG);//���� �����
		SelectObject(hdc, (HBRUSH)GetStockObject(BLACK_BRUSH));
		SelectObject(hdc, (HPEN)GetStockObject(BLACK_PEN));

		Rectangle(hdc, 10, 10, 240, 390);//�����
		StretchBlt(hdc, 15, 15, 220, 370, memBit, 10, 10, 220, 370, SRCCOPY);//���� ������

		SelectObject(hdc, GallowPen);

		for (int i = 0; i < mistakes; i++)
		{
			//������� ��������
			switch (i)
			{
			case 0:
				MoveToEx(hdc, 200, 360, 0);
				LineTo(hdc, 200, 200);
				break;
			case 1://����. �����
				MoveToEx(hdc, 225, 200, 0);
				LineTo(hdc, 120, 200);
				break;
			case 2://����
				MoveToEx(hdc, 140, 200, 0);
				LineTo(hdc, 140, 230);
				break;
			case 3://�����
				SelectObject(hdc, GetStockObject(NULL_BRUSH));
				SelectObject(hdc, (MyBlackPen));
				Ellipse(hdc, 125, 230, 155, 280);
				break;
			case 4://������
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
			case 5://ҳ��
				Ellipse(hdc, 130, 270, 150, 330);

				SelectObject(hdc, (MyBlackPen));
				Arc(hdc, 125, 230, 155, 280, 130, 270, 155, 280);
				break;
			case 6://����
				SelectObject(hdc, (GallowPen));

				MoveToEx(hdc, 130, 290, 0);
				LineTo(hdc, 120, 270);

				MoveToEx(hdc, 150, 290, 0);
				LineTo(hdc, 160, 270);
				break;
			case 7://����
				MoveToEx(hdc, 130, 330, 0);
				LineTo(hdc, 120, 350);

				MoveToEx(hdc, 150, 330, 0);
				LineTo(hdc, 160, 350);

				ResetGame(hWnd, outtext, word, false, score);

				break;
			}
		}
		//�����
		SetTextColor(hdc, RGB(0, 250, 250));
		SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, TitleFont);
		TextOut(hdc, 340, -8, (LPCSTR)"��������", 8);

		//������� ���� 
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

		//����� �������
		if (strcmp(outtext, word) == 0 && StartGame == false)
			ResetGame(hWnd, outtext, word, true, score);

		EndPaint(hWnd, &ps); 		//�������� ��������� ����	
		}
		break;

		//������� ������� ����
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
		case 1://������� ������ "��������"

			//������� ������ � �����
			GetWindowText((HWND)myControls[0].hControl, buff, 2);

			//������ � ������ ����� 
			if ((int('�') <= *buff && *buff <= int('�')))
				*buff -= 32;

			//�������� �� ��, �� ������� �����. �� ���� �� ������������
			for (int i = 0; i < mistakes; i++)
			{
				if ((MistakeMass[i] == *buff))
				{
					return 0;
				}
			}

			//�������� �� �������� ����� � ����
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

			//����� ������� �� ����������
			if (SymInWord == true) //����������
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
				//������� ����
				SendMessage((HWND)myControls[0].hControl, EM_SETSEL, 0, -1);
				SendMessage((HWND)myControls[0].hControl, WM_CLEAR, 0, 0);
				SetFocus(HWND(myControls[0].hControl));

			}
			if (SymInWord == false) // �������
			{
				//���� ����� ������� � ����
				if ((*buff >= '�' && *buff <= '�') || (*buff >= '�' && *buff <= '�'))
					MistakeMass[mistakes++] = *buff;

				//��������� ����
				InvalidateRect(hWnd, &gallowrect, 0);
				for (int i = 0; i < mistakes; i++)
					InvalidateRect(hWnd, &mistakerect[i], 0);
				SetFocus(HWND(myControls[0].hControl));
				//����� ������������ �����
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

		//��������� edit
	case WM_CTLCOLOREDIT:
		{
			// ����������� ���������� �������� ��������
			SendMessage((HWND)lParam, EM_LIMITTEXT, 1, 0); 
			if ( (HWND)lParam == GetDlgItem(hWnd, myControls[0].id)) {
				SetTextColor((HDC)wParam, RGB(65, 250, 225));
				SetBkColor((HDC)wParam, RGB(0, 100, 250));
				//HideCaret((HWND)myControls[0].id);
				return 0;
			}
		}
		break;

		//��������������� ����
	case WM_LBUTTONUP:
		{
		for (int i = 0; i < MenuButton; i++)
		{
			if (StartGame == true && (PtInRect(&menurectangles[0], { xPos, yPos }) || PtInRect(&menurectangles[1], { xPos, yPos })))
			{
				MessageBox(hWnd,"�� �� ������ �������� �� ������������� ���, ���� �� ������ ������ ���\n3-�� ����� ����","������������",MB_OK);
				break;
			}
			if (PtInRect(&menurectangles[i], { xPos, yPos }))
			{
				switch (i)
				{
				case 0://���������� ���

					EnableWindow(hWnd, false);
					ScoreFile.lpstrTitle = ("������� ���� ��� ������");
					ScoreFile.Flags = OFN_NOTESTFILECREATE;
					if (GetSaveFileName(&ScoreFile) == TRUE)
					{
						//��������� ����� ������
						//������� ������
						for (int i = 0; i < 500; i++)
							ScoreFileBuff[i] = '\0';

						//�������� �����
						for (int i = 0; i < strlen(word); i++)
							ScoreFileBuff[i] = word[i];
						ScoreFileBuff[strlen(word)] = '\n';

						//����
						for (int koef = 100000, i = strlen(word) + 1; i < strlen(word) + 6; i++, koef /= 10)
							ScoreFileBuff[i] = (score) % koef / (koef / 10) + '0';
						ScoreFileBuff[strlen(word) + 6] = '\n';

						//�������
						for (int i = strlen(word) + 7; i < strlen(word) + mistakes + 7; i++)
							ScoreFileBuff[i] = MistakeMass[i - strlen(word) - 7];
						ScoreFileBuff[strlen(word) + mistakes + 7] = '\n';

						//�� ������� �����
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

				case 1://���������� ���
					EnableWindow(hWnd, false);

					ScoreFile.lpstrTitle = ("������� ���� ��� ������");
					ScoreFile.Flags = OFN_HIDEREADONLY;
					if (GetOpenFileName(&ScoreFile) == TRUE)
					{
						//��������� �����
						for (int i = 0; i < 500; i++)
							ScoreFileBuff[i] = '\0';
						hFile = CreateFile(ScoreFileName, GENERIC_READ, 0, NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						ReadFile(hFile, ScoreFileBuff, 500, &nCharRead, NULL);
						CloseHandle(hFile);

						//���������� ����
						score = 0;
						mistakes = 0;
						for (int i = 0; i < 35; i++)
						{
							outtext[i] = '\0';
							word[i] = '\0';
						}

						//��������� ����� �� ������ ���� 
						for (int koef = 10,f = 0,pos = 0, i = 0; i < strlen(ScoreFileBuff); i++, f++)
						{
							if (ScoreFileBuff[i] == '\n')
							{
								pos++; f = 0; i++;
							}
							switch (pos)
							{
							case 0:word[f] = ScoreFileBuff[i]; //�����
								break;
							case 1:score = atoi(&ScoreFileBuff[i]); i += 4;//�������
								break;
							case 2:
								if (ScoreFileBuff[i] == '\n')
								{
									pos++; f = -1;
								}
								else
								{
									MistakeMass[f] = ScoreFileBuff[i];//�������
									mistakes++;
								}
								break;
							case 3:outtext[f] = ScoreFileBuff[i];//���� �����
								break;
							}
						}
						//��������� ����
						InvalidateRect(hWnd, &scorerect, 0);//�����
						InvalidateRect(hWnd, &gallowrect, 1);
						InvalidateRect(hWnd, &textrect, 1);
						for (int i = 0; i < 8; i++)
							InvalidateRect(hWnd, &mistakerect[i], 1);
					}
					EnableWindow(hWnd, 1);
					ShowWindow(hWnd, SW_SHOWNA);
					break;
				case 2://����� �������� 
					EnableWindow(hWnd, false);
					WordTuple.lpstrTitle = ("������� ���� ��� ������");
					WordTuple.Flags = OFN_HIDEREADONLY;

					//if (!StartGame)
					//	MessageBox(hWnd, "�����! \n������� ��� ���� ��������. �������� ���", "�����!!!", MB_OK);

					if (GetOpenFileName(&WordTuple) == TRUE)//³�������� ����
					{
						//��� �����������
						if (StartGame)
						{
							EnableWindow(myControls[0].hControl, 1);
							StartGame = false;
						}
						

						ResetWord(outtext, word, 1);
						score = 0;//��������� ���������

						//��������� ����� ����� � �������� ���
						for (int i = strlen(FileName); i >= 0; i--)
						{
							if (FileName[i] == '.')
							{
								FileName[i] = '\0';
								break;
							}
							FileName[i] = '\0';
						}

						InvalidateRect(hWnd, &scorerect, 0);//�����
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
				case 3://����
					MessageBox(hWnd, "�������� - ��� ��� ����������� ���. \n������ ���������� �����, ��� �� ������� �������, �������������� ����� ������� � ��������� ������� �������� ������� �������(� ����� ��������� ����� 8).\n\n����:\n1. �������� ���\n2. ��������� ���\n3. ������� ���� ��� ��� ���\n4. ����������\n5. ����� \n\n��������: ��������� ����� ʲ�ʲ-19-7, ������� ����������� ", "������", MB_OK);
					break;
				case 4://�����
					SendMessage(hWnd, WM_DESTROY, 0, 0);
					break;
				}
			}
		}
		}
		break;

		//���������� ����
	case WM_LBUTTONDOWN:
		{
		//���������� ���� ��� ������� ����
			if (PtInRect(&title, { xPos, yPos }))
				SendMessage(hWnd, WM_SYSCOMMAND, SC_SIZE | HTMAXBUTTON, 0);
		//�������� ������� � "edit", ���� ������� ���� �����
			if (!PtInRect(&editrect, { xPos, yPos }))
			{
				SetFocus(hWnd);
				SendMessage((HWND)myControls[0].hControl, EM_SETSEL, 0, -1);
				SendMessage((HWND)myControls[0].hControl, WM_CLEAR, 0, 0);
			}
		}
		break;

		//������ ������
	case WM_KEYDOWN:
		{
		switch (wParam)
		{
		case VK_F1:
			MessageBox(hWnd, "�������� - ��� ��� ����������� ���. \n������ ���������� �����, ��� �� ������� �������, �������������� ����� ������� � ��������� ������� �������� ������� �������(� ����� ��������� ����� 8).\n\n����:\n1. �������� ���\n2. ��������� ���\n3. ������� ���� ��� ��� ���\n4. ����������\n5. ����� \n\n��������: ��������� ����� ʲ�ʲ-19-7, ������� ����������� ", "������", MB_OK);
			break;
		case VK_ESCAPE:
			SendMessage(hWnd, WM_DESTROY, 0, 0);
			break;
		}
		}
		break;

		//���������� ������ �� ��������� ��'���� �� ���'��
	case WM_DESTROY: 
		{
		DeleteObject(memBit);
		DeleteObject(&bm);
		//�������
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
		//������
		DeleteObject(MainFont);
		DeleteObject(TitleFont);
		DeleteObject(EditFont);
		DeleteObject(MistakeFont);
		//����� / ����
		DeleteObject(GallowPen);
		DeleteObject(MyBlackPen);
		DeleteObject(TextBg);
		//�����
		DeleteObject(&ScoreFile);
		DeleteObject(&WordTuple);
		PostQuitMessage(0);			
		}
		break;
		
	default:
		//������� ����������, �� �� �������� ������������
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//³��� ��� ���������� ��� ������� �� ��������. 
LRESULT CALLBACK WndWL(HWND hWL, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	static RECT rt;
	static int sx, sy;
	static char buff[35];//����� ��� �����
	static char buff1[200];//����� ��� �������
	static HFONT MainFont1;//����� ������
	static HWND butt;//������
	switch (message)
	{
	case WM_CREATE: 				//����������� ��������� ��� �������� ����
		{
		SetFocus(hWL);//������������ �����
		for (int i = 0; i < 35; i++)//���������� �����
			buff[i] = word[i];

		//��������� �����
		MainFont1 = CreateFont(30, 11, 0, 0, 700, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Times New Roman");

		//���������� ������
		butt = CreateWindow(
			"button", "OK",
			BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
			110, 110, 60, 30,
			hWL, (HMENU)5,
			hInst, NULL);
		rt = { 10,10,270,200 };//���� ������
		}
		break;

		//������� ������� �����
	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_RETURN:
			SendMessage(hWL, WM_CLOSE, 0, 0);//�������� ���� ������� ���������� ������
			break;
		case VK_ESCAPE:
			SendMessage(hWL, WM_CLOSE, 0, 0);
			break;
		}
		break;

		//������� ������
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case 5:
			SendMessage(hWL, WM_CLOSE, 0, 0);
			break;
		}
		break;

	case WM_PAINT: 				//������������ ����
		{
		hdc = BeginPaint(hWL, &ps); 	//������ ��������� ����	
		SelectObject(hdc, MainFont1);
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(0, 0, 0));
		if (guess_the_word)
			DrawText(hdc, buff1, wsprintf(buff1, "�� ������� ����� :)\n�� ������ ������� \n%d ����", score), &rt, DT_CENTER | DT_VCENTER | DT_WORD_ELLIPSIS);
		else
			DrawText(hdc, buff1, wsprintf(buff1, "�� �������� :(\n�������� �����:\n| %s |", buff), &rt, DT_CENTER | DT_VCENTER | DT_WORD_ELLIPSIS);
		EndPaint(hWL, &ps); 		//�������� ��������� ����	
		}
		break;

		//ĳ� ��� ������� ����
	case WM_CLOSE:
		{
		ShowWindow(hWnd, SW_RESTORE);//������������� ����
		EnableWindow(hWnd, 1);
		SetFocus(myControls[0].hControl);
		DestroyWindow(hWL);

		//��������� ���������� ����
		for (int i = 0; i < mistakes; i++)//�������
			InvalidateRect(hWnd, &mistakerect[i], 0);
		//��������� ����
		InvalidateRect(hWnd, &gallowrect, 0);//��������
		InvalidateRect(hWnd, &textrect, 0);//�����
		InvalidateRect(hWnd, &scorerect, 0);//����

		mistakes = 0;//��������� �������

		//������� edit
		SendMessage((HWND)myControls[0].hControl, EM_SETSEL, 0, -1);
		SendMessage((HWND)myControls[0].hControl, WM_CLEAR, 0, 0);

		return 0;
		}
		break;

	default:
		//������� ����������, �� �� �������� ������������
		return DefWindowProc(hWL, message, wParam, lParam);
	}
	return 0;
}