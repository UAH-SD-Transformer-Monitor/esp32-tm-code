// ********* Configuration of ATM90E26 ********* //

// L line calibration gain - check datasheet () and Excel sheet
double lgain = 0x1D39;
// voltage RMS gain - check datasheet () and Excel sheet
double ugain = 0x6F0B;
// L line current gain - check datasheet () and Excel sheet
double igainl = 0x7FF4;
// voltage sag threshold - check datasheet (2.3.2) and Excel sheet
double vsag = 0x0CD5;

// PL_Constant - check datasheet (3.2.2) and Excel sheet
double PLConstH = 0x0007;
double PLConstL = 0x24A1;

// ********************************************* //