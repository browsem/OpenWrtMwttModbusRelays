class DualSerial : public Print {
public:
    DualSerial(Print &a, Print &b) : out1(a), out2(b) , bEnable1(true), bEnable2(true) {}
    void enableFirst(bool state) { bEnable1 = state; }
    void enableSecond(bool state) { bEnable2 = state; }
    void clear() {
        if (bEnable1) out1.write("\033[2J\033[H");
        if (bEnable2) out2.write("\033[2J\033[H");
    }

    bool isfirstEnabled() const  { return bEnable1; }
    bool issecondEnabled() const { return bEnable2; }

    size_t write(uint8_t c) override {
        if (bEnable1) out1.write(c);
        if (bEnable2) out2.write(c);
        return 1;
    }
    size_t write(const uint8_t *buffer, size_t size) override {
        if (bEnable1) out1.write(buffer, size);
        if (bEnable2) out2.write(buffer, size);
        return size;
    }
private:
    Print &out1;
    Print &out2;
    bool bEnable1;
    bool bEnable2;

};
