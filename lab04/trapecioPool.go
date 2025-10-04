package main

import (
	"context"
	"fmt"
	"math"
	"os"
	"runtime"
	"strconv"
	"sync"
	"time"
)

func f(x float64) float64 { return 2*x*x + 3*x + 0.5 }

func clave(v float64) string { return fmt.Sprintf("%.12f", v) }

type trabajo struct{ ini, fin int64; a, h float64 }

func worker(ctx context.Context, trabajos <-chan trabajo, wg *sync.WaitGroup, out chan<- float64) {
	defer wg.Done()
	for {
		select {
		case <-ctx.Done():
			return
		case t, ok := <-trabajos:
			if !ok {
				return
			}
			suma := 0.0
			for i := t.ini; i <= t.fin; i++ {
				x := t.a + float64(i)*t.h
				suma += f(x)
			}
			out <- suma
		}
	}
}

func integrar(a, b float64, n int64, hilos int) float64 {
	h := (b - a) / float64(n)
	interior := n - 1
	if interior < 0 {
		interior = 0
	}
	base := interior / int64(hilos)
	resto := interior % int64(hilos)
	trabajos := make(chan trabajo, hilos)
	out := make(chan float64, hilos)
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	var wg sync.WaitGroup
	for w := 0; w < hilos; w++ {
		wg.Add(1)
		go worker(ctx, trabajos, &wg, out)
	}
	ini := int64(1)
	for w := 0; w < hilos; w++ {
		tam := base
		if int64(w) < resto {
			tam++
		}
		fin := ini + tam - 1
		if tam == 0 {
			fin = ini - 1
		}
		trabajos <- trabajo{ini, fin, a, h}
		ini = fin + 1
	}
	close(trabajos)
	suma := 0.0
	for w := 0; w < hilos; w++ {
		suma += <-out
	}
	close(out)
	wg.Wait()
	fa, fb := f(a), f(b)
	return h * (0.5*fa + suma + 0.5*fb)
}

func main() {
	a, b, Nmax, hilos := 2.0, 20.0, int64(2000000), runtime.NumCPU()
	if len(os.Args) >= 2 {
		if v, e := strconv.ParseFloat(os.Args[1], 64); e == nil {
			a = v
		}
	}
	if len(os.Args) >= 3 {
		if v, e := strconv.ParseFloat(os.Args[2], 64); e == nil {
			b = v
		}
	}
	if len(os.Args) >= 4 {
		if v, e := strconv.ParseInt(os.Args[3], 10, 64); e == nil {
			Nmax = v
		}
	}
	if len(os.Args) >= 5 {
		if v, e := strconv.Atoi(os.Args[4]); e == nil && v > 0 {
			hilos = v
		}
	}
	ult := ""
	val := math.NaN()
	rep := int64(-1)
	t0 := time.Now()
	for n := int64(1); n <= Nmax; n++ {
		v := integrar(a, b, n, hilos)
		k := clave(v)
		if n == 1 {
			ult, val = k, v
		} else {
			if k == ult {
				rep, val = n, v
				break
			}
			ult, val = k, v
		}
		if n <= 10 || n%100000 == 0 {
			fmt.Printf("n=%d -> %.12f\n", n, v)
		}
	}
	elap := time.Since(t0).Seconds()
	if rep > 0 {
		fmt.Printf("Repetido en n=%d -> %.12f\n", rep, val)
	} else {
		fmt.Printf("Finalizado -> %.12f\n", val)
	}
	fmt.Printf("Tiempo: %.3f s\n", elap)
}
