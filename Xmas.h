#pragma once

class Xmas {
private:
    int state = 1;
    int mode = 0;
    long timer = 0;

public:
    void Enable();
    void Disable();
    void Run();
};