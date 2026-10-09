// -----------------------------------------------------------------------
// Файл:       program2_visualizer_window.cpp
// Призначення: Програма №2 "Візуалізація даних" (віконна, WinAPI).
//              Відображає data.dat у пам'ять і кожні 0.5 секунди (таймер)
//              перемальовує вікно, виводячи кожен елемент масиву як
//              горизонтальний рядок із N символів "*", де N - значення
//              елемента. Реагує на зміни, які вносить програма №1 (або
//              №4), оскільки всі процеси мапують один і той самий файл.
// Автор:      Зінченко Максим Вячеславович, гр. ІПЗ-23-5
// -----------------------------------------------------------------------
#include "common.h"

// Ідентифікатор таймера та інтервал (0.5 секунди = 500 мс)
static const UINT_PTR TIMER_ID = 1;
static const UINT TIMER_INTERVAL_MS = 500;

// Глобальний контекст MMF. Використання глобальної змінної тут
// виправдане специфікою WinAPI - віконна процедура WndProc не має
// власного механізму передачі даних користувача без ускладнення коду
// (GWLP_USERDATA), тому для навчальної задачі обрано простіший варіант.
static MmfContext g_ctx;
static bool g_mmfReady = false;

// -----------------------------------------------------------------------
// ФУНКЦІЯ:  DrawArrayAsBars
// ПОВЕРТАЄ: нічого (void)
// ПАРАМЕТРИ:
//   hdc - контекст пристрою (device context) вікна, отриманий у
//         BeginPaint/GetDC
// ВИКОНУЄ:  Захоплює мьютекс, копіює поточний стан масиву з MMF у
//           локальний буфер (щоб мінімізувати час утримання блокування
//           під час промальовки), звільняє мьютекс, а потім виводить
//           кожен елемент як рядок символів "*" через TextOutW.
// ОСОБЛИВОСТІ:
//   Копіювання даних під локом, а малювання - поза локом: це свідоме
//   рішення, щоб не тримати мьютекс (і не блокувати програми №1/№4)
//   протягом усього часу рендерингу, який може бути відносно повільним.
// -----------------------------------------------------------------------
void DrawArrayAsBars(HDC hdc)
{
    if (!g_mmfReady) return;

    // Локальна копія масиву - виділяємо на стеку/купі поза MMF
    static int localCopy[256];
    DWORD count = 0;

    // ---------------- Критична секція: читання масиву ----------------
    if (LockMmf(g_ctx.hMutex, 2000))
    {
        count = g_ctx.dwCount;
        if (count > 256) count = 256; // захист від переповнення буфера
        for (DWORD i = 0; i < count; ++i)
            localCopy[i] = g_ctx.pData[i];
        UnlockMmf(g_ctx.hMutex);
    }
    else
    {
        wchar_t buf[] = L"Не вдалося прочитати дані (мьютекс зайнятий)";
        TextOutW(hdc, 10, 10, buf, (int)wcslen(buf));
        return;
    }
    // ------------------- Кінець критичної секції ----------------------

    // Малювання поза критичною секцією
    const int rowHeight = 20;
    for (DWORD i = 0; i < count; ++i)
    {
        int value = localCopy[i];
        if (value < 0) value = 0;
        if (value > 200) value = 200; // захист від "зʼїдання" пам'яті рядка

        wchar_t line[256];
        int pos = 0;
        for (int s = 0; s < value && pos < 250; ++s)
            line[pos++] = L'*';
        line[pos] = L'\0';

        // Виводимо номер елемента, значення та відповідну кількість "*"
        wchar_t full[300];
        swprintf_s(full, L"[%2lu] (%3d) %s", i, localCopy[i], line);

        TextOutW(hdc, 10, 10 + static_cast<int>(i) * rowHeight,
            full, (int)wcslen(full));
    }
}

// -----------------------------------------------------------------------
// ФУНКЦІЯ:  WndProc
// ПОВЕРТАЄ: LRESULT - результат обробки повідомлення
// ПАРАМЕТРИ:
//   hwnd, msg, wParam, lParam - стандартні параметри віконної процедури
// ВИКОНУЄ:  Обробляє повідомлення WM_CREATE (підключення до MMF та
//           запуск таймера), WM_TIMER (запит на перемальовку вікна),
//           WM_PAINT (власне промальовка через BeginPaint/EndPaint) та
//           WM_DESTROY (звільнення ресурсів і завершення програми).
// ОСОБЛИВОСТІ:
//   Підключення до MMF винесено у WM_CREATE, а не в main(), оскільки
//   за завданням "відобразити файл data.dat у пам'ять" є частиною
//   ініціалізації самого вікна.
// -----------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        g_ctx.hMutex = OpenNamedMutexOrCreate();
        if (g_ctx.hMutex == nullptr)
        {
            MessageBoxW(hwnd, L"Не вдалося створити/відкрити мьютекс.",
                L"Помилка", MB_ICONERROR);
            return -1; // скасовує створення вікна
        }

        if (!OpenExistingDataMmf(g_ctx))
        {
            DWORD err = GetLastError();
            LPWSTR msgBuf = nullptr;
            FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR)&msgBuf, 0, nullptr);

            wchar_t details[512];
            swprintf_s(details,
                L"Не вдалося відкрити data.dat / MMF.\n\n"
                L"Код помилки Windows: %lu\n"
                L"Опис: %ls\n\n"
                L"Переконайтесь, що програма №3 (генератор) "
                L"була запущена першою і в тій самій папці.",
                err, msgBuf ? msgBuf : L"(немає опису)");

            if (msgBuf) LocalFree(msgBuf);

            MessageBoxW(hwnd, details, L"Помилка", MB_ICONERROR);
            return -1;
        }

        g_mmfReady = true;

        // Таймер на 0.5 секунди - за завданням SetTimer/WM_TIMER
        SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL_MS, nullptr);
        return 0;
    }

    case WM_TIMER:
    {
        // За кожним тіком таймера просимо систему перемалювати вікно
        InvalidateRect(hwnd, nullptr, TRUE); // TRUE = очистити фон
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawArrayAsBars(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
    {
        KillTimer(hwnd, TIMER_ID);
        ReleaseMmfContext(g_ctx);
        g_mmfReady = false;
        PostQuitMessage(0);
        return 0;
    }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// -----------------------------------------------------------------------
// ФУНКЦІЯ:  wWinMain
// ПОВЕРТАЄ: int - код завершення програми
// ВИКОНУЄ:  Стандартна точка входу для GUI-програм Windows. Реєструє
//           клас вікна, створює вікно та запускає цикл обробки
//           повідомлень (message loop).
// -----------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Lab1_VisualizerWindowClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    if (!RegisterClassW(&wc))
    {
        PrintLastError("RegisterClassW");
        return 1;
    }

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Програма №2: Візуалізація даних (MMF)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 700,
        nullptr, nullptr, hInstance, nullptr);

    if (hwnd == nullptr)
    {
        PrintLastError("CreateWindowExW");
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}