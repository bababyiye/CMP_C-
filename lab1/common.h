// -----------------------------------------------------------------------
// Файл:       common.h
// Призначення: Спільні константи та допоміжні функції для програм №1-4
//              лабораторної роботи "Процеси та потоки. IPC через MMF".
//              Містить обгортки над WinAPI для роботи з файлом,
//              memory-mapped file та іменованим мьютексом, а також
//              уніфіковану обробку помилок (без аварійного завершення).
// Автор:      Зінченко Максим Вячеславович, гр. ІПЗ-23-5
// -----------------------------------------------------------------------
#pragma once

#include <windows.h>
#include <cstdio>
#include <cstdlib>

// ----------------------- Константи проекту ------------------------------

// Ім'я файлу з даними.
static const wchar_t* DATA_FILE_NAME = L"data.dat";

// Ім'я об'єкта відображення файлу в пам'ять (унікальне для всіх процесів)
static const wchar_t* MMF_MAP_NAME = L"Global\\MMF_DATA_ARRAY";

// Ім'я іменованого мьютекса - критично, щоб було ОДНАКОВИМ у всіх
// програмах (№1, №2, №3, №4), інакше синхронізація не працюватиме.
static const wchar_t* MMF_MUTEX_NAME = L"Global\\MMF_MUTEX";

// Діапазон значень масиву та його розмір за замовчуванням
static const int ARRAY_MIN_VALUE = 10;
static const int ARRAY_MAX_VALUE = 100;
static const int ARRAY_MIN_COUNT = 20;
static const int ARRAY_MAX_COUNT = 30;

// -----------------------------------------------------------------------
// Структура, що інкапсулює всі handle-и, пов'язані з MMF.
// Використовується у всіх чотирьох програмах, щоб не дублювати логіку
// відкриття/закриття ресурсів.
// -----------------------------------------------------------------------
struct MmfContext
{
    HANDLE hFile   = INVALID_HANDLE_VALUE; // handle відкритого файлу data.dat
    HANDLE hMap    = nullptr;              // handle об'єкта file-mapping
    int*   pData   = nullptr;              // вказівник на масив int у пам'яті
    DWORD  dwCount = 0;                    // кількість елементів масиву
    HANDLE hMutex  = nullptr;              // handle іменованого мьютекса
};

// -----------------------------------------------------------------------
// ФУНКЦІЯ:  PrintLastError
// ПОВЕРТАЄ: нічого (void)
// ПАРАМЕТРИ:
//   context - текстовий опис місця виникнення помилки (для журналу)
// ВИКОНУЄ:  Отримує код помилки через GetLastError(), формує
//           локалізоване повідомлення через FormatMessage() і виводить
//           його у консоль/файл журналу.
// ОСОБЛИВОСТІ:
//   Не викликає exit() чи abort() - виклик має самостійно вирішити,
//   чи продовжувати роботу, чи звільнити ресурси й коректно завершитись.
// -----------------------------------------------------------------------
inline void PrintLastError(const char* context)
{
    DWORD err = GetLastError();
    LPWSTR msgBuf = nullptr;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&msgBuf, 0, nullptr);

    fwprintf(stderr, L"[ПОМИЛКА] %hs: код=%lu, опис=%ls\n",
              context, err, msgBuf ? msgBuf : L"(немає опису)");

    if (msgBuf) LocalFree(msgBuf);
}

// -----------------------------------------------------------------------
// ФУНКЦІЯ:  ReleaseMmfContext
// ПОВЕРТАЄ: нічого (void)
// ПАРАМЕТРИ:
//   ctx - контекст MMF, ресурси якого потрібно звільнити
// ВИКОНУЄ:  Коректно звільняє (у правильному порядку!) усі відкриті
//           handle-и: спочатку UnmapViewOfFile, потім CloseHandle для
//           mapping-об'єкта, файлу та мьютекса.
// ОСОБЛИВОСТІ:
//   Безпечна при повторному виклику (idempotent) - перевіряє кожен
//   handle перед закриттям. Це критично для запобігання "зависанню"
//   ресурсів (resource leak) при аварійних сценаріях.
// -----------------------------------------------------------------------
inline void ReleaseMmfContext(MmfContext& ctx)
{
    if (ctx.pData != nullptr)
    {
        FlushViewOfFile(ctx.pData, 0);
        UnmapViewOfFile(ctx.pData);
        ctx.pData = nullptr;
    }
    if (ctx.hMap != nullptr)
    {
        CloseHandle(ctx.hMap);
        ctx.hMap = nullptr;
    }
    if (ctx.hFile != INVALID_HANDLE_VALUE && ctx.hFile != nullptr)
    {
        CloseHandle(ctx.hFile);
        ctx.hFile = INVALID_HANDLE_VALUE;
    }
    if (ctx.hMutex != nullptr)
    {
        CloseHandle(ctx.hMutex);
        ctx.hMutex = nullptr;
    }
}

// -----------------------------------------------------------------------
// ФУНКЦІЯ:  OpenNamedMutexOrCreate
// ПОВЕРТАЄ: HANDLE - логічний номер мьютекса, або nullptr при помилці
// ВИКОНУЄ:  Створює (якщо не існує) або відкриває (якщо вже існує)
//           іменований мьютекс з іменем MMF_MUTEX_NAME. Завдяки
//           однаковому імені у просторі Global\ усі процеси - навіть
//           запущені різними користувачами в одній сесії - будуть
//           синхронізуватись через один і той самий kernel-об'єкт.
// ОСОБЛИВОСТІ:
//   CreateMutexW() є ідемпотентною відносно вже існуючого об'єкта:
//   якщо мьютекс з таким іменем вже існує, функція просто повертає
//   handle на нього, а GetLastError() поверне ERROR_ALREADY_EXISTS
//   (це НЕ є помилкою, а нормальним сценарієм).
// -----------------------------------------------------------------------
inline HANDLE OpenNamedMutexOrCreate()
{
    HANDLE hMutex = CreateMutexW(nullptr, FALSE, MMF_MUTEX_NAME);
    if (hMutex == nullptr)
    {
        PrintLastError("CreateMutexW");
        return nullptr;
    }
    // ERROR_ALREADY_EXISTS - нормальна ситуація: мьютекс уже створений
    // іншим процесом, ми просто отримали handle на той самий об'єкт.
    return hMutex;
}

// -----------------------------------------------------------------------
// ФУНКЦІЯ:  LockMmf / UnlockMmf
// ПРИЗНАЧЕННЯ: Обгортки над WaitForSingleObject/ReleaseMutex для
//              зменшення дублювання коду критичних секцій.
// -----------------------------------------------------------------------
inline bool LockMmf(HANDLE hMutex, DWORD timeoutMs = INFINITE)
{
    DWORD result = WaitForSingleObject(hMutex, timeoutMs);
    if (result == WAIT_OBJECT_0) return true;
    if (result == WAIT_TIMEOUT)
    {
        fwprintf(stderr, L"[ПОПЕРЕДЖЕННЯ] Таймаут очікування мьютекса "
                          L"- можливе взаємне блокування (deadlock)\n");
        return false;
    }
    if (result == WAIT_ABANDONED)
    {
        // Попередній власник мьютекса завершився аварійно, не звільнивши
        // ресурс. Ми все одно отримуємо володіння, але дані під захистом
        // могли залишитись у неузгодженому стані - варто це врахувати.
        fwprintf(stderr, L"[ПОПЕРЕДЖЕННЯ] Мьютекс залишений (abandoned) "
                          L"попереднім власником - можлива неузгодженість "
                          L"даних\n");
        return true;
    }
    PrintLastError("WaitForSingleObject");
    return false;
}

inline void UnlockMmf(HANDLE hMutex)
{
    if (!ReleaseMutex(hMutex))
    {
        PrintLastError("ReleaseMutex");
    }
}

// -----------------------------------------------------------------------
// ФУНКЦІЯ:  OpenExistingDataMmf
// ПОВЕРТАЄ: bool - true при успіху, false при будь-якій помилці
// ПАРАМЕТРИ:
//   ctx - структура, яка буде заповнена handle-ами та вказівником
// ВИКОНУЄ:
//   1. Відкриває існуючий файл data.dat (CreateFile з OPEN_EXISTING);
//   2. Перевіряє, що розмір файлу > 0 (захист від пошкоджених даних);
//   3. Створює file-mapping об'єкт (CreateFileMapping) розміром,
//      що дорівнює розміру файлу;
//   4. Відображає файл у адресний простір процесу (MapViewOfFile).
// ОСОБЛИВОСТІ:
//   Використовується програмами №1, №2, №4, які ОЧІКУЮТЬ, що файл
//   уже створений програмою №3. Якщо файл відсутній - функція виводить
//   зрозуміле повідомлення і повертає false, не завершуючи процес
//   аварійно.
// -----------------------------------------------------------------------
inline bool OpenExistingDataMmf(MmfContext& ctx)
{
    // Крок 1: перевірка існування та відкриття файлу
    ctx.hFile = CreateFileW(
        DATA_FILE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, // дозволяємо іншим процесам
        nullptr,
        OPEN_EXISTING,      // файл має вже існувати (створений прогр. №3)
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (ctx.hFile == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND)
        {
            fwprintf(stderr, L"[ПОМИЛКА] Файл '%ls' не знайдено. "
                              L"Спочатку запустіть програму №3 "
                              L"(генератор масиву).\n", DATA_FILE_NAME);
        }
        else
        {
            PrintLastError("CreateFileW (OPEN_EXISTING)");
        }
        return false;
    }

    // Крок 2: перевірка, що дані не пошкоджені (розмір > 0)
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(ctx.hFile, &fileSize) || fileSize.QuadPart <= 0)
    {
        fwprintf(stderr, L"[ПОМИЛКА] Файл '%ls' порожній або пошкоджений "
                          L"(розмір <= 0)\n", DATA_FILE_NAME);
        ReleaseMmfContext(ctx);
        return false;
    }
    if (fileSize.QuadPart % sizeof(int) != 0)
    {
        fwprintf(stderr, L"[ПОПЕРЕДЖЕННЯ] Розмір файлу не кратний "
                          L"sizeof(int) - дані можуть бути пошкоджені\n");
    }
    ctx.dwCount = static_cast<DWORD>(fileSize.QuadPart / sizeof(int));

    // Крок 3: створення file-mapping об'єкта
    ctx.hMap = CreateFileMappingW(
        ctx.hFile,
        nullptr,
        PAGE_READWRITE,
        0, 0,               // розмір = розмір файлу (0,0 означає "весь файл")
        MMF_MAP_NAME);

    if (ctx.hMap == nullptr)
    {
        PrintLastError("CreateFileMappingW");
        ReleaseMmfContext(ctx);
        return false;
    }

    // Крок 4: відображення файлу у пам'ять процесу
    ctx.pData = static_cast<int*>(MapViewOfFile(
        ctx.hMap,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        0));                // 0 = відобразити весь файл

    if (ctx.pData == nullptr)
    {
        PrintLastError("MapViewOfFile");
        ReleaseMmfContext(ctx);
        return false;
    }

    return true;
}
