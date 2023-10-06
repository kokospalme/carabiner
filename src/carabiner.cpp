#include "carabiner.h"



/*Set the minimum interval between updates delivered by Carabiner when changes are occurring on the Link session. The default value is 20, to deliver updates up to fifty times per second.
*/
bool Carabiner::setUpdateInterval(int interval_ms){
    if(interval_ms <=1 && interval_ms > 1000){
        interval_ms = UPDATEINTERVAL_MS_DEFAULT;   //set to default 
        return false;
    }
    else{
        updateInterval_ms = interval_ms;
        return true;
    }
    
}