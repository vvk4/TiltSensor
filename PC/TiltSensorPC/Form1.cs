using System;
using System.IO.Ports;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.Threading;
using System.Windows.Forms.DataVisualization.Charting;

namespace ControlBalance
{
    public partial class Form1 : Form
    {
        Thread readThread;
        int CntSerialPort;
        int BtRcPrev, CntRec;
        bool GettingPacket_FL, StopCharts;
        const int SizeOfRecArray = 110;
#pragma warning disable IDE0044 
        private byte[] PacketRec = new byte[SizeOfRecArray];
#pragma warning restore IDE0044 
        uint Timestamp;
        short AccX, AccY, AccZ;
        short GyroX, GyroY, GyroZ;
        float TiltX, TiltY;
        double TiltXAccel, TiltYAccel;
        byte[] TrmArray = new byte[255];
        List<uint> TimestampList = new List<uint>();
        List<short> AccXList = new List<short>();
        List<short> AccYList = new List<short>();
        List<short> AccZList = new List<short>();
        List<short> GyroXList = new List<short>();
        List<short> GyroYList = new List<short>();
        List<short> GyroZList = new List<short>();
        List<float> TiltXList = new List<float>();
        List<float> TiltYList = new List<float>();

        StreamWriter sw;



        const byte HEADER1 = 0x39;
        const byte HEADER2 = 0xC3;


        //команды из контроллера
        const byte CMD_ACCEL_DATA = 1;

        //команды в контроллер
        const byte K_ACCEL_GYRO = 2;
        const byte CALIBRATE_ACCEL_GYRO = 3;



        public Form1()
        {
            GettingPacket_FL = false;
            StopCharts = false;
            InitializeComponent();
            COMPorts();
            comboBoxPorts.Text = Properties.Settings.Default.COMPort;

            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 1;
            comboBox3.SelectedIndex = 2;

        }


        private void buttonRenew_Click(object sender, EventArgs e)
        {
            COMPorts();
            comboBoxPorts.Text = "";
        }
        private void COMPorts()
        {
            string[] ports = System.IO.Ports.SerialPort.GetPortNames();
            comboBoxPorts.Items.Clear();
            for (int i = 0; i < ports.Length; i++)
                comboBoxPorts.Items.Add(ports[i]);
        }



        int OnConnect()
        {


            if (!serialPort1.IsOpen)
            {
                serialPort1.PortName = comboBoxPorts.Text;

                serialPort1.BaudRate = 460800;// 256000;
                serialPort1.ReadBufferSize = 1000000;

                serialPort1.Parity = Parity.None;
                serialPort1.DataBits = 8;
                serialPort1.StopBits = StopBits.One;
                serialPort1.Handshake = Handshake.None;

                serialPort1.ReadTimeout = 100;
                serialPort1.WriteTimeout = 100;



                try
                {
                    serialPort1.Open();
                }
                catch (IOException)
                {

                    try
                    {
                        serialPort1.Close();
                    }
                    catch (System.IO.IOException)
                    {
                    }
                    MessageBox.Show("Не удалось установить соединение. Проверьте COM-порт и откройте его заново", "Ошибка", MessageBoxButtons.OK);
                    return 1;
                }



                backgroundWorker1.RunWorkerAsync();


                MessageBox.Show("Соединение установлено", "Информация", MessageBoxButtons.OK);


                Properties.Settings.Default.COMPort = serialPort1.PortName;
                Properties.Settings.Default.Save();

                return 0;
            }
            else
                return 1;
        }



        void ProcessByte(byte BtRc)
        {

            if (!GettingPacket_FL)
            {
                if ((BtRcPrev == HEADER1) && (BtRc == HEADER2))
                {
                    BtRcPrev = 0;
                    GettingPacket_FL = true;
                    CntRec = 0;
                }
                else
                {
                    BtRcPrev = (byte)BtRc;
                }
            }
            else
            {
                if (CntRec > 100)
                    GettingPacket_FL = false;
                else
                {
                    PacketRec[CntRec] = (byte)BtRc;
                    if (PacketRec[0] > 100)
                    {
                        GettingPacket_FL = false;
                    }
                    CntRec++;
                    if (CntRec > (PacketRec[0] - 3))
                    {
                        GettingPacket_FL = false;



                        if (CalcCheckSumm(PacketRec, PacketRec[0] - 3, 0) == PacketRec[PacketRec[0] - 3])
                        {
                            //Есть пакет
                            ProcessData();
                        }
                        else
                        {
                            CntRec = 0;
                        }
                    }
                }
            }
        }
        unsafe void ProcessData()
        {
            byte Cmd = PacketRec[1];
            int Cnt = 2;

            switch (Cmd)
            {
                case CMD_ACCEL_DATA:
                    fixed (byte* Ptr = &PacketRec[Cnt])
                    {
                        uint* p = (uint*)Ptr;
                        Timestamp = *p;
                    }
                    Cnt += 4;

                    fixed (byte* Ptr = &PacketRec[Cnt])
                    {
                        short* p = (short*)Ptr;
                        AccX = *p;
                    }
                    Cnt += +sizeof(short);

                    fixed (byte* Ptr = &PacketRec[Cnt])
                    {
                        short* p = (short*)Ptr;
                        AccY = *p;
                    }
                    Cnt += sizeof(short);

                    fixed (byte* Ptr = &PacketRec[Cnt])
                    {
                        short* p = (short*)Ptr;
                        AccZ = *p;
                    }
                    Cnt += sizeof(short);

                    fixed (byte* Ptr = &PacketRec[Cnt])
                    {
                        short* p = (short*)Ptr;
                        GyroX = *p;
                    }
                    Cnt += sizeof(short);


                    fixed (byte* Ptr = &PacketRec[Cnt])
                    {
                        short* p = (short*)Ptr;
                        GyroY = *p;
                    }
                    Cnt += sizeof(short);

                    fixed (byte* Ptr = &PacketRec[Cnt])
                    {
                        short* p = (short*)Ptr;
                        GyroZ = *p;
                    }
                    Cnt += sizeof(short);

                    fixed (byte* Ptr = &PacketRec[Cnt])
                    {
                        float* p = (float*)Ptr;
                        TiltX = *p;
                    }
                    Cnt += sizeof(float);

                    fixed (byte* Ptr = &PacketRec[Cnt])
                    {
                        float* p = (float*)Ptr;
                        TiltY = *p;
                    }
                    Cnt += sizeof(float);


                    TimestampList.Add(Timestamp);
                    AccXList.Add(AccX);
                    AccYList.Add(AccY);
                    AccZList.Add(AccZ);
                    GyroXList.Add(GyroX);
                    GyroYList.Add(GyroY);
                    GyroZList.Add(GyroZ);
                    TiltXList.Add(TiltX);
                    TiltYList.Add(TiltY);

                    break;
                default:
                    break;

            }
        }


        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            backgroundWorker1.CancelAsync();
            GC.Collect();


            if (sw != null)
            {
                sw.Close();
                sw = null;
            }

        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                try
                {
                    serialPort1.Close();
                }
                catch (System.IO.IOException)
                {
                }
            }

            if (sw != null)
            {
                sw.Close();
                sw = null;
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {

            if (comboBoxPorts.SelectedIndex == -1)
            {
                button_connect.Enabled = false;
                buttonClose.Enabled = false;
                labelPortStatus.Text = "Порт закрыт";
                labelPortStatus.ForeColor = Color.Red;
                buttonWrite.Enabled = false;
            }
            else
                if (!serialPort1.IsOpen)
            {
                button_connect.Enabled = true;
                buttonClose.Enabled = false;
                labelPortStatus.Text = "Порт закрыт";
                labelPortStatus.ForeColor = Color.Red;
                buttonWrite.Enabled = false;
            }
            else
            {
                button_connect.Enabled = false;
                buttonClose.Enabled = true;
                labelPortStatus.Text = "Порт открыт";
                labelPortStatus.ForeColor = Color.Green;
                buttonWrite.Enabled = true;
            }

            if (CntSerialPort > 0)
            {
                CntSerialPort--;
                if (CntSerialPort == 0)
                {
                    GettingPacket_FL = false;//Таймаут порта
                }
            }



            if (!serialPort1.IsOpen)
            {
                if (readThread != null)
                    if (readThread.IsAlive)
                    {
                        readThread.Abort();
                        readThread = null;
                    }
                if (sw != null)
                {
                    sw.Close();
                    sw = null;
                }

            }
            
            if (sw != null)
            {
                buttonWrite.Text = "Остановить запись в файл";
                buttonWrite.ForeColor = Color.Red;
            }
            else
            {
                buttonWrite.Text = "Начать запись в файл";
                buttonWrite.ForeColor = Color.Blue;
            }

            

        }






        public byte CalcCheckSumm(byte[] ChkArray, int n, int Strt)
        {
            uint summ = 0, j;

            for (j = 0; j < n; j++)
                summ += ChkArray[j + Strt];

            summ = ~summ;

            return (byte)summ;

        }



        private void backgroundWorker1_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {

            switch (e.ProgressPercentage)
            {
                case CMD_ACCEL_DATA:

                    var time = TimeSpan.FromMilliseconds(Timestamp);
                    string Str = time.ToString();
                    Str = Str.Substring(0, Str.Length - 4);

                    if (sw != null)
                    {
                        sw.Write(AccX + "       ");
                        sw.Write(AccY + "       ");
                        sw.Write(AccZ + "       ");
                        sw.Write(GyroX + "       ");
                        sw.Write(GyroY + "       ");
                        sw.Write(GyroZ + "       ");
                        sw.Write(TiltX + "       ");
                        sw.Write(TiltY + "       ");
                        sw.Write(Str);
                        sw.WriteLine("");

                    }



                    int i;
                    for (i = 0; i < AccYList.Count; i++)
                    {
                        if (!backgroundWorker1.CancellationPending)
                        {
                            if (!StopCharts)
                            {
                                if (checkBox2.Checked)
                                    Charting(chart1.Series[0].Points, comboBox1.SelectedIndex, i);
                                if (checkBox3.Checked)
                                    Charting(chart1.Series[1].Points, comboBox2.SelectedIndex, i);
                                if (checkBox4.Checked)
                                    Charting(chart1.Series[2].Points, comboBox3.SelectedIndex, i);
                            }

                            if ((chart1.Series[0].Points.Count >= 1000) && !checkBox1.Checked)
                                chart1.Series[0].Points.RemoveAt(0);
                            if ((chart1.Series[1].Points.Count >= 1000) && !checkBox1.Checked)
                                chart1.Series[1].Points.RemoveAt(0);
                            if ((chart1.Series[2].Points.Count >= 1000) && !checkBox1.Checked)
                                chart1.Series[2].Points.RemoveAt(0);


                        }
                    }



                    if (!backgroundWorker1.CancellationPending)
                        chart1.ChartAreas[0].RecalculateAxesScale();


                    label1.Text = Str;
                    if (AccXList.Count != 0)
                    {
                        label2.Text = AccXList[AccXList.Count - 1].ToString();
                        label3.Text = AccYList[AccYList.Count - 1].ToString();
                        label9.Text = AccZList[AccZList.Count - 1].ToString();
                        label14.Text = GyroXList[AccZList.Count - 1].ToString();
                        label13.Text = GyroYList[AccZList.Count - 1].ToString();
                        label10.Text = GyroZList[AccZList.Count - 1].ToString();
                        label20.Text = String.Format("{0:0.00}", TiltXList[AccZList.Count - 1]);
                        label19.Text = String.Format("{0:0.00}", TiltYList[AccZList.Count - 1]);
                        label26.Text = String.Format("{0:0.00}", TiltXAccel);
                        label25.Text = String.Format("{0:0.00}", TiltYAccel);
                    }

                    TimestampList.Clear();
                    AccXList.Clear();
                    AccYList.Clear();
                    AccZList.Clear();
                    GyroXList.Clear();
                    GyroYList.Clear();
                    GyroZList.Clear();
                    TiltXList.Clear();
                    TiltYList.Clear();
                    break;
            }

        }

        private void backgroundWorker1_DoWork(object sender, DoWorkEventArgs e)
        {
            byte BtRc;
            int byteRecieved;
            ulong messByteCnt;



            while (true)
            {
                Thread.Sleep(100);
                try
                {
                    BtRc = 0;
                    if (serialPort1.IsOpen)
                    {
                        try
                        {
                            do
                            {
                                byteRecieved = serialPort1.BytesToRead;
                            }
                            while (byteRecieved < 20);

                            byte[] messByte = new byte[byteRecieved];
                            serialPort1.Read(messByte, 0, byteRecieved);
                            messByteCnt = 0;

                            do
                            {
                                BtRc = messByte[messByteCnt++];
                                ProcessByte(BtRc);
                            }
                            while (messByteCnt < (ulong)messByte.Length);
                            messByte = null;

                            if (backgroundWorker1.CancellationPending)
                            {
                                e.Cancel = true;
                                break;
                            }


                            backgroundWorker1.ReportProgress(CMD_ACCEL_DATA);


                        }
                        catch (System.ArgumentNullException)
                        {

                        }
                        catch (System.IO.IOException)
                        {

                            try
                            {
                                serialPort1.Close();
                            }
                            catch (System.IO.IOException)
                            {
                            }
                            String str = "Таймаут чтения порта";
                            MessageBox.Show(str, "Ошибка", MessageBoxButtons.OK);
                            return;
                        }


                    }
                    else
                    {
                        try
                        {
                            serialPort1.Close();
                        }
                        catch (System.IO.IOException)
                        {
                        }
                        MessageBox.Show("Связь с устройством потеряна.", "Ошибка", MessageBoxButtons.OK);
                        return;
                    }




                    CntSerialPort = 5;//Счетчик таймаута



                }
                catch (TimeoutException)
                {


                }
            }

        }

        private void chart1_Click(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {


        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (!checkBox1.Checked)
            {
                chart1.Series[0].Points.Clear();
                chart1.Series[1].Points.Clear();
                chart1.Series[2].Points.Clear();
            }
        }

        private void button2_Click_1(object sender, EventArgs e)
        {
            TrmArray[0] = HEADER1;
            TrmArray[1] = HEADER2;
            TrmArray[2] = 1;//N
            TrmArray[3] = CALIBRATE_ACCEL_GYRO;
            TrmArray[4] = CalcCheckSumm(TrmArray, TrmArray[2] + 1, 2);
            Trm();
        }

        private void numericUpDown10_ValueChanged(object sender, EventArgs e)
        {

            short Trmm = Decimal.ToInt16(numericUpDown10.Value);

            if (Trmm < 10)
                Trmm = (short)(Trmm * 100);
            if (Trmm < 100)
                Trmm = (short)(Trmm * 10);


            TrmArray[0] = HEADER1;
            TrmArray[1] = HEADER2;
            TrmArray[2] = 3;//N
            TrmArray[3] = K_ACCEL_GYRO;
            TrmArray[4] = (byte)Trmm;
            TrmArray[5] = (byte)(Trmm >> 8);
            TrmArray[6] = CalcCheckSumm(TrmArray, TrmArray[2] + 1, 2);
            Trm();

        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox2.Checked)
                chart1.Series[0].Enabled = true;
            else
                chart1.Series[0].Enabled = false;
        }

        private void checkBox3_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox3.Checked)
                chart1.Series[1].Enabled = true;
            else
                chart1.Series[1].Enabled = false;
        }

        private void checkBox4_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox4.Checked)
                chart1.Series[2].Enabled = true;
            else
                chart1.Series[2].Enabled = false;
        }

        public void Trm()
        {
            if (serialPort1.IsOpen == false)
                return;

            try
            {
                serialPort1.Write(TrmArray, 0, (TrmArray[2] + 4));
            }
            catch (System.TimeoutException)
            {
                try
                {
                    serialPort1.Close();
                }
                catch (System.IO.IOException)
                {
                }

                return;
            }
            catch (System.IO.IOException)
            {
                try
                {
                    serialPort1.Close();
                }
                catch (System.IO.IOException)
                {
                }
                MessageBox.Show("COM-port error", "Error", MessageBoxButtons.OK);
                return;
            }




        }





        private void button1_Click_1(object sender, EventArgs e)
        {
            chart1.Series[0].Points.Clear();
            chart1.Series[1].Points.Clear();
            chart1.Series[2].Points.Clear();
        }

        private void buttonstop_Click(object sender, EventArgs e)
        {
            if (StopCharts)
            {
                StopCharts = false;
                buttonstop.Text = "Остановить";
            }
            else
            {
                StopCharts = true;
                buttonstop.Text = "Продолжить";
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (sw == null)
            {
                saveFileDialog1.FileName = "";
                saveFileDialog1.ShowDialog();
                if (saveFileDialog1.FileName != "")
                {
                    sw = new StreamWriter(saveFileDialog1.FileName, true, Encoding.ASCII);
                    sw.WriteLine("-----------------------------------------------------------------------------------------");
                    sw.WriteLine(DateTime.Now);
                    sw.WriteLine("");
                    sw.Write("AccX       AccY       AccZ       GyroX     GyroY     GyroZ     TiltX     TiltY     Time");
                    sw.WriteLine("");
                    sw.WriteLine("");
                    sw.WriteLine("*****************************************************************************************");
                    sw.WriteLine("");
                    sw.WriteLine("");
                }
            }
            else
            {
                sw.Close();
                sw = null;
            }
        }

        private void button_connect_Click(object sender, EventArgs e)
        {
            if (comboBoxPorts.SelectedIndex != -1)
            {
                OnConnect();
            }

        }

        private void comboBoxPorts_SelectedIndexChanged(object sender, EventArgs e)
        {
            button_connect.Enabled = true;
        }
        void Charting(DataPointCollection Point, int N, int i)
        {
            double AccX = AccXList[i];
            double AccY = AccYList[i];
            double AccZ = AccZList[i];
            TiltXAccel = 0;

            if (!((AccY == 0) && (AccZ == 0)))
                TiltXAccel = Math.Atan(AccX / (Math.Sqrt(AccY * AccY + AccZ * AccZ))) * 57.296;

            TiltYAccel = 0;
            if (!((AccY == 0) && (AccZ == 0)))
                TiltYAccel = Math.Atan(AccY / (Math.Sqrt(AccX * AccX + AccZ * AccZ))) * 57.296;


            switch (N)
            {
                case 0:
                    Point.Add(AccXList[i]);
                    break;
                case 1:
                    Point.Add(AccYList[i]);
                    break;
                case 2:
                    Point.Add(AccZList[i]);
                    break;
                case 3:
                    Point.Add(GyroXList[i]);
                    break;
                case 4:
                    Point.Add(GyroYList[i]);
                    break;
                case 5:
                    Point.Add(GyroZList[i]);
                    break;
                case 6:
                    Point.Add(TiltXList[i]);
                    break;
                case 7:
                    Point.Add(TiltYList[i]);
                    break;
                case 8:
                     Point.Add(TiltXAccel);
                    break;
                case 9:
                     Point.Add(TiltYAccel);
                    break;
            }
        }

    }
}


//