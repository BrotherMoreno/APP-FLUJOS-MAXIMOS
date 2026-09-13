// FlujoMaximo.cpp : archivo principal del proyecto.
// Este es el punto de entrada de la aplicacion: crea el formulario (Form1)
// y lo pone en pantalla.
//
// Si Visual Studio ya genero este archivo al crear el proyecto (con el
// nombre "FlujoMaximo.cpp", visible en el Explorador de soluciones junto a
// Form1.h), NO necesitas reemplazarlo: el que ya tienes debe verse igual o
// muy parecido a este. Solo usa este archivo si por error lo borraste o si
// tu proyecto no arranca.

#include "Form1.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace FlujoMaximo;

[STAThreadAttribute]
int main(array<String^>^ args)
{
    // Habilita los estilos visuales modernos de Windows para los controles.
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    // Crea la ventana principal (Form1) y la ejecuta.
    Application::Run(gcnew Form1());
    return 0;
}
