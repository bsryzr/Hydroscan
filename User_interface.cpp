//USER INTERFACE WINDOWS FORM APPLICATION CODE
using System;
using System.ComponentModel;
using System.Globalization;
using System.IO.Ports;
using System.Windows.Forms;
using System.Drawing;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Button;

namespace WinFormsApp5
{
    public partial class Form1 : Form
    {
        SerialPort serialPort = new SerialPort();
        string dataBuffer = string.Empty;
        string lastReceivedData = string.Empty;
        BackgroundWorker backgroundWorker;
        bool isReading = false;

        public Form1()
        {
            InitializeComponent();

            // SerialPort ayarlarını yapılandırın
            serialPort.PortName = "COM12"; // Kullandığınız port adını belirtin
            serialPort.BaudRate = 9600;
            serialPort.Parity = Parity.None;
            serialPort.DataBits = 8;
            serialPort.StopBits = StopBits.One;
            serialPort.Handshake = Handshake.None;

            // BackgroundWorker ayarlarını yapın
            backgroundWorker = new BackgroundWorker();
            backgroundWorker.DoWork += new DoWorkEventHandler(BackgroundWorker_DoWork);
            backgroundWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(BackgroundWorker_RunWorkerCompleted);

            // this.Controls.Remove(groupBox1); 

            RoundedGroupBox roundedGroupBox = new RoundedGroupBox();
            // roundedGroupBox.Text = "Yuvarlak Köşeli GroupBox";
            roundedGroupBox.CornerRadius = 20;
            roundedGroupBox.Size = new Size(200, 100);
            roundedGroupBox.Location = new Point(10, 10);
            this.Controls.Add(roundedGroupBox);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            chart1.Series["Series1"].Color = Color.Red; // İstediğiniz rengi buraya ayarlayın
            // Seri portu açın
            try
            {
                if (!serialPort.IsOpen)
                {
                    serialPort.Open();
                    MessageBox.Show("Seri port açıldı.");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Seri port açılırken hata: " + ex.Message);
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (serialPort.IsOpen)
            {
                serialPort.Close();
            }
        }

        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (isReading)
            {
                string data = serialPort.ReadExisting();
                lock (dataBuffer)
                {
                    dataBuffer += data;
                }

                // Veri tam alındıysa işleme alın
                if (dataBuffer.Contains("\n") && !backgroundWorker.IsBusy)
                {
                    backgroundWorker.RunWorkerAsync(dataBuffer);
                    dataBuffer = string.Empty;
                }
            }
        }

        private void BackgroundWorker_DoWork(object? sender, DoWorkEventArgs e)
        {
            string data = (string)e.Argument!;
            e.Result = data;
        }

        private void BackgroundWorker_RunWorkerCompleted(object? sender, RunWorkerCompletedEventArgs e)
        {
            string data = (string)e.Result!;
            lastReceivedData = data; // En son alınan veriyi saklayın
            DisplayReceivedData(data); // Alınan veriyi hemen göster
        }

        private void DisplayReceivedData(string data)
        {
            if (InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { DisplayReceivedData(data); }));
            }
            else
            {
                // Tüm gelen veriyi textBoxTemperature'a yazdır
                textBoxTemperature.AppendText(data + Environment.NewLine);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            isReading = !isReading;

            if (isReading)
            {
                serialPort.DataReceived += SerialPort_DataReceived;
                MessageBox.Show("Seri port okuma başlatıldı.");
            }
            else
            {
                serialPort.DataReceived -= SerialPort_DataReceived;
                MessageBox.Show("Seri port okuma durduruldu.");
            }

            // Değerleri kontrol et ve resimleri ayarla
            string[] lines = textBoxTemperature.Lines;
            foreach (var line in lines)
            {
                if (line.Contains("TemperatureValue"))
                {
                    string temperatureValueStr = line.Split(':')[1].Trim();
                    if (double.TryParse(temperatureValueStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double temperatureValue))
                    {
                        if (temperatureValue >= 10 && temperatureValue <= 25)
                        {
                            pictureBox1.Image = Properties.Resources.green;
                        }
                        else
                        {
                            pictureBox1.Image = Properties.Resources.redd;
                        }
                    }
                }
                else if (line.Contains("tdsValue"))
                {
                    string tdsValueStr = line.Split(':')[1].Trim();
                    if (double.TryParse(tdsValueStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double tdsValue))
                    {
                        if (tdsValue >= 50 && tdsValue <= 500)
                        {
                            pictureBox2.Image = Properties.Resources.green;
                        }
                        else
                        {
                            pictureBox2.Image = Properties.Resources.redd;
                        }
                    }
                }
                else if (line.Contains("TurbidityValue"))
                {
                    string turbidityValueStr = line.Split(':')[1].Trim();
                    if (double.TryParse(turbidityValueStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double turbidityValue))
                    {
                        if (turbidityValue >= 0)
                        {
                            pictureBox3.Image = Properties.Resources.green;
                        }
                        else
                        {
                            pictureBox3.Image = Properties.Resources.redd;
                        }
                    }
                }
                else if (line.Contains("phValue"))
                {
                    string phValueStr = line.Split(':')[1].Trim();
                    if (double.TryParse(phValueStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double phValue))
                    {
                        if (phValue >= 6.3 && phValue <= 8.3)
                        {
                            pictureBox4.Image = Properties.Resources.green;
                        }
                        else
                        {
                            pictureBox4.Image = Properties.Resources.redd;
                        }
                    }
                }
            }
        }

        private void buttontds_Click_1(object sender, EventArgs e)
        {
            chart1.Series["Series1"].Points.Clear();
            chart1.Titles.Clear();
            chart1.Titles.Add("TDS");
            chart1.ChartAreas[0].AxisY.Minimum = 50;
            chart1.ChartAreas[0].AxisY.Maximum = 500;

            // textBoxTemperature'dan tdsValue değerini al ve grafikte göster
            string[] lines = textBoxTemperature.Lines;
            foreach (var line in lines)
            {
                if (line.Contains("tdsValue"))
                {
                    string tdsValueStr = line.Split(':')[1].Trim();
                    if (double.TryParse(tdsValueStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double tdsValue))
                    {
                        chart1.Series["Series1"].Points.AddY(tdsValue);
                    }
                }
            }
        }

        private void buttonturbidity_Click_1(object sender, EventArgs e)
        {
            chart1.Series["Series1"].Points.Clear();
            chart1.Titles.Clear();
            chart1.Titles.Add("TURBIDITY");
            chart1.ChartAreas[0].AxisY.Minimum = 0;
            chart1.ChartAreas[0].AxisY.Maximum = 1;

            // textBoxTemperature'dan turbidityValue değerini al ve grafikte göster
            string[] lines = textBoxTemperature.Lines;
            foreach (var line in lines)
            {
                if (line.Contains("TurbidityValue"))
                {
                    string turbidityValueStr = line.Split(':')[1].Trim();
                    if (double.TryParse(turbidityValueStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double turbidityValue))
                    {
                        chart1.Series["Series1"].Points.AddY(turbidityValue);
                    }
                }
            }
        }

        private void buttonph_Click_1(object sender, EventArgs e)
        {
            chart1.Series["Series1"].Points.Clear();
            chart1.Titles.Clear();
            chart1.Titles.Add("PH");
            chart1.ChartAreas[0].AxisY.Minimum = 0;
            chart1.ChartAreas[0].AxisY.Maximum = 14;

            if (!string.IsNullOrEmpty(lastReceivedData))
            {
                string[] lines = lastReceivedData.Split(new[] { "\r\n", "\r", "\n" }, StringSplitOptions.None);
                foreach (var line in lines)
                {
                    if (line.Contains("phValue"))
                    {
                        string phValueStr = line.Split(':')[1].Trim();
                        if (double.TryParse(phValueStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double phValue))
                        {
                            chart1.Series["Series1"].Points.AddY(phValue);
                        }
                    }
                }
            }
        }

        private void buttontemp_Click(object sender, EventArgs e)
        {
            // chart1'in başlığını ve serisini temizle
            chart1.Series["Series1"].Points.Clear();
            chart1.Titles.Clear();
            chart1.Titles.Add("Temperature");
            chart1.ChartAreas[0].AxisY.Minimum = 0;
            chart1.ChartAreas[0].AxisY.Maximum = 100;

            // textBoxTemperature'dan temperatureValue değerini al ve grafikte göster
            string[] lines = textBoxTemperature.Lines;
            foreach (var line in lines)
            {
                if (line.Contains("TemperatureValue"))
                {
                    string temperatureValueStr = line.Split(':')[1].Trim();
                    if (double.TryParse(temperatureValueStr, NumberStyles.Any, CultureInfo.InvariantCulture, out double temperatureValue))
                    {
                        // Değerin doğru parse edilip edilmediğini kontrol edin
                        if (temperatureValue > 0 && temperatureValue <= 100)
                        {
                            chart1.Series["Series1"].Points.AddY(temperatureValue);
                        }
                    }
                }
            }
        }

        private void buttonAbout_Click(object sender, EventArgs e)
        {
            string aboutText = "                                                                                                                                                                                                                                                                                                                                                                                                                              Monitoring water quality is essential for ecosystems and human health, but traditional methods are often slow and expensive. An autonomous surface vehicle has been developed to provide continuous, real-time measurements of important water quality parameters such as pH, temperature, turbidity, and dissolved oxygen. This innovation improves monitoring efficiency and coverage, providing timely data to support environmental management and policy decisions, thereby enhancing the protection and management of water resources.A catamaran boat was chosen to carry out this task. The autonomous vehicle can operate in both automatic and manual modes, and it sends four types of data (TDS, Temperature, pH, Turbidity) to the ground station.";
            textBox1.Text = aboutText;
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
} } }




