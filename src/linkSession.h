#ifndef linksession_h
#define linksession_h
#include <Arduino.h>

class LinkSession{
    public:
    void start();
    void stop();
    void print();

    void setTempo(uint64_t tempo_mspb);
    uint64_t getTempo();
    double getTempoReadable();

    void setBeatphase(uint64_t phase);
    uint64_t getBeatphase();
    private:
    bool started = false;
    uint64_t tempo_msperBeat = 500000;    // bpm = 60.000.000 / tempo --> 50.0000->120bpm
    uint64_t timestampStart = 0;
    uint64_t beatSinceStart = 0;   //important!
    uint64_t timesSinceStart = 0;

    uint64_t printTimer = 0;

};

#endif