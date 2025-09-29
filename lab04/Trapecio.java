import java.util.*;
import java.math.BigDecimal;
import java.math.RoundingMode;

interface Funcion {
    double evaluar(double x);
}

final class FuncionCuadratica implements Funcion {
    public double evaluar(double x) { return 2.0*x*x + 3.0*x + 0.5; }
}

class Tarea implements Runnable {
    private final Funcion f;
    private final double a;
    private final double h;
    private final long inicio, fin;
    private double resultado;

    Tarea(Funcion f, double a, double h, long inicio, long fin) {
        this.f = f; this.a = a; this.h = h; this.inicio = inicio; this.fin = fin;
    }
    public void run() {
        double suma = 0.0;
        for (long i = inicio; i <= fin; i++) {
            double x = a + i * h;
            suma += f.evaluar(x);
        }
        resultado = suma;
    }
    public double get() { return resultado; }
}

public class Trapecio {
    private static String clave(double v) {
        return new BigDecimal(v).setScale(12, RoundingMode.HALF_UP).toPlainString();
    }
    private static double integrar(Funcion f, double a, double b, long n, int hilos) throws Exception {
        double h = (b - a) / n;
        long interior = Math.max(0, n - 1);
        long base = interior / hilos;
        long resto = interior % hilos;
        List<Thread> ts = new ArrayList<>(hilos);
        List<Tarea> tareas = new ArrayList<>(hilos);
        long inicio = 1;
        for (int w = 0; w < hilos; w++) {
            long tam = base + (w < resto ? 1 : 0);
            long fin = (tam == 0) ? (inicio - 1) : (inicio + tam - 1);
            Tarea t = new Tarea(f, a, h, inicio, fin);
            tareas.add(t);
            Thread th = new Thread(t);
            ts.add(th);
            th.start();
            inicio = fin + 1;
        }
        for (Thread th : ts) th.join();
        double suma = 0.0;
        for (Tarea t : tareas) suma += t.get();
        double fa = f.evaluar(a), fb = f.evaluar(b);
        return h * (0.5 * fa + suma + 0.5 * fb);
    }
    public static void main(String[] args) throws Exception {
        double a = (args.length >= 1) ? Double.parseDouble(args[0]) : 2.0;
        double b = (args.length >= 2) ? Double.parseDouble(args[1]) : 20.0;
        long Nmax = (args.length >= 3) ? Long.parseLong(args[2]) : 2_000_000L;
        int hilos = (args.length >= 4) ? Integer.parseInt(args[3]) : Math.max(1, Runtime.getRuntime().availableProcessors());
        Funcion f = new FuncionCuadratica();
        String ultimaClave = null;
        double ultimoValor = Double.NaN;
        long repetido = -1;
        long n;
        long t0 = System.nanoTime();
        for (n = 1; n <= Nmax; n++) {
            double val = integrar(f, a, b, n, hilos);
            String k = clave(val);
            if (k.equals(ultimaClave)) { repetido = n; ultimoValor = val; break; }
            ultimaClave = k;
            ultimoValor = val;
            if (n <= 10 || n % 100000 == 0) {
                System.out.printf(Locale.US, "n=%d -> %.12f%n", n, val);
            }
        }
        long t1 = System.nanoTime();
        if (repetido > 0) System.out.printf(Locale.US, "Repetido en n=%d -> %.12f%n", repetido, ultimoValor);
        else System.out.printf(Locale.US, "Finalizado en n=%d -> %.12f%n", n - 1, ultimoValor);
        System.out.printf(Locale.US, "Tiempo: %.3f s%n", (t1 - t0)/1e9);
    }
}
