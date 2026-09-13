#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

double exp_maclaurin(double x, int terms) {
    double sum = 1.0;
    double term = 1.0;
    for (int n = 1; n < terms; n++) {
        term *= x / n;
        sum += term;
    }
    return sum;
}

void run_correctness_tests(int terms) {
    double test_x[] = { -5.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0, 5.0 };
    int n_tests = sizeof(test_x) / sizeof(test_x[0]);
    const double eps = 1e-6; // допустимое отклонение для теста

    printf("ПРОВЕРКА ПРАВИЛЬНОСТИ(terms = %d)\n", terms);
    printf("%8s %16s %16s %12s %6s\n", "x", "exp_maclaurin", "math.h exp", "abs_err", "test");

    for (int i = 0; i < n_tests; i++) {
        double x = test_x[i];
        double mine = exp_maclaurin(x, terms);
        double ref  = exp(x);
        double abs_err = fabs(mine - ref);
        const char *verdict = (abs_err < eps) ? "OK" : "FAIL";
        printf("%8.2f %16.10f %16.10f %12.2e %6s\n", x, mine, ref, abs_err, verdict);
    }
    printf("\n");
}

int main(void) {
    SetConsoleOutputCP(CP_UTF8);

    const double x = 1.0;   //точка вычисления e^x
    const int terms = 30;   //число членов ряда

    run_correctness_tests(terms);

    LARGE_INTEGER freq, t1, t2;
    QueryPerformanceFrequency(&freq);
    const double timer_resolution = 1.0 / (double)freq.QuadPart; //цена одного отсчета(сек)

    const double target_time   = 15.0;
    const double max_rel_error = 0.01;

    long long repeat = 1000;
    double measured_time = 0.0;
    volatile double sink = 0.0;

    for (;;) {
        QueryPerformanceCounter(&t1);
        for (long long i = 0; i < repeat; i++) {
            sink = exp_maclaurin(x, terms);
        }
        QueryPerformanceCounter(&t2);

        measured_time = (double)(t2.QuadPart - t1.QuadPart) / (double)freq.QuadPart;
        double rel_error = timer_resolution / measured_time;

        if (rel_error <= max_rel_error && measured_time >= target_time * 0.9) {
            break; // точность обеспечена и время близко к целевому
        }

        if (measured_time < target_time) {
            double scale = target_time / (measured_time > 0 ? measured_time : 1e-9);
            repeat = (long long)((double)repeat * scale) + 1;
        } else {
            repeat *= 2;
        }
    }

    //многократное измерение
    const int trials = 5;
    double best_time = 1e18;
    for (int k = 0; k < trials; k++) {
        QueryPerformanceCounter(&t1);
        for (long long i = 0; i < repeat; i++) {
            sink = exp_maclaurin(x, terms);
        }
        QueryPerformanceCounter(&t2);
        double t = (double)(t2.QuadPart - t1.QuadPart) / (double)freq.QuadPart;
        if (t < best_time) best_time = t;
    }

    double time_per_call   = best_time / (double)repeat;
    double rel_error_final = timer_resolution / best_time;

    printf("e^x = %.10f (эталон: %.10f)\n", sink, exp(x));
    printf("Время работы подпрограммы exp_maclaurin: %.9e s\n", time_per_call);
    printf("Относительная погрешность измерения: %.6f%%\n", rel_error_final * 100.0);

    return 0;
}