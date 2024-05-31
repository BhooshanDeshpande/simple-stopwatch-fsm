//////////////////////////////////////////////////////////////////////////////
// The following code implements a simple stopwatch state-machine
// See the UML diagram below 
//  --------------------------------
// |                                |
// |           O     Active         |
// |           |                    |<----
// |           v                    |     | EvReset
// |  ----------------------------  |     |
// | |                            | |-----
// | |         Stopped            | |
// |  ----------------------------  |
// |  |              ^              |
// |  | EvStartStop  | EvStartStop  |<-----O
// |  v              |              |
// |  ----------------------------  |
// | |                            | |
// | |         Running            | |
// |  ----------------------------  |
//  --------------------------------
//////////////////////////////////////////////////////////////////////////////


#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/transition.hpp>

#include <boost/config.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>

#ifdef BOOST_NO_STDC_NAMESPACE
namespace std
{
  using ::time;
  using ::difftime;
  using ::time_t;
}
#endif

#ifdef BOOST_INTEL
#  pragma warning( disable: 304 ) // access control not specified
#  pragma warning( disable: 444 ) // destructor for base is not virtual
#  pragma warning( disable: 981 ) // operands are evaluated in unspecified order
#endif

/*
Reference 
sc::event event_class_name
  initialize - pass the c++ class/struct
  use - state_machine.process_event( event_class_name() )
sc::state_machnine 
  initialize - pass the c++ class/struct, and first inner state
  use - .initiate()
  use - .state_cast<state>() --> this helps get to the context of the state ONLY IF state machine is in this state
sc::simple_state 
  initialize - pass the c++ class/struct, context state, and first inner state (optional)
  use - holds all the reactions and functions for that state (context)
sc::transition 
  intialize -pass the event class and destination class
  use - defines what transition the state will make on what event 
*/

namespace sc = boost::statechart;

// event class to specify as event
struct EvStartStop : sc::event< EvStartStop > {}; //event 
struct EvReset : sc::event< EvReset > {}; //event
struct Active; //simple_state
struct Stopped; //simple_state
struct Running;
struct StopWatch : sc::state_machine< StopWatch, Active > {}; //state_machine

//this class is smartly put here as an abstract function. It is implemented by both Stopped and Running states. 
// whenever this is called -- compiler will call the function of whatever state is currently active
// this eliminates having to determine what state we are currently in and then use
// state_cast < state_x > to get the context of state_x and then call the ElapsedTime function
struct IElapsedTime
{
  virtual double ElapsedTime() const = 0;
};
// current state, outer state (state machine if current state is outermost) and entry inner state
struct Active : sc::simple_state< Active, StopWatch, Stopped >
{
  public:
    // event which will trigger the transition, destination state to make a transition to
    typedef sc::transition< EvReset, Active > reactions;

    Active() : elapsedTime_( 0.0 ) {}

    double & ElapsedTime()
    {
      return elapsedTime_;
    }

    double ElapsedTime() const
    {
      return elapsedTime_;
    }

  private:
    double elapsedTime_;
};


struct Running : IElapsedTime, sc::simple_state< Running, Active >
{
  public:
    typedef sc::transition< EvStartStop, Stopped > reactions;

    Running() : startTime_( std::time( 0 ) ) {}

    ~Running()
    {
      context< Active >().ElapsedTime() = ElapsedTime();
    }

    virtual double ElapsedTime() const
    {
      return context< Active >().ElapsedTime() +
        std::difftime( std::time( 0 ), startTime_ );
    }

  private:
    std::time_t startTime_;
};

struct Stopped : IElapsedTime, sc::simple_state< Stopped, Active >
{
  typedef sc::transition< EvStartStop, Running > reactions;

  virtual double ElapsedTime() const
  {
    return context< Active >().ElapsedTime();
  }
};

// Helper functions 
char GetKey()
{
  char key;
  std::cin >> key;
  return key;
}

void clearLines(int numLines) {
    for (int i = 0; i < numLines; ++i) {
        // Move the cursor up one line
        std::cout << "\033[A";
        // Clear the line
        std::cout << "\033[K";
    }
}


//////////////////////////////////////////////////////////////////////////////
int main()
{
  std::cout << "StopWatch FSM example\n\n";
  std::cout << "s: Starts/Stops stop watch\n";
  std::cout << "r: Resets stop watch\n";
  std::cout << "d: Displays the elapsed time in seconds\n";
  std::cout << "e: Exits the program\n\n";
  std::cout << "You may chain commands, e.g. rs resets and starts stop watch\n\n";

  StopWatch stopWatch;

  //stopwatch moves to active state 
  stopWatch.initiate();

  char key = GetKey();

  while ( key != 'e' )
  {
    std::cout << "Elapsed time: " << std::endl; 
    switch( key )
    {
      case 'r':
      {
        // stopwatch elapsed time resets to zero - moves to active state 
        stopWatch.process_event( EvReset() );
        clearLines(1);
        break;    
      }

      case 's':
      {
        // stopwatch starts counting time - moves to running state && stops counting time - moves to stopped state 
        stopWatch.process_event( EvStartStop() );
        clearLines(1);
        break;
      }

      case 'd':
      {
        // abstract function helps here to just invoke elapsed time of whatever state we're in  
        std::cout << stopWatch.state_cast< const IElapsedTime & >().ElapsedTime() << "\n";
        break;
      }
      default:
      {
        std::cout << "Invalid key!\n";
        break;
      }
    }

    key = GetKey();
    clearLines(1);
  }

  return 0;
}