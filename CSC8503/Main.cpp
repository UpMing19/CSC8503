#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"
#include "MenuStates.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

vector<Vector3> testNodes;

void TestPathfinding() {

    NavigationGrid grid("TestGrid1.txt");

    NavigationPath outPath;

    Vector3 startPos(80, 0, 10);
    Vector3 endPos(80, 0, 80);

    bool found = grid.FindPath(startPos, endPos, outPath);

    Vector3 pos;
    while (outPath.PopWaypoint(pos)) {
        testNodes.push_back(pos);
    }
}

void DisplayPathfinding() {
    for (int i = 1; i < testNodes.size(); ++i) {
        Vector3 a = testNodes[i - 1];
        Vector3 b = testNodes[i];

        Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
    }
}

void TestStateMachine() {
    StateMachine *testMachine = new StateMachine();
    int data = 0;

    State *A = new State([&](float dt) -> void {
        std::cout << "I am in state A\n" << std::endl;
        data++;
    });
    State *B = new State([&](float dt) -> void {
        std::cout << "I am in state B\n" << std::endl;
        data--;
    });


    StateTransition *stateAB = new StateTransition(A, B, [&](void) -> bool {
        return data > 10;
    });
    StateTransition *stateBA = new StateTransition(B, A, [&](void) -> bool {
        return data < 0;
    });


    testMachine->AddState(A);
    testMachine->AddState(B);
    testMachine->AddTransition(stateAB);
    testMachine->AddTransition(stateBA);

    for (int i = 0; i < 100; i++) {
        testMachine->Update(1.0f);
    }


}


void TestBehaviourTree() {
    float behaviourTimer;
    float distanceToTarget;
    BehaviourAction *findKey = new BehaviourAction(" Find Key ",
                                                   [&](float dt, BehaviourState state) -> BehaviourState {
                                                       if (state == Initialise) {
                                                           std::cout << " Looking for a key !\n";
                                                           behaviourTimer = rand() % 100;
                                                           state = Ongoing;
                                                       } else if (state == Ongoing) {
                                                           behaviourTimer -= dt;
                                                           if (behaviourTimer <= 0.0f) {
                                                               std::cout << " Found a key !\n";
                                                               return Success;
                                                           }
                                                       }
                                                       return state; // will be ’ongoing ’ until success
                                                   }
    );
    BehaviourAction *goToRoom = new BehaviourAction("Go To Room ",
                                                    [&](float dt, BehaviourState state) -> BehaviourState {
                                                        if (state == Initialise) {
                                                            std::cout << " Going to the loot room !\n";
                                                            state = Ongoing;
                                                        } else if (state == Ongoing) {
                                                            distanceToTarget -= dt;
                                                            if (distanceToTarget <= 0.0f) {
                                                                std::cout << " Reached room !\n";
                                                                return Success;
                                                            }
                                                        }
                                                        return state; // will be ’ongoing ’ until success
                                                    }
    );

    BehaviourAction *openDoor = new BehaviourAction(" Open Door ",
                                                    [&](float dt, BehaviourState state) -> BehaviourState {
                                                        if (state == Initialise) {
                                                            std::cout << " Opening Door !\n";
                                                            return Success;
                                                        }
                                                        return state;
                                                    }
    );
    BehaviourAction *lookForTreasure = new BehaviourAction(" Look For Treasure ",
                                                           [&](float dt, BehaviourState state) -> BehaviourState {
                                                               if (state == Initialise) {
                                                                   std::cout << " Looking for treasure !\n";
                                                                   return Ongoing;
                                                               } else if (state == Ongoing) {
                                                                   bool found = rand() % 2;
                                                                   if (found) {
                                                                       std::cout << "I found some treasure !\n";
                                                                       return Success;
                                                                   }
                                                                   std::cout << "No treasure in here ...\ n";
                                                                   return Failure;
                                                               }
                                                               return state;
                                                           }
    );

    BehaviourAction *lookForItems = new BehaviourAction(" Look For Items ",
                                                        [&](float dt, BehaviourState state) -> BehaviourState {
                                                            if (state == Initialise) {
                                                                std::cout << " Looking for items !\n";
                                                                return Ongoing;
                                                            } else if (state == Ongoing) {
                                                                bool found = rand() % 2;
                                                                if (found) {
                                                                    std::cout << "I found some items !\n";
                                                                    return Success;
                                                                }
                                                                std::cout << "No items in here ...\ n";
                                                                return Failure;
                                                            }
                                                            return state;
                                                        }
    );


    BehaviourSequence *sequence = new BehaviourSequence(" Room Sequence ");
    sequence->AddChild(findKey);
    sequence->AddChild(goToRoom);
    sequence->AddChild(openDoor);

    BehaviourSelector *selection = new BehaviourSelector(" Loot Selection ");
    selection->AddChild(lookForTreasure);
    selection->AddChild(lookForItems);

    BehaviourSequence *rootSequence = new BehaviourSequence(" Root Sequence ");
    rootSequence->AddChild(sequence);
    rootSequence->AddChild(selection);

    for (int i = 0; i < 5; ++i) {
        rootSequence->Reset();
        behaviourTimer = 0.0f;
        distanceToTarget = rand() % 250;
        BehaviourState state = Ongoing;
        std::cout << "We ’re going on an adventure !\n";
        while (state == Ongoing) {
            state = rootSequence->Execute(1.0f); // fake dt
        }
        if (state == Success) {
            std::cout << " What a successful adventure !\n";
        } else if (state == Failure) {
            std::cout << " What a waste of time !\n";
        }
    }
    std::cout << "All done !\n";
}


int pause_flag = 0;
class PauseScreen : public PushdownState {

    PushdownResult OnUpdate(float dt, PushdownState **newState) override {
        if (Window::GetKeyboard()->KeyPressed(KeyCodes::U)) {
            return PushdownResult::Pop;
        }
        return PushdownResult::NoChange;
    }

    void OnAwake() override {
        std::cout << " Press U to unpause game !\n";
        pause_flag = 1;
    }

};

class GameScreen : public PushdownState {
    PushdownResult OnUpdate(float dt,
                            PushdownState **newState) override {
        pauseReminder -= dt;
        if (pauseReminder < 0) {
            std::cout << " Coins mined : " << coinsMined << "\n";
            std::cout << " Press P to pause game , or F1  to return to main menu!\n";
            pauseReminder += 1.0f;
        }
        if (Window::GetKeyboard()->KeyDown(KeyCodes::P)) {
            *newState = new PauseScreen();
            return PushdownResult::Push;
        }
        if (Window::GetKeyboard()->KeyDown(KeyCodes::F1)) {
            std::cout << " Returning to main menu !\n";
            return PushdownResult::Pop;
        }
        if (rand() % 70 == 0) {
            coinsMined++;
        }
        return PushdownResult::NoChange;
    };

    void OnAwake() override {
        pause_flag  =0 ;
        std::cout << " Preparing to mine coins !\n";
    }

protected :
    int coinsMined = 0;
    float pauseReminder = 1;
};

class IntroScreen : public PushdownState {
    PushdownResult OnUpdate(float dt,
                            PushdownState **newState) override {
        if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
            *newState = new GameScreen();
            return PushdownResult::Push;
        }
        if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
            return PushdownResult::Pop;
        }
        return PushdownResult::NoChange;
    };


    void OnAwake() override {
        pause_flag= 0 ;
        std::cout << " Welcome to a really awesome game !\n";
        std::cout << " Press Space To Begin or escape to quit !\n";
    }
};

void TestPushdownAutomata(Window *w) {
    PushdownMachine machine(new IntroScreen());
    while (w->UpdateWindow()) {
        float dt = w->GetTimer().GetTimeDeltaSeconds();
        if (!machine.Update(dt)) {
            return;
        }
    }
}




struct GameObjectPact:public GamePacket{
    GameObject *gameObject;
    GameObjectPact(GameObject *object){
        type = BasicNetworkMessages::Full_State;
        size = sizeof(object);
        gameObject = object;
    }
};
struct WorldObjectPact:public GamePacket{
    TutorialGame *world;
    WorldObjectPact(TutorialGame *w){
        type = BasicNetworkMessages::Full_State;
        size = sizeof(world);
        world = w;
    }
};
class TestPacketReceiver : public PacketReceiver {
public:
    TestPacketReceiver(std::string name,TutorialGame *w) {
        this->name = name;
        this->world = w;
    }

    void ReceivePacket(int type, GamePacket *payload, int source) {
        if (type == String_Message) {
            StringPacket *realPacket = (StringPacket *) payload;
            std::string msg = realPacket->GetStringFromData();
            std::cout << name << " received message: " << msg << std::endl;
        }
        if(type==Full_State){
            GameObjectPact  *realPacket = (GameObjectPact  *)payload;

        }
    }

protected:
    std::string name;
    TutorialGame *world ;
};

int main() {


  //  TestPacketReceiver clientReceiver("Client");
    //TestBehaviourTree();
    //TestStateMachine();
    //TestNetworking();
    Window *w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);
    //Window *w = Window::CreateGameWindow("CSC8503 Game technology!", 600, 300);
    //TestPushdownAutomata(w);
    if (!w->HasInitialised()) {
        return -1;
    }

    w->ShowOSPointer(false);
    w->LockMouseToWindow(true);
    //TestPathfinding();

   // TutorialGame *g = new TutorialGame();
    TutorialGame *g = new NetworkedGame();
    PushdownMachine *menuState = new PushdownMachine( new MenuState(g));
    PushdownMachine machine(new IntroScreen());

    w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
    while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
        float dt = w->GetTimer().GetTimeDeltaSeconds();
        if (dt > 0.1f) {
            std::cout << "Skipping large time delta" << std::endl;
            continue; //must have hit a breakpoint or something to have a 1 second frame time!
        }

        if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
            w->ShowConsole(true);
        }
        if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
            w->ShowConsole(false);
        }

        if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
            w->SetWindowPosition(0, 0);
        }

        w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

        machine.Update(dt);

        if(!pause_flag){
            g->UpdateGame(dt);
            menuState->Update(dt);
        }


        //DisplayPathfinding();
    }

    Window::DestroyGameWindow();

}