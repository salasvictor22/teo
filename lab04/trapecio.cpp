#include <iostream>
#include <vector>
#include <thread>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <limits>
#include <algorithm>

using namespace std;

struct Funcion {
    virtual double eval(double x) const = 0;
    virtual ~Funcion() = default;
};

struct FuncionCuadratica : Funcion {
    double eval(double x) const override {
        return 2.0 * x * x + 3.0 * x + 0.5;
    }
};

struct Tarea {
    const Funcion* f;
    double a;
    double h;
    long long inicio;
    long long fin;
    double resultado;

    void operator()() {
        double suma = 0.0;
        for (long long i = inicio; i <= fin; ++i) {
            double x = a + i * h;
            suma += f->eval(x);
        }
        resultado = suma;
    }
};

static string clave(double v) {
    ostringstream oss;
    oss.setf(std::ios::fixed);
    oss << setprecision(12) << v;
    return oss.str();
}

double integrar(const Funcion& f, double a, double b, long long n, size_t hilos) {
    double h = (b - a) / static_cast<double>(n);
    long long interior = max(0LL, n - 1);
    long long base = interior / static_cast<long long>(hilos);
    long long resto = interior % static_cast<long long>(hilos);

    vector<thread> th;
    th.reserve(hilos);
    vector<Tarea> tareas(hilos);

    long long inicio = 1;
    for (size_t w = 0; w < hilos; ++w) {
        long long tam = base + (static_cast<long long>(w) < resto ? 1 : 0);
        long long fin = (tam == 0) ? (inicio - 1) : (inicio + tam - 1);
        tareas[w] = Tarea{&f, a, h, inicio, fin, 0.0};
        th.emplace_back(ref(tareas[w]));
        inicio = fin + 1;
    }

    for (auto& t : th) t.join();

    double suma = 0.0;
    for (auto& t : tareas) suma += t.resultado;

    double fa = f.eval(a);
    double fb = f.eval(b);
    return h * (0.5 * fa + suma + 0.5 * fb);
}

int main(int argc, char** argv) {
    double a = (argc >= 2) ? atof(argv[1]) : 2.0;
    double b = (argc >= 3) ? atof(argv[2]) : 20.0;
    long long Nmax = (argc >= 4) ? atoll(argv[3]) : 2000000LL;
    size_t hilos = (argc >= 5) ? static_cast<size_t>(atoll(argv[4]))
                               : max(1u, thread::hardware_concurrency());

    FuncionCuadratica f;

    cout.setf(std::ios::fixed);
    cout << setprecision(12);

    string ultima;
    double valor = numeric_limits<double>::quiet_NaN();
    long long repetido = -1;

    auto t0 = chrono::steady_clock::now();
    for (long long n = 1; n <= Nmax; ++n) {
        double v = integrar(f, a, b, n, hilos);
        string k = clave(v);
        if (n == 1) {
            ultima = k;
            valor = v;
        } else {
            if (k == ultima) {
                repetido = n;
                valor = v;
                break;
            }
            ultima = k;
            valor = v;
        }
        if (n <= 10 || n % 100000 == 0) {
            cout << "n=" << n << " -> " << v << "\n";
        }
    }
    auto t1 = chrono::steady_clock::now();
    double s = chrono::duration<double>(t1 - t0).count();

    if (repetido > 0) {
        cout << "Repetido en n=" << repetido << " -> " << valor << "\n";
    } else {
        cout << "Finalizado -> " << valor << "\n";
    }
    cout << "Tiempo: " << s << " s\n";
    return 0;
}
