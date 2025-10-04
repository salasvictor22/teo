import java.util.*;
import java.util.concurrent.*;
import java.math.BigDecimal;
import java.math.RoundingMode;

interface Funcion {
    double evaluar(double x);
}

final class FuncionCuadratica implements Funcion {
    public double evaluar(double x) { return 2.0*x*x + 3.0*x + 0.5; }
}

final class TareaTrapecio implements Callable<Double> {
    private final Funcion f;
    private final double a;
    private final double h;
    private final long inicio, fin;

    TareaTrapecio(Funcion f, double a, double h, long inicio, long fin) {
        this.f = f; this.a = a; this.h = h; this.inicio = inicio; this.fin = fin;
    }

    @Override public Double call() {
        double suma = 0.0;
        for (long i = inicio; i <= fin; i++) {
            double x = a + i * h;
            suma += f.evaluar(x);
        }
        return suma;
    }
}

final class IntegradorTrapecio {
    private final Funcion f;
    private final ExecutorService pool;
    private final int hilos;

    IntegradorTrapecio(Funcion f, int hilos) {
        this.f = f;
        this.hilos = Math.max(1, hilos);
        this.pool = Executors.newFixedThreadPool(this.hilos);
    }

    public double integrar(double a, double b, long n) throws Exception {
        final double h = (b - a) / n;
        List<Future<Double>> futuros = new ArrayList<>(hilos);
        long interior = Math.max(0, n - 1);
        long base = interior / hilos;
        long resto = interior % hilos;
        long inicio = 1;
        for (int w = 0; w < hilos; w++) {
            long tam = base + (w < resto ? 1 : 0);
            long fin = (tam == 0) ? (inicio - 1) : (inicio + tam - 1);
            futuros.add(pool.submit(new TareaTrapecio(f, a, h, inicio, fin)));
            inicio = fin + 1;
        }
        double suma = 0.0;
        for (Future<Double> fu : futuros) suma += fu.get();
        double fa = f.evaluar(a);
        double fb = f.evaluar(b);
        return h * (0.5 * fa + suma + 0.5 * fb);
    }

    public void cerrar() { pool.shutdown(); }
}

public class TrapecioPool {
    private static String clave(double v) {
        return new BigDecimal(v).setScale(12, RoundingMode.HALF_UP).toPlainString();
    }

    public static void main(String[] args) throws Exception {
        double a = (args.length >= 1) ? Double.parseDouble(args[0]) : 2.0;
        double b = (args.length >= 2) ? Double.parseDouble(args[1]) : 20.0;
        long Nmax = (args.length >= 3) ? Long.parseLong(args[2]) : 2_000_000L;
        int hilos = (args.length >= 4) ? Integer.parseInt(args[3]) : Math.max(1, Runtime.getRuntime().availableProcessors());

        Funcion f = new FuncionCuadratica();
        IntegradorTrapecio integ = new IntegradorTrapecio(f, hilos);

        String ultimaClave = null;
        double ultimoValor = Double.NaN;
        long repetido = -1;
        long n;
        long t0 = System.nanoTime();
        for (n = 1; n <= Nmax; n++) {
            double val = integ.integrar(a, b, n);
            String k = clave(val);
            if (k.equals(ultimaClave)) { repetido = n; ultimoValor = val; break; }
            ultimaClave = k;
            ultimoValor = val;
            if (n <= 10 || n % 100000 == 0) {
                System.out.printf(Locale.US, "n=%d -> %.12f%n", n, val);
            }
        }
        long t1 = System.nanoTime();

        if (repetido > 0) {
            System.out.printf(Locale.US, "Repetido en n=%d -> %.12f%n", repetido, ultimoValor);
        } else {
            System.out.printf(Locale.US, "Finalizado en n=%d -> %.12f%n", n - 1, ultimoValor);
        }
        System.out.printf(Locale.US, "Tiempo: %.3f s%n", (t1 - t0)/1e9);
        integ.cerrar();
    }
}
