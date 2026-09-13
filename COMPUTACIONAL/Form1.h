#pragma once

namespace FlujoMaximo {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections::Generic;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;
    using namespace System::Drawing::Drawing2D;
    using namespace System::IO;
    using namespace System::Text;

  
    public ref class CambioArista
    {
    public:
        int desde, hasta;
        bool reversa;
        int residualAntes, residualDespues;
        int flujoAntes, flujoDespues;
        String^ nota;
    };

    public ref class PasoInfo
    {
    public:
        List<int>^ camino;
        List<CambioArista^>^ cambios;
        int cuelloBotella;
        array<int, 2>^ flujo;
        array<int, 2>^ residual;
        int flujoAcumulado;
        int flujoMax;
        int iteracion;
        bool esFinal;
        String^ titulo;
    };


    // Barra de progreso interactiva  (●━━━━●━━━━○)
    // Clic o arrastre cambia el paso y dispara el evento CambioPaso

    public ref class BarraProgreso : public Panel
    {
    public:
        event EventHandler^ CambioPaso;

        BarraProgreso()
        {
            total = 0;
            actual = 0;
            this->DoubleBuffered = true;
            this->Height = 36;
            this->Cursor = Cursors::Hand;
            this->BackColor = Color::FromArgb(24, 24, 38);
        }

        property int Total
        {
            int get() { return total; }
            void set(int v) { total = v; if (actual >= total) actual = Math::Max(0, total - 1); Invalidate(); }
        }
        property int Actual
        {
            int get() { return actual; }
            void set(int v) { actual = v; Invalidate(); }
        }

    protected:
        virtual void OnPaint(PaintEventArgs^ e) override
        {
            Panel::OnPaint(e);
            Graphics^ g = e->Graphics;
            g->SmoothingMode = SmoothingMode::AntiAlias;

            int margen = 22;
            int x0 = margen, x1 = this->Width - margen;
            int y = this->Height / 2;

            Pen^ pFondo = gcnew Pen(Color::FromArgb(69, 71, 90), 5.0f);
            pFondo->StartCap = LineCap::Round; pFondo->EndCap = LineCap::Round;
            g->DrawLine(pFondo, x0, y, x1, y);

            if (total <= 1) return;

            float paso = (float)(x1 - x0) / (float)(total - 1);
            float xa = x0 + paso * actual;

            Pen^ pAct = gcnew Pen(Color::FromArgb(250, 179, 135), 5.0f);
            pAct->StartCap = LineCap::Round; pAct->EndCap = LineCap::Round;
            if (actual > 0) g->DrawLine(pAct, (float)x0, (float)y, xa, (float)y);

            float radio = Math::Min(7.0f, Math::Max(3.0f, paso / 3.0f));
            SolidBrush^ bOn = gcnew SolidBrush(Color::FromArgb(250, 179, 135));
            SolidBrush^ bOff = gcnew SolidBrush(Color::FromArgb(49, 50, 68));
            Pen^ pBorde = gcnew Pen(Color::FromArgb(110, 114, 140), 1.5f);
            Pen^ pSel = gcnew Pen(Color::White, 2.0f);

            for (int i = 0; i < total; i++)
            {
                float cx = x0 + paso * i;
                g->FillEllipse(i <= actual ? bOn : bOff, cx - radio, y - radio, radio * 2, radio * 2);
                if (i == actual)
                    g->DrawEllipse(pSel, cx - radio - 2, y - radio - 2, radio * 2 + 4, radio * 2 + 4);
                else
                    g->DrawEllipse(pBorde, cx - radio, y - radio, radio * 2, radio * 2);
            }
        }

        virtual void OnMouseDown(MouseEventArgs^ e) override
        {
            Panel::OnMouseDown(e);
            Seleccionar(e->X);
        }
        virtual void OnMouseMove(MouseEventArgs^ e) override
        {
            Panel::OnMouseMove(e);
            if (e->Button == System::Windows::Forms::MouseButtons::Left) Seleccionar(e->X);
        }
        virtual void OnResize(EventArgs^ e) override
        {
            Panel::OnResize(e);
            Invalidate();
        }

    private:
        int total, actual;

        void Seleccionar(int x)
        {
            if (total <= 1) return;
            int margen = 22;
            float paso = (float)(this->Width - 2 * margen) / (float)(total - 1);
            int idx = (int)Math::Round((x - margen) / paso);
            if (idx < 0) idx = 0;
            if (idx > total - 1) idx = total - 1;
            if (idx != actual)
            {
                actual = idx;
                Invalidate();
                CambioPaso(this, EventArgs::Empty);
            }
        }
    };

    //  FORMULARIO PRINCIPAL

    public ref class Form1 : public System::Windows::Forms::Form
    {
    public:
        Form1(void)
        {
            n = 0;
            pasoActual = 0;
            fuente = 0;
            sumidero = 0;
            modo3D = true;
            modoMatriz = 0;
            rng = gcnew Random();
            pasos = gcnew List<PasoInfo^>();
            capacidad = nullptr;
            paginaLista = false;
            ultimoJson = nullptr;

            InitializeComponent();
            IniciarVista3D();
        }

    protected:
        ~Form1()
        {
            if (components)
            {
                delete components;
            }
        }

    private:
        System::ComponentModel::Container^ components;

        // Controles superiores 
        NumericUpDown^ numN;
        RadioButton^ radioAuto;
        RadioButton^ radioManual;
        Button^ btnGenerar;
        ComboBox^ cmbFuente;
        ComboBox^ cmbSumidero;
        Button^ btnCalcular;
        Button^ btn2D;
        Button^ btn3D;
        Button^ btnAjustar;

        //  Panel matriz 
        DataGridView^ gridMatriz;
        Button^ btnMatCap;
        Button^ btnMatFlujo;
        Button^ btnMatRes;

        //  Vista de la red
        WebBrowser^ web3D;

        //  Panel inferior 
        Button^ btnAnterior;
        Button^ btnSiguiente;
        Button^ btnReproducir;
        Button^ btnReiniciar;
        Label^ lblPaso;
        Label^ lblFlujoActual;
        BarraProgreso^ barra;
        TextBox^ txtLog;
        DataGridView^ gridCambios;
        Label^ lblCambios;
        System::Windows::Forms::Timer^ timerPlay;

        //  Datos del algoritmo 
        int n;
        array<int, 2>^ capacidad;
        List<PasoInfo^>^ pasos;
        int pasoActual;
        int fuente, sumidero;
        Random^ rng;

        //Estado de la interfaz 
        bool modo3D;        // true = 3D, false = 2D
        int modoMatriz;     // 0 = capacidad, 1 = flujo, 2 = residual
        bool paginaLista;
        String^ ultimoJson;

        //  UTILIDADES DE ESTILO

        static Color CFondo() { return Color::FromArgb(30, 30, 46); }
        static Color CPanel() { return Color::FromArgb(24, 24, 38); }
        static Color CPanel2() { return Color::FromArgb(36, 37, 54); }
        static Color CBorde() { return Color::FromArgb(58, 60, 82); }
        static Color CTexto() { return Color::FromArgb(205, 214, 244); }
        static Color CAzul() { return Color::FromArgb(137, 180, 250); }
        static Color CVerde() { return Color::FromArgb(166, 227, 161); }
        static Color CRosa() { return Color::FromArgb(243, 139, 168); }
        static Color CNaranja() { return Color::FromArgb(250, 179, 135); }
        static Color CGris() { return Color::FromArgb(49, 50, 68); }
        static Color COscuro() { return Color::FromArgb(17, 17, 27); }

        static Color Mezclar(Color c, int d)
        {
            return Color::FromArgb(Math::Min(255, (int)c.R + d), Math::Min(255, (int)c.G + d), Math::Min(255, (int)c.B + d));
        }

        static void Redondear(Control^ c, int radio)
        {
            GraphicsPath^ path = gcnew GraphicsPath();
            int d = radio * 2;
            System::Drawing::Rectangle r = System::Drawing::Rectangle(0, 0, c->Width, c->Height);
            path->AddArc(r.X, r.Y, d, d, 180, 90);
            path->AddArc(r.Right - d, r.Y, d, d, 270, 90);
            path->AddArc(r.Right - d, r.Bottom - d, d, d, 0, 90);
            path->AddArc(r.X, r.Bottom - d, d, d, 90, 90);
            path->CloseFigure();
            c->Region = gcnew System::Drawing::Region(path);
        }

        Button^ CrearBoton(String^ texto, Color fondo, Color frente, int ancho, int alto)
        {
            Button^ b = gcnew Button();
            b->Text = texto;
            b->Size = System::Drawing::Size(ancho, alto);
            b->BackColor = fondo;
            b->ForeColor = frente;
            b->FlatStyle = FlatStyle::Flat;
            b->FlatAppearance->BorderSize = 0;
            b->FlatAppearance->MouseOverBackColor = Mezclar(fondo, 25);
            b->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
            b->Cursor = Cursors::Hand;
            b->UseVisualStyleBackColor = false;
            Redondear(b, 8);
            return b;
        }

        void EstiloToggle(Button^ b, bool activo)
        {
            b->BackColor = activo ? CAzul() : CGris();
            b->ForeColor = activo ? COscuro() : CTexto();
            b->FlatAppearance->MouseOverBackColor = activo ? Mezclar(CAzul(), 20) : Mezclar(CGris(), 20);
        }

        Label^ CrearLabel(String^ texto, Color color, int x, int y)
        {
            Label^ l = gcnew Label();
            l->Text = texto;
            l->ForeColor = color;
            l->Location = Point(x, y);
            l->AutoSize = true;
            return l;
        }

        void EstiloGrid(DataGridView^ g)
        {
            g->BackgroundColor = CPanel2();
            g->GridColor = CBorde();
            g->BorderStyle = BorderStyle::None;
            g->AllowUserToAddRows = false;
            g->AllowUserToDeleteRows = false;
            g->AllowUserToResizeRows = false;
            g->RowHeadersWidth = 42;
            g->DefaultCellStyle->BackColor = CPanel2();
            g->DefaultCellStyle->ForeColor = Color::White;
            g->DefaultCellStyle->SelectionBackColor = CAzul();
            g->DefaultCellStyle->SelectionForeColor = Color::Black;
            g->ColumnHeadersDefaultCellStyle->BackColor = CPanel();
            g->ColumnHeadersDefaultCellStyle->ForeColor = CTexto();
            g->ColumnHeadersDefaultCellStyle->SelectionBackColor = CPanel();
            g->RowHeadersDefaultCellStyle->BackColor = CPanel();
            g->RowHeadersDefaultCellStyle->ForeColor = CTexto();
            g->RowHeadersDefaultCellStyle->SelectionBackColor = CPanel();
            g->EnableHeadersVisualStyles = false;
            g->ColumnHeadersBorderStyle = DataGridViewHeaderBorderStyle::Single;
            g->CellBorderStyle = DataGridViewCellBorderStyle::Single;
        }


        //  CONSTRUCCION DE LA INTERFAZ

        void InitializeComponent(void)
        {
            this->components = gcnew System::ComponentModel::Container();
            this->Text = L"Flujo M\u00E1ximo - Edmonds-Karp";
            this->ClientSize = System::Drawing::Size(1280, 800);
            this->MinimumSize = System::Drawing::Size(1100, 700);
            this->StartPosition = FormStartPosition::CenterScreen;
            this->WindowState = FormWindowState::Maximized;
            this->BackColor = CFondo();
            this->Font = gcnew System::Drawing::Font(L"Segoe UI", 9);

            timerPlay = gcnew System::Windows::Forms::Timer(this->components);
            timerPlay->Interval = 1400;
            timerPlay->Tick += gcnew EventHandler(this, &Form1::timerPlay_Tick);

            //PANEL DERECHO (red) 
         
            Panel^ panelDerecho = gcnew Panel();
            panelDerecho->Dock = DockStyle::Fill;
            panelDerecho->BackColor = CFondo();
            panelDerecho->Padding = System::Windows::Forms::Padding(6, 6, 6, 0);
            this->Controls->Add(panelDerecho);

            Panel^ marcoRed = gcnew Panel();
            marcoRed->Dock = DockStyle::Fill;
            marcoRed->BackColor = CBorde();
            marcoRed->Padding = System::Windows::Forms::Padding(1);
            panelDerecho->Controls->Add(marcoRed);

            web3D = gcnew WebBrowser();
            web3D->Dock = DockStyle::Fill;
            web3D->ScriptErrorsSuppressed = true;
            web3D->IsWebBrowserContextMenuEnabled = false;
            web3D->WebBrowserShortcutsEnabled = false;
            web3D->DocumentCompleted += gcnew WebBrowserDocumentCompletedEventHandler(this, &Form1::web3D_DocumentCompleted);
            marcoRed->Controls->Add(web3D);

            //PANEL IZQUIERDO (matriz) 
            Panel^ panelIzquierdo = gcnew Panel();
            panelIzquierdo->Dock = DockStyle::Left;
            panelIzquierdo->Width = 370;
            panelIzquierdo->BackColor = CFondo();
            panelIzquierdo->Padding = System::Windows::Forms::Padding(8, 6, 2, 0);
            this->Controls->Add(panelIzquierdo);

            Panel^ marcoMatriz = gcnew Panel();
            marcoMatriz->Dock = DockStyle::Fill;
            marcoMatriz->BackColor = CPanel();
            marcoMatriz->Padding = System::Windows::Forms::Padding(6);
            panelIzquierdo->Controls->Add(marcoMatriz);

            gridMatriz = gcnew DataGridView();
            gridMatriz->Dock = DockStyle::Fill;
            EstiloGrid(gridMatriz);
            gridMatriz->CellEndEdit += gcnew DataGridViewCellEventHandler(this, &Form1::gridMatriz_CellEndEdit);
            marcoMatriz->Controls->Add(gridMatriz);

            Panel^ panelModos = gcnew Panel();
            panelModos->Dock = DockStyle::Top;
            panelModos->Height = 40;
            panelModos->BackColor = CPanel();
            marcoMatriz->Controls->Add(panelModos);

            btnMatCap = CrearBoton(L"Capacidad", CAzul(), COscuro(), 100, 28);
            btnMatCap->Location = Point(0, 6);
            btnMatCap->Click += gcnew EventHandler(this, &Form1::btnMatCap_Click);
            panelModos->Controls->Add(btnMatCap);

            btnMatFlujo = CrearBoton(L"Flujo", CGris(), CTexto(), 100, 28);
            btnMatFlujo->Location = Point(108, 6);
            btnMatFlujo->Click += gcnew EventHandler(this, &Form1::btnMatFlujo_Click);
            panelModos->Controls->Add(btnMatFlujo);

            btnMatRes = CrearBoton(L"Residual", CGris(), CTexto(), 100, 28);
            btnMatRes->Location = Point(216, 6);
            btnMatRes->Click += gcnew EventHandler(this, &Form1::btnMatRes_Click);
            panelModos->Controls->Add(btnMatRes);

            Label^ lblMatriz = gcnew Label();
            lblMatriz->Text = L"MATRIZ  (editable en modo Capacidad)";
            lblMatriz->ForeColor = CTexto();
            lblMatriz->Font = gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold);
            lblMatriz->Dock = DockStyle::Top;
            lblMatriz->Height = 28;
            lblMatriz->Padding = System::Windows::Forms::Padding(0, 4, 0, 0);
            marcoMatriz->Controls->Add(lblMatriz);

            // PANEL INFERIOR
            Panel^ panelInferior = gcnew Panel();
            panelInferior->Dock = DockStyle::Bottom;
            panelInferior->Height = 262;
            panelInferior->BackColor = CPanel();
            panelInferior->Padding = System::Windows::Forms::Padding(8, 0, 8, 6);
            this->Controls->Add(panelInferior);

            // --- zona de informacion  ---
            Panel^ panelInfo = gcnew Panel();
            panelInfo->Dock = DockStyle::Fill;
            panelInfo->BackColor = CPanel();
            panelInferior->Controls->Add(panelInfo);

            Panel^ panelTabla = gcnew Panel();
            panelTabla->Dock = DockStyle::Fill;
            panelTabla->BackColor = CPanel();
            panelTabla->Padding = System::Windows::Forms::Padding(8, 0, 0, 0);
            panelInfo->Controls->Add(panelTabla);

            gridCambios = gcnew DataGridView();
            gridCambios->Dock = DockStyle::Fill;
            EstiloGrid(gridCambios);
            gridCambios->ReadOnly = true;
            gridCambios->RowHeadersVisible = false;
            gridCambios->AllowUserToResizeColumns = false;
            gridCambios->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            gridCambios->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
            gridCambios->MultiSelect = false;
            gridCambios->ColumnHeadersHeight = 26;
            gridCambios->Columns->Add(L"colArista", L"Arista");
            gridCambios->Columns->Add(L"colResidual", L"Residual");
            gridCambios->Columns->Add(L"colFlujo", L"Flujo");
            gridCambios->Columns->Add(L"colNota", L"Nota");
            gridCambios->Columns[0]->FillWeight = 26;
            gridCambios->Columns[1]->FillWeight = 20;
            gridCambios->Columns[2]->FillWeight = 20;
            gridCambios->Columns[3]->FillWeight = 34;
            panelTabla->Controls->Add(gridCambios);

            lblCambios = gcnew Label();
            lblCambios->Text = L"ACTUALIZACI\u00D3N DE RESIDUALES Y FLUJO";
            lblCambios->ForeColor = CAzul();
            lblCambios->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
            lblCambios->Dock = DockStyle::Top;
            lblCambios->Height = 22;
            lblCambios->Padding = System::Windows::Forms::Padding(2, 3, 0, 0);
            panelTabla->Controls->Add(lblCambios);

            Panel^ panelExp = gcnew Panel();
            panelExp->Dock = DockStyle::Left;
            panelExp->Width = 470;
            panelExp->BackColor = CPanel();
            panelInfo->Controls->Add(panelExp);

            txtLog = gcnew TextBox();
            txtLog->Multiline = true;
            txtLog->ReadOnly = true;
            txtLog->ScrollBars = ScrollBars::Vertical;
            txtLog->Dock = DockStyle::Fill;
            txtLog->BackColor = CPanel2();
            txtLog->ForeColor = Color::White;
            txtLog->BorderStyle = BorderStyle::FixedSingle;
            txtLog->Font = gcnew System::Drawing::Font(L"Consolas", 9.5f);
            panelExp->Controls->Add(txtLog);

            Label^ lblExp = gcnew Label();
            lblExp->Text = L"INFORMACI\u00D3N DEL PASO ACTUAL";
            lblExp->ForeColor = CAzul();
            lblExp->Font = gcnew System::Drawing::Font(L"Segoe UI", 9, FontStyle::Bold);
            lblExp->Dock = DockStyle::Top;
            lblExp->Height = 22;
            lblExp->Padding = System::Windows::Forms::Padding(2, 3, 0, 0);
            panelExp->Controls->Add(lblExp);

            // --- barra de progreso ---
            barra = gcnew BarraProgreso();
            barra->Dock = DockStyle::Top;
            barra->CambioPaso += gcnew EventHandler(this, &Form1::barra_CambioPaso);
            panelInferior->Controls->Add(barra);

            // --- controles de navegacion ---
            Panel^ panelNav = gcnew Panel();
            panelNav->Dock = DockStyle::Top;
            panelNav->Height = 44;
            panelNav->BackColor = CPanel();
            panelInferior->Controls->Add(panelNav);

            btnAnterior = CrearBoton(L"\u25C0  Anterior", CGris(), CTexto(), 110, 30);
            btnAnterior->Location = Point(0, 7);
            btnAnterior->Enabled = false;
            btnAnterior->Click += gcnew EventHandler(this, &Form1::btnAnterior_Click);
            panelNav->Controls->Add(btnAnterior);

            btnSiguiente = CrearBoton(L"Siguiente  \u25B6", CGris(), CTexto(), 110, 30);
            btnSiguiente->Location = Point(118, 7);
            btnSiguiente->Enabled = false;
            btnSiguiente->Click += gcnew EventHandler(this, &Form1::btnSiguiente_Click);
            panelNav->Controls->Add(btnSiguiente);

            btnReproducir = CrearBoton(L"\u25B6 Reproducir", CVerde(), COscuro(), 120, 30);
            btnReproducir->Location = Point(236, 7);
            btnReproducir->Enabled = false;
            btnReproducir->Click += gcnew EventHandler(this, &Form1::btnReproducir_Click);
            panelNav->Controls->Add(btnReproducir);

            btnReiniciar = CrearBoton(L"Reiniciar", CGris(), CTexto(), 90, 30);
            btnReiniciar->Location = Point(364, 7);
            btnReiniciar->Enabled = false;
            btnReiniciar->Click += gcnew EventHandler(this, &Form1::btnReiniciar_Click);
            panelNav->Controls->Add(btnReiniciar);

            FlowLayoutPanel^ flowNav = gcnew FlowLayoutPanel();
            flowNav->Dock = DockStyle::Right;
            flowNav->AutoSize = true;
            flowNav->FlowDirection = FlowDirection::RightToLeft;
            flowNav->WrapContents = false;
            flowNav->BackColor = CPanel();
            panelNav->Controls->Add(flowNav);

            lblFlujoActual = gcnew Label();
            lblFlujoActual->Text = L"Flujo actual: -";
            lblFlujoActual->ForeColor = CVerde();
            lblFlujoActual->Font = gcnew System::Drawing::Font(L"Segoe UI", 12, FontStyle::Bold);
            lblFlujoActual->AutoSize = true;
            lblFlujoActual->Margin = System::Windows::Forms::Padding(16, 10, 6, 0);
            flowNav->Controls->Add(lblFlujoActual);

            lblPaso = gcnew Label();
            lblPaso->Text = L"";
            lblPaso->ForeColor = CNaranja();
            lblPaso->Font = gcnew System::Drawing::Font(L"Segoe UI", 10, FontStyle::Bold);
            lblPaso->AutoSize = true;
            lblPaso->Margin = System::Windows::Forms::Padding(10, 12, 10, 0);
            flowNav->Controls->Add(lblPaso);

            // ======================= PANEL SUPERIOR (configuracion) =======================
            Panel^ panelSuperior = gcnew Panel();
            panelSuperior->Dock = DockStyle::Top;
            panelSuperior->Height = 96;
            panelSuperior->BackColor = CPanel();
            this->Controls->Add(panelSuperior);

            Label^ lblTitulo = gcnew Label();
            lblTitulo->Text = L"Problema del Flujo M\u00E1ximo  \u2014  Algoritmo de Edmonds-Karp";
            lblTitulo->ForeColor = CAzul();
            lblTitulo->Font = gcnew System::Drawing::Font(L"Segoe UI", 14, FontStyle::Bold);
            lblTitulo->Location = Point(15, 10);
            lblTitulo->AutoSize = true;
            panelSuperior->Controls->Add(lblTitulo);

            // selector 2D / 3D + ajustar vista (arriba a la derecha)
            FlowLayoutPanel^ flowVista = gcnew FlowLayoutPanel();
            flowVista->Dock = DockStyle::Right;
            flowVista->AutoSize = true;
            flowVista->FlowDirection = FlowDirection::RightToLeft;
            flowVista->WrapContents = false;
            flowVista->BackColor = CPanel();
            flowVista->Padding = System::Windows::Forms::Padding(0, 10, 12, 0);
            panelSuperior->Controls->Add(flowVista);

            btnAjustar = CrearBoton(L"Ajustar vista", CGris(), CTexto(), 100, 30);
            btnAjustar->Margin = System::Windows::Forms::Padding(12, 0, 0, 0);
            btnAjustar->Click += gcnew EventHandler(this, &Form1::btnAjustar_Click);
            flowVista->Controls->Add(btnAjustar);

            btn3D = CrearBoton(L"3D", CAzul(), COscuro(), 60, 30);
            btn3D->Margin = System::Windows::Forms::Padding(4, 0, 0, 0);
            btn3D->Click += gcnew EventHandler(this, &Form1::btn3D_Click);
            flowVista->Controls->Add(btn3D);

            btn2D = CrearBoton(L"2D", CGris(), CTexto(), 60, 30);
            btn2D->Margin = System::Windows::Forms::Padding(0, 0, 0, 0);
            btn2D->Click += gcnew EventHandler(this, &Form1::btn2D_Click);
            flowVista->Controls->Add(btn2D);

            Label^ lblVista = CrearLabel(L"Vista:", CTexto(), 0, 0);
            lblVista->Margin = System::Windows::Forms::Padding(0, 7, 8, 0);
            flowVista->Controls->Add(lblVista);

            // fila de controles
            int y = 56;
            panelSuperior->Controls->Add(CrearLabel(L"V\u00E9rtices (n):", CTexto(), 15, y + 3));

            numN = gcnew NumericUpDown();
            numN->Minimum = 5;
            numN->Maximum = 15;
            numN->Value = 6;
            numN->Location = Point(100, y);
            numN->Width = 55;
            numN->BackColor = CPanel2();
            numN->ForeColor = Color::White;
            numN->BorderStyle = BorderStyle::FixedSingle;
            panelSuperior->Controls->Add(numN);

            radioAuto = gcnew RadioButton();
            radioAuto->Text = L"Generaci\u00F3n autom\u00E1tica";
            radioAuto->ForeColor = CTexto();
            radioAuto->Location = Point(175, y + 1);
            radioAuto->AutoSize = true;
            radioAuto->Checked = true;
            panelSuperior->Controls->Add(radioAuto);

            radioManual = gcnew RadioButton();
            radioManual->Text = L"Ingreso manual";
            radioManual->ForeColor = CTexto();
            radioManual->Location = Point(340, y + 1);
            radioManual->AutoSize = true;
            panelSuperior->Controls->Add(radioManual);

            btnGenerar = CrearBoton(L"Generar matriz", CAzul(), COscuro(), 130, 30);
            btnGenerar->Location = Point(470, y - 3);
            btnGenerar->Click += gcnew EventHandler(this, &Form1::btnGenerar_Click);
            panelSuperior->Controls->Add(btnGenerar);

            panelSuperior->Controls->Add(CrearLabel(L"Fuente:", CVerde(), 625, y + 3));

            cmbFuente = gcnew ComboBox();
            cmbFuente->Location = Point(680, y);
            cmbFuente->Width = 62;
            cmbFuente->DropDownStyle = ComboBoxStyle::DropDownList;
            cmbFuente->FlatStyle = FlatStyle::Flat;
            cmbFuente->BackColor = CPanel2();
            cmbFuente->ForeColor = Color::White;
            cmbFuente->SelectedIndexChanged += gcnew EventHandler(this, &Form1::cmb_SelectedIndexChanged);
            panelSuperior->Controls->Add(cmbFuente);

            panelSuperior->Controls->Add(CrearLabel(L"Sumidero:", CRosa(), 760, y + 3));

            cmbSumidero = gcnew ComboBox();
            cmbSumidero->Location = Point(830, y);
            cmbSumidero->Width = 62;
            cmbSumidero->DropDownStyle = ComboBoxStyle::DropDownList;
            cmbSumidero->FlatStyle = FlatStyle::Flat;
            cmbSumidero->BackColor = CPanel2();
            cmbSumidero->ForeColor = Color::White;
            cmbSumidero->SelectedIndexChanged += gcnew EventHandler(this, &Form1::cmb_SelectedIndexChanged);
            panelSuperior->Controls->Add(cmbSumidero);

            btnCalcular = CrearBoton(L"Calcular Flujo M\u00E1ximo", CVerde(), COscuro(), 175, 30);
            btnCalcular->Location = Point(912, y - 3);
            btnCalcular->Click += gcnew EventHandler(this, &Form1::btnCalcular_Click);
            panelSuperior->Controls->Add(btnCalcular);
        }

        //  EVENTOS DE LA INTERFAZ


        void btnGenerar_Click(Object^ sender, EventArgs^ e)
        {
            timerPlay->Stop();
            btnReproducir->Text = L"\u25B6 Reproducir";

            n = (int)numN->Value;
            capacidad = gcnew array<int, 2>(n, n);

            if (radioAuto->Checked)
            {
      
                double densidad = Math::Min(0.40, 4.5 / (n - 1));

                for (int i = 0; i < n; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        if (i == j) { capacidad[i, j] = 0; continue; }
                        if (rng->NextDouble() < densidad)
                            capacidad[i, j] = rng->Next(3, 26);
                        else
                            capacidad[i, j] = 0;
                    }
                }
                for (int i = 0; i < n; i++)
                {
                    bool salida = false, entrada = false;
                    for (int j = 0; j < n; j++)
                    {
                        if (capacidad[i, j] > 0) salida = true;
                        if (capacidad[j, i] > 0) entrada = true;
                    }
                    if (!salida && i != n - 1)
                    {
                        int j = (i + 1) % n; if (j == i) j = (j + 1) % n;
                        capacidad[i, j] = rng->Next(3, 26);
                    }
                    if (!entrada && i != 0)
                    {
                        int j = (i - 1 + n) % n; if (j == i) j = (j - 1 + n) % n;
                        capacidad[j, i] = rng->Next(3, 26);
                    }
                }
            }

            pasos = gcnew List<PasoInfo^>();
            pasoActual = 0;
            modoMatriz = 0;
            ActualizarEstiloModosMatriz();

            ConstruirGrid();
            ConstruirCombos();
            ReiniciarNavegacion();

            txtLog->Text = radioManual->Checked
                ? L"INGRESO MANUAL" + Environment::NewLine + Environment::NewLine +
                L"Completa la matriz de capacidades en la grilla de la izquierda (modo Capacidad)." + Environment::NewLine +
                L"La red se actualiza al terminar de editar cada celda." + Environment::NewLine +
                L"Luego selecciona fuente y sumidero y presiona 'Calcular Flujo M\u00E1ximo'."
                : L"MATRIZ GENERADA" + Environment::NewLine + Environment::NewLine +
                L"La red de flujo se muestra a la derecha (puedes editar la matriz si lo deseas)." + Environment::NewLine +
                L"Selecciona fuente y sumidero y presiona 'Calcular Flujo M\u00E1ximo'.";

            MostrarRedPrevia();
        }

        void btnCalcular_Click(Object^ sender, EventArgs^ e)
        {
            if (capacidad == nullptr || n <= 0)
            {
                MessageBox::Show(L"Primero genera la matriz de capacidades.", L"Aviso",
                    MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            if (cmbFuente->SelectedIndex < 0 || cmbSumidero->SelectedIndex < 0)
            {
                MessageBox::Show(L"Selecciona el v\u00E9rtice fuente y el sumidero.", L"Aviso",
                    MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            if (cmbFuente->SelectedIndex == cmbSumidero->SelectedIndex)
            {
                MessageBox::Show(L"La fuente y el sumidero deben ser distintos.", L"Aviso",
                    MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            timerPlay->Stop();
            btnReproducir->Text = L"\u25B6 Reproducir";

            if (modoMatriz == 0)
                LeerGridACapacidad();

            fuente = cmbFuente->SelectedIndex;
            sumidero = cmbSumidero->SelectedIndex;

            pasos = gcnew List<PasoInfo^>();
            EjecutarEdmondsKarp(capacidad, n, fuente, sumidero, pasos);

            pasoActual = 0;
            barra->Total = pasos->Count;
            btnReproducir->Enabled = true;
            btnReiniciar->Enabled = true;
            ActualizarVistaPaso();
        }

        void btnAnterior_Click(Object^ sender, EventArgs^ e)
        {
            if (pasoActual > 0) { pasoActual--; ActualizarVistaPaso(); }
        }

        void btnSiguiente_Click(Object^ sender, EventArgs^ e)
        {
            if (pasos != nullptr && pasoActual < pasos->Count - 1) { pasoActual++; ActualizarVistaPaso(); }
        }

        void btnReproducir_Click(Object^ sender, EventArgs^ e)
        {
            if (pasos == nullptr || pasos->Count == 0) return;
            if (timerPlay->Enabled)
            {
                timerPlay->Stop();
                btnReproducir->Text = L"\u25B6 Reproducir";
                return;
            }
            if (pasoActual >= pasos->Count - 1)
            {
                pasoActual = 0;
                ActualizarVistaPaso();
            }
            btnReproducir->Text = L"\u275A\u275A Pausar";
            timerPlay->Start();
        }

        void btnReiniciar_Click(Object^ sender, EventArgs^ e)
        {
            timerPlay->Stop();
            btnReproducir->Text = L"\u25B6 Reproducir";
            if (pasos == nullptr || pasos->Count == 0) return;
            pasoActual = 0;
            ActualizarVistaPaso();
        }

        void timerPlay_Tick(Object^ sender, EventArgs^ e)
        {
            if (pasos == nullptr || pasoActual >= pasos->Count - 1)
            {
                timerPlay->Stop();
                btnReproducir->Text = L"\u25B6 Reproducir";
                return;
            }
            pasoActual++;
            ActualizarVistaPaso();
            if (pasoActual >= pasos->Count - 1)
            {
                timerPlay->Stop();
                btnReproducir->Text = L"\u25B6 Reproducir";
            }
        }

        void barra_CambioPaso(Object^ sender, EventArgs^ e)
        {
            if (pasos == nullptr || pasos->Count == 0) return;
            timerPlay->Stop();
            btnReproducir->Text = L"\u25B6 Reproducir";
            pasoActual = barra->Actual;
            ActualizarVistaPaso();
        }

        void btn2D_Click(Object^ sender, EventArgs^ e) { CambiarModoVista(false); }
        void btn3D_Click(Object^ sender, EventArgs^ e) { CambiarModoVista(true); }

        void btnAjustar_Click(Object^ sender, EventArgs^ e)
        {
            InvocarJs(L"ajustarVista", nullptr);
        }

        void btnMatCap_Click(Object^ sender, EventArgs^ e) { CambiarModoMatriz(0); }
        void btnMatFlujo_Click(Object^ sender, EventArgs^ e) { CambiarModoMatriz(1); }
        void btnMatRes_Click(Object^ sender, EventArgs^ e) { CambiarModoMatriz(2); }

        void cmb_SelectedIndexChanged(Object^ sender, EventArgs^ e)
        {
  
            if (capacidad != nullptr && (pasos == nullptr || pasos->Count == 0))
                MostrarRedPrevia();
        }

        void gridMatriz_CellEndEdit(Object^ sender, DataGridViewCellEventArgs^ e)
        {
            if (modoMatriz != 0 || capacidad == nullptr) return;
            LeerGridACapacidad();

            if (pasos != nullptr && pasos->Count > 0)
            {
                timerPlay->Stop();
                btnReproducir->Text = L"\u25B6 Reproducir";
                pasos = gcnew List<PasoInfo^>();
                pasoActual = 0;
                ReiniciarNavegacion();
                txtLog->Text = L"La matriz cambi\u00F3. Presiona 'Calcular Flujo M\u00E1ximo' para recalcular.";
            }
            MostrarRedPrevia();
        }

        void web3D_DocumentCompleted(Object^ sender, WebBrowserDocumentCompletedEventArgs^ e)
        {
            paginaLista = true;
            InvocarJs(L"setModo", gcnew array<Object^> { modo3D ? L"3d" : L"2d" });
            if (ultimoJson != nullptr)
                InvocarJs(L"actualizarGrafo", gcnew array<Object^> { ultimoJson });
        }

    
        //  LOGICA AUXILIAR DE INTERFAZ

        void ReiniciarNavegacion()
        {
            barra->Total = 0;
            barra->Actual = 0;
            btnAnterior->Enabled = false;
            btnSiguiente->Enabled = false;
            btnReproducir->Enabled = false;
            btnReiniciar->Enabled = false;
            lblPaso->Text = L"";
            lblFlujoActual->Text = L"Flujo actual: -";
            lblFlujoActual->ForeColor = CVerde();
            gridCambios->Rows->Clear();
        }

        void CambiarModoVista(bool a3D)
        {
            modo3D = a3D;
            EstiloToggle(btn2D, !modo3D);
            EstiloToggle(btn3D, modo3D);
            InvocarJs(L"setModo", gcnew array<Object^> { modo3D ? L"3d" : L"2d" });
        }

        void ActualizarEstiloModosMatriz()
        {
            EstiloToggle(btnMatCap, modoMatriz == 0);
            EstiloToggle(btnMatFlujo, modoMatriz == 1);
            EstiloToggle(btnMatRes, modoMatriz == 2);
        }

        void CambiarModoMatriz(int m)
        {
            if (m == modoMatriz) return;
            if (modoMatriz == 0 && capacidad != nullptr) LeerGridACapacidad();
            modoMatriz = m;
            ActualizarEstiloModosMatriz();
            RefrescarMatriz();
        }

  
        PasoInfo^ PasoVigente()
        {
            if (pasos != nullptr && pasos->Count > 0) return pasos[pasoActual];
            PasoInfo^ p = gcnew PasoInfo();
            p->camino = gcnew List<int>();
            p->cambios = gcnew List<CambioArista^>();
            p->cuelloBotella = 0;
            p->flujo = gcnew array<int, 2>(n, n);
            p->residual = (array<int, 2>^)capacidad->Clone();
            p->flujoAcumulado = 0;
            p->flujoMax = 0;
            p->iteracion = 0;
            p->esFinal = false;
            p->titulo = L"Red de flujo (sin flujo asignado)";
            return p;
        }

        void MostrarRedPrevia()
        {
            if (capacidad == nullptr || n <= 0) return;
            if (cmbFuente->SelectedIndex < 0 || cmbSumidero->SelectedIndex < 0) return;
            fuente = cmbFuente->SelectedIndex;
            sumidero = cmbSumidero->SelectedIndex;
            EnviarEstado(PasoVigente(), L"");
            if (modoMatriz != 0) RefrescarMatriz();
        }

        void LeerGridACapacidad()
        {
            if (capacidad == nullptr || gridMatriz->Rows->Count < n) return;
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    if (i == j) { capacidad[i, j] = 0; continue; }
                    Object^ val = gridMatriz->Rows[i]->Cells[j]->Value;
                    int c = 0;
                    if (val != nullptr)
                        Int32::TryParse(val->ToString(), c);
                    if (c < 0) c = 0;
                    capacidad[i, j] = c;
                    gridMatriz->Rows[i]->Cells[j]->Value = c;
                }
            }
        }

        void ConstruirGrid()
        {
            gridMatriz->Rows->Clear();
            gridMatriz->Columns->Clear();
            gridMatriz->ColumnCount = n;
            for (int j = 0; j < n; j++)
            {
                gridMatriz->Columns[j]->Name = L"v" + j.ToString();
                gridMatriz->Columns[j]->HeaderText = L"v" + j.ToString();
                gridMatriz->Columns[j]->Width = 42;
                gridMatriz->Columns[j]->SortMode = DataGridViewColumnSortMode::NotSortable;
            }
            for (int i = 0; i < n; i++)
            {
                gridMatriz->Rows->Add();
                gridMatriz->Rows[i]->HeaderCell->Value = L"v" + i.ToString();
            }
            RefrescarMatriz();
        }

        void RefrescarMatriz()
        {
            if (capacidad == nullptr || n <= 0 || gridMatriz->Rows->Count < n) return;
            PasoInfo^ p = PasoVigente();

            gridMatriz->ReadOnly = (modoMatriz != 0);

            // celdas afectadas en este paso (fisicas: donde hay capacidad)
            Dictionary<int, bool>^ resaltadas = gcnew Dictionary<int, bool>();
            for (int k = 0; k < p->cambios->Count; k++)
            {
                CambioArista^ c = p->cambios[k];
                int a = c->reversa ? c->hasta : c->desde;
                int b = c->reversa ? c->desde : c->hasta;
                resaltadas[a * n + b] = true;
                if (modoMatriz == 2) resaltadas[c->hasta * n + c->desde] = true; 
            }

            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    DataGridViewCell^ celda = gridMatriz->Rows[i]->Cells[j];
                    int valor;
                    if (modoMatriz == 0) valor = capacidad[i, j];
                    else if (modoMatriz == 1) valor = p->flujo[i, j];
                    else valor = p->residual[i, j];
                    celda->Value = valor;

                    if (i == j)
                    {
                        celda->ReadOnly = true;
                        celda->Style->BackColor = Color::FromArgb(60, 60, 75);
                        celda->Style->ForeColor = Color::FromArgb(120, 120, 140);
                    }
                    else if (resaltadas->ContainsKey(i * n + j))
                    {
                        celda->Style->BackColor = Color::FromArgb(92, 62, 40);
                        celda->Style->ForeColor = CNaranja();
                    }
                    else if (modoMatriz == 1 && valor > 0)
                    {
                        celda->Style->BackColor = Color::FromArgb(40, 62, 46);
                        celda->Style->ForeColor = CVerde();
                    }
                    else if (modoMatriz == 1 && capacidad[i, j] > 0 && valor >= capacidad[i, j] && valor > 0)
                    {
                        celda->Style->BackColor = Color::FromArgb(70, 40, 52);
                        celda->Style->ForeColor = CRosa();
                    }
                    else if (valor == 0)
                    {
                        celda->Style->BackColor = CPanel2();
                        celda->Style->ForeColor = Color::FromArgb(110, 114, 140);
                    }
                    else
                    {
                        celda->Style->BackColor = CPanel2();
                        celda->Style->ForeColor = Color::White;
                    }
                }
            }
        }

        void ConstruirCombos()
        {
            cmbFuente->Items->Clear();
            cmbSumidero->Items->Clear();
            for (int i = 0; i < n; i++)
            {
                cmbFuente->Items->Add(L"v" + i.ToString());
                cmbSumidero->Items->Add(L"v" + i.ToString());
            }
            if (n > 0)
            {
                cmbFuente->SelectedIndex = 0;
                cmbSumidero->SelectedIndex = n - 1;
            }
        }

        String^ TextoCamino(List<int>^ camino)
        {
            String^ s = L"";
            for (int k = 0; k < camino->Count; k++)
            {
                s += L"v" + camino[k].ToString();
                if (k < camino->Count - 1) s += L" \u2192 ";
            }
            return s;
        }

       
        String^ ConstruirExplicacion(PasoInfo^ p)
        {
            String^ nl = Environment::NewLine;
            StringBuilder^ sb = gcnew StringBuilder();

            if (pasoActual == 0)
            {
                sb->Append(L"RED INICIAL" + nl + nl);
                sb->Append(L"Fuente: v" + fuente.ToString() + L"     Sumidero: v" + sumidero.ToString() + nl);
                sb->Append(L"Todos los flujos son 0. La red residual coincide con la matriz de capacidades." + nl + nl);
                sb->Append(L"Edmonds-Karp repite hasta que no exista camino aumentante:" + nl);
                sb->Append(L"  1. Buscar por BFS el camino aumentante con menos aristas" + nl);
                sb->Append(L"     de la fuente al sumidero en la red residual." + nl);
                sb->Append(L"  2. Calcular el cuello de botella (m\u00EDnima capacidad residual)." + nl);
                sb->Append(L"  3. Aumentar el flujo y actualizar las capacidades residuales" + nl);
                sb->Append(L"     (arista directa: -cuello, arista reversa: +cuello)." + nl + nl);
                sb->Append(L"Usa Siguiente / Reproducir para ver cada iteraci\u00F3n.");
                return sb->ToString();
            }

            if (p->esFinal)
            {
                sb->Append(L"ALGORITMO COMPLETADO" + nl + nl);
                sb->Append(L"BFS ya no encuentra ning\u00FAn camino de v" + fuente.ToString() + L" a v" + sumidero.ToString());
                sb->Append(L" en la red residual, por lo tanto el flujo es m\u00E1ximo." + nl + nl);
                sb->Append(L"FLUJO M\u00C1XIMO = " + p->flujoMax.ToString() + nl);
                sb->Append(L"Iteraciones realizadas: " + (pasos->Count - 2).ToString() + nl + nl);

                sb->Append(L"Aristas saturadas (flujo = capacidad):" + nl);
                int cont = 0;
                for (int i = 0; i < n; i++)
                    for (int j = 0; j < n; j++)
                        if (capacidad[i, j] > 0 && p->flujo[i, j] >= capacidad[i, j])
                        {
                            sb->Append(L"  v" + i.ToString() + L" \u2192 v" + j.ToString() + L"  (" + p->flujo[i, j].ToString() + L"/" + capacidad[i, j].ToString() + L")" + nl);
                            cont++;
                        }
                if (cont == 0) sb->Append(L"  (ninguna)" + nl);

                int salida = 0, entrada = 0;
                for (int j = 0; j < n; j++) { salida += p->flujo[fuente, j]; entrada += p->flujo[j, sumidero]; }
                sb->Append(nl + L"Verificaci\u00F3n: flujo que sale de v" + fuente.ToString() + L" = " + salida.ToString());
                sb->Append(L"   |   flujo que entra a v" + sumidero.ToString() + L" = " + entrada.ToString());
                return sb->ToString();
            }

            int anterior = p->flujoAcumulado - p->cuelloBotella;
            sb->Append(L"ITERACI\u00D3N " + p->iteracion.ToString() + L" \u2014 CAMINO AUMENTANTE" + nl + nl);
            sb->Append(L"1. Se busca un camino aumentante v" + fuente.ToString() + L" \u2192 v" + sumidero.ToString());
            sb->Append(L" mediante BFS" + nl + L"   sobre la red residual (el de menor n\u00FAmero de aristas)." + nl);
            sb->Append(L"2. Se encuentra:" + nl + L"   " + TextoCamino(p->camino) + nl);

            String^ mins = L"";
            for (int k = 0; k < p->cambios->Count; k++)
            {
                mins += p->cambios[k]->residualAntes.ToString();
                if (k < p->cambios->Count - 1) mins += L", ";
            }
            sb->Append(L"3. Cuello de botella = min(" + mins + L") = " + p->cuelloBotella.ToString() + nl);
            sb->Append(L"4. Se aumenta el flujo en " + p->cuelloBotella.ToString() + L" a lo largo del camino." + nl);
            sb->Append(L"5. Se actualizan las capacidades residuales:" + nl);
            sb->Append(L"   directas -" + p->cuelloBotella.ToString() + L", reversas +" + p->cuelloBotella.ToString());

            bool hayReversa = false;
            for (int k = 0; k < p->cambios->Count; k++) if (p->cambios[k]->reversa) hayReversa = true;
            if (hayReversa)
                sb->Append(nl + L"   (este camino usa una arista REVERSA: cancela flujo previamente asignado)");
            sb->Append(nl);
            sb->Append(L"6. Flujo acumulado = " + anterior.ToString() + L" + " + p->cuelloBotella.ToString() + L" = " + p->flujoAcumulado.ToString());
            return sb->ToString();
        }

        void LlenarTablaCambios(PasoInfo^ p)
        {
            gridCambios->Rows->Clear();

            if (p->esFinal)
            {
                lblCambios->Text = L"RED FINAL \u2014 ARISTAS CON FLUJO";
                for (int i = 0; i < n; i++)
                    for (int j = 0; j < n; j++)
                    {
                        if (capacidad[i, j] <= 0 || p->flujo[i, j] <= 0) continue;
                        bool sat = p->flujo[i, j] >= capacidad[i, j];
                        int idx = gridCambios->Rows->Add(gcnew array<Object^> {
                            L"v" + i.ToString() + L" \u2192 v" + j.ToString(),
                                p->residual[i, j].ToString(),
                                p->flujo[i, j].ToString() + L" / " + capacidad[i, j].ToString(),
                                sat ? L"saturada" : L"con flujo"
                        });
                        gridCambios->Rows[idx]->DefaultCellStyle->ForeColor = sat ? CRosa() : CVerde();
                    }
                return;
            }

            lblCambios->Text = L"ACTUALIZACI\u00D3N DE RESIDUALES Y FLUJO";
            if (p->cambios == nullptr || p->cambios->Count == 0) return;

            for (int k = 0; k < p->cambios->Count; k++)
            {
                CambioArista^ c = p->cambios[k];
                String^ arista = L"v" + c->desde.ToString() + L" \u2192 v" + c->hasta.ToString();
                String^ residual = c->residualAntes.ToString() + L" \u2192 " + c->residualDespues.ToString();
                String^ flujoTxt = c->flujoAntes.ToString() + L" \u2192 " + c->flujoDespues.ToString();
                if (c->reversa)
                {
                    arista += L"  (reversa)";
                    flujoTxt += L"  en v" + c->hasta.ToString() + L"\u2192v" + c->desde.ToString();
                }
                int idx = gridCambios->Rows->Add(gcnew array<Object^> { arista, residual, flujoTxt, c->nota });
                if (c->reversa)
                    gridCambios->Rows[idx]->DefaultCellStyle->ForeColor = Color::FromArgb(203, 166, 247);
                else if (c->residualDespues == 0)
                    gridCambios->Rows[idx]->DefaultCellStyle->ForeColor = CRosa();
                else
                    gridCambios->Rows[idx]->DefaultCellStyle->ForeColor = CNaranja();
            }
        }

        void ActualizarVistaPaso()
        {
            if (pasos == nullptr || pasos->Count == 0) return;
            PasoInfo^ p = pasos[pasoActual];

            String^ etiquetaPaso = L"Paso " + (pasoActual + 1).ToString() + L" de " + pasos->Count.ToString();
            lblPaso->Text = etiquetaPaso;

            txtLog->Text = ConstruirExplicacion(p);
            LlenarTablaCambios(p);

            if (p->esFinal)
            {
                lblFlujoActual->Text = L"FLUJO M\u00C1XIMO: " + p->flujoMax.ToString();
                lblFlujoActual->ForeColor = CNaranja();
            }
            else
            {
                lblFlujoActual->Text = L"Flujo actual: " + p->flujoAcumulado.ToString();
                lblFlujoActual->ForeColor = CVerde();
            }

            barra->Total = pasos->Count;
            barra->Actual = pasoActual;
            btnAnterior->Enabled = (pasoActual > 0);
            btnSiguiente->Enabled = (pasoActual < pasos->Count - 1);

            RefrescarMatriz();
            EnviarEstado(p, etiquetaPaso);
        }

        //EDMONDS-KARP (Ford-Fulkerson con BFS)


        bool BFSCaminoAumentante(array<int, 2>^ residual, int nn, int src, int snk, array<int>^ padre)
        {
            array<bool>^ visitado = gcnew array<bool>(nn);
            for (int i = 0; i < nn; i++) { visitado[i] = false; padre[i] = -1; }

            Queue<int>^ q = gcnew Queue<int>();
            q->Enqueue(src);
            visitado[src] = true;

            while (q->Count > 0)
            {
                int u = q->Dequeue();
                for (int v = 0; v < nn; v++)
                {
                    if (!visitado[v] && residual[u, v] > 0)
                    {
                        padre[v] = u;
                        visitado[v] = true;
                        if (v == snk) return true;
                        q->Enqueue(v);
                    }
                }
            }
            return false;
        }

        int EjecutarEdmondsKarp(array<int, 2>^ cap, int nn, int src, int snk, List<PasoInfo^>^ pasosList)
        {
            array<int, 2>^ residual = gcnew array<int, 2>(nn, nn);
            array<int, 2>^ flujo = gcnew array<int, 2>(nn, nn);
            for (int i = 0; i < nn; i++)
                for (int j = 0; j < nn; j++)
                {
                    residual[i, j] = cap[i, j];
                    flujo[i, j] = 0;
                }

            PasoInfo^ inicial = gcnew PasoInfo();
            inicial->camino = gcnew List<int>();
            inicial->cambios = gcnew List<CambioArista^>();
            inicial->cuelloBotella = 0;
            inicial->flujo = (array<int, 2>^)flujo->Clone();
            inicial->residual = (array<int, 2>^)residual->Clone();
            inicial->flujoAcumulado = 0;
            inicial->iteracion = 0;
            inicial->esFinal = false;
            inicial->titulo = L"Red inicial (sin flujo)";
            pasosList->Add(inicial);

            int flujoTotal = 0;
            array<int>^ padre = gcnew array<int>(nn);
            int iteracion = 0;

            while (BFSCaminoAumentante(residual, nn, src, snk, padre))
            {
                iteracion++;

                List<int>^ camino = gcnew List<int>();
                int cuello = Int32::MaxValue;
                int v = snk;
                while (v != src)
                {
                    int u = padre[v];
                    cuello = Math::Min(cuello, residual[u, v]);
                    camino->Add(v);
                    v = u;
                }
                camino->Add(src);
                camino->Reverse();

                List<CambioArista^>^ cambios = gcnew List<CambioArista^>();
                for (int k = 0; k < camino->Count - 1; k++)
                {
                    int u = camino[k];
                    int w = camino[k + 1];

                    CambioArista^ c = gcnew CambioArista();
                    c->desde = u;
                    c->hasta = w;
                    c->reversa = (cap[u, w] <= 0);
                    c->residualAntes = residual[u, w];
                    c->flujoAntes = c->reversa ? flujo[w, u] : flujo[u, w];

                    residual[u, w] -= cuello;
                    residual[w, u] += cuello;

                    if (cap[u, w] > 0)
                        flujo[u, w] += cuello;
                    else
                        flujo[w, u] -= cuello;

                    c->residualDespues = residual[u, w];
                    c->flujoDespues = c->reversa ? flujo[w, u] : flujo[u, w];
                    if (c->reversa) c->nota = L"reversa (cancela flujo)";
                    else if (c->residualDespues == 0) c->nota = L"saturada";
                    else c->nota = L"adelante";
                    cambios->Add(c);
                }
                flujoTotal += cuello;

                PasoInfo^ paso = gcnew PasoInfo();
                paso->camino = camino;
                paso->cambios = cambios;
                paso->cuelloBotella = cuello;
                paso->flujo = (array<int, 2>^)flujo->Clone();
                paso->residual = (array<int, 2>^)residual->Clone();
                paso->flujoAcumulado = flujoTotal;
                paso->iteracion = iteracion;
                paso->esFinal = false;
                paso->titulo = L"Iteraci\u00F3n " + iteracion.ToString() + L" \u2014 camino aumentante (cuello = " + cuello.ToString() + L")";
                pasosList->Add(paso);
            }

            PasoInfo^ final = gcnew PasoInfo();
            final->camino = gcnew List<int>();
            final->cambios = gcnew List<CambioArista^>();
            final->cuelloBotella = 0;
            final->flujo = (array<int, 2>^)flujo->Clone();
            final->residual = (array<int, 2>^)residual->Clone();
            final->flujoAcumulado = flujoTotal;
            final->iteracion = iteracion;
            final->esFinal = true;
            final->titulo = L"Red de flujo \u00F3ptima";
            pasosList->Add(final);

            for (int k = 0; k < pasosList->Count; k++) pasosList[k]->flujoMax = flujoTotal;

            return flujoTotal;
        }

        //  VISTA DE LA RED (WebBrowser + canvas JS, modos 2D y 3D)

        void InvocarJs(String^ funcion, array<Object^>^ args)
        {
            if (!paginaLista || web3D->Document == nullptr) return;
            try { web3D->Document->InvokeScript(funcion, args); }
            catch (Exception^) { }
        }

        String^ EscaparJson(String^ s)
        {
            return s->Replace(L"\\", L"\\\\")->Replace(L"\"", L"\\\"");
        }

        void AgregarMatrizJson(StringBuilder^ sb, String^ nombre, array<int, 2>^ m)
        {
            sb->Append(L"\"" + nombre + L"\":[");
            for (int i = 0; i < n; i++)
            {
                sb->Append(L"[");
                for (int j = 0; j < n; j++)
                {
                    sb->Append(m[i, j].ToString());
                    if (j < n - 1) sb->Append(L",");
                }
                sb->Append(L"]");
                if (i < n - 1) sb->Append(L",");
            }
            sb->Append(L"]");
        }

        String^ ConstruirJsonEstado(PasoInfo^ p, String^ etiquetaPaso)
        {
          
            int h = 17;
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    h = h * 31 + capacidad[i, j];

            StringBuilder^ sb = gcnew StringBuilder();
            sb->Append(L"{");
            sb->Append(L"\"n\":" + n.ToString() + L",");
            sb->Append(L"\"fuente\":" + fuente.ToString() + L",");
            sb->Append(L"\"sumidero\":" + sumidero.ToString() + L",");
            sb->Append(L"\"titulo\":\"" + EscaparJson(p->titulo) + L"\",");
            sb->Append(L"\"etiquetaPaso\":\"" + EscaparJson(etiquetaPaso) + L"\",");
            sb->Append(L"\"claveCap\":" + h.ToString() + L",");
            sb->Append(L"\"esFinal\":" + (p->esFinal ? L"true" : L"false") + L",");
            sb->Append(L"\"flujoMax\":" + p->flujoMax.ToString() + L",");
            sb->Append(L"\"flujoAcumulado\":" + p->flujoAcumulado.ToString() + L",");
            sb->Append(L"\"cuello\":" + p->cuelloBotella.ToString() + L",");

            AgregarMatrizJson(sb, L"capacidad", capacidad); sb->Append(L",");
            AgregarMatrizJson(sb, L"flujo", p->flujo);       sb->Append(L",");
            AgregarMatrizJson(sb, L"residual", p->residual); sb->Append(L",");

            sb->Append(L"\"camino\":[");
            for (int k = 0; k < p->camino->Count; k++)
            {
                sb->Append(p->camino[k].ToString());
                if (k < p->camino->Count - 1) sb->Append(L",");
            }
            sb->Append(L"]");

            sb->Append(L"}");
            return sb->ToString();
        }

        void EnviarEstado(PasoInfo^ p, String^ etiquetaPaso)
        {
            if (p == nullptr || n <= 0 || capacidad == nullptr) return;
            String^ json = ConstruirJsonEstado(p, etiquetaPaso);
            ultimoJson = json;
            InvocarJs(L"actualizarGrafo", gcnew array<Object^> { json });
        }

        void LimpiarVista3D()
        {
            ultimoJson = nullptr;
            InvocarJs(L"limpiarGrafo", nullptr);
        }

        void IniciarVista3D()
        {
            try
            {
                String^ html = ConstruirHtml3D();
                String^ carpeta = Path::Combine(Path::GetTempPath(), L"FlujoMaximo3D");
                if (!Directory::Exists(carpeta))
                    Directory::CreateDirectory(carpeta);
                String^ ruta = Path::Combine(carpeta, L"grafo3d.html");
                File::WriteAllText(ruta, html, System::Text::Encoding::UTF8);
                web3D->Navigate(ruta);
            }
            catch (Exception^ ex)
            {
                MessageBox::Show(L"No se pudo preparar la vista de la red: " + ex->Message, L"Aviso",
                    MessageBoxButtons::OK, MessageBoxIcon::Warning);
            }
        }

       
        String^ ConstruirHtml3D()
        {
            return gcnew String(LR"CH0(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<style>
  html, body { margin:0; padding:0; overflow:hidden; background:#181826; }
  #lienzo { display:block; cursor:grab; }
  #ayuda { position:absolute; left:12px; bottom:8px; color:#8b90ad; font-family:Segoe UI, Arial, sans-serif; font-size:12px; }
</style>
</head>
<body>
<canvas id="lienzo"></canvas>
<div id="ayuda"></div>
<script>
var canvas = document.getElementById("lienzo");
var ctx = canvas.getContext("2d");
var ayuda = document.getElementById("ayuda");

var estado = null;
var modo = "3d";
var claveLayout = "";
var unit3 = [], unit2 = [];
var angY = 0.6, angX = -0.35, zoom3 = 1.0, pan3X = 0, pan3Y = 0;
var zoom2 = 1.0, pan2X = 0, pan2Y = 0;
var arr = false, tipoArr = "", nodoArr = -1, ultX = 0, ultY = 0;
var R3 = 200;
var MARGEN_SUP = 74, MARGEN_INF = 36;

var COL = {
  fuente: [166,227,161], sumidero: [243,139,168], normal: [137,180,250],
  camino: [250,179,135], saturada: [243,139,168], flujo: [166,227,161], libre: [104,108,136]
};
function rgb(c, a) {
  if (a === undefined) return "rgb(" + c[0] + "," + c[1] + "," + c[2] + ")";
  return "rgba(" + c[0] + "," + c[1] + "," + c[2] + "," + a + ")";
}
function claro(c) { return "rgb(" + Math.min(255, c[0] + 70) + "," + Math.min(255, c[1] + 70) + "," + Math.min(255, c[2] + 70) + ")"; }
function oscuro(c) { return "rgb(" + Math.max(0, c[0] - 70) + "," + Math.max(0, c[1] - 70) + "," + Math.max(0, c[2] - 70) + ")"; }
function radioNodo(esc) { return Math.max(7, 21 * esc); }

function actualizarAyuda() {
  ayuda.innerHTML = (modo === "3d")
    ? "Arrastrar: rotar &nbsp;|&nbsp; Clic derecho o Shift+arrastrar: desplazar &nbsp;|&nbsp; Rueda: zoom &nbsp;|&nbsp; Doble clic: ajustar vista"
    : "Arrastrar v&eacute;rtice: mover &nbsp;|&nbsp; Arrastrar fondo: desplazar &nbsp;|&nbsp; Rueda: zoom &nbsp;|&nbsp; Doble clic: ajustar vista";
}

function ajustarTamano() {
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
  R3 = Math.min(canvas.width, canvas.height) * 0.40;
  if (estado) ajustarVista(); else dibujar();
}
window.addEventListener("resize", ajustarTamano);
)CH0"
LR"CH1(
function esfera(n) {
  var pts = [];
  if (n <= 1) { pts.push({ x: 0, y: 0, z: 0 }); return pts; }
  var off = 2.0 / n, inc = Math.PI * (3.0 - Math.sqrt(5.0));
  for (var i = 0; i < n; i++) {
    var y = (i * off) - 1 + off / 2;
    var r = Math.sqrt(Math.max(0, 1 - y * y));
    var phi = i * inc;
    pts.push({ x: Math.cos(phi) * r, y: y, z: Math.sin(phi) * r });
  }
  return pts;
}

/* Distribucion de nodos:
   3D -> esfera de Fibonacci (fuente arriba, sumidero abajo).
   2D -> capas por distancia BFS desde la fuente (fuente a la izquierda,
         sumidero a la derecha), como un diagrama de red clasico. */
function calcularLayouts() {
  var n = estado.n, f = estado.fuente, s = estado.sumidero, cap = estado.capacidad;
  var base = esfera(n), orden = [f], i, k;
  for (i = 0; i < n; i++) if (i !== f && i !== s) orden.push(i);
  if (s !== f) orden.push(s);
  unit3 = new Array(n);
  for (k = 0; k < n; k++) unit3[orden[k]] = base[k];

  var dist = [], cola = [f];
  for (i = 0; i < n; i++) dist[i] = -1;
  dist[f] = 0;
  while (cola.length > 0) {
    var u = cola.shift();
    for (var v = 0; v < n; v++) if (dist[v] < 0 && cap[u][v] > 0) { dist[v] = dist[u] + 1; cola.push(v); }
  }
  var maxD = 0;
  for (i = 0; i < n; i++) if (i !== s && dist[i] > maxD) maxD = dist[i];
  var capa = [];
  for (i = 0; i < n; i++) capa[i] = (dist[i] < 0) ? maxD + 1 : dist[i];
  var L = 0;
  for (i = 0; i < n; i++) if (i !== s && capa[i] > L) L = capa[i];
  L = L + 1;
  capa[s] = L;
  var grupos = [];
  for (i = 0; i <= L; i++) grupos.push([]);
  for (i = 0; i < n; i++) grupos[capa[i]].push(i);
  unit2 = new Array(n);
  for (var c = 0; c <= L; c++) {
    var g = grupos[c], m = g.length;
    for (k = 0; k < m; k++) {
      var x = (L === 0) ? 0 : -1 + 2 * c / L;
      var y = (m === 1) ? 0 : -1 + 2 * k / (m - 1);
      if (m > 2) x += (k % 2 === 0) ? -0.05 : 0.05;
      unit2[g[k]] = { x: x, y: y };
    }
  }
}

function rotar(p) {
  var cy = Math.cos(angY), sy = Math.sin(angY);
  var x1 = p.x * cy - p.z * sy, z1 = p.x * sy + p.z * cy;
  var cx = Math.cos(angX), sx = Math.sin(angX);
  return { x: x1, y: p.y * cx - z1 * sx, z: p.y * sx + z1 * cx };
}
function proy3(i) {
  var r = rotar(unit3[i]);
  var f = 3.0 / (3.0 + r.z);
  return { x: canvas.width / 2 + pan3X + r.x * R3 * f * zoom3,
           y: canvas.height / 2 + pan3Y + r.y * R3 * f * zoom3,
           escala: f * zoom3, z: r.z };
}
function proy2(i) {
  var p = unit2[i];
  return { x: canvas.width / 2 + pan2X + p.x * canvas.width * 0.40 * zoom2,
           y: canvas.height / 2 + pan2Y + p.y * canvas.height * 0.36 * zoom2,
           escala: Math.sqrt(zoom2), z: 0 };
}
function proyectarTodos() {
  var P = [];
  for (var i = 0; i < estado.n; i++) P.push(modo === "3d" ? proy3(i) : proy2(i));
  return P;
}
function caja(P) {
  var b = { minX: 1e9, minY: 1e9, maxX: -1e9, maxY: -1e9 };
  for (var i = 0; i < P.length; i++) {
    var r = radioNodo(P[i].escala) + 30;
    if (P[i].x - r < b.minX) b.minX = P[i].x - r;
    if (P[i].x + r > b.maxX) b.maxX = P[i].x + r;
    if (P[i].y - r < b.minY) b.minY = P[i].y - r;
    if (P[i].y + r > b.maxY) b.maxY = P[i].y + r;
  }
  return b;
}
/* Ajusta zoom y desplazamiento para que toda la red quepa en el panel. */
function ajustarVista() {
  if (!estado) { dibujar(); return; }
  if (modo === "3d") { zoom3 = 1; pan3X = 0; pan3Y = 0; } else { zoom2 = 1; pan2X = 0; pan2Y = 0; }
  var dispW = canvas.width - 24, dispH = canvas.height - MARGEN_SUP - MARGEN_INF;
  for (var it = 0; it < 3; it++) {
    var b = caja(proyectarTodos());
    var fz = Math.min(dispW / (b.maxX - b.minX), dispH / (b.maxY - b.minY));
    if (modo === "3d") zoom3 = Math.max(0.15, Math.min(10, zoom3 * fz));
    else zoom2 = Math.max(0.15, Math.min(10, zoom2 * fz));
  }
  var b2 = caja(proyectarTodos());
  var dx = canvas.width / 2 - (b2.minX + b2.maxX) / 2;
  var dy = (MARGEN_SUP + dispH / 2) - (b2.minY + b2.maxY) / 2;
  if (modo === "3d") { pan3X += dx; pan3Y += dy; } else { pan2X += dx; pan2Y += dy; }
  dibujar();
}
)CH1"
LR"CH2(
function flecha(x2, y2, ang, tam, color) {
  ctx.beginPath();
  ctx.moveTo(x2, y2);
  ctx.lineTo(x2 - tam * Math.cos(ang - Math.PI / 7), y2 - tam * Math.sin(ang - Math.PI / 7));
  ctx.lineTo(x2 - tam * Math.cos(ang + Math.PI / 7), y2 - tam * Math.sin(ang + Math.PI / 7));
  ctx.closePath();
  ctx.fillStyle = color;
  ctx.fill();
}

function etiqueta(texto, x, y, tf, color, negrita) {
  ctx.font = (negrita ? "bold " : "") + tf + "px Segoe UI, Arial, sans-serif";
  var w = ctx.measureText(texto).width;
  ctx.fillStyle = "rgba(24,24,38,0.92)";
  ctx.fillRect(x - w / 2 - 3, y - tf / 2 - 2, w + 6, tf + 4);
  ctx.fillStyle = color;
  ctx.fillText(texto, x - w / 2, y + tf / 2 - 3);
}

function dibujarArista(i, j, P, enCam, enRev, hayCamino) {
  var f = estado.flujo[i][j], c = estado.capacidad[i][j], r = estado.residual[i][j];
  var esCam = enCam[i + "_" + j] === true, esRev = enRev[i + "_" + j] === true;
  var color, grosor, alpha = 1, punteada = false;
  if (esCam) { color = COL.camino; grosor = 3.8; }
  else if (esRev) { color = COL.camino; grosor = 3.0; punteada = true; }
  else if (f >= c) { color = COL.saturada; grosor = 2.4; punteada = true; alpha = hayCamino ? 0.5 : 0.95; }
  else if (f > 0) { color = COL.flujo; grosor = 2.2; alpha = hayCamino ? 0.5 : 0.95; }
  else { color = COL.libre; grosor = 1.2; alpha = hayCamino ? 0.35 : 0.75; }

  var pi = P[i], pj = P[j], esc = (pi.escala + pj.escala) / 2;
  var dx = pj.x - pi.x, dy = pj.y - pi.y;
  var dist = Math.sqrt(dx * dx + dy * dy) || 1;
  var ux = dx / dist, uy = dy / dist;
  var rA = radioNodo(pi.escala), rB = radioNodo(pj.escala);
  var off = (estado.capacidad[j][i] > 0) ? 6 * esc : 0;
  var px = -uy, py = ux;
  var x1 = pi.x + ux * rA + px * off, y1 = pi.y + uy * rA + py * off;
  var x2 = pj.x - ux * (rB + 2) + px * off, y2 = pj.y - uy * (rB + 2) + py * off;

  ctx.save();
  ctx.globalAlpha = alpha;
  ctx.strokeStyle = rgb(color);
  ctx.lineWidth = Math.max(0.7, grosor * esc);
  if (punteada && ctx.setLineDash) ctx.setLineDash([7 * esc, 4 * esc]);
  ctx.beginPath();
  ctx.moveTo(x1, y1);
  ctx.lineTo(x2, y2);
  ctx.stroke();
  if (ctx.setLineDash) ctx.setLineDash([]);
  flecha(x2, y2, Math.atan2(y2 - y1, x2 - x1), Math.max(5, 8 * esc), rgb(color));

  var largo = dist - rA - rB;
  if (esCam || esRev || largo > 34) {
    var tf = Math.max(8, 11 * esc);
    var mx = (x1 + x2) / 2 + px * 8 * esc, my = (y1 + y2) / 2 + py * 8 * esc;
    var txt = f + "/" + c;
    if (esRev) txt = "rev " + f + "/" + c;
    etiqueta(txt, mx, my, tf, rgb(color), esCam || esRev);
    if (esCam || esRev) {
      var tf2 = Math.max(7, 9 * esc);
      etiqueta("res: " + (esRev ? estado.residual[j][i] : r), mx, my + tf + 2, tf2, "rgba(250,179,135,0.85)", false);
    }
  }
  ctx.restore();
}

function dibujarNodo(i, P, enNodo) {
  var p = P[i], rn = radioNodo(p.escala);
  var base;
  if (i === estado.fuente) base = COL.fuente;
  else if (i === estado.sumidero) base = COL.sumidero;
  else if (enNodo[i]) base = COL.camino;
  else base = COL.normal;

  if (enNodo[i]) {
    ctx.beginPath();
    ctx.arc(p.x, p.y, rn + 8, 0, Math.PI * 2);
    ctx.fillStyle = "rgba(250,179,135,0.30)";
    ctx.fill();
    ctx.beginPath();
    ctx.arc(p.x, p.y, rn + 4, 0, Math.PI * 2);
    ctx.strokeStyle = rgb(COL.camino);
    ctx.lineWidth = 2.5;
    ctx.stroke();
  }

  ctx.beginPath();
  ctx.arc(p.x + 3, p.y + 5, rn, 0, Math.PI * 2);
  ctx.fillStyle = "rgba(0,0,0,0.35)";
  ctx.fill();

  var grad = ctx.createRadialGradient(p.x - rn * 0.35, p.y - rn * 0.4, rn * 0.15, p.x, p.y, rn);
  grad.addColorStop(0, claro(base));
  grad.addColorStop(1, oscuro(base));
  ctx.beginPath();
  ctx.arc(p.x, p.y, rn, 0, Math.PI * 2);
  ctx.fillStyle = grad;
  ctx.fill();
  ctx.strokeStyle = "rgba(20,20,30,0.9)";
  ctx.lineWidth = 1.5;
  ctx.stroke();

  var tn = Math.max(8, 12 * p.escala);
  ctx.font = "bold " + tn + "px Segoe UI, Arial, sans-serif";
  ctx.fillStyle = "#101018";
  var et = "v" + i;
  ctx.fillText(et, p.x - ctx.measureText(et).width / 2, p.y + tn / 3);

  if (i === estado.fuente || i === estado.sumidero) {
    var rol = (i === estado.fuente) ? "FUENTE" : "SUMIDERO";
    var tr = Math.max(8, 10 * p.escala);
    ctx.font = "bold " + tr + "px Segoe UI, Arial, sans-serif";
    ctx.fillStyle = rgb(base);
    ctx.fillText(rol, p.x - ctx.measureText(rol).width / 2, p.y + rn + tr + 3);
  }
}
)CH2"
LR"CH3(
function leyenda() {
  var items = [["Fuente", COL.fuente, "nodo"], ["Sumidero", COL.sumidero, "nodo"],
               ["Camino aumentante", COL.camino, "linea"], ["Con flujo", COL.flujo, "linea"],
               ["Saturada", COL.saturada, "linea"], ["Sin flujo", COL.libre, "linea"]];
  var alto = items.length * 16 + 10, ancho = 150;
  var x = canvas.width - ancho - 8, y = canvas.height - alto - 8;
  ctx.fillStyle = "rgba(24,24,38,0.80)";
  ctx.fillRect(x, y, ancho, alto);
  ctx.font = "11px Segoe UI, Arial, sans-serif";
  for (var k = 0; k < items.length; k++) {
    var yy = y + 14 + k * 16;
    if (items[k][2] === "nodo") {
      ctx.beginPath(); ctx.arc(x + 14, yy - 3, 5, 0, Math.PI * 2);
      ctx.fillStyle = rgb(items[k][1]); ctx.fill();
    } else {
      ctx.strokeStyle = rgb(items[k][1]); ctx.lineWidth = 3;
      ctx.beginPath(); ctx.moveTo(x + 8, yy - 3); ctx.lineTo(x + 20, yy - 3); ctx.stroke();
    }
    ctx.fillStyle = "#cdd6f4";
    ctx.fillText(items[k][0], x + 28, yy);
  }
}

function dibujar() {
  ctx.fillStyle = "#181826";
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  if (!estado) {
    ctx.fillStyle = "#8b90ad";
    ctx.font = "italic 16px Segoe UI, Arial, sans-serif";
    var msg = "Genera la matriz para ver la red de flujo.";
    ctx.fillText(msg, (canvas.width - ctx.measureText(msg).width) / 2, canvas.height / 2);
    return;
  }

  var n = estado.n, P = proyectarTodos(), i, j, k;
  var hayCamino = estado.camino && estado.camino.length > 1;
  var enCam = {}, enRev = {}, enNodo = {};
  if (hayCamino) {
    for (k = 0; k + 1 < estado.camino.length; k++) {
      var a = estado.camino[k], b = estado.camino[k + 1];
      if (estado.capacidad[a][b] > 0) enCam[a + "_" + b] = true;
      else enRev[b + "_" + a] = true;
    }
    for (k = 0; k < estado.camino.length; k++) enNodo[estado.camino[k]] = true;
  }

  var elementos = [];
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      if (estado.capacidad[i][j] <= 0) continue;
      elementos.push({ tipo: "arista", i: i, j: j, prof: (P[i].z + P[j].z) / 2 });
    }
  }
  if (modo === "3d") {
    for (i = 0; i < n; i++) elementos.push({ tipo: "nodo", i: i, prof: P[i].z });
    elementos.sort(function (a, b) { return b.prof - a.prof; });
  }
  /* las aristas del camino se dibujan al final para que queden encima */
  var normales = [], destacadas = [];
  for (k = 0; k < elementos.length; k++) {
    var el = elementos[k];
    if (el.tipo === "arista" && (enCam[el.i + "_" + el.j] || enRev[el.i + "_" + el.j])) destacadas.push(el);
    else normales.push(el);
  }
  var todos = normales.concat(destacadas);
  for (k = 0; k < todos.length; k++) {
    var e2 = todos[k];
    if (e2.tipo === "arista") dibujarArista(e2.i, e2.j, P, enCam, enRev, hayCamino);
    else dibujarNodo(e2.i, P, enNodo);
  }
  if (modo !== "3d") for (i = 0; i < n; i++) dibujarNodo(i, P, enNodo);

  ctx.fillStyle = "#e6e6f0";
  ctx.font = "bold 15px Segoe UI, Arial, sans-serif";
  ctx.fillText(estado.titulo, 14, 26);
  if (estado.etiquetaPaso) {
    ctx.fillStyle = "#fab387";
    ctx.font = "bold 13px Segoe UI, Arial, sans-serif";
    ctx.fillText(estado.etiquetaPaso, canvas.width - ctx.measureText(estado.etiquetaPaso).width - 14, 26);
  }
  if (hayCamino) {
    var cam = "Camino: ";
    for (k = 0; k < estado.camino.length; k++) cam += "v" + estado.camino[k] + (k < estado.camino.length - 1 ? " \u2192 " : "");
    cam += "    Cuello de botella: " + estado.cuello + "    Flujo acumulado: " + estado.flujoAcumulado;
    ctx.fillStyle = "#fab387";
    ctx.font = "13px Segoe UI, Arial, sans-serif";
    ctx.fillText(cam, 14, 48);
  }
  if (estado.esFinal) {
    var t = "ALGORITMO COMPLETADO   \u2014   FLUJO M\u00C1XIMO: " + estado.flujoMax;
    ctx.font = "bold 20px Segoe UI, Arial, sans-serif";
    var tw = ctx.measureText(t).width;
    var bx = (canvas.width - tw) / 2 - 18, by = 38;
    ctx.fillStyle = "rgba(166,227,161,0.16)";
    ctx.fillRect(bx, by, tw + 36, 38);
    ctx.strokeStyle = "rgba(166,227,161,0.9)";
    ctx.lineWidth = 1.5;
    ctx.strokeRect(bx, by, tw + 36, 38);
    ctx.fillStyle = "#a6e3a1";
    ctx.fillText(t, bx + 18, by + 26);
  }
  leyenda();
}
)CH3"
LR"CH4(
canvas.oncontextmenu = function () { return false; };
function posMouse(e) {
  var r = canvas.getBoundingClientRect();
  return { x: e.clientX - r.left, y: e.clientY - r.top };
}
function nodoEn(x, y) {
  if (!estado) return -1;
  var P = proyectarTodos(), mejor = -1, mejorZ = 1e9;
  for (var i = 0; i < P.length; i++) {
    var dx = P[i].x - x, dy = P[i].y - y;
    if (Math.sqrt(dx * dx + dy * dy) <= radioNodo(P[i].escala) + 3 && P[i].z < mejorZ) { mejor = i; mejorZ = P[i].z; }
  }
  return mejor;
}
canvas.addEventListener("mousedown", function (e) {
  if (!estado) return;
  var pm = posMouse(e);
  ultX = pm.x; ultY = pm.y; arr = true;
  var pan = (e.button === 2) || e.shiftKey;
  if (modo === "2d") {
    var k = pan ? -1 : nodoEn(pm.x, pm.y);
    if (k >= 0) { tipoArr = "nodo"; nodoArr = k; } else tipoArr = "pan";
  } else {
    tipoArr = pan ? "pan" : "rotar";
  }
  canvas.style.cursor = "grabbing";
  if (e.preventDefault) e.preventDefault();
});
window.addEventListener("mouseup", function () { arr = false; tipoArr = ""; nodoArr = -1; canvas.style.cursor = "grab"; });
window.addEventListener("mousemove", function (e) {
  if (!arr || !estado) return;
  var pm = posMouse(e);
  var dx = pm.x - ultX, dy = pm.y - ultY;
  ultX = pm.x; ultY = pm.y;
  if (tipoArr === "rotar") {
    angY += dx * 0.008; angX += dy * 0.008;
    if (angX > 1.5) angX = 1.5;
    if (angX < -1.5) angX = -1.5;
  } else if (tipoArr === "pan") {
    if (modo === "3d") { pan3X += dx; pan3Y += dy; } else { pan2X += dx; pan2Y += dy; }
  } else if (tipoArr === "nodo" && nodoArr >= 0) {
    unit2[nodoArr].x += dx / (canvas.width * 0.40 * zoom2);
    unit2[nodoArr].y += dy / (canvas.height * 0.36 * zoom2);
  }
  dibujar();
});
canvas.addEventListener("wheel", function (e) {
  if (e.preventDefault) e.preventDefault();
  if (!estado) return;
  var pm = posMouse(e);
  var factor = (e.deltaY > 0) ? 0.88 : 1.14;
  var cx = canvas.width / 2, cy = canvas.height / 2;
  if (modo === "3d") {
    var nz = Math.max(0.15, Math.min(12, zoom3 * factor));
    factor = nz / zoom3; zoom3 = nz;
    pan3X = (pm.x - cx) - ((pm.x - cx) - pan3X) * factor;
    pan3Y = (pm.y - cy) - ((pm.y - cy) - pan3Y) * factor;
  } else {
    var nz2 = Math.max(0.15, Math.min(12, zoom2 * factor));
    factor = nz2 / zoom2; zoom2 = nz2;
    pan2X = (pm.x - cx) - ((pm.x - cx) - pan2X) * factor;
    pan2Y = (pm.y - cy) - ((pm.y - cy) - pan2Y) * factor;
  }
  dibujar();
}, false);
canvas.addEventListener("dblclick", function () { ajustarVista(); });

/* ---------- API llamada desde C++ ---------- */
window.actualizarGrafo = function (jsonStr) {
  try {
    var nuevo = JSON.parse(jsonStr);
    var clave = nuevo.n + "_" + nuevo.fuente + "_" + nuevo.sumidero + "_" + nuevo.claveCap;
    var reajustar = false;
    if (clave !== claveLayout) {
      estado = nuevo;
      calcularLayouts();
      claveLayout = clave;
      reajustar = true;
    }
    estado = nuevo;
    if (reajustar) ajustarVista(); else dibujar();
  } catch (err) { }
};
window.setModo = function (m) {
  modo = (m === "2d") ? "2d" : "3d";
  actualizarAyuda();
  if (estado) ajustarVista(); else dibujar();
};
window.ajustarVista = ajustarVista;
window.limpiarGrafo = function () { estado = null; claveLayout = ""; dibujar(); };

actualizarAyuda();
ajustarTamano();
</script>
</body>
</html>
)CH4");
        }
    };
}