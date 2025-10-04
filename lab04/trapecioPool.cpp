#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <limits>
#include <algorithm>
#include <type_traits>

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

class Pool {
public:
    explicit Pool(size_t n) : detener(false) {
        n = max<size_t>(1, n);
        for (size_t i = 0; i < n; ++i) {
            trabajadores.emplace_back([this]() {
                for (;;) {
                    function<void()> tarea;
                    {
                        unique_lock<mutex> lk(mtx);
                        cv.wait(lk, [this]() { return detener || !cola.empty(); });
                        if (detener && cola.empty()) return;
                        tarea = move(cola.front());
                        cola.pop();
                    }
                    tarea();
                }
            });
        }
    }

    template <class F>
    auto enviar(F&& f) -> future<invoke_result_t<F&>> {
        using R = invoke_result_t<F&>;
        packaged_task<R()> task(forward<F>(f));
        auto fut = task.get_future();
        {
            unique_lock<mutex> lk(mtx);
            if (detener) throw runtime_error("pool detenido");
            cola.emplace([t = move(task)]() mutable { t(); });
        }
        cv.notify_one();
        return fut;
    }

    ~Pool() {
        {
            unique_lock<mutex> lk(mtx);
            detener = true;
        }
        cv.notify_all();
        for (auto& w : trabajadores) w.join();
    }

private:
    vector<thread> trabajadores;
    queue<function<void()>> cola;
    mutex mtx;
    condition_variable cv;
    bool detener;
};

class Integrador {
public:
    Integrador(const Funcion& f, size_t hilos)
        : f(f), pool(hilos), hilos(hilos) {}

    double integrar(double a, double b, long long n) {
        double h = (b - a) / static_cast<double>(n);
        long long interior = max(0LL, n - 1);
        long long base = interior / static_cast<long long>(hilos);
        long long resto = interior % static_cast<long long>(hilos);

        vector<future<double>> futs;
        futs.reserve(hilos);

        long long inicio = 1;
        for (size_t w = 0; w < hilos; ++w) {
            long long tam = base + (static_cast<long long>(w) < resto ? 1 : 0);
            long long fin = (tam == 0) ? (inicio - 1) : (inicio + tam - 1);

            auto tarea = [this, a, h, inicio, fin]() -> double {
                double suma = 0.0;
                for (long long i = inicio; i <= fin; ++i) {
                    double x = a + i * h;
                    suma += f.eval(x);
                }
                return suma;
            };

            futs.emplace_back(pool.enviar(tarea));
            inicio = fin + 1;
        }

        double suma = 0.0;
        for (auto& fu : futs) suma += fu.get();

        double fa = f.eval(a);
        double fb = f.eval(b);
        return h * (0.5 * fa + suma + 0.5 * fb);
    }

private:
    const Funcion& f;
    Pool pool;
    size_t hilos;
};

static string clave(double v) {
    ostringstream oss;
    oss.setf(std::ios::fixed);
    oss << setprecision(12) << v;
    return oss.str();
}

int main(int argc, char** argv) {
    double a = (argc >= 2) ? atof(argv[1]) : 2.0;
    double b = (argc >= 3) ? atof(argv[2]) : 20.0;
    long long Nmax = (argc >= 4) ? atoll(argv[3]) : 2000000LL;
    size_t hilos = (argc >= 5) ? static_cast<size_t>(atoll(argv[4]))
                               : max(1u, thread::hardware_concurrency());

    FuncionCuadratica f;
    Integrador integ(f, hilos);

    cout.setf(std::ios::fixed);
    cout << setprecision(12);

    string ultima;
    double valor = numeric_limits<double>::quiet_NaN();
    long long repetido = -1;

    auto t0 = chrono::steady_clock::now();
    for (long long n = 1; n <= Nmax; ++n) {
        double v = integ.integrar(a, b, n);
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
