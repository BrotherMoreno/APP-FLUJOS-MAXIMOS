# 🚀 Aplicación de Flujo Máximo

Aplicación interactiva desarrollada para el curso de **Matemática Computacional**. Permite crear una red de flujo mediante una **matriz de capacidades**, elegir la **fuente** y el **sumidero**, y calcular el **flujo máximo** con el algoritmo **Edmonds-Karp** (Ford-Fulkerson con caminos aumentantes más cortos), mostrando el resultado de forma visual.

Funciona sin conexión a internet.

## ▶️ Uso

1. Ingresar el número de vértices.
2. Construir la matriz de capacidades (automática o manual).
3. Generar la red de flujo.
4. Seleccionar el vértice fuente y el sumidero.
5. Ejecutar el cálculo y visualizar el proceso.
6. Obtener el flujo máximo.

## 📦 Descargas

Cada vez que se publica una versión (etiqueta `vX.Y.Z`) los archivos aparecen en **Releases**. También se generan en cada `push` desde la pestaña **Actions → Compilar instaladores de Windows → Artifacts**:

| Archivo | Descripción |
| `FlujoMaximo-Setup-x.y.z.exe` | Instalador (asistente en español, accesos directos, desinstalador) |
| `FlujoMaximo-Portable-x.y.z.exe` | Un solo archivo, sin instalación |
| `FlujoMaximo-x.y.z-x64.msi` | Instalador MSI (instalación para todos los usuarios) |

> Los programas no están firmados digitalmente: Windows SmartScreen puede avisar. Pulsa **Más información → Ejecutar de todas formas**.

## 🛠️ Compilar

La compilación corre sola en GitHub Actions (gratis). Para hacerlo en tu PC (Windows, con Node.js 20+):

```bash
npm install
npm start        # ejecutar en modo desarrollo
npm run dist     # genera los .exe en la carpeta dist/
```

Para publicar una versión:

```bash
git tag v2.0.0
git push origin v2.0.0
```

## 📁 Estructura

```
main.js                       Ventana de escritorio (Electron)
www/                          Interfaz y lógica (index.html, logic.js, vendor/)
installer/FlujoMaximo.wxs     Definición del MSI (WiX 5)
.github/workflows/build.yml   Compilación automática
```

## 🎓 Proyecto académico

**Curso:** Matemática Computacional · **Proyecto:** Problema del Flujo Máximo  
**Autor:** Brother Moreno
