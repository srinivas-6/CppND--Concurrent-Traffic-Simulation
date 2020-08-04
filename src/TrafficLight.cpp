#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include "TrafficLight.h"
#include <queue>
#include <future>

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{

    // unique lock with conditional variable
    std::unique_lock<std::mutex> lck(_mutex_MQ);

    _condition_MQ.wait(lck, [this] {return !_queue.empty();});

    // pull latest element using move semantics and remove it from queue
    T get_msg = std::move(_queue.front());
    _queue.pop_front();

    return get_msg;
}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    //use the mechanisms std::lock_guard<std::mutex>, move msg into queue
    
    std::lock_guard<std::mutex> lck(_mutex_MQ);
    _queue.emplace_back(msg);

    // notify to add a new message to queue
    _condition_MQ.notify_one();
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{

    while(1)
    {
        // call receive function to get current phase light
        auto msg = _msgqueue.receive();
       // return if green
        if(msg == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));


}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    
   /*
   
   return a random value bwteen max and min value
   https://stackoverflow.com/questions/12657962/how-do-i-generate-a-random-number-between-two-variables-that-i-have-stored
   
   */

    int max_value = 6;
    int min_value = 4;
    double rand_cycle_duration = rand()%(max_value - min_value + 1) + min_value;
    
    auto init_update = std::chrono::system_clock::now();

    while(1)
    {
       
        auto time_diff = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - init_update).count();

        // wait for 1ms bwteen two cycles

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        // toggle traffic light between red and green
        if(time_diff >= rand_cycle_duration)
        {
           if(_currentPhase == red)
               _currentPhase = green;
           else
               _currentPhase = red;            
        
                
        // send msg to message queue using move semantics
        auto _is_sent = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, &_msgqueue, std::move(_currentPhase));

        // wait for it to be sent
        _is_sent.wait();

        // reset the clock and random value for next loop
        
        rand_cycle_duration = rand()%(max_value - min_value + 1) + min_value;
        
        init_update = std::chrono::system_clock::now();


        }
    }

}

