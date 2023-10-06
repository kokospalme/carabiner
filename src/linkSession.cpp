#include "linker.h"


void LinkSession::print(){
    if(millis() >= printTimer + 50){
        printTimer = millis();
        Serial.println(getBeatphase());
    }
}

// Start Session
void LinkSession::start(){
    if(started != false){
        timestampStart = micros();
        started = true;
    }
}

// Stop Session
void LinkSession::stop(){
    started = false;
}

//set tempo in microseconds per Beat
void LinkSession::setTempo(uint64_t tempo_mspb){
    tempo_msperBeat = tempo_mspb;
}

uint64_t LinkSession::getTempo(){
    return tempo_msperBeat;
}

void LinkSession::setBeatphase(uint64_t phase){

}

uint64_t LinkSession::getBeatphase(){
    uint64_t deltaTime = micros() - timestampStart;
    uint64_t beatphase = deltaTime * 4000 / tempo_msperBeat;
    return beatphase;
}


double LinkSession::getTempoReadable(){
    long double _msPerBeat = (long double) tempo_msperBeat;
    long double _tempo = 60000.0 / _msPerBeat;
    _tempo *= 1000;
    // double _tempo2 = (double) _tempo;    //ToDo irrelavant?
    return (double) _tempo;
}