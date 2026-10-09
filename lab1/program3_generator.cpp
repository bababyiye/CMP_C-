// -----------------------------------------------------------------------
// Файл:       program3_generator.cpp
// Призначення: Програма №3 "Генератор масиву". Створює (або перезаписує)
//              бінарний файл data.dat з 20-30 випадковими 32-бітними
//              цілими числами у діапазоні [10;100]. Працює автономно і
//              має запускатись ПЕРШОЮ, перед програмами №1 та №2.
// Автор:      Зінченко Максим Вячеславович, гр. ІПЗ-23-5
// -----------------------------------------------------------------------
#include "common.h"
#include <ctime>
#include <vector>

// -----------------------------------------------------------------------
// ФУНКЦІЯ:  GenerateRandomArray
// ПОВЕРТАЄ: std::vector<int> - згенерований масив
// ПАРАМЕТРИ:
//   count - кількість елементів масиву
// ВИКОНУЄ:  Заповнює вектор випадковими числами у діапазоні
//           [ARRAY_MIN_VALUE; ARRAY_MAX_VALUE] за допомогою rand().
// ОСОБЛИВОСТІ:
//   Використовує стандартний rand()/srand() - для навчальної задачі
//   цього достатньо; у "бойовому" коді варто застосувати
//   <random> (std::mt19937) для кращої якості випадкових чисел.
// -----------------------------------------------------------------------
std::vector<int> GenerateRandomArray(int count)
{
    std::vector<int> arr;
    arr.reserve(count);
    for (int i = 0; i < count; ++i)
    {
        int value = ARRAY_MIN_VALUE +
                    (rand() % (ARRAY_MAX_VALUE - ARRAY_MIN_VALUE + 1));
        arr.push_back(value);
    }
    return arr;
}

// -----------------------------------------------------------------------
// ФУНКЦІЯ:  WriteArrayToFile
// ПОВЕРТАЄ: bool - true при успіху
// ПАРАМЕТРИ:
//   arr - масив для запису
// ВИКОНУЄ:
//   1. Відкриває мьютекс синхронізації (на випадок, якщо програми №1/№2
//      вже запущені й читають файл у цей момент - хоча за сценарієм
//      генератор запускається першим, захист все одно додається згідно
//      вимог "усі критичні секції запису у файл повинні бути захищені");
//   2. Створює файл data.dat у режимі CREATE_ALWAYS (перезапис);
//   3. Записує масив як бінарні дані (WriteFile), А НЕ як текст;
//   4. Звільняє усі ресурси.
// ОСОБЛИВОСТІ:
//   Файл відкривається з CREATE_ALWAYS, тобто попередній вміст
//   ЗАВЖДИ затирається - це відповідає завданню "створити або
//   перезаповнити файл data.dat".
// -----------------------------------------------------------------------
bool WriteArrayToFile(const std::vector<int>& arr)
{
    HANDLE hMutex = OpenNamedMutexOrCreate();
    if (hMutex == nullptr) return false;

    bool success = false;

    // ---------------- Критична секція: запис у файл ----------------
    if (LockMmf(hMutex))
    {
        HANDLE hFile = CreateFileW(
            DATA_FILE_NAME,
            GENERIC_WRITE,
            0,                  // ексклюзивний доступ на час запису
            nullptr,
            CREATE_ALWAYS,      // створити або перезаписати
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            PrintLastError("CreateFileW (CREATE_ALWAYS)");
        }
        else
        {
            DWORD bytesWritten = 0;
            DWORD bytesToWrite =
                static_cast<DWORD>(arr.size() * sizeof(int));

            BOOL ok = WriteFile(hFile, arr.data(), bytesToWrite,
                                 &bytesWritten, nullptr);

            if (!ok || bytesWritten != bytesToWrite)
            {
                PrintLastError("WriteFile");
            }
            else
            {
                success = true;
            }
            CloseHandle(hFile);
        }
        UnlockMmf(hMutex);
    }
    // ------------------- Кінець критичної секції ---------------------

    CloseHandle(hMutex);
    return success;
}

int wmain()
{
    // Ініціалізація генератора випадкових чисел поточним часом
    srand(static_cast<unsigned int>(time(nullptr)));

    // Кількість елементів масиву - випадкове число у [20;30]
    int count = ARRAY_MIN_COUNT +
                (rand() % (ARRAY_MAX_COUNT - ARRAY_MIN_COUNT + 1));

    wprintf(L"=== Програма №3: Генератор масиву ===\n");
    wprintf(L"Генерація %d випадкових чисел у діапазоні [%d;%d]...\n",
            count, ARRAY_MIN_VALUE, ARRAY_MAX_VALUE);

    std::vector<int> arr = GenerateRandomArray(count);

    if (!WriteArrayToFile(arr))
    {
        wprintf(L"Не вдалося записати дані у файл '%ls'. "
                L"Роботу завершено без запису.\n", DATA_FILE_NAME);
        return 1;
    }

    wprintf(L"Файл '%ls' успішно створено (%d елементів, %zu байт).\n",
            DATA_FILE_NAME, count, arr.size() * sizeof(int));
    wprintf(L"Згенерований масив:\n");
    for (int v : arr) wprintf(L"%d ", v);
    wprintf(L"\n");

    wprintf(L"Робота завершена\n");
    return 0;
}
