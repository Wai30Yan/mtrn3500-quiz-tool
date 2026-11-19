// QuizTool.cpp  -  MTRN3500 quiz study aid (interactive)
// Part 1: one function per type of sample question.
// Part 2: RunExamples() shows an example call of every function.
// Part 3: a menu - pick the type of question you are working on and type in the numbers.
//
// Works as a normal C++ console program (also fine in a C++/CLI console project).
#include "pch.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstdlib>
using namespace std;

const double PI = 3.14159265358979323846;
double DegToRad(double d) { return d * PI / 180.0; }
double RadToDeg(double r) { return r * 180.0 / PI; }

// =====================================================================
// A. BYTES <-> VALUES  (float, double, int, char, ASCII, little-endian)
// =====================================================================

// "00 00 BC 41" -> {0x00, 0x00, 0xBC, 0x41}   (also accepts 0x00 style)
vector<unsigned char> ParseHexBytes(const string& text)
{
    vector<unsigned char> bytes;
    istringstream in(text);
    string token;
    while (in >> token)
        bytes.push_back((unsigned char)stoi(token, nullptr, 16));
    return bytes;
}

// {0x00, 0x00, 0xBC, 0x41} -> "00 00 BC 41"
string BytesToHex(const vector<unsigned char>& bytes)
{
    ostringstream out;
    out << hex << uppercase << setfill('0');
    for (size_t i = 0; i < bytes.size(); i++)
        out << setw(2) << (int)bytes[i] << (i + 1 < bytes.size() ? " " : "");
    return out.str();
}

// Bytes of a float in memory order (little-endian: least significant byte first)
vector<unsigned char> FloatToBytes(float value)
{
    vector<unsigned char> bytes(4);
    memcpy(bytes.data(), &value, 4);
    return bytes;
}

// Bytes of a double in memory order
vector<unsigned char> DoubleToBytes(double value)
{
    vector<unsigned char> bytes(8);
    memcpy(bytes.data(), &value, 8);
    return bytes;
}

// ASCII text -> bytes (no terminating 0)
vector<unsigned char> AsciiToBytes(const string& text)
{
    return vector<unsigned char>(text.begin(), text.end());
}

// Same bytes, read as different types.
// bigEndian = false : first byte is the LEAST significant (normal PC)
// bigEndian = true  : first byte is the MOST significant (network byte order)
void ShowBytesAs(const vector<unsigned char>& bytes, bool bigEndian = false)
{
    cout << "bytes in memory : " << BytesToHex(bytes)
        << (bigEndian ? "   <- read as BIG-endian (first byte = most significant)"
            : "   <- read as LITTLE-endian (first byte = least significant)") << "\n";
    if (bytes.size() >= 1)
        cout << "  char          : '" << (bytes[0] >= 32 && bytes[0] < 127 ? (char)bytes[0] : '?')
        << "' (code " << (int)bytes[0] << ")\n";
    if (bytes.size() >= 4)
    {
        vector<unsigned char> b4(bytes.begin(), bytes.begin() + 4);
        if (bigEndian) reverse(b4.begin(), b4.end());       // put them in little-endian order for memcpy
        int i; unsigned int u; float f;
        memcpy(&i, b4.data(), 4);
        memcpy(&u, b4.data(), 4);
        memcpy(&f, b4.data(), 4);
        cout << "  as one number : 0x" << hex << uppercase << setw(8) << setfill('0') << u
            << dec << setfill(' ') << (bigEndian ? "  (bytes as written)\n" : "  (bytes reversed)\n");
        cout << "  int           : " << i << "\n";
        cout << "  unsigned int  : " << u << "\n";
        cout << "  float         : " << setprecision(7) << f << "\n";
    }
    if (bytes.size() >= 8)
    {
        vector<unsigned char> b8(bytes.begin(), bytes.begin() + 8);
        if (bigEndian) reverse(b8.begin(), b8.end());
        double d;
        memcpy(&d, b8.data(), 8);
        cout << "  double        : " << setprecision(15) << d << "\n";
    }
}

// number -> binary text, e.g. ToBinary(5, 8) = "00000101"
string ToBinary(unsigned long long value, int width)
{
    string s;
    for (int bit = width - 1; bit >= 0; bit--)
        s += ((value >> bit) & 1ULL) ? '1' : '0';
    return s;
}

// Sign / exponent / mantissa split (IEEE-754) for a float (1+8+23, bias 127)
void ShowFloatBits(float value)
{
    unsigned int bits;
    memcpy(&bits, &value, 4);
    unsigned int sign = bits >> 31;
    unsigned int expo = (bits >> 23) & 0xFF;
    unsigned int mant = bits & 0x7FFFFF;
    cout << setprecision(7) << value << " as float\n";
    cout << "  sign     : " << sign << "\n";
    cout << "  exponent : " << ToBinary(expo, 8) << " = " << expo << "  (minus bias 127 = " << (int)expo - 127 << ")\n";
    cout << "  mantissa : " << ToBinary(mant, 23) << "   (value = 1.mantissa x 2^" << (int)expo - 127 << ")\n";
    vector<unsigned char> fb = FloatToBytes(value);
    cout << "  bytes    : " << BytesToHex(fb) << "  (little-endian memory order, normal PC)\n";
    reverse(fb.begin(), fb.end());
    cout << "  bytes    : " << BytesToHex(fb) << "  (big-endian order)\n";
}

// Same for a double (1+11+52, bias 1023)
void ShowDoubleBits(double value)
{
    unsigned long long bits;
    memcpy(&bits, &value, 8);
    unsigned long long sign = bits >> 63;
    unsigned long long expo = (bits >> 52) & 0x7FF;
    unsigned long long mant = bits & 0xFFFFFFFFFFFFFULL;
    cout << setprecision(15) << value << " as double\n";
    cout << "  sign     : " << sign << "\n";
    cout << "  exponent : " << ToBinary(expo, 11) << " = " << expo << "  (minus bias 1023 = " << (long long)expo - 1023 << ")\n";
    cout << "  mantissa : " << ToBinary(mant, 52) << "\n";
    vector<unsigned char> db = DoubleToBytes(value);
    cout << "  bytes    : " << BytesToHex(db) << "  (little-endian memory order, normal PC)\n";
    reverse(db.begin(), db.end());
    cout << "  bytes    : " << BytesToHex(db) << "  (big-endian order)\n";
}

// =====================================================================
// B. STRUCT / CLASS LAYOUT  (#pragma pack, padding, vptr, decoding bytes)
// =====================================================================

struct Member
{
    string name;
    string type;        // "char", "short", "int", "long", "long long", "float", "double"
    int count;          // 1 for a normal member, N for an array like Discards[N]
    bool isUnsigned;

    Member(const string& n, const string& t, int c = 1, bool u = false)
        : name(n), type(t), count(c), isUnsigned(u) {
    }
};

int SizeOfType(const string& type)
{
    if (type == "char")   return 1;
    if (type == "short")  return 2;
    if (type == "int")    return 4;
    if (type == "long")   return 4;      // Windows: long is 4 bytes
    if (type == "long long") return 8;
    if (type == "float")  return 4;
    if (type == "double") return 8;
    return 0;
}

struct Layout
{
    vector<string> name;      // member names (first one is "vptr" if the class has one)
    vector<string> type;
    vector<int> count;        // array length (1 = not an array)
    vector<bool> isUnsigned;
    vector<int> offset;       // start byte of each member
    vector<int> size;
    vector<int> padAfter;     // unused bytes directly after each member
    int alignment = 1;        // struct alignment = biggest min(member size, pack)
    int totalSize = 0;
    int unusedBytes = 0;
};

int RoundUp(int value, int multiple)
{
    return (value + multiple - 1) / multiple * multiple;
}

// pack      : the number in #pragma pack(push, N)
// vptrSize  : 0 for a struct, 4 (32-bit) or 8 (64-bit) for a class with a virtual function
Layout ComputeLayout(const vector<Member>& members, int pack, int vptrSize = 0)
{
    Layout L;
    vector<Member> all;
    if (vptrSize > 0)
        all.push_back(Member("vptr", "vptr"));
    all.insert(all.end(), members.begin(), members.end());

    int current = 0;                                   // next free byte
    for (size_t i = 0; i < all.size(); i++)
    {
        int elem = (all[i].type == "vptr") ? vptrSize : SizeOfType(all[i].type);
        int sz = elem * all[i].count;                  // whole member (array = count x element)
        int align = min(elem, pack);                   // alignment depends on ONE element, not the whole array
        int start = RoundUp(current, align);           // next legal address

        if (i > 0) L.padAfter[i - 1] = start - current;   // gap goes after the previous member
        string shownName = all[i].name;
        if (all[i].count > 1) shownName += "[" + to_string(all[i].count) + "]";
        L.name.push_back(shownName);
        L.type.push_back(all[i].type);
        L.count.push_back(all[i].count);
        L.isUnsigned.push_back(all[i].isUnsigned);
        L.offset.push_back(start);
        L.size.push_back(sz);
        L.padAfter.push_back(0);

        L.alignment = max(L.alignment, align);
        current = start + sz;
    }
    L.totalSize = RoundUp(current, L.alignment);       // whole struct is a multiple of its alignment
    L.padAfter.back() = L.totalSize - current;         // tail padding

    int used = 0;
    for (int s : L.size) used += s;
    L.unusedBytes = L.totalSize - used;
    return L;
}

void PrintLayout(const Layout& L)
{
    cout << left << setw(8) << "offset" << setw(16) << "member" << setw(8) << "size" << "unused bytes after\n";
    for (size_t i = 0; i < L.name.size(); i++)
        cout << left << setw(8) << L.offset[i] << setw(16) << L.name[i] << setw(8) << L.size[i] << L.padAfter[i] << "\n";
    cout << "total size = " << L.totalSize << ", unused bytes = " << L.unusedBytes
        << ", alignment = " << L.alignment << "\n";
}

// Addresses of the unused bytes (smallest first) as 16 hex digits
void PrintPaddingAddresses(const Layout& L, unsigned long long baseAddress)
{
    for (size_t i = 0; i < L.name.size(); i++)
        for (int p = 0; p < L.padAfter[i]; p++)
        {
            unsigned long long addr = baseAddress + L.offset[i] + L.size[i] + p;
            cout << "  unused byte at " << hex << uppercase << setw(16) << setfill('0') << addr
                << dec << setfill(' ') << "\n";
        }
}

// Read one value of the given type from raw bytes
string ValueFromBytes(const string& type, const unsigned char* p, bool isUnsigned = false)
{
    ostringstream out;
    if (type == "char")        out << "'" << (char)p[0] << "' (code " << (int)p[0] << ")";
    else if (type == "short") { short v;  memcpy(&v, p, 2); if (isUnsigned) out << (unsigned short)v; else out << v; }
    else if (type == "int" || type == "long")
    {
        int v; memcpy(&v, p, 4);
        if (isUnsigned) out << (unsigned int)v; else out << v;
    }
    else if (type == "long long")
    {
        long long v; memcpy(&v, p, 8);
        if (isUnsigned) out << (unsigned long long)v; else out << v;
    }
    else if (type == "float") { float v;  memcpy(&v, p, 4); out << setprecision(7) << v; }
    else if (type == "double") { double v; memcpy(&v, p, 8); out << setprecision(15) << v; }
    return out.str();
}

// Put 'bytes' into the object starting at byte 'startOffset' (e.g. 4 = leave the vptr alone)
// and show what each member becomes.
void DecodeIntoLayout(const Layout& L, const vector<unsigned char>& bytes, int startOffset,
    bool bigEndian = false, int decimals = 3)
{
    for (size_t i = 0; i < L.name.size(); i++)
    {
        if (L.type[i] == "vptr") { cout << "  " << L.name[i] << " : left unchanged\n"; continue; }
        int rel = L.offset[i] - startOffset;
        if (rel < 0 || rel + L.size[i] >(int)bytes.size())
        {
            cout << "  " << L.name[i] << " : not covered by the given bytes\n";
            continue;
        }
        cout << "  " << L.name[i] << " (" << L.type[i] << ", bytes " << L.offset[i] << "-"
            << L.offset[i] + L.size[i] - 1 << ") = ";
        if (L.count[i] > 1)
            cout << BytesToHex(vector<unsigned char>(bytes.begin() + rel, bytes.begin() + rel + L.size[i])) << "\n";
        else
        {
            unsigned char tmp[8] = { 0 };                       // copy this member's bytes ...
            memcpy(tmp, &bytes[rel], L.size[i]);
            if (bigEndian) reverse(tmp, tmp + L.size[i]);       // ... reversed if big-endian
            cout << ValueFromBytes(L.type[i], tmp, L.isUnsigned[i]);
            if (L.type[i] == "float" || L.type[i] == "double")      // also show a fixed number of decimals
            {
                double v;
                if (L.type[i] == "float") { float f; memcpy(&f, tmp, 4); v = f; }
                else { memcpy(&v, tmp, 8); }
                cout << "   ->  " << fixed << setprecision(decimals) << v << " (" << decimals << " d.p.)"
                    << defaultfloat << setprecision(6);
            }
            cout << "\n";
        }
    }
}

// =====================================================================
// C. LIDAR  (index <-> angle, range -> x,y, hex tokens)
// =====================================================================

int HexTokenToInt(const string& token) { return stoi(token, nullptr, 16); }

// LMDscandata start angle / step tokens are in 1/10000 degree
double TokenToDegrees(const string& hexToken) { return HexTokenToInt(hexToken) / 10000.0; }

double IndexToAngleDeg(int index, double startDeg, double resDeg)
{
    return startDeg + index * resDeg;
}

int AngleDegToIndex(double angleDeg, double startDeg, double resDeg)
{
    return (int)llround((angleDeg - startDeg) / resDeg);
}

// Assignment convention: x = R sin(theta), y = -R cos(theta), theta in RADIANS
void RangeToXY(double range, double angleDeg, double& x, double& y)
{
    double theta = DegToRad(angleDeg);
    x = range * sin(theta);
    y = -range * cos(theta);
}

// =====================================================================
// D. ACKERMANN STEERING
// =====================================================================

// R = L / tan(delta)   (delta = commanded centre angle)
double AckermannRadius(double L, double deltaDeg)
{
    return L / tan(DegToRad(deltaDeg));
}

// Inner (left) wheel: horizontal distance to the ICR is R - B/2
double AckermannLeftDeg(double L, double B, double deltaDeg)
{
    double R = AckermannRadius(L, deltaDeg);
    return RadToDeg(atan(L / (R - B / 2.0)));
}

// Outer (right) wheel: R + B/2
double AckermannRightDeg(double L, double B, double deltaDeg)
{
    double R = AckermannRadius(L, deltaDeg);
    return RadToDeg(atan(L / (R + B / 2.0)));
}

// =====================================================================
// E. COMMAND MESSAGE  "# steer speed flag #"
// =====================================================================

string BuildCommand(double steer, double speed, int flag, int decimals = 3)
{
    ostringstream out;
    out << fixed << setprecision(decimals) << "# " << steer << " " << speed << " " << flag << " #";
    return out.str();
}

// Longest possible message: try all four sign combinations of the limits
int MaxCommandLength(double maxSteer, double maxSpeed, int decimals = 3)
{
    size_t longest = 0;
    for (int s = -1; s <= 1; s += 2)
        for (int v = -1; v <= 1; v += 2)
            longest = max(longest, BuildCommand(s * maxSteer, v * maxSpeed, 1, decimals).size());
    return (int)longest;
}

// =====================================================================
// F. FINDING A HEADER IN A BUFFER  (e.g. GNSS 0xAA44121C)
// =====================================================================

// Method 1: cast 4 bytes to unsigned int (the constant is the bytes REVERSED: 0x1C1244AA)
int FindHeaderCast(const unsigned char* buffer, int length, unsigned int valueAsRead = 0x1C1244AA)
{
    for (int i = 0; i + 4 <= length; i++)
    {
        unsigned int v;
        memcpy(&v, buffer + i, 4);
        if (v == valueAsRead) return i;
    }
    return -1;                                  // not found
}

// Method 2 (lecture style): shift each byte into a 32-bit register
int FindHeaderShift(const unsigned char* buffer, int length, unsigned int header = 0xAA44121C)
{
    unsigned int reg = 0;
    for (int i = 0; i < length; i++)
    {
        reg = (reg << 8) | buffer[i];
        if (i >= 3 && reg == header) return i - 3;    // i = last header byte, so start = i - 3
    }
    return -1;
}

// =====================================================================
// main: one example for each function
// =====================================================================

void Title(const string& text)
{
    cout << "\n==== " << text << " ====\n";
}

void RunExamples()
{
    // ---------- A. bytes <-> values ----------
    Title("A1. float -> bytes (23.5 should be 00 00 BC 41)");
    ShowFloatBits(23.5f);

    Title("A2. double -> bytes (12345.54321)");
    ShowDoubleBits(12345.54321);

    Title("A3. ASCII text -> bytes");
    cout << "\"Hello\" = " << BytesToHex(AsciiToBytes("Hello")) << "\n";

    Title("A4. bytes -> char / int / float / double");
    ShowBytesAs(ParseHexBytes("00 00 BC 41"));
    ShowBytesAs(ParseHexBytes("66 66 66 66 66 66 2D 40"));

    // ---------- B. struct / class layout ----------
    Title("B1. pack(4): int A; double B; char C;  (address of unused bytes)");
    {
        vector<Member> m = { {"A","int"}, {"B","double"}, {"C","char"} };
        Layout L = ComputeLayout(m, 4);
        PrintLayout(L);
        PrintPaddingAddresses(L, 0xAB00E22857998DE0ULL);
    }

    Title("B2. pack(4): double A; int B; char C;  (size 16, unused 3)");
    {
        vector<Member> m = { {"A","double"}, {"B","int"}, {"C","char"} };
        PrintLayout(ComputeLayout(m, 4));
    }

    Title("B3. pack(8): char A; double B; int C;  (size 24, unused 11, 7 after A, 4 after C)");
    {
        vector<Member> m = { {"A","char"}, {"B","double"}, {"C","int"} };
        PrintLayout(ComputeLayout(m, 8));
    }

    Title("B4. class with virtual destructor, pack(1): char D; double A; int C; float B;");
    {
        vector<Member> m = { {"D","char"}, {"A","double"}, {"C","int"}, {"B","float"} };
        Layout L = ComputeLayout(m, 1, 4);              // 4 = vptr size
        PrintLayout(L);
        cout << "Store the given bytes starting at byte 4:\n";
        DecodeIntoLayout(L, ParseHexBytes("41 66 66 66 66 66 66 2D 40 00 00 BC 41 10 00 00 00"), 4);
        cout << "Same bytes if the declaration order were D, A, B, C:\n";
        vector<Member> m2 = { {"D","char"}, {"A","double"}, {"B","float"}, {"C","int"} };
        DecodeIntoLayout(ComputeLayout(m2, 1, 4), ParseHexBytes("41 66 66 66 66 66 66 2D 40 00 00 BC 41 10 00 00 00"), 4);
    }

    // ---------- C. LiDAR ----------
    Title("C1. LiDAR hex tokens");
    cout << "step token 1388 -> " << TokenToDegrees("1388") << " degrees\n";
    cout << "count token 169 -> " << HexTokenToInt("169") << " points\n";
    cout << "range token 960 -> " << HexTokenToInt("960") << " mm\n";

    Title("C2. index <-> angle (start 0 deg, 0.5 deg steps)");
    cout << "index 74 -> angle " << IndexToAngleDeg(74, 0, 0.5) << " deg\n";
    cout << "angle 90 deg -> index " << AngleDegToIndex(90, 0, 0.5) << "\n";

    Title("C3. range + index -> x, y   (range 2400 mm, index 74)");
    {
        double x, y;
        RangeToXY(2400, IndexToAngleDeg(74, 0, 0.5), x, y);
        cout << setprecision(6) << "x = " << x << ", y = " << y << "  (sin/cos use radians)\n";
    }

    // ---------- D. Ackermann ----------
    Title("D. Ackermann: L=800, B=350, delta=30 deg");
    cout << "R       = " << AckermannRadius(800, 30) << " mm\n";
    cout << "alpha_L = " << fixed << setprecision(3) << AckermannLeftDeg(800, 350, 30) << " deg\n";
    cout << "alpha_R = " << AckermannRightDeg(800, 350, 30) << " deg\n";
    cout << defaultfloat << setprecision(6);

    // ---------- E. command message ----------
    Title("E. command message (steer +-40, speed +-2, 3 decimals)");
    cout << "example   : " << BuildCommand(-40, -2, 1) << "\n";
    cout << "max length: " << MaxCommandLength(40, 2) << " bytes\n";

    // ---------- F. header search ----------
    Title("F. find the header AA 44 12 1C in a buffer");
    {
        vector<unsigned char> buf = ParseHexBytes("01 02 03 AA 44 12 1C 55 66");
        cout << "cast method : header starts at index " << FindHeaderCast(buf.data(), (int)buf.size()) << "\n";
        cout << "shift method: header starts at index " << FindHeaderShift(buf.data(), (int)buf.size()) << "\n";
    }

}

// =====================================================================
// INTERACTIVE MENU  -  reading input
// =====================================================================

// Thrown when the user types b / back: main() catches it and shows the menu again
struct BackToMenu {};

// Read a line; quit the program cleanly if the input ends; "b" or "back" goes to the main menu
string AskLine(const string& prompt)
{
    cout << prompt;
    string line;
    if (!getline(cin, line)) { cout << "\n"; exit(0); }
    if (line == "b" || line == "back") throw BackToMenu();
    return line;
}

double AskDouble(const string& prompt, bool hasDefault = false, double defaultValue = 0)
{
    string shown = prompt;
    if (hasDefault) { ostringstream d; d << " [" << defaultValue << "]"; shown += d.str(); }
    while (true)
    {
        string line = AskLine(shown + ": ");
        if (line.empty() && hasDefault) return defaultValue;
        try
        {
            size_t used = 0;
            double v = stod(line, &used);
            if (used == line.size()) return v;
        }
        catch (...) {}
        cout << "  Please type a number.\n";
    }
}

int AskInt(const string& prompt, bool hasDefault = false, int defaultValue = 0)
{
    string shown = prompt;
    if (hasDefault) shown += " [" + to_string(defaultValue) + "]";
    while (true)
    {
        string line = AskLine(shown + ": ");
        if (line.empty() && hasDefault) return defaultValue;
        try
        {
            size_t used = 0;
            int v = stoi(line, &used);
            if (used == line.size()) return v;
        }
        catch (...) {}
        cout << "  Please type a whole number.\n";
    }
}

bool AskYesNo(const string& prompt, bool defaultYes = false)
{
    string line = AskLine(prompt + (defaultYes ? " (Y/n): " : " (y/N): "));
    if (line.empty()) return defaultYes;
    return line[0] == 'y' || line[0] == 'Y';
}

vector<unsigned char> AskHexBytes(const string& prompt)
{
    while (true)
    {
        string line = AskLine(prompt);
        try
        {
            vector<unsigned char> bytes = ParseHexBytes(line);
            if (!bytes.empty()) return bytes;
        }
        catch (...) {}
        cout << "  Type hex bytes separated by spaces, e.g.  00 00 BC 41\n";
    }
}

// =====================================================================
// INTERACTIVE MENU  -  one screen per question type
// =====================================================================

void MenuNumberToBytes()
{
    cout << "\n1) float (4 bytes)   2) double (8 bytes)\n";
    int type = AskInt("Type", true, 1);
    double value = AskDouble("Number");
    cout << "\n";
    if (type == 2) ShowDoubleBits(value);
    else           ShowFloatBits((float)value);
}

void MenuBytesToValues()
{
    cout << "\nBytes in memory order, exactly as the question lists them.\n";
    vector<unsigned char> bytes = AskHexBytes("Bytes: ");
    cout << "\n";
    ShowBytesAs(bytes, false);
    cout << "\n";
    ShowBytesAs(bytes, true);
    cout << "\nUse the LITTLE-endian reading unless the question says big-endian / network byte order.\n";
    if (bytes.size() < 4) cout << "(give at least 4 bytes to see int / float, 8 for double)\n";
}

void MenuAscii()
{
    cout << "\n1) text -> bytes   2) bytes -> text\n";
    int choice = AskInt("Choose", true, 1);
    if (choice == 2)
    {
        vector<unsigned char> bytes = AskHexBytes("Bytes: ");
        string text(bytes.begin(), bytes.end());
        cout << "text = \"" << text << "\"\n";
    }
    else
    {
        string text = AskLine("Text: ");
        vector<unsigned char> bytes = AsciiToBytes(text);
        cout << "hex     : " << BytesToHex(bytes) << "\n";
        cout << "decimal :";
        for (unsigned char b : bytes) cout << " " << (int)b;
        cout << "\n";
    }
}

vector<Member> AskMembers()
{
    while (true)
    {
        string line = AskLine("Members in declaration order, e.g.\n"
            "  unsigned int Header, unsigned char Junk[40], double North, char C\n> ");
        line.erase(remove(line.begin(), line.end(), ';'), line.end());   // ignore ;

        vector<Member> members;
        bool ok = true;
        stringstream ss(line);
        string part;
        while (getline(ss, part, ','))
        {
            istringstream ps(part);
            string token;
            bool isUnsigned = false;
            vector<string> words;
            while (ps >> token)
            {
                if (token == "unsigned") isUnsigned = true;
                else if (token != "signed") words.push_back(token);
            }
            if (words.empty() && !isUnsigned) continue;           // empty piece

            auto isTypeWord = [](const string& w)
                { return w == "char" || w == "short" || w == "int" || w == "long" || w == "float" || w == "double"; };

            string name;
            if (!words.empty() && !isTypeWord(words.back()))
            {
                name = words.back();
                words.pop_back();
            }
            int count = 1;                                        // Name[40] -> Name, 40
            size_t bracket = name.find('[');
            if (bracket != string::npos)
            {
                count = atoi(name.substr(bracket + 1).c_str());
                name = name.substr(0, bracket);
            }

            string type;
            if (words.empty()) type = "int";                      // plain "unsigned"
            else if (words.size() >= 2 && words[0] == "long" && words[1] == "long") type = "long long";
            else type = words[0];

            if (SizeOfType(type) == 0 || count < 1)
            {
                cout << "  Could not read '" << part << "'. Types: char, short, int, long, long long, float, double.\n";
                ok = false;
                break;
            }
            if (name.empty()) name = "m" + to_string(members.size() + 1);
            members.push_back(Member(name, type, count, isUnsigned));
        }
        if (ok && !members.empty()) return members;
    }
}

void MenuLayout()
{
    cout << "\n-- struct / class layout --\n";
    int pack = AskInt("N in #pragma pack(push, N)  (if the question says '8 byte packing boundary', N = 8)", true, 8);
    while (pack < 1) pack = AskInt("N must be 1 or more", true, 8);
    int vptr = 0;
    if (AskYesNo("Class with a virtual function or virtual destructor?"))
        vptr = AskInt("vptr size in bytes (32-bit = 4, 64-bit = 8)", true, 4);
    vector<Member> members = AskMembers();

    Layout L = ComputeLayout(members, pack, vptr);
    cout << "\n";
    PrintLayout(L);

    string baseText = AskLine("\nAddress of the object in hex (blank to skip): ");
    if (!baseText.empty())
    {
        try
        {
            unsigned long long base = stoull(baseText, nullptr, 16);
            if (L.unusedBytes == 0) cout << "  No unused bytes.\n";
            else                    PrintPaddingAddresses(L, base);
        }
        catch (...) { cout << "  Could not read that address.\n"; }
    }

    if (AskYesNo("\nStore some bytes into the object and read the members?"))
    {
        int start = AskInt("Store from which byte number", true, vptr);
        vector<unsigned char> bytes = AskHexBytes("Bytes: ");
        bool big = AskYesNo("Are the values BIG-endian? (No = little-endian, the normal PC)");
        cout << "\n";
        DecodeIntoLayout(L, bytes, start, big);
        if (AskYesNo("\nTry the same bytes with a different member order?"))
        {
            vector<Member> other = AskMembers();
            cout << "\n";
            Layout L2 = ComputeLayout(other, pack, vptr);
            PrintLayout(L2);
            DecodeIntoLayout(L2, bytes, start, big);
        }
    }
}

void MenuPacket()
{
    cout << "\n-- packet of several values (no padding) --\n";
    vector<Member> members = AskMembers();
    vector<unsigned char> bytes = AskHexBytes("Bytes in the order they were sent: ");
    bool big = AskYesNo("Big-endian? (No = little-endian = least significant byte first)");
    int decimals = AskInt("Decimal places to show", true, 3);

    Layout L = ComputeLayout(members, 1);                      // pack(1): values follow each other
    cout << "\n";
    if ((int)bytes.size() != L.totalSize)
        cout << "WARNING: the types need " << L.totalSize << " bytes but you gave " << bytes.size() << ".\n";
    DecodeIntoLayout(L, bytes, 0, big, decimals);
    cout << "\n(Type the answer with the same number of decimal places the question or key uses.)\n";
}

void MenuLidar()
{
    cout << "\n-- LiDAR --\n";
    cout << "1) hex token -> value\n"
        << "2) index -> angle\n"
        << "3) angle -> index\n"
        << "4) range + index -> x, y\n";
    int choice = AskInt("Choose");

    if (choice == 1)
    {
        string token = AskLine("Hex token (e.g. 1388, 169, 960): ");
        try
        {
            cout << "decimal                : " << HexTokenToInt(token) << "\n";
            cout << "as degrees (/10000)    : " << TokenToDegrees(token) << "\n";
            cout << "if it is a range       : " << HexTokenToInt(token) << " mm = " << HexTokenToInt(token) / 1000.0 << " m\n";
        }
        catch (...) { cout << "  Not a hex number.\n"; }
        return;
    }

    double start = AskDouble("Start angle in degrees", true, 0);
    double res = AskDouble("Angular resolution", true, 0.5);
    if (AskYesNo("Is that resolution in RADIANS?")) res = RadToDeg(res);
    cout << "(using start " << start << " deg, resolution " << res << " deg)\n\n";

    if (choice == 2)
    {
        int index = AskInt("Index");
        cout << "angle = " << IndexToAngleDeg(index, start, res) << " degrees\n";
    }
    else if (choice == 3)
    {
        double angle = AskDouble("Angle in degrees");
        cout << "index = " << AngleDegToIndex(angle, start, res) << "\n";
    }
    else if (choice == 4)
    {
        double range;
        if (AskYesNo("Is the range given in HEX?"))
        {
            while (true)
            {
                string token = AskLine("Hex range: ");
                try { range = HexTokenToInt(token); break; }
                catch (...) { cout << "  Not a hex number.\n"; }
            }
            cout << "range = " << range << "\n";
        }
        else range = AskDouble("Range");

        double rangeMm = range;                                 // LiDAR ranges are always in mm

        int index;
        if (AskYesNo("Does the question say the Nth value, counting from 1 (e.g. '75th value')?"))
        {
            int n = AskInt("N");
            index = n - 1;
            cout << "index = N - 1 = " << index << "   (first value is index 0)\n";
        }
        else index = AskInt("Index (0 = first value)");

        double angle = IndexToAngleDeg(index, start, res);
        double xMm, yMm;
        RangeToXY(rangeMm, angle, xMm, yMm);                    // x, y in mm

        cout << "\nangle = " << angle << " deg = " << DegToRad(angle) << " rad\n";
        cout << fixed << setprecision(3);
        cout << "in mm     : x = " << xMm << "   y = " << yMm << "\n";
        cout << "in metres : x = " << xMm / 1000.0 << "   y = " << yMm / 1000.0 << "\n";
        cout << "(3 decimal places - use the line that matches the unit the question asks for)\n";
        cout << defaultfloat << setprecision(6);
    }
}

void MenuAckermann()
{
    cout << "\n-- Ackermann steering --\n";
    double L = AskDouble("L (wheelbase)");
    double B = AskDouble("B (front pivot spacing)");
    double delta = AskDouble("Commanded steering angle delta in degrees");
    double W = AskDouble("W (rear track, 0 to skip)", true, 0);

    double R = AckermannRadius(L, delta);
    cout << fixed << setprecision(3);
    cout << "\nR (centre)      = " << R << "\n";
    cout << "alpha_L (inner) = " << AckermannLeftDeg(L, B, delta) << " deg\n";
    cout << "alpha_R (outer) = " << AckermannRightDeg(L, B, delta) << " deg\n";
    if (W > 0)
    {
        cout << "R_L (left rear) = " << R - W / 2 << "\n";
        cout << "R_R (right rear)= " << R + W / 2 << "\n";
    }
    cout << defaultfloat << setprecision(6);
}

void MenuCommand()
{
    cout << "\n-- command message  # steer speed flag # --\n";
    double maxSteer = AskDouble("Maximum steering (+-)", true, 40);
    double maxSpeed = AskDouble("Maximum speed (+-)", true, 2);
    int decimals = AskInt("Maximum decimal places", true, 3);
    cout << "\nlongest message: " << BuildCommand(-maxSteer, -maxSpeed, 1, decimals) << "\n";
    cout << "length         : " << MaxCommandLength(maxSteer, maxSpeed, decimals) << " bytes\n";
}

void MenuHeader()
{
    cout << "\n-- find a header in received bytes --\n";
    vector<unsigned char> header;
    while (true)
    {
        string text = AskLine("Header bytes in the order they arrive [AA 44 12 1C]: ");
        if (text.empty()) text = "AA 44 12 1C";
        try { header = ParseHexBytes(text); }
        catch (...) { header.clear(); }
        if (header.size() == 4) break;
        cout << "  Give exactly 4 hex bytes, e.g.  AA 44 12 1C\n";
    }
    unsigned int shifted = ((unsigned int)header[0] << 24) | ((unsigned int)header[1] << 16)
        | ((unsigned int)header[2] << 8) | header[3];
    unsigned int asRead = 0;
    memcpy(&asRead, header.data(), 4);

    cout << "shift-register constant : 0x" << hex << uppercase << shifted << "\n";
    cout << "cast constant (reversed): 0x" << asRead << dec << "\n\n";

    vector<unsigned char> buffer = AskHexBytes("Received bytes: ");
    cout << "shift method: index " << FindHeaderShift(buffer.data(), (int)buffer.size(), shifted) << "\n";
    cout << "cast method : index " << FindHeaderCast(buffer.data(), (int)buffer.size(), asRead) << "\n";
    cout << "(-1 means not found)\n";
}

// =====================================================================
// main  -  the menu
// =====================================================================

int main()
{
    while (true)
    {
        cout << "\n=========== MTRN3500 quiz helper ===========\n"
            << " 1  number -> bytes   (float / double, sign-exponent-mantissa)\n"
            << " 2  bytes  -> values  (char / int / float / double)\n"
            << " 3  ASCII text <-> bytes\n"
            << " 4  struct / class layout  (#pragma pack, padding, vptr, store bytes)\n"
            << " 5  LiDAR  (index, angle, x/y, hex tokens)\n"
            << " 6  Ackermann steering\n"
            << " 7  command message length\n"
            << " 8  find a header in received bytes\n"
            << " 9  packet of several values  (e.g. two doubles then two floats)\n"
            << "10  run all the built-in examples\n"
            << " 0  quit\n";
        try
        {
            string choice = AskLine("Choose: ");

            if (choice == "0" || choice == "q" || choice == "Q") break;
            if (choice.size() == 1 && choice[0] >= '1' && choice[0] <= '9')
                cout << "(type b at any prompt to go back to this menu)\n";

            if (choice == "1") MenuNumberToBytes();
            else if (choice == "2") MenuBytesToValues();
            else if (choice == "3") MenuAscii();
            else if (choice == "4") MenuLayout();
            else if (choice == "5") MenuLidar();
            else if (choice == "6") MenuAckermann();
            else if (choice == "7") MenuCommand();
            else if (choice == "8") MenuHeader();
            else if (choice == "9") MenuPacket();
            else if (choice == "10") RunExamples();
            else cout << "Please choose a number from the menu.\n";
        }
        catch (const BackToMenu&)
        {
            cout << "\n(back to the main menu)\n";
        }
    }
    return 0;
}