#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

double exp_maclaurin(double x, long long N) {
    double sum = 1.0;   // член ряда для n = 0
    double term = 1.0;
    for (long long n = 1; n < N; n++) {
        term *= x / (double)n;
        sum += term;
    }
    return sum;
}

void run_correctness_tests(long long N_test) {
    double test_x[] = { -5.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 5.0 };
    int n_tests = sizeof(test_x) / sizeof(test_x[0]);
    const double eps = 1e-6;

    printf("--- ПРОВЕРКА ПРАВИЛЬНОСТИ (N = %lld) ---\n", N_test);
    printf("%8s %16s %16s %12s %6s\n", "x", "exp_maclaurin", "math.h exp", "abs_err", "test");

    for (int i = 0; i < n_tests; i++) {
        double x = test_x[i];
        double mine = exp_maclaurin(x, N_test);
        double ref  = exp(x);
        double abs_err = fabs(mine - ref);
        const char *verdict = (abs_err < eps) ? "OK" : "FAIL";
        printf("%8.2f %16.10f %16.10f %12.2e %6s\n", x, mine, ref, abs_err, verdict);
    }
    printf("\n");
}

int main(void) {
    SetConsoleOutputCP(CP_UTF8);

    const double x = 1.0;   // точка вычисления e^x

    run_correctness_tests(30);

    LARGE_INTEGER freq, t1, t2;
    QueryPerformanceFrequency(&freq);
    const double timer_resolution = 1.0 / (double)freq.QuadPart; // цена одного отсчета(сек)

    const double target_time   = 15.0;
    const double max_rel_error = 0.01;

    long long N = 100000000;
    double measured_time = 0.0;
    volatile double sink = 0.0;

    for (;;) {
        QueryPerformanceCounter(&t1);
        sink = exp_maclaurin(x, N);
        QueryPerformanceCounter(&t2);

        measured_time = (double)(t2.QuadPart - t1.QuadPart) / (double)freq.QuadPart;
        double rel_error = timer_resolution / measured_time;


        if (rel_error <= max_rel_error &&
            measured_time >= target_time * 0.9 &&
            measured_time <= target_time * 1.5) {
            break; // время близко к целевому и точность обеспечена
        }

        if (measured_time < 1e-9) measured_time = 1e-9;
        double scale = target_time / measured_time;
        N = (long long)((double)N * scale) + 1;
    }

    const int trials = 5;
    double best_time = 1e18;
    for (int k = 0; k < trials; k++) {
        QueryPerformanceCounter(&t1);
        sink = exp_maclaurin(x, N);
        QueryPerformanceCounter(&t2);
        double t = (double)(t2.QuadPart - t1.QuadPart) / (double)freq.QuadPart;
        if (t < best_time) best_time = t;
    }

    double rel_error_final = timer_resolution / best_time;

    printf("e^x = %.10f (эталон: %.10f)\n", sink, exp(x));
    printf("Подобранное N (число членов ряда): %lld\n", N);
    printf("Время работы программы (минимум из %d измерений): %.6f s\n", trials, best_time);
    printf("Относительная погрешность измерения: %.6f%%\n", rel_error_final * 100.0);

    return 0;
}