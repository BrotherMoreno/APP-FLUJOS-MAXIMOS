# 🚀 Aplicación de Flujo Máximo

Aplicación interactiva desarrollada para el curso de **Matemática Computacional**, cuyo objetivo es representar y resolver el **Problema del Flujo Máximo** mediante una red de flujo.

## 📌 ¿Qué hace la aplicación?

La aplicación permite crear una red de flujo y calcular la **cantidad máxima de flujo** que puede pasar desde un vértice de origen hasta un vértice de destino.

El usuario puede ingresar el número de vértices y construir la red mediante una **matriz de capacidades**. Luego, la aplicación representa gráficamente la red y permite seleccionar la **fuente** y el **sumidero**.

Finalmente, el sistema ejecuta el algoritmo de flujo máximo y muestra el resultado de manera visual.

## ⚙️ ¿Cómo funciona?

El funcionamiento de la aplicación se divide en los siguientes pasos:

### 1️⃣ Ingresar los vértices

El usuario indica la cantidad de vértices que tendrá la red.

La aplicación permite trabajar con:

**5 ≤ n ≤ 15**

### 2️⃣ Crear la matriz de capacidades

El programa permite construir la matriz de dos formas:

* **Automática:** genera las capacidades de las conexiones.
* **Manual:** el usuario ingresa las capacidades.

La matriz indica cuánto flujo puede pasar entre cada par de vértices.

### 3️⃣ Generar la red

A partir de la matriz, la aplicación crea y muestra la **red de flujo** gráficamente.

Cada nodo representa un vértice y cada conexión representa una posible dirección del flujo.

### 4️⃣ Seleccionar fuente y sumidero

El usuario selecciona:

* 🔵 **Fuente:** punto donde comienza el flujo.
* 🔴 **Sumidero:** punto donde termina el flujo.

### 5️⃣ Calcular el flujo máximo

La aplicación analiza la red y busca los caminos disponibles desde la fuente hasta el sumidero.

Durante este proceso se actualizan las capacidades residuales de las conexiones hasta encontrar el **flujo máximo posible**.

### 6️⃣ Mostrar el resultado

Al finalizar, la aplicación muestra el **valor del flujo máximo** y permite visualizar cómo se distribuyó el flujo dentro de la red.

## 🧮 ¿Qué problema resuelve?

El Problema del Flujo Máximo busca determinar la mayor cantidad de flujo que puede enviarse desde un punto de origen hasta un punto de destino dentro de una red, respetando la capacidad máxima de cada conexión.

Por ejemplo:

```text
        10
   A --------> B
   |           |
  5|           |8
   ↓           ↓
   C --------> D
        10
```

La aplicación analiza las diferentes rutas y determina cuánto flujo puede circular como máximo desde el origen hasta el destino.

## 🧠 Algoritmo

Para resolver el problema se utilizan conceptos de **teoría de grafos** y algoritmos de flujo máximo, como:

* Ford-Fulkerson
* Edmonds-Karp
* Caminos aumentantes
* Redes residuales

El algoritmo busca rutas disponibles, envía flujo por ellas y actualiza las capacidades de la red hasta alcanzar el máximo posible.

## 🎯 Objetivo del proyecto

El objetivo principal es aplicar conocimientos de **Matemática Computacional** para convertir un problema matemático en una aplicación interactiva que permita:

* Modelar una red de flujo.
* Representar gráficamente un grafo.
* Trabajar con matrices.
* Aplicar algoritmos de optimización.
* Visualizar el proceso de resolución.
* Obtener el flujo máximo de una red.

## 🌎 Aplicaciones en la vida real

El concepto de flujo máximo puede utilizarse para analizar diferentes tipos de redes, por ejemplo:

🚚 **Transporte:** determinar la cantidad máxima de productos que pueden circular por una red de rutas.

🌐 **Redes informáticas:** analizar el flujo máximo de información entre diferentes dispositivos.

💧 **Distribución de agua:** estudiar la cantidad máxima de agua que puede transportarse mediante una red de tuberías.

⚡ **Distribución de energía:** analizar capacidades dentro de una red de distribución.

## 💻 Tecnologías y conocimientos utilizados

* Programación
* Matemática Computacional
* Teoría de grafos
* Algoritmos
* Estructuras de datos
* Matrices
* Redes de flujo
* Optimización

## ▶️ Uso de la aplicación

1. Ingresar el número de vértices.
2. Seleccionar cómo construir la matriz.
3. Generar la red de flujo.
4. Seleccionar el vértice fuente.
5. Seleccionar el vértice sumidero.
6. Ejecutar el cálculo.
7. Visualizar el proceso.
8. Obtener el flujo máximo.

## 📸 Vista de la aplicación

Puedes agregar aquí las capturas de pantalla de tu aplicación para mostrar su funcionamiento.

## 🎓 Proyecto académico

**Curso:** Matemática Computacional
**Proyecto:** Problema del Flujo Máximo
**Área:** Algoritmos y modelamiento matemático

---

## 👨‍💻 Autor

**Brother Córdova**

Proyecto desarrollado con fines académicos para aplicar conceptos de Matemática Computacional en una solución interactiva.
