# Laboratorio 04 – Programación Paralela con Threads

**Curso:** Tecnología de Objetos  
**Repositorio:** [salasvictor22/teo](https://github.com/salasvictor22/teo)  
**Lenguajes:** Java, C++ y Go  

---

## Descripción

Este laboratorio tiene como finalidad aplicar la **programación paralela** mediante el uso de **threads** (hilos) para resolver un problema de integración numérica utilizando el **método del trapecio**.

Se implementan tres versiones del programa en los lenguajes **Java**, **C++** y **Go**, y además se desarrolla una segunda versión para cada uno que utiliza un **Pool de Threads** para mejorar la eficiencia en la ejecución paralela.

El programa calcula el área bajo la curva de la función:

f(x) = 2x² + 3x + ½  
en el intervalo [a = 2, b = 20],  
empleando el método del trapecio desde n = 1 hasta N, deteniéndose cuando el valor hallado se repite.

---

## Estructura del Repositorio

```
/teo
│
├── TrapecioPool.java          # Versión con Pool de Threads (Java)
├── trapecioPool.cpp           # Versión con Pool de Threads (C++)
├── trapecioPool.go            # Versión con Pool de Threads (Go)
│
├── Trapecio.java       # Versión sin Pool (Java)
├── trapecio.cpp        # Versión sin Pool (C++)
├── trapecio.go         # Versión sin Pool (Go)
│
└── README.md                  # Descripción general del laboratorio
```

---

## Descripción de los Códigos

### Java
- Implementa el método del trapecio con y sin uso de **ExecutorService**.  
- La versión sin pool crea los hilos manualmente.  
- La versión con pool usa un grupo fijo de hilos que se reutilizan durante el cálculo.

### C++
- Implementación orientada a objetos usando clases y funciones.  
- La versión sin pool crea hilos independientes y los sincroniza con `join()`.  
- La versión con pool usa una clase **Pool** personalizada que administra los hilos en una cola de tareas.

### Go
- Usa **goroutines** y canales para manejar la concurrencia.  
- La versión sin pool lanza goroutines directamente.  
- La versión con pool crea un conjunto fijo de workers que procesan los trabajos enviados por canales.

---

## Ejecución de los Programas

### Java
```bash
javac TrapecioPool.java
java TrapecioPool 2 20 200000 8
```

### C++
```bash
g++ -O2 -std=c++17 -pthread trapecioPool.cpp -o trapecioPool.exe
trapecioPool.exe 2 20 200000 8
```

### Go
```bash
go run trapecioPool.go 2 20 200000 8
```

---

## Comentarios Finales

- El uso del **Pool de Threads** reduce el tiempo total de ejecución al reutilizar los hilos existentes.  
- **Go** maneja la concurrencia de manera más simple gracias a su modelo nativo de goroutines.  
- **C++** ofrece mayor control, pero requiere una implementación más detallada.  
- **Java** simplifica el manejo de hilos mediante su API de concurrencia.

---
