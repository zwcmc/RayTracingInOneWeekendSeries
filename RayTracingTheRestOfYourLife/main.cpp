// #include <random>
// #include <time.h>
// #include <iostream>
// #include <iomanip>

// #include <chrono>
// #include <thread>

// double f(float x)
// {
//     return 3.0 * x * x;
// }

// int main()
// {
//     std::cout << std::fixed << std::setprecision(12);

//     int n = 0;
//     float a = 1.0f, b = 3.0f;
//     double sum = 0.0;
//     std::default_random_engine seed(time(NULL));
//     std::uniform_real_distribution<float> random(a, b);

//     std::chrono::milliseconds interval(1);

//     while (true)
//     {
//         auto start = std::chrono::steady_clock::now();

//         n++;
//         float sample = random(seed);
//         sum += f(sample);

//         if (n % 1000 == 0)
//         {
//             std::cout << "n = " << n  << ", and F_n(X) = " << (sum * (b - a) / n) << '\n';
//         }

//         auto end = std::chrono::steady_clock::now();
//         std::chrono::duration<double> elapsed = end - start;

//         std::chrono::milliseconds sleep_time = interval - std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

//         if (sleep_time.count() > 0)
//         {
//             std::this_thread::sleep_for(sleep_time);
//         }
//     }

//     return 0;
// }
// #include <random>
// #include <time.h>
// #include <cmath>
// #include <iostream>

// #define PI 3.14159265358979323846

// double random_generator1(double x)
// {
//     return x * PI / 2.0;
// }

// double random_generator2(double x)
// {
//     return sqrt(x) * PI / 2.0;
// }

// double pdf1(double x)
// {
//     return 2.0 / PI;
// }

// double pdf2(double x)
// {
//     return 8.0 * x / (PI * PI);
// }

// double f(double x)
// {
//     return sin(x);
// }

// double van_der_corput(int x, int base)
// {
//     double q = 0, bk = 1.0 / base;
//     while (x > 0)
//     {
//         q += (x % base) * bk;
//         x /= base;
//         bk /= base;
//     }
//     return q;
// }

// int main()
// {
    // const int total_times = 16;
    // int run_times = 1;
    // double real = 1.0;
    // std::default_random_engine seed(time(NULL));
    // printf("No     Uniform     Importance   Err.Uniform(%%) Err.Importance(%%)\n");
    // while ((run_times++) < total_times)
    // {
    //     int num_samples = 16;
    //     double sum1 = 0.0, sum2 = 0.0;
    //     std::uniform_real_distribution<double> random(0, 1);

    //     for (int i = 0; i < num_samples; i++)
    //     {
    //         double sample = random(seed);
    //         double random1 = random_generator1(sample);
    //         double random2 = random_generator2(sample);
    //         double p1 = pdf1(random1);
    //         double p2 = pdf2(random2);

    //         sum1 += f(random1) / p1;
    //         sum2 += f(random2) / p2;
    //     }
    //     sum1 /= num_samples;
    //     sum2 /= num_samples;
    //     printf("%-6d %-12.4f %-14.4f %-14.1f %-12.1f\n", run_times, sum1, sum2, 100.0 * (sum1 - real) / real, 100.0 * (sum2 - real) / real);
    // }
//     for (int i = 1; i < 14; i++)
//     {
//         double s = van_der_corput(i, 10);
//         std::cout << s << std::endl;
//     }
    
//     return 0;
// }
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>
#include <random>

#define Float float

const int Primes[] = {
    2, 3, 5, 7, 11,
    13, 17, 19, 23,
    29, 31, 37, 41,
    43, 47, 53, 59,
    61, 67, 71, 73};

const int Width = 512;
const int Height = 512;
const int NumSamples = 200;
const int Strand = 3;

const int Offset[9][2] = {
    {0, 0},
    {0, 1},
    {0, -1},
    {1, 0},
    {-1, 0},
    {1, 1},
    {-1, -1},
    {1, -1},
    {-1, 1},
};

static Float RadicalInverse(int a, int base)
{
    const Float invBase = (Float)1 / (Float)base;
    int reversedDigits = 0;
    Float invBaseN = 1;
    while (a)
    {
        int next = a / base;
        int digit = a - next * base;
        reversedDigits = reversedDigits * base + digit;
        invBaseN *= invBase;
        a = next;
    }
    return reversedDigits * invBaseN;
}

int GetNthPrime(int dimension)
{
    return Primes[dimension];
}

Float Halton(int dimension, int index)
{
    return RadicalInverse(index, GetNthPrime(dimension));
}

Float Hammersley(int dimension, int index, int numSamples)
{
    if (dimension == 0)
        return index / (Float)numSamples;
    else
        return RadicalInverse(index, GetNthPrime(dimension - 1));
}

void WritePPM(const char *filename, const unsigned char *data, int width, int height)
{
    std::ofstream ofs;
    ofs.open(filename);
    ofs << "P5\n"
        << width << " " << height << "\n255\n";
    ofs.write((char *)data, width * height);
    ofs.close();
}

void SaveSample(unsigned char *pixels, int width, int height, Float x, Float y)
{
    int px = (int)(x * Width + 0.5);
    int py = (int)(y * Height + 0.5);
    for (int i = 0; i < 9; i++)
    {
        int col = std::max(std::min(px + Offset[i][0], Width - 1), 0);
        int row = std::max(std::min(py + Offset[i][1], Height - 1), 0);
        pixels[row * width + col] = 0;
    }
}

int main()
{
    unsigned char pixels[Height][Width];
    // generate halton sequence.
    memset(pixels, 0xff, sizeof(pixels));
    for (int i = 0; i < NumSamples; i++)
    {
        Float x = Halton(0, i);
        Float y = Halton(1, i);
        SaveSample(&pixels[0][0], Width, Height, x, y);
    }
    WritePPM("halton.ppm", &pixels[0][0], Width, Height);

    // generate hammersley sequence.
    memset(pixels, 0xff, sizeof(pixels));
    for (int i = 0; i < NumSamples; i++)
    {
        Float x = Hammersley(0, i, NumSamples);
        Float y = Hammersley(1, i, NumSamples);
        SaveSample(&pixels[0][0], Width, Height, x, y);
    }
    WritePPM("hammersley.ppm", &pixels[0][0], Width, Height);
    return 0;
}