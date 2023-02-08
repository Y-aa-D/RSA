#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <fstream>

using namespace std;
unsigned long seed = 1444674243;

unsigned long get_seed()
{    
    SYSTEMTIME lct;    

    cout << "Выберите инструкции:" << endl << "1 - зерно задано по умолчанию (Seed = 1444674243)" << endl << "2 - зерно формируется с помощью системного времени" << endl << "Любая другая клавиша - выйти с программы" << endl;
    switch (_getch())
    {
        case '1': 
        { 
            seed = 1444674243;
            break;
        }
        case '2': 
        { 
            GetLocalTime(&lct);
            seed = lct.wMilliseconds; //9
            seed = seed << 6;
            seed = seed | lct.wSecond;
            seed = seed << 6;
            seed = seed | lct.wMinute;
            seed = seed << 4;
            seed = seed | lct.wHour;
            seed = seed << 5;
            seed = seed | lct.wDay;
            seed = seed << 4;
            seed = seed | (lct.wSecond & lct.wMinute);
            cout << "seed - " << seed << endl;
            break;
        }
        default: exit(0);
    }
    return seed;
}

unsigned long get_number(unsigned long max)
{
    unsigned long tmp, SR, SL, res;

    SR = SL = seed;

    tmp = 0xFF000000;
    tmp = tmp & SL;
    SL = SL << 8;
    tmp = tmp >> 24;
    SL = SL + tmp;

    tmp = 0x000000FF;
    tmp = tmp & SR;
    SR = SR >> 8;
    tmp = tmp << 24;
    SR = SR + tmp;

    SL = SL + SR;
    seed = SL;
    res = seed % max;

    return res;
}

unsigned long long modulo_degree(unsigned long long number, unsigned long degree, unsigned long module)
{
    unsigned long long result = 1;
    while( degree != 0 )
    {        
        if ((degree & 1) != 0)
        {
            result = (result * number) % module;
        }
        number = (number * number) % module;
        degree = degree >> 1;
    }

    return result;
}

bool rabin_miller(unsigned long long number)
{
    // true - вероятно простое, false - составное
    // Представить n − 1 в виде 2^(s)·t, где t нечётно

    unsigned long copy_number = number - 1;
    unsigned long s = 0;
    unsigned long a = copy_number/4; // число свидетель
    unsigned long long x;

    if (number % 2 == 0) { return false; }
    else {

        while (copy_number % 2 == 0)
        {
            copy_number /= 2;
            s++;
        }

        // 1-(1/(2^k)) - вероятность верного определения простоты числа, при k=6 - 98.4375 %
        for (int k = 0; k < 6; k++)
        {
            a = 1.1 * a;
            x = modulo_degree(a, copy_number, number);

            if (x == 1 || x == (number - 1)) { continue; }
            else
            {
                for (int i = 0; i < (s - 1); i++)
                {
                    x = modulo_degree(x, 2, number);

                    if (x == 1)
                    {
                        return false;
                    }

                    if (x == number - 1)
                    {
                        return false;
                    }
                }
                return false;
            }            
        }
        return true;
    }
}

unsigned long extended_euclid(unsigned long number1, unsigned long number2, unsigned long &y)
{
    unsigned int d0, d1, y1, y0, r0, r1;
    unsigned int R, r, d;

    r0 = number1;
    r1 = number2;
    d0 = 1;
    d1 = 0;
    y0 = 0;
    y1 = 1;
    int i = 0;

    do
    {
        R = r0 % r1;
        r = r0 / r1;
        d = d0 + (r * d1);
        y = y0 + (r * y1);
        r0 = r1;
        r1 = R;
        d0 = d1;
        d1 = d;
        y0 = y1;
        y1 = y;
        i++;
    } while (R != 0);

    if ((i % 2) != 0) 
    {
        y = number1*(d1 - d0) - number2*(y1 - y0);
        return d1 - d0;        
    }

    else 
    {
        y = number1 * d0 - number2 * y0;
        return d0;
    }
}

unsigned long gcd(unsigned long number1, unsigned long number2)
{
    unsigned long long a = number1, b = number2;
    int tmp = 1;

    while (tmp != 0)
    {
        tmp = a % b;
        a = b;
        b = tmp;
    }

    return a;
}

int RSA_create_keys()
{
    unsigned long seed, p, q, n, f_n, e, d, y;
    
    while (true)
    {
        cout << "Выберите инструкции:" << endl << "1 - сгенерировать новую пару ключей" << endl << "2 - оставить текущие ключи" << endl << endl;

        switch (_getch())
        {
            case '1':
            {
                p = get_number(0x7FF) | 0x407;
                do
                {
                    p--;
                } while (!rabin_miller(p));

                q = get_number(0x7FF) | 0x407;
                do
                {
                    q--;
                } while (!rabin_miller(q));
                n = q * p;
                f_n = (p - 1) * (q - 1);
                e = get_number(f_n - 1) | 0x11;

                while (gcd(e, f_n) != 1)
                {
                    e--;
                }

                d = extended_euclid(e, f_n, y);

                ofstream close_key("close_key.txt"), open_key("open_key.txt");
                cout << "Закрытый ключ (d,n) - " << d << " " << n << endl;
                cout << "Открытый ключ (e,n) - " << e << " " << n << endl << endl;
                close_key << d << '\n' << n;
                open_key << e << '\n' << n;
                close_key.close();
                open_key.close();
                break;
            }

            case '2':
            {
                return false;
            }
        }
    }
}

string Transport_coding(int block)
{
    int tmp;
    string arr = "QWERTYUIOP123456", str_block, str_block_reverse;
    for (int i = 0; i <= 5; i++)
    {        
        tmp = block & 0x00000F;
        block = block >> 4;
        str_block = str_block + arr[tmp];
    }

    for (int i = 5; i >= 0; i--)
    {
        str_block_reverse = str_block_reverse + str_block[i];
    }

    return str_block_reverse;
}

void RSA_encrypt()
{
    ofstream encrypt("encrypt.txt");
    ifstream open_key("open_key.txt"), input("input.txt");

    int block = 0, tmpF0 = 0, tmp0F = 0, ch = 0, length_file = 0;
    unsigned long n, e;
    open_key >> e;
    open_key >> n;
    open_key.close();

    input.seekg(0, input.end);
    length_file = input.tellg() / 5;
    input.seekg(0, input.beg);

    for (length_file; length_file > 0; length_file--)
    {
        for (int i = 0; i <= 1; i++)
        {
            block = block << 8;
            block = block + input.get();            
        }
        block = block << 4;
        
        tmpF0 = input.get();
        tmp0F = tmpF0;
        tmp0F = tmp0F & 0x0F;
        tmpF0 = tmpF0 >> 4;
        block = block + tmpF0;

        block = modulo_degree(block, e, n);
        encrypt << Transport_coding(block);
        block = 0;

        block = block + tmp0F; 
        for (int i = 0; i <= 1; i++)
        {
            block = block << 8;
            block = block + input.get();
        }
        block = modulo_degree(block, e, n);
        encrypt << Transport_coding(block);
        block = 0;
    }  

    //последний двойной блок дополняется пробелами ( ** ** *|* ** ** ___ ** 00 0|0 00 00) 
    ch = input.get();
    input.seekg(-1, input.cur);
    if (ch != -1) 
    { 
        for (int i = 0; i <= 1; i++)
        {
            ch = input.get();
            if (ch == EOF) { ch = 32; }
            block = block << 8;
            block = block + ch;
        }
        block = block << 4;

        ch = input.get();
        if (ch == EOF) { ch = 32; }        
        tmpF0 = ch;
        tmp0F = tmpF0;
        tmp0F = tmp0F & 0x0F;
        tmpF0 = tmpF0 >> 4;
        block = block + tmpF0;

        block = modulo_degree(block, e, n);
        encrypt << Transport_coding(block);
        block = 0;

        
        block = block + tmp0F;
        for (int i = 0; i <= 1; i++)
        {
            block = block << 8;
            ch = input.get();
            if (ch == EOF) { ch = 32; }
            block = block + ch;
        }
        block = modulo_degree(block, e, n);
        encrypt << Transport_coding(block);
        block = 0;
    }    
}

int Transport_decoding(string str_block)
{
    int j = 0, block = 0;
    string arr = "QWERTYUIOP123456";
    for (int i = 0; i <= 5; i++)
    {
        while (str_block [i] != arr[j])
        {
            j++;
        }
        block = block + j;
        block = block << 4;
        j = 0;
    }
    block = block >> 4;
    return block;
}

void RSA_decrypt()
{
    ofstream decrypt("decrypt.txt");
    ifstream close_key("close_key.txt"), encrypt("encrypt.txt");

    int block = 0, tmpF0 = 0, tmp0F = 0, ch = 0, length_file = 0;
    unsigned long n, d;
    string str_block;
    close_key >> d;
    close_key >> n;
    close_key.close();

    encrypt.seekg(0, encrypt.end);
    length_file = encrypt.tellg() / 12 - 1;
    encrypt.seekg(0, encrypt.beg);

    for (length_file; length_file > 0; length_file--)
    {
        ch = encrypt.get();
        if (ch == EOF) { break; }
        else { encrypt.seekg(-1, encrypt.cur); }

        for (int i = 0; i <= 5; i++)
        {
            str_block = str_block + char(encrypt.get());
        }
        block = Transport_decoding(str_block);
        str_block = "";
        block = modulo_degree(block, d, n);

        tmpF0 = block & 0x0000F;
        tmpF0 = tmpF0 << 4;
        block = block >> 4;
        decrypt << char(block >> 8);
        decrypt << char(block & 0xFF);

        for (int i = 0; i <= 5; i++)
        {
            str_block = str_block + char(encrypt.get());
        }
        block = Transport_decoding(str_block);
        str_block = "";
        block = modulo_degree(block, d, n);

        tmp0F = block & 0xF0000;
        tmp0F = tmp0F >> 16;
        tmpF0 = tmpF0 + tmp0F;
        decrypt << char(tmpF0);
        block = block & 0x0FFFF;
        decrypt << char(block >> 8);
        decrypt << char(block & 0xFF);
    }

    //последний двойной блок дополняется пробелами ( ** ** *|* ** ** ___ ** 00 0|0 00 00) 
    ch = encrypt.get();
    encrypt.seekg(-1, encrypt.cur);
    if (ch != -1)
    {
        for (int i = 0; i <= 5; i++)
        {
            str_block = str_block + char(encrypt.get());
        }
        block = Transport_decoding(str_block);
        str_block = "";
        block = modulo_degree(block, d, n);

        tmpF0 = block & 0x0000F;
        tmpF0 = tmpF0 << 4;
        block = block >> 4;
        if ((block >> 8) != 32) { decrypt << char(block >> 8); }
        if ((block & 0xFF) != 32) { decrypt << char(block & 0xFF); }

        for (int i = 0; i <= 5; i++)
        {
            str_block = str_block + char(encrypt.get());
        }
        block = Transport_decoding(str_block);
        str_block = "";
        block = modulo_degree(block, d, n);

        tmp0F = block & 0xF0000;
        tmp0F = tmp0F >> 16;
        tmpF0 = tmpF0 + tmp0F;
        if (tmpF0 != 32) { decrypt << char(tmpF0); }
        block = block & 0x0FFFF;        
        if ((block >> 8) != 32) { decrypt << char(block >> 8); }
        if ((block & 0xFF) != 32) { decrypt << char(block & 0xFF); }
    }
}

int main()
{
    unsigned long a, b, c;
    system("color F0");
    setlocale(LC_ALL, "Russian");

    while (true)
    {
        cout << "Выберите инструкции:" << endl << "1 - режим отладки функций" << endl << "2 - режим работы" << endl << "Любая другая клавиша - выйти с программы" << endl << endl;
        switch (_getch())
        {
            case '1':
            {
                cout << "Выберите инструкции:" << endl;
                cout << "1 - Тестирование модульного возведения в степень" << endl;
                cout << "2 - Тестирование алгоритма Эвклида" << endl;
                cout << "3 - Тестирование алгоритма Миллера-Рабина" << endl;
                cout << "4 - Тестирование расширенного алгоритма Эвклида" << endl;
                cout << "5 - Тестирование алгоритма генерации псевдослучайных чисел" << endl;
                cout << "6 - Тестирование алгоритма генерации ключей" << endl;
                cout << "7 - Тестирование алгоритмов шифрования, кодирования, декодирования и расшифрования блока" << endl;

                switch (_getch())
                {
                    case '1':
                    {
                        cout << "Введите основание - ";
                        cin >> a;
                        cout << "Введите степень - ";
                        cin >> b;
                        cout << "Введите модуль - ";
                        cin >> c;
                        cout << "Результат - " << modulo_degree(a, b, c) << endl << endl;
                        break;
                    }

                    case '2':
                    {
                        cout << "Введите первое число - ";
                        cin >> a;
                        cout << "Введите второе число - ";
                        cin >> b;
                        cout << "Результат - " << gcd(a, b) << endl << endl;
                        break;
                    }

                    case '3':
                    {
                        cout << "Введите число - ";
                        cin >> a;
                        if (rabin_miller(a)) { cout << "вероятно простое" << endl << endl; }
                        else { cout << "составное" << endl << endl; }
                        break;
                    }

                    case '4':
                    {
                        cout << "Введите первое число - ";
                        cin >> a;
                        cout << "Введите второе число - ";
                        cin >> b;
                        cout << "d - " << extended_euclid(a, b, c) << endl;
                        cout << "y - " << c << endl << endl;
                        break;
                    }

                    case '5':
                    {
                        cout << "Введите максимальное число - ";
                        cin >> a;
                        cout << "Результат - " << get_number(a) << endl << endl;
                        break;
                    }

                    case '6':
                    {
                        RSA_create_keys();
                        break;
                    }

                    case '7':
                    {
                        cout << "Алгоритмы отработали, проверьте файлы - ";
                        RSA_encrypt();
                        RSA_decrypt();
                    }
                }
                break;
            }

            case '2':
            {
                get_seed();
                RSA_create_keys();
                RSA_encrypt();
                RSA_decrypt();
                break;
            }
            default: exit(0);
        }
        
    }
    
    return 0;
}